// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zlib.h>

struct termios tattr;
int pid, socket_fd;
int to_shell[2];
int from_shell[2];
bool to_shell_close = false;
int BUFFER_SIZE = 256;
z_stream defstream;
z_stream infstream;

struct opts {
    int port_num;
    bool compress_flag;
};

bool read_options (int argc, char* argv[], struct opts* opts);
void shut_down ();
void sigpipe_handler (int sig);
int server_connect (unsigned int port_num);


int
main(int argc, char **argv)
{  
    struct opts options;
    if (! read_options(argc, argv, &options))
    {
        fprintf(stderr, "--port=PORT_NUM option is mandatory\n");
        exit(1);
    }

    socket_fd = server_connect(options.port_num);

    // register SIGPIPE handler
    if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR)
    {
        fprintf(stderr, "signal error: %s\n", strerror(errno));
        exit(1);
    }

    // create two pipes
    if (pipe(to_shell) < 0)
    {
        fprintf(stderr, "pipe error(1st pipe): %s\n", strerror(errno));
        exit(1);
    }

    if (pipe(from_shell) < 0)
    {
        fprintf(stderr, "pipe error(2nd pipe): %s\n", strerror(errno));
        exit(1);
    }

    if (options.compress_flag)
    {
        infstream.zalloc = Z_NULL;
        infstream.zfree = Z_NULL;
        infstream.opaque = Z_NULL;
        
        int ret = inflateInit(&infstream);
        if (ret != Z_OK)
        {
            fprintf(stderr, "error when initiate inflate(decompress)\n");
            exit(1);
        }
    }

    // fork the new process
    int pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "fork error: %s\n", strerror(errno));
        exit(1);
    }
    else if (pid == 0)
    {
        // child process

        // close to_shell[1], the write end of th to_shell pipe
        if (close(to_shell[1]) < 0) {
            fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
            exit(1);
        }

        // redirect the output from terminal to stdin
        if (close(0) < 0) {
            fprintf(stderr, "close error when closing stdin: %s\n", strerror(errno));
            exit(1);
        }
        if (dup(to_shell[0]) < 0) {
            fprintf(stderr, "dup error when dup to_shell[0]: %s\n", strerror(errno));
            exit(1);
        }
        if (close(to_shell[0]) < 0) {
            fprintf(stderr, "close error when closing to_shell[0]: %s\n", strerror(errno));
            exit(1);
        }

        // close from_shell[0], read end of the from_shell pipe 
        if (close(from_shell[0]) < 0) {
            fprintf(stderr, "close error when closing from_shell[0]: %s\n", strerror(errno));
            exit(1);
        }

        // redirect stdout and stderr to the input to terminal
        if (dup2(from_shell[1], 1) < 0) {
            fprintf(stderr, "dup2 error when dup2 stdout to from_shell[1]: %s\n", strerror(errno));
            exit(1);
        }
        if (dup2(from_shell[1], 2) < 0) {
            fprintf(stderr, "dup2 error when dup2 stderr to from_shell[1]: %s\n", strerror(errno));
            exit(1);
        }

        // close from_shell[1], now it can be replaced by stdout and stderr
        if (close(from_shell[1]) < 0) {
            fprintf(stderr, "close error when closing from_shell[1]: %s\n", strerror(errno));
            exit(1);
        }
            
        // execute the shell program
        if (execlp("/bin/bash", "bash", NULL) < 0) {
            fprintf(stderr, "execlp error when execlp /bin/bash: %s\n", strerror(errno));
            exit(1);
        }

        // if the process still execute the code below, it means
        // execlp did not take up this process, there is some error
        fprintf(stderr, 
                "should not get here, error when executing the child process: %s\n", 
                 strerror(errno));
        exit(1);
    }
    else
    {
        // parent process

        // close to_shell[0], the read end of the to_shell pipe
        if (close(to_shell[0]) < 0) {
            fprintf(stderr, "close error when closing to_shell[0]: %s\n", strerror(errno));
            exit(1);
        }
        // close from_shell[1], the write end of the from_shell pipe
        if (close(from_shell[1]) < 0) {
            fprintf(stderr, "close error when closing from_shell[1]: %s\n", strerror(errno));
            exit(1);
        }

        bool shut_down_flag = false;
        // initiate the pollfd structure
        struct pollfd pollfds[2];
        pollfds[0].fd = socket_fd;
        pollfds[0].events = (POLLIN + POLLHUP + POLLERR);
        pollfds[1].fd = from_shell[0];
        pollfds[1].events = (POLLIN + POLLHUP + POLLERR);

        // keep the read-write process until need to shut down
        while ( !shut_down_flag)
        {
            // call poll to avoid the reads to block each other
            if (poll(pollfds, 2, -1) < 0) {
                fprintf(stderr, "poll error: %s\n", strerror(errno));
                exit(1);
            }

            // socket_fd ready to read
            if(pollfds[0].revents & POLLIN)
            {
                // read from socket_fd, process special characters, send to to_shell[1]
                unsigned char buffer[BUFFER_SIZE];
                int count = read(socket_fd, buffer, BUFFER_SIZE);

                if (count < 0)
                {
                    fprintf(stderr, "error when reading from client: %s\n", strerror(errno));
                    exit(1);
                }

                
                if (options.compress_flag)
                {
                    unsigned char outbuf[BUFFER_SIZE];
                    int received_bytes;
                    infstream.avail_in = count;
                    infstream.next_in = buffer;
                    infstream.avail_out = sizeof(outbuf);
                    infstream.next_out = outbuf;

                    while (infstream.avail_in > 0) {
                        inflate(&infstream, Z_SYNC_FLUSH);
                    }
                    received_bytes = sizeof(outbuf) - infstream.avail_out;
                    for (int k = 0; k < count; k++)
                    {
                        char c = outbuf[k];
                        // read ^D from client
                        if (c == 0x04)
                        {
                            // close the read end of the to_shell pipe if not closed
                            if (! to_shell_close) {
                                if (close(to_shell[1]) < 0) {
                                        fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                                        exit(1);
                                    }
                                to_shell_close = true;
                            }
                            shut_down_flag = true;
                        }
                        // read ^C from client
                        else if (c == 0x03)
                        {
                            if (kill(pid, SIGINT) < 0) {
                                fprintf(stderr, "kill error when sending SIGINT to shell: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        else if (c == '\r' || c == '\n')
                        {
                            if (write(to_shell[1], "\n", 1) < 0) {
                                fprintf(stderr, "write error when writing <lf> to shell: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        else
                        {
                            if (write(to_shell[1], &c, 1) < 0) {
                                fprintf(stderr, "write error when writing to shell: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                    }
                }
                else
                {
                    for (int k = 0; k < count; k++)
                    {
                        char c = buffer[k];
                        // read ^D from client
                        if (c == 0x04)
                        {
                            // close the read end of the to_shell pipe if not closed
                            if (! to_shell_close) {
                                if (close(to_shell[1]) < 0) {
                                        fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                                        exit(1);
                                    }
                                to_shell_close = true;
                            }
                            shut_down_flag = true;
                        }
                        // read ^C from client
                        else if (c == 0x03)
                        {
                            if (kill(pid, SIGINT) < 0) {
                                fprintf(stderr, "kill error when sending SIGINT to shell: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        else if (c == '\r' || c == '\n')
                        {
                            if (write(to_shell[1], "\n", 1) < 0) {
                                fprintf(stderr, "write error when writing <lf> to shell: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        else
                        {
                            if (write(to_shell[1], &c, 1) < 0) {
                                fprintf(stderr, "write error when writing to shell: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                    }
                }
                


            }

            // from_shell[0] ready to read
            if (pollfds[1].revents & POLLIN)
            {
                // read from from_shell[0], process special characters, send to socket_fd
                char buffer[BUFFER_SIZE];
                int count = read(from_shell[0], buffer, BUFFER_SIZE);
                if (count < 0)
                {
                    fprintf(stderr, "error when reading from shell: %s\n", strerror(errno));
                    exit(1);
                }
                for (int k = 0; k < count; k++)
                {
                    char c = buffer[k];
                    if (c == '\n')
                    {
                        if (write(socket_fd, "\r\n", 2) < 0) {
                            fprintf(stderr, "write error when writing cr lf to stdout: %s\n", strerror(errno));
                            exit(1);
                        }
                    }
                    // terminate if receiving ^D
                    else if (c == 0x04)
                    {
                        shut_down_flag = true;
                    }
                    else
                    {
                        if (write(socket_fd, &c, 1) < 0) {
                            fprintf(stderr, "write error when writing to stdout: %s\n", strerror(errno));
                            exit(1);
                        }
                    }
                }
            }
            // handling error 
            if( (pollfds[0].revents &  (POLLHUP | POLLERR) ) || 
                    (pollfds[1].revents & (POLLHUP | POLLERR)) )
            {
                // Proceed to exit process: read every last byte from from_shell[0], write
                // to socket_fd, get the exit status of the process and report to stderr.
                char c;
                bool EOF_flag = false;
                while (! EOF_flag)
                {
                    int count = read(from_shell[0], &c, 1);
                    if (count > 0)
                    {
                        if (write(socket_fd, &c, 1) < 0)
                        {
                            fprintf(stderr, 
                                "error when writing last bytes from server: %s\n", 
                                strerror(errno));
                            exit(1);
                        }
                    }
                    else if (count == 0)
                        EOF_flag = true;
                    else
                    {
                        fprintf(stderr, 
                            "error when reading the last bytes from server: %s\n", 
                            strerror(errno));
                        exit(1);
                    }
                    
                }
                shut_down_flag = true;
            }
        }

        // close the not closed pipe ends and shut down
        if (! to_shell_close) {
            if (close(to_shell[1]) < 0) {
                fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                exit(1);
            }
            to_shell_close = true;
        }
            
        if (close(from_shell[0]) < 0) {
            fprintf(stderr, "close error when closing from_shell[0]: %s\n", strerror(errno));
            exit(1);
        }

        if (options.compress_flag) {
            inflateEnd(&infstream);
        }

        shut_down();
    }

}

bool read_options (int argc, char* argv[], struct opts* opts)
{
    int opt = 0;
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"compress", no_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    bool flag = false;

    int long_index = 0;
    opts -> port_num = -1;
    opts -> compress_flag = false;
    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 'p':
            opts -> port_num = atoi(optarg);
            flag = true;
            break;
        case 'c':
            opts -> compress_flag = true;
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab1b-server [--port=PORT_NUM --compress]\n");
            exit(1);
        }
    }

    if (optind < argc)
    {
        fprintf (stderr, "non-option ARGV-elements: ");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf (stderr, "\n");
        exit(1);
    }

    return flag;
}

void shut_down ()
{
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
    {
        fprintf(stderr, "waitpid error: %s", strerror(errno));
        exit(1);
    }
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", 
                    WTERMSIG(status), WEXITSTATUS(status));
    if (close(socket_fd) < 0)
    {
        fprintf(stderr, "error when closing the socket: %s\n", strerror(errno));
        exit(1);
    }
    exit(0);
}

void sigpipe_handler (int sig)
{
    if (sig == SIGPIPE)
    {
        fprintf(stderr, "SIGPIPE signal has been received.\n");
        if (!to_shell_close)
        {
            if (close(to_shell[1]) < 0) {
                fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                exit(1);
            }
            to_shell_close = true;
        }
        if (close(from_shell[0]) < 0) {
            fprintf(stderr, "close error when closing from_shell[0]: %s\n", strerror(errno));
            exit(1);
        }

        shut_down();
    }
}

int server_connect (unsigned int port_num)
{
    int sockfd, new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    int sin_size;

    // create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "cannot create the socket: %s\n", strerror(errno));
        exit(1);
    }
    // set the address info
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port_num);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));

    // bind the socket to the IP address and port number
    int status = bind(sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr));
    if (status < 0)
    {
        fprintf(stderr, "cannot bind to the given IP and port num: %s\n", strerror(errno));
        exit(1);
    }
    if (listen(sockfd, 5) < 0)
    {
        fprintf(stderr, "listen fails: %s\n", strerror(errno));
        exit(1);
    }

    sin_size = sizeof(struct sockaddr_in);

    // wait for client's connection, their_addr stores client's address
    new_fd = accept(sockfd, (struct sockaddr*)&their_addr, (socklen_t*)&sin_size);
    if (new_fd < 0)
    {
        fprintf(stderr, "accept fails: %s\n", strerror(errno));
        exit(1);
    }
    return new_fd;
}
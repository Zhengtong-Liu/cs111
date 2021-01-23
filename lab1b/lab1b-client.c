// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>

struct termios tattr;
struct opts {
    int port_num;
    char* file_name;
    bool compress_flag;
};
int BUFFER_SIZE = 256;

void restore_and_exit (int exit_status);
bool read_options (int argc, char* argv[], struct opts* opts);
void terminal_setup ();
int client_connect (char* host_name, unsigned int port);


int
main (int argc, char **argv)
{
    // process command line arguments
    struct opts options;
    if (! read_options(argc, argv, &options))
    {
        fprintf(stderr, "--port=PORT_NUM option is mandatory\n");
        exit(1);
    }

    terminal_setup();

    int socket_fd = client_connect("localhost", options.port_num);
    

    // initiate the pollfd structure
    struct pollfd pollfds[2];
    pollfds[0].fd = socket_fd;
    pollfds[0].events = (POLLIN + POLLHUP + POLLERR);
    pollfds[1].fd = 0;
    pollfds[1].events = (POLLIN + POLLHUP + POLLERR);

    bool shut_down_flag = false;

    while (! shut_down_flag) {

        if (poll(pollfds, 2, -1) < 0) {
            fprintf(stderr, "poll error: %s\n", strerror(errno));
            restore_and_exit(1);
        }

        // socket_id ready to read
        if (pollfds[0].revents & POLLIN) {
            // read from socket_fd, process special characters, send to stdout
            char buffer[BUFFER_SIZE];
            int count = read(socket_fd, buffer, BUFFER_SIZE);
            if (count < 0)
            {
                fprintf(stderr, "error when reading from server: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            else if (count == 0)
                shut_down_flag = true;
            
            if (write(0, buffer, count) < 0)
            {
                fprintf(stderr, "error when writing to stdout: %s\n", strerror(errno));
                restore_and_exit(1);
            }
        }

        // stdin ready to read
        if (pollfds[1].revents & POLLIN) {
            // read from stdin, process special characters, send to stdout and socket_fd
            char buffer[BUFFER_SIZE];
            int count = read(0, buffer, BUFFER_SIZE);
            if (count < 0)
            {
                fprintf(stderr, "error when reading from stdin: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            if (write(socket_fd, buffer, count) < 0)
            {
                fprintf(stderr, "error when writing to the server: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            for (int k = 0; k < count; k++)
            {
                char c = buffer[k];
                if (c == '\r' || c == '\n')
                {
                    if (write(1, "\r\n", 2) < 0) {
                        fprintf(stderr, "write error when writing <cr><lf> to stdout: %s\n", strerror(errno));
                        exit(1);
                    }
                }
                else
                {
                    if (write(1, &c, 1) < 0) {
                        fprintf(stderr, "write error when writing to stdout: %s\n", strerror(errno));
                        exit(1);
                    }
                }
                
            }
        }

        // handling error 
        if( (pollfds[0].revents &  (POLLHUP | POLLERR)) ||
            (pollfds[1].revents & (POLLHUP | POLLERR))  )
        {
            // proceed to exit process: read every last byte from socket_fd, write to stdout,
            // restore terminal and exit
            char c;
            bool EOF_flag = false;
            while (! EOF_flag)
            {
                int count = read(socket_fd, &c, 1);
                if (count > 0)
                {
                    if (write(0, &c, 1) < 0)
                    {
                        fprintf(stderr, 
                            "error when writing last bytes from server: %s\n", 
                            strerror(errno));
                        restore_and_exit(1);
                    }
                }
                else if (count == 0)
                    EOF_flag = true;
                else
                {
                    fprintf(stderr, 
                        "error when reading the last bytes from server: %s\n", 
                        strerror(errno));
                    restore_and_exit(1);
                }
                
            }
            shut_down_flag = true;
        }

    }

    // close socket
    if (close(socket_fd) < 0)
    {
        fprintf(stderr, "error when closing the socket: %s\n", strerror(errno));
        restore_and_exit(1);
    }
    restore_and_exit(0);
}

void restore_and_exit (int exit_status)
{
    if (tcsetattr(0, TCSANOW, &tattr) != 0)
    {
        fprintf(stderr, "tcsetattr error(restore time): %s\n", strerror(errno));
        exit(1);
    }
    exit(exit_status);
}

bool read_options (int argc, char* argv[], struct opts* opts)
{
    int opt = 0;
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"log", required_argument, 0, 'l'},
        {"compress", no_argument, 0, 'c'},
        {0, 0, 0, 0}
    };

    bool flag = false;

    int long_index = 0;
    opts -> port_num = -1;
    opts -> file_name = NULL;
    opts -> compress_flag = false;
    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 'p':
            opts -> port_num = atoi(optarg);
            flag = true;
            break;
        case 'l':
            opts -> file_name = optarg;
            break;
        case 'c':
            opts -> compress_flag = true;
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab1b-client [--port=PORT_NUM --log=FILENAME --compress]\n");
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

void terminal_setup ()
{
    if (! isatty(0))
    {
        fprintf(stderr, "File descriptor 0 is not for terminal");
        exit(1);
    }

    if (tcgetattr(0, &tattr) != 0)
    {
        fprintf(stderr, "tcgetattr(1st time) error: %s\n", strerror(errno));
        exit(1);
    }

    struct termios temp;
    if (tcgetattr(0, &temp) != 0)
    {
        fprintf(stderr, "tcgetattr(2nd time) error: %s\n", strerror(errno));
        exit(1);
    }

    temp.c_iflag = ISTRIP; temp.c_oflag = 0; temp.c_lflag = 0;
    if (tcsetattr(0, TCSANOW, &temp) != 0)
    {
        fprintf(stderr, "tcsetattr error(set time): %s\n", strerror(errno));
        exit(1);
    }
}

int client_connect(char* host_name, unsigned int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server;
    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "cannot create the socket: %s\n", strerror(errno));
        restore_and_exit(1);
    }
    // fill in socket address information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    server = gethostbyname(host_name);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero));
    // connect socket with corresponding address
    int status = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (status < 0)
    {
        fprintf(stderr, "cannot connect to port: %d: %s\n", port, strerror(errno));
        restore_and_exit(1);
    }
    return sockfd;

}
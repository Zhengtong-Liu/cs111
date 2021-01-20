// NAME: Zhengtong Liu
// EMAIL: ericliu2023@g.ucla.edu
// ID: 505375562

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

struct termios tattr;
int pid;
int to_shell[2];
int to_terminal[2];
bool to_shell_close = false;

void restore_and_exit (int exit_status)
{
    if (tcsetattr(0, TCSANOW, &tattr) != 0)
    {
        fprintf(stderr, "tcsetattr error(restore time): %s\n", strerror(errno));
        exit(1);
    }
    exit(exit_status);
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

bool read_options (int argc, char* argv[])
{
    int opt = 0;
    static struct option long_options[] = {
        {"shell", no_argument, 0, 's'},
        {0, 0, 0, 0}
    };

    int long_index = 0;
    bool shell_flag = false;
    while ((opt = getopt_long(argc, argv, "",
                    long_options, &long_index)) != -1) {
        switch (opt)
        {
        case 's':
            shell_flag = true;
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab1a [--shell]\n");
            restore_and_exit(1);
        }
    }

    if (optind < argc)
    {
        fprintf (stderr, "non-option ARGV-elements: ");
        while (optind < argc)
            fprintf(stderr, "%s", argv[optind++]);
        fprintf (stderr, "\n");
        restore_and_exit(1);
    }

    return shell_flag;
}

void shut_down ()
{
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
    {
        fprintf(stderr, "waitpid error: %s", strerror(errno));
        restore_and_exit(1);
    }
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", 
                    WTERMSIG(status), WEXITSTATUS(status));
    restore_and_exit(0);
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
                restore_and_exit(1);
            }
            to_shell_close = true;
        }
        if (close(to_terminal[0]) < 0) {
            fprintf(stderr, "close error when closing to_terminal[0]: %s\n", strerror(errno));
            restore_and_exit(1);
        }

        shut_down();
    }
}

int
main(int argc, char **argv)
{

    terminal_setup();

    bool shell_flag = read_options(argc, argv);

    int read_size = 256;

    if (shell_flag)
    {
        if (pipe(to_shell) < 0)
        {
            fprintf(stderr, "pipe error(1st pipe): %s\n", strerror(errno));
            restore_and_exit(1);
        }
        if (pipe(to_terminal) < 0)
        {
            fprintf(stderr, "pipe error(2nd pipe): %s\n", strerror(errno));
            restore_and_exit(1);
        }

        if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR)
        {
            fprintf(stderr, "signal error: %s\n", strerror(errno));
            restore_and_exit(1);
        }

        int pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "fork error: %s\n", strerror(errno));
            restore_and_exit(1);
        }
        else if (pid == 0)
        {
            // child process

            // close to_shell[1], the write end of th to_shell pipe
            if (close(to_shell[1]) < 0) {
                fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            // redirect the output from terminal to stdin
            if (close(0) < 0) {
                fprintf(stderr, "close error when closing stdin: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            if (dup(to_shell[0]) < 0) {
                fprintf(stderr, "dup error when dup to_shell[0]: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            if (close(to_shell[0]) < 0) {
                fprintf(stderr, "close error when closing to_shell[0]: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            // close to_terminal[0], read end of the to_terminal pipe 
            if (close(to_terminal[0]) < 0) {
                fprintf(stderr, "close error when closing to_terminal[0]: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            // redirect stdout and stderr to the input to terminal
            if (dup2(to_terminal[1], 1) < 0) {
                fprintf(stderr, "dup2 error when dup2 stdout to to_terminal[1]: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            if (dup2(to_terminal[1], 2) < 0) {
                fprintf(stderr, "dup2 error when dup2 stderr to to_terminal[1]: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            // close to_terminal[1], now it can be replaced by stdout and stderr
            if (close(to_terminal[1]) < 0) {
                fprintf(stderr, "close error when closing to_terminal[1]: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            
            // execute the shell program
            if (execlp("/bin/bash", "bash", NULL) < 0) {
                fprintf(stderr, "execlp error when execlp /bin/bash: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            // if the process still execute the code below, it means
            // execlp did not take up this process, there is some error
            fprintf(stderr, 
                    "should not get here, error when executing the child process: %s\n", 
                    strerror(errno));
            restore_and_exit(1);
        }
        else
        {
            // parent process

            // close to_shell[0], the read end of the to_shell pipe
            if (close(to_shell[0]) < 0) {
                fprintf(stderr, "close error when closing to_shell[0]: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            // close to_terminal[1], the write end of the to_terminal pipe
            if (close(to_terminal[1]) < 0) {
                fprintf(stderr, "close error when closing to_terminal[1]: %s\n", strerror(errno));
                restore_and_exit(1);
            }

            // this flag determines whether to shut down
            bool shut_down_flag = false;

            // initiate the pollfd structure
            struct pollfd pollfds[2];
            pollfds[0].fd = 0;
            pollfds[0].events = (POLLIN + POLLHUP + POLLERR);
            pollfds[1].fd = to_terminal[0];
            pollfds[1].events = (POLLIN + POLLHUP + POLLERR);

            // keep the read-write process until need to shut down
            while ( !shut_down_flag)
            {
                // call poll to avoid the reads to block each other
                if (poll(pollfds, 2, -1) < 0) {
                    fprintf(stderr, "poll error: %s\n", strerror(errno));
                    restore_and_exit(1);
                }

                // getting read from stdin
                if(pollfds[0].revents & POLLIN)
                {
                    char c[read_size];
                    int count = read(0, c, sizeof(c));
                    if (count < 0) {
                        fprintf(stderr, "read error from stdin in --shell option: %s\n", strerror(errno));
                        restore_and_exit(1);
                    }
                    for(int k = 0; k < count; k++)
                    {
                        if (c[k] == 0x04)
                        {
                            // shut down if reciving ^D from stdin
                            if (write(1, "^D", 2) < 0) {
                                fprintf(stderr, "write error when writing ^D: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                            // close the read end of the to_shell pipe if not closed
                            if (! to_shell_close) {
                                if (close(to_shell[1]) < 0) {
                                        fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                                        restore_and_exit(1);
                                    }
                                to_shell_close = true;
                            }
                            shut_down_flag = true;
                        }
                        // send the "right" newline to stdout and shell
                        else if (c[k] == '\r' || c[k] == '\n')
                        {
                            if (write(1, "\r\n", 2) < 0) {
                                fprintf(stderr, "write error when writing <cr><lf> to stdout: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                            if (write(to_shell[1], "\n", 1) < 0) {
                                fprintf(stderr, "write error when writing <lf> to shell: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                        }
                        // send the interrupt signal to shell
                        else if (c[k] == 0x03)
                        {
                            if (write(1, "^C", 2) < 0) {
                                fprintf(stderr, "write error when writing ^C to stdout: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                            if (kill(pid, SIGINT) < 0) {
                                fprintf(stderr, "kill error when sending SIGINT to shell: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                        }
                        else
                        {
                            if (write(1, &c[k], 1) < 0) {
                                fprintf(stderr, "write error when writing to stdout: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                            if (write(to_shell[1], &c[k], 1) < 0) {
                                fprintf(stderr, "write error when writing to shell: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                        }
                        
                    }
                }
                // getting read from shell
                if (pollfds[1].revents & POLLIN)
                {
                    char c[read_size];
                    int count = read(to_terminal[0], c, sizeof(c));
                    if (count < 0) {
                        fprintf(stderr, "read error when reading from shell: %s\n", strerror(errno));
                        restore_and_exit(1);
                    }
                    for (int k = 0; k < count; k++)
                    {
                        // send the "right" newline to stdout
                        if (c[k] == '\n')
                        {
                            if (write(1, "\r\n", 2) < 0) {
                                fprintf(stderr, "write error when writing cr lf to stdout: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                        }
                        // terminate if receiving ^D
                        else if (c[k] == 0x04)
                        {
                            if (write(1, "^D", 2) < 0) {
                                fprintf(stderr, "write error when writing ^D to sdtout: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                            shut_down_flag = true;
                        }
                        else
                        {
                            if (write(1, &c[k], 1) < 0) {
                                fprintf(stderr, "write error when writing to stdout: %s\n", strerror(errno));
                                restore_and_exit(1);
                            }
                        }
                        
                    }
                }
                // handling error 
                if( (pollfds[0].revents &  (POLLHUP | POLLERR) ) || 
                        (pollfds[1].revents & (POLLHUP | POLLERR)) )
                    shut_down_flag = true;
            }
            // close the not closed pipe ends and shut down
            if (! to_shell_close) {
                if (close(to_shell[1]) < 0) {
                    fprintf(stderr, "close error when closing to_shell[1]: %s\n", strerror(errno));
                    restore_and_exit(1);
                }
                to_shell_close = true;
            }
            
            if (close(to_terminal[0]) < 0) {
                fprintf(stderr, "close error when closing to_terminal[0]: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            shut_down();
        }
    }
    else
    {
        // the default option
        while(1)
        {
            char c[read_size];
            int count = read(0, c, sizeof(c));
            if (count < 0) {
                fprintf(stderr, "read error in the default option: %s\n", strerror(errno));
                restore_and_exit(1);
            }
            for(int k = 0; k < count; k++)
            {
                if (c[k] == 0x4)
                {
                    if (write(1, "^D", 2) < 0) {
                        fprintf(stderr, "write error when writing ^D to stdout: %s\n", strerror(errno));
                        restore_and_exit(1);
                    }
                    restore_and_exit(0);
                }
                else if (c[k] == '\r' || c[k] == '\n')
                {
                    if (write(1, "\r\n", 2) < 0) {
                        fprintf(stderr, "write error when writing cr lf to stdout: %s\n", strerror(errno));
                        restore_and_exit(1);
                    }
                }    
                else if (c[k] == 0x03)
                {
                    if (write(1, "^C", 2) < 0) {
                        fprintf(stderr, "write error when writing to stdout: %s\n", strerror(errno));
                        restore_and_exit(1);
                    }
                }   
                else
                {
                    if (write(1, &c[k], 1) < 0) {
                        fprintf(stderr, "write error: %s\n", strerror(errno));
                        restore_and_exit(1);
                    }
                }
                
            }
        }
    }
    


}
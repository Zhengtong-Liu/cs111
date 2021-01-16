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

struct termios tattr;
int pid;
int to_shell[2];
int to_terminal[2];

void terminal_setup ()
{
    if (! isatty(0))
    {
        fprintf(stderr, "File descriptor 0 is not for terminal");
        exit(1);
    }

    if (tcgetattr(0, &tattr) != 0)
    {
        fprintf(stderr, "tcgetattr error\n");
        exit(1);
    }

    struct termios temp;
    tcgetattr(0, &temp);
    temp.c_iflag = ISTRIP; temp.c_oflag = 0; temp.c_lflag = 0;
    if (tcsetattr(0, TCSANOW, &temp) != 0)
    {
        fprintf(stderr, "tcsetattr error\n");
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
            printf("--shell evoked\n");
            break;
        default:
            fprintf(stderr, "%s: Incorrect usage\n", argv[0]);
            fprintf(stderr, "usage: ./lab1a [--shell]\n");
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

    return shell_flag;
}

void shut_down ()
{
    int status = 0;
    waitpid(pid, &status, 0);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", 
                    WTERMSIG(status), WEXITSTATUS(status));
    if (tcsetattr(0, TCSANOW, &tattr) != 0)
    {
        fprintf(stderr, "tcsetattr error\n");
        exit(1);
    }
    exit(0);
}

void sigpipe_handler (int sig)
{
    if (sig == SIGPIPE)
    {
        fprintf(stderr, "SIGPIPE signal has been received.\n");
        close(to_shell[1]);
        close(to_terminal[0]);
        kill(pid, SIGINT);
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
        pipe(to_shell);
        pipe(to_terminal);

        signal(SIGPIPE, sigpipe_handler);

        int pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (pid == 0)
        {
            // child process

            // redirect stdin to the output from terminal
            close(to_shell[1]);
            close(0);
            dup(to_shell[0]);
            close(to_shell[0]);

            close(to_terminal[0]);

            dup2(to_terminal[1], 1);
            dup2(to_terminal[1], 2);
            // write stdout/stderr to to_terminal[1]
            execlp("/bin/bash", "bash", NULL);
        }
        else
        {
            // parent process
            close(to_shell[0]);
            close(to_terminal[1]);

            bool shut_down_flag = false;

            struct pollfd pollfds[2];
            pollfds[0].fd = 0;
            pollfds[0].events = (POLLIN + POLLHUP + POLLERR);
            pollfds[1].fd = to_terminal[0];
            pollfds[1].events = (POLLIN + POLLHUP + POLLERR);

            while ( !shut_down_flag)
            {
                poll(pollfds, 2, 0);
                if(pollfds[0].revents & POLLIN)
                {
                    char c[read_size];
                    int count = read(0, c, sizeof(c));
                    for(int k = 0; k < count; k++)
                    {
                        if (c[k] == 0x04)
                        {
                            write(1, "^D", 3);
                            close(to_shell[1]);
                            shut_down_flag = true;
                        }
                        else if (c[k] == '\r' || c[k] == '\n')
                        {
                            write(1, "\r\n", 3);
                            write(to_shell[1], "\n", 2);
                        }
                        else if (c[k] == 0x03)
                        {
                            write(1, "^C", 3);
                            kill(pid, SIGINT);
                        }
                        else
                        {
                            write(1, &c[k], 1);
                            write(to_shell[1], &c[k], 1);
                        }
                        
                    }
                }
                if (pollfds[1].revents & POLLIN)
                {
                    char c[read_size];
                    int count = read(to_terminal[0], c, sizeof(c));
                    for (int k = 0; k < count; k++)
                    {
                        if (c[k] == '\n')
                            write(1, "\r\n", 3);
                        else
                        {
                            write(1, &c[k], 1);
                        }
                        
                    }
                }
                if (pollfds[0].revents & (POLLHUP | POLLERR))
                    shut_down_flag = true;
            }

            close(to_shell[1]);
            close(to_terminal[0]);
            shut_down();
        }
    }
    else
    {
        while(1)
        {
            char c[read_size];
            int count = read(0, c, sizeof(c));
            for(int k = 0; k < count; k++)
            {
                if (c[k] == 0x4)
                {
                    write(1, "^D", 3);
                    if (tcsetattr(0, TCSANOW, &tattr) != 0)
                    {
                        fprintf(stderr, "tcsetattr error\n");
                        exit(1);
                    }
                    exit(0);
                }
                else if (c[k] == '\r' || c[k] == '\n')
                    write(1, "\r\n", 3);
                else if (c[k] == 0x03)
                    write(1, "^C", 3);
                else
                {
                    write(1, &c[k], 1);
                }
                
            }
        }
    }
    


}
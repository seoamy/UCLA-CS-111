// NAME: Amy Seo
// EMAIL: amyseo@g.ucla.edu
// ID: 505328863

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <poll.h>
#include <sys/wait.h>

#define BUFFERSIZE 256
#define BASH "/bin/bash"
#define CTRLD 0x04
#define CTRLC 0x03
#define LF 0x0A 
#define CR 0x0D

char newline[2] = {CR, LF};
char lineFeed = LF;
struct termios originalMode;
int pipeToShell[2];
int pipeToTerminal[2];
int id, status;

void restoreTerminal() {
    if (tcsetattr(0, TCSANOW, &originalMode)) {
        fprintf(stderr, "%s\n", "ERROR: Failed to reset terminal mode.");
        exit(1);
    }
}

void pipeWithErrorCheck(int pipeFds[2]){
    if (pipe(pipeFds) != 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to create pipe.");
            exit(1);
    }
}

void sigpipeHandler() {
    if (close(pipeToShell[1]) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to close pipe to shell.");
        exit(1);
    }
    if (close(pipeToTerminal[0]) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to close pipe back to terminal.");
        exit(1);
    }
    if (kill(id, SIGINT) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to kill child process.");
        exit(1);
    }
    if (waitpid(id, &status, 0) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to wait for child process to die.");
        exit(1);
    }
    fprintf(stderr, "%s%d%s%d\n", "SHELL EXIT SIGNAL=",WTERMSIG(status), "STATUS=",WEXITSTATUS(status));
    exit(0);
}

int main(int argc, char **argv) {
    int opt;
    bool shell_f = false;

    static struct option opts[] = {// all possible optional arguments
                                   {"shell", 0, NULL, 's'},
                                   {0, 0, 0, 0}};

    // Get argument
    while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
        switch (opt) {
            case 's':
                shell_f = true;
                break;
            default:
                fprintf(stderr, "%s\n", "No such argument. Allowed arguments are --shell");
                exit(1);
                break;
        }
    }

    // Save original terminal mode
    if (tcgetattr(0, &originalMode)) {
        fprintf(stderr, "%s\n", "ERROR: Failed to get terminal attributes.");
        exit(1);
    }

    struct termios tmp;
    if (tcgetattr(0, &tmp)) {
        fprintf(stderr, "%s\n", "ERROR: Failed to get terminal attributes.");
        exit(1);
    }

    atexit(restoreTerminal); // guarantees restoration of terminal upon exit

    tmp.c_iflag = ISTRIP;
    tmp.c_oflag = 0;
    tmp.c_lflag = 0;

    // Set terminal to noncanonical mode
    if (tcsetattr(0, TCSANOW, &tmp)) {
        fprintf(stderr, "%s\n", "ERROR: Failed to set terminal attributes.");
        exit(1);
    }

    // shell argument
    if (shell_f) {
        bool endOfFile = false;
        struct pollfd pollfds[2];

        // set up pipes
        pipeWithErrorCheck(pipeToShell);
        pipeWithErrorCheck(pipeToTerminal);
        signal(SIGPIPE, sigpipeHandler);

        pollfds[0].fd = 0;
        pollfds[0].events = POLLIN + POLLHUP + POLLERR;
        pollfds[1].fd = pipeToTerminal[0];
        pollfds[1].events = POLLIN + POLLHUP + POLLERR;

        id = fork();
        if (id < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to fork.");
            exit(1);
        }
        else if (id == 0) { // child process
            // redirect input of shell to be stdin (terminal)
            if (close(pipeToShell[1]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close writing end of parent pipe.");
                exit(1);
            }
            if (close(0) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close stdin.");
                exit(1);
            }
            if (dup(pipeToShell[0]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to duplicate read end of child pipe.");
                exit(1);
            } 

            // redirect output of parent to stdout
            if (close(pipeToTerminal[0]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close read end of parent pipe.");
                exit(1);
            }
            if (close(1) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close stdout.");
                exit(1);
            }
            if (dup(pipeToTerminal[1]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to duplicate write end of parent pipe.");
                exit(1);
            }
            
            // redirect output of parent to stderr
            if (dup2(pipeToTerminal[1], STDERR_FILENO) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to redirect output of.");
                exit(1);
            }

            // close unused ends of the pipes
            if (close(pipeToShell[0]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close read end of child pipe.");
                exit(1);
            }
            if (close(pipeToTerminal[1]) <0 ) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close write end of parent pipe.");
                exit(1);
            }

            // create bash shell
            if (execlp(BASH, BASH, NULL) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to create shell from exec.");
                exit(1);
            }
        }
        else { // parent process
            // close shell read and terminal write to process current input
            if (close(pipeToShell[0]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close read end of child process");
                exit(1);
            }
            if (close(pipeToTerminal[1]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close write end of parent process.");
                exit(1);
            }
            char c;
            while(1) {
                if (poll(pollfds, 2, -1) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to poll.");
                    exit(1);
                }
                if (pollfds[0].revents & POLLIN) { // input detected in stdin
                    char buff[BUFFERSIZE];
                    int count = read(0, &buff, sizeof(buff));
                    if (count < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to read.");
                        exit(1);
                    }
                    int j;
                    for(j = 0; j < count; j++) {
                        if (buff[j] == CTRLD) {
                            endOfFile = true;
                        }
                        else if (buff[j] == CTRLC) {
                            if (kill(id, SIGINT) < 0) {
                                fprintf(stderr, "%s\n", "ERROR: Failed to kill child process.");
                                exit(1);        
                            }
                        }
                        else if (buff[j] == CR || buff[j] == LF) {
                            if (write(1, &newline, sizeof(char)*2) < 0) { // echo newline to stdout
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                                exit(1);
                            }
                            if (write(pipeToShell[1], &lineFeed, sizeof(char))< 0) { // forward LF to shell
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                                exit(1);
                            }
                        }
                        else {
                            c = buff[j];
                            if (write(1, &c, sizeof(char)) < 0) { // echo to stdout
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                                exit(1);
                            }
                            if (write(pipeToShell[1], &c, sizeof(char))< 0) { // forward to shell
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                                exit(1);
                            }
                        }
                    }
                }
                if (pollfds[1].revents & POLLIN) { // output detected in terminal from shell
                    char buff[BUFFERSIZE];
                    int count = read(pipeToTerminal[0], &buff, sizeof(buff));
                    if (count < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to read.");
                        exit(1);
                    }
                    int k;
                    for(k = 0; k < count; k++) {
                        if (buff[k] == CTRLD) {
                            endOfFile = true;
                        }
                        else if (buff[k] == LF) {
                            if (write(1, &newline, sizeof(char)*2) < 0) {
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                                exit(1);
                            }
                        }
                        else {
                            c = buff[k];
                            if (write(1, &c, sizeof(char))< 0) {
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                                exit(1);
                            }
                        }
                    }
                }
                if (pollfds[0].revents & (POLLHUP | POLLERR)) { // errors in polling from stdin
                    fprintf(stderr, "%s\n", "ERROR in taking stdin.");
                    exit(1);
                }
                if (endOfFile || (pollfds[1].revents & (POLLHUP | POLLERR))) {
                    break;
                }
            }
            if (close(pipeToTerminal[0]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close pipe back to terminal.");
                exit(1);
            }
            if (close(pipeToShell[1]) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to close pipe to shell.");
                exit(1);
            }
            if (waitpid(id, &status, 0) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to wait for child process.");
                exit(1);
            }
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status),WEXITSTATUS(status));
            exit(0);
        }
    }
    else {
        // read input from terminal without shell
        char buff[BUFFERSIZE];
        while (1)
        {
            int i;
            char c;
            int count = read(0, &buff, sizeof(buff));
            if (count < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to read from terminal.");
                exit(1);
            }
            for(i = 0; i < count; i++) {
                c = buff[i];
                if (c == CTRLD){
                    exit(0);
                }       
                else if (c == CR || c == LF) {
                    if (write(1, &newline, sizeof(char) * 2) < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                        exit(1);
                    }
                }
                else {
                    if (write(1, &c, sizeof(char)) < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                        exit(1);
                    }
                }
            }
        }
        if (write(1, &newline, sizeof(char)*2) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
            exit(1);
        }
    }
    exit(0);
}
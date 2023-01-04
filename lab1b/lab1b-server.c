// NAME: Amy Seo
// EMAIL: amyseo@g.ucla.edu
// ID: 505328863

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <getopt.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <poll.h>
#include <zlib.h>

#define BUFFERSIZE 256
#define BASH "/bin/bash"
#define MAXCONNECTIONS 5
#define CTRLD 0x04
#define CTRLC 0x03
#define LF 0x0A 
#define CR 0x0D

char newline[2] = {CR, LF};
char lineFeed = LF;
struct termios originalMode;
int pipeToShell[2];
int pipeFromShell[2];
int id, status;

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
    if (close(pipeFromShell[0]) < 0) {
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

int server_connect(unsigned int port) {
    int this_sockfd, new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;

    this_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this_sockfd < 0) {
        fprintf(stderr, "%s", "ERROR: Failed to create socket.");
        exit(0);
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));
    if (bind(this_sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "%s", "ERROR: Failed to bind socket.");
        exit(0);
    }

    if (listen(this_sockfd, MAXCONNECTIONS) < 0) {
        fprintf(stderr, "%s", "ERROR: Failed to listen on socket.");
        exit(0);
    }
    sin_size = sizeof(struct sockaddr_in);

    new_fd = accept(this_sockfd, (struct sockaddr*) &their_addr, &sin_size);
    if (new_fd < 0) {
        fprintf(stderr, "%s", "ERROR: Failed to accept socket.");
        exit(0);
    }
    if (close(this_sockfd) < 0) {
        fprintf(stderr, "%s", "ERROR: Failed to close socket.");
        exit(0);
    }
    return new_fd; 
}

int main(int argc, char **argv) {
    int opt, port, socketfd;
    bool compress = false;
    z_stream compress_strm;
    z_stream decompress_strm;

    static struct option opts[] = {// all possible optional arguments
                                   {"port", required_argument, NULL, 'p'},
                                   {"compress", 0, NULL, 'c'},
                                   {0, 0, 0, 0}};

    // Get argument
    while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                compress = true;
                compress_strm.zalloc=Z_NULL;
                compress_strm.zfree=Z_NULL;
                compress_strm.opaque=Z_NULL;
                decompress_strm.zalloc=Z_NULL;
                decompress_strm.zfree=Z_NULL;
                decompress_strm.zfree=Z_NULL;
                if (deflateInit(&compress_strm, Z_DEFAULT_COMPRESSION) != Z_OK){
                    fprintf(stderr, "%s\n", "ERROR: Failed to initialize compression stream.");
                    exit(1);
                }
                if (inflateInit(&decompress_strm) != Z_OK){
                    exit(1);
                    fprintf(stderr, "%s\n", "ERROR: Failed to initialize decompression stream.");
                }
                break;
            default:
                fprintf(stderr, "%s\n", "No such argument. Allowed arguments are --port=portNumber, --log=filename, --compress");
                exit(1);
                break;
        }
    }

    // set up socket and get its fd
    socketfd = server_connect(port);

    bool endOfFile = false;
    struct pollfd pollfds[2];

    // set up pipes
    pipeWithErrorCheck(pipeToShell);
    pipeWithErrorCheck(pipeFromShell);
    signal(SIGPIPE, sigpipeHandler);

    pollfds[0].fd = socketfd;
    pollfds[0].events = POLLIN + POLLHUP + POLLERR;
    pollfds[1].fd = pipeFromShell[0];
    pollfds[1].events = POLLIN + POLLHUP + POLLERR;

    id = fork();
    if (id < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to fork.");
        exit(1);
    }
    else if (id == 0) { // child process
        // redirect input of shell to be socket
        if (close(pipeToShell[1]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close writing end of parent pipe.");
            exit(1);
        }
        if (close(0) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close stdin.");
            exit(1);
        }
        if (dup(pipeToShell[0]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to duplicate socketfd into stdin.");
            exit(1);
        } 

        // redirect output of parent to socket
        if (close(pipeFromShell[0]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close read end of parent pipe.");
            exit(1);
        }
        if (close(1) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close stdout.");
            exit(1);
        }
        if (dup(pipeFromShell[1]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to duplicate write end of pipe from shell into stdout.");
            exit(1);
        }
        
        // redirect output of parent to stderr
        if (dup2(pipeFromShell[1], STDERR_FILENO) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to redirect output of shell to stderr.");
            exit(1);
        }

        // close unused ends of the pipes
        if (close(pipeToShell[0]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close write end of parent pipe.");
            exit(1);
        }
        if (close(pipeFromShell[1]) <0 ) {
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
        // close shell read and write to process current input
        if (close(pipeToShell[0]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close read end of child process");
            exit(1);
        }
        if (close(pipeFromShell[1]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close write end of parent process.");
            exit(1);
        }
        char c;
        while(1) {
            if (poll(pollfds, 2, -1) < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to poll.");
                exit(1);
            }
            if (pollfds[1].revents & POLLIN) { // output detected from shell
                char buff[BUFFERSIZE];
                int count = read(pipeFromShell[0], &buff, sizeof(buff));
                if (count < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to read.");
                    exit(1);
                }
                if (compress) { // compress output from shell
                    unsigned char comp_buff[BUFFERSIZE];
                    compress_strm.avail_in = count;
                    compress_strm.next_in = (unsigned char*) buff;
                    compress_strm.avail_out = BUFFERSIZE;
                    compress_strm.next_out = comp_buff;
                    while (compress_strm.avail_in > 0) {
                        if (deflate(&compress_strm, Z_SYNC_FLUSH) < 0) {
                            fprintf(stderr, "%s\n", "ERROR: Failed to compress.");
                            exit(1);
                        }
                    }
                    int compressed_bytes = BUFFERSIZE - compress_strm.avail_out;
                    if (write(socketfd, &comp_buff, compressed_bytes) < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to socket.");
                        exit(1);
                    }
                }
                else { // no compression
                    int k;
                    for(k = 0; k < count; k++) {
                        c = buff[k];
                        if (c == CTRLD) {
                            endOfFile = true;
                        }
                        else {
                            if (write(socketfd, &c, sizeof(char))< 0) {
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                                exit(1);
                            }
                        }
                    }
                }
            }
            if (pollfds[0].revents & POLLIN) { // socket ready to read
                char buff[BUFFERSIZE];
                int count = read(socketfd, &buff, sizeof(buff));
                if (count < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to read.");
                    exit(1);
                }
                if (compress) { // decompress input from socket
                    unsigned char decomp_buff[BUFFERSIZE];
                    decompress_strm.avail_in = count;
                    decompress_strm.next_in = (unsigned char*) buff;
                    decompress_strm.avail_out = BUFFERSIZE;
                    decompress_strm.next_out = decomp_buff;
                    while (decompress_strm.avail_in > 0) {
                        if (inflate(&decompress_strm, Z_SYNC_FLUSH) < 0) {
                            fprintf(stderr, "%s\n", "ERROR: Failed to decompress.");
                            exit(1);
                        }
                    }
                    int decompressed_bytes = BUFFERSIZE - decompress_strm.avail_out;
                    char c;
                    for (int j = 0; j < decompressed_bytes; j++) {
                        c = decomp_buff[j];
                        if (c == CTRLD) {
                            endOfFile = true;
                        }
                        else if (c == CTRLC) {
                            if (kill(id, SIGINT) < 0) {
                                fprintf(stderr, "%s\n", "ERROR: Failed to kill child process.");
                                exit(1);        
                            }
                        }
                        else if (c == CR || c == LF) {
                            if (write(pipeToShell[1], &lineFeed, sizeof(char))< 0) { // forward LF to shell
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                                exit(1);
                            }
                        }
                        else {
                            if (write(pipeToShell[1], &c, sizeof(char))< 0) { // forward to shell
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                                exit(1);
                            }
                        }
                    }
                }    
                else { // NO decompressing
                    char c; 
                    for(int j = 0; j < count; j++) {
                        c = buff[j];
                        if (c == CTRLD) {
                            endOfFile = true;
                        }
                        else if (c == CTRLC) {
                            if (kill(id, SIGINT) < 0) {
                                fprintf(stderr, "%s\n", "ERROR: Failed to kill child process.");
                                exit(1);        
                            }
                        }
                        else if (c == CR || c == LF) {
                            if (write(pipeToShell[1], &lineFeed, sizeof(char))< 0) { // forward LF to shell
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                                exit(1);
                            }
                        }
                        else {
                            if (write(pipeToShell[1], &c, sizeof(char))< 0) { // forward to shell
                                fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                                exit(1);
                            }
                        }
                    }
                }
            }
            if (endOfFile || (pollfds[1].revents & (POLLHUP | POLLERR))) {
                break;
            }
            if (pollfds[0].revents & (POLLHUP | POLLERR)) { // errors in polling from socket
                fprintf(stderr, "%s\n", "ERROR in polling the socket.");
                exit(1);
            }
        }
        if (close(pipeToShell[1]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close pipe to shell.");
            exit(1);
        }
        if (close(pipeFromShell[0]) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close pipe back to terminal.");
            exit(1);
        }
        if (close(socketfd) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to close socket.");
            exit(1);
        }
        if (waitpid(id, &status, 0) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to wait for child process.");
            exit(1);
        }
        if (compress) {
            if (deflateEnd(&compress_strm) == Z_STREAM_ERROR) {
                fprintf(stderr, "%s\n", "ERROR: Failed to end deflate for compression stream.");
                exit(1);
            }
            if (inflateEnd(&decompress_strm) == Z_STREAM_ERROR) {
                fprintf(stderr, "%s\n", "ERROR: Failed to end inflate for decompression stream.");
                exit(1);
            }
        }
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status),WEXITSTATUS(status));
        exit(0);
    }
    exit(0);
}
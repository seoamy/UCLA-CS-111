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
#include <netdb.h>
#include <zlib.h>

#define BUFFERSIZE 256
#define BASH "/bin/bash"
#define CTRLD 0x04
#define CTRLC 0x03
#define LF 0x0A 
#define CR 0x0D

char newline[2] = {CR, LF};
char lineFeed = LF;
struct termios originalMode;

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

int client_connect(char* hostname, unsigned int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "%s", "ERROR: Failed to create socket.");
        exit(0);
    }
    // fill in socket address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    server = gethostbyname(hostname);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    memset(serv_addr.sin_zero, '\0', sizeof(serv_addr.sin_zero));

    // connect socket
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to connect client socket.");
        exit(1);
    }
    return sockfd;
}

void writeHeaderToLogFile(char sr, int logfd, int count) {
    if (sr == 'r') {
        if (write(logfd, "RECEIVED ", strlen("RECEIVED ")) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
            exit(1);
        }
    }
    else if (sr == 's') {
        if (write(logfd, "SENT ", strlen("SENT ")) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
            exit(1);
        }
    }
    if (dprintf(logfd, "%d", count) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
        exit(1);
    }
    if (write(logfd, " bytes: ", strlen(" bytes: ")) < 0) {
        fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
        exit(1);
    }
}

int main(int argc, char **argv) {
    int opt, port, socketfd, logfd;
    char *logfile;
    bool compress = false;
    bool log = false;
    z_stream compress_strm;
    z_stream decompress_strm;

    static struct option opts[] = {// all possible optional arguments
                                   {"port", required_argument, NULL, 'p'},
                                   {"log", required_argument, NULL, 'l'},
                                   {"compress", 0, NULL, 'c'},
                                   {0, 0, 0, 0}};
    // Get argument
    while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'l':
                log = true;
                logfile = optarg;
                break;
            case 'c':
                compress = true;
                break;
            default:
                fprintf(stderr, "%s\n", "No such argument. Allowed arguments are --port=portNumber, --log=filename, --compress");
                exit(1);
                break;
        }
    }
    // connect client to socket & get the fd for the socket
    socketfd = client_connect("localhost", port);

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
    
    // setup poling from stdin and socket (server)
    struct pollfd pollfds[2];
    pollfds[0].fd = 0;
    pollfds[0].events = POLLIN + POLLHUP + POLLERR;
    pollfds[1].fd = socketfd;
    pollfds[1].events = POLLIN + POLLHUP + POLLERR;

    // open or create log file
    if (log) {
        logfd = open(logfile, O_RDWR | O_CREAT | O_TRUNC,0777);
        if (logfd < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to write open or create log file.");
            exit(1);
        }
    }

    // set up compression streams
    if (compress) {
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
    }

    while(1) {
        if (poll(pollfds, 2, -1) < 0) {
            fprintf(stderr, "%s\n", "ERROR: Failed to poll.");
            exit(1);
        }
        if (pollfds[1].revents & POLLIN) { // socket ready to read
            char buff[BUFFERSIZE];
            int count = read(socketfd, &buff, sizeof(buff));
            if (count == 0) break;
            if (count < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to read from socket.");
                exit(1);
            }
            if (log) { // log (compressed) output from socket
                writeHeaderToLogFile('r', logfd, count);
                if (write(logfd, &buff, count) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
                    exit(1);
                }
                if (write(logfd, &newline, sizeof(newline)) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
                    exit(1);
                }
            }
            if (compress) { // decompress socket
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
                for(int k = 0; k < decompressed_bytes; k++) {
                    if (decomp_buff[k] == LF || decomp_buff[k] == CR) {
                        if (write(1, &newline, sizeof(char)*2) < 0) {
                            fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                            exit(1);
                        }
                    }
                    else {
                        c = decomp_buff[k];
                        if (write(1, &c, sizeof(char))< 0) {
                            fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                            exit(1);
                        }
                    }
                }
            }
            else { // no decompression
                int k;
                char c;
                for(k = 0; k < count; k++) {
                    if (buff[k] == LF || buff[k] == CR) {
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
        }
        if (pollfds[0].revents & POLLIN) { // input detected in stdin
            char buff[BUFFERSIZE];
            int count = read(0, &buff, sizeof(buff));
            if (count < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to read.");
                exit(1);
            }
            int j;
            char c;
            if (compress) {
                // compress
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
                if (log) {
                    writeHeaderToLogFile('s', logfd, compressed_bytes);
                    if (write(logfd, &comp_buff, compressed_bytes) < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
                        exit(1);
                    }
                    if (write(logfd, &newline, sizeof(newline)) < 0) {
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to log file.");
                        exit(1);
                    }
                }
            }
            for(j = 0; j < count; j++) {
                c = buff[j];
                if (c == CR || c == LF) {
                    if (write(1, &newline, sizeof(char)*2) < 0) { // echo newline to stdout
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                        exit(1);
                    }
                    if (!compress) {
                        if (write(socketfd, &c, sizeof(char))< 0) { // forward LF to server
                            fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                            exit(1);
                        }
                    }
                }
                else { // treat CTRLD and CTRLC as regular characters
                    if (write(1, &c, sizeof(char)) < 0) { // echo to stdout
                        fprintf(stderr, "%s\n", "ERROR: Failed to write to terminal.");
                        exit(1);
                    }
                    if (!compress) {
                        if (write(socketfd, &c, sizeof(char))< 0) { // forward to server
                            fprintf(stderr, "%s\n", "ERROR: Failed to write to shell.");
                            exit(1);
                        }
                    }
                }
            }
            if (!compress && log) { // log sent
                writeHeaderToLogFile('s', logfd, count);
                if (write(logfd, &buff, count) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to write to logfile.");
                    exit(1);
                }
                if (write(logfd, &newline, sizeof(newline)) < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to write to logfile.");
                    exit(1);
                }
            }
        }
        if (pollfds[0].revents & (POLLHUP | POLLERR)) { // errors in polling from stdin
            fprintf(stderr, "%s\n", "ERROR in polling for stdin.");
            exit(1);
        }
        if (pollfds[1].revents & (POLLHUP | POLLERR)) { // closed communication with socket
            break;
        }
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
    exit(0);
}
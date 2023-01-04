/* 
Name: Amy Seo
Email: amyseo@g.ucla.edu
ID: 505328863
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFERSIZE 1024

void makeSegFault()
{
    char *p = NULL;
    *p = 'a';
}

void segFaultHandler(int sig)
{
    if (sig == SIGSEGV)
    {
        fprintf(stderr, "ERROR: Segmentation fault.\n");
        exit(4);
    }
}

int main(int argc, char **argv)
{
    char buff[BUFFERSIZE];
    char *input = NULL;
    char *output = NULL;
    bool seg_f = false;
    bool catch_f = false;
    int i; // current optional argument
    int ret;
    int ifd = 0; // default from standard input
    int ofd = 1; // default from standard output

    static struct option opts[] = {// all possible optional arguments
                                   {"input", 1, NULL, 'i'},
                                   {"output", 1, NULL, 'o'},
                                   {"segfault", 0, NULL, 's'},
                                   {"catch", 0, NULL, 'c'},
                                   {0, 0, 0, 0}};

    while ((i = getopt_long(argc, argv, "", opts, NULL)) != -1)
    {
        switch (i)
        {
        case 'i':
            input = optarg;
            ifd = open(input, O_RDONLY);
            break;
        case 'o':
            output = optarg;
            ofd = creat(output, 0666); //0666 gives read and write permissions to user, group, and others
            break;
        case 's':
            seg_f = true;
            break;
        case 'c':
            catch_f = true;
            break;
        default:
            fprintf(stderr, "%s\n", "No such argument. Allowed arguments are --input=filename, --output=filename, --segfault, and --catch");
            exit(1);
            break;
        }
    }

    // Catch segfault
    if (catch_f)
    {
        signal(SIGSEGV, segFaultHandler);
    }

    // Create segfault with no catch
    if (seg_f)
    {
        makeSegFault();
    }

    // set input file to fd0
    if (input != NULL)
    {
        if (ifd >= 0)
        {
            close(0);
            dup(ifd);
            close(ifd);
        }
        else
        {
            fprintf(stderr, "%s: %s\n", input, strerror(errno));
            exit(2);
        }
    }

    // set output file to fd1
    if (output != NULL)
    {
        if (ofd >= 0)
        {
            close(1);
            dup(ofd);
            close(ofd);
        }
        else
        {
            fprintf(stderr, "%s: %s\n", output, strerror(errno));
            exit(3);
        }
    }

    while ((ret = read(0, &buff, BUFFERSIZE)) > 0)
    {
        // read from file descriptor 0
        // write to file descriptor 1
        if (write(1, &buff, ret) != ret)
        {
            fprintf(stderr, "%s: %s\n", output, strerror(errno));
            exit(3);
        }
    }

    close(0); // closes input file
    close(1); // closes output file
    exit(0);  // exit sucessfully
}

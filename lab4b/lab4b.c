// NAME: Amy Seo
// EMAIL: amyseo@g.ucla.edu
// ID: 505328863

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <poll.h>
#include <getopt.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define B 4275
#define R0 100000.0 

#ifdef DUMMY
#define MRAA_GPIO_IN 0
typedef int mraa_aio_context;
typedef int mraa_gpio_context;

// define dummy functions
mraa_aio_context mraa_aio_init(int p){
    p  = p + 1;
    return 0;
}

int mraa_aio_read(mraa_aio_context c) {
    c = c + 1;
    return 650;
}

void mraa_aio_close(mraa_aio_context c) {
    c = c + 1;
}

mraa_gpio_context mraa_gpio_init(int p){
    p = p+1;
    return 0;
}

void mraa_gpio_dir(mraa_gpio_context c, int d) {
    c = c + d;
}

int mraa_gpio_read(mraa_gpio_context c) {
    c = c + 1;
    return 0;
}

void mraa_gpio_close(mraa_gpio_context c) {
    c = c + 1;
}

#else
#include <mraa.h>
#include <mraa/aio.h>
#endif

struct tm cur_time;
time_t now;
int prev_time = 0;
int period = 1;
char scale = 'F';
int log_fd;
bool has_log = false;
bool stop = false;
bool on = true;
int exit_code = 0;
mraa_aio_context temp_sensor;
mraa_gpio_context button;

float convert_temper_reading(int reading) {
    float R = 1023.0/((float)reading)-1.0;
    R = R0 * R;
    float C = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
    float F = (C * 9)/5 + 32;
    if (scale == 'C') {
        return C;
    }
    else if (scale == 'F'){
        return F;
    }
    else {
        fprintf(stderr, "%s\n", "ERROR: Invalid temperature scale. Only accepts C or F.");
        exit(0);
    }
}

struct tm get_current_time(void) {
    now = time(NULL);
    struct tm* tm = localtime(&now);
    return *tm;
}

void shutdown() {
    cur_time = get_current_time();
    char buff[256];
    sprintf(buff, "%.2d:%.2d:%.2d SHUTDOWN\n", cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec);
    dprintf(1, buff, sizeof(buff));
    if (has_log){
        dprintf(log_fd, buff, sizeof(buff));
    }
    mraa_aio_close(temp_sensor);
    mraa_gpio_close(button);
    exit(exit_code);
}

void parse_stdin (char* stdin_input) {
    if (strcmp(stdin_input, "SCALE=F") == 0) {
        scale = 'F';
    }
    else if (strcmp(stdin_input, "SCALE=C") == 0) {
        scale = 'C';
    }
    else if (strncmp(stdin_input, "PERIOD=", strlen("PERIOD=")) == 0) {
        period = atoi(stdin_input+strlen("PERIOD="));
    }
    else if (strcmp(stdin_input, "STOP") == 0) {
        stop = true;
    }
    else if (strcmp(stdin_input, "START") == 0) {
        stop = false;
    }
    else if (strcmp(stdin_input, "OFF") == 0) {
        if(has_log){
            dprintf(log_fd, "%s\n", stdin_input);
        }
        shutdown();
    }
    else if (strncmp(stdin_input, "LOG", strlen("LOG")) == 0){
    }
    else {
        exit_code = 1;
    }

    if(has_log){
        dprintf(log_fd, "%s\n", stdin_input);
    }
}

int main(int argc, char **argv){
    int opt;
    static struct option opts[] = {// all possible optional arguments
                                   {"period", required_argument, NULL, 'p'},
                                   {"scale", required_argument, NULL, 's'},
                                   {"log", required_argument, NULL, 'l'},
                                   {0, 0, 0, 0}};
     // Get argument
    while ((opt = getopt_long(argc, argv, "", opts, NULL)) != -1) {
        switch (opt) {
            case 'p':
                period = atoi(optarg);
                break;
            case 's':
                if (optarg[0] != 'C' && optarg[0] != 'F') {
                    fprintf(stderr,  "%s\n", "ERROR: Invalid temperature scale. Only accepts C or F.");
                    exit(1);
                }
                else {
                    scale = optarg[0];
                }
                break;
            case 'l':
                log_fd = open(optarg, O_RDWR | O_CREAT | O_TRUNC,0777);
                if (log_fd < 0) {
                    fprintf(stderr, "%s\n", "ERROR: Failed to write open or create log file.");
                    exit(1);
                }
                has_log = true;
                break;
            default:
                fprintf(stderr, "%s\n", "ERROR: No such argument. Allowed arguments are --period=N, --scale=C/F, --log=logfile");
                exit(1);
                break;
        }
    }
    // initialize sensors
    button = mraa_gpio_init(60);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    temp_sensor = mraa_aio_init(1);
    struct pollfd pollStdin = {0, POLLIN | POLLHUP | POLLERR, 0};

    while(1) { // while button is not pressed
        // report temperature if period
        cur_time = get_current_time();
        if (!stop && ((now - prev_time) >= period)) {
            prev_time = now;
            char buff[256];
            int cur_temp_reading = mraa_aio_read(temp_sensor);
            float cur_temp = convert_temper_reading(cur_temp_reading);
            sprintf(buff, "%.2d:%.2d:%.2d %.1f\n", cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec, cur_temp);
            dprintf(1, buff, sizeof(buff));
            if (has_log) {
                dprintf(log_fd, buff, sizeof(buff));
            }
        }

        // poll input from stdin
        int ret = poll(&pollStdin, 1, 1000);
        if (ret >= 1) {
            char input_buff[256];
            char read_in_buff[256];
            memset(input_buff, 0, 256);
            memset(read_in_buff, 0, 256);
            int buff_index = 0;
            int bytes_read = read(0, input_buff, sizeof(input_buff));
            if (bytes_read < 0) {
                fprintf(stderr, "%s\n", "ERROR: Failed to read from socket.");
                exit(1);
            }
            int i;
            for (i = 0; i < bytes_read;i++) {
                if (input_buff[i] == '\n') {
                    read_in_buff[buff_index] = 0;
                    parse_stdin(read_in_buff);
                    buff_index = 0;
                    memset(read_in_buff, 0, 256);
                }
                else if ((buff_index == 0 && input_buff[i] == ' ') || (buff_index == 0 && input_buff[i] == 0x09)) {
                    continue;
                }
                else {
                    read_in_buff[buff_index] = input_buff[i];
                    buff_index++;
                }
            }
        }
        else if (ret < 0) {
            fprintf(stderr, "ERROR: Poll failed.\n");
            exit(1);
        }

        // check for button press
        int read = mraa_gpio_read(button);
        if (read == 1) {
            shutdown();
        }
    }
    shutdown();
}










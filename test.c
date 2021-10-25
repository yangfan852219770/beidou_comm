#include "minmea.h"
#include "beidou_read.h"

#define SERIALPATH "/dev/ttyUSB0" 
#define BAUD_RATE 230400
#define DATA_BIT 8
#define PARITY_BIT 'N'
#define STOP_BIT 1

int main(int argc, char const *argv[])
{
    unsigned int fd;
    struct termios oldtio, newtio;

    unsigned char buf[READ_MAX_LENGTH];
    int len;
    // 等待1s
    int time = 1;
    int count = 0;

    // 测试解析的运行时间
    uint64_t diff;
    struct timespec start, end;

    // Open the port
    bool res = open_usb_port( SERIALPATH, &fd, O_RDONLY | O_NOCTTY);
    if( !res )
        return -1;
    // Set config
    res = false;
    res = set_parameter_port( &newtio, &oldtio, fd, BAUD_RATE, DATA_BIT, PARITY_BIT, STOP_BIT );
    if( !res )
        return -1;

    while(1){
        // measure monotonic time
	    clock_gettime(CLOCK_MONOTONIC, &start);
        
        memset(buf, 0, MINMEA_MAX_LENGTH);
        // Read data
        read_data( fd, buf, &len, time);

        //buf[len] = '\0';
        printf("No.%d, res = %d, Raw NMEA: %s\n", ++count, len, buf);
        printf("Hex = ");
        for(int i = 0; i < len; ++i)
            printf("%x",(int)(buf[i]));
        putchar('\n');

        struct minmea_sentence_zda frame;
        /*
        if (minmea_parse_zda(&frame, buf))
            printf("$xxZDA: %d:%d:%d:%d %02d.%02d.%d UTC%+03d:%02d\n",
                    frame.time.hours,
                    frame.time.minutes,
                    frame.time.seconds,
                    frame.time.microseconds,
                    frame.date.day,
                    frame.date.month,
                    frame.date.year,
                    frame.hour_offset,
                    frame.minute_offset);
        else
            printf("$xxZDA sentence is not parsed\n");
        */

        // mark the end time 
        clock_gettime(CLOCK_MONOTONIC, &end);	
        diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
    }
    // Restore the old config
    tcsetattr( fd, TCSANOW, &oldtio );
    // Close the port
    close( fd );
    return 0;
}

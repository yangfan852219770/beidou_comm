#include "minmea.h"
#include "beidou_read.h"

#define SERIALPATH "/dev/ttyUSB0" 
#define BAUD_RATE 115200
#define DATA_BIT 8
#define PARITY_BIT 'N'
#define STOP_BIT 1

int main(int argc, char const *argv[])
{
    uint16_t fd;
    struct termios oldtio, newtio;

    uint8_t buf[READ_MAX_LENGTH];
    int len;
    // 等待1s
    int time = 1;
    int count = 0;

    // 测试解析的运行时间
    uint64_t diff;
    struct timespec start, end;

    nmea_time nmea_t;
    nmea_date nmea_d;

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
        printf("No.%d, res = %d, Raw NMEA: %s", ++count, len, buf);

        // 处理时分秒
        res = nmea_parse_zda_time(&nmea_t, buf);
        if(res){
            printf("ZDA time: %d:%d:%d:%d\n", 
                    nmea_t.hours, 
                    nmea_t.minutes,
                    nmea_t.seconds,
                    nmea_t.microseconds);
        }else
            printf("ZDA time sentence is not parsed\n");
        // mark the end time 
        clock_gettime(CLOCK_MONOTONIC, &end);	
        diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);

        // 处理日月年
        res = nmea_parse_zda_date(&nmea_d, buf);
        if(res){
            printf("ZDA date: %d.%02d.%02d\n",
                    nmea_d.year,
                    nmea_d.month,
                    nmea_d.day);
        }else
            printf("ZDA date sentence is not parsed\n");
        putchar('\n');
    }
    // Restore the old config
    tcsetattr( fd, TCSANOW, &oldtio );
    // Close the port
    close( fd );
    return 0;
}
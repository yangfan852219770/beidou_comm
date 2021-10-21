/**
 * Copyright © Yangfan <yangyangfan@stud.tjut.edu.cn>
 * This program is free software.
 */ 
#include "beidou_read.h"
#include "minmea.h"

#define BILLION 1000000000L

bool open_usb_port( const char *port_path, int *fd, int flags ){
    *fd = open( port_path, flags );
    if( *fd == -1 ){
        perror( "Open serial port error" );
        return false;
    }
    printf( "Open the port:%s success, the file descriptor is: %d.\n", port_path, *fd );
    return true;
}

bool set_parameter_port( struct termios *newtio, struct termios *oldtio, int fd, int baud_rate, int data_bit, char parity_flag, int stop_bit ){
    int b_baud_rate = -1;
    // Save the config of board to restore when close the port
    tcgetattr( fd, oldtio );
    // Initialize the new struct
    bzero(newtio, sizeof( *newtio ));

    /**
     * CLOCAL: Local line
     * CREAD: Enable receiver
     */ 
    newtio->c_cflag |= ( CLOCAL | CREAD );

    // Set data bit
    newtio->c_cflag &= ~CSIZE;
    switch (data_bit)
    {
    case 5:
        newtio->c_cflag |= CS5;
        break;
    case 6:
        newtio->c_cflag |= CS6;
        break;
    case 7:
        newtio->c_cflag |= CS7;
        break;
    case 8:
        newtio->c_cflag |= CS8;
        break;
    default:
        perror( "No such data bit" );
        return false;
    }
    // End set data bit

    // Set parity bit
    switch ( parity_flag ) 
    {
    // Odd parity
    case 'O':
        // Enable parity bit
        newtio->c_cflag |=PARENB;
        /**
         * INPCK: Enable input parity
         * ISTRIP: Ignore parity errors and pass the data, if you have a strict rule with frame
         * delete this option.
         */
        newtio->c_iflag |= ( INPCK | ISTRIP );
        // Enable odd parity
        newtio->c_cflag |= PARODD;
        break;
    // Even parity
    case 'E':
        newtio->c_cflag |=PARENB;
        newtio->c_iflag |= ( INPCK | ISTRIP );
        // Enable even parity
        newtio->c_cflag &= ~PARODD;
        break;
    // None parity
    case 'N':
        newtio->c_cflag &= ~PARENB;
        break;
    default:
        perror("No such parity");
        return false;
    }
    // End set parity bit

    // Set stop bit
    switch (stop_bit)
    {
    case 1:
        newtio->c_cflag &= ~CSTOPB; 
        break;
    case 2:
        newtio->c_cflag |= CSTOPB; 
        break;
    default:
        perror("No such stop_bit");
        return false;
    }
    // End set stop bit

    // Set baud rate, other baud rate also can be set, such as 9600, 4800
    switch ( baud_rate )
    {
    case 9600:
        b_baud_rate = B9600;
        break;
    case 115200:
        b_baud_rate = B115200;
        break;
    case 230400:
        b_baud_rate = B230400;
        break;
    case 460800:
        b_baud_rate = B460800;
        break;
    case 921600:
        b_baud_rate = B921600;
        break;
    default:
        perror( "No such baud rate" );
        return false;
    }
    // Input baud rate
    cfsetispeed( newtio, b_baud_rate );
    // Output baud rate
    cfsetospeed( newtio, b_baud_rate );
    // End set baud rate

    // Disable RTS/CTS hardware flow control
    newtio->c_cflag &= ~CRTSCTS;

    /* *** Line option **** */
    // Canonical input 
    newtio->c_lflag |= (ICANON | ECHO | ECHOE);
    // raw input
    //newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // disable software flow control
    //newtio->c_iflag &= ~(IXON | IXOFF | IXANY);
    //newtio->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    /* *** Control characters **** */
    // Time to wait for data
    newtio->c_cc[VTIME] = 0; // 十分之一秒为单位
    // Minimum number of characters to read
    newtio->c_cc[VMIN] = 1;

    // Flushes the input and/or output queue
    tcflush( fd, TCIFLUSH );

    /**
     * Set config to port
     * TCSANOW: Make changes now without waiting for data to complete
     * TCSADRAIN: Wait until everything has been transmitted
     * TCSAFLUSH: Flush input and output buffers and make the change
     */
    if ( tcsetattr( fd, TCSANOW, newtio) != 0) 
    {
        perror ("Set config error" ) ;
        return false;
    } else
        return true;
}

void read_data(int fd)
{
    int res = 1;
    int count = 0;
    unsigned char buf[MINMEA_MAX_LENGTH];
    // 测试解析的运行时间
    uint64_t diff;
    struct timespec start, end;

    while (res != -1)
    {
        // measure monotonic time
	    clock_gettime(CLOCK_MONOTONIC, &start);

        memset(buf, 0, MINMEA_MAX_LENGTH);
        
        res = read(fd, buf, MINMEA_MAX_LENGTH);
        if (res == -1)
        {
            perror("Read NMEA data error");
            return;
        }
        //buf[res] = '\0';
        // 当前只输出ZDA
        printf("No.%d, res = %d, Raw NMEA: %s\n", ++count, res, buf);
        for(int i = 0; i < res; ++i)
            printf("%x",(int)(buf[i]));
        putchar('\n');
        struct minmea_sentence_zda frame;
        
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

        // mark the end time 
        clock_gettime(CLOCK_MONOTONIC, &end);	
        diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
        printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
    }
}

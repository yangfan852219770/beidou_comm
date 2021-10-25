/**
 * Copyright © Yangfan <yangyangfan@stud.tjut.edu.cn>
 * This program is free software.
 */ 
#include "beidou_read.h"
#include "minmea.h"



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
    
    cfsetispeed( newtio, b_baud_rate );
    cfsetospeed( newtio, b_baud_rate );

    // Canonical input 
    newtio->c_lflag |= (ICANON | ECHO | ECHOE);
    // raw input
    //newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    // disable software flow control
    newtio->c_iflag &= ~(ISTRIP | IXON | IXOFF | IXANY | BRKINT | IMAXBEL | PARMRK);
  
    newtio->c_iflag |= IGNCR | IGNPAR | IGNBRK;

    // Time to wait for data
    //newtio->c_cc[VTIME] = 1    ; // 十分之一秒为单位
    // Minimum number of characters to read
    newtio->c_cc[VMIN] = 1;

    // Flushes the input and/or output queue
    tcflush( fd, TCSANOW );

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

void read_data(int fd, unsigned char *buf, int *actual_length, int timeout)
{
    int ret;
    int count = 0;
    fd_set rd;
    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    FD_ZERO(&rd);
    FD_SET(fd, &rd);

    // 检查串口数据是否准备好
    ret = select(fd + 1, &rd, NULL, NULL, &tv);
    if(ret > 0){
        if (FD_ISSET(fd, &rd)) {
            *actual_length = read(fd, buf, READ_MAX_LENGTH);
        }
    }
    else if(ret == 0){
        printf("[%s %d]timeout\n", __FUNCTION__, __LINE__);
        return;
    }
    else{
        printf("[%s %d] select error!\n", __FUNCTION__, __LINE__);
        return;
    }
}

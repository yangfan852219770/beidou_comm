/*
    USB to UART
*/

#ifndef _BEIDOU_READ_H
#define _BEIDOU_READ_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> /* Unix standard function def */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>   /* File control def */
#include <termios.h> /* POSIX terminal control def */
#include <errno.h>

#include "minmea.h"

#define READ_MAX_LENGTH 128
#define BILLION 1000000000L

/**
 * Open the port
 * port_path: The path of port
 * fd: File descriptor for the port
 * flags: The mode of accessing the port
 * Return true or false.
 */
bool open_usb_port( const char *port_path, int *fd, int flags );

/**
 * Set parameters of the port, such as baud rate and parity
 * oldtio: The old config of board
 * newtio: The new config of board
 * fd: File descriptor for the port
 * baud_rate: The baud of your Beidou board
 * data_bit: The bits of data in a frame
 * parity_flag: The parity of your frame, Maybe even, odd or none
 * stop_bit: The bits of stop. Maybe 1 bit or 2 bits
 * Return true or false.
 */ 
bool set_parameter_port( struct termios * oldtio, struct termios * newtio, int fd, int baud_rate, int data_bit, char parity_flag, int stop_bit );

/**
 * Read and parse the NMEA data 
 * fd: File descriptor for the port
 * 
 */
void read_data(int fd, unsigned char *buf, int *actual_length, int timeout);

#endif
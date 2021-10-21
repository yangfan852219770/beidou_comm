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
    // Open the port
    bool res = open_usb_port( SERIALPATH, &fd, O_RDONLY | O_NOCTTY);
    if( !res )
        return -1;
    // Set config
    res = false;
    res = set_parameter_port( &newtio, &oldtio, fd, BAUD_RATE, DATA_BIT, PARITY_BIT, STOP_BIT );
    if( !res )
        return -1;
    // Read data
    read_data( fd );
    // Restore the old config
    tcsetattr( fd, TCSANOW, &oldtio );
    // Close the port
    close( fd );
    return 0;
}

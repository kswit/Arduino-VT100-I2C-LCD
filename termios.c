#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <stdbool.h>

#define BAUDRATE B9600
#define MODEMDEVICE "/dev/ttyACM0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define XON  0x11
#define XOFF 0x13
char read_buf [256];

volatile int STOP=FALSE;

    fd_set rfds;
    struct timeval tv;
    int retval;


void signal_handler_IO (int status); /* definition of signal handler */
int wait_flag=TRUE; /* TRUE while no signal received */

struct termios tty;



int serial_open(char *port, int baud)
{
    int fd;
   // struct termios tty;
//   if((fd = open(port, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY)) < 0)
    if((fd = open(port, O_RDWR | O_NOCTTY  | O_NONBLOCK)) < 0)
    {
        return -1;
    }

    if(tcgetattr(fd, &tty) < 0)
    {
        return -1;
    }
   //fcntl(fd, F_SETOWN, getpid());
   //fcntl(fd, F_SETFL, FASYNC);

    cfsetospeed(&tty, (speed_t)baud);
    cfsetispeed(&tty, (speed_t)baud);
    
 // tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
  tty.c_cflag &= ~(PARENB | PARODD);
  
 tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  //tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_lflag |= ISIG;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of rec>
  //tty.c_iflag &= (IXON | IXOFF | IXANY);
  //tty.c_iflag |=  IXON ;
  //tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)
  tty. c_cflag &= ~HUPCL;
  tty.c_cc[VSTART]=0x11;
  tty.c_cc[VSTOP]=0x13;
  tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is receiv>
  tty.c_cc[VMIN] = 0;


    if(tcsetattr(fd, TCSANOW, &tty))
    {
        return -1;
    }

    return fd;
}

/* example usage */
int main(int argc,char** argv)
{
unsigned char text='a';
volatile int  wlen;
unsigned char buf[1];
unsigned char c='D';
unsigned char xon=XON;
int fd;
int bytes;
bool trans;


//fd = serial_open("/dev/ttyACM0", B9600);

//ioctl(fd, FIONREAD, &bytes);
//printf("buffer %d ",bytes);

//if(flock(fd, LOCK_EX | LOCK_NB) == -1) {printf("perror");
  //   }
    // else{
    // fd = serial_open("/dev/ttyACM0", B9600);
      fd = serial_open(argv[1], B9600);

        if(fd>0){
        //tcsetattr(fd, TCSAFLUSH,&tty); 
        
         FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        

        tv.tv_sec = 5;
        tv.tv_usec = 0;


        sleep(5);
        tcflow(fd,TCION);

        }else{exit(1);};
        
        int data=0;

	for (int i=0 ;i<150;i++){
        int n = read(fd,&read_buf, sizeof(read_buf));
        wlen = write(fd ,&text,sizeof(text));
	if(wlen>0)
        {
        //retval = select(fd, &rfds, NULL, NULL, &tv);
	if(read_buf[0]==XOFF)sleep(1); 
        printf("input buff=0x%x \n",read_buf[0]);
        
        printf("write %d,%d ",data++,i);
        tcdrain(fd);
       
        if (bytes>0){trans=0;}
	}
	else{
        if(!trans) tcflow(fd,TCOOFF);
	printf("stop %d ,%d",wlen,trans);
	sleep(0.1);
        //write(fd,&xon,1);
	wlen=1;
	data=0;
	tcsetattr(fd, TCSAFLUSH,&tty);
	    }

                            }

close(fd);
return 0;
}
//}

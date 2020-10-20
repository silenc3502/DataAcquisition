#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t mtx;

int set_interface_attribs(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}

void *serial_read(void *fd)
{
	int serial_fd = *((int *)fd);
	char buf[128] = "";

	for(;;)
    {
		read(serial_fd, buf, sizeof(buf));
		printf("Serial Read = %s\n", buf);
		memset(buf, 0x0, 128);

		usleep(5000);
	}
}

void *serial_write(void *fd)
{
	int serial_fd = *((int *)fd);
	char buf[128] = "";

	for(;;)
    {
        pthread_mutex_lock(&mtx);

		read(0, buf, sizeof(buf));
		write(serial_fd, buf, strlen(buf));
		printf("Serial Write: %s\n", buf);
		memset(buf, 0x0, 128);

		pthread_mutex_unlock(&mtx);

		usleep(5000);
	}
}

int main()
{
    char *portname = "/dev/ttyACM0";
    int fd;
    int wlen;

	pthread_t p_thread[2];
    int thread_id, status;

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    //set_interface_attribs(fd, B115200);
    set_interface_attribs(fd, B9600);
    //set_mincount(fd, 0);                /* set to pure timed read */

	pthread_mutex_init(&mtx, NULL);

	thread_id = pthread_create(&p_thread[0], NULL, serial_write, (void *)&fd);
    if(thread_id < 0)
    {
        perror("network recv thread create error: ");
        exit(0);
    }

    thread_id = pthread_create(&p_thread[1], NULL, serial_read, (void *)&fd);
    if(thread_id < 0)
    {
        perror("command proc thread create error: ");
        exit(0);
    }

	pthread_join(p_thread[0], (void **)&status);
    pthread_join(p_thread[1], (void **)&status);

#if 0
    /* simple output */
    wlen = write(fd, "Hello!\n", 7);
    if (wlen != 7) {
        printf("Error from write: %d, %d\n", wlen, errno);
    }
    tcdrain(fd);    /* delay for output */


    /* simple noncanonical input */
    do {
        unsigned char buf[80];
        int rdlen;

        rdlen = read(fd, buf, sizeof(buf) - 1);
        if (rdlen > 0) {
#ifdef DISPLAY_STRING
            buf[rdlen] = 0;
            printf("Read %d: %s\n", rdlen, buf);
#else /* display hex */
            unsigned char   *p;
            printf("Read %d:", rdlen);
            for (p = buf; rdlen-- > 0; p++)
                printf(" 0x%x", *p);
            printf("\n");
#endif
        } else if (rdlen < 0) {
            printf("Error from read: %d: %s\n", rdlen, strerror(errno));
        } else {  /* rdlen == 0 */
            printf("Timeout from read\n");
        }               
        /* repeat read to get full message */
    } while (1);
#endif

	return 0;
}

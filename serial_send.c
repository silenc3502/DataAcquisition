#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <termios.h>

int open_serial(char *dev_name, int baud, int vtime, int vmin)
{
	int fd;
	struct termios newtio;

	fd = open(dev_name, O_RDWR | O_NOCTTY);

	if(fd < 0)
	{
		printf("Device Open Fail %s\n", dev_name);
		return -1;
	}

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	newtio.c_cflag = CS8 | CLOCAL | CREAD;

	switch(baud)
	{
		case 115200:
			newtio.c_cflag |= B115200;
			break;
		case 57600:
			newtio.c_cflag |= B57600;
			break;
		case 38400:
			newtio.c_cflag |= B38400;
			break;
		case 19200:
			newtio.c_cflag |= B19200;
			break;
		case 9600:
			newtio.c_cflag |= B9600;
			break;
		case 4800:
			newtio.c_cflag |= B4800;
			break;
		case 2400:
			newtio.c_cflag |= B2400;
			break;
		default:
			newtio.c_cflag |= B115200;
			break;
	}

	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = vtime;
	newtio.c_cc[VMIN] = vmin;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
}

void close_serial(int fd)
{
	close(fd);
}

int main(int argc, char **argv)
{
	int fd;
	int baud;
	char dev_name[128];
	char cc, buf[128];
	int rdcnt;

	if(argc != 3)
	{
		printf("sample_serial[device][baud]\n" \
				"device: /dev/ttyUSBx ...\n" \
				"baud: 2400 ... 115200\n");
		return -1;
	}

	printf("Serial Test Start... (%s)\n", __DATE__);

	strcpy(dev_name, argv[1]);
	baud = stroul(argv[2], NULL, 10);

	fd = open_serial(dev_name, baud, 10, 32);

	if(fd < 0)
		return -2;

	for(cc = 'A'; cc <= 'z'; cc++)
	{
		memset(buf, cc, 32);
		write(fd, buf, 32);

		rdcnt = read(fd, buf, sizeof(buf));
		if(rdcnt > 0)
		{
			buf[rdcnt] = '\0';
			printf("<%s rd=%2d> %s\n", dev_name, rdcnt, buf);
		}

		sleep(1);
	}

	close_serial(fd);

	printf("serial test end\n");

	return 0;
}

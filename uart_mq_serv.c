#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include <termios.h>

struct uart_data
{
	char buf[64];
};

struct message
{
	long msg_type;
	struct uart_data data;
};

int fd;

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

void printMsgInfo(int msqid)
{
	struct msqid_ds m_stat;
	printf("========== messege queue info =============\n");

	if(msgctl(msqid, IPC_STAT, &m_stat) == -1)
	{
		printf("msgctl failed");
		exit(0);
	}

	printf(" message queue info \n");
	printf(" msg_lspid : %d\n", m_stat.msg_lspid);
	printf(" msg_qnum : %lu\n", m_stat.msg_qnum);
	printf(" msg_stime : %lu\n", m_stat.msg_stime);

	printf("========== messege queue info end =============\n");
}

void finish(int signo)
{
	printf("Finish Data Acquisition\n");
	close(fd);
	exit(0);
}

int main(void) {
	char *portname = "/dev/ttyACM0";
    int wlen;

	key_t key = 12345;
	int msqid;

	struct message msg;
	msg.msg_type = 1;

	pid_t pid;
	int i;

	pid = fork();
	if(pid == -1) {
		printf("fork\n");
		return -1;
	}

	if(pid != 0) {
		exit(EXIT_SUCCESS);
	}

	if(setsid() == -1) {
		return -1;
	}

	if(chdir("/") == -1) {
		printf("chdir()\n");
		return -1;
	}

	open("/dev/null", O_RDWR);
	dup(0);
	dup(0);

	fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }

    set_interface_attribs(fd, B9600);

	signal(SIGINT, finish);

	for(;;) {
		unsigned char buf[80];
        int rdlen;

        rdlen = read(fd, buf, sizeof(buf) - 1);
		buf[rdlen] = 0;

		strcpy(msg.data.buf, buf);
		//strcpy(msg.data.buf, "Hi");

		// msqid를 얻어옴.
		if((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
		{
			printf("msgget failed\n");
			exit(0);
		}

		// 메시지 보내기 전 msqid_ds를 한번 보자.
		//printMsgInfo(msqid);

		// 메시지를 보낸다.
		if(msgsnd(msqid, &msg, sizeof(struct uart_data), 0) == -1)
		{
			printf("msgsnd failed\n");
			exit(0);
		}

		printf("message sent\n");

		// 메시지 보낸 후 msqid_ds를 한번 보자.
		//printMsgInfo(msqid);
		//sleep(2);
	}

	return 0;
}

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

struct uart_data
{
	char buf[64];
};

struct message
{
	long msg_type;
	struct uart_data data;
};

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

int main(void) {
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

	for(;;) {
		strcpy(msg.data.buf, "Hi");

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
		sleep(2);
	}

	return 0;
}

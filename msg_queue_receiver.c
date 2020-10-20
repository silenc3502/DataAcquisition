#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>

struct uart_data
{
	char buf[64];
};

struct message
{
	long msg_type;
	struct uart_data data;
};

int main(void)
{
	key_t key = 12345;
	int msqid;
	struct message msg;

	for(;;) {
		//받아오는 쪽의 msqid얻어오고
		if((msqid = msgget(key, IPC_CREAT | 0666)) == -1)
		{
			printf("msgget failed\n");
			exit(0);
		}

		//메시지를 받는다.
		if(msgrcv(msqid, &msg, sizeof(struct uart_data), 0, 0) == -1)
		{
			printf("msgrcv failed\n");
			exit(0);
		}
	
		printf("buf : %s\n", msg.data.buf);
	
		//이후 메시지 큐를 지운다.
		if(msgctl(msqid, IPC_RMID, NULL) == -1)
		{
			printf("msgctl failed\n");
			exit(0);
		}
	}
}

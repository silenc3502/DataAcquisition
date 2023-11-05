#include <stdio.h>
#include <string.h>

int main(void)
{
	printf("AT+CSCS=\"GSM\"\n\r");
	printf("Length = %d\n", strlen("AT+CSCS=\"GSM\"\n\r"));
	return 0;
}

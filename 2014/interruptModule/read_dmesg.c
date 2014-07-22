#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define INTERRUPT_TXT_FILE "/home/root/interruptLog.txt"


/* Shell script that prints dmesg to interruptLog.txt in home folder.
*  -t makes dmesg print w/o timestamp so we can just read char.
*  -c makes dmesg clear after printing to text file.
*/
#define SHELLSCRIPT1"\
#/bin/bash \n\
dmesg -t -c > /home/root/interruptLog.txt \n\
"

//Check flags function
void checkFlags()
{
	system(SHELLSCRIPT);
	
	//printf("1\n");

	FILE *interruptLog = fopen(INTERRUPT_TXT_FILE,"r");
	if(!interruptLog)
	{
		printf("File could not be opened\n");
	}

	//printf("2\n");
	
	char flag[1];
	memset(flag,0,sizeof(flag));
	fread(flag,sizeof(char),1,interruptLog);
	fclose(interruptLog);
	
	int i;
	
	//none of the if statements are running
	char flagA = '#';
	char flagB = '$';
	char flagAB = '&';
	if(flag[0] == flagA)
	{
		printf("Interrupt on channel B");
		flag[0] = '\0';
	}
	else if(flag[0] == flagB)
	{
		printf("Interrupt on channel A");
		flag[0] = '\0';
	}
	else if(flag[0] == flagAB)
	{
		printf("Interrupt on channel AB");
		flag[0] = '\0';
	}
	else
	{
		//NO INTERRUPT DETECTED
	}
}




int main()
{
	while(1)
	{
//		printf("!\n");
		checkFlags();
	}
	return 0;
}

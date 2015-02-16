#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "globaldefs.h"

int updateEventCounter(struct xray *xrayData_ptr)
{
	// Update event A counter
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t readLine;
	fp = fopen("/home/root/peakLogging/eventACounts", "r");
	if (fp == NULL)
	{
		;// skip
	}
	else
	{
		getline(&line, &len, fp);
		xrayData_ptr->countsA = atoi(line);
		//printf("%d\n", count);
	}
	// Update event B counter
	fflush(fp);
	fclose(fp);
	fp = fopen("/home/root/peakLogging/eventBCounts", "r");
	if (fp == NULL)
	{
		;//skip
	}
	else
	{
		getline(&line, &len, fp);
		xrayData_ptr->countsB = atoi(line);
		//printf("%d\n", count);
	}

	fflush(fp);
        fclose(fp);

	// Update event AB counter
	fp = fopen("/home/root/peakLogging/eventABCounts", "r");
	if (fp == NULL)
	{	
		;//skip
		
	}
	else	
	{
		getline(&line, &len, fp);
		xrayData_ptr->countsAB = atoi(line);
		//printf("%d\n", count);
	}

	fflush(fp);
        fclose(fp);
	
	if (line)
		free(line);
	

	return 1;
}

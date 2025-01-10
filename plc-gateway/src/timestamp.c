/*
 * timestamp.c
 *
 *  Created on: 22-Apr-2024
 *      Author: madhulika
 */
#include <time.h>
#include <stdio.h>
void get_timestamp(char *timestamp) {
	time_t now;
	struct tm *local_time;
	time(&now);
	local_time = localtime(&now);
	sprintf(timestamp, "[%04d-%02d-%02d] [%02d:%02d:%02d]",
			local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday,
			local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
}

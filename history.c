#include "history.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct history arr[HIST_MAX]; //Initializes array of structs

char* getCommand(int num, int size)
{
	if(num > size)
	{
		return NULL;
	}
	else
	{
		struct history temp = arr[num];
		char* command = temp.entry;
		return command;
	}
}

//If '!!' command (or the equivalent) was entered, this function updates the history log accordingly.
void updateEntry(char* text, int index)
{
	size_t size = strlen(text);
	char *b = malloc(size);
	strcpy(b, text);
	arr[index].entry = b;
}

//Helper function (same as .startsWith)
bool prefix(const char *pre, const char *str)
{
	size_t lenpre = strlen(pre), lenstr = strlen(str);
	return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

//Functions that extracts the most recent entry in the history log.
char* getMostRecent(int size)
{
	char* p = "ls";
	int i = size;
	while(i >= 0) //finds most recent, so iterates backwards
	{
		struct history tmp = arr[i]; //finds index in array structs
		if(prefix(p, tmp.entry) != false)
		{
			printf("index:%d | entry: %s\n", i, tmp.entry);
			return tmp.entry;
		}
		i--;
	}
	return NULL;
}

//Updates the run time and adds it back to the struct
void updateTime(int index, double time)
{
	arr[index].run_time = time;
}

//Prints the last 100 history entries
void print_history(int size)
{
	int start;
	if(size < 100)
	{
		start = 0;
	}
	else
	{
		start = size - 100;
	}
	for(int i = start; i < size; i++)
	{
		struct history tmp = arr[i];
		printf("[%d|%0.3f] %s",tmp.cmd_id, tmp.run_time, tmp.entry);
	}
}

//Adds to history struct
void add(int index, char* tokens, int commandCount, double time)
{
	char *cmd = tokens;
	size_t size = strlen(cmd);
	char *c = malloc(size);
	strcpy(c, cmd);
	int id = commandCount;
	double rt = time;
	arr[index].entry = c;
	arr[index].cmd_id = id;
	arr[index].run_time = rt;
}

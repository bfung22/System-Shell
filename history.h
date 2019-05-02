#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history
{
	unsigned int cmd_id;
	double run_time;
	char *entry;
};

void print_history(int size);
void updateTime(int index, double time);
void updateEntry(char* text, int index);
void add(int index, char* line, int commandCount, double time);
char* getCommand(int num, int size);
char* getMostRecent(int size);
#endif

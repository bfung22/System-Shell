#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include "history.h"
#include "timer.h"
#include <pwd.h>

int size = 1;
int commandCount = 0;
char* func;

void sigint_handler(int signo);
void prompt();
int changeDirectory(char* args[]);
bool validNumber(char num[]);
bool startsWith(const char *pre, const char *str);
void exec(char *args[]);
int cleanNumber(char* s);
void redirection(char *args[]);
void redirectOutput(char *args[]);
int parseCommands(char *args[]);

//Signal handler to exit gracefully
void sigint_handler(int signo)
{
    printf("\nGraceful exit ....\n");
    fflush(stdout);
    exit(0);
}

//Opening shell prompt that displays username and host name,
//the current directory, as well as the command ID number
void prompt()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char* cwd;
    char hostn[1024] = "";
    char buf[1024];
    gethostname(hostn, sizeof(hostn));
    cwd = getcwd(buf, sizeof(buf));

    if(strcmp(homedir, cwd) == 0)
    {
        printf("[%d|%s@%s:~]$ ", commandCount, getlogin(), hostn);
    }
    else
    {
        printf("[%d|%s@%s:%s]$ ", commandCount, getlogin(), hostn, cwd);
    }
}

//Function that changes directory (similar to 'cd' on terminal)
int changeDirectory(char* args[])
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;

	if (args[1] == NULL)
	{
		chdir(homedir); 
		return 1;
	}

	else
	{ 
		if (chdir(args[1]) != 0)
		{
			perror("");
			return -1;
		}
	}
	return 1;
}

//Function that makes sure input char is a valid number
//Used in parseCommands function
bool validNumber(char num[])
{
    for(int i = 1; num[i] != 0; i++)
    {
        if(num[i] == '-' || num[i] == '.')
        {
            ++i;
        }
        else
        {
            if(!isdigit(num[i]))
            {
                return false;
            }
        }
    }
    return true;
}

//Function that checks to see if a string is a prefix of another string (taken from stackoverflow)
//Used in parseCommands function
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre), lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

//Function that runs execvp commands
void exec(char *args[])
{
	pid_t pid = fork();
	if (pid == 0)
	{
		if(execvp(args[0],args) == -1)
		{
		      printf("Invalid command or nonexistent file/directory\n");
		      exit(1);
		}
	}
	else if(pid < 0)
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	else
	{
		int status;
		wait(&status);
	}
}

//Functions that converts a char to a digit (for example) '99' to 99. 
int cleanNumber(char* s)
{
	char str[50];
	strcpy(str, s);
	char strStripped[50];
	
	int i = 0, c = 0; /*I'm assuming you're not using C99+*/
	for(; i < strlen(str); i++)
	{
		if (isalnum(str[i]))
		{
			strStripped[c] = str[i];
			c++;
		}
	}
	strStripped[c] = '\0';
	return atoi(strStripped);
}

//Redirect output such as: echo "hello world!" > my_file.txt
void redirectOutput(char *args[])
{
	int i = 0;
	while(args[i] != NULL)//
	{
		//Assuming that file is the last argument
		i++;
	}

	pid_t pid = fork();
	if(pid == 0) //child process
	{
		if(execvp(args[0],args) == -1)
		{
			printf("Does not exist\n");
			exit(1);
		}
		int output = open(args[i] ,O_CREAT | O_WRONLY, 0666);
		dup2(output, STDOUT_FILENO);
	}
	else if(pid < 0) //something went wrong
	{
		perror("Error");
		exit(EXIT_FAILURE);
	}
	else //Parent
	{
		int status;
		wait(&status);
		//Child finished executing, parent exiting
	}
}

//Incomplete
void redirection(char *args[])
{
	int i = 0;
	int p = 0;
	int redir = 0;
        while(args[i] != NULL)
        {
		if(strcmp(args[i], "|"))
		{
			p++;
		}
		if(strcmp(args[i], ">"))
		{
			redir++;
		}
		if(p > 0 && redir == 0)
		{
                	int output = open(args[p] ,O_CREAT | O_WRONLY, 0666);
                	dup2(output, STDOUT_FILENO);
		}
                i++;
        }
	int fd[2];
	pipe(fd);

	pid_t pid = fork();
	if(pid == 0)
	{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		
	}
	else
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
	}
	close(fd[1]);
}

//Function that runs built in commands such as:
//exit, cd, history (last 100 entries), and various ! commands
int parseCommands(char *args[])
{
	int it = 0;
	int redirect = 0;
	int l = 0;
	while(args[it]!= NULL)
	{
		if(strcmp(args[it], ">") == 0)
		{
			redirect++;
		}
		if(strcmp(args[it], "|") == 0)
		{
			l++;
		}
		it++;
	}

	if(redirect > 0 && l ==0)
	{
		redirectOutput(args);
		return 1;
	}

	if(redirect >0 && l > 0)
	{
		redirection(args);
		return 1;
	}
	char* input;
	if(strcmp(args[0], "exit") == 0) //graceful exit
	{
		printf("Exiting...\n");
		exit(1);
	}
	else if (strcmp(args[0], "cd") == 0) //change directory
	{
		changeDirectory(args);
		return 1;
	}

	else if (strcmp(args[0],"history") == 0) //history log
	{
		print_history(commandCount);
		return 1;
	}
       	else if (startsWith("!", args[0])) 
       	{
		if(strcmp(args[0], "!") == 0) //if it's a single '!' , run execvp
		{
			exec(args);
			return -1;
		}
		else if(validNumber(args[0])) //runs ![number] , but first checks for valid number
		{
			if(getCommand(cleanNumber(args[0]),commandCount) == NULL)
			{
				//if number does not exceed the number of history entries
				printf("Command does not exist in history\n");
				return -1;
			}

			input = getCommand(cleanNumber(args[0]),commandCount); //gets the entry
			char a[1024];
			strcpy(a, input);
			char *token1 = strtok(a, " \t\n");
			char *tokens1[PATH_MAX];
			int index1 = 0;
			while (token1 != NULL)
			{
				tokens1[index1++] = token1;
				token1 = strtok(NULL, " \t\n");
			}
			tokens1[index1] = (char *) 0;
			parseCommands(tokens1); //reruns command
			updateEntry(input, commandCount); //updates history log
		}
		else
		{
			if(strcmp(args[0], "!!") == 0) //!!
			{
				input = getCommand(commandCount-1, commandCount); //gets entry
				char b[1024];
				strcpy(b, input);
				char *token2 = strtok(b, " \t\n");
				char *tokens2[PATH_MAX];
				int index2 = 0;
				while (token2 != NULL)
				{
					tokens2[index2++] = token2;
					token2 = strtok(NULL, " \t\n");
				}
				tokens2[index2] = (char *) 0;
				parseCommands(tokens2); //after tokenizing to char*[], rerun command
				updateEntry(input, commandCount);
			}
			else if(strcmp(args[0], "!ls") == 0) //checks for '!ls'
			{
				if(getMostRecent(commandCount) == NULL) //does not exist
				{
					printf("No 'ls' command found\n");
					return -1;
				}
				char* input2;
				input = getMostRecent(commandCount); //gets last 'ls' entry
				char c[1024];
				input2 = input;
                                strcpy(c, input2);
                                char *token3 = strtok(c, " \t\n");
                                char *tokens3[PATH_MAX];
                                int index3 = 0;
                                while (token3 != NULL)
                                {
                                        tokens3[index3++] = token3;
                                        token3 = strtok(NULL, " \t\n");
                                }
                                tokens3[index3] = (char *) 0;
                                parseCommands(tokens3); //reruns command after tokenizing
				input = NULL;
				return 1;
			}
			else
			{
				printf("Invalid command\n");
				return -1;
			}
		}
	}
	else //run execvp if parameter being passed does not match the conditions
	{
		exec(args);
	}
	return -1;
}

//Main function that gets user input through fgets, tokenizes the input, and adds to struct history.
int main(int argc, char *argv[])
{
	int index = 0;
	int size = 1;
	signal(SIGINT, sigint_handler);
	while(true)
	{
		char line[PATH_MAX];
		prompt();
		fgets(line, PATH_MAX, stdin);
		double start = get_time();
		char tk[PATH_MAX];
		strcpy(tk, line);
		char *token = strtok(tk, " \t\n");
		char *tokens[PATH_MAX];
		int i = 0;
		while (token != NULL)
		{
			tokens[i++] = token;
			token = strtok(NULL, " \t\n");
		}
		tokens[i] = (char *) 0;

		if(tk[0]!='\n' && tk[0]!= '#' && tokens[0]!= NULL)
		{
			double end = get_time();
			add(commandCount, line, commandCount, end-start);
			parseCommands(tokens);
			end = get_time();
			updateTime(commandCount, end-start);
			size++;
			index++;
			commandCount++;
		}
	}
	return 0;
}

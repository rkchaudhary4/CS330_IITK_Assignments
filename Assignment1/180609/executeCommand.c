#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
int executeCommand(char *cmd)
{
	pid_t pid = fork();
	if (pid < 0)
	{
		perror("Fork");
		printf("UNABLE TO EXECUTE\n");
		exit(-1);
	}
	if (!pid)
	{
		char temp[strlen(cmd) + 1];
		strcpy(temp, cmd);
		int count = 0;
		char *tok = strtok(temp, " ");
		while (tok != NULL)
		{
			count++;
			tok = strtok(NULL, " ");
		}
		char **c = (char **)malloc(count * sizeof(char *) + sizeof(NULL));
		tok = strtok(cmd, " ");
		int i = 0;
		while (tok != NULL)
		{
			c[i++] = tok;
			tok = strtok(NULL, " ");
		}
		c[i] = NULL;
		char *path = getenv("CS330_PATH");
		char *token = strtok(path, ":");
		while (token != NULL)
		{
			char s[strlen(token) + strlen(cmd) + 2];
			strcpy(s, token);
			strcat(s, "/");
			strcat(s, cmd);
			execv(s, c);
			token = strtok(NULL, ":");
		}
		printf("UNABLE TO EXECUTE\n");
		exit(-1);
	}
	else
	{
		int status;
		wait(&status);
		if (status == 255)
			return -1;
		return status;
	}
}

int main(int argc, char *argv[])
{
	return executeCommand(argv[1]);
}

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void executeCommand(char *cmd)
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

int execute_in_parallel(char *infile, char *outfile)
{
	close(0);
	close(1);
	int r = open(infile, O_RDONLY);
	int w = open(outfile, O_RDWR | O_CREAT, 0777);
	if (w < 0 || r < 0)
	{
		perror("File descriptors");
		printf("UNABLE TO EXECUTE\n");
		exit(-1);
	}
	char str[64];
	char **lines = (char **)malloc(sizeof(char *) * 50);
	int i = 0;
	while (fgets(str, 64, stdin))
	{
		str[strlen(str) - 1] = '\0';
		lines[i] = (char *)malloc(sizeof(char) * (strlen(str) + 1));
		strcpy(lines[i], str);
		i++;
	}
	int pfds[i][2];
	for (int j = 0; j < i; j++)
	{
		pipe(pfds[j]);
		if (pfds[j][0] < 0 || pfds[j][1] < 0)
		{
			perror("pipe");
			printf("UNABLE TO EXECUTE\n");
			exit(-1);
		}
		pid_t pid = fork();
		if (pid < 0)
		{
			perror("fork");
			printf("UNABLE TO EXECUTE\n");
			exit(-1);
		}
		if (!pid)
		{
			dup2(pfds[j][1], 1);
			executeCommand(lines[j]);
		}
	}
	for (int j = 0; j < i; j++)
	{
		char buff[1024];
		int len = read(pfds[j][0], buff, 1024);
		buff[len] = '\0';
		printf("%s", buff);
	}
}

int main(int argc, char *argv[])
{
	return execute_in_parallel(argv[1], argv[2]);
}


#define ROCK 0
#define PAPER 1
#define SCISSORS 2

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#include "gameUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int getWalkOver(int numPlayers); // Returns a number between [1, numPlayers]

int firstWin(char out1, char out2)
{
	if (out1 == '0')
	{
		if (out2 == '2')
			return 1;
		else if (out2 == '1')
			return 0;
	}
	else if (out1 == '1')
	{
		if (out2 == '0')
			return 1;
		else if (out2 == '2')
			return 0;
	}
	else if (out1 == '2')
	{
		if (out2 == '1')
			return 1;
		else if (out2 == '0')
			return 0;
	}
}

int main(int argc, char *argv[])
{
	int n = 10, fd;
	if (argc == 4)
	{
		n = atoi(argv[2]);
		fd = open(argv[3], O_RDONLY);
	}
	else
	{
		fd = open(argv[1], O_RDONLY);
	}
	if (fd == -1)
	{
		exit(-1);
	}
	int np;
	char p;
	if (read(fd, &p, 1) != -1)
	{
		np = atoi(&p);
	}
	else
	{
		exit(-1);
	}
	if (read(fd, &p, 1) == -1)
	{
		exit(-1);
	}
	int reads[np][2], writes[np][2];
	for (int i = 0; i < np; i++)
	{
		if (pipe(reads[i]) == -1)
		{
			exit(-1);
		}
		if (pipe(writes[i]) == -1)
		{
			exit(-1);
		}
	}
	for (int i = 0; i < np; i++)
	{
		char path[100];
		int curr = 0;
		while ((read(fd, &p, 1) != -1) && (p != '\n'))
		{
			path[curr++] = p;
		}
		path[curr] = '\0';
		char *cmd[] = {path, NULL};
		pid_t pid = fork();
		if (pid < 0)
			exit(-1);
		if (!pid)
		{
			for (int j = 0; j < np; j++)
			{
				if (j == i)
				{
					close(writes[j][1]);
					close(reads[j][0]);
				}
				else
				{
					close(writes[j][0]);
					close(writes[j][1]);
					close(writes[j][0]);
					close(writes[j][1]);
				}
			}
			dup2(writes[i][0], 0);
			dup2(reads[i][1], 1);
			execv(cmd[0], cmd);
			exit(-1);
		}
		else
		{
			close(writes[i][0]);
			close(reads[i][1]);
		}
	}
	int rem = np;
	int active[np];
	for (int i = 0; i < np; i++)
		active[i] = 1;
	while (rem != 1)
	{
		int print = 0;
		for (int i = 0; i < np; i++)
		{
			if (active[i])
			{
				if (print)
					printf(" ");
				printf("p%d", i);
				print = 1;
			}
		}
		printf("\n");
		int walk = -1;
		int pairs[rem / 2][2];
		int points[rem / 2][2];
		for (int i = 0; i < rem / 2; i++)
		{
			pairs[i][0] = -1;
			pairs[i][1] = -1;
			points[i][0] = 0;
			points[i][0] = 0;
		}
		int curr = 0;
		if (rem % 2)
		{
			walk = getWalkOver(rem);
		}
		int act = 0;
		for (int i = 0; i < np; i++)
		{
			if (active[i])
			{
				act++;
				if (act == walk)
					continue;
				if (pairs[curr][0] == -1)
					pairs[curr][0] = i;
				else
				{
					pairs[curr][1] = i;
					curr++;
				}
			}
		}
		char str[] = {'G', 'O', '\0'};
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < rem / 2; j++)
			{
				char out1[2], out2[2];
				write(writes[pairs[j][0]][1], str, 3);
				write(writes[pairs[j][1]][1], str, 3);
				if ((read(reads[pairs[j][0]][0], out1, 2) != -1) && (read(reads[pairs[j][1]][0], out2, 2) != -1))
				{
					if (out1[0] == out2[0])
						continue;
					if (firstWin(out1[0], out2[0]))
						points[j][0]++;
					else
						points[j][1]++;
				}
				else
					exit(-1);
			}
		}
		for (int j = 0; j < rem / 2; j++)
		{
			if (points[j][0] >= points[j][1])
			{
				active[pairs[j][1]] = 0;
			}
			else
			{
				active[pairs[j][0]] = 0;
			}
		}
		rem -= rem / 2;
	}
	int print = 0;
	for (int i = 0; i < np; i++)
	{
		if (active[i])
		{
			if (print)
				printf(" ");
			printf("p%d", i);
			print = 1;
		}
	}
	for (int i = 0; i < np; i++)
	{
		close(writes[i][1]);
		close(reads[i][0]);
	}
	return 0;
}

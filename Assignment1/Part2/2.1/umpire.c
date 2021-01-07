#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
	int fp = 0, sp = 0;
	int pfd1[2], pfd2[2];
	if (pipe(pfd1) == -1)
	{
		perror("pipe fail");
		exit(-1);
	}
	if (pipe(pfd2) == -1)
	{
		perror("pipe failed");
		exit(-1);
	}
	int pfd3[2], pfd4[2];
	if (pipe(pfd3) == -1)
	{
		perror("pipe failed");
		exit(-1);
	}
	if (pipe(pfd4) == -1)
	{
		perror("pipe failed");
		exit(-1);
	}
	char str[] = {'G', 'O', '\0'};
	pid_t pid1 = fork();
	if (pid1 < 0)
	{
		perror("fork failed");
		exit(-1);
	}
	if (!pid1)
	{
		close(pfd3[1]);
		close(pfd4[1]);
		close(pfd3[0]);
		close(pfd4[0]);
		close(pfd1[1]);
		close(pfd2[0]);
		dup2(pfd1[0], 0);
		dup2(pfd2[1], 1);
		if (execl(argv[1], "player1", NULL) == -1)
		{
			perror("exec failed");
			exit(-1);
		}
	}
	pid_t pid2 = fork();
	if (pid2 < 0)
	{
		perror("fork failed");
		exit(-1);
	}
	if (!pid2)
	{
		close(pfd1[1]);
		close(pfd2[1]);
		close(pfd1[0]);
		close(pfd2[0]);
		close(pfd3[1]);
		close(pfd4[0]);
		dup2(pfd3[0], 0);
		dup2(pfd4[1], 1);
		if(execl(argv[2], "player2", NULL) == -1){
			perror("exec failed");
			exit(-1);
		}
	}
	close(pfd2[1]);
	close(pfd4[1]);
	for (int i = 0; i < 10; i++)
	{
		write(pfd1[1], str, 3);
		write(pfd3[1], str, 3);
		char out1[2], out2[2];
		if ((read(pfd2[0], out1, 2) != -1) && (read(pfd4[0], out2, 2) != -1))
		{
			if (out1[0] == out2[0])
				continue;
			if (out1[0] == '0')
			{
				if (out2[0] == '2')
					fp++;
				else if (out2[0] == '1')
					sp++;
			}
			else if (out1[0] == '1')
			{
				if (out2[0] == '0')
					fp++;
				else if (out2[0] == '2')
					sp++;
			}
			else if (out1[0] == '2')
			{
				if (out2[0] == '1')
					fp++;
				else if (out2[0] == '0')
					sp++;
			}
		}
	}
	close(pfd1[1]);
	close(pfd3[1]);
	close(pfd2[0]);
	close(pfd2[0]);
	printf("%d %d", fp, sp);
	return 0;
}
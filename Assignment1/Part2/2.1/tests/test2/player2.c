#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <errno.h>

#define BUFSIZE 	8 
#define GO_MSG_SIZE 	3 
#define STDIN 		0
#define STDOUT 		1

#define ROCK 		0 
#define PAPER 		1 
#define SCISSORS	2 
#define TOTAL_MOVES	3



int err(char *str, int errorCode)
{
	dprintf(2, "%s\n", str);
	close(STDIN);
	close(STDOUT);
	exit(errorCode);
}

void chooseMove(char *move)
{
    /* I hate playing scissors */
	static unsigned int i = 0;
	++i;
    if (i%2) {
        *move = '0'; // ROCK
    }
    else {
        *move = '0' + 1; // PAPER
    }
}

int main()
{
	int n = 0;
	int seed = time(0);	
	char *goMsg = "GO";
	int goMsgLen = 0;
        char goBuf[BUFSIZE];
	char move = '\0';
	char oppMove = '\0';

	//initial housekeeping
	seed = abs(seed*seed*getpid());
	srand(seed);
	goMsgLen = strlen(goMsg);



	//Play each round
	while(-1 != (n = read(STDIN, goBuf, goMsgLen+1)))
	{
		//end of game 
		if(n == 0)
		{
			break;
		}

		//check whether the go ahead message received is correct or not.
		if(0 != (n = strncmp(goBuf, goMsg, goMsgLen)))
			err("GO message is incorrect!.", -1);


		//make move
		chooseMove(&move);
		if(-1 == (n = write(STDOUT, &move, 1)))
			err("Failed to make move.\n", -1);

		
		//read opponents move 
		//if(-1 == (n = read(STDIN, &oppMove, 1)))
		//	err("Failed to read the opponents move of the round.", -1);	
		//dprintf(2, "read: %d bytes, opponents move: %c\n", n, oppMove);

			
	}

	if(n == -1)
		err("Failed to read GO message.\n", -1);


	printf("child (%d) is terminating.\n", getpid());
	close(STDOUT);

	return 0;
}


#include "gameUtils.h"
#include <stdlib.h>


int getResult(int myMove, int theirMove) {
    if (myMove==ROCK && theirMove==ROCK) {
	    return TIE;
    }
    if (myMove==ROCK && theirMove==PAPER) {
	    return LOSE;
    }
    if (myMove==ROCK && theirMove==SCISSORS) {
	    return WIN;
    }
    if (myMove==PAPER && theirMove==ROCK) {
	    return WIN;
    }
    if (myMove==PAPER && theirMove==PAPER) {
	    return TIE;
    }
    if (myMove==PAPER && theirMove==SCISSORS) {
	    return LOSE;
    }
    if (myMove==SCISSORS && theirMove==ROCK) {
	    return LOSE;
    }
    if (myMove==SCISSORS && theirMove==PAPER) {
	    return WIN;
    }
    if (myMove==SCISSORS && theirMove==SCISSORS) {
	    return TIE;
    }
    return -1; // INVALID MOVE!
}

int playRound(int roundNumber, int player1, int player2, int outPlayer[], int inPlayer[]) {
    char player1Move;
    char player2Move;
	int res = 0;
    static char startMsg[] = {'G', 'O', '\0'};

	fprintf(stderr, "playRound() Called!\n");

    // fprintf(outPlayer[player1], "GO\n");
    // fflush(outPlayer[player1]);
	/*static int i = 0;
	if(!i)
	{
		for(int i =0 ; i< 10; i++)
		{
			write(outPlayer[player1], startMsg, 3);
		}
		i = 1;
	}*/

    res = write(outPlayer[player1], startMsg, 3);
	fprintf(stderr, "write() returned: %d\n", res);
	if(res == -1)
	{
		fprintf(stderr, "Failed to send GO message to player1!\n");
		exit(-1);
	}

 
   res = write(outPlayer[player2], startMsg, 3);
	fprintf(stderr, "write() returned: %d\n", res);
	if(res == -1)
	{	
		fprintf(stderr, "Failed to send GO message to player2!\n");
		exit(-1);
	}
 

    // fprintf(outPlayer[player2], "GO\n");
    // fflush(outPlayer[player2]);


    // fscanf(inPlayer[player1], "%d", &player1Move);
    // fscanf(inPlayer[player2], "%d", &player2Move);
    res = read(inPlayer[player1], &player1Move, 1);
	fprintf(stderr, "read() returned: %d\n", res);
	if(res <= 0)
	{
		fprintf(stderr, "Failed to read move from player1!\n");
		exit(-1);
	}
    res = read(inPlayer[player2], &player2Move, 1);
	fprintf(stderr, "read() returned: %d\n", res);
	if(res <= 0)
	{
		fprintf(stderr, "Failed to read move from player2!\n");
		exit(-1);
	}

    // printf("Round %d: %d and %d\n", roundNumber, player1Move, player2Move);
    int result = getResult(player1Move-'0', player2Move-'0');
	fprintf(stderr, "[Inside playRound]result: %d\n", result);
    return result;
}

void playMatch(
    int player1,
    int player2,
    char** programNames,
    int numRounds,
    int outPlayer[],
    int inPlayer[],
    int* player1score,
    int* player2score,
    int* numTies) {

	fprintf(stderr, "playMatch() Called!\n");
    for (int i=0; i<numRounds; i++) {
		int result = playRound(i+1, player1, player2, outPlayer, inPlayer);
		fprintf(stderr, "[Inside playMatch]result: %d\n", result);
		if (result == WIN) {
			(*player1score)++;
		}
		else if (result == LOSE) {
			(*player2score)++;
		}
		else if (result == TIE) {
			(*numTies)++;
		}
		else {
			fprintf(stderr,"Invalid moves!.\n");
			return;
		}
    }
}

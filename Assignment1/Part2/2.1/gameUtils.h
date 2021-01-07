#include <stdio.h>
#include <unistd.h>

#define ROCK 0
#define PAPER 1
#define SCISSORS 2

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define LOSE 0
#define WIN 1
#define TIE 2

#define READ 0
#define WRITE 1

int getResult(int myMove, int theirMove);

int playRound(int roundNumber, int player1, int player2, int outPlayer[], int inPlayer[]);


void playSuddenDeath(
    int player1,
    int player2,
    char** programNames,
    int numRounds,
    int outPlayer[],
    int inPlayer[],
    int* player1score,
    int* player2score,
    int* numTies);


void playMatch(
    int player1,
    int player2,
    char** programNames,
    int numRounds,
    int outPlayer[],
    int inPlayer[],
    int* player1score,
    int* player2score,
    int* numTies);

int chooseNextPair(int* player1, int* player2, int prevPlayer1, int prevPlayer2, int players, int lost[]);

int max(int a, int b);
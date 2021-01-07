CC = gcc
CFLAGS = -g

umpire: umpire.c gameUtils.c player.c
		$(CC) $(CFLAGS) -o umpire umpire.c gameUtils.c
		$(CC) $(CFLAGS) -o player1 player.c
		$(CC) $(CFLAGS) -o player2 player.c

test: 	umpire.c tests/test1/player1.c tests/test1/player2.c tests/test2/player1.c tests/test2/player2.c
		$(CC) $(CFLAGS) -o ./tests/test1/player1 ./tests/test1/player1.c
		$(CC) $(CFLAGS) -o ./tests/test1/player2 ./tests/test1/player2.c
		$(CC) $(CFLAGS) -o ./tests/test2/player1 ./tests/test2/player1.c
		$(CC) $(CFLAGS) -o ./tests/test2/player2 ./tests/test2/player2.c
		$(CC) $(CFLAGS) -o ./tests/test3/player1 ./tests/test3/player1.c
		$(CC) $(CFLAGS) -o ./tests/test3/player2 ./tests/test3/player2.c
		$(CC) $(CFLAGS) -o ./tests/test4/player1 ./tests/test4/player1.c
		$(CC) $(CFLAGS) -o ./tests/test4/player2 ./tests/test4/player2.c
		$(CC) $(CFLAGS) -o ./tests/test5/player2 ./tests/test5/player2.c
		$(CC) $(CFLAGS) -o ./tests/test6/player1 ./tests/test6/player1.c
		$(CC) $(CFLAGS) -o ./tests/test7/player1 ./tests/test7/player1.c
		$(CC) $(CFLAGS) -o ./tests/test7/player2 ./tests/test7/player2.c
		$(CC) $(CFLAGS) -o ./tests/test8/player1 ./tests/test8/player1.c
		$(CC) $(CFLAGS) -o ./tests/test8/player2 ./tests/test8/player2.c
		$(CC) $(CFLAGS) -o ./tests/test9/player1 ./tests/test9/player1.c
		$(CC) $(CFLAGS) -o ./tests/test9/player2 ./tests/test9/player2.c
clean:
		rm player1 player2 umpire ./tests/test1/player1 ./tests/test1/player2 ./tests/test2/player1 ./tests/test2/player2 \
		./tests/test3/player1 ./tests/test3/player2 ./tests/test4/player1 ./tests/test4/player2 \
		./tests/test5/player2 ./tests/test6/player1 ./tests/test7/player1 ./tests/test7/player2 \
		./tests/test8/player1 ./tests/test8/player2 \
		./tests/test9/player1 ./tests/test9/player2 

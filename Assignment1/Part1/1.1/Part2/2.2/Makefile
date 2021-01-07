 
CC = gcc
CFLAGS = -g -o

umpire: umpire2.c player.c gameUtils.c
		$(CC) $(CFLAGS) umpire2 umpire2.c gameUtils.c
		$(CC) $(CFLAGS) player1 player.c
		$(CC) $(CFLAGS) player2 player.c

test: 	tests/test1/player1.c tests/test1/player2.c tests/test2/player1.c tests/test2/player2.c
		$(CC) $(CFLAGS) ./tests/test1/player1 ./tests/test1/player1.c
		$(CC) $(CFLAGS) ./tests/test1/player2 ./tests/test1/player2.c
		$(CC) $(CFLAGS) ./tests/test2/player1 ./tests/test2/player1.c
		$(CC) $(CFLAGS) ./tests/test2/player2 ./tests/test2/player2.c
		$(CC) $(CFLAGS) ./tests/test3/player1 ./tests/test3/player1.c
		$(CC) $(CFLAGS) ./tests/test3/player2 ./tests/test3/player2.c
		$(CC) $(CFLAGS) ./tests/test3/player3 ./tests/test3/player3.c
		$(CC) $(CFLAGS) ./tests/test4/player1 ./tests/test4/player1.c
		$(CC) $(CFLAGS) ./tests/test4/player2 ./tests/test4/player2.c
		$(CC) $(CFLAGS) ./tests/test4/player3 ./tests/test4/player3.c
		$(CC) $(CFLAGS) ./tests/test5/player1 ./tests/test5/player1.c
		$(CC) $(CFLAGS) ./tests/test6/player1 ./tests/test6/player1.c
		$(CC) $(CFLAGS) ./tests/test6/player2 ./tests/test6/player2.c
		$(CC) $(CFLAGS) ./tests/test7/player1 ./tests/test7/player1.c
		$(CC) $(CFLAGS) ./tests/test7/player2 ./tests/test7/player2.c
		$(CC) $(CFLAGS) ./tests/test8/player1 ./tests/test8/player1.c
		$(CC) $(CFLAGS) ./tests/test8/player2 ./tests/test8/player2.c
		$(CC) $(CFLAGS) ./tests/test9/player1 ./tests/test9/player1.c
		$(CC) $(CFLAGS) ./tests/test9/player2 ./tests/test9/player2.c
		$(CC) $(CFLAGS) ./tests/test10/player1 ./tests/test10/player1.c
		$(CC) $(CFLAGS) ./tests/test10/player2 ./tests/test10/player2.c
		$(CC) $(CFLAGS) ./tests/test11/player1 ./tests/test11/player1.c
		$(CC) $(CFLAGS) ./tests/test11/player2 ./tests/test11/player2.c
		$(CC) $(CFLAGS) ./tests/test12/player1 ./tests/test12/player1.c
		$(CC) $(CFLAGS) ./tests/test12/player2 ./tests/test12/player2.c

clean:
		rm umpire2 player1 player2 ./tests/test1/player1 ./tests/test1/player2 ./tests/test2/player1 ./tests/test2/player2 \
		./tests/test3/player1 ./tests/test3/player2 ./tests/test3/player3 \
		./tests/test4/player1 ./tests/test4/player2 ./tests/test4/player3 \
		./tests/test5/player1 \
		./tests/test6/player1 ./tests/test6/player2 \
		./tests/test7/player1 ./tests/test7/player2 ./tests/test7/straceOut \
		./tests/test8/player1 ./tests/test8/player2 ./tests/test8/straceOut ./tests/test8/temp\
		./tests/test9/player1 ./tests/test9/player2 ./tests/test9/straceOut ./tests/test9/temp\
		./tests/test10/player1 ./tests/test10/player2 ./tests/test10/straceOut ./tests/test10/numExec\
		./tests/test11/player1 ./tests/test11/player2 ./tests/test11/straceOut ./tests/test11/straceGO\
		./tests/test12/player1 ./tests/test12/player2 ./tests/test11/straceOut 


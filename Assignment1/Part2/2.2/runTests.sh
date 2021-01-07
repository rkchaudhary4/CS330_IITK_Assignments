#!/usr/bin/env bash


test () {
    ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> tests/test$1/debug.txt
    diff tests/test$1/output.txt tests/test$1/expected.txt
    if [[ $? -eq 0 ]]
    then
        echo "Test $1 success"
    else
        echo "Test $1 failed"
    fi
    #rm tests/test$1/output.txt
}

exit_test () {
    ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> tests/test$1/debug.txt
	#ret=$?
    if [[ $? -eq 255 ]]
    then
        echo "Exit Test $1 success"
    else
        echo "Exit Test $1 failed"
    fi
	
}

play_match () {
    gcc -D MOVE_FILE=1 -o tests/moves/player1 tests/moves/player.c
    gcc -D MOVE_FILE=2 -o tests/moves/player2 tests/moves/player.c
    gcc -D MOVE_FILE=3 -o tests/moves/player3 tests/moves/player.c
    ./umpire2 tests/moves/players$1.txt > tests/moves/output.txt 2> tests/moves/debug_umpire2.txt
    diff tests/moves/output.txt tests/moves/expected$1.txt
    if [[ $? -eq 0 ]]
    then
        echo "Test match $1 success"
    else
        echo "Test match $1 failed"
	#mv tests/moves/debug_umpire2.txt tests/moves/debug_umpire2_failed.txt
	#exit
    fi
    # rm tests/moves/player1
    # rm tests/moves/player2
}


play_match_walkover () {
    rm umpire2
    gcc -o umpire2 umpire2.c gameUtils1.c # use different walkover function

    gcc -D MOVE_FILE=1 -o tests/moves/player1 tests/moves/player.c
    gcc -D MOVE_FILE=2 -o tests/moves/player2 tests/moves/player.c
    gcc -D MOVE_FILE=3 -o tests/moves/player3 tests/moves/player.c
    ./umpire2 tests/moves/players$1.txt > tests/moves/output.txt 2> tests/moves/debug.txt
    diff tests/moves/output.txt tests/moves/expected$1.txt
    if [[ $? -eq 0 ]]
    then
        echo "Test match walkover $1 success"
    else
        echo "Test match walkover $1 failed"
    fi
    # rm tests/moves/player1
    # rm tests/moves/player2
}


wait_test () {
    strace -c -o tests/test$1/straceOut ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> /dev/null
    grep -nrw "wait4" tests/test$1/straceOut > /dev/null
	#grep on successfull search returns: 0
	#grep on unsuccessfull search returns: 1
    if [[ $? -eq 0 ]]
    then
        echo "wait test $1 success"
    else
        echo "wait test $1 failed"
    fi
}

exec_test () {
    strace -cf -o tests/test$1/straceOut ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> /dev/null
    grep "execve" tests/test$1/straceOut  | cut  -c40-50 > tests/test$1/numExec #since we used -c40-50,this means numExec contains 10(11?) chars (can be blank)
    diff tests/test$1/numExec tests/test$1/expectedNumExec
	#grep on successfull search returns: 0
	#grep on unsuccessfull search returns: 1
    if [[ $? -eq 0 ]]
    then
        echo "exec test $1 success"
    else
        echo "exec test $1 failed"
    fi

}


fork_test () {
    strace -cf -o tests/test$1/straceOut ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> /dev/null
    totalForks=$(grep "clone" tests/test$1/straceOut  | cut  -c40-41) #If we use -c40-50,this means numExec contains 10(11?) chars (can be blank)
	#echo $totalForks
    if [[ $totalForks -ge 7 ]]
    then
        echo "fork test $1 success"
    else
        echo "fork test $1 failed"
    fi
}




alt_moves_test (){

    	strace -f -e write -o tests/test$1/straceOut ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> /dev/null
    	grep -o ".*write.*GO.*" tests/test$1/straceOut | cut -b 7- | cut -d' ' -f1,2 > tests/test$1/straceGO
	#echo "Checkout straceGO"

	totalLines=$(wc -l tests/test$1/straceGO| cut -d' ' -f1)
	#echo $totalLines
	lastLine=2
	firstLine=1
	result=1
	while [ $lastLine -le $totalLines ]; do
		#echo $(head -$lastLine straceGO | tail +$firstLine)
		numLines=$(head -$lastLine tests/test$1/straceGO | tail --line +$firstLine | uniq | wc -l)
		#echo $numLines
		if [[ $numLines == 1 ]]; then
			echo "Alternate moves test $1 failed"
			#echo $(head -$lastLine tests/test$1/straceGO | tail +$firstLine)
			result=0
			break
		fi

		((lastLine++))
		((firstLine++))
	done

	if [[ $result == 1 ]]; then
		echo "Alternate moves test $1 success"
	fi

}


termination_test (){

    	strace -f -o tests/test$1/straceOut ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> /dev/null
	firstExit=$(grep -n -o ".*exited with 0.*" tests/test$1/straceOut | cut -d':' -f1 | head -n 1)
	tail --line +$firstExit tests/test$1/straceOut > tests/test$1/temp
    	goMsgsSent=$(grep -o ".*write.*GO.*" tests/test$1/temp | wc -l)
	if [[ $goMsgsSent -ge 60 ]]; then	#No. of messages sent after 1st level, dependent upon players.txt used by us in this test case
		echo "Termination test $1 success"
	else
		echo "Termination test $1 failed"
	fi

}

wait_termination_test (){

    	strace -f -o tests/test$1/straceOut ./umpire2 tests/test$1/players.txt > tests/test$1/output.txt 2> /dev/null
	firstWait=$(grep -n -o "wait4" tests/test$1/straceOut | cut -d':' -f1 | head -n 1)
	tail --line +$firstWait tests/test$1/straceOut > tests/test$1/temp
    	goMsgsSent=$(grep -o ".*write.*GO.*" tests/test$1/temp | wc -l)
	if [[ $goMsgsSent -ge 60 ]]; then	#No. of messages sent after 1st level, dependent upon players.txt used by us in this test case
		echo "Wait Plus Termination test $1 success"
	else
		echo "Wait Plus Termination test $1 failed"
	fi

}


make
make test

test 1
test 2
test 3
test 4
# exit_test 5
exit_test 6
wait_test 7
termination_test 8 
wait_termination_test 9 
exec_test 10
alt_moves_test 11
fork_test 12

play_match 1 
play_match 2
play_match 3
play_match 4
# play_match 5
play_match 6

play_match_walkover 7

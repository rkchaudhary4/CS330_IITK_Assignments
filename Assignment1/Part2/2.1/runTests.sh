#!/usr/bin/env bash


test () {
    ./umpire tests/test$1/player1 tests/test$1/player2 > tests/test$1/output.txt 2> tests/test$1/debug.txt
    diff tests/test$1/output.txt tests/test$1/expected.txt
    if [[ $? -eq 0 ]]
    then
        echo "Test $1 success"
    else
        echo "Test $1 failed"
    fi
    rm tests/test$1/output.txt
}

exit_test1 () {
    ./umpire tests/test$1/garbage tests/test$1/player2 > tests/test$1/output.txt 2> tests/test$1/debug.txt
	#ret=$?
    if [[ $? -eq 255 ]]
    then
        echo "Exit Test $1 success"
    else
        echo "Exit Test $1 failed"
    fi
	
}

exit_test2 () {
    ./umpire tests/test$1/player1 tests/test$1/garbage > tests/test$1/output.txt  2> tests/test$1/debug.txt
	#ret=$?
    if [[ $? -eq 255 ]]
    then
        echo "Exit Test $1 success"
    else
        echo "Exit Test $1 failed"
    fi
	
}




play_match () {
    gcc -D MOVE_FILE=$1 -o tests/moves/player1 tests/moves/player.c
    gcc -D MOVE_FILE=$2 -o tests/moves/player2 tests/moves/player.c
    # ./umpire_oracle tests/moves/player1 tests/moves/player2 > tests/moves/expected$3.txt 2> tests/test$1/debug.txt
    ./umpire tests/moves/player1 tests/moves/player2 > tests/moves/output.txt 2> tests/test$1/debug.txt
    diff tests/moves/output.txt tests/moves/expected$3.txt
    if [[ $? -eq 0 ]]
    then
        echo "Test match $1 vs $2 success"
    else
        echo "Test match $1 vs $2 failed"
    fi
    # rm tests/moves/player1
    # rm tests/moves/player2
}


exec_test () {
    strace -cf -o tests/test$1/straceOut ./umpire tests/test$1/player1 tests/test$1/player2 > tests/test$1/output.txt 2> tests/test$1/debug.txt
    grep "execve" tests/test$1/straceOut  | cut  -c40-41 > tests/test$1/numExec #If we use -c40-50,this means numExec contains 10(11?) chars (can be blank)
    diff tests/test$1/numExec tests/test$1/expectedNumExec
    if [[ $? -eq 0 ]]
    then
        echo "exec test $1 success"
    else
        echo "exec test $1 failed"
    fi

}

fork_test () {
    strace -cf -o tests/test$1/straceOut ./umpire tests/test$1/player1 tests/test$1/player2 > tests/test$1/output.txt 2> tests/test$1/debug.txt
    totalForks=$(grep "clone" tests/test$1/straceOut  | cut  -c40-41) #If we use -c40-50,this means numExec contains 10(11?) chars (can be blank)
	#echo $totalForks
    if [[ $totalForks -ge 2 ]]
    then
        echo "fork test $1 success"
    else
        echo "fork test $1 failed"
    fi
}



alt_moves_test (){

    	strace -f -e write -o tests/test$1/straceOut ./umpire tests/test$1/player1 tests/test$1/player2 > tests/test$1/output.txt 2> tests/test$1/debug.txt
    	grep -o ".*write.*GO.*" tests/test$1/straceOut | cut -b 7- | cut -d' ' -f1,2 > tests/test$1/straceGO
	#echo "Checkout straceGO"

	totalLines=$(wc -l tests/test$1/straceGO| cut -d' ' -f1)
	#echo $totalLines
	lastLine=2
	firstLine=1
	result=1
	while [ $lastLine -le $totalLines ]; do
		#echo $(head -$lastLine tests/test$1/straceGO | tail +$firstLine)
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


make
make test

test 1
test 2
test 3
test 4
exit_test1 5
exit_test2 6
exec_test  7
alt_moves_test 8 
fork_test 9

play_match 1 2 1
play_match 2 3 2
play_match 4 5 3
play_match 8 9 4
play_match 6 7 5

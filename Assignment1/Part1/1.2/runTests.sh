#!/usr/bin/env bash

test () {
	# remove old output file
	[ -f outputs/output$1.txt ] && rm outputs/output$1.txt
	
	./executeInParallel inputs/input$1.txt outputs/output$1.txt 
	diff outputs/output$1.txt tests/expected_outputs/expected_output$1.txt > /dev/null
	
	if [[ $? -eq 0 ]]
	then
		echo "TEST $1 PASSED"
	else
		echo "TEST $1 FAILED"
	fi
}

# set the environment variable
export CS330_PATH="tests/bin:tests/bin/sbin:tests/bin/ubin"

# cleanup old executable 
[ -f executeInParallel ] && rm executeInParallel

# compile
gcc -o executeInParallel executeInParallel.c

# tests
test 1 
test 2 
test 3 
test 4 

# options
test 5
# execute the first found
test 6
# no execute permission
test 7
# many commands
test 8

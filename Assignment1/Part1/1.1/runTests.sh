#!/usr/bin/env bash

test () {

	# Remove old output files if any
	[ -f output$1.txt ] && rm output$1.txt
	
	# Execute the file and save the printed output.
	./executeCommand "$2" > output_org$1.txt
	
	# Added by Shiv on 14 Oct 2020
	# Process the output file for multiple spaces and '\n' character.
	# First convert all letters to upper case then remove extra spaces.
	cat output_org$1.txt | tr [a-z] [A-Z] | tr -s [:blank:] ' ' > output$1.txt
	rm output_org$1.txt
	
	# Compare the content of expected and generated output file.
	./compare "output$1.txt" "tests/expected_outputs/expected_output$1.txt"
	
	# If both are equal the test is passed.
	if [[ $? -eq 0 ]]
	then
		echo "TEST $1 PASSED"
	else
		echo "TEST $1 FAILED"
	fi
	
}

# Set the environment variable
export CS330_PATH="tests/bin:tests/bin/sbin:tests/bin/ubin"

# Cleanup old executable 
[ -f executeCommand ] && rm executeCommand
[ -f compare ] && rm compare

# Compile
gcc -o executeCommand executeCommand.c
g++ -o compare compare.cpp

# Tests added by Shiv on 14 Oct 2020.
test 1 "ls_shiv"
test 2 "grep_shiv"
test 3 "runc"
test 4 "grep_shiv -b"
test 5 "grep_shiv hdchgc"
test 6 "runc -arg"
test 7 "unknwn"
test 8 "grep_shiv -a"

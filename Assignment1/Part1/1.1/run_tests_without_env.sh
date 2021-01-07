#!/usr/bin/env bash

# Added by Shiv on 14 Oct 2020

test () {

	# Remove old output files if any
	[ -f output_without_env$1.txt ] && rm output_without_env$1.txt
	
	# Generate the output file.
	./executeCommand "$2" > output_without_env_org$1.txt
	
	# Process the output file for multiple spaces and lower case character.
	# First convert all letters to upper case then remove extra spaces.
	cat output_without_env_org$1.txt | tr [a-z] [A-Z] | tr -s [:blank:] ' ' > output_without_env$1.txt
	rm output_without_env_org$1.txt
	
	# Compare the content of expected and generated output file.
	./compare "output_without_env$1.txt" "tests/expected_outputs/expected_output_unable_to_execute.txt"
	
	# If both are equal the test is passed.
	if [[ $? -eq 0 ]]
	then
		echo "TEST $1 PASSED"
	else
		echo "TEST $1 FAILED"
	fi
	
}

# cleanup old executable 
[ -f executeCommand ] && rm executeCommand
[ -f compare ] && rm compare

# compile
gcc -o executeCommand executeCommand.c
g++ -o compare compare.cpp

# Tests
test 1 "grep"
test 2 "ls_shiv"
test 3 "abcd"

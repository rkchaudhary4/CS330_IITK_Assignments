#!/usr/bin/env bash

# remove old executables
[ -f ls ] && rm ls
[ -f grep ] && rm grep
[ -f ps ] && rm ps
[ -f echo ] && rm echo

# compile 
gcc -o ls ls.c
gcc -o grep grep.c
gcc -o ps ps.c
gcc -o echo echo.c

# clean the old directory structure
[ -f ../bin/ps ] && rm ../bin/ps
[ -f ../bin/ls ] && rm ../bin/ls
[ -f ../bin/sbin/ps ] && rm ../bin/sbin/ps
[ -f ../bin/sbin/grep ] && rm ../bin/sbin/grep
[ -f ../bin/ubin/grep ] && rm ../bin/ubin/grep

# copy to the correct directory structure
cp ls ../bin/ls
touch ../bin/ps
touch ../bin/echo
touch ../bin/noexec
cp ps ../bin/sbin/ps
cp echo ../bin/sbin/echo
touch ../bin/sbin/grep
touch ../bin/sbin/noexec
cp grep ../bin/ubin/grep
# for execute first found test
cp ps ../bin/ubin/echo
touch ../bin/ubin/noexec

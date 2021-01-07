#!/usr/bin/env bash

# remove old executables
[ -f ls_shiv ] && rm ls_shiv
[ -f grep_shiv ] && rm grep_shiv
[ -f ps_shiv ] && rm ps_shiv

# compile 
gcc -o ls_shiv ls.c
gcc -o grep_shiv grep.c
gcc -o ps_shiv ps.c

# clean the old directory structure
[ -f ../bin/ps_shiv ] && rm ../bin/ps_shiv
[ -f ../bin/ls_shiv ] && rm ../bin/ls_shiv
[ -f ../bin/sbin/ps_shiv ] && rm ../bin/sbin/ps_shiv
[ -f ../bin/sbin/grep_shiv ] && rm ../bin/sbin/grep_shiv
[ -f ../bin/ubin/grep_shiv ] && rm ../bin/ubin/grep_shiv

# copy to the correct directory structure
cp ls_shiv ../bin/ls_shiv
touch ../bin/ps_shiv
touch ../bin/runc
cp ps_shiv ../bin/sbin/ps_shiv
touch ../bin/sbin/grep_shiv
cp grep_shiv ../bin/ubin/grep_shiv

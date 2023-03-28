#!/bin/sh

# Usage: myclock [outfile]
#        If outfile is specified, the script first removes "outfile" then iterates
#	 appending the output of "date" to "outfile" (or stdout, if "outfile" is omitted),
#	 then sleeps for "interval".
#
# Running:  From your BASH prompt, either
#           1. run "sh myclock outfile", or
#           2. make "myclock" executable by issuing the shell command  "chmod +x myclock"
#	       then run "./myclock outfile", or
#	    3. (a) if the current directory "." is not already part of your search PATH then
#	           edit file "~/.bashrc" and add a new line "PATH=$PATH\:.",
#	       (b) use "source ~/.bashrc" to read the new BASH configuration
#	       (c) make "myclock" executable, as in (2) above, and
#	       (d) run "myclock outfile"


    env echo "myclock [pid= $$] [$1]"
    interval=2
    case $1 in
    "")   while true; do date; sleep $interval; done;;
    *)    rm -f $1;
    	  while true; do date >> $1; sleep $interval; done;;
    esac	  

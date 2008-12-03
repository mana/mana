#!/bin/bash
#automated script to update aethyra from git
#author=Blame <blame@aethyra.com>
#last modified 2008-12-03 Tametomo
file=~/src/aethyra
file1=~/src
# test for the aethyra client folder 
if [ -e $file ]; then
	#change to the aethyra directory
	cd $file;
	#issue the git update command
	git pull;
	#issue the autobuild command 
	./autobuild.sh;
	echo "Aethyra has been updated"
	exit 1;
else 
	#make directory /home/user-name/src
	mkdir $file1
	#change to directory /home/user-name/src
	cd $file1
	#execute the git command to download the client files
	git clone git://gitorious.org/tmw/aethyra.git
	#issue the auto build script to compile the client
	./autobuild
	echo "congratulations on installing aethyra enjoy"
	exit 1;
fi

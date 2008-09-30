#!/bin/bash
#automated script to update aethyra from SVN
#author=Blame <blame@aethyra.com>
#last modified 2008-09-21 Blame
file=~/src/aethyra
file1=~/src
# test for the aethyra client folder 
if [ -e $file ]; then
	#change to the aethyra directory
	cd $file;
	#issue the svn update command
	svn up;
	#issue the autobuild command 
	./autobuild.sh;
	echo "Aethyra has been updated"
	exit 1;
else 
	#make directory /home/user-name/src
	mkdir $file1
	#change to directory /home/user-name/src
	cd $file1
	#execute the svn command to download the client files
	svn co svn://www.aethyra.org/client/trunk aethyra
	#issue the auto build script to compile the client
	./autobuild
	echo "congratulations on installing aethyra enjoy"
	exit 1;
fi

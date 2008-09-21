#!/bin/bash
#automated script to update aethyra from SVN
#author=Blame <blame@aethyra.com>
#last modified 2008-09-21 Blame

# test for the aethyra client 
if [ ~/src/aethyra ]; then
	#change to the aethyra directory
	cd ~/src/aethyra;
	#issue the svn update command
	svn up;
	#issue the autobuild command 
	./autobuild.sh;
	echo "Aethyra has been updated"
	exit 1;
else 
	#make directory /home/user-name/src
	mkdir ~/src
	#change to directory /home/user-name/src
	cd ~/src
	#execute the svn command to download the client files
	svn co svn://209.168.213.109/client/trunk aethyra
	#issue the auto build script to compile the client
	./autobuild
	echo "congratulations on installing aethyra enjoy"
	exit 1;
fi

#!/bin/sh
# auto build script based on fedora but usable for other distros
#author=Blame <blame582@gmail.com> 
#last modified date 2008-09-29 Blameu
# right now thes should be ok for testing 
file=/etc/fedora-release
file1=/etc/lsb-release

echo "Generating build information using aclocal, autoheader, automake and autoconf."
echo

# Regerate configuration files
autoreconf -i

echo
echo "Now you are ready to run ./configure"

./configure

echo
echo "now running make"

make

echo
echo "now running make install please input your root password";
# we must test for fedora first otherwise we get an error
if [ -e $file ]; then
	echo "Fedora found";
	su -c "make install";
	su -c "make clean";
	echo "Done";
	aethyra &
	exit 1;
elif [ -e $file1 ];then
	echo "ubuntu found";
	sudo make install;
	aethyra &
	exit 1;
else 
	echo "login as root and run make install"
fi
echo "done"


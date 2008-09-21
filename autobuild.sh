#!/bin/sh
# auto build script based on fedora but usable for other distros
#author=Blame <blame@aethyra.com> 
#last modified date 2008-09-16 Blame


echo "Generating build information using aclocal, autoheader, automake and autoconf."
echo

# Regerate configuration files
aclocal
autoheader
automake --gnu --add-missing --copy
autoconf

echo
echo "Now you are ready to run ./configure"

./configure

echo
echo "now running make"

make

echo
echo "now running make install please input your root password";
if [ /etc/fedora-release ]; then
	echo "Fedora found";
	su -c "make install";
	su -c "make clean";
	echo "Done";
	aethyra &
	exit 1;
elif [ /etc/lsb-release ];then
	echo "ubuntu found";
	sudo make install;
	aethyra &
	exit 1;
else 
	echo "login as root and run make install"
fi
echo "done"


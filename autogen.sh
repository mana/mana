#!/bin/sh

echo "Generating build information using aclocal, autoheader, automake and autoconf."
echo

# Regerate configuration files
aclocal-1.9
autoheader
automake-1.9 --gnu --add-missing --copy
autoconf

echo
echo "Now you are ready to run ./configure"

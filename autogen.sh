#!/bin/sh

echo "Generating build information using aclocal, autoheader, automake and autoconf."
echo

# Regerate configuration files
aclocal-1.7
autoheader
automake-1.7 --gnu --add-missing --copy
autoconf

echo
echo "Now you are ready to run ./configure"

#!/bin/sh

echo "Generating build information using autopoint, aclocal, autoheader, automake, and autoconf."
echo

# Regerate configuration files
autopoint
aclocal
autoheader
automake --gnu --add-missing --copy
autoconf

echo
echo "Now you are ready to run ./configure"

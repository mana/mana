#!/bin/bash
# Copyright (C) 2001-2010 Wormux Team.
# Copyright (C) 2010 The Mana World Development Team.

new_year="$1"
[[ -z $new_year ]] && echo "Missing parameter: year" && exit 1

[[ ! -e src ]] && echo "This script should be ran from the top mana/manaserv dir" && exit 2

tmp_file="w$RANDOM$RANDOM$RANDOM$RANDOM"
[[ -e $tmp_file ]] && tmp_file="w$RANDOM$RANDOM$RANDOM$RANDOM"

# update the dates, creating the interval if it doesn't exist yet
find -iname "*.cpp" -or -iname "*.h" -or -iname "*.hpp" | 
  xargs sed -i "/Copyright.*The Mana Developers/ s,\(20[0-9]*\) \|\(20[0-9]*\)-20[0-9]* ,\1\2-$new_year ,"

# do a semi-automated commit check
git diff > $tmp_file
echo "The next +/- counts mentioning copyrights should match:"
grep "^[-+][^-+]" $tmp_file | sort | uniq -c
echo "If they don't, try finding the offending files with grep -rl <\$bad_line>"

# Remove temp file
[[ -e $tmp_file ]] && rm $tmp_file

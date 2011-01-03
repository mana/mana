#!/bin/sh
find \( -name \*.cpp \
        -o -name \*.h \
        -o -name \*.txt \
        -o -name \*.xml \
        -o -name \*.mana \
     \) ! -wholename \*/CMakeFiles/\* \
        ! -name CMakeCache.txt \
  | sort > mana.files

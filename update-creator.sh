#!/bin/sh
find -path ./libs -prune -path ./CMake -prune \
  -o -name \*.cpp \
  -o -name \*.h \
  -o -name \*.am \
  -o -name \*.txt \
  -o -name \*.xml \
  -o -name \*.mana \
  | sort > mana.files

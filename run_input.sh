#!/bin/bash
FILES=./input/jnh/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done

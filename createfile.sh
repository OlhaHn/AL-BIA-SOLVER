#!/bin/bash
FILES=./input/test_one_folder/*
FILE=output

if [ -f "$FILE" ]; then
    echo "$FILE exist"
else
    touch $FILE
    echo -n "SAT;" >> $FILE
    for f in $FILES
    do
    echo -n "$f;" >> $FILE
    done
fi
echo "" >> $FILE
echo -n "v000001;" >> $FILE
for f in $FILES
do
  #echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f >> $FILE
done
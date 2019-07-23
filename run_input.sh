#!/bin/bash
FILES=./input/test/rand_7_UNSAT/critical_88.4_var_clause_ratio89/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done

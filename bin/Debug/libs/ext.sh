#! /bin/bash
deplist=$( ldd $1 | awk '{if (match($3,"/")){ print $3}}' )
cp $deplist $2

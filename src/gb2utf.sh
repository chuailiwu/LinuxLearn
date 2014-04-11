#!/bin/bash

exts=" cpp mk "

for ext in ${exts}
do

for i in $(find $HOME/csrc/src/ -name "*.$ext")
	do
		echo -e "$i-$ext" 
		iconv -c -f gbk -t utf8 $i -o /tmp/iconv.tmp
		mv /tmp/iconv.tmp $i
	done
done

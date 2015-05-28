#!/bin/bash
#Samuel Weber (994587341)

if [ $# == "0" ]; then
	echo "usage: makemake.sh executable_name"
	exit 1
fi

name=$1
shift
echo -e -n "" > Makefile

echo -e -n "$name :" >> Makefile
for file in *.cpp; do
	echo -e -n "$file" | awk -F. '{printf " " $1 ".o"}' >> Makefile
done
echo -e -n "\n\tg++ -ansi -Wall -o $name -g $* " >> Makefile
for file in *.cpp; do
	echo -e -n "$file" | awk -F. '{printf " " $1 ".o"}' >> Makefile
done
echo -e -n "\n\n" >> Makefile

for file in *.cpp; do
	echo -e -n "$file" | awk -F. '{printf $1 ".o"}' >> Makefile
	echo -e -n " : $file" >> Makefile
	egrep -i '#include[ \t\n]?\"' $file | awk -F\" '{printf " " $2}' >> Makefile
	echo -e -n "\n\tg++ -ansi -Wall -g -c $* $file\n\n" >> Makefile
done

echo -e -n "clean :\n\trm -f $name" >> Makefile
for file in *.cpp; do
	echo -e -n "$file" | awk -F. '{printf " " $1 ".o"}' >> Makefile
done

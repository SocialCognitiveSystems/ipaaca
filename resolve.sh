#!/bin/bash

PACKAGES="rsb"

shopt -q nullglob || resetnullglob=1
shopt -s nullglob
shopt -q dotglob || resetdotglob=1
shopt -s dotglob

mkdir -p deps/bin
mkdir -p deps/lib
mkdir -p deps/include
mkdir -p deps/scripts
mkdir -p deps/python
for P in $PACKAGES; do
	files=(../$P/dist/bin/*);
	[ "$files" ] && cp -a ../$P/dist/bin/* deps/bin/
	files=(../$P/dist/lib/*);
	[ "$files" ] && cp -a ../$P/dist/lib/* deps/lib/
	files=(../$P/dist/include/*);
	[ "$files" ] && cp -a ../$P/dist/include/* deps/include/
	files=(../$P/dist/scripts/*);
	[ "$files" ] && cp -a ../$P/dist/scripts/* deps/scripts/
	files=(../$P/dist/python/*.zip);
	[ "$files" ] && for zipfile in ../$P/dist/python/*.zip; do
		unzip -oqq $zipfile -d deps/python
	done
done

[ "$resetdotglob" ] && shopt -u dotglob
[ "$resetnullglob" ] && shopt -u nullglob


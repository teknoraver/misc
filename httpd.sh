#!/bin/bash

[ "$#" -eq 0 ] && exec nc -klp 8080 -e "$0 serve"

IFS="$IFS"$'\r'

read -r get file http

if ! [ -e ".$file" ]; then
	printf "$http 404 Not found\\r\\n"
elif ! [ -r ".$file" ]; then
	printf "$http 403 Access denied\\r\\n"
elif [ -d ".$file" ]; then
	printf "$http 200 OK\\r\\n\\r\\n"
	(
		cd ".$file"
		for i in *; do
			if [ -d "$i" ]; then
				stat --printf='<a href="%n/">%n/</a><br>' "$i"
			else
				stat --printf='<a href="%n">%n</a>\t%s bytes<br>' "$i"
			fi
		done
	)
else
	printf "$http 200 OK\\r\\n\\r\\n"
	exec cat ".$file"
fi

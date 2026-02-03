#!/bin/bash

if [ "$1" != "__httpd_serve_2f8a9c__" ]; then
	port="${1:-8080}"
	dir="${2:-.}"
	script=$(realpath "$0")
	cd "$dir" || exit 1
	exec ncat -klp "$port" -e "$script __httpd_serve_2f8a9c__"
fi

error_page() {
	printf "$http $1 $2\\r\\n\\r\\n"
	cat <<-EOF
		<html>
		<head><title>$1 $2</title></head>
		<body>
		<center><h1>$1 $2</h1></center>
		</body>
		</html>
	EOF
}

get_mime_type() {
	case "${1,,}" in
		*.txt) mime="text/plain" ;;
		*.html|*.htm) mime="text/html" ;;
		*.css) mime="text/css" ;;
		*.js) mime="text/javascript" ;;
		*.json) mime="application/json" ;;
		*.xml) mime="application/xml" ;;
		*.jpg|*.jpeg) mime="image/jpeg" ;;
		*.png) mime="image/png" ;;
		*.gif) mime="image/gif" ;;
		*.svg) mime="image/svg+xml" ;;
		*.webp) mime="image/webp" ;;
		*.ico) mime="image/x-icon" ;;
		*.pdf) mime="application/pdf" ;;
		*.zip) mime="application/zip" ;;
		*.tar) mime="application/x-tar" ;;
		*.gz) mime="application/gzip" ;;
		*.mp3) mime="audio/mpeg" ;;
		*.m4a) mime="audio/mp4" ;;
		*.mp4) mime="video/mp4" ;;
		*.webm) mime="video/webm" ;;
		*.avi) mime="video/x-msvideo" ;;
		*.mov) mime="video/quicktime" ;;
		*.mkv) mime="video/x-matroska" ;;
		*) mime="application/octet-stream" ;;
	esac
}

IFS="$IFS"$'\r'

read -r get file http

file=$(printf '%b' "${file//%/\\x}")

if ! [ -e ".$file" ]; then
	error_page 404 "Not Found"
elif ! [ -r ".$file" ]; then
	error_page 403 "Forbidden"
elif [ -d ".$file" ]; then
	printf "$http 200 OK\\r\\n"
	printf "Content-Type: text/html\\r\\n\\r\\n"
	cd ".$file"
	cat <<-EOF
		<html>
		<head>
		<title>Index of $file</title>
		<style>
		table { font-family: monospace; }
		td, th { padding: 0 15px; }
		</style>
		</head>
		<body>
		<h1>Index of $file</h1>
		<table border="0">
		<tr><th align="left">Name</th><th align="left">Date</th><th align="left">Size</th></tr>
		<tr><td><a href="../">../</a></td><td>-</td><td>-</td></tr>
	EOF
	for i in *; do
		read mode mtime size < <(stat -c '%f %Y %s' "$i")
		mtime=$(date -d "@$mtime" '+%Y-%m-%d %T')
		if [ -d "$i" ]; then
			printf '<tr><td><a href="%s/">%s/</a></td><td>%s</td><td>-</td></tr>\n' "$i" "$i" "$mtime"
		else
			printf '<tr><td><a href="%s">%s</a></td><td>%s</td><td>%s bytes</td></tr>\n' "$i" "$i" "$mtime" "$size"
		fi
	done
	cat <<-EOF
		</table>
		</body>
		</html>
	EOF
else
	get_mime_type "$file"
	printf "$http 200 OK\\r\\n"
	printf "Content-Type: $mime\\r\\n\\r\\n"
	exec cat ".$file"
fi

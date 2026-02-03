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
		*.txt) echo "text/plain" ;;
		*.html|*.htm) echo "text/html" ;;
		*.css) echo "text/css" ;;
		*.js) echo "text/javascript" ;;
		*.json) echo "application/json" ;;
		*.xml) echo "application/xml" ;;
		*.jpg|*.jpeg) echo "image/jpeg" ;;
		*.png) echo "image/png" ;;
		*.gif) echo "image/gif" ;;
		*.svg) echo "image/svg+xml" ;;
		*.webp) echo "image/webp" ;;
		*.ico) echo "image/x-icon" ;;
		*.pdf) echo "application/pdf" ;;
		*.zip) echo "application/zip" ;;
		*.tar) echo "application/x-tar" ;;
		*.gz) echo "application/gzip" ;;
		*.mp3) echo "audio/mpeg" ;;
		*.m4a) echo "audio/mp4" ;;
		*.mp4) echo "video/mp4" ;;
		*.webm) echo "video/webm" ;;
		*.avi) echo "video/x-msvideo" ;;
		*.mov) echo "video/quicktime" ;;
		*.mkv) echo "video/x-matroska" ;;
		*) echo "application/octet-stream" ;;
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
		mtime=$(date -d "@$(stat -c %Y "$i")" '+%Y-%m-%d %T')
		if [ -d "$i" ]; then
			printf '<tr><td><a href="%s/">%s/</a></td><td>%s</td><td>-</td></tr>\n' "$i" "$i" "$mtime"
		else
			size=$(stat -c %s "$i")
			printf '<tr><td><a href="%s">%s</a></td><td>%s</td><td>%s bytes</td></tr>\n' "$i" "$i" "$mtime" "$size"
		fi
	done
	cat <<-EOF
		</table>
		</body>
		</html>
	EOF
else
	mime=$(get_mime_type "$file")
	printf "$http 200 OK\\r\\n"
	printf "Content-Type: $mime\\r\\n\\r\\n"
	exec cat ".$file"
fi

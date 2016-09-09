package main

import (
	"fmt"
	"github.com/mattn/go-getopt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

var root, _ = os.Getwd()

func sizeToByte(size int64) string {
	const suffixes string = " KMGT"

	for i, s := range suffixes {
		if size < 1024 || i == len(suffixes)-1 {
			return fmt.Sprintf("%d%c", size, s)
		}
		size >>= 10
	}
	return ""
}

func listDir(resp http.ResponseWriter, req *http.Request, path string) {
	files, _ := ioutil.ReadDir(root + path)
	io.WriteString(resp, "<!DOCTYPE html>\n<html><head><title>"+path+"</title></head><body><table style='font-family: monospace;'>\n")
	if path == "/" {
		path = ""
	} else {
		io.WriteString(resp, "<tr><td colspan=\"2\"></td><td><a href=\"..\">../</a></td></tr>\n")
	}
	for _, file := range files {
		name := file.Name()
		if file.Mode()&os.ModeSymlink != 0 {
			link, _ := os.Readlink(root + path + "/" + name)
			name += " -> " + link
		}
		io.WriteString(resp,
			"<tr><td>"+file.Mode().String()+"</td><td>"+sizeToByte(file.Size())+
				`</td><td><a href="`+path+"/"+file.Name()+`">`+name+"</a></td></tr>")
	}
	io.WriteString(resp, "</table></body></html>")
}

func serveStatic(resp http.ResponseWriter, req *http.Request) {
	path := req.URL.Path

	if fi, err := os.Stat(root + path); os.IsNotExist(err) {
		resp.WriteHeader(http.StatusNotFound)
		io.WriteString(resp, path+": "+http.StatusText(http.StatusNotFound))
	} else {
		if file, err := os.Open(root + path); err == nil {
			if fi.IsDir() {
				listDir(resp, req, path)
			} else {
				io.Copy(resp, file)
			}
			file.Close()
		} else if os.IsPermission(err) {
			resp.WriteHeader(http.StatusForbidden)
			io.WriteString(resp, path+": "+http.StatusText(http.StatusForbidden))
		}
	}
}

func main() {
	var address string = ":80"
	for c := 0; c != getopt.EOF; c = getopt.Getopt("b:r:h") {
		switch c {
		case 'b':
			address = getopt.OptArg
		case 'r':
			root = getopt.OptArg
		case 'h':
			log.Printf("usage: httpd [-b address:port] [-r root]")
			os.Exit(0)
		}
	}

	http.HandleFunc("/", serveStatic)
	log.Fatal(http.ListenAndServe(address, nil))
}

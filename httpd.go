package main

import (
	"fmt"
	. "github.com/mattn/go-getopt"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

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
	files, err := ioutil.ReadDir(path)
	switch {
	case os.IsNotExist(err):
		resp.WriteHeader(http.StatusNotFound)
		io.WriteString(resp, path+": not found")
	case os.IsPermission(err):
		resp.WriteHeader(http.StatusForbidden)
		io.WriteString(resp, path+": access denied")
	default:
		io.WriteString(resp, "<html><head></head><body><table>\n")
		if path != "." {
			io.WriteString(resp, "<tr><td colspan=\"2\"></td><td><a href=\"..\">../</a></td></tr>\n")
		}
		for _, file := range files {
			name := file.Name()
			if file.Mode()&os.ModeSymlink != 0 {
				link, _ := os.Readlink(path + "/" + name)
				name += " -> " + link
			}
			res := "<td>" + file.Mode().String() + "</td><td>" + sizeToByte(file.Size()) + "</td>"
			res += `<td><a href="/` + path + "/" + file.Name() + `">` + name + "</a></td>"
			io.WriteString(resp, "<tr>"+res+"</tr>\n")
		}
		io.WriteString(resp, "</table></body><html>")
	}
}

func serveStatic(resp http.ResponseWriter, req *http.Request) {
	path := req.URL.Path[1:]
	if len(path) == 0 {
		path = "."
	}

	switch fi, err := os.Stat(path); {
	case os.IsNotExist(err):
		resp.WriteHeader(http.StatusNotFound)
		io.WriteString(resp, path+": not found")
	case fi.IsDir():
		listDir(resp, req, path)
	default:
		if file, err := os.Open(path); err == nil {
			io.Copy(resp, file)
			file.Close()
		} else if os.IsPermission(err) {
			resp.WriteHeader(http.StatusForbidden)
			io.WriteString(resp, path+": access denied")
		}
	}
}

func main() {
	var port string = "80"
	var root string
	var c int
	OptErr = 0
	for {
		if c = Getopt("p:r:h"); c == EOF {
			break
		}
		switch c {
		case 'p':
			port = OptArg
		case 'r':
			root = OptArg
		case 'h':
			log.Printf("usage: httpd -p port -r root")
			os.Exit(0)
		}
	}

	os.Chdir(root)
	http.HandleFunc("/", serveStatic)
	log.Fatal(http.ListenAndServe(":"+port, nil))
}

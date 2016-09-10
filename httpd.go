package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
)

func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <address:port> <wwwroot>\n", os.Args[0])
		os.Exit(1)
	}
	log.Fatal(http.ListenAndServe(os.Args[1], http.FileServer(http.Dir(os.Args[2]))))
}

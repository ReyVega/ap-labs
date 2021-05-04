// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 227.

// Netcat is a simple read/write client for TCP servers.
package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"os"
)

//!+
func main() {
	// Validations of cmd
	args := os.Args[0:]

	if len(args) != 5 {
		fmt.Println("Usage: go run client.go -user <name> -server localhost:<port>")
		return
	}

	if args[1] != "-user" || args[3] != "-server" {
		fmt.Println("Usage: go run client.go -user <name> -server localhost:<port>")
		return
	}

	conn, err := net.Dial("tcp", args[4])
	if err != nil {
		log.Fatal(err)
	}

	username := flag.String("username", os.Args[2], "username")
	fmt.Fprintf(conn, *username)

	done := make(chan struct{})
	go func() {
		io.Copy(os.Stdout, conn) // NOTE: ignoring errors
		done <- struct{}{}       // signal the main goroutine
	}()
	mustCopy(conn, os.Stdin)
	conn.Close()
	<-done // wait for background goroutine to finish
}

//!-

func mustCopy(dst io.Writer, src io.Reader) {
	if _, err := io.Copy(dst, src); err != nil {
		log.Fatal(err)
	}
}

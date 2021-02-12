package main

import (
	"fmt"
	"os"
)

func main() {
  args := os.Args[1:]
  if len(args) > 0 {
    var msg = ""
    for _, element := range args {
      msg += element + " "
    }
    msg = msg[:len(msg) - 1] + ","
    fmt.Println("Hello " + msg + " Welcome to the jungle")
  } else {
    fmt.Println("Error, no arguments")
  }
}


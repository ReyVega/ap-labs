// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 254.
//!+

// Chat is a server that lets clients chat with each other.
package main

import (
	"bufio"
	"bytes"
	"fmt"
	"log"
	"net"
	"os"
	"strings"
	"time"
)

type User struct {
	username    string
	loggingTime string
	ipAddress   string
	isAdmin     bool
	isKicked    bool
	msgChannel  client
	conn        net.Conn
}

var users = []*User{}

//!+broadcaster
type client chan<- string // an outgoing message channel

var (
	entering = make(chan client)
	leaving  = make(chan client)
	userSent = make(chan client)
	messages = make(chan string)
)

func broadcaster() {
	clients := make(map[client]bool) // all connected clients
	for {
		select {
		case msg := <-messages:
			userWhoSent := <-userSent
			for cli := range clients {
				if cli != userWhoSent {
					cli <- msg
				}
			}
		case cli := <-entering:
			clients[cli] = true

		case cli := <-leaving:
			delete(clients, cli)
			close(cli)
		}
	}
}

func printTerminal(conn net.Conn, text string) {
	fmt.Fprint(conn, text) // NOTE: ignoring network errors
}

//!+handleConn
func handleConn(conn net.Conn) {
	ch := make(chan string) // outgoing client messages
	buffer := make([]byte, 128)
	conn.Read(buffer)
	username := string(bytes.Trim(buffer, "\x00"))

	// Assigning username
	who := username

	go clientWriter(conn, ch, who)

	// Welcome message to the user
	fmt.Printf("irc-server > New connected user [%s]\n", who)
	welcomeMsg := "irc-server > Welcome to the Simple IRC Server\nirc-server > Your user [" + who + "] is successfully logged"

	//Manage if it is the first user to assign host
	// Creating and adding user to users
	dt := time.Now()
	var user User
	if len(users) == 0 {
		fmt.Printf("irc-server > [%s] was promoted as the channel ADMIN\n", who)
		user = User{username: who, loggingTime: dt.Format("01-02-2006 15:04:05"), ipAddress: conn.RemoteAddr().String(), isAdmin: true, isKicked: false, msgChannel: ch, conn: conn}
		users = append(users, &user)
		ch <- welcomeMsg + "\nirc-server > Congrats, you were the first user.\nirc-server > You're the new IRC Server ADMIN"
	} else {
		user = User{username: who, loggingTime: dt.Format("01-02-2006 15:04:05"), ipAddress: conn.RemoteAddr().String(), isAdmin: false, isKicked: false, msgChannel: ch, conn: conn}
		users = append(users, &user)
		ch <- welcomeMsg
	}

	// Sending message to other users about who arrived
	messages <- "\nirc-server > New connected user [" + who + "]"
	userSent <- ch
	entering <- ch

	// Management of messages and commands
	input := bufio.NewScanner(conn)
	for input.Scan() {
		isMsg := true
		// Check if input is different than "" to avoid spam to other users
		if strings.TrimSpace(input.Text()) != "" {

			// Check if it is a command
			msg := strings.Trim(input.Text(), "\r\n")
			args := strings.Split(msg, " ")
			cmd := strings.TrimSpace(args[0])

			switch cmd {
			case "/users":
				viewAllUsers(conn, args)
			case "/msg":
				sendPrivateMsg(conn, args, who)
			case "/time":
				getLocalTime(conn, args)
			case "/user":
				viewSpecificUser(conn, args)
			case "/kick":
				isMsg = kickUser(conn, args, user.isAdmin, who)
			default:
				// Else assume it is a message
				messages <- "\n" + who + " > " + input.Text()
				userSent <- ch
			}
		}
		if isMsg == true {
			// Format of terminal
			printTerminal(conn, who+" > ")
		}
	}

	// Passing ADMIN
	if user.isAdmin && len(users) > 1 {
		users[1].isAdmin = true
		fmt.Fprintf(users[1].conn, "\nirc-server > You're the new IRC Server ADMIN")
		fmt.Printf("irc-server > [%s] was promoted as the channel ADMIN\n", users[1].username)
	}

	// Managing who left the chat or if it was a kick
	leaving <- ch
	if user.isKicked == true {
		fmt.Printf("irc-server > [%s] was kicked\n", who)
		messages <- "\nirc-server > [" + who + "] was kicked from channel for bad language policy violation"
		userSent <- ch
	} else {
		fmt.Printf("irc-server > [%s] left\n", who)
		messages <- "\nirc-server > " + "[" + who + "] " + "left"
		userSent <- ch
	}

	// Close connection and remove user from users
	removeUser(conn)
	conn.Close()

}

func viewAllUsers(conn net.Conn, args []string) {
	if len(args) > 1 {
		fmt.Fprintf(conn, "irc-server > bad command\n")
		return
	}
	if len(users) == 1 {
		fmt.Fprintf(conn, "irc-server > you are the only one connected\n")
		return
	}
	for i := 0; i < len(users); i++ {
		if users[i].ipAddress != conn.RemoteAddr().String() {
			fmt.Fprintf(conn, "irc-server > %s - connected since %s\n", users[i].username, users[i].loggingTime)
		}
	}
}

func sendPrivateMsg(conn net.Conn, args []string, sender string) {
	if len(args) < 3 {
		fmt.Fprintf(conn, "irc-server > bad command\n")
		return
	}
	msg := strings.Join(args[2:], " ")
	for i := 0; i < len(users); i++ {
		if users[i].username == args[1] {
			users[i].msgChannel <- "\n" + sender + " > " + msg
			return
		}
	}
	fmt.Fprintf(conn, "irc-server > user not found\n")
}

func viewSpecificUser(conn net.Conn, args []string) {
	if len(args) != 2 {
		fmt.Fprintf(conn, "irc-server > bad command\n")
		return
	}
	for i := 0; i < len(users); i++ {
		if users[i].username == args[1] {
			fmt.Fprintf(conn, "irc-server > username: %s, IP: %s Connected since: %s\n", users[i].username, users[i].ipAddress, users[i].loggingTime)
			return
		}
	}
	fmt.Fprintf(conn, "irc-server > user not found\n")
}

func getLocalTime(conn net.Conn, args []string) {
	if len(args) > 1 {
		fmt.Fprintf(conn, "irc-server > bad command\n")
		return
	}
	loc, err := time.LoadLocation("America/Mexico_City")
	if err != nil {
		panic(err)
	}
	fmt.Fprintf(conn, "irc-server > Local Time: America/Mexico_City %s\n", time.Now().In(loc).Format("15:04"))
}

func kickUser(conn net.Conn, args []string, isAdmin bool, who string) bool {
	if len(args) != 2 {
		fmt.Fprintf(conn, "irc-server > bad command\n")
		return true
	}
	if isAdmin == false {
		fmt.Fprintf(conn, "irc-server > you are not ADMIN\n")
		return true
	} else {
		for i := 0; i < len(users); i++ {
			if users[i].username == args[1] {
				users[i].isKicked = true
				fmt.Fprint(conn, who+" > ")
				fmt.Fprint(users[i].conn, "\nirc-server > You're kicked from this channel\nirc-server > Bad language is not allowed on this channel\n")
				users[i].conn.Close()
				return false
			}
		}
		fmt.Fprintf(conn, "irc-server > user not found\n")
		return true
	}
}

func clientWriter(conn net.Conn, ch <-chan string, who string) {
	for msg := range ch {
		fmt.Fprintln(conn, msg)
		fmt.Fprint(conn, who+" > ")
	}
}

func removeUser(conn net.Conn) {
	for i := 0; i < len(users); i++ {
		if users[i].ipAddress == conn.RemoteAddr().String() {
			users = append(users[:i], users[i+1:]...)
			break
		}
	}
}

func main() {
	// Validations of cmd
	args := os.Args[0:]

	if len(args) != 5 {
		fmt.Println("Usage: go run server.go -host <name> -port <port>")
		return
	}

	if args[1] != "-host" || args[3] != "-port" {
		fmt.Println("Usage: go run server.go -host <name> -port <port>")
		return
	}

	address := strings.Join([]string{args[2], args[4]}, ":")

	listener, err := net.Listen("tcp", address)
	if err != nil {
		log.Fatal(err)
	}

	fmt.Printf("irc-server > Simple IRC Server started at %s\n", address)
	fmt.Println("irc-server > Ready for receiving new clients")

	go broadcaster()
	for {
		// Waitin for client and setting up his configutation
		conn, err := listener.Accept()
		if err != nil {
			log.Print(err)
			continue
		}
		go handleConn(conn)
	}
}

//!-main

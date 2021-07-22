package main

import (
	"os"
	"golang.org/x/crypto/bcrypt"
)

func main() {
	NEW_PASSWORD := "hallo"
	hashedPassword, hashErr := bcrypt.GenerateFromPassword([]byte(NEW_PASSWORD), bcrypt.DefaultCost)
	file, fileErr := os.OpenFile("../passwd.txt", os.O_CREATE|os.O_WRONLY, 0666)
	file.Truncate(0)
	if hashErr != nil || fileErr != nil {
		panic("Error occurred")
	}
	file.WriteString(string(hashedPassword))
	defer file.Close()
}

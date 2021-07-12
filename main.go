package main

import (
	"html/template"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
	auth "github.com/abbot/go-http-auth"
	"golang.org/x/crypto/bcrypt"
)

type Settings struct {
	Options []string
	Selected string
}

var (
	WarningLogger *log.Logger
	ErrorLogger *log.Logger
	InfoLogger *log.Logger
	cfg = Settings {
		Options: []string {"off", "time", "compact"},
		Selected: "compact",
	}
)

func init() {
	os.Remove("logs.txt")
	file, err := os.OpenFile("logs.txt", os.O_CREATE|os.O_WRONLY, 0666)
	if err != nil {
		log.Println("can not create log file")
	}

	InfoLogger = log.New(file, "INFO:    ", log.Ldate|log.Ltime|log.Lshortfile)
	WarningLogger = log.New(file, "WARNING: ", log.Ldate|log.Ltime|log.Lshortfile)
	ErrorLogger = log.New(file, "ERROR:   ", log.Ldate|log.Ltime|log.Lshortfile)

	InfoLogger.Println("Server started")
}

func Secret(user, realm string) string {
	if user == "jord" {
		content, _ := ioutil.ReadFile("passwd.txt")
		pw := strings.Split(string(content), "\n")[0]
		return pw
	}
	return ""
}

func homePage(w http.ResponseWriter, r *auth.AuthenticatedRequest) {
	tmpl := template.Must(template.ParseFiles("templates/form.tmpl"))

	if r.Method != http.MethodPost {
		tmpl.Execute(w, cfg)
		return
	}

	// TODO: do something with new value
	val := r.FormValue("settings")
	cfg.Selected = val
	InfoLogger.Printf("New mode selected: %s\n", val)

	tmpl.Execute(w, cfg)
}

func logPage(w http.ResponseWriter, r *auth.AuthenticatedRequest) {
	tmpl := template.Must(template.ParseFiles("templates/log.tmpl"))
	content, err := ioutil.ReadFile("logs.txt")
	if err != nil {
		ErrorLogger.Println("can not read log file")
	}
	strParts := strings.Split(string(content), "\n")
	tmpl.Execute(w, strParts)
}

func setPasswordPage(w http.ResponseWriter, r *auth.AuthenticatedRequest) {
	tmpl := template.Must(template.ParseFiles("templates/setpw.tmpl"))
	if r.Method != http.MethodPost {
		tmpl.Execute(w, nil)
		return
	}
	newPw := r.FormValue("password")
	InfoLogger.Println(newPw)
	hashedPassword, hashErr := bcrypt.GenerateFromPassword([]byte(newPw), bcrypt.DefaultCost)
	file, fileErr := os.OpenFile("passwd.txt", os.O_CREATE|os.O_WRONLY, 0666)
	file.Truncate(0)
	if hashErr != nil || fileErr != nil {
		w.Write([]byte("<script>alert('hi');</script>"))
	}
	file.WriteString(string(hashedPassword))
	defer file.Close()
	tmpl.Execute(w, nil)
}

func main() {
	// use this to add the css
	// http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir("static"))))
	authenticator := auth.NewBasicAuthenticator("example.com", Secret)
	http.HandleFunc("/", authenticator.Wrap(homePage))
	http.HandleFunc("/log", authenticator.Wrap(logPage))
	http.HandleFunc("/setpw", authenticator.Wrap(setPasswordPage))
	http.ListenAndServe(":8080", nil)
}

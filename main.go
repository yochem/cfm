package main

import (
	"encoding/json"
	auth "github.com/abbot/go-http-auth"
	"golang.org/x/crypto/bcrypt"
	"html/template"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
)

type Settings struct {
	Options  []string
	Selected string
}

type Config struct {
	Name string
	TL   []bool
}

var (
	WarningLogger *log.Logger
	ErrorLogger   *log.Logger
	InfoLogger    *log.Logger
	cfg           = Settings{
		Options:  []string{"off", "time", "compact"},
		Selected: "compact",
	}
)

func init() {
	file, err := os.OpenFile("logs.txt", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0666)
	if err != nil {
		log.Println("can not create log file")
	}

	InfoLogger = log.New(file, "[I] ", log.Ldate|log.Ltime|log.Lshortfile)
	WarningLogger = log.New(file, "[W] ", log.Ldate|log.Ltime|log.Lshortfile)
	ErrorLogger = log.New(file, "[E] ", log.Ldate|log.Ltime|log.Lshortfile)

	InfoLogger.Println("===============================================")
	InfoLogger.Println("Server started")
}

func Secret(user, _ string) string {
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

func logPage(w http.ResponseWriter, _ *auth.AuthenticatedRequest) {
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
		ErrorLogger.Println("not able to get password, run scripts/newpassword.go")
	}
	file.WriteString(string(hashedPassword))
	defer file.Close()
	tmpl.Execute(w, nil)
}

func receiveSettings(w http.ResponseWriter, r *http.Request) {
	var cfg Config
	err := json.NewDecoder(r.Body).Decode(&cfg)
	if err != nil {
		WarningLogger.Println("could not decode JSON")
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}

	InfoLogger.Printf("Received new settings: %s\n", cfg.Name)
}

func main() {
	authenticator := auth.NewBasicAuthenticator("example.com", Secret)
	http.Handle("/public/", http.StripPrefix("/public/", http.FileServer(http.Dir("./public"))))
	http.HandleFunc("/", authenticator.Wrap(homePage))
	http.HandleFunc("/log", authenticator.Wrap(logPage))
	http.HandleFunc("/setpw", authenticator.Wrap(setPasswordPage))
	http.HandleFunc("/ajax", receiveSettings)
	http.ListenAndServe(":80", nil)
}

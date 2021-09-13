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

type Artwork struct {
	Name     string
	TL       []bool
	InRandom bool
}

type Settings struct {
	Artworks []Artwork
	Selected string
}

type Config struct {
	ArtworksFile string
}

var (
	WarningLogger *log.Logger
	ErrorLogger   *log.Logger
	InfoLogger    *log.Logger
	CFM           Settings
	DATA_FILE     string
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

	loadArtworks("./artworks.json")
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
	tmpl := template.Must(template.ParseFiles("templates/select.tmpl"))

	if r.Method != http.MethodPost {
		tmpl.Execute(w, CFM)
		return
	}

	// TODO: do something with new value
	val := r.FormValue("settings")
	CFM.Selected = val
	InfoLogger.Printf("New mode selected: %s\n", val)
	tmpl.Execute(w, CFM)
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

func receiveNewArtwork(w http.ResponseWriter, r *http.Request) {
	if r.Method == http.MethodGet {
		w.WriteHeader(403)
		w.Write([]byte("403 Forbidden"))
		return
	}

	var art Artwork
	err := json.NewDecoder(r.Body).Decode(&art)
	art.InRandom = true

	if err != nil {
		WarningLogger.Printf("JSON decode error: %s\n", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}

	InfoLogger.Printf("Received new artwork: %s\n", art.Name)
	CFM.Artworks = append(CFM.Artworks, art)
	writeArtworks("./artworks.json")
}

func writeArtworks(filepath string) {
	jsonString, err := json.Marshal(&CFM)
	if err != nil {
		WarningLogger.Printf("json.Marshal error: %s\n", err)
		return
	}

	ioutil.WriteFile(filepath, jsonString, os.ModePerm)
	InfoLogger.Println("wrote new data to JSON file")
}

func loadArtworks(filepath string) {
	fileContent, err := ioutil.ReadFile(filepath)
	if err != nil {
		ErrorLogger.Printf("Not able to read artworks JSON: %s\n", err)
		panic("Not able to read artworks JSON")
	}

	err = json.Unmarshal(fileContent, &CFM)
	if err != nil {
		ErrorLogger.Printf("json.Unmarshal error: %s\n", err)
		panic("error parsing artworks JSON")
	}

	InfoLogger.Println("succesfully loaded in artworks from JSON")
}

func main() {
	authenticator := auth.NewBasicAuthenticator("example.com", Secret)

	// static public files
	http.Handle("/js/", http.StripPrefix("/js/", http.FileServer(http.Dir("./public/js"))))
	http.Handle("/css/", http.StripPrefix("/css/", http.FileServer(http.Dir("./public/css"))))
	http.Handle("/img/", http.StripPrefix("/img/", http.FileServer(http.Dir("./public/img"))))
	http.Handle("/html/", http.StripPrefix("/html/", http.FileServer(http.Dir("./public/html"))))

	http.HandleFunc("/", authenticator.Wrap(homePage))
	http.HandleFunc("/log", authenticator.Wrap(logPage))
	http.HandleFunc("/setpw", authenticator.Wrap(setPasswordPage))
	http.HandleFunc("/ajax", receiveNewArtwork)
	http.ListenAndServe(":80", nil)
}

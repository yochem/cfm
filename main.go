package main

import (
	"encoding/json"
	"errors"
	"math/rand"
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
	"time"

	auth "github.com/abbot/go-http-auth"
	"github.com/huin/goserial"
	"golang.org/x/crypto/bcrypt"
)

type Artwork struct {
	Name     string
	TL       []bool
	InRandom bool
}

type Settings struct {
	Artworks []Artwork
	// default mode will be 'random' which will randomly show artworks where
	// InRandom is true. Other values might be 'time' which will show the time
	// and 'countdown' for NYE.
	Mode              string
	TimeDisplayTime   int // how many seconds the current time is shown during random
	RandomDisplayTime int // how many seconds the random artwork is shown
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
	Serial        io.ReadWriteCloser
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

	const DATA_FILE = "./artworks.json"
	loadArtworks(DATA_FILE)

	arduinoPath, err := FindArduinoDevice()
	if err != nil {
		fmt.Println("Can't find Arduino!")
	}
	serialConfig := &goserial.Config{Name: arduinoPath, Baud: 9600}
	Serial, err = goserial.OpenPort(serialConfig)
	if err != nil {
		fmt.Printf("Serial port not opening: %v\n", err)
		return;
	}

	// needed for establishing the serial connection
	time.Sleep(2 * time.Second)

	CFMToBytes()
	startLoop()
}

/******************************************************************************
ARDUINO PART
*******************************************************************************/
var quit chan struct{}

func sendRandomArtwork(artworks []Artwork) {
	var message strings.Builder
	randomIndex := rand.Intn(len(artworks))
	for _, mode := range artworks[randomIndex].TL {
		if mode {
			message.WriteString("1")
		} else {
			message.WriteString("0")
		}
	}

	Serial.Write([]byte(message.String()))
}

func startLoop() {
	quit = make(chan struct{})
	go loop()
}

func stopLoop() {
	quit <- struct{}{}
}

func loop() {
	// This ticker will put something in its channel every 2s
	ticker := time.NewTicker(2 * time.Second)
	// If you don't stop it, the ticker will cause memory leaks
	defer ticker.Stop()

	artworksInRandom := []Artwork{}

	for i, artwork := range CFM.Artworks {
		if artwork.InRandom {
			artworksInRandom = append(artworksInRandom, CFM.Artworks[i])
		}
	}

	for {
		select {
		case <-quit:
			return
		case <-ticker.C:
			sendRandomArtwork(artworksInRandom)
		}
	}
}

func FindArduinoDevice() (string, error) {
	contents, _ := ioutil.ReadDir("/dev")

	for _, f := range contents {
		if strings.Contains(f.Name(), "tty.usb") ||
			strings.Contains(f.Name(), "ttyACM") ||
			strings.Contains(f.Name(), "ttyUSB") {
			InfoLogger.Println("Arduino found: /dev/" + f.Name())
			return "/dev/" + f.Name(), nil
		}
	}
	ErrorLogger.Println("can't find Arduino device in /dev/")
	return "", errors.New("can't find Arduino device in /dev/")
}

func CFMToBytes() []byte {
	var message strings.Builder

	artworksInRandom := []Artwork{}

	for i, artwork := range CFM.Artworks {
		if artwork.InRandom {
			artworksInRandom = append(artworksInRandom, CFM.Artworks[i])
		}
	}

	randoms := len(artworksInRandom)

	// config line
	s := fmt.Sprintf("%s|%d|%d|%d|", CFM.Mode, CFM.TimeDisplayTime, CFM.RandomDisplayTime, randoms)
	message.WriteString(s)

	for i, artwork := range artworksInRandom {
		num := int64(0)
		for i, tl := range artwork.TL {
			if tl {
				num |= 1 << (i)
			}
		}
		message.WriteString(fmt.Sprintf("%d", num))
		if i != randoms-1 {
			message.WriteString("|")
		}
	}
	return []byte(message.String())
}

/******************************************************************************
WEBSERVER PART
*******************************************************************************/

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
	CFM.Mode = val
	InfoLogger.Printf("New mode selected: %s\n", val)
	tmpl.Execute(w, CFM)
}

func createPage(w http.ResponseWriter, r *auth.AuthenticatedRequest) {
	tmpl := template.Must(template.ParseFiles("templates/create.tmpl"))

	if r.Method != http.MethodPost {
		tmpl.Execute(w, CFM)
		return
	}

	// TODO: do something with new value
	val := r.FormValue("settings")
	CFM.Mode = val
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

	if err != nil {
		WarningLogger.Printf("JSON decode error: %s\n", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}

	InfoLogger.Printf("Received new artwork: %s\n", art.Name)
	CFM.Artworks = append(CFM.Artworks, art)

	// ensures that the arduino is updated
	stopLoop()
	startLoop()

	writeArtworks(DATA_FILE)
}

func receiveAllArtworks(w http.ResponseWriter, r *http.Request) {
	if r.Method == http.MethodGet {
		w.WriteHeader(403)
		w.Write([]byte("403 Forbidden"))
		return
	}

	err := json.NewDecoder(r.Body).Decode(&CFM.Artworks)

	// ensures that the arduino is updated
	stopLoop()
	startLoop()

	if err != nil {
		WarningLogger.Printf("JSON decode error: %s\n", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
	}

	InfoLogger.Println("Received all artworks")
	writeArtworks(DATA_FILE)
}

func writeArtworks(filepath string) {
	jsonString, err := json.Marshal(&CFM)
	if err != nil {
		WarningLogger.Printf("json.Marshal error: %s\n", err)
		return
	}

	ioutil.WriteFile(filepath, jsonString, 0644)
	InfoLogger.Println("wrote new data to JSON file")
}

func loadArtworks(filepath string) {
	// load artworks from file, happens at initalization
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

	http.HandleFunc("/", authenticator.Wrap(homePage))
	http.HandleFunc("/create", authenticator.Wrap(createPage))
	http.HandleFunc("/log", authenticator.Wrap(logPage))
	http.HandleFunc("/setpw", authenticator.Wrap(setPasswordPage))
	http.HandleFunc("/ajax", receiveNewArtwork)
	http.HandleFunc("/ajax2", receiveAllArtworks)
	http.ListenAndServe(":4567", nil)
}

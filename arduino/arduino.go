package arduino

import (
	"errors"
	"fmt"
	"github.com/huin/goserial"
	"io"
	"io/ioutil"
	"log"
	"os"
	"strings"
)

var (
	WarningLogger *log.Logger
	ErrorLogger   *log.Logger
	InfoLogger    *log.Logger
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

	// find arduino device in /dev/ and initialize Serial port
	arduinoPath, _ := FindDevice()
	serialConfig := &goserial.Config{Name: arduinoPath, Baud: 9600}
	Serial, _ = goserial.OpenPort(serialConfig)
}

func FindDevice() (string, error) {
	contents, _ := ioutil.ReadDir("/dev")

	for _, f := range contents {
		if strings.Contains(f.Name(), "tty.usb") ||
			strings.Contains(f.Name(), "ttyUSB") {
			fmt.Println("Arduino found: /dev/" + f.Name())
			return "/dev/" + f.Name(), nil
		}
	}
	ErrorLogger.Println("can't find Arduino device in /dev/")
	return "", errors.New("can't find Arduino device in /dev/")
}

func GetMode() (string, error) {
	// TODO
	// get current option from arduino
	return "time", nil
}

func setMode(command []byte) error {
	_, err := Serial.Write(command)
	if err != nil {
		WarningLogger.Println(err)
		return err
	}

	return nil
}

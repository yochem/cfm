package arduino

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"strings"
)

func FindDevice() (string, error) {
	contents, _ := ioutil.ReadDir("/dev")

	for _, f := range contents {
		if strings.Contains(f.Name(), "tty.usbserial") ||
		   strings.Contains(f.Name(), "ttyUSB") {
			fmt.Println("Arduino found: /dev/" + f.Name())
			return "/dev/" + f.Name(), nil
		}
	}

	return "", errors.New("can't find Arduino device in /dev/");
}

func GetMode() (string, error) {
	// TODO
	// get current option from arduino
	return "time", nil
}

func SetMode(command byte, argument float32, serialPort io.ReadWriteCloser) error {
	if serialPort == nil {
		return nil
	}

	// Package argument for transmission
	bufOut := new(bytes.Buffer)
	err := binary.Write(bufOut, binary.LittleEndian, argument)
	if err != nil {
		return err
	}

	// Transmit command and argument down the pipe.
	for _, v := range [][]byte{{command}, bufOut.Bytes()} {
		_, err = serialPort.Write(v)
		if err != nil {
			return err
		}
	}

	return nil
}

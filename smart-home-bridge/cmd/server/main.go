package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"log"
	huev1 "main/api/v1/hue"
	"main/api/v1/kasa"
	"main/mike"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"time"

	"github.com/pkg/errors"
)

var (
	variableStorage = map[string]string{}
	funcStorage     = map[string]func(arg ...string){}
)

func main() {

	kasaClient, err := kasa.LoadClient("kasa.json")
	if err != nil {
		log.Panic(errors.Wrap(err, "kasa.LoadClient"))
	}

	hueClient, err := huev1.LoadConfig("connection.json")
	if err != nil {
		log.Panic(errors.Wrap(err, "huev1.LoadConfig"))
	}

	funcStorage["toggle-room"] = func(arg ...string) {
		if len(arg) == 0 {
			return
		}
		hueClient.ToggleRoom(arg[0])
		kasaClient.ToggleRoom(arg[0])
	}

	funcStorage["brighter"] = func(arg ...string) {
		if len(arg) == 0 {
			return
		}
		hueClient.Brighter(arg[0])
	}

	funcStorage["darker"] = func(arg ...string) {
		if len(arg) == 0 {
			return
		}
		hueClient.Darker(arg[0])
	}

	for {
		if len(hueClient.Username) == 0 {
			log.Println("Username not found, querying server...")
			if err := hueClient.GetUsername(); err != nil {
				if errors.Is(err, huev1.ErrMustPressButton) {
					log.Println(err.Error())
				} else {
					log.Panic(errors.Wrap(err, "config.GetUsername"))
				}
			}
		}

		if len(hueClient.Username) > 0 {
			break
		}

		log.Println("No username yet, wait 15 seconds and try again")
		time.Sleep(15 * time.Second)
	}

	f, err := os.Open(hueClient.InputDevice)
	if err != nil {
		panic(err)
	}
	defer f.Close()

	b := make([]byte, 24)

	for {
		f.Read(b)
		var value int32
		typ := binary.LittleEndian.Uint16(b[16:18])
		code := binary.LittleEndian.Uint16(b[18:20])
		binary.Read(bytes.NewReader(b[20:]), binary.LittleEndian, &value)
		if typ == 1 && value > 0 {
			codeString := strconv.Itoa(int(code))
			if cmd, ok := hueClient.Map[codeString]; ok {
				switch cmd.Command {
				case "set":
					fmt.Printf("set %s to %s\n", cmd.Target, strings.Join(cmd.Values, ","))
					variableStorage[cmd.Target] = strings.Join(cmd.Values, ",")
				case "run":
					fmt.Printf("run %s with %s\n", cmd.Target, variableStorage[cmd.Values[0]])
					go func() {
						exec.Command(cmd.Target, cmd.Values...)
						if f, ok := funcStorage[cmd.Target]; ok {
							lookupvalues := mike.Map(cmd.Values, func(v string) string {
								if storedValue, ok := variableStorage[v]; ok {
									return storedValue
								}
								return ""
							})
							f(lookupvalues...)
						}
					}()
				}
			} else {
				fmt.Printf("unassigned key: %s\n", codeString)
			}
		}
	}
}

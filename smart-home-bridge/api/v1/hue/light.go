package hue

import (
	"fmt"
	"strconv"
)

type Light struct {
	Config   *Config
	LightKey string

	State struct {
		On        bool      `json:"on"`
		Bri       int       `json:"bri"`
		Hue       int       `json:"hue"`
		Sat       int       `json:"sat"`
		Effect    string    `json:"effect"`
		XY        []float64 `json:"xy"`
		Ct        int       `json:"ct"`
		Alert     string    `json:"alert"`
		ColorMode string    `json:"colormode"`
		Mode      string    `json:"mode"`
		Reachable bool      `json:"reachable"`
	} `json:"state"`

	Type             string `json:"type"`
	Name             string `json:"name"`
	ModelId          string `json:"modelid"`
	ManufacturerName string `json:"manufacturername"`
	ProductName      string `json:"productname"`
	UniqueID         string `json:"uniqueid"`
}

func (l *Light) Off() {
	l.SetBool("on", false)
}

func (l *Light) On() {
	l.SetBool("on", true)
}

func (l *Light) Brighter() {
	newBri := l.State.Bri + 32
	if newBri > 254 {
		newBri = 254
	}

	if newBri < 1 {
		newBri = 1
	}

	l.SetInt("bri", newBri)
}

func (l *Light) Darker() {
	newBri := l.State.Bri - 32

	if newBri > 254 {
		newBri = 254
	}

	if newBri < 1 {
		newBri = 1
	}

	l.SetInt("bri", newBri)
}

func (l *Light) SetInt(key string, val int) {
	l.Set(key, strconv.Itoa(val))
}

func (l *Light) SetBool(key string, val bool) {
	if val {
		l.Set(key, "true")
		return
	}

	l.Set(key, "false")
}

func (l *Light) SetString(key string, val string) {
	l.Set(key, fmt.Sprintf("\"%s\"", val))
}

func (l *Light) Set(key string, val string) {
	body := fmt.Sprintf("{\"%s\":%s}", key, val)
	res := l.Config.CallAuthed("PUT", fmt.Sprintf("/lights/%s/state", l.LightKey), []byte(body))
	fmt.Println(res)
}

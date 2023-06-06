package hue

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
)

type Light struct {
	Name     string `json:"name"`
	Type     string `json:"type"`
	UniqueId string `json:"uniqueid"`

	State struct {
		On         bool      `json:"on"`
		Brightness int32     `json:"bri"`
		Hue        int32     `json:"hue"`
		Saturation int32     `json:"sat"`
		Effect     string    `json:"effect"`
		XY         []float32 `json:"xy"`
		ColorMode  string    `json:"colormode"`
		Mode       string    `json:"mode"`
	} `json:"state"`
}

func GetLights(server string, username string) map[string]Light {
	var lights map[string]Light

	resp, err := http.Get(fmt.Sprintf("http://%s.local/api/%s/lights", server, username))
	if err != nil {
		panic(err)
	}
	defer resp.Body.Close()

	data, err := io.ReadAll(resp.Body)
	if err != nil {
		panic(err)
	}

	json.Unmarshal(data, &lights)

	return lights
}

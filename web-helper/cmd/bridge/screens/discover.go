package screens

import (
	"encoding/json"
	"io"
	"net/http"

	"github.com/pkg/errors"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type HueBridgeServer struct {
	Id        string `json:"id"`
	IpAddress string `json:"internalipaddress"`
	Port      int64  `json:"port"`
}

var (
	servers []HueBridgeServer
)

func discover() ([]HueBridgeServer, error) {
	resp, err := http.Get("https://discovery.meethue.com")
	if err != nil {
		return nil, errors.Wrap(err, "http.Get")
	}
	defer resp.Body.Close()

	data, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, errors.Wrap(err, "io.ReadAll")
	}

	var servers []HueBridgeServer
	if err := json.Unmarshal(data, &servers); err != nil {
		return nil, errors.Wrap(err, "json.Unmarshal")
	}

	return servers, nil
}

func CreateDiscover() {
	go func() {
		s, err := discover()
		if err != nil {
			// TODO: show error screen
			rl.TraceLog(rl.LogError, err.Error())
		}
		servers = append(servers, s...)
		ActiveScreen.Destroy()

		ActiveScreen.SetFuncs([]func(){nil, nil, DrawChooseServer, nil})
		ActiveScreen.Create()
	}()
}

func DrawDiscover() {
	rl.DrawText("Discovering servers...", 10, 10, rl.GetFontDefault().BaseSize*3, rl.Green)
}

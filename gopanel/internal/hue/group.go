package hue

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
)

type Group struct {
	Name   string   `json:"name"`
	Lights []string `json:"lights"`
	Type   string   `json:"type"`
	State  struct {
		AnyOn bool `json:"any_on"`
		AllOn bool `json:"all_on"`
	} `json:"state"`
}

func GetGroups(server string, username string) []Group {
	resp, err := http.Get(fmt.Sprintf("http://%s.local/api/%s/groups", server, username))
	if err != nil {
		panic(err)
	}
	defer resp.Body.Close()

	data, err := io.ReadAll(resp.Body)
	if err != nil {
		panic(err)
	}

	var groups []Group
	var g map[string]Group
	json.Unmarshal(data, &g)

	for _, group := range g {
		if group.Type == "Room" {
			groups = append(groups, group)
		}
	}

	return groups
}

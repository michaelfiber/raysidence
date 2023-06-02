package hue

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"main/mike"
	"net/http"
	"os"
	"time"

	"github.com/pkg/errors"
)

var ErrMustPressButton = errors.New("must press button on Hue Bridge no more than 30 seconds before attempting this")

type Config struct {
	path        string
	Username    string `json:"username"`
	Server      string `json:"server"`
	InputDevice string `json:"input_device"`
	Map         map[string]struct {
		Command string   `json:"cmd"`
		Target  string   `json:"target"`
		Values  []string `json:"values"`
	} `json:"map"`

	Groups     map[string]*Group
	Lights     map[string]*Light
	LastUpdate time.Time
}

type GetUsernameReq struct {
	DeviceType string `json:"devicetype"`
}

func LoadConfig(path string) (*Config, error) {
	config := new(Config)

	config.path = path

	data, err := os.ReadFile(path)
	if err != nil {
		return nil, errors.Wrap(err, "os.Readfile")
	}

	if err := json.Unmarshal(data, config); err != nil {
		return nil, errors.Wrap(err, "json.Unmarshal")
	}

	config.GetGroups()
	config.GetLights()

	go func() {
		for {
			config.GetGroups()
			config.GetLights()
			config.LastUpdate = time.Now()
			time.Sleep(1 * time.Second)
		}
	}()

	return config, nil
}

func (c *Config) Save() error {
	data, err := mike.ToJson(c)
	if err != nil {
		return errors.Wrap(err, "mike.ToJson")
	}
	return os.WriteFile(c.path, []byte(data), os.ModePerm)
}

func (c *Config) GetUsername() error {
	body := GetUsernameReq{DeviceType: "rf_hue#go_client"}
	bodyString, err := json.Marshal(body)
	if err != nil {
		return errors.Wrap(err, "json.Marshal")
	}

	resp, err := http.Post(fmt.Sprintf("http://%s.local/api", c.Server), "application/json", bytes.NewBuffer(bodyString))
	if err != nil {
		return errors.Wrap(err, "http.Post")
	}
	defer resp.Body.Close()
	respBody, _ := io.ReadAll(resp.Body)

	var responseItems []ApiItem

	json.Unmarshal(respBody, &responseItems)

	errors101 := mike.Filter(responseItems, func(item ApiItem) bool {
		return item.Error.ErrorType == 101
	})

	if len(errors101) > 0 {
		return ErrMustPressButton
	}

	usernames := mike.Filter(responseItems, func(item ApiItem) bool {
		return len(item.Success.Username) > 0
	})

	if len(usernames) == 0 {
		return errors.New("unknown error")
	}

	c.Username = usernames[0].Success.Username
	if err := c.Save(); err != nil {
		return errors.Wrap(err, "c.Save()")
	}

	return nil
}

func (c *Config) CallAuthed(method string, path string, body []byte) string {
	url := fmt.Sprintf("http://%s.local/api/%s%s", c.Server, c.Username, path)
	req, err := http.NewRequest(method, url, bytes.NewBuffer(body))
	if err != nil {
		fmt.Printf("%v\n", err)
		return ""
	}

	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		fmt.Printf("%v\n", err)
		return ""
	}
	defer resp.Body.Close()

	respBody, _ := io.ReadAll(resp.Body)
	return string(respBody)
}

func (c *Config) GetLights() {
	res := c.CallAuthed("GET", "/lights", nil)

	if len(res) == 0 {
		return
	}

	lightResponse := new(map[string]*Light)
	json.Unmarshal([]byte(res), &lightResponse)

	for lightKey, light := range *lightResponse {
		light.Config = c
		light.LightKey = lightKey
	}

	c.Lights = *lightResponse
}

func (c *Config) GetGroups() {
	groupResponse := new(map[string]*Group)

	res := c.CallAuthed("GET", "/groups", nil)
	if len(res) == 0 {
		return
	}

	json.Unmarshal([]byte(res), &groupResponse)

	for groupKey, group := range *groupResponse {
		group.GroupKey = groupKey
		group.Config = c
	}

	c.Groups = *groupResponse
}

func (c *Config) ToggleRoom(roomName string) {
	if room := c.FindRoom(roomName); room != nil {
		room.Toggle()
	}
}

func (c *Config) Brighter(roomName string) {
	if room := c.FindRoom(roomName); room != nil {
		room.Brighter()
	}
}

func (c *Config) Darker(roomName string) {
	if room := c.FindRoom(roomName); room != nil {
		room.Darker()
	}
}

func (c *Config) FindRoom(roomName string) *Group {
	for _, group := range c.Groups {
		if group.Name != roomName {
			continue
		}
		return group
	}

	return nil
}

package kasa

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"regexp"
	"strings"
	"time"

	"github.com/pkg/errors"
)

type RoomAssignment struct {
	Plugs []string `json:"plugs"`
}

type Client struct {
	Plugs map[string]Plug
	Rooms map[string]RoomAssignment `json:"rooms"`
}

var (
	nameRegex = regexp.MustCompile("== (.*) - [A-Za-z0-9]*.*")
	ipRegex   = regexp.MustCompile(`Host: ([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)`)
)

func LoadClient(configPath string) (*Client, error) {
	client := new(Client)

	data, err := os.ReadFile(configPath)
	if err != nil {
		return nil, errors.Wrap(err, "os.Readfile")
	}

	if err := json.Unmarshal(data, &client); err != nil {
		return nil, errors.Wrap(err, "json.Unmarshal")
	}

	fmt.Printf("Client: %v\n", client)

	client.Init()

	return client, nil
}

func (c *Client) Init() {
	output, err := exec.Command("kasa").Output()
	if err != nil {
		panic(err)
	}

	plugMap := make(map[string]Plug)
	hostname := ""

	lines := strings.Split(string(output), "\n")
	for _, line := range lines {
		result := nameRegex.FindStringSubmatch(line)
		if len(result) > 1 {
			hostname = result[1]
			continue
		}

		result = ipRegex.FindStringSubmatch(line)
		if len(result) > 1 {
			if len(hostname) > 0 {
				plugMap[hostname] = Plug{
					Name: hostname,
					Ip:   result[1],
					Info: PlugInfo{},
				}
				hostname = ""
			}
		}
	}

	c.Plugs = plugMap

	go func() {
		c.Update()
		time.Sleep(2 * time.Second)
	}()
}

func (c *Client) Update() {
	for key := range c.Plugs {
		plug := c.Plugs[key]
		output, err := exec.Command("kasa", "--host", plug.Ip, "--json", "state").Output()

		if err != nil {
			continue
		}
		json.Unmarshal(output, &plug.Info)
		c.Plugs[key] = plug
	}
}

func (c *Client) ToggleRoom(roomName string) {
	fmt.Printf("[kasa] toggle room - %s - %v\n", roomName, c.Rooms)
	roomAssignment, ok := c.Rooms[roomName]
	if !ok {
		return
	}

	for _, plugName := range roomAssignment.Plugs {
		if plug, ok := c.Plugs[plugName]; ok {
			fmt.Printf("Examine state of plug %s\n", plug.Name)
			if plug.Info.System.SysInfo.RelayState == 1 {
				plug.Off()
			} else {
				plug.On()
			}
		}
	}

	go c.Update()
}

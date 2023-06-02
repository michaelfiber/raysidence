package kasa

import (
	"fmt"
	"os/exec"
)

type Plug struct {
	Name string
	Ip   string
	Info PlugInfo
}

type PlugInfo struct {
	System struct {
		SysInfo struct {
			Type       string `json:"type"`
			Model      string `json:"model"`
			DeviceName string `json:"dev_name"`
			Alias      string `json:"alias"`
			RelayState int    `json:"relay_state"`
		} `json:"get_sysinfo"`
	} `json:"System"`
}

func (p *Plug) RunCommand(commandName string) {
	output, err := exec.Command("kasa", "--host", p.Ip, "--type", "plug", commandName).Output()
	if err != nil {
		fmt.Printf("Error setting %s to %s: %v\n", p.Ip, commandName, err)
		return
	}
	fmt.Printf("kasa exec output: %s\n", output)
}

func (p *Plug) Off() {
	p.RunCommand("off")
}

func (p *Plug) On() {
	p.RunCommand("on")
}

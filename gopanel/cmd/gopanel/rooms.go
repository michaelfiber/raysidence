package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Room struct {
	Rec        rl.Rectangle `json:"rec"`
	Name       string       `json:"name"`
	GroupIndex int
}

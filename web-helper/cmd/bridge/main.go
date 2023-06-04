package main

import (
	"time"
	"webhelper/cmd/bridge/screens"

	rl "github.com/gen2brain/raylib-go/raylib"
)

const (
	SceneDiscover     int = 0
	SceneUsername     int = 1
	SceneChooseServer int = 2
	FakeDelay             = 3500 * time.Millisecond
)

func main() {

	screens.ActiveScreen.SetFuncs([]func(){screens.CreateDiscover, nil, screens.DrawDiscover, nil})
	screens.ActiveScreen.Create()

	rl.InitWindow(800, 480, "raysidence")
	defer rl.CloseWindow()

	rl.SetTargetFPS(60)

	for !rl.WindowShouldClose() {

		screens.ActiveScreen.Update()

		// draw
		rl.BeginDrawing()
		rl.ClearBackground(rl.Black)

		screens.ActiveScreen.Draw()

		rl.EndDrawing()
	}
}

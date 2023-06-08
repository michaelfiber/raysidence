package main

import (
	"encoding/json"
	"fmt"
	"main/internal"
	"main/internal/gridshape"
	"main/internal/gui"
	"main/internal/helper"
	"main/internal/hue"
	"os"
	"strings"
	"time"

	rl "github.com/gen2brain/raylib-go/raylib"
)

var (
	stage         int
	startingVec2I gridshape.Vector2I
	currentRecI   gridshape.RectangleI
	rooms         []Room
	groups        []hue.Group
	lights        map[string]hue.Light
	changeCount   int = 0
	selectedRoom  int
	center        gridshape.Vector2I
	isEditing     bool
	sprites       []Sprite
	primaryCol    rl.Color = rl.Red
	darkCol       rl.Color = rl.Color{
		R: 160,
		G: 0,
		B: 0,
		A: 200,
	}

	saveButton gui.Button = gui.Button{
		Rec: rl.Rectangle{
			X:      0,
			Y:      0,
			Width:  100,
			Height: 40,
		},
		Text: "Save",
		Dock: gui.ButtonDockRight | gui.ButtonDockBottom,
		Bg:   rl.Red,
		Fg:   rl.Black,
	}

	clearButton gui.Button = gui.Button{
		Rec: rl.Rectangle{
			X:      0,
			Y:      0,
			Width:  100,
			Height: 40,
		},
		Text: "Clear",
		Dock: gui.ButtonDockRight | gui.ButtonDockBottom,
		Bg:   rl.Blue,
		Fg:   rl.Black,
	}
)

func Connect() {
	server := internal.Server
	username := internal.Username

	// get rooms
	newGroups := hue.GetGroups(server, username)

	for _, newGroup := range newGroups {
		// Get a list of indices for the groups slice that are present in this update.
		toUpdate := helper.FilteredMap(groups, func(index int, item hue.Group) (int, bool) {
			if item.Name == newGroup.Name {
				return index, true
			}
			return -1, false
		})

		if len(toUpdate) > 0 {
			for _, i := range toUpdate {
				if groups[i].State.AllOn != newGroup.State.AllOn || groups[i].State.AnyOn != newGroup.State.AnyOn {
					rl.TraceLog(rl.LogInfo, fmt.Sprintf("Fire off a sprite for %s", newGroup.Name))
					addSprite(newGroup.Name)
				}
				groups[i].State.AllOn = newGroup.State.AllOn
				groups[i].State.AnyOn = newGroup.State.AnyOn
			}
		} else {
			groups = append(groups, newGroup)
		}
	}

	// get lights
	lights = hue.GetLights(server, username)

	stage = 0
}

func LoadRooms() {
	isEditing = true

	data, err := os.ReadFile("rooms.json")
	if err != nil {
		rl.TraceLog(rl.LogError, err.Error())
		return
	}
	if err := json.Unmarshal(data, &rooms); err != nil {
		rl.TraceLog(rl.LogError, err.Error())
	}

	min := rl.Vector2{
		X: 999999,
		Y: 999999,
	}
	var max rl.Vector2

	for _, r := range rooms {
		min.X = helper.Min(min.X, r.Rec.X)
		min.Y = helper.Min(min.Y, r.Rec.Y)
		max.X = helper.Max(max.X, r.Rec.X+r.Rec.Width)
		max.Y = helper.Max(max.Y, r.Rec.Y+r.Rec.Height)
	}

	var overall rl.Rectangle
	overall.X = min.X
	overall.Y = min.Y
	overall.Width = max.X - min.X
	overall.Height = max.Y - min.Y

	xdiff := (overall.X + overall.Width/2 - float32(rl.GetScreenWidth())/2) / 10 * 10
	ydiff := (overall.Y + overall.Height/2 - float32(rl.GetScreenHeight())/2) / 10 * 10

	for i := range rooms {
		room := rooms[i]
		room.Rec.X -= xdiff
		room.Rec.Y -= ydiff
		rooms[i] = room
	}

	isEditing = false
}

func MapRooms() {
	for i := range rooms {
		room := rooms[i]

		room.GroupIndex = -1
		for j, group := range groups {
			if group.Name == room.Name {
				room.GroupIndex = j
				break
			}
		}

		rooms[i] = room
	}
}

func UpdateRooms(full bool) {
	if full {
		LoadRooms()
	}

	Connect()

	MapRooms()
}

func RoomUpdateLoop() {
	for {
		if !isEditing {
			UpdateRooms(false)
		}
		time.Sleep(2500 * time.Millisecond)
	}
}

func main() {
	selectedRoom = -1
	stage = -1

	rl.SetTargetFPS(30)

	rl.InitWindow(800, 480, "Raysidence")
	defer rl.CloseWindow()

	center.X = int32(rl.GetScreenWidth() / 2)
	center.Y = int32(rl.GetScreenHeight() / 2)

	go func() {
		UpdateRooms(true)
		RoomUpdateLoop()
	}()

	for !rl.WindowShouldClose() {

		if isEditing {
			Edit()
		} else {
			Use()
		}

	}
}

func Use() {

	// Update
	updateSprites()

	// Draw
	rl.BeginDrawing()
	defer rl.EndDrawing()

	fadeColor := rl.Fade(rl.Black, 0.07)
	rl.DrawRectangle(0, 0, int32(rl.GetScreenWidth()), int32(rl.GetScreenHeight()), fadeColor)

	drawSprites()

	for i := 0; i < 2; i++ {
		x := rl.GetRandomValue(0, int32(rl.GetScreenWidth())) / 50 * 50
		y := rl.GetRandomValue(0, int32(rl.GetScreenHeight())) / 50 * 50
		rl.DrawRectangle(x, y, 50, 50, rl.Fade(darkCol, 0.1))
	}

	for _, room := range rooms {

		if room.GroupIndex < 0 || room.GroupIndex > len(groups) || len(groups) == 0 {
			continue
		}

		if groups[room.GroupIndex].State.AnyOn {
			rl.DrawRectangleRec(room.Rec, rl.Fade(darkCol, 0.05))
		}

		rl.DrawRectangleLinesEx(room.Rec, 2.0, primaryCol)
		flicker := rl.GetRandomValue(0, 18) - 6
		rl.DrawRectangleLinesEx(
			rl.Rectangle{
				X:      room.Rec.X - float32(flicker),
				Y:      room.Rec.Y - float32(flicker),
				Width:  room.Rec.Width + float32(flicker*2),
				Height: room.Rec.Height + float32(flicker*2),
			},
			(1-float32(flicker+6)/18)*5,
			rl.Fade(darkCol, 0.1),
		)

		rl.DrawText(strings.ReplaceAll(room.Name, " ", "\n"), int32(room.Rec.X)+10, int32(room.Rec.Y)+8, rl.GetFontDefault().BaseSize*3, primaryCol)

		for i, lightKey := range groups[room.GroupIndex].Lights {
			lightRec := rl.NewRectangle(
				room.Rec.X+5,
				room.Rec.Y+room.Rec.Height-float32(25-i*-25),
				20,
				20,
			)
			rl.DrawRectangleLinesEx(lightRec, 2.0, primaryCol)

			if lights[lightKey].State.On {
				rl.DrawRectangleRec(lightRec, primaryCol)
			}

			rl.DrawText(lights[lightKey].Name, int32(lightRec.X+lightRec.Width+5), int32(lightRec.Y+5), rl.GetFontDefault().BaseSize, primaryCol)
		}
	}
}

func Edit() {
	fadeColor := rl.Fade(rl.Black, 0.25)

	// UPDATE
	mousePos := rl.GetMousePosition()

	// snap to 10x10 grid
	currentX := int32(mousePos.X+5) / 10 * 10
	currentY := int32(mousePos.Y+5) / 10 * 10

	// UPDATE

	func() {
		if changeCount > 0 && saveButton.IsPressed(0) {
			data, err := json.MarshalIndent(rooms, "", "  ")
			if err != nil {
				panic(err)
			}
			os.WriteFile("rooms.json", data, os.ModePerm)
			changeCount = 0
			rooms = rooms[:0]
			LoadRooms()

			return
		} else if clearButton.IsPressed(0) {
			rooms = rooms[:0]
			return
		}

		if rl.IsMouseButtonPressed(1) && stage == 1 {
			stage--
		}

		if rl.IsMouseButtonReleased(0) && stage == 1 {
			if currentRecI.Width > 2 && currentRecI.Height > 2 {
				rooms = append(rooms, Room{
					Rec:  currentRecI.Rectangle(),
					Name: "",
				})
				stage++
			} else {
				stage = 0
			}
		}

		if rl.IsMouseButtonPressed(0) && stage == 0 {
			keepGoing := true
			for i, room := range rooms {
				if rl.CheckCollisionPointRec(rl.GetMousePosition(), room.Rec) {
					selectedRoom = i
					keepGoing = false
					break
				}
			}
			if keepGoing {
				stage++
				startingVec2I.X = currentX
				startingVec2I.Y = currentY
				currentRecI.X = startingVec2I.X
				currentRecI.Y = startingVec2I.Y
				currentRecI.Width = 10
				currentRecI.Height = 10
			}
		}
	}()

	// DRAW
	func() {
		rl.BeginDrawing()
		defer rl.EndDrawing()

		rl.DrawRectangle(0, 0, int32(rl.GetScreenWidth()), int32(rl.GetScreenHeight()), fadeColor)

		rl.DrawRectangleLines(currentX, currentY, 10, 10, rl.Gray)
		rl.DrawPixel(currentX, currentY, rl.White)

		for i, room := range rooms {
			rl.DrawRectangleRec(room.Rec, rl.Gray)
			rl.DrawText(room.Name, int32(room.Rec.X)+5, int32(room.Rec.Y)+5, rl.GetFontDefault().BaseSize*2, rl.Black)

			if selectedRoom == i {
				rl.DrawRectangleLinesEx(room.Rec, 1.0, rl.Green)
			}
		}

		if stage == -1 {
			rl.DrawText("Connecting to smart home...", 10, 10, rl.GetFontDefault().BaseSize*4, rl.Blue)
			rl.DrawRectangle(rl.GetRandomValue(0, int32(rl.GetScreenWidth())), rl.GetRandomValue(0, int32(rl.GetScreenHeight())), 10, 10, rl.DarkBlue)
		}

		if stage == 0 {
			rl.DrawText("Draw Buttons", 10, 10, rl.GetFontDefault().BaseSize*4, rl.Green)
		}

		if stage == 1 {
			currentRecI.X = helper.Min(startingVec2I.X, currentX+10)
			currentRecI.Y = helper.Min(startingVec2I.Y, currentY+10)
			currentRecI.Width = helper.Max(startingVec2I.X, currentX+10)
			currentRecI.Height = helper.Max(startingVec2I.Y, currentY+10)

			currentRecI.Width = currentRecI.Width - currentRecI.X
			currentRecI.Height = currentRecI.Height - currentRecI.Y

			rl.DrawRectangle(currentRecI.X, currentRecI.Y, currentRecI.Width, currentRecI.Height, rl.White)
		}

		if stage == 2 {
			rl.DrawRectangle(0, 0, int32(rl.GetScreenWidth()), int32(rl.GetScreenHeight()), fadeColor)

			prompt := "Select Room"
			size := rl.MeasureTextEx(rl.GetFontDefault(), prompt, float32(rl.GetFontDefault().BaseSize)*3, 1.0)

			rl.DrawText(prompt, int32(rl.GetScreenWidth())/2-int32(size.X)/2, 10, rl.GetFontDefault().BaseSize*3, rl.Green)

			for i, group := range groups {
				width := rl.MeasureText(group.Name, rl.GetFontDefault().BaseSize*3)
				rec := rl.Rectangle{
					X:      float32(int32(rl.GetScreenWidth())/2) - 150,
					Y:      float32(40+i*40) - 1,
					Width:  300,
					Height: 33,
				}
				rl.DrawRectangleRec(rec, rl.Yellow)

				rl.DrawText(group.Name, int32(rl.GetScreenWidth()/2)-width/2, int32(40+i*40), rl.GetFontDefault().BaseSize*3, rl.Black)

				if rl.IsMouseButtonPressed(0) && rl.CheckCollisionPointRec(rl.GetMousePosition(), rec) {
					changeCount++
					rooms[len(rooms)-1].Name = group.Name
					stage = 0
				}
			}
		}

		if changeCount > 0 {
			saveButton.Draw()
		} else {
			clearButton.Draw()
		}
	}()
}

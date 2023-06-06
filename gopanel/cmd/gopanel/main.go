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

type Room struct {
	Rec        rl.Rectangle `json:"rec"`
	Name       string       `json:"name"`
	GroupIndex int
}

type Sprite struct {
	Rec  rl.Rectangle
	Fade float32
}

var (
	stage         int
	startingVec2I gridshape.Vector2I
	currentRecI   gridshape.RectangleI
	currentText   string
	rooms         []Room
	nameLengthMax int = 32
	groups        []hue.Group
	lights        map[string]hue.Light
	changeCount   int = 0
	selectedRoom  int
	center        gridshape.Vector2I
	isEditing     bool
	sprites       []Sprite

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

func removeSprite(index int) {
	sprites[index] = sprites[len(sprites)-1]
	sprites = sprites[:len(sprites)-1]
}

func updateSprites() {
	var toRemove []Sprite

	for i := range sprites {
		sprite := sprites[i]

		d := rl.GetFrameTime() * 50
		sprite.Rec.X -= d
		sprite.Rec.Y -= d
		sprite.Rec.Width += d * 2
		sprite.Rec.Height += d * 2
		sprite.Fade -= rl.GetFrameTime()
		if sprite.Fade <= 0 {
			toRemove = append(toRemove, sprite)
		}
		rl.TraceLog(rl.LogInfo, fmt.Sprintf("\tfade level: %f", sprite.Fade))

		sprites[i] = sprite
	}

	for _, sprite := range toRemove {
		for i := range sprites {
			if sprites[i] == sprite {
				removeSprite(i)
				break
			}
		}
	}
}

func drawSprites() {
	for _, sprite := range sprites {
		//rl.DrawRectangleLinesEx(sprite.Rec, rl.GetFrameTime()*50, rl.Fade(rl.White, sprite.Fade))
		rl.DrawRectangleRoundedLines(sprite.Rec, (1 - sprite.Fade), 32, rl.GetFrameTime()*50, rl.Fade(rl.White, sprite.Fade))
	}
}

func addSprite(roomName string) {
	for _, room := range rooms {
		if room.Name == roomName {
			rl.TraceLog(rl.LogInfo, fmt.Sprintf("adding sprite for %s: (%v)", roomName, room.Rec))
			sprites = append(sprites, Sprite{
				Rec:  room.Rec,
				Fade: 0.75,
			})
		}
	}
}

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
		time.Sleep(5 * time.Second)
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

	updateSprites()

	rl.BeginDrawing()
	defer rl.EndDrawing()

	fadeColor := rl.Fade(rl.Black, 0.05)
	rl.DrawRectangle(0, 0, int32(rl.GetScreenWidth()), int32(rl.GetScreenHeight()), fadeColor)

	drawSprites()

	for i := 0; i < 3; i++ {
		x := rl.GetRandomValue(0, int32(rl.GetScreenWidth()))
		y := rl.GetRandomValue(0, int32(rl.GetScreenHeight()))
		rl.DrawRectangle(x, y, 10, 10, rl.Fade(rl.Green, 0.25))
	}

	for _, room := range rooms {

		if room.GroupIndex > -1 && groups[room.GroupIndex].State.AnyOn {
			rl.DrawRectangleRec(room.Rec, rl.White)
		}

		rl.DrawRectangleLinesEx(room.Rec, 2.0, rl.White)

		rl.DrawText(strings.ReplaceAll(room.Name, " ", "\n"), int32(room.Rec.X)+11, int32(room.Rec.Y)+9, rl.GetFontDefault().BaseSize*2, rl.Black)
		rl.DrawText(strings.ReplaceAll(room.Name, " ", "\n"), int32(room.Rec.X)+10, int32(room.Rec.Y)+8, rl.GetFontDefault().BaseSize*2, rl.Yellow)
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
				currentText = ""
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

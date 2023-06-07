package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

// A Sprite is a rectangle that starts the same shape as a room and expands while fading out.
// "Sprite" is a terrible name for it.
type Sprite struct {
	Rec  rl.Rectangle
	Fade float32
}

func removeSprite(index int) {
	sprites[index] = sprites[len(sprites)-1]
	sprites = sprites[:len(sprites)-1]
}

func updateSprites() {
	var toRemove []Sprite

	for i := range sprites {
		sprite := sprites[i]

		d := rl.GetFrameTime() * 100
		sprite.Rec.X -= d
		sprite.Rec.Y -= d
		sprite.Rec.Width += d * 2
		sprite.Rec.Height += d * 2
		sprite.Fade -= rl.GetFrameTime() / 10
		if sprite.Fade < 0 || (sprite.Rec.X < 0 && sprite.Rec.Y < 0 && sprite.Rec.X+sprite.Rec.Width > float32(rl.GetScreenWidth()) && sprite.Rec.Y+sprite.Rec.Height > float32(rl.GetScreenHeight())) {
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
		rl.DrawRectangleLinesEx(sprite.Rec, 4, rl.Fade(primaryCol, sprite.Fade))
	}
}

func addSprite(roomName string) {
	for _, room := range rooms {
		if room.Name == roomName {
			rl.TraceLog(rl.LogInfo, fmt.Sprintf("adding sprite for %s: (%v)", roomName, room.Rec))
			sprites = append(sprites, Sprite{
				Rec:  room.Rec,
				Fade: 0.25,
			})
		}
	}
}

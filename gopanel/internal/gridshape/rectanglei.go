package gridshape

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

type Vector2I struct {
	X int32
	Y int32
}

type RectangleI struct {
	Vector2I
	Width  int32
	Height int32
}

func (r *RectangleI) Rectangle() rl.Rectangle {
	return rl.Rectangle{
		X:      float32(r.X),
		Y:      float32(r.Y),
		Width:  float32(r.Width),
		Height: float32(r.Height),
	}
}

func (r *RectangleI) JustPressed() bool {
	return rl.IsMouseButtonPressed(0) && rl.CheckCollisionPointRec(rl.GetMousePosition(), r.Rectangle())
}

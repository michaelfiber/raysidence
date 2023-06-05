package gui

import rl "github.com/gen2brain/raylib-go/raylib"

type Button struct {
	Rec  rl.Rectangle
	Text string
	Dock uint8
	Bg   rl.Color
	Fg   rl.Color

	textSize  rl.Vector2
	layoutRec rl.Rectangle
}

const (
	ButtonDockTop    = 1 << 0
	ButtonDockRight  = 1 << 1
	ButtonDockBottom = 1 << 2
	ButtonDockLeft   = 1 << 3
)

func (b *Button) DockRight() bool {
	return b.Dock&ButtonDockRight > 0
}

func (b *Button) DockLeft() bool {
	return b.Dock&ButtonDockLeft > 0
}

func (b *Button) DockTop() bool {
	return b.Dock&ButtonDockTop > 0
}

func (b *Button) DockBottom() bool {
	return b.Dock&ButtonDockBottom > 0
}

func (b *Button) GetLayoutRec() rl.Rectangle {
	if b.layoutRec.Width > 0 {
		return b.layoutRec
	}

	var r rl.Rectangle

	r.X = b.Rec.X
	r.Y = b.Rec.Y
	r.Width = b.Rec.Width
	r.Height = b.Rec.Height

	if b.DockLeft() {
		r.X = 10
	}

	if b.DockBottom() {
		r.Y = float32(rl.GetScreenHeight() - 10 - int(r.Height))
	}

	if b.DockRight() {
		r.X = float32(rl.GetScreenWidth() - 10 - int(r.Width))
	}

	if b.DockTop() {
		r.Y = 10
	}

	b.layoutRec = r

	return r
}

func (b *Button) IsPressed(button int32) bool {
	return rl.IsMouseButtonPressed(button) && rl.CheckCollisionPointRec(rl.GetMousePosition(), b.GetLayoutRec())
}

func (b *Button) Draw() {
	if b.textSize.X <= 0 {
		b.textSize = rl.MeasureTextEx(rl.GetFontDefault(), b.Text, float32(rl.GetFontDefault().BaseSize)*2, 1.0)
	}

	rl.DrawRectangleRec(b.GetLayoutRec(), b.Bg)
	rl.DrawText(b.Text, int32(b.GetLayoutRec().X+b.GetLayoutRec().Width/2-b.textSize.X/2), int32(b.GetLayoutRec().Y+b.GetLayoutRec().Height/2-b.textSize.Y/2), rl.GetFontDefault().BaseSize*2, b.Fg)
}

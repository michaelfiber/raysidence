package screens

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

func DrawChooseServer() {
	rl.DrawText("Choose the server you are controlling:", 10, 10, rl.GetFontDefault().BaseSize*3, rl.Green)
	for i, _ := range servers {

		name := "http://##############.local (192.168.1.###)"
		size := rl.MeasureTextEx(rl.GetFontDefault(), name, float32(rl.GetFontDefault().BaseSize*3), 2.0)
		var rec rl.Rectangle
		rec.X = float32(int32(rl.GetScreenWidth()/2) - int32(size.X)/2)
		rec.Y = float32(80 + i*40)
		rec.Width = float32(size.X)
		rec.Height = float32(size.Y)

		rl.DrawText(name, int32(rl.GetScreenWidth())/2-int32(size.X)/2, int32(80+i*40), rl.GetFontDefault().BaseSize*2, rl.Purple)

		rl.DrawRectangleLinesEx(rec, 4, rl.Purple)
	}
}

package screens

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

var (
	ActiveScreen Screen
)

type Screen struct {
	Floats  map[string]float32
	Ints    map[string]int
	Strings map[string]string
	Vecs    map[string]rl.Vector2
	Recs    map[string]rl.Rectangle
	Funcs   []func()
}

func (s *Screen) SetFuncs(funcs []func()) {
	s.Funcs = append(s.Funcs[:0], funcs...)
	fmt.Printf("Funcs: %v\n", s.Funcs)
}

func (s *Screen) runFunc(i int) {
	if i < len(s.Funcs) && s.Funcs[i] != nil {
		s.Funcs[i]()
	}
}

func (s *Screen) Create() {
	s.runFunc(0)
}

func (s *Screen) Update() {
	s.runFunc(1)
}

func (s *Screen) Draw() {
	s.runFunc(2)
}

func (s *Screen) Destroy() {
	s.runFunc(3)
}

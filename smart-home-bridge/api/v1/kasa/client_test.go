package kasa_test

import (
	"main/api/v1/kasa"
	"testing"
)

func TestClient(t *testing.T) {
	c := kasa.Client{}
	c.Init()

	t.Errorf("Output: %v\n", c)
}

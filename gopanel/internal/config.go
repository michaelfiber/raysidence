package internal

import "os"

var (
	Username = os.Getenv("HUE_BRIDGE_USERNAME")
	Server   = os.Getenv("HUE_BRIDGE_ID")
)

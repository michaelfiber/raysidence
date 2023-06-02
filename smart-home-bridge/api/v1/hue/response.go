package hue

type ApiItem struct {
	Error struct {
		ErrorType   int    `json:"type"`
		Address     string `json:"address"`
		Description string `json:"description"`
	} `json:"error"`

	Success struct {
		Username string `json:"username"`
	} `json:"success"`
}

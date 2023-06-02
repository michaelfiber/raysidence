package hue

type Group struct {
	Config   *Config
	GroupKey string

	Name    string   `json:"name"`
	Lights  []string `json:"lights"`
	Sensors []string `json:"sensors"`
	Type    string   `json:"type"`
	State   struct {
		AllOn bool `json:"all_on"`
		AnyOn bool `json:"any_on"`
	} `json:"state"`
	Recycle bool   `json:"recycle"`
	Class   string `json:"class"`
}

func (g *Group) Toggle() {
	if g.State.AnyOn {
		g.Off()
		return
	}
	g.On()
}

func (g *Group) Off() {
	for _, lightKey := range g.Lights {
		g.Config.Lights[lightKey].Off()
	}
}

func (g *Group) On() {
	for _, lightKey := range g.Lights {
		g.Config.Lights[lightKey].On()
	}
}

func (g *Group) Brighter() {
	for _, lightKey := range g.Lights {
		g.Config.Lights[lightKey].Brighter()
	}
}

func (g *Group) Darker() {
	for _, lightKey := range g.Lights {
		g.Config.Lights[lightKey].Darker()
	}
}

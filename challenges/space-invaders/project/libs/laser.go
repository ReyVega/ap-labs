package libs

import "github.com/faiface/pixel"

type Laser struct {
	pic       pixel.Picture
	pos       *pixel.Vec
	vel       float64
	isVisible bool
	world     *World
	sprite    *pixel.Sprite
}

func (l *Laser) GetLaserPos() *pixel.Vec {
	return l.pos
}

func NewBaseLaser(path string, vel float64, world *World) (*Laser, error) {
	pic, err := NewloadPicture(path)
	if err != nil {
		return nil, err
	}

	return &Laser{
		pic:   pic,
		vel:   vel,
		world: world,
	}, nil
}

func (l *Laser) NewLaser(pos pixel.Vec) *Laser {
	spr := pixel.NewSprite(l.pic, l.pic.Bounds())

	return &Laser{
		pos:       &pos,
		vel:       l.vel,
		sprite:    spr,
		isVisible: true,
		world:     l.world,
	}
}

func (l Laser) Draw(t pixel.Target) {
	if l.isVisible {
		l.sprite.Draw(t, pixel.IM.Moved(*l.pos))
	}
}

func (l *Laser) Update() {
	l.pos.Y += l.vel
	if l.pos.Y > l.world.height {
		l.isVisible = false
	}
}

package libs

import "github.com/faiface/pixel"

type EnemyLaser struct {
	pic       pixel.Picture
	pos       *pixel.Vec
	vel       float64
	isVisible bool
	world     *World
	sprite    *pixel.Sprite
}

func NewEnemyLaser(path string, vel float64, world *World) (*EnemyLaser, error) {
	pic, err := NewloadPicture(path)
	if err != nil {
		return nil, err
	}

	return &EnemyLaser{
		pic:   pic,
		vel:   vel,
		world: world,
	}, nil
}

func (l *EnemyLaser) NewEnemLaser(pos pixel.Vec) *EnemyLaser {
	spr := pixel.NewSprite(l.pic, l.pic.Bounds())

	return &EnemyLaser{
		pos:       &pos,
		vel:       l.vel,
		sprite:    spr,
		isVisible: true,
		world:     l.world,
	}
}

func (l EnemyLaser) Draw(t pixel.Target) {
	if l.isVisible {
		l.sprite.Draw(t, pixel.IM.Moved(*l.pos))
	}
}

func (l *EnemyLaser) Update() {
	l.pos.Y -= l.vel
	if l.pos.Y > l.world.height {
		l.isVisible = false
	}
}

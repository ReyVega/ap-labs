package libs

import (
	"github.com/faiface/pixel"
)

type Player struct {
	direction Direction
	world     *World
	sprite    *pixel.Sprite
	life      int
	pos       *pixel.Vec
	vel       float64
	laser     *Laser

	lasers map[string]*Laser
}

const (
	playerVel    = 250.0
	laserImg     = "assets/textures/world.png"
	laserVel     = 400.0
	rechargeTime = 20
)

var (
	laserDelay = rechargeTime
)

func NewPlayer(path string, life int, world *World) (*Player, error) {
	// Initialize sprite to use with the player
	pic, err := NewloadPicture(path)
	if err != nil {
		return nil, err
	}
	spr := pixel.NewSprite(pic, pic.Bounds())
	initialPos := pixel.V(world.Bounds().W()/2, 30)

	// Initialize the laser for the player
	l, err := NewBaseLaser(laserImg, 270.0, world)
	if err != nil {
		return nil, err
	}

	return &Player{
		life:   life,
		sprite: spr,
		world:  world,
		pos:    &initialPos,
		vel:    250.00,
		laser:  l,
		lasers: make(map[string]*Laser),
	}, nil
}

func (p Player) CheckFortress(coordenadasFortress [32]pixel.Vec, deadFortress [16]int) (coordenadasFortalezas [32]pixel.Vec, deadFortalezas [16]int) {
	for k, l := range p.lasers {
		l.Update()
		for i := 0; i < 16; i++ {
			if l.pos.X >= coordenadasFortress[i*2].X && l.pos.X <= coordenadasFortress[i*2+1].X && l.pos.Y >= coordenadasFortress[i*2].Y && l.pos.Y <= coordenadasFortress[i*2+1].Y {

				delete(p.lasers, k)
				deadFortress[i] = 1
				coordenadasFortress[i*2] = pixel.V(0, 0)
				coordenadasFortress[i*2+1] = pixel.V(0, 0)
			}
		}

	}
	return coordenadasFortress, deadFortress
}

func (p Player) Draw(t pixel.Target) {
	mat := pixel.IM
	mat = mat.Moved(*p.pos)
	mat = mat.Scaled(*p.pos, 0.10)
	p.sprite.Draw(t, mat)

	for _, l := range p.lasers {
		l.Draw(t)
	}
}

func (p *Player) Update(direction Direction, action Action, dt float64) {
	p.direction = direction
	p.move(direction, dt)
	p.shoot(action, dt)

	for k, l := range p.lasers {
		l.Update()

		// remove unused lasers
		if !l.isVisible {
			delete(p.lasers, k)
		}
	}
}

func (p *Player) move(direction Direction, dt float64) {
	switch direction {
	case LeftDirection:
		newX := p.pos.X - (p.vel * dt)
		if newX > 14 {
			p.pos.X = newX
		}

	case RightDirection:
		newX := p.pos.X + (p.vel * dt)
		if newX < p.world.Bounds().W()-14 {
			p.pos.X = newX
		}
	}
}

func (p *Player) shoot(action Action, dt float64) {

	if laserDelay >= 0 {
		laserDelay--
	}

	if action == ShootAction && laserDelay <= 0 {
		l := p.laser.NewLaser(*p.pos)
		l.vel *= dt
		p.lasers[NewULID()] = l
		laserDelay = rechargeTime
	}
}

func (p *Player) GetLife() int {
	return p.life
}

func (p *Player) SetLife(l int) {
	p.life = l
}

func (player *Player) CheckBulletPlayer(inv Invader) bool {
	var x int = 0
	var y int = 0

	for k, l := range inv.lasers {
		x = int(l.pos.X)
		y = int(l.pos.Y)

		if (int(player.pos.X)-20 < x && x < int(player.pos.X)+20) && (int(player.pos.Y)-12 < y && y < int(player.pos.Y)+12) {
			delete(inv.lasers, k)
			//print("HOLA")
			return true
		}
	}
	return false
}

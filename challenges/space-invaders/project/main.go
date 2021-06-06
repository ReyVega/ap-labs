package main

import (
	"fmt"
	_ "image/png"
	"log"
	spacegame "spaceInvaders/libs"
	"time"

	"github.com/faiface/pixel"
	"github.com/faiface/pixel/pixelgl"
	"github.com/faiface/pixel/text"
	"golang.org/x/image/colornames"
	"golang.org/x/image/font/basicfont"
)

const (
	windowWidth  = 800
	windowHeight = 600
)

var numberOfEnemies = 0
var numberOfLives = 0

func run() {
	cfg := pixelgl.WindowConfig{
		Title:  "Space Invaders",
		Bounds: pixel.R(0, 0, windowWidth, windowHeight),
		VSync:  true,
	}

	win, err := pixelgl.NewWindow(cfg)
	if err != nil {
		log.Fatal(err)
	}

	world := spacegame.NewWorld(windowWidth, windowHeight)
	if err := world.AddBackground("assets/textures/background.png"); err != nil {
		log.Fatal(err)
	}

	player, err := spacegame.NewPlayer("assets/textures/ship.png", numberOfLives, world)
	if err != nil {
		log.Fatal(err)
	}

	//50 is the standard for alien number
	enemies, err := spacegame.NewCreateEnemies(win, numberOfEnemies, world)
	if err != nil {
		log.Fatal(err)
	}

	var deadFortress [16]int
	coordenadasFortalezas := spacegame.NewCreateFortress(win, deadFortress)

	direction := spacegame.Idle
	last := time.Now()
	action := spacegame.NoneAction
	var isRunning bool = true
	var firstTime bool = true
	var gameOver bool = false
	var gameWon bool = false
	var cont int = 0
	var score int = 0
	var enemiesMovementX = true
	var enemiesMovementY = false
	basicAtlas := text.NewAtlas(basicfont.Face7x13, text.ASCII)

	for !win.Closed() {
		dt := time.Since(last).Seconds()
		last = time.Now()

		win.Clear(colornames.Black)
		world.Draw(win)

		if firstTime {

			logoImg, err := spacegame.NewloadPicture("assets/textures/logo.png")
			logo := pixel.NewSprite(logoImg, logoImg.Bounds())
			mat := pixel.IM
			mat = mat.Moved(pixel.V(win.Bounds().Center().X, win.Bounds().Center().Y+175))
			mat = mat.Scaled(pixel.V(win.Bounds().Center().X, win.Bounds().Center().Y), 0.5)
			logo.Draw(win, mat)
			if err != nil {
				panic(err)
			}

			tvMenu := text.New(pixel.V(windowWidth/2-150, windowHeight/2-175), basicAtlas)
			fmt.Fprintln(tvMenu, "Press ENTER to start")
			tvMenu.Draw(win, pixel.IM.Scaled(tvMenu.Orig, 2))
			if win.JustPressed(pixelgl.KeyEnter) {
				firstTime = false
			}
		}

		if isRunning && !firstTime {
			if win.Pressed(pixelgl.KeyLeft) {
				direction = spacegame.LeftDirection
			}

			if win.Pressed(pixelgl.KeyRight) {
				direction = spacegame.RightDirection
			}
			if win.Pressed(pixelgl.KeySpace) {
				action = spacegame.ShootAction
			}

			for i := 0; i < len(enemies); i++ {
				enemies[i].Draw(win)
				enemies[i].Update(enemiesMovementX, enemiesMovementY, dt)
				coordenadasFortalezas, deadFortress = enemies[i].CheckFortressInvaders(coordenadasFortalezas, deadFortress)
				var wasShot bool = enemies[i].CheckBullet(player)
				var lostLife bool = player.CheckBulletPlayer(enemies[i])
				var invaderToLimit bool = enemies[i].CheckLimitInvader()

				if wasShot {
					enemies = append(enemies[:i], enemies[i+1:]...)
					score += 25
					if len(enemies) <= 0 {
						gameWon = true
					}
				}

				if invaderToLimit {
					gameOver = true
				}

				if lostLife {
					player.SetLife(player.GetLife() - 1)
					if player.GetLife() <= 0 {
						gameOver = true
					}
				}
			}

			enemiesMovementY = false

			if enemiesMovementX {
				cont++
			} else {
				cont--
			}

			if cont == windowWidth/10 {
				enemiesMovementX = false
				enemiesMovementY = true
			}

			if cont == windowWidth/8*-1 {
				enemiesMovementX = true
				enemiesMovementY = true
			}
			spacegame.NewCreateFortress(win, deadFortress)
			coordenadasFortalezas, deadFortress = player.CheckFortress(coordenadasFortalezas, deadFortress)

			player.Update(direction, action, dt)
			player.Draw(win)
			direction = spacegame.Idle
			action = spacegame.NoneAction

			tvScore := text.New(pixel.V(20, 570), basicAtlas)
			tvLives := text.New(pixel.V(690, 570), basicAtlas)
			fmt.Fprintln(tvScore, "Score:", score)
			fmt.Fprintln(tvLives, "Lives:", player.GetLife())
			tvScore.Draw(win, pixel.IM.Scaled(tvScore.Orig, 1.5))
			tvLives.Draw(win, pixel.IM.Scaled(tvLives.Orig, 1.5))
		} else if !firstTime && !gameOver && !gameWon {
			tvPause := text.New(pixel.V(windowWidth/2-70, windowHeight/2), basicAtlas)
			fmt.Fprintln(tvPause, "Paused")
			tvPause.Draw(win, pixel.IM.Scaled(tvPause.Orig, 4))
		}

		if win.JustPressed(pixelgl.KeyP) && !firstTime {
			isRunning = !isRunning
		}

		if gameOver {
			isRunning = false
			overImg, err := spacegame.NewloadPicture("assets/textures/gameover.png")
			over := pixel.NewSprite(overImg, overImg.Bounds())
			mat := pixel.IM
			mat = mat.Moved(pixel.V(win.Bounds().Center().X, win.Bounds().Center().Y))
			over.Draw(win, mat)
			if err != nil {
				panic(err)
			}

			tvFinalScore := text.New(pixel.V(windowWidth/2-100, windowHeight/2-225), basicAtlas)
			fmt.Fprintln(tvFinalScore, "Final Score: ", score)
			tvFinalScore.Draw(win, pixel.IM.Scaled(tvFinalScore.Orig, 2))
		}

		if gameWon {
			isRunning = false
			wonImg, err := spacegame.NewloadPicture("assets/textures/gamewon2.png")
			won := pixel.NewSprite(wonImg, wonImg.Bounds())
			mat := pixel.IM
			mat = mat.Moved(pixel.V(win.Bounds().Center().X+125, win.Bounds().Center().Y))
			mat = mat.Scaled(pixel.V(win.Bounds().Center().X, win.Bounds().Center().Y), 0.9)
			won.Draw(win, mat)
			if err != nil {
				panic(err)
			}

			tvFinalScore := text.New(pixel.V(windowWidth/2+40, windowHeight/2-295), basicAtlas)
			fmt.Fprintln(tvFinalScore, "Final Score: ", score)
			tvFinalScore.Draw(win, pixel.IM.Scaled(tvFinalScore.Orig, 1.75))
		}
		win.Update()
	}
}

func main() {
	numberOfEnemies = 50
	numberOfLives = 10
	pixelgl.Run(run)
}

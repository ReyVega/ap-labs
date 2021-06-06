package libs

import (
	"github.com/faiface/pixel"
	"github.com/faiface/pixel/imdraw"
	"github.com/faiface/pixel/pixelgl"
	"golang.org/x/image/colornames"
)

func NewCreateFortress(win *pixelgl.Window, deadFortress [16]int) [32]pixel.Vec {
	//Setup 3x4x4
	var coordenadas [32]pixel.Vec
	contadorOtro := 0
	for i := 0; i < 2; i++ {
		for j := 0; j < 4; j++ {
			for k := 0; k < 2; k++ {
				imd := imdraw.New(nil)
				imd.Color = colornames.White
				if deadFortress[contadorOtro] == 1 {

				} else {
					imd.Push(pixel.V(float64(65+30+k*50+j*173), float64(80+i*50)))
					imd.Push(pixel.V(float64(95+30+k*50+j*173), float64(110+i*50)))
					coordenadas[contadorOtro*2] = pixel.V(float64(65+30+k*50+j*173), float64(80+i*50))
					coordenadas[contadorOtro*2+1] = pixel.V(float64(95+30+k*50+j*173), float64(110+i*50))

					imd.Rectangle(3)
					imd.Draw(win)
				}
				contadorOtro++

			}
		}
	}
	return coordenadas

}

package libs

import (
	"image"
	_ "image/png"
	"os"

	"github.com/faiface/pixel"
)

func NewloadPicture(path string) (pixel.Picture, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	img, _, err := image.Decode(f)
	if err != nil {
		return nil, err
	}
	return pixel.PictureDataFromImage(img), nil
}

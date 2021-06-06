deps:
	go get github.com/faiface/pixel
	go get github.com/faiface/pixel/imdraw
	go get github.com/faiface/pixel/pixelgl
	go get golang.org/x/image/colornames
	go get github.com/oklog/ulid/v2

build:
	go build main.go

test: deps build
	./main
	
clean:
	rm -rf main
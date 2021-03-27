// Copyright © 2016 Alan A. A. Donovan & Brian W. Kernighan.
// License: https://creativecommons.org/licenses/by-nc-sa/4.0/

// See page 156.

// Package geometry defines simple types for plane geometry.
//!+point
package main

import (
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
	"time"
)

type Point struct{ x, y float64 }

var p0 Point

// Getters
func (p Point) X() float64 {
	return p.x
}

func (p Point) Y() float64 {
	return p.y
}

// traditional function
func Distance(p, q Point) float64 {
	return math.Hypot(p.X()-q.X(), p.Y()-q.Y())
}

// same thing, but as a method of the Point type
func (p Point) Distance(q Point) float64 {
	return math.Hypot(p.X()-q.X(), p.Y()-q.Y())
}

//!-point

//!+path

// A Path is a journey connecting the points with straight lines.
type Path []Point

// Distance returns the distance traveled along the path.
func (path Path) Distance() float64 {
	sum := 0.0
	for i := range path {
		if i > 0 {
			sum += path[i-1].Distance(path[i])
		}
	}
	// Distance of last point with first point
	sum += path[len(path)-1].Distance(path[0])
	return sum
}

func orientation(p1, p2, p3 Point) int {
	var val float64 = (p2.y-p1.y)*(p3.x-p2.x) - (p2.x-p1.x)*(p3.y-p2.y)

	if val > 0 {
		// Clockwise orientation
		return 1
	} else if val < 0 {
		// Counterclockwise orientation
		return 2
	} else {
		// Colinear
		return 0
	}
}

func compare(p1, p2 *Point) bool {
	var o int = orientation(p0, *p1, *p2)
	if o == 0 {
		if Distance(p0, *p2) >= Distance(p0, *p1) {
			return false
		} else {
			return true
		}
	}

	if o == 2 {
		return false
	} else {
		return true
	}
}

func qsort(path Path) Path {
	if len(path) < 2 {
		return path
	}

	left, right := 0, len(path)-1

	// Pick a pivot
	pivotIndex := rand.Int() % len(path)

	// Move the pivot to the right
	path[pivotIndex], path[right] = path[right], path[pivotIndex]

	// Pile elements smaller than the pivot on the left
	for i := range path {
		if compare(&path[i], &path[right]) {
			path[i], path[left] = path[left], path[i]
			left++
		}
	}

	// Place the pivot after the last smaller element
	path[left], path[right] = path[right], path[left]

	// Go down the rabbit hole
	qsort(path[:left])
	qsort(path[left+1:])

	return path
}

func swap(p1, p2 *Point) {
	var temp Point = *p1
	*p1 = *p2
	*p2 = temp
}

func findClosedPath(path Path, n int) {
	var y_min float64 = path[0].y
	var min int = 0
	for i := 1; i < n; i++ {
		var y float64 = path[i].y
		if (y < y_min) || (y_min == y && path[i].x < path[min].x) {
			y_min = path[i].y
			min = i
		}
	}
	swap(&path[0], &path[min])
	p0 = path[0]
	qsort(path)

	fmt.Printf("- Generating a [%d] sides figure\n", n)
	fmt.Println("- Figure's vertices")

	// Note: only during the printing the vertices
	// have two decimals
	for i := 0; i < n; i++ {
		fmt.Printf(" - (  %.2f,  %.2f)\n", path[i].x, path[i].y)
	}

	fmt.Println("- Figure's Perimeter")
	// Distance of first line
	fmt.Printf(" - %f", path[0].Distance(path[1]))
	for i := 2; i < n; i++ {
		// Distance of all lines except last one
		fmt.Printf(" + %f", path[i-1].Distance(path[i]))
	}
	// Distance of last line
	fmt.Printf(" + %f", path[n-1].Distance(path[0]))
	// Perimeter
	fmt.Printf(" = %f\n", path.Distance())
}

func generateX() float64 {
	x_coordinate := (-100) + rand.Float64()*(100-(-100))
	return x_coordinate
}

func generateY() float64 {
	y_coordinate := (-100) + rand.Float64()*(100-(-100))
	return y_coordinate
}

func main() {
	var args []string = os.Args[1:]
	sides, _ := strconv.Atoi(args[0])

	if len(args) == 1 && sides > 2 {
		rand.Seed(time.Now().UnixNano())

		// Generate points
		var path Path = make(Path, sides)

		for i := 0; i < sides; i++ {
			path[i] = Point{x: generateX(), y: generateY()}
		}
		findClosedPath(path, sides)
	} else {
		fmt.Println("ERROR: sides must be greater 2")
	}
}

//!-path

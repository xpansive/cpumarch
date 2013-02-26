img = render
imgviewer = feh -.
cppflags = -c -std=c++11 -O3 -ffast-math -march=native -msse2 -Wall -Wextra -flto
ldflags = -lpthread -flto -O3 

$(scene) : $(scene).o cpumarch.o common.o
	g++ $(ldflags) cpumarch.o common.o $(scene).o -o $(scene)

$(scene).o : $(scene).cpp scene.h common.h
	g++ $(cppflags) $(scene).cpp -o $(scene).o

cpumarch.o : cpumarch.cpp scene.h common.h
	g++ $(cppflags) cpumarch.cpp -o cpumarch.o

common.o : common.cpp common.h
	g++ $(cppflags) common.cpp -o common.o

clean :
	rm -f cpumarch.o outline.o mandelbulb.o outline mandelbulb common.o

view : $(scene)
	./$(scene) | $(imgviewer) -

ppm : $(scene)
	./$(scene) > $(img).ppm

png : ppm
	pnm2png $(img).ppm $(img).png


CXX=mpicxx
CXXFLAGS=-std=c++17 -O2

all: a2

clean:
	rm -rf a2
run:
	mpiexec -n 4 ./a2 64

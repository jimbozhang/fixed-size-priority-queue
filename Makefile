.PHONY: all clean

all:
	g++ -g3 test.cc -o test -std=c++17

clean:
	rm test

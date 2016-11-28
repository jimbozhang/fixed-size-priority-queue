.PHONY: all clean

all:
	g++ -g test.cc -o test

clean:
	rm test

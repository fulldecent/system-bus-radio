all : gmain cmain

clean :
	rm -f gmain
	rm -f cmain

grun : gmain
	./gmain

crun : cmain
	./cmain


gmain : main.cpp
	g++ -Wall -O2 -std=c++11 -pthread -lrt -o gmain main.cpp

cmain : main.cpp
	clang++ -Wall -O2 -std=c++11 -stdlib=libc++ -pthread -lrt -o cmain main.cpp


.PHONY : all clean run

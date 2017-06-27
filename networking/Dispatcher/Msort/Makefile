NAME = msort.a

.PHONY: all build re

all: build
	gcc -c srcs/bettersort.c -o build/bettersort.o -I includes/ -I ../
	gcc -c srcs/msort.c -o build/msort.o -I includes/ -I ../
	gcc -c srcs/msort_helper.c -o build/msort_helper.o -I includes/ -I ../
	ar rc $(NAME) build/msort.o build/msort_helper.o build/bettersort.o

build:
	mkdir build

clean:
	rm -rf build/
	rm -f $(NAME)

re: clean all

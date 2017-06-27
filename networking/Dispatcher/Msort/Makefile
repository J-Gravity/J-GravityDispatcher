NAME = msort.a

.PHONY: all build re

all: build
	gcc -c srcs/msort.c -o build/msort.o -I includes/
	gcc -c srcs/msort_helper.c -o build/msort_helper.o -I includes/
	ar rc $(NAME) build/msort.o build/msort_helper.o

build:
	mkdir build

clean:
	rm -rf build/
	rm -f $(NAME)

re: clean all

all: worker

CCFLAGS += -D DEVICE=$(DEVICE)

tt:
	make -C ../TurboTranspose-master
lz4:
	make -C ../lz4-lib

worker: tt lz4
	gcc -o worker *.c ../TurboTranspose-master/transpose.o ../TurboTranspose-master/transpose_sse.o ../lz4-lib/liblz4.a -O3 -I ../TurboTranspose-master -I ../lz4-lib -framework OpenCL

clean:
	rm -f dispatcher
	make -C ../TurboTranspose-master clean
	make -C ../lz4-lib clean

re: clean all
all: worker

CCFLAGS += -D DEVICE=$(DEVICE)

tt:
	make -C ../TurboTranspose-master clean
	make -C ../TurboTranspose-master
lz4:
	make -C ../lz4-lib clean
	make -C ../lz4-lib

worker: tt lz4
	rm -f worker
	gcc -o worker *.c ../TurboTranspose-master/transpose.o ../TurboTranspose-master/transpose_sse.o ../lz4-lib/liblz4.a -O3 -I ../TurboTranspose-master -I ../lz4-lib -framework OpenCL -Wno-deprecated-declarations

quick:
	rm -f worker
	gcc -o worker *.c ../TurboTranspose-master/transpose.o ../TurboTranspose-master/transpose_sse.o ../lz4-lib/liblz4.a -O3 -I ../TurboTranspose-master -I ../lz4-lib -framework OpenCL -Wno-deprecated-declarations

amd:
	gcc -o worker *.c ../TurboTranspose-master/transpose.o ../TurboTranspose-master/transpose_sse.o ../lz4-lib/liblz4.a -O3 -I ../TurboTranspose-master -I ../lz4-lib -lopencl

clean:
	rm -f worker
	make -C ../TurboTranspose-master clean
	make -C ../lz4-lib clean

re: clean all

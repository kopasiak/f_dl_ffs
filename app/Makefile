objs = main.o aio_ep.o handle_ep.o cmd_stream.o render.o 640x480.o

CFLAGS = -Wall -O2 -c
LDFLAGS = -laio -flto

CFLAGS += `pkg-config --cflags opencv`
LDFLAGS += `pkg-config --libs opencv`

all : udl

udl : $(objs)
	g++ $(objs) -o udl $(LDFLAGS)

640x480.o : 640x480.bin
	ld -r -b binary -o 640x480.o 640x480.bin

render.o : render.cpp
	g++ $(CFLAGS) -c $< -o $@

%.o : %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf udl *.o

#makefile OpenGL

CC= gcc
CFLAGS= `pkg-config --cflags sdl2 glew`
LDFLAGS= `pkg-config --libs sdl2 glew`
CFLAGS+= -Wall -I include -c -std=c99
LDFLAGS+= -lm
SRCS= $(wildcard src/*.c)
BDIR= build
OBJS= $(addprefix $(BDIR)/obj/, $(notdir $(SRCS:.c=.o)))
EXEC= $(BDIR)/ogl

.PHONY: checkdeps

all: checkdeps $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

checkdeps:
  pkg-config sdl2 glew || (echo "Error: SDL2 and/or GLEW not found."; exit 1)

$(BDIR)/obj/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

CFLAGS += -MMD
-include $(OBJS:.o=.d)

clean:
	rm $(OBJS)


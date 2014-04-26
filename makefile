#makefile OpenGL

CC= gcc
CFLAGS= -Wall -I include -c -std=c99
LDFLAGS= -lSDL2 -lGL -lm
SRCS= $(wildcard src/*.c)
BDIR= build
OBJS= $(addprefix $(BDIR)/obj/, $(notdir $(SRCS:.c=.o)))
EXEC= $(BDIR)/ogl

$(EXEC): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

$(BDIR)/obj/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

CFLAGS += -MMD
-include $(OBJS:.o=.d)

clean:
	rm obj/*.o


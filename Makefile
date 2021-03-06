

NAME := pstree
SRCS := $(shell find . -maxdepth 2 -name "*.c")
DEPS := $(shell find . -maxdepth 2 -name "*.h") $(SRCS)
OBJ_DIR := ./build

CFLAGS += -O1 -ggdb -Wall -Wno-unused-result -Wno-unused-value -Wno-unused-variable

.PHONY: all clean debug test

all: $(NAME)-64 $(NAME)-32 test

$(NAME)-64: $(DEPS)
	gcc -m64 $(CFLAGS) $(SRCS) -o ./build/$@

$(NAME)-32: $(DEPS)
	gcc -m32 $(CFLAGS) $(SRCS) -o ./build/$@

test:
	./build/$(NAME)-64 -test

clean:
	rm -f *.out *.o ./build/$(NAME)-32 ./build/$(NAME)-64

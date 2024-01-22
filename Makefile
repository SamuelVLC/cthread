CC = gcc
CFLAGS = -Wall -fPIC -g -Wextra -pedantic  # Add extra warning flags
LDFLAGS = -shared

LIB_NAME = lib.so

SRCS = ctlist.c cthreads.c
OBJS = $(SRCS:%.c=output/%.o)

all: $(LIB_NAME)

$(LIB_NAME): $(OBJS)
	@mkdir -p clib
	$(CC) $(LDFLAGS) -o clib/$(LIB_NAME) $^

output/%.o: %.c
	@mkdir -p output
	$(CC) $(CFLAGS) -c -o $@ $<

clib: $(LIB_NAME)

clean:
	rm -f $(OBJS) clib/$(LIB_NAME)

# initrb

# Set the C compiler
CC ?= clang

# Set the CFLAGS and LDFLAGS
override CFLAGS := -Wall -std=c99 -g -D_GNU_SOURCE $(CFLAGS)
override LDFLAGS := $(LDFLAGS)

# Define includes and linker libs
INCLUDES = -Iinclude -I/usr/include/ruby-2.2.0 -I/usr/include/ruby-2.2.0/x86_64-linux -I/usr/include/readline
LIBS = -lm -lruby -lreadline

INITRB_SRCS = $(shell find src -type f -name '*.c')
INITRB_OBJS = $(INITRB_SRCS:.c=.o)

# Export variables for subprocesses
export PATH := ./build/lib:./build/bin:$(PATH)
export LD_LIBRARY_PATH := ./build/lib:$(LD_LIBRARY_PATH)

# Perform some basic platform detection
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	DL_EXT := .so
	BIN_EXT := 
	override CFLAGS := -fPIC $(CFLAGS)
else
	$(error Unknown development platform, we support Linux only)
endif

# Define the executable file
INITRB = build/bin/initrb$(BIN_EXT)

# Define `make all`, `make tests`, and `make test`.
all: $(INITRB)
initrb: $(INITRB)
.PHONY: clean

# Define the libinitrb-test target
$(INITRB): LDFLAGS += -Lbuild/lib
$(INITRB): $(LIBINITRB) $(INITRB_OBJS)
	@echo "LD $(INITRB)"
	@$(CC) $(CFLAGS) $(INITRB_CFLAGS) $(INCLUDES) -o $(INITRB) $(INITRB_OBJS) $(LDFLAGS) $(LIBS)

# Rule for translating .c into .o
.c.o:
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

# Rule for cleaning build files
clean:
	@echo "RM $(INITRB)"
	@rm -f $(INITRB)
	@echo "RM *.o"
	@find src -name '*.o' -exec rm -f {} \;

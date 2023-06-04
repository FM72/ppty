# Compiler options
CC := clang
CFLAGS := -Wall -Wextra -std=c99

# Output executable name
TARGET := ppty

# Source file
SRCS := ppty.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)


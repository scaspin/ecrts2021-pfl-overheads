CC = gcc

TARGET = main_rw

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin
INCLUDES = include

SRC = src/main_rw.c

OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ -O3  -D_GNU_SOURCE -pthread -g

all: $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -I$(INCLUDES) -c $< -o $@ -O3  -D_GNU_SOURCE -g

clean:
	@rm $(BUILD_DIR)/*.o
	@rm $(TARGET)


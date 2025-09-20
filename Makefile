# Detect operating system
UNAME_S := $(shell uname -s)

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 $(INCLUDES)

# Platform-specific libraries and include paths
ifeq ($(UNAME_S),Linux)
    LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    INCLUDES = 
endif
ifeq ($(UNAME_S),Darwin)
    LIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    RAYLIB_PATH = $(shell brew --prefix raylib)
    INCLUDES = -I$(RAYLIB_PATH)/include
    LIBS += -L$(RAYLIB_PATH)/lib
endif

# Source directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
ALGO_DIR = $(SRC_DIR)/algorithms
UI_DIR = $(SRC_DIR)/ui
UTILS_DIR = $(SRC_DIR)/utils
TEST_DIR = $(SRC_DIR)/tests

# Source files
CORE_SOURCES = $(CORE_DIR)/game.c $(CORE_DIR)/player.c
ALGO_SOURCES = $(ALGO_DIR)/algorithm.c $(ALGO_DIR)/bubble_sort.c $(ALGO_DIR)/selection_sort.c $(ALGO_DIR)/insertion_sort.c $(ALGO_DIR)/merge_sort.c $(ALGO_DIR)/quick_sort.c
UI_SOURCES = $(UI_DIR)/ui.c
UTILS_SOURCES = $(UTILS_DIR)/colors.c

# All source files
SOURCES = $(CORE_SOURCES) $(ALGO_SOURCES) $(UI_SOURCES) $(UTILS_SOURCES)

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Main executable
TARGET = discretia

# Test executable
TEST_TARGET = test_bubble_sort
TEST_SOURCES = $(TEST_DIR)/bubble_sort_test.c $(ALGO_SOURCES) $(UTILS_SOURCES)

.PHONY: all clean test platform-info

all: $(TARGET)

$(TARGET): $(OBJECTS) main.c
	$(CC) $(CFLAGS) -o $@ main.c $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o $@ $(TEST_SOURCES) $(LIBS)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_TARGET)

# Create main.c if it doesn't exist
main.c:
	@echo "Creating main.c..."
	@echo '#include "src/core/game.h"' > main.c
	@echo '#include "raylib.h"' >> main.c
	@echo '' >> main.c
	@echo 'int main(void) {' >> main.c
	@echo '    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Discretia - Algorithm Visualizer");' >> main.c
	@echo '    SetTargetFPS(60);' >> main.c
	@echo '' >> main.c
	@echo '    InitGame();' >> main.c
	@echo '' >> main.c
	@echo '    while (!WindowShouldClose()) {' >> main.c
	@echo '        UpdateGame();' >> main.c
	@echo '' >> main.c
	@echo '        BeginDrawing();' >> main.c
	@echo '        ClearBackground(WHITE);' >> main.c
	@echo '        RenderGame();' >> main.c
	@echo '        EndDrawing();' >> main.c
	@echo '    }' >> main.c
	@echo '' >> main.c
	@echo '    CleanupGame();' >> main.c
	@echo '    CloseWindow();' >> main.c
	@echo '    return 0;' >> main.c
	@echo '}' >> main.c

run: $(TARGET)
	./$(TARGET)

install-deps:
	@echo "Installing raylib dependencies..."
ifeq ($(UNAME_S),Linux)
	@echo "Detected Linux - using apt-get"
	sudo apt-get update
	sudo apt-get install -y build-essential git cmake
	sudo apt-get install -y libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev
endif
ifeq ($(UNAME_S),Darwin)
	@echo "Detected macOS - using Homebrew"
	@if ! command -v brew >/dev/null 2>&1; then \
		echo "Homebrew not found. Please install Homebrew first:"; \
		echo "/bin/bash -c \"\$$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""; \
	else \
		brew install raylib; \
	fi
endif

platform-info:
	@echo "Detected platform: $(UNAME_S)"
	@echo "Using libraries: $(LIBS)"
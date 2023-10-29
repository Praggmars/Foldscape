CC = g++
CFLAGS = -MMD -MP -std=c++20 -c -g -O2 -Wall `pkg-config --cflags gtk4`
LIBS = -lepoxy `pkg-config --libs gtk4`
PROJNAME = Foldscape
BUILD_DIR = build
SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS = Makefile $(wildcard *.h)

all: dir $(BUILD_DIR)/$(PROJNAME)

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(PROJNAME): $(OBJS) $(DEPS)
	$(CC) $(OBJS) $(LIBS) -o $(BUILD_DIR)/$(PROJNAME)

$(OBJS): $(BUILD_DIR)/%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
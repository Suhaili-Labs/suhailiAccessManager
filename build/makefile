# Makefile for building accessman

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -O2 -std=c++17

# Linker flags for fully static output.
LDFLAGS = -static -static-libstdc++ -static-libgcc

# FTXUI source and build cache (outside repository).
FTXUI_CACHE_DIR ?= $(HOME)/.cache/suhailiAccessManager/ftxui
FTXUI_SRC_DIR = $(FTXUI_CACHE_DIR)/src
FTXUI_BUILD_DIR = $(FTXUI_CACHE_DIR)/build
FTXUI_REPO_URL = https://github.com/ArthurSonzogni/FTXUI.git

# Source files
SOURCES = ../tui.cpp

# Object files
OBJECTS = $(addprefix obj/, $(notdir $(SOURCES:.cpp=.o)))

# Executable name
EXECUTABLE = accessman

# Static FTXUI archives required for dependency-free binary.
FTXUI_COMPONENT_A = $(FTXUI_BUILD_DIR)/libftxui-component.a
FTXUI_DOM_A = $(FTXUI_BUILD_DIR)/libftxui-dom.a
FTXUI_SCREEN_A = $(FTXUI_BUILD_DIR)/libftxui-screen.a

LIBS = $(FTXUI_COMPONENT_A) $(FTXUI_DOM_A) $(FTXUI_SCREEN_A) -lpthread -ldl

# Default target
all: $(EXECUTABLE)

.PHONY: all ensure_ftxui_static check_static_libs clean

# Build static FTXUI into cache when missing.
ensure_ftxui_static:
	@mkdir -p "$(FTXUI_CACHE_DIR)"
	@if [ ! -d "$(FTXUI_SRC_DIR)/.git" ]; then \
		echo "Cloning FTXUI into cache: $(FTXUI_SRC_DIR)"; \
		git clone --depth 1 "$(FTXUI_REPO_URL)" "$(FTXUI_SRC_DIR)"; \
	fi
	@cmake -S "$(FTXUI_SRC_DIR)" -B "$(FTXUI_BUILD_DIR)" \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DFTXUI_BUILD_EXAMPLES=OFF \
		-DFTXUI_BUILD_TESTS=OFF
	@cmake --build "$(FTXUI_BUILD_DIR)" -j$$(nproc)

# Dependency check for static archives.
check_static_libs:
	@test -f $(FTXUI_COMPONENT_A) || (echo "Missing static library: $(FTXUI_COMPONENT_A)" && exit 1)
	@test -f $(FTXUI_DOM_A) || (echo "Missing static library: $(FTXUI_DOM_A)" && exit 1)
	@test -f $(FTXUI_SCREEN_A) || (echo "Missing static library: $(FTXUI_SCREEN_A)" && exit 1)

# Rule to link object files to create the executable
$(EXECUTABLE): ensure_ftxui_static check_static_libs $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

# Rule to compile source files into object files
obj/%.o: ../%.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target to remove generated files
clean:
	rm -rf obj $(EXECUTABLE)

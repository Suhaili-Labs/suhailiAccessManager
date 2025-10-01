# Makefile for building accessman

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -static

# Source files
SOURCES = ../tui.cpp ../accessman.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = accessman

# Libraries
LIBS = /usr/lib/libftxui-component.a /usr/lib/libftxui-dom.a /usr/lib/libftxui-screen.a

# Default target
all: $(EXECUTABLE)

# Rule to link object files to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS) $(CXXFLAGS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CXX) -c $< -o $@

# Clean target to remove generated files
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

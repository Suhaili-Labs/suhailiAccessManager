# Makefile for building accessman

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS =

# Source files
SOURCES = ../tui.cpp ../accessman.cpp

# Object files
OBJECTS = $(addprefix obj/, $(notdir $(SOURCES:.cpp=.o)))

# Executable name
EXECUTABLE = accessman

# Libraries
LIBS = -lftxui-component -lftxui-dom -lftxui-screen

# Default target
all: $(EXECUTABLE)

# Rule to link object files to create the executable
$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS) $(CXXFLAGS)

# Rule to compile source files into object files
obj/%.o: ../%.cpp
	@mkdir -p obj
	$(CXX) -c $< -o $@

# Clean target to remove generated files
clean:
	rm -rf obj $(EXECUTABLE)

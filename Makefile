# raindrop Makefile

EXEC = raindrop

CXXFLAGS = -g -std=c++14
LDFLAGS  = -lcurl

# All C++ source files of the project
CXXFILES   = $(shell find -maxdepth 1 -type f -name '*.cpp')
CXXOBJECTS = $(CXXFILES:.cpp=.o)

SOURCES = $(CXXFILES)
OBJECTS = $(CXXOBJECTS)

all: $(EXEC)
	# Build successful!

$(EXEC): $(OBJECTS)
	# Linking...
	$(CXX) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

src/%.o: src/%.cpp
	# Compiling $<...
	$(CXX) $(CXXFLAGS) $(INCLUDE) $< -c -o $@

src/%.o: src/%.c
	# Compiling $<...
	$(CC) $(CFLAGS) $(INCLUDE) $< -c -o $@

run: all
	./$(EXEC)

clean:
	# Cleaning...
	rm -f $(EXEC) *.o

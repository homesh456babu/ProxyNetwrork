# Compiler
CXX = g++

# Flags
CXXFLAGS = -Wall -Wextra -std=c++17 -Iheaderfiles

# Directories
SRC_DIR = sourcefiles

# Source & object files
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:.cpp=.o)

# Output binary
TARGET = proxy_server

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $(TARGET)

# Compile rule (IMPORTANT: this line MUST start with a TAB)
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run
run: $(TARGET)
	./$(TARGET)

# Clean
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

.PHONY: all run clean

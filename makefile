CC := g++
FLAGS := -std=c++11 -w -g
INC_DIR := include
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := .
OUTPUT_DIR := data/output/*
INCLUDE := -I./$(INC_DIR)
 
$(BIN_DIR)/main: $(BUILD_DIR)/main.o $(BUILD_DIR)/find_modules.o $(BUILD_DIR)/simplify.o $(BUILD_DIR)/tools.o
	@mkdir -p $(BIN_DIR)
	@$(CC) $(FLAGS) $(INCLUDE) $^ -o $@
 
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(FLAGS) $(INCLUDE) -c -o $@ $<
 
clean:
	@rm -rf $(OUTPUT_DIR)

f:
	@rm -rf $(OUTPUT_DIR)
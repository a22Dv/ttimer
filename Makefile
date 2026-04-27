NAME := ttimer
CXX := g++
CXX_DFLAGS := -std=c++23 -Og -g -fno-rtti -Wall -Wpedantic -Wshadow -Werror 
CXX_RFLAGS := -std=c++23 -O2 -s -fno-rtti -Wall -Wpedantic -Wshadow -Werror -DNDEBUG
CXX_RLDFLAGS := -flto

DEBUG_DIR := build/debug
RELEASE_DIR := build/release

INCLUDE_DIR := include
SRC_DIR := src

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
INCLS := $(wildcard $(INCLUDE_DIR)/*.hpp)

DEBUG_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(SRCS))
RELEASE_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(SRCS))

.PHONY: all debug release clean

all: debug release 
debug: $(DEBUG_DIR)/$(NAME) 
release: $(RELEASE_DIR)/$(NAME) 
clean:
	rm -rf $(DEBUG_DIR) $(RELEASE_DIR)

$(DEBUG_DIR)/$(NAME): $(DEBUG_OBJS)
	$(CXX) $^ -o $@

$(RELEASE_DIR)/$(NAME): $(RELEASE_OBJS)
	$(CXX) $^ -o $@ $(CXX_RLDFLAGS)

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLS) | $(DEBUG_DIR)
	$(CXX) -c $(CXX_DFLAGS) -I$(INCLUDE_DIR) $< -o $@

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLS) | $(RELEASE_DIR)
	$(CXX) -c $(CXX_RFLAGS) -I$(INCLUDE_DIR) $< -o $@

$(DEBUG_DIR):
	mkdir -p $(DEBUG_DIR)

$(RELEASE_DIR):
	mkdir -p $(RELEASE_DIR)


	





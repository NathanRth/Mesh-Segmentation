CXX ?= g++

# path #
SRC_PATH = src
BUILD_PATH = bin
RELEASE_PATH = $(BUILD_PATH)/release
DEBUG_PATH = $(BUILD_PATH)/debug

# executable # 
BIN_NAME = segmenteur.out

# extensions #
SRC_EXT = cpp

# code lists #
# Find all source files in the source directory, sorted by
# most recently modified
SOURCES = $(shell find $(SRC_PATH) -name '*.$(SRC_EXT)' | sort -k 1nr | cut -f2-)
# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.$(SRC_EXT)=$(BUILD_PATH)/%.o)
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)

# flags #
COMPILE_FLAGS = -std=c++11
RELEASE_FLAGS = -o2
DEBUG_FLAGS = -Wall -Wextra -g
INCLUDES = -I include/ -I /usr/local/include -I /usr/include/GLFW -I/usr/include
# Space-separated pkg-config libraries used by this project
LIBS = 

.PHONY: default_target
default_target: debug

.PHONY: release
release: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(RELEASE_FLAGS)
release: export BIN_PATH := $(BIN_PATH) $(RELEASE_PATH)
release: dirs
	@$(MAKE) all

.PHONY: debug
debug: export CXXFLAGS := $(CXXFLAGS) $(COMPILE_FLAGS) $(DEBUG_FLAGS)
debug: export BIN_PATH := $(BIN_PATH) $(DEBUG_PATH)
debug: dirs
	@$(MAKE) all

.PHONY: dirs
dirs:
	@echo "\033[0;32mCreating directories\033[0;0m"
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

.PHONY: clean
clean:
	@echo "\033[0;33mDeleting $(BIN_NAME) symlink\033[0;0m"
	@$(RM) $(BIN_NAME)
	@echo "\033[0;33mDeleting directories\033[0;0m"
	@$(RM) -r $(BUILD_PATH)
	@$(RM) -r $(BIN_PATH)

# checks the executable and symlinks to the output
.PHONY: all
all: $(BIN_PATH)/$(BIN_NAME)
	@echo "\033[0;33mMaking symlink: $(BIN_NAME) -> $<\033[0;0m"
	@$(RM) $(BIN_NAME)
	@ln -s $(BIN_PATH)/$(BIN_NAME) $(BIN_NAME)

# Creation of the executable
$(BIN_PATH)/$(BIN_NAME): $(OBJECTS)
	@echo "\033[0;32mLinking: $@\033[0;0m"
	$(CXX) $(OBJECTS) $(LIBS) -o $@

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
# After the first compilation they will be joined with the rules from the
# dependency files to provide header dependencies
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.$(SRC_EXT)
	@echo "\033[0;32mCompiling: $< -> $@\033[0;0m"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@
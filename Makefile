CMAKE ?= cmake
CTEST ?= ctest
BUILD_DIR ?= build
CONFIG ?= Release
CMAKE_FLAGS ?=

.PHONY: all build run run-windowed test clean

all: build

build:
	$(CMAKE) -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE=$(CONFIG) $(CMAKE_FLAGS)
	$(CMAKE) --build "$(BUILD_DIR)" --config $(CONFIG) --parallel

run: build
	"./$(BUILD_DIR)/conveyor_factory"

run-windowed: build
	"./$(BUILD_DIR)/conveyor_factory" --windowed

test: build
	$(CTEST) --test-dir "$(BUILD_DIR)" -C $(CONFIG) --output-on-failure

clean:
	$(CMAKE) -E rm -rf "$(BUILD_DIR)"

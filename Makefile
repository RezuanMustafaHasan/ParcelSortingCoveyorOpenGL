PATH := /opt/homebrew/bin:/usr/local/bin:$(PATH)
export PATH

CMAKE ?= $(shell which cmake 2>/dev/null || echo /opt/homebrew/bin/cmake)

.PHONY: all build run run-windowed clean

all: build

build:
	$(CMAKE) -S . -B build
	$(CMAKE) --build build -j

run: build
	./build/conveyor_factory

run-windowed: build
	./build/conveyor_factory --windowed

clean:
	rm -rf build

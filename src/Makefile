# Convenience commands for humans. CMake remains the authoritative build graph.

ROOT := $(abspath $(CURDIR)/..)
PRESET ?= dev-debug
BUILD_DIR := $(ROOT)/build/cmake/$(PRESET)

# MSYS make does not pass TMP/TEMP through to its children. The native-Windows
# toolchain then falls back to C:\WINDOWS\, which is not writable, and both GCC
# and the test suite fail with "Cannot create temporary file". Guarantee a
# writable one, as a Windows-form path since the tools are native binaries.
SCRATCH := $(ROOT)/build/tmp
WIN_SCRATCH := $(shell cygpath -w "$(SCRATCH)" 2>/dev/null || echo "$(SCRATCH)")
export TMP := $(if $(strip $(TMP)),$(TMP),$(WIN_SCRATCH))
export TEMP := $(if $(strip $(TEMP)),$(TEMP),$(TMP))

# Tests that read real game data need an installation to point at.
# Override on the command line:  make test GAME_DIR="C:/Games/Lemmings Revolution"
GAME_DIR ?= C:/progs/Lemmings Revolution

ifeq ($(OS),Windows_NT)
LRTOOL := $(BUILD_DIR)/lrtool.exe
else
LRTOOL := $(BUILD_DIR)/lrtool
endif

.DEFAULT_GOAL := build
.PHONY: build configure test run js info dump png lvlmap clean help

build: configure
	@cd "$(ROOT)" && cmake --build --preset "$(PRESET)"

configure:
	@mkdir -p "$(SCRATCH)"
	@if [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		cd "$(ROOT)" && cmake --preset "$(PRESET)" -DWEBOLUTION_GAME_DIR="$(GAME_DIR)"; \
	fi

run: build
	@"$(BUILD_DIR)/webolution" || "$(BUILD_DIR)/webolution.exe"

# WebAssembly build; needs an activated Emscripten SDK (EMSDK set).
js:
	@if [ -z "$$EMSDK" ]; then echo "error: activate the Emscripten SDK so EMSDK is set" >&2; exit 1; fi
	@cd "$(ROOT)" && cmake --preset emscripten-release && cmake --build --preset emscripten-release

test: build
	@cd "$(ROOT)" && ctest --preset "$(PRESET)"

# Summarise the archives in a Lemmings Revolution installation.
info: build
	@"$(LRTOOL)" info "$(GAME_DIR)"

# Extract everything to build/assets for inspection.
dump: build
	@"$(LRTOOL)" dump "$(GAME_DIR)" "$(ROOT)/build/assets"

# Decode every texture to a PNG for inspection (~380 MB).
png: build
	@"$(LRTOOL)" png "$(GAME_DIR)" "$(ROOT)/build/png"

# Unroll a level's cylinder surface to a PNG for inspection.
#   make lvlmap LEVEL=MM_CLIMB_002
LEVEL ?= MM_CLIMB_002
lvlmap: build
	@mkdir -p "$(ROOT)/build/lvlmap"
	@"$(LRTOOL)" lvlmap "$(GAME_DIR)" "$(LEVEL)" "$(ROOT)/build/lvlmap/$(LEVEL).png"

clean:
	@rm -rf "$(ROOT)/build/cmake"

help:
	@echo "make build     compile (preset: $(PRESET))"
	@echo "make test      compile and run the test suite"
	@echo "make run       build and run the game"
	@echo "make js        build the WebAssembly target (needs EMSDK)"
	@echo "make info      summarise the game archives"
	@echo "make dump      extract every archive entry to build/assets"
	@echo "make png       decode every texture to PNG in build/png"
	@echo "make lvlmap    unroll a level's surface to PNG (LEVEL=name)"
	@echo "make clean     remove the cmake build tree"
	@echo ""
	@echo "GAME_DIR=$(GAME_DIR)"

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
.PHONY: build configure test info dump clean help

build: configure
	@cd "$(ROOT)" && cmake --build --preset "$(PRESET)"

configure:
	@mkdir -p "$(SCRATCH)"
	@if [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		cd "$(ROOT)" && cmake --preset "$(PRESET)" -DWEBOLUTION_GAME_DIR="$(GAME_DIR)"; \
	fi

test: build
	@cd "$(ROOT)" && ctest --preset "$(PRESET)"

# Summarise the archives in a Lemmings Revolution installation.
info: build
	@"$(LRTOOL)" info "$(GAME_DIR)"

# Extract everything to build/assets for inspection.
dump: build
	@"$(LRTOOL)" dump "$(GAME_DIR)" "$(ROOT)/build/assets"

clean:
	@rm -rf "$(ROOT)/build/cmake"

help:
	@echo "make build     compile (preset: $(PRESET))"
	@echo "make test      compile and run the test suite"
	@echo "make info      summarise the game archives"
	@echo "make dump      extract every archive entry to build/assets"
	@echo "make clean     remove the cmake build tree"
	@echo ""
	@echo "GAME_DIR=$(GAME_DIR)"

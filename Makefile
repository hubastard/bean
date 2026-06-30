# Bean — macOS Cocoa wrappers in pure C

ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD := $(ROOT)/build
INCLUDE := $(ROOT)/include
EXAMPLES := $(ROOT)/examples
CODEGEN := $(ROOT)/tools/codegen
SDK := $(or $(SDKROOT),$(shell xcrun --show-sdk-path))
PYTHON := $(ROOT)/.venv/bin/python3
PIP := $(ROOT)/.venv/bin/pip

CFLAGS := -std=c99 -Wall -Wextra -Werror \
	-I$(INCLUDE) -I$(EXAMPLES) \
	-isysroot $(SDK) \
	-iframework $(SDK)/System/Library/Frameworks \
	-F $(SDK)/System/Library/Frameworks

LDFLAGS := -lobjc

# Codegen options forwarded to bean_gen.py, e.g.:
#   make generate GEN_ARGS="--framework foundation --framework appkit"
GEN_ARGS ?=

# Example name for: make example NAME=appkit  (or: make example appkit)
NAME ?=
EXAMPLE ?=

# Allow: make example appkit
ifeq ($(firstword $(MAKECMDGOALS)),example)
  EXAMPLE := $(or $(EXAMPLE),$(word 2,$(MAKECMDGOALS)))
  ifneq ($(EXAMPLE),)
    $(eval $(EXAMPLE):;@:)
  endif
endif

# Allow: make examples metal  (common typo for make example metal)
ifeq ($(firstword $(MAKECMDGOALS)),examples)
  EXAMPLE := $(or $(EXAMPLE),$(word 2,$(MAKECMDGOALS)))
  ifneq ($(EXAMPLE),)
    $(eval $(EXAMPLE):;@:)
  endif
endif

EXAMPLE_NAMES := $(patsubst $(EXAMPLES)/%/main.c,%,$(wildcard $(EXAMPLES)/*/main.c))
EXAMPLE_NAME := $(or $(NAME),$(EXAMPLE))

include $(EXAMPLES)/frameworks.mk

SMOKE_LDFLAGS := -framework Foundation -framework AppKit -framework CoreGraphics

.PHONY: help venv generate test example examples clean
.DEFAULT_GOAL := help

help:
	@echo "Bean make targets:"
	@echo ""
	@echo "  make venv                         Create Python venv + install codegen deps"
	@echo "  make generate [GEN_ARGS=\"...\"]    Regenerate headers (default: all frameworks)"
	@echo "  make test                         Run unit tests, smoke tests, examples, header checks"
	@echo "  make example NAME=<name>          Build and run one example"
	@echo "  make example <name>               Same as above (e.g. make example appkit)"
	@echo "  make examples                     List available examples"
	@echo "  make examples <name>              Same as make example <name>"
	@echo "  make clean                        Remove build artifacts"
	@echo ""
	@echo "Examples: $(EXAMPLE_NAMES)"
	@echo ""
	@echo "Generate one framework:"
	@echo "  make generate GEN_ARGS=\"--framework foundation\""

venv: $(PYTHON)

$(PYTHON): $(CODEGEN)/requirements.txt
	python3 -m venv $(ROOT)/.venv
	$(PIP) install -q -r $(CODEGEN)/requirements.txt

generate: venv
	cd $(CODEGEN) && PYTHONPATH=. $(PYTHON) bean_gen.py --sdk "$(SDK)" $(GEN_ARGS)

examples:
ifneq ($(EXAMPLE),)
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) example $(EXAMPLE)
else
	@printf '%s\n' $(EXAMPLE_NAMES)
endif

example:
	@test -n "$(EXAMPLE_NAME)" || ( \
		echo "Usage: make example NAME=appkit   (or: make example appkit)" >&2; \
		echo "Examples: $(EXAMPLE_NAMES)" >&2; \
		exit 1)
	@test -f "$(EXAMPLES)/$(EXAMPLE_NAME)/main.c" || ( \
		echo "Unknown example: $(EXAMPLE_NAME)" >&2; \
		echo "Examples: $(EXAMPLE_NAMES)" >&2; \
		exit 1)
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $(BUILD)/example_$(EXAMPLE_NAME)
	@echo "Running example/$(EXAMPLE_NAME)..."
	@$(BUILD)/example_$(EXAMPLE_NAME)

$(BUILD)/example_%: $(EXAMPLES)/%/main.c $(EXAMPLES)/frameworks.mk | $(BUILD)
	@fw='$(EXAMPLE_FRAMEWORKS_$*)'; \
	if [ -z "$$fw" ]; then \
		echo "No frameworks listed for example '$*' in examples/frameworks.mk" >&2; \
		exit 1; \
	fi; \
	set --; \
	for name in $$fw; do set -- "$$@" -framework "$$name"; done; \
	echo "Building example/$*..."; \
	sect=""; \
	if [ -f "$(EXAMPLES)/$*/Info.plist" ]; then \
		sect="-sectcreate __TEXT __info_plist $(EXAMPLES)/$*/Info.plist"; \
	fi; \
	xcrun clang $(CFLAGS) "$(EXAMPLES)/$*/main.c" $$sect "$$@" $(LDFLAGS) -o "$@"; \
	if [ -f "$(EXAMPLES)/$*/sample.pdf" ]; then \
		cp "$(EXAMPLES)/$*/sample.pdf" "$(BUILD)/sample.pdf"; \
	fi; \
	if [ "$*" = "avfoundation" ]; then \
		cp "$(EXAMPLES)/avfoundation/sample.wav" "$(BUILD)/sample.wav"; \
	fi

$(BUILD)/test_foundation: $(ROOT)/tests/smoke/test_foundation.c | $(BUILD)
	@echo "Building smoke test: foundation..."
	xcrun clang $(CFLAGS) $< $(SMOKE_LDFLAGS) $(LDFLAGS) -o $@

$(BUILD)/test_appkit: $(ROOT)/tests/smoke/test_appkit.c | $(BUILD)
	@echo "Building smoke test: appkit..."
	xcrun clang $(CFLAGS) $< $(SMOKE_LDFLAGS) $(LDFLAGS) -o $@

$(BUILD)/examples.stamp: $(patsubst %,$(EXAMPLES)/%/main.c,$(EXAMPLE_NAMES)) \
                         $(EXAMPLES)/frameworks.mk | $(BUILD)
	@for ex in $(EXAMPLE_NAMES); do \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $(BUILD)/example_$$ex; \
	done
	@touch $@

test: venv $(BUILD)/test_foundation $(BUILD)/test_appkit $(BUILD)/examples.stamp
	@echo "SDK: $(SDK)"
	@xcrun clang --version | head -1
	@echo "Running apinotes unit tests..."
	@cd $(CODEGEN) && PYTHONPATH=. $(PYTHON) test_apinotes.py
	@echo "Running docs unit tests..."
	@cd $(CODEGEN) && PYTHONPATH=. $(PYTHON) test_docs.py
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) _test_headers
	@echo "All tests passed."

_test_headers:
	@failed=0; \
	for fw in $$(cd $(CODEGEN) && PYTHONPATH=. $(PYTHON) -c "from bean.config import GENERATION_ORDER; print(' '.join(GENERATION_ORDER))"); do \
		header="$(INCLUDE)/bean/$$fw.h"; \
		if [ ! -f "$$header" ]; then \
			echo "Skipping bean/$$fw.h (not generated)"; \
			continue; \
		fi; \
		echo "Checking bean/$$fw.h..."; \
		err="$(BUILD)/$$fw.header.errors"; \
		if ! xcrun clang $(CFLAGS) -fsyntax-only -x c -include "bean/$$fw.h" /dev/null \
			2>"$$err"; then \
			sed -n '1,8p' "$$err" >&2; \
			failed=1; \
		else \
			rm -f "$$err"; \
		fi; \
	done; \
	if [ "$$failed" -ne 0 ]; then exit 1; fi

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

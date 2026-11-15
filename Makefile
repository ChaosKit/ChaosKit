PRESET ?= debug
DIFF_RANGE ?=

CPP_LINT := .agents/skills/qt-cpp-review/references/lint-scripts/qt_review_lint.py
QML_LINT := .agents/skills/qt-qml-review/references/lint-scripts/qt_qml_lint.py
PORTABILITY_LINT := scripts/cpp_portability_lint.py

define resolve_diff_range
if [ -n "$(DIFF_RANGE)" ]; then \
	echo "$(DIFF_RANGE)"; \
elif ! git diff --cached --quiet; then \
	echo "--cached"; \
else \
	echo "HEAD"; \
fi
endef

.PHONY: configure build test test-unit lint lint-cpp lint-qml format-check format hooks

configure:
	@if [ -f .envrc ]; then . ./.envrc; fi; \
	cmake --preset "$(PRESET)"

build:
	@if [ -f .envrc ]; then . ./.envrc; fi; \
	cmake --build --preset "$(PRESET)"

test:
	ctest --preset "$(PRESET)"

test-unit:
	ctest --preset "$(PRESET)" -LE gpu

lint: lint-cpp lint-qml format-check

lint-cpp:
	@RANGE=$$($(resolve_diff_range)); \
	STATUS=0; \
	if [ -f "$(CPP_LINT)" ]; then \
		python3 "$(CPP_LINT)" --summary --diff="$$RANGE -- . ':!.agents/'" || STATUS=1; \
	fi; \
	if [ -f "$(PORTABILITY_LINT)" ]; then \
		python3 "$(PORTABILITY_LINT)" --summary --compile-commands="build/$(PRESET)/compile_commands.json" --diff="$$RANGE -- . ':!.agents/'" || STATUS=1; \
	fi; \
	exit $$STATUS

lint-qml:
	@if [ -f .envrc ]; then . ./.envrc; fi; \
	RANGE=$$($(resolve_diff_range)); \
	if [ -f "$(QML_LINT)" ]; then \
		python3 "$(QML_LINT)" --summary --qmllint --diff="$$RANGE -- . ':!.agents/'"; \
	fi

format-check:
	@RANGE=$$($(resolve_diff_range)); \
	STATUS=0; \
	CPP_FILES=$$(git diff --name-only --diff-filter=ACMR "$$RANGE" -- '*.cpp' '*.h' '*.hpp' ':!.agents/' ':!.idea/'); \
	if [ -n "$$CPP_FILES" ]; then \
		if ! command -v clang-format >/dev/null 2>&1; then \
			echo "clang-format not found on PATH" >&2; exit 1; \
		fi; \
		clang-format --dry-run --Werror $$CPP_FILES || STATUS=1; \
	fi; \
	CMAKE_FILES=$$(git diff --name-only --diff-filter=ACMR "$$RANGE" -- 'CMakeLists.txt' '**/CMakeLists.txt' '*.cmake' ':!.agents/' ':!.idea/'); \
	if [ -n "$$CMAKE_FILES" ]; then \
		if ! command -v cmake-format >/dev/null 2>&1; then \
			echo "cmake-format not found on PATH" >&2; exit 1; \
		fi; \
		cmake-format --check $$CMAKE_FILES || STATUS=1; \
	fi; \
	exit $$STATUS

format:
	@if command -v clang-format >/dev/null 2>&1; then \
		git ls-files -z '*.cpp' '*.h' '*.hpp' ':!.agents/' ':!.idea/' | xargs -0 clang-format -i; \
	else \
		echo "clang-format not found on PATH" >&2; exit 1; \
	fi
	@if command -v cmake-format >/dev/null 2>&1; then \
		git ls-files -z 'CMakeLists.txt' '**/CMakeLists.txt' '*.cmake' ':!.agents/' ':!.idea/' | xargs -0 cmake-format -i; \
	else \
		echo "cmake-format not found on PATH" >&2; exit 1; \
	fi

hooks:
	git config core.hooksPath .githooks

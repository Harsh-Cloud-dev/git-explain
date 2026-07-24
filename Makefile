CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src

# ── Source Files ───────────────────────────────────────────────────────────────
SRC = \
	src/main.cpp \
	src/executor/executor.cpp \
	src/detection/detector.cpp \
	src/errors/error_types.cpp \
	src/recovery/recovery.cpp \
	src/history/history.cpp \
	src/ui/ui.cpp

TESTS = \
	tests/test_main.cpp \
	tests/test_detection.cpp \
	tests/test_recovery.cpp

# ── Executable Names ───────────────────────────────────────────────────────────
TARGET = git-explain
TEST_TARGET = run_tests

# ── Build Main Program ─────────────────────────────────────────────────────────
$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# ── Build Tests ────────────────────────────────────────────────────────────────
$(TEST_TARGET):
	$(CXX) $(CXXFLAGS) $(TESTS) \
	src/detection/detector.cpp \
	src/errors/error_types.cpp \
	src/recovery/recovery.cpp \
	-o $(TEST_TARGET)

# ── Default Target ─────────────────────────────────────────────────────────────
all: $(TARGET) $(TEST_TARGET)

# ── Run Tests ──────────────────────────────────────────────────────────────────
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# ── Clean ──────────────────────────────────────────────────────────────────────
clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all test clean

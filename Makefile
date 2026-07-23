CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I./src

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

TARGET = git-explain
TEST_TARGET = run_tests

$(TARGET):
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(TEST_TARGET):
	$(CXX) $(CXXFLAGS) $(TESTS) \
	src/detection/detector.cpp \
	src/errors/error_types.cpp \
	src/recovery/recovery.cpp \
	-o $(TEST_TARGET)

all: $(TARGET) $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all test clean

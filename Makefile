CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

SRCS := main.cpp
TARGET := kuwahara

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) new_cat.png

.PHONY: all clean
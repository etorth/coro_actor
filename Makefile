CXX := g++-14
STD := c++26
TARGET := a.out

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPENDS := $(OBJS:.o=.d)

DEBUG_FLAGS := -g3 -DDEBUG -O0 -fno-omit-frame-pointer
WARNINGS := -Wall -Wextra -Wpedantic

ifeq ($(ENABLE_ASAN), 1)
    DEBUG_FLAGS += -fsanitize=address
endif

CXXFLAGS := $(WARNINGS) $(DEBUG_FLAGS) -std=$(STD) -MMD -MP -pthread

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPENDS)

clean:
	rm -f $(TARGET) *.o *.d

commit: clean
	git add .
	git commit -m f

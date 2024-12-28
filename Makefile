TARGET = $(BUILD_DIR)/centi
CXX_FLAGS = -g -Og -fno-exceptions -fno-rtti -Iinclude -Iinclude/sl
CXX_SRCS = src/Main.cpp src/NanoPrintf.cpp src/HostLinux.cpp \
	src/editor/Buffer.cpp src/editor/Editor.cpp \
	src/editor/Command.cpp src/editor/BuiltinCommands.cpp
LD_FLAGS =

BUILD_DIR = .build
OBJS = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(CXX_SRCS))

.PHONY: all
all: $(TARGET)

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: debug
debug: $(TARGET)
	gdb $(TARGET)

.PHONY: clean
clean:
	-rm -r $(BUILD_DIR)
	-rm -r $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LD_FLAGS) -o $(TARGET)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -c $< -o $@

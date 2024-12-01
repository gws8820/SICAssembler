TARGET = SICASM
OBJ = Assembler.o Loader.o OpTab.o SymTab.o main.o

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)  # macOS
    CC = clang
    CFLAGS = -Wall -g
else ifeq ($(UNAME_S), Linux)  # Linux
    CC = gcc
    CFLAGS = -Wall -g
else ifeq ($(OS), Windows_NT)  # Windows
    CC = gcc
    CFLAGS = -Wall -g
else
    $(error Unsupported platform: $(UNAME_S))
endif

all: $(TARGET) clean_obj

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean_obj:
ifeq ($(OS), Windows_NT)
	del /F /Q $(OBJ)
else
	rm -f $(OBJ)
endif

clean:
ifeq ($(OS), Windows_NT)
	del /F /Q $(OBJ) $(TARGET).exe
else
	rm -f $(OBJ) $(TARGET)
endif
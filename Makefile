TARGET = SICASM
OBJ = Assembler.o Loader.o OpTab.o SymTab.o main.o

ifdef OS
    ifeq ($(OS), Windows_NT)  # Windows
        CC = gcc
        CFLAGS = -Wall -g
        RM = del /F /Q
        EXE = .exe
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)  # macOS
        CC = clang
        CFLAGS = -Wall -g
        RM = rm -f
        EXE =
    else ifeq ($(UNAME_S), Linux)  # Linux
        CC = gcc
        CFLAGS = -Wall -g
        RM = rm -f
        EXE =
    else
        $(error Unsupported platform: $(UNAME_S))
    endif
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@$(EXE) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	$(RM) $(OBJ) $(TARGET)$(EXE)
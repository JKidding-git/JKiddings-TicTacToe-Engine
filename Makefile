# Portable Makefile — works on Linux, macOS, Windows (make under MSYS/MinGW/Cygwin)
CC ?= gcc
CFLAGS ?= -O2
what_avx_is_this_os := $(shell grep -m1 -o 'avx2\|avx' /proc/cpuinfo 2>/dev/null)


# Detect OS to set executable extension and add aggressive flags on Unix-like systems
ifeq ($(OS),Windows_NT)
	EXEEXT := .exe
else
	UNAME_S := $(shell uname -s 2>/dev/null)
	ifneq (,$(findstring Linux,$(UNAME_S)))
		ifneq (,$(findstring avx2,$(what_avx_is_this_os)))
			CFLAGS += -mavx2
		else ifneq (,$(findstring avx,$(what_avx_is_this_os)))
			CFLAGS += -mavx
		endif
		CFLAGS += -Ofast -march=native
	endif
	ifneq (,$(findstring Darwin,$(UNAME_S)))
		# macOS: don't use -march=native/-mavx2 by default
		CFLAGS += -Ofast
	endif
	EXEEXT :=
endif

# Optional: enable save-temp flags only when using gcc
SAVE_TEMP_FLAGS :=
ifneq (,$(findstring gcc,$(shell $(CC) --version 2>/dev/null)))
	SAVE_TEMP_FLAGS := -save-temps -fverbose-asm $(CFLAGS)
endif

SRCS := src/main.c
GEN_SRCS := src/Generator/generate_lookup_table.c

.PHONY: all clean

all: main$(EXEEXT) generator$(EXEEXT)

main$(EXEEXT): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

generator$(EXEEXT): $(GEN_SRCS)
	$(CC) $(CFLAGS) -o $@ $^
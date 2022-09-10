TARGET = splatpost

DEBUG   ?= 0
ARCH    ?= arm64
SDK     ?= macosx

SYSROOT  := $(shell xcrun --sdk $(SDK) --show-sdk-path)
ifeq ($(SYSROOT),)
$(error Could not find SDK "$(SDK)")
endif
CLANG    := clang
CC       := $(CLANG) -isysroot $(SYSROOT) -arch $(ARCH)

#CFLAGS  = -O1 -Wall -g -fstack-protector-all -fsanitize=address -fsanitize=float-divide-by-zero -fsanitize=leak
#LDFLAGS = -fsanitize=address -fsanitize=float-divide-by-zero -static-libsan -fsanitize=leak

CFLAGS  = -O1 -Wall -g -fstack-protector-all
LDFLAGS = -L. -lusbd

ifneq ($(DEBUG),0)
DEFINES += -DDEBUG=$(DEBUG)
endif

FRAMEWORKS = -framework CoreFoundation -framework IOKit

SOURCES = main.c

HEADERS = libusbd.h bmp.h HIDReportData.h switch_hid.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(FRAMEWORKS) $(DEFINES) $(LDFLAGS) -o $@ $(SOURCES)
	codesign -s - $@

clean:
	rm -f -- $(TARGET)

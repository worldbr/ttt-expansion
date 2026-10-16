TARGET = ttt_expansion
OBJS = src/main.o src/overlay.o src/toggle.o
CFLAGS = -O2 -Wall -Wextra -std=gnu11
LDFLAGS = -nostartfiles
BUILD_PRX = 1
PRX_EXPORTS = exports.exp
LIBS = -lpspdisplay -lpspctrl -lpspiofilemgr -lpspuser
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

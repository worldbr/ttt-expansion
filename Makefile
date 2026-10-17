TARGET = ttt_expansion
OBJS = src/main.o src/overlay.o src/toggle.o src/menu_state.o src/input_guard.o src/input_hook.o src/input_plan.o src/framecheck.o src/gpu_probe.o src/callsite_plan.o src/frame_hook.o
CFLAGS = -O2 -G0 -Wall -Wextra -std=gnu11
LDFLAGS = -nostartfiles
BUILD_PRX = 1
PRX_EXPORTS = exports.exp
LIBS = -lpspgu -lpspge -lpspdisplay -lpspctrl -lpspuser
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

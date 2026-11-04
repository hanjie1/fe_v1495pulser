DEBUG ?= 1
QUIET ?= 1

ifeq ($(QUIET),1)
Q := @
endif

ifdef CODA_VME
INC_CODA := -I$(CODA_VME)/include
LIB_CODA := -L$(CODA_VME_LIB)
endif

LINUXVME_LIB ?= ../../lib
LINUXVME_INC ?= ../../include
LINUXVME_BIN ?= ../../bin

TARGET := v1495firmware
SRC := v1495firmware.c v1495.c
OBJS := $(SRC:.c=.o)
PULSER_LIB := libv1495pulser.a
PULSER_SRC := v1495_pulser.c
PULSER_OBJS := $(PULSER_SRC:.c=.o)

CPPFLAGS := -DLinux_vme -include jvme.h -I. -I$(LINUXVME_INC) $(INC_CODA)
CFLAGS := -Wall
LDFLAGS := -L$(LINUXVME_LIB) $(LIB_CODA)
LDLIBS := -ljvme -lrt

ifeq ($(DEBUG),1)
CFLAGS += -g
else
CFLAGS += -O2
endif

DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d
DEPFILES := $(SRC:%.c=$(DEPDIR)/%.d) $(PULSER_SRC:%.c=$(DEPDIR)/%.d)

.PHONY: all clean install

all: $(TARGET) $(PULSER_LIB)

$(TARGET): $(OBJS)
	@echo " LD     $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(PULSER_LIB): $(PULSER_OBJS)
	@echo " AR     $@"
	$(Q)$(AR) rcs $@ $(PULSER_OBJS)

%.o: %.c | $(DEPDIR)
	@echo " CC     $@"
	$(Q)$(CC) $(DEPFLAGS) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(DEPDIR):
	@mkdir -p $@

install: $(TARGET)
	@echo " CP     $(TARGET)"
	$(Q)cp $(TARGET) $(LINUXVME_BIN)/

clean:
	$(Q)rm -f $(TARGET) $(OBJS) $(PULSER_OBJS) $(PULSER_LIB) $(DEPFILES)

-include $(wildcard $(DEPFILES))

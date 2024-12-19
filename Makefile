#
# make the IRIG Decoder and Test
#

CFLAGS = -MD $(ADDED_CFLAGS)

ifeq ($(IRIG_CONFIG_H),)
    ifneq ($(IRIG_SAMPLES),)
        IRIG_CONFIG_H = irigConfig$(IRIG_SAMPLES).h
    endif
endif
ifneq ($(IRIG_SAMPLES),)
    CFLAGS += -DIRIG_SAMPLES=$(IRIG_SAMPLES)
endif
ifneq ($(IRIG_CONFIG_H),)
    CFLAGS += -DIRIG_CONFIG_H=\"$(IRIG_CONFIG_H)\"
endif

OBJS = irigTest.o irig.o
DEPS = $(OBJS:.o=.d)

irigTest: $(OBJS)

clean:
	rm -f $(OBJS) $(DEPS) irigTest.exe irigTest

-include $(DEPS)


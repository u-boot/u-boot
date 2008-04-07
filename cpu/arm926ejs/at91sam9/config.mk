PLATFORM_CPPFLAGS += -march=armv5te
PLATFORM_CPPFLAGS += $(call cc-option,-mtune=arm926ejs,)

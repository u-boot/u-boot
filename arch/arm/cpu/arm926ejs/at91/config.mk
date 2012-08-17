PF_CPPFLAGS_TUNE := $(call cc-option,-mtune=arm926ejs,)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_TUNE)

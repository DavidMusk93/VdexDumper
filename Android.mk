LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_LDLIBS += -lz -llog
LOCAL_CFLAGS += -fPIE
LOCAL_CPPFLAGS += -std=c++17
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_MODULE := vdexdumper
SRCS := $(wildcard *.cc)
LOCAL_SRC_FILES := $(SRCS)
# $(warning $(LOCAL_SRC_FILES))

include $(BUILD_EXECUTABLE)

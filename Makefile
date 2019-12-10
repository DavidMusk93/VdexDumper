all: vdexdumper

SECURE_DIR := $(shell bash helper.sh 0)

.PHONY: vdexdumper

vdexdumper:
	ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk
	adb push ./libs/arm64-v8a/vdexdumper ${SECURE_DIR}

.PHONY: pull

pull:
	@mkdir -p adb
	adb pull ${SECURE_DIR}/data ./adb/
	adb pull ${SECURE_DIR}/system ./adb/

.PHONY: clean

clean:
	@rm -rf ./libs ./obj ./adb

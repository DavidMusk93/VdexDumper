all: vdexdumper

SECURE_DIR := $(shell bash helper.sh 0)
OUTPUT_DIR := ./adb

.PHONY: vdexdumper

vdexdumper:
	ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk
	adb push ./libs/arm64-v8a/vdexdumper ${SECURE_DIR}

.PHONY: pull

pull:
	@mkdir -p ${OUTPUT_DIR}
	adb pull ${SECURE_DIR}/data ${OUTPUT_DIR}/
	adb pull ${SECURE_DIR}/system ${OUTPUT_DIR}/

.PHONY: clean

clean:
	@rm -rf ./libs ./obj ${OUTPUT_DIR}

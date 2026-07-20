SHELL := /bin/sh

BUILD_DIR := build
BIN_DIR := bin
NATIVE_CXX ?= g++
NATIVE_CXXFLAGS ?= -std=c++14 -O2 -Wall -Wextra -Werror -Iinclude
NATIVE_CC ?= gcc
NATIVE_CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Werror -Iinclude
CROSS_COMPILE ?= /opt/linux/x86-arm/aarch64-mix210-linux/bin/aarch64-mix210-linux-
TARGET_CXX := $(CROSS_COMPILE)g++
TARGET_CC := $(CROSS_COMPILE)gcc
REPO_ROOT := $(abspath ../..)
SDK_ROOT ?= $(REPO_ROOT)/在线仓库/SS928V100_SDK_V2.0.2.2_MPP_Sample-master
ACL_INCLUDE := $(SDK_ROOT)/include/hisilicon/npu
ACL_STUB_DIR := $(SDK_ROOT)/lib/linux/hisilicon/npu/stub
TARGET_CXXFLAGS ?= -std=c++14 -O2 -Wall -Wextra -Werror -fPIE -mcpu=cortex-a53 -Iinclude -I$(SDK_ROOT)/include/hisilicon -I$(ACL_INCLUDE)
TARGET_LDFLAGS ?= -pie -L$(ACL_STUB_DIR) -Wl,-rpath-link,$(ACL_STUB_DIR) -lascendcl -lpthread -lm -ldl

.PHONY: all tests om_inspect prepare_input test_model_info test_core test_backend test_preprocess test_decode test_acl_contract test_pipeline offline raw_diagnostics live clean

all: tests om_inspect offline

tests: test_model_info test_core test_backend test_preprocess test_decode test_acl_contract test_pipeline

$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

$(BUILD_DIR)/test_model_tensor_info: tests/test_model_tensor_info.cpp src/model_tensor_info.cpp include/model_tensor_info.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) tests/test_model_tensor_info.cpp src/model_tensor_info.cpp -o $@

test_model_info: $(BUILD_DIR)/test_model_tensor_info
	./$(BUILD_DIR)/test_model_tensor_info

$(BUILD_DIR)/test_core: tests/test_core.c src/vot.c include/vot.h | $(BUILD_DIR)
	$(NATIVE_CC) $(NATIVE_CFLAGS) tests/test_core.c src/vot.c -lm -o $@

test_core: $(BUILD_DIR)/test_core
	./$(BUILD_DIR)/test_core

$(BUILD_DIR)/test_backend: tests/test_backend.cpp src/vot_backend.cpp include/vot_backend.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) tests/test_backend.cpp src/vot_backend.cpp -o $@

test_backend: $(BUILD_DIR)/test_backend
	./$(BUILD_DIR)/test_backend

$(BUILD_DIR)/test_preprocess: tests/test_preprocess.cpp src/frame_preprocess.cpp include/frame_preprocess.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) tests/test_preprocess.cpp src/frame_preprocess.cpp -o $@

test_preprocess: $(BUILD_DIR)/test_preprocess
	./$(BUILD_DIR)/test_preprocess

$(BUILD_DIR)/test_yolov8_decode: tests/test_yolov8_decode.cpp src/yolov8_postprocess.cpp src/frame_preprocess.cpp src/vot_backend.cpp include/yolov8_postprocess.h include/frame_preprocess.h include/vot_backend.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) tests/test_yolov8_decode.cpp src/yolov8_postprocess.cpp src/frame_preprocess.cpp src/vot_backend.cpp -o $@

test_decode: $(BUILD_DIR)/test_yolov8_decode
	./$(BUILD_DIR)/test_yolov8_decode

$(BIN_DIR)/om_inspect: tools/om_inspect.cpp src/model_tensor_info.cpp include/model_tensor_info.h | $(BIN_DIR)
	$(TARGET_CXX) $(TARGET_CXXFLAGS) tools/om_inspect.cpp src/model_tensor_info.cpp -o $@ $(TARGET_LDFLAGS)

om_inspect: $(BIN_DIR)/om_inspect

$(BIN_DIR)/current_raw_runner: diagnostics/current_raw_runner.cpp src/ss928_acl_detector.cpp src/model_tensor_info.cpp include/ss928_acl_detector.h include/model_tensor_info.h | $(BIN_DIR)
	$(TARGET_CXX) $(TARGET_CXXFLAGS) diagnostics/current_raw_runner.cpp src/ss928_acl_detector.cpp src/model_tensor_info.cpp -o $@ $(TARGET_LDFLAGS)

$(BIN_DIR)/reference_raw_runner: diagnostics/reference_raw_runner.cpp | $(BIN_DIR)
	$(TARGET_CXX) $(TARGET_CXXFLAGS) diagnostics/reference_raw_runner.cpp -o $@ $(TARGET_LDFLAGS)

raw_diagnostics: $(BIN_DIR)/current_raw_runner $(BIN_DIR)/reference_raw_runner

$(BUILD_DIR)/prepare_input: tools/prepare_input.cpp src/frame_preprocess.cpp include/frame_preprocess.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) tools/prepare_input.cpp src/frame_preprocess.cpp -o $@

prepare_input: $(BUILD_DIR)/prepare_input

$(BUILD_DIR)/test_acl_contract: tests/test_acl_contract.cpp src/ss928_acl_detector.cpp src/model_tensor_info.cpp include/ss928_acl_detector.h include/model_tensor_info.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) -DSS928_ACL_CONTRACT_ONLY tests/test_acl_contract.cpp src/ss928_acl_detector.cpp src/model_tensor_info.cpp -o $@

test_acl_contract: $(BUILD_DIR)/test_acl_contract
	./$(BUILD_DIR)/test_acl_contract

$(BUILD_DIR)/test_vot_pipeline: tests/test_vot_pipeline.cpp src/vot_pipeline.cpp src/vot_backend.cpp src/vot.c include/vot_pipeline.h include/vot_backend.h include/vot.h | $(BUILD_DIR)
	$(NATIVE_CXX) $(NATIVE_CXXFLAGS) tests/test_vot_pipeline.cpp src/vot_pipeline.cpp src/vot_backend.cpp src/vot.c -lm -o $@

test_pipeline: $(BUILD_DIR)/test_vot_pipeline
	./$(BUILD_DIR)/test_vot_pipeline

TARGET_CPP_SOURCES := src/main.cpp src/model_tensor_info.cpp src/ss928_acl_detector.cpp src/frame_preprocess.cpp src/yolov8_postprocess.cpp src/vot_backend.cpp src/vot_pipeline.cpp
TARGET_CPP_OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/target_%.o,$(TARGET_CPP_SOURCES))

$(BUILD_DIR)/target_%.o: src/%.cpp | $(BUILD_DIR)
	$(TARGET_CXX) $(TARGET_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/target_vot.o: src/vot.c include/vot.h | $(BUILD_DIR)
	$(TARGET_CC) -std=c99 -O2 -Wall -Wextra -Werror -fPIE -Iinclude -c $< -o $@

$(BIN_DIR)/ss928_yolov8_vot: $(TARGET_CPP_OBJECTS) $(BUILD_DIR)/target_vot.o | $(BIN_DIR)
	$(TARGET_CXX) $(TARGET_CPP_OBJECTS) $(BUILD_DIR)/target_vot.o -o $@ $(TARGET_LDFLAGS)

offline: $(BIN_DIR)/ss928_yolov8_vot

# The camera code path remains phase-gated until the offline real-detection
# acceptance test passes.  This target deliberately builds the same AArch64
# program; scripts/run_camera.sh enforces the runtime gate.
live: offline

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

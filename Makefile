TARGET := iphone:clang:latest:15.0
ARCHS := arm64

INSTALL_TARGET_PROCESSES := SpringBoard

# The package metadata is kept in .deb-staging/DEBIAN/control in this project.
THEOS_LAYOUT_DIR_NAME := .deb-staging

include $(THEOS)/makefiles/common.mk

TWEAK_NAME := Il2cppDumper

# Keep the source list in sync with the project tree. The demo translation unit
# is intentionally omitted; it is not used by the menu and adds considerable
# compile time and binary size.
Il2cppDumper_FILES := $(shell find Il2ccpDumper -type f \( \
	-name '*.c' -o -name '*.m' -o -name '*.mm' -o -name '*.cpp' \
\) ! -name 'imgui_demo.cpp' | sort)

Il2cppDumper_FRAMEWORKS := Foundation UIKit Metal MetalKit Security
Il2cppDumper_CFLAGS := -Wno-unused-variable -I$(THEOS_PROJECT_DIR)/Il2ccpDumper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/ImGui -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/wrapper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/apple_priv -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/support
Il2cppDumper_CCFLAGS := -std=c++20 -Wno-nontrivial-memcall -Wno-unused-function -Wno-uninitialized-const-pointer -Wno-macro-redefined -Wno-deprecated-declarations -Wno-deprecated-anon-enum-enum-conversion -I$(THEOS_PROJECT_DIR)/Il2ccpDumper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/ImGui -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/wrapper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/apple_priv -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/support
Il2cppDumper_OBJCFLAGS := -fobjc-arc -Wno-unused-variable -I$(THEOS_PROJECT_DIR)/Il2ccpDumper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/ImGui -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/wrapper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/apple_priv -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/support
Il2cppDumper_OBJCXXFLAGS := -fobjc-arc -std=c++20 -Wno-nontrivial-memcall -Wno-unused-function -Wno-uninitialized-const-pointer -Wno-macro-redefined -I$(THEOS_PROJECT_DIR)/Il2ccpDumper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/ImGui -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/wrapper -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/apple_priv -I$(THEOS_PROJECT_DIR)/Il2ccpDumper/lib/libSupport/support

include $(THEOS_MAKE_PATH)/tweak.mk

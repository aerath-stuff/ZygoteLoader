NDK_PATH := $(ANDROID_HOME)/ndk/29.0.14206865
.DEFAULT_GOAL := all

CCA := $(NDK_PATH)/toolchains/llvm/prebuilt/linux-x86_64/bin/clang

# Zygisk related variables
ZYGISK_FILES := src/native/dex.c src/native/main.c

# Java related variables
PLATFORM_VERSION := 36
BUILD_TOOLS_VERSION := 36.1.0
D8 := $(ANDROID_HOME)/build-tools/$(BUILD_TOOLS_VERSION)/d8

MODULE_ID                 ?= zygote_loader_example
MODULE_NAME               ?= ZygoteLoader Example
MODULE_AUTHOR             ?= v7878
MODULE_DESC               ?= An example Zygisk module
MODULE_ENTRY              ?= com.example.zygote_loader.EntryPoint
MODULE_ARCHIVE            ?= ZygoteLoader.zip
MODULE_UPDATE_JSON        ?= https://example.com/zygote_loader_update.json
MODULE_ATTACH_NATIVE_LIBS ?= false

clean:
	@echo Cleaning build artifacts...
	@rm -rf build module/module.prop module/classes.dex module/zygisk

all:
	@echo Creating build directory...
	@mkdir -p build

	@echo Building Zygisk library...
	@$(CCA) --target=aarch64-linux-android25 -shared -fPIC $(ZYGISK_FILES) -Isrc/native/ -o build/libzloader-arm64-v8a.so
	@$(CCA) --target=armv7a-linux-androideabi25 -shared -fPIC $(ZYGISK_FILES) -Isrc/native/ -o build/libzloader-armeabi-v7a.so
	@$(CCA) --target=x86_64-linux-android25 -shared -fPIC $(ZYGISK_FILES) -Isrc/native/ -o build/libzloader-x86_64.so
	@$(CCA) --target=i686-linux-android25 -shared -fPIC $(ZYGISK_FILES) -Isrc/native/ -o build/libzloader-x86.so

	@echo Building Java classes.dex...
	@javac -d build/classes src/java/*.java -cp $(ANDROID_HOME)/platforms/android-$(PLATFORM_VERSION)/android.jar
	@jar -cf build/classes.jar -C build/classes .
	@echo Dexing to classes.dex...
	@$(D8) --output build --lib $(ANDROID_HOME)/platforms/android-$(PLATFORM_VERSION)/android.jar build/classes.jar
	@echo Cleaning up...
	@rm -rf build/classes build/classes.jar

	@echo Creating module.prop...
	@printf '%s\n' \
		'id=$(MODULE_ID)' \
		'name=$(MODULE_NAME)' \
		'author=$(MODULE_AUTHOR)' \
		'description=$(MODULE_DESC)' \
		'entrypoint=$(MODULE_ENTRY)' \
		'updateJson=$(MODULE_UPDATE_JSON)' \
		'attachNativeLibs=$(MODULE_ATTACH_NATIVE_LIBS)' \
	> module/module.prop

	@echo Transfering files to module directory...
	@cp build/classes.dex module/
	@mkdir -p module/zygisk
	@cp build/libzloader-arm64-v8a.so module/zygisk/arm64-v8a.so
	@cp build/libzloader-armeabi-v7a.so module/zygisk/armeabi-v7a.so
	@cp build/libzloader-x86_64.so module/zygisk/x86_64.so
	@cp build/libzloader-x86.so module/zygisk/x86.so

	@echo Creating module archive...
	@cd module && zip -r ../$(MODULE_ARCHIVE) . > /dev/null

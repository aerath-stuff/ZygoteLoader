#include "main.hpp"

#include "logger.hpp"
#include "process.hpp"
#include "dex.hpp"
#include "raii.hpp"

#include <jni.h>

#include <unistd.h>
#include <fcntl.h>

#include <string.h> // NOLINT(*-deprecated-headers)

void ZygoteLoaderModule::onLoad(zygisk::Api *_api, JNIEnv *_env) {
    api = _api;
    env = _env;

    LOGD("Request dlclose module");
    api->setOption(zygisk::DLCLOSE_MODULE_LIBRARY);
}

const char *get_package_name(const char *data_dir, const char *process_name) {
    struct stat st; // NOLINT(*-pro-type-member-init)
    // check data dir is accessible for current uid
    if (stat(data_dir, &st) == -1) {
        LOGD("skip injecting into %s - failed to stat data directory", process_name);
        return nullptr;
    }
    const char *last_slash = strrchr(data_dir, '/');
    const char *package_name;
    if (last_slash != nullptr) {
        package_name = last_slash + 1;
        LOGD("Package name for global process: %s", package_name);
    } else {
        LOGD("Failed to parse package name from app_data_dir: %s", data_dir);
        return nullptr;
    }
    return package_name;
}

void ZygoteLoaderModule::preAppSpecialize(zygisk::AppSpecializeArgs *args) {
    RAIIPtr process_name = get_string_data(env, args->nice_name);
    RAIIPtr data_dir = get_string_data(env, args->app_data_dir);

    if (process_name == nullptr || data_dir == nullptr) {
        LOGD("skip injecting into %d because its process_name or app_data_dir is null", args->uid);
        return;
    }

    const char *package_name = get_package_name(data_dir, process_name);
    if (!package_name) {
        return;
    }

    RAIIFD module_dir = api->getModuleDir(); // keep alive during preSpecialize
    tryLoadDex(module_dir, package_name, process_name);
    callJavaPreSpecialize();
}

void ZygoteLoaderModule::postAppSpecialize(const zygisk::AppSpecializeArgs *args) {
    callJavaPostSpecialize();
}

void ZygoteLoaderModule::preServerSpecialize(zygisk::ServerSpecializeArgs *args) {
    RAIIFD module_dir = api->getModuleDir(); // keep alive during preSpecialize
    tryLoadDex(module_dir, PACKAGE_NAME_SYSTEM_SERVER, PROCESS_NAME_SYSTEM_SERVER);
    callJavaPreSpecialize();
}

void ZygoteLoaderModule::postServerSpecialize(const zygisk::ServerSpecializeArgs *args) {
    callJavaPostSpecialize();
}

bool testPackage(int packages_dir, const char *name) {
    return faccessat(packages_dir, name, F_OK, 0) == 0;
}

bool shouldEnable(int module_dir, const char *package_name) {
    RAIIFD<true> packages_dir = openat(module_dir, "packages", O_PATH | O_DIRECTORY);
    if (!packages_dir.isValid()) return false;
    return testPackage(packages_dir, package_name) ^
           testPackage(packages_dir, ALL_PACKAGES_NAME);
}

void ZygoteLoaderModule::tryLoadDex(
        int module_dir, const char *package_name, const char *process_name) {
    if (!shouldEnable(module_dir, package_name)) {
        return;
    }

    LOGD("Loading in %s", package_name);

    RAIIFile dex(module_dir, "classes.dex");

    entrypoint = (jclass) env->NewGlobalRef(
            dex_load_and_init(
                    env, package_name, process_name, module_dir,
                    dex.data, dex.length
            )
    );
}

void ZygoteLoaderModule::callJavaPreSpecialize() {
    if (entrypoint != nullptr) {
        call_pre_specialize(env, entrypoint);
    }
}

void ZygoteLoaderModule::callJavaPostSpecialize() {
    if (entrypoint != nullptr) {
        call_post_specialize(env, entrypoint);

        env->DeleteGlobalRef(entrypoint);
        entrypoint = nullptr;
    }
}

REGISTER_ZYGISK_MODULE(ZygoteLoaderModule)

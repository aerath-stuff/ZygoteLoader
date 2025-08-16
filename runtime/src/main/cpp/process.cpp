#include "process.hpp"
#include <string.h> // NOLINT(*-deprecated-headers)

void process_get_name(JNIEnv *env, jstring jprocess_name, char **process_name) {
    const char *str = env->GetStringUTFChars(jprocess_name, nullptr);
    *process_name = strdup(str);
    env->ReleaseStringUTFChars(jprocess_name, str);
}

void process_fix_package_name(char *process_name) {
    char *split = strchr(process_name, ':');
    if (split != nullptr) {
        *split = '\0';
    }
}

// TODO: process subname?

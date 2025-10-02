#pragma once

#include <jni.h>
#include <stdint.h> // NOLINT(*-deprecated-headers)

jclass dex_load_and_init(JNIEnv *env, const char *package_name,
                         const char *process_name, int module_dir,
                         const void *dex_block, uint32_t dex_length);

void call_pre_specialize(JNIEnv *env, jclass entrypoint);

void call_post_specialize(JNIEnv *env, jclass entrypoint);

char *get_string_data(JNIEnv *env, jstring value);

#ifndef DEX_H
#define DEX_H

#include <stdint.h>

#include <jni.h>

jclass dex_load_and_init(JNIEnv *env, int module_dir, const char *package_name, const char *process_name, void **files_data, size_t *files_sizes, size_t dex_count);

void call_pre_specialize(JNIEnv *env, jclass entrypoint);

void call_post_specialize(JNIEnv *env, jclass entrypoint);

#endif /* DEX_H */

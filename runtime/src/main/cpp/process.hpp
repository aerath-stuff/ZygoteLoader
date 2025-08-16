#pragma once

#include <jni.h>

#define PACKAGE_NAME_SYSTEM_SERVER "android"
#define ALL_PACKAGES_NAME ".all"

void process_get_name(JNIEnv *env, jstring jprocess_name, char **process_name);

void process_fix_package_name(char *process_name);
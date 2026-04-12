#include <string.h>
#include <stdbool.h>

#include "dex.h"
#include "logger.h"

#define JNI_CHECK_ERROR(cond, msg) \
  if (cond) {                      \
    LOGE(msg);                     \
                                   \
    return NULL;                   \
  }

jclass dex_load_and_init(JNIEnv *env, int module_dir, const char *package_name, const char *process_name, void **files_data, size_t *files_sizes, size_t dex_count) {
  jclass c_class_loader = (*env)->FindClass(env, "java/lang/ClassLoader");
  JNI_CHECK_ERROR(!c_class_loader, "Failed to find ClassLoader class");

  jmethodID m_get_system_class_loader = (*env)->GetStaticMethodID(env, c_class_loader, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
  JNI_CHECK_ERROR(!m_get_system_class_loader, "Failed to find ClassLoader.getSystemClassLoader method");

  jobject o_system_class_loader = (*env)->CallStaticObjectMethod(env, c_class_loader, m_get_system_class_loader);
  JNI_CHECK_ERROR(!o_system_class_loader, "Failed to get system class loader");

  jclass c_dex_class_loader = (*env)->FindClass(env, "dalvik/system/InMemoryDexClassLoader");
  JNI_CHECK_ERROR(!c_dex_class_loader, "Failed to find InMemoryDexClassLoader class");

  jmethodID m_dex_class_loader = (*env)->GetMethodID(env, c_dex_class_loader, "<init>", "([Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
  JNI_CHECK_ERROR(!m_dex_class_loader, "Failed to find InMemoryDexClassLoader.<init> method");

  jclass c_byte_buffer = (*env)->FindClass(env, "java/nio/ByteBuffer");
  JNI_CHECK_ERROR(!c_byte_buffer, "Failed to find ByteBuffer class");

  jobject o_buffers = (*env)->NewObjectArray(env, dex_count, c_byte_buffer, NULL);
  JNI_CHECK_ERROR(!o_buffers, "Failed to create buffer array");

  for (size_t i = 0; i < dex_count; i++) {
    jobject o_buffer = (*env)->NewDirectByteBuffer(env, files_data[i], files_sizes[i]);
    JNI_CHECK_ERROR(!o_buffer, "Failed to create direct byte buffer");

    (*env)->SetObjectArrayElement(env, o_buffers, i, o_buffer);
  }

  jobject o_dex_class_loader = (*env)->NewObject(
    env,
    c_dex_class_loader,
    m_dex_class_loader,
    o_buffers,
    o_system_class_loader
  );
  JNI_CHECK_ERROR(!o_dex_class_loader, "Failed to create InMemoryDexClassLoader instance");

  jmethodID m_load_class = (*env)->GetMethodID(
    env,
    c_class_loader,
    "loadClass",
    "(Ljava/lang/String;)Ljava/lang/Class;"
  );
  JNI_CHECK_ERROR(!m_load_class, "Failed to find ClassLoader.loadClass method");

  jstring s_entrypoint_name = (*env)->NewStringUTF(env, "com.v7878.zygisk.EntryPoint");
  JNI_CHECK_ERROR(!s_entrypoint_name, "Failed to create entry point name string");

  jclass c_entrypoint = (jclass) (*env)->CallObjectMethod(
    env,
    o_dex_class_loader,
    m_load_class,
    s_entrypoint_name
  );
  JNI_CHECK_ERROR(!c_entrypoint, "Failed to load entry point class");

  jmethodID m_load = (*env)->GetStaticMethodID(
    env,
    c_entrypoint,
    "load",
    "(Ljava/lang/String;Ljava/lang/String;I)Z"
  );
  JNI_CHECK_ERROR(!m_load, "Failed to find entry point load method");

  jstring s_package_name = (*env)->NewStringUTF(env, package_name);
  JNI_CHECK_ERROR(!s_package_name, "Failed to create package name string");

  jstring s_process_name = (*env)->NewStringUTF(env, process_name);
  JNI_CHECK_ERROR(!s_process_name, "Failed to create process name string");

  bool success = (*env)->CallStaticBooleanMethod(
    env,
    c_entrypoint,
    m_load,
    s_package_name,
    s_process_name,
    module_dir
  );

  return success ? c_entrypoint : NULL;
}

#undef JNI_CHECK_ERROR
#define JNI_CHECK_ERROR(cond, msg) \
  if (cond) {                      \
    LOGE(msg);                     \
                                   \
    return;                        \
  }

void call_pre_specialize(JNIEnv *env, jclass entrypoint) {
  jmethodID m_pre_specialize = (*env)->GetStaticMethodID(
    env,
    entrypoint,
    "preSpecialize",
    "()V"
  );
  JNI_CHECK_ERROR(!m_pre_specialize, "Failed to find entry point preSpecialize method");

  (*env)->CallStaticVoidMethod(env, entrypoint, m_pre_specialize);
}

void call_post_specialize(JNIEnv *env, jclass entrypoint) {
  jmethodID m_post_specialize = (*env)->GetStaticMethodID(
    env,
    entrypoint,
    "postSpecialize",
    "()V"
  );
  JNI_CHECK_ERROR(!m_post_specialize, "Failed to find entry point postSpecialize method");

  (*env)->CallStaticVoidMethod(env, entrypoint, m_post_specialize);
}

#undef JNI_CHECK_ERROR

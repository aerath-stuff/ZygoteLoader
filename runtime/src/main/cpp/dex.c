#include <string.h>
#include <stdbool.h>

#include "dex.h"
#include "logger.h"

jclass dex_load_and_init(JNIEnv *env, int module_dir, const char *package_name, const char *process_name, void **files_data, size_t *files_sizes, size_t dex_count) {
  jclass c_class_loader = (*env)->FindClass(env, "java/lang/ClassLoader");
  fatal_assert(c_class_loader != NULL);
  jmethodID m_get_system_class_loader = (*env)->GetStaticMethodID(env, c_class_loader, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
  fatal_assert(m_get_system_class_loader != NULL);

  jobject o_system_class_loader = (*env)->CallStaticObjectMethod(env, c_class_loader, m_get_system_class_loader);
  fatal_assert(o_system_class_loader != NULL);

  jclass c_dex_class_loader = (*env)->FindClass(env, "dalvik/system/InMemoryDexClassLoader");
  fatal_assert(c_dex_class_loader != NULL);
  jmethodID m_dex_class_loader = (*env)->GetMethodID(env, c_dex_class_loader, "<init>", "([Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
  fatal_assert(m_dex_class_loader != NULL);

  jclass c_byte_buffer = (*env)->FindClass(env, "java/nio/ByteBuffer");
  fatal_assert(c_byte_buffer != NULL);
  jobject o_buffers = (*env)->NewObjectArray(env, dex_count, c_byte_buffer, NULL);
  fatal_assert(o_buffers != NULL);

  for (size_t i = 0; i < dex_count; i++) {
    jobject o_buffer = (*env)->NewDirectByteBuffer(env, files_data[i], files_sizes[i]);
    fatal_assert(o_buffer != NULL);
    (*env)->SetObjectArrayElement(env, o_buffers, i, o_buffer);
  }

  jobject o_dex_class_loader = (*env)->NewObject(
    env,
    c_dex_class_loader,
    m_dex_class_loader,
    o_buffers,
    o_system_class_loader
  );
  fatal_assert(o_dex_class_loader != NULL);

  jmethodID m_load_class = (*env)->GetMethodID(
    env,
    c_class_loader,
    "loadClass",
    "(Ljava/lang/String;)Ljava/lang/Class;"
  );
  fatal_assert(m_load_class != NULL);

  jstring s_entrypoint_name = (*env)->NewStringUTF(env, "com.v7878.zygisk.EntryPoint");
  fatal_assert(s_entrypoint_name != NULL);

  jclass c_entrypoint = (jclass) (*env)->CallObjectMethod(
    env,
    o_dex_class_loader,
    m_load_class,
    s_entrypoint_name
  );
  fatal_assert(c_entrypoint != NULL);

  jmethodID m_load = (*env)->GetStaticMethodID(
    env,
    c_entrypoint,
    "load",
    "(Ljava/lang/String;Ljava/lang/String;I)Z"
  );
  fatal_assert(m_load != NULL);
  jstring s_package_name = (*env)->NewStringUTF(env, package_name);
  fatal_assert(s_package_name != NULL);
  jstring s_process_name = (*env)->NewStringUTF(env, process_name);
  fatal_assert(s_process_name != NULL);

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

void call_pre_specialize(JNIEnv *env, jclass entrypoint) {
    jmethodID m_pre_specialize = (*env)->GetStaticMethodID(
      env,
      entrypoint,
      "preSpecialize",
      "()V"
    );
    fatal_assert(m_pre_specialize != NULL);
    (*env)->CallStaticVoidMethod(env, entrypoint, m_pre_specialize);
}

void call_post_specialize(JNIEnv *env, jclass entrypoint) {
    jmethodID m_post_specialize = (*env)->GetStaticMethodID(
            env,
            entrypoint,
            "postSpecialize",
            "()V"
    );
    fatal_assert(m_post_specialize != NULL);
    (*env)->CallStaticVoidMethod(env, entrypoint, m_post_specialize);
}

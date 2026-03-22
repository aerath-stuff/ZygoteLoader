/* TODO: Test this thing */
/* TODO: We might need to do some Java programming to remove the dynamic targets */

#include <string.h>
#include <limits.h>
#include <stdio.h>

#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "zygisk.h"

#include "logger.h"
#include "dex.h"

#include <jni.h>

struct api_table *api_table;
JNIEnv *java_env;

jclass entrypoint;

char *get_string_data(JNIEnv *env, jstring *value) {
  const char *str = (*env)->GetStringUTFChars(env, *value, 0);
  if (str == NULL) return NULL;

  char *out = strdup(str);
  (*env)->ReleaseStringUTFChars(env, *value, str);

  return out;
}

const char *get_package_name(const char *data_dir, const char *process_name) {
  struct stat st;
  if (stat(data_dir, &st) == -1) {
    LOGE("Failed to stat data directory: %s", data_dir);

    return NULL;
  }

  const char *last_slash = strrchr(data_dir, '/');
  if (!last_slash) {
    LOGE("Failed to parse package name from data directory: %s", data_dir);

    return NULL;
  }

  const char *package_name = last_slash + 1;
  LOGD("Package name: %s for process: %s", package_name, process_name);

  return package_name;
}

int should_load_for_package(int module_dir, const char *package_name) {
  if (faccessat(module_dir, "packages/.all", F_OK, 0) == 0)
    return 1;

  char path[PATH_MAX];
  int size = snprintf(path, sizeof(path), "packages/%s", package_name);
  if (size < 0 || size >= (int) sizeof(path)) {
    LOGE("Package path is too long: %s", package_name);

    return 0;
  }

  return faccessat(module_dir, path, F_OK, 0) == 0;
}

void tryLoadDex(int module_dir, const char *package_name, const char *process_name) {
    if (!should_load_for_package(module_dir, package_name)) {
      LOGD("Skipping loading in %s", package_name);

      return;
    }

    LOGD("Loading in %s", package_name);

    void **class_files = NULL;
    size_t *class_files_sizes = 0;
    size_t class_files_size = 0;

    int module_dir_dup = dup(module_dir);
    if (module_dir_dup == -1) {
      LOGE("Failed to duplicate module directory");

      return;
    }

    DIR *dir = fdopendir(module_dir_dup);
    if (!dir) {
      LOGE("Failed to open module directory");
  
      close(module_dir_dup);

      return;
    }

    struct dirent64 *entry;
    while ((entry = readdir64(dir)) != NULL) {
      if (entry->d_type != DT_REG) continue;
      if (strncmp(entry->d_name, "classes", strlen("classes")) != 0 || !strstr(entry->d_name, ".dex")) continue;

      int fd = openat(dirfd(dir), entry->d_name, O_RDONLY);
      if (fd == -1) {
        LOGE("Failed to open file: %s", entry->d_name);

        continue;
      }

      struct stat st;
      if (fstat(fd, &st) == -1) {
        LOGE("Failed to stat file: %s", entry->d_name);

        close(fd);

        continue;
      }

      void *data = malloc(st.st_size);
      if (!data) {
        LOGE("Failed to allocate memory for file: %s", entry->d_name);

        close(fd);

        continue;
      }

      ssize_t read_bytes = read(fd, data, st.st_size);
      if (read_bytes != st.st_size) {
        LOGE("Failed to read file: %s", entry->d_name);

        free(data);

        close(fd);

        continue;
      }

      close(fd);

      class_files = (void **)realloc(class_files, sizeof(void *) * (class_files_size + 1));
      if (!class_files) {
        LOGE("Failed to realloc class_files");

        free(data);

        continue;
      }

      class_files_sizes = (size_t *)realloc(class_files_sizes, sizeof(size_t) * (class_files_size + 1));
      if (!class_files_sizes) {
        LOGE("Failed to realloc class_files_sizes");

        free(data);

        continue;
      }

      class_files[class_files_size] = data;
      class_files_sizes[class_files_size] = st.st_size;
      class_files_size++;
    }

    closedir(dir);

    entrypoint = (jclass) (*java_env)->NewGlobalRef(
      java_env,
      dex_load_and_init(
        java_env, module_dir,
        package_name, process_name,
        class_files, class_files_sizes, class_files_size
      )
    );

    for (size_t i = 0; i < class_files_size; i++) {
      free(class_files[i]);
    }

    free(class_files);
    free(class_files_sizes);
}

void callJavaPreSpecialize() {
  if (entrypoint) call_pre_specialize(java_env, entrypoint);
}

void callJavaPostSpecialize() {
  if (entrypoint) {
    call_post_specialize(java_env, entrypoint);

    (*java_env)->DeleteGlobalRef(java_env, entrypoint);
    entrypoint = NULL;
  }
}


void pre_app_specialize(void *mod_data, struct AppSpecializeArgs *args) {
  (void) mod_data;

  api_table->setOption(api_table->impl, DLCLOSE_MODULE_LIBRARY);

  char *process_name = get_string_data(java_env, args->nice_name);
  char *data_dir = get_string_data(java_env, args->app_data_dir);

  if (!process_name || !data_dir) {
    LOGE("Failed to get process name or data dir");

    if (process_name) free(process_name);
    if (data_dir) free(data_dir);
    
    return;
  }

  const char *package_name = get_package_name(data_dir, process_name);
  if (!package_name) {
    LOGE("Failed to get package name for process: %s", process_name);

    free(process_name);
    free(data_dir);

    return;
  }

  int module_dir = api_table->getModuleDir(api_table->impl);
  if (module_dir == -1) {
    LOGE("Failed to get module directory");

    free(process_name);
    free(data_dir);

    return;
  }

  tryLoadDex(module_dir, package_name, process_name);

  close(module_dir);

  callJavaPreSpecialize();

  free(process_name);
  free(data_dir);
}

void post_app_specialize(void *mod_data, const struct AppSpecializeArgs *args) {
  (void) mod_data; (void) args;

  callJavaPostSpecialize();
}

void pre_server_specialize(void *mod_data, struct ServerSpecializeArgs *args) {
  (void) mod_data; (void) args;

  LOGD("Request dlclose module");
  api_table->setOption(api_table->impl, DLCLOSE_MODULE_LIBRARY);

  int module_dir = api_table->getModuleDir(api_table->impl);
  if (module_dir == -1) {
    LOGE("Failed to get module directory");

    return;
  }

  tryLoadDex(module_dir, "android", "system_server");
  callJavaPreSpecialize();
}

void post_server_specialize(void *mod_data, const struct ServerSpecializeArgs *args) {
  (void) mod_data; (void) args;

  callJavaPostSpecialize();
}

void zygisk_module_entry(struct api_table *table, JNIEnv *env) {
  api_table = table;
  java_env = env;

  static struct module_abi abi = {
    .api_version = 5,
    .preAppSpecialize = pre_app_specialize,
    .postAppSpecialize = post_app_specialize,
    .preServerSpecialize = pre_server_specialize,
    .postServerSpecialize = post_server_specialize
  };

  if (!table->registerModule(table, &abi)) return;
}

void zygisk_companion_entry(int fd) {
  (void) fd;
}

package com.v7878.zygisk;

import android.annotation.SuppressLint;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

import dalvik.system.BaseDexClassLoader;

class Utils {
    public static Map<String, String> toMap(String properties) {
        Map<String, String> map = new HashMap<>();

        for (String line : properties.split("\n")) {
            String[] kv = line.split("=", 2);
            if (kv.length != 2)
                continue;

            map.put(kv[0].trim(), kv[1].trim());
        }

        return Collections.unmodifiableMap(map);
    }

    public static String getCurrentInstructionSet() throws ReflectiveOperationException {
        @SuppressLint("DiscouragedPrivateApi")
        class Holder {
            final Method getCurrentInstructionSet;

            {
                try {
                    Class<?> VMRuntime = Class.forName("dalvik.system.VMRuntime");
                    getCurrentInstructionSet = VMRuntime.getDeclaredMethod("getCurrentInstructionSet");
                    getCurrentInstructionSet.setAccessible(true);
                } catch (Throwable th) {
                    throw new ExceptionInInitializerError(th);
                }
            }
        }
        Holder holder = new Holder();
        return (String) holder.getCurrentInstructionSet.invoke(null);
    }

    public static String getNativeLibraryFolderName() throws ReflectiveOperationException {
        String isa = getCurrentInstructionSet();
        if ("arm".equals(isa)) {
            return "armeabi-v7a";
        }
        if ("arm64".equals(isa)) {
            return "arm64-v8a";
        }
        return isa;
    }

    public static void addNativePath(BaseDexClassLoader loader, File... libPaths) throws ReflectiveOperationException {
        @SuppressLint("DiscouragedPrivateApi")
        @SuppressWarnings("JavaReflectionMemberAccess")
        class Holder {
            final Field pathList;
            final Field nativeElements;
            final Constructor<?> initElement;

            {
                try {
                    pathList = BaseDexClassLoader.class.getDeclaredField("pathList");
                    pathList.setAccessible(true);
                    Class<?> DexPathList = Class.forName("dalvik.system.DexPathList");
                    nativeElements = DexPathList.getDeclaredField("nativeLibraryPathElements");
                    nativeElements.setAccessible(true);
                    Class<?> NativeLibraryElement = Class.forName("dalvik.system.DexPathList$NativeLibraryElement");
                    initElement = NativeLibraryElement.getDeclaredConstructor(File.class);
                    initElement.setAccessible(true);
                } catch (Throwable th) {
                    throw new ExceptionInInitializerError(th);
                }
            }
        }
        Objects.requireNonNull(loader);
        if (libPaths.length == 0) {
            return;
        }
        Holder holder = new Holder();
        Object pathList = holder.pathList.get(loader);
        Object[] elements = (Object[]) holder.nativeElements.get(pathList);
        //noinspection DataFlowIssue
        int oldLength = elements.length;
        elements = Arrays.copyOf(elements, oldLength + libPaths.length);
        for (int i = 0; i < libPaths.length; i++) {
            elements[oldLength + i] = holder.initElement.newInstance(libPaths[i]);
        }
        holder.nativeElements.set(pathList, elements);
    }
}

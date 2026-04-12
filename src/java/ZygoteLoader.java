package com.v7878.zygisk;

import java.util.Map;

/**
 * Utilize to manage ZygoteLoader
 */
public final class ZygoteLoader {
    /**
     * Package name that indicate currently in system_server
     * <p>
     * {@link #getPackageName}
     */
    public static final String PACKAGE_SYSTEM_SERVER = "android";
    public static final String PROCESS_SYSTEM_SERVER = "system_server";
    public static final String ALL_PACKAGES = ".all";

    /**
     * Get currently injected package name
     *
     * @return package name
     */
    public static String getPackageName() {
        return EntryPoint.getPackageName();
    }

    /**
     * Get currently injected process name
     *
     * @return package name
     */
    public static String getProcessName() {
        return EntryPoint.getProcessName();
    }

    /**
     * Get properties that read from module.prop
     *
     * @return map of module.prop
     */
    public static Map<String, String> getProperties() {
        return EntryPoint.getProperties();
    }

    /**
     * Retrieves the module's root path
     * <p>
     * This path is dynamically resolved and is only valid prior to process specialization.
     * After specialization, it becomes inaccessible due to SELinux policies
     * <p>
     * The path must be used immediately upon retrieval and should not be cached or stored for later use.
     * Any attempt to access it after specialization will result in a security denial or runtime failure
     *
     * @return the module root path (without trailing slash), or null after specialization
     */
    public static String getModuleDir() {
        return EntryPoint.getModuleDir();
    }
}

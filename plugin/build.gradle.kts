plugins {
    alias(libs.plugins.java.gradle.plugin)
    alias(libs.plugins.maven.publish)
}

java {
    toolchain.languageVersion = JavaLanguageVersion.of(17)
    withSourcesJar()
}

dependencies {
    compileOnly(gradleApi())
    compileOnly(libs.android.gradle)
}

gradlePlugin {
    plugins {
        create("zygote") {
            id = when {
                rootProject.group.toString().isEmpty() -> rootProject.name
                else -> "${rootProject.group}.${rootProject.name}"
            }
            implementationClass = "com.v7878.zygisk.gradle.ZygoteLoaderPlugin"
        }
    }
}

val dynamicSources = layout.buildDirectory.dir("generated/dynamic")

sourceSets {
    named("main") {
        java.srcDir(dynamicSources)
    }
}

val generator = tasks.register("generateDynamicSources") {
    outputs.dir(dynamicSources)

    doFirst {
        val buildConfig = dynamicSources.get().file(
            "com/v7878/zygisk/gradle/BuildConfig.java"
        ).asFile

        buildConfig.parentFile.mkdirs()

        val runtimeProject = project(":runtime")
        buildConfig.writeText(
            """
            package com.v7878.zygisk.gradle;
            
            public final class BuildConfig {
                public static final String RUNTIME_DEPENDENCY =
                 "${rootProject.group}.${rootProject.name}:${runtimeProject.name}:${runtimeProject.version}";
            }
            """.trimIndent()
        )
    }
}

tasks["compileJava"].dependsOn(generator)
tasks["sourcesJar"].dependsOn(generator)

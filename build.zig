const Builder = @import("std").build.Builder;
pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();
    var exe = b.addExecutable("test", null);
    exe.setBuildMode(mode);
    exe.addCSourceFile("src/webview.cc", &[_][]const u8{"-std=c++20"});
    exe.linkLibC();
    exe.linkLibCpp();
    exe.linkSystemLibraryName("m");
    exe.linkSystemLibraryName("dl");
    exe.linkSystemLibraryName("crypt");
    exe.linkSystemLibraryName("GLX");
    exe.linkSystemLibraryName("OpenGL");
    exe.addIncludeDir("../serenity");
    exe.addIncludeDir("../serenity/Userland/Libraries");

    // TODO: shell out to our own lagom-build cmake instead of leeching the ladybird
    exe.addIncludeDir("../ladybird/Build/_deps/lagom-build");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-compress.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-core.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-crypto.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-gemini.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-gfx.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-gl.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-gpu.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-ipc.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-js.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-markdown.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-regex.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-textcodec.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-timezone.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-unicode.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-wasm.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-web.a");
    exe.addObjectFile("../ladybird/Build/_deps/lagom-build/liblagom-xml.a");

    exe.install();

    const connect = b.step("run", "connecto wired");
    const run = exe.run();
    run.step.dependOn(b.getInstallStep());
    connect.dependOn(&run.step);
}

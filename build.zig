const Builder = @import("std").build.Builder;
pub fn build(b: *Builder) void {
    const mode = b.standardReleaseOptions();
    var exe = b.addExecutable("test", null);
    exe.setBuildMode(mode);
    exe.addCSourceFile("src/webview.cc", &[_][]const u8{
        "-std=c++20",
        "-fdiagnostics-color=always",
        "-fno-delete-null-pointer-checks",
        "-fno-exceptions",
        "-fno-semantic-interposition",
    });
    // exe.addObjectFile("webview.o");
    exe.linkLibC();
    exe.linkLibCpp();
    exe.linkSystemLibraryName("m");
    exe.linkSystemLibraryName("dl");
    exe.linkSystemLibraryName("crypt");
    exe.linkSystemLibraryName("GLX");
    exe.linkSystemLibraryName("OpenGL");
    exe.addIncludeDir("../ladybird/Build/ladybird_autogen/include/");
    exe.addIncludeDir("../serenity/Userland/Libraries");
    exe.addIncludeDir("../ladybird/Build/_deps/lagom-build/Services/");
    exe.addIncludeDir("../serenity");
    exe.addIncludeDir("../ladybird/Build/_deps/lagom-build");

    // TODO: shell out to our own lagom-build cmake instead of leeching the ladybird
    if (false) {
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
    } else {
        exe.addLibPath("../ladybird/Build/_deps/lagom-build/");
        exe.linkSystemLibraryName("lagom-compress");
        exe.linkSystemLibraryName("lagom-core");
        exe.linkSystemLibraryName("lagom-crypto");
        exe.linkSystemLibraryName("lagom-gemini");
        exe.linkSystemLibraryName("lagom-gfx");
        exe.linkSystemLibraryName("lagom-gl");
        exe.linkSystemLibraryName("lagom-gpu");
        exe.linkSystemLibraryName("lagom-ipc");
        exe.linkSystemLibraryName("lagom-js");
        exe.linkSystemLibraryName("lagom-markdown");
        exe.linkSystemLibraryName("lagom-regex");
        exe.linkSystemLibraryName("lagom-textcodec");
        exe.linkSystemLibraryName("lagom-timezone");
        exe.linkSystemLibraryName("lagom-unicode");
        exe.linkSystemLibraryName("lagom-wasm");
        exe.linkSystemLibraryName("lagom-web");
        exe.linkSystemLibraryName("lagom-xml");
    }

    exe.install();

    const connect = b.step("run", "connecto wired");
    const run = exe.run();
    run.step.dependOn(b.getInstallStep());
    connect.dependOn(&run.step);
}

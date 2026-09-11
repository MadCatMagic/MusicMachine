const std = @import("std");

const imguiIncludes = [_][]const u8{
    "lib/imgui-docking/",
    "lib/imgui-docking/backends/",
    "lib/glfw-3.5.1/include/",
};

// cmake -S ./lib/glfw-3.5.1/ -B ./bin/glfw -D GLFW_BUILD_WAYLAND=0
pub fn buildImGUI(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.Optimize) *std.Build.Step.Compile {
    const imguiLibModule = b.createModule(.{
        .link_libc = true,
        .target = target,
        .optimize = optimize,

        .sanitize_c = .off,
        .sanitize_thread = false,
    });
    for (imguiIncludes) |inclDir| {
        imguiLibModule.addIncludePath(b.path(inclDir));
    }

    imguiLibModule.addCSourceFiles(.{
        .root = b.path("lib/imgui-docking/"),
        .language = .cpp,
        .files = &.{
            "imgui_demo.cpp",
            "imgui_draw.cpp",
            "imgui_tables.cpp",
            "imgui_widgets.cpp",
            "imgui.cpp",
            "backends/imgui_impl_glfw.cpp",
            "backends/imgui_impl_opengl3.cpp",
        },
        .flags = &.{ "-std=c++17", "-ffast-math" },
    });

    return b.addLibrary(.{
        .name = "imgui",
        .root_module = imguiLibModule,
    });
}

pub fn build(b: *std.Build) void {
    var threaded: std.Io.Threaded = .init_single_threaded;
    const io = threaded.io();
    const gpa = std.heap.page_allocator;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exeModule = b.createModule(.{
        .link_libc = true,
        .link_libcpp = true,

        //.root_source_file = b.path("src/MusicMachine.cpp"),
        .target = target,
        .optimize = optimize,
        .imports = &.{},
        .sanitize_c = .off,
        .sanitize_thread = false,
    });
    exeModule.addIncludePath(b.path("include/"));

    // load source files
    var sourceFiles: [256][]const u8 = undefined;
    var sourceFileNum: usize = 0;

    const cwd = std.Io.Dir.cwd();
    const srcDir = std.Io.Dir.openDir(cwd, io, "src/", .{ .iterate = true }) catch unreachable;
    defer srcDir.close(io);
    var walker = srcDir.walk(gpa) catch @panic("OOM");
    defer walker.deinit();

    while (walker.next(io) catch unreachable) |f| {
        switch (f.kind) {
            .file => {
                sourceFiles[sourceFileNum] = gpa.dupe(u8, f.path) catch @panic("OOM");
                sourceFileNum += 1;
            },
            else => {},
        }
    }
    defer for (0..sourceFileNum) |i| {
        gpa.free(sourceFiles[i]);
    };

    exeModule.addCSourceFiles(.{
        .root = b.path("src/"),
        .language = .cpp,
        .files = sourceFiles[0..sourceFileNum],
        .flags = &.{"-std=c++17"},
    });

    // now add extra header files for libraries
    exeModule.addIncludePath(b.path("lib/glew-2.3.1/include"));
    for (imguiIncludes) |inclDir| {
        exeModule.addIncludePath(b.path(inclDir));
    }
    exeModule.addIncludePath(b.path("lib/portaudio/include"));

    exeModule.addCMacro("GLEW_NO_GLU", "");

    // link libraries
    const imguiLib = buildImGUI(b, target, optimize);
    exeModule.linkLibrary(imguiLib);
    exeModule.addLibraryPath(b.path("bin/glfw/src"));
    exeModule.addObjectFile(b.path("bin/glfw/src/libglfw3.a"));
    exeModule.linkSystemLibrary("GLEW", .{ .needed = true });
    exeModule.linkSystemLibrary("portaudio", .{ .needed = true });
    const exe = b.addExecutable(.{
        .name = "MusicMachine",
        .root_module = exeModule,
    });

    b.installArtifact(exe);

    const run_step = b.step("run", "Run the app");

    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    run_cmd.step.dependOn(b.getInstallStep());
}

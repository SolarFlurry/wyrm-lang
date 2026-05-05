const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const wyrm_exe = b.addExecutable(.{
        .name = "wyrm",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        }),
    });

    const test_exe = b.addExecutable(.{
        .name = "test",
        .root_module = b.createModule(.{
            .target = target,
            .optimize = .ReleaseSmall,
            .link_libcpp = true,
        }),
    });

    const clap = b.dependency("clap", .{});
    wyrm_exe.root_module.addImport("clap", clap.module("clap"));

    const compiler_c = b.addTranslateC(.{
        .root_source_file = b.path("src/compiler/compiler.h"),
        .optimize = optimize,
        .target = target,
    });

    const vm_c = b.addTranslateC(.{
        .root_source_file = b.path("src/vm/debug.h"),
        .optimize = optimize,
        .target = target,
    });

    compiler_c.addIncludePath(b.path("src/"));
    vm_c.addIncludePath(b.path("src/"));
    wyrm_exe.root_module.addImport("c", compiler_c.createModule());
    wyrm_exe.root_module.addImport("vm", vm_c.createModule());

    test_exe.root_module.addCSourceFile(.{
        .language = .cpp,
        .file = b.path("src/test.cpp"),
        .flags = &.{"-std=c++23"},
    });

    // wyrm_exe.addCSourceFiles(.{
    //     .language = .cpp,
    //     .files = &.{
    //         "src/main.cpp",
    //         "src/cli/cli_parser.cpp",
    //     },
    //     .flags = &.{"-std=c++23"},
    // });

    wyrm_exe.root_module.addCSourceFiles(.{
        .language = .c,
        .files = &.{
            "src/utils/memory.c",
            "src/vm/chunk.c",
            "src/vm/debug.c",
            "src/vm/value.c",
            "src/vm/vm.c",
            //"src/compiler/lexer/lexer.c",
            "src/compiler/parser/parser.c",
            "src/compiler/parser/expr.c",
            "src/compiler/parser/decl.c",
            "src/compiler/parser/type.c",
            "src/compiler/error/error.c",
            "src/compiler/ast.c",
            "src/compiler/codegen/codegen.c",
            "src/compiler/compiler.c",
            "src/compiler/token.c",
            "src/compiler/semantic_analysis/symtable.c",
            "src/compiler/semantic_analysis/name_res.c",
            "src/compiler/semantic_analysis/typechecker.c",
            "src/compiler/semantic_analysis/expr.c",
            // "src/compiler/cst_parser/parser.c",
            "src/compiler/common/cst.c",
        },
    });

    wyrm_exe.root_module.addIncludePath(b.path("src/"));

    wyrm_exe.linkage = .dynamic;

    test_exe.linkage = .dynamic;

    b.installArtifact(wyrm_exe);
    b.installArtifact(test_exe);
}

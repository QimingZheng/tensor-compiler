cc_binary(
    name="example",
    srcs=["example.cc"],
    deps=[
        "//lib:polly",
        "@isl//:isl",
    ],
    linkopts = ["-lgmp"],
)

cc_binary(
    name="auto_scheduler",
    srcs=["test_auto_scheduler.cc"],
    linkopts=["-lgmp"],
    copts=["--std=c++11"],
    deps=[
        "//lib:polly",
    ],
)

cc_binary(
    name="jit",
    srcs=["test_jit_module.cc"],
    deps=[
        "//lib:polly",
    ],
)

cc_test(
    name="codegen",
    srcs=[
        "test_codegen.cc",
    ],
    deps=[
        "//lib:polly",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name="pass",
    srcs=[
        "pass/test_check_pass.cc",
        "pass/test_analysis_pass.cc",
        "pass/test_optimization_pass.cc",
        "pass/test_transform_pass.cc",
        "pass/test_parallelization_utils.cc",
        "pass/test_parallelization_pass.cc",
    ],
    deps=[
        "//lib:polly",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@isl//:isl",
    ],
)

cc_binary(
    name="example",
    srcs=["example.cc"],
    deps=[
        "//lib:polly",
    ],
)

cc_binary(
    name="auto_scheduler",
    srcs=["test_auto_scheduler.cc"],
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
    name="check_pass_test",
    srcs=[
        "test_check_pass.cc",
    ],
    deps=[
        "//lib:polly",
        "@gtest//:gtest",
        "@gtest//:gtest_main",
        "@isl//:isl",
    ],
)
 
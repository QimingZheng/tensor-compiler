cc_library(
    name = "isl",
    hdrs = glob([
        "**/*.h",
    ]),
    srcs = glob([
       "**/*.so",
       "**/*.a",
    ]),
    visibility = ["//visibility:public"],
    includes = [
        "./include/",
        ".",
    ],
)
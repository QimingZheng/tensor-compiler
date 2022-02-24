cc_library(
    name = "polly",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    includes = ["."],
    deps = [
        "@isl//:isl",
    ],
    visibility = [
        "//test:__subpackages__",
        "//dnn_cost_model:__subpackages__",
    ],
)

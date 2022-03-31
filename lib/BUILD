cc_library(
    name = "polly",
    srcs = glob(["**/*.cc"]),
    hdrs = glob(["**/*.h"]),
    includes = ["."],
    deps = [
        "@isl//:isl",
        # "@org_tensorflow//tensorflow:tensorflow_framework",
        # "@tensorflow//tensorflow/core:tensorflow",
        # "@tensorflow//tensorflow/tools/cc_package:build_cc_package",
        "@org_tensorflow//tensorflow/core:lib",
    ],
    visibility = [
        "//test:__subpackages__",
        "//dnn_cost_model:__subpackages__",
    ],
    copts = ["-std=c++14"],
)

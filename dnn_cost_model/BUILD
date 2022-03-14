cc_binary(
    name="data_generation",
    srcs=["train_data_gen.cc"],
    linkopts = ["-lgmp"],
    deps=[
        "//lib:polly",
    ],
)
cc_binary(
    name="gbt_feature_extract",
    srcs=["gbt_feature_extract.cc"],
    linkopts = ["-lgmp"],
    deps=[
        "//lib:polly",
    ],
)
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
# load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest",
    branch = "v1.10.x",
)

new_local_repository(
    name = "isl",
    path = "/home/qmzheng/course/isl/build/",
    build_file = "bazel/BUILD.isl",
)

load("//lib:repo.bzl", "tensorflow_http_archive")

tensorflow_http_archive(
    name = "org_tensorflow",
    sha256 = "3e6c98de0842520a65978549be7b1b6061080ecf9fa9f3a87739e19a0447a85c",
    git_commit = "1f8f692143aa9a42c55f8b35d09aeed93bdab66e",
)

load(
    "@org_tensorflow//tensorflow:version_check.bzl",
    "check_bazel_version_at_least"
)
check_bazel_version_at_least("5.0.0")

load("@org_tensorflow//tensorflow:workspace3.bzl", "workspace")
workspace()
load("@org_tensorflow//tensorflow:workspace2.bzl", "workspace")
workspace()
load("@org_tensorflow//tensorflow:workspace1.bzl", "workspace")
workspace()
load("@org_tensorflow//tensorflow:workspace0.bzl", "workspace")
workspace()

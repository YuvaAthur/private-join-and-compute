# Copyright 2019 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Copyright 2019 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""WORKSPACE file for Private Join and Compute."""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_github_glog_glog",
    sha256 = "f28359aeba12f30d73d9e4711ef356dc842886968112162bc73002645139c39c",
    strip_prefix = "glog-0.4.0",
    urls = ["https://github.com/google/glog/archive/v0.4.0.tar.gz"],
)

# gflags
# Needed for glog
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = [
        "https://mirror.bazel.build/github.com/gflags/gflags/archive/v2.2.2.tar.gz",
        "https://github.com/gflags/gflags/archive/v2.2.2.tar.gz",
    ],
)

# gRPC
http_archive(
    name = "com_github_grpc_grpc",
    sha256 = "c2ab8a42a0d673c1acb596d276055adcc074c1116e427f118415da3e79e52969",
    strip_prefix = "grpc-1.27.3",
    urls = [
        "https://github.com/grpc/grpc/archive/v1.27.3.tar.gz",
    ],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

# Includes boringssl, com_google_absl, and other dependencies.
grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

# Loads transitive dependencies of GRPC.
grpc_extra_deps()

#google-shell encryption dependencies can be brought in here
# abseil-cpp
http_archive(
    name = "com_google_absl",
    sha256 = "728a813291bdec2aa46eab8356ace9f75ac2ed9dfe2df5ab603c4e6c09f1c353",
    strip_prefix = "abseil-cpp-20200225",
    urls = [
         "https://github.com/abseil/abseil-cpp/archive/20200225.tar.gz",
    ],
)

# microsoft seal integration
# #https://github.com/bazelbuild/rules_go/blob/0.19.0/go/workspace.rst#proto-dependencies
# git_repository(
#     name = "com_google_protobuf",
#     commit = "09745575a923640154bcf307fba8aedff47f240a",
#     remote = "https://github.com/protocolbuffers/protobuf",
#     shallow_since = "1558721209 -0700",
# )

# load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

# protobuf_deps()

# #https://chromium.googlesource.com/external/github.com/google/quic-trace/+/refs/heads/master/WORKSPACE

# http_archive(
#     name = "zlib",
#     build_file = "@com_google_protobuf//:third_party/zlib.BUILD",
#     sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
#     strip_prefix = "zlib-1.2.11",
#     urls = ["https://zlib.net/zlib-1.2.11.tar.gz"],
# )

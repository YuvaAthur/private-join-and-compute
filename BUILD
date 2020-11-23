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

load("@com_github_grpc_grpc//bazel:grpc_build_system.bzl", "grpc_proto_library")

# integrating shell build
# load("@rules_cc//cc:defs.bzl", "cc_library")
# load("@rules_proto//proto:defs.bzl", "proto_library")

# package(default_visibility = ["//visibility:public"])
#

grpc_proto_library(
    name = "match_proto",
    srcs = ["match.proto"],
)

grpc_proto_library(
    name = "private_intersection_sum_proto",
    srcs = ["private_intersection_sum.proto"],
    deps = [
        ":match_proto",
    ],
)

grpc_proto_library(
    name = "private_join_and_compute_proto",
    srcs = ["private_join_and_compute.proto"],
    deps = [
        ":private_intersection_sum_proto",
    ],
)

cc_library(
    name = "message_sink",
    hdrs = ["message_sink.h"],
    deps = [
        ":private_join_and_compute_proto",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

cc_library(
    name = "protocol_client",
    hdrs = ["protocol_client.h"],
    deps = [
        ":message_sink",
        ":private_join_and_compute_proto",
        "//util:status_includes",
    ],
)

# ------------------------ client setup ----------------

# client_impl : Uses Pair Constructor (Original Code) <id, col>
cc_library(
    name = "client_impl",
    srcs = ["client_impl.cc"],
    hdrs = ["client_impl.h"],
    deps = [
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_client",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)


# ---------------------------------------------------
# integrating microsoft seal library
# reference BUILD in /seal sub directory
# ----------------------------------------------------

# package(default_visibility = ["//visibility:public"])


# cc_library(
#     name = "seal_includes",
#     hdrs = [
#         "seal.h",
#         "baseconverter.h",
#         "batchencoder.h",
#         "biguint.h",
#         "blake2-impl.h",
#         "blake2.h",
#         "ciphertext.h",
#         "ckks.h",
#         "clang.h",
#         "clipnormal.h",
#         "common.h",
#         "config.h",
#         "context.h",
#         "croots.h",
#         "decryptor.h",
#         "defines.h",
#         "encryptionparams.h",
#         "encryptor.h",
#         "evaluator.h",
#         "galoiskeys.h",
#         "gcc.h",
#         "globals.h",
#         "hash.h",
#         "hestdparms.h",
#         "intarray.h",
#         "intencoder.h",
#         "keygenerator.h",
#         "kswitchkeys.h",
#         "locks.h",
#         "memorymanager.h",
#         "mempool.h",
#         "modulus.h",
#         "msvc.h",
#         "numth.h",
#         "plaintext.h",
#         "pointer.h",
#         "polyarith.h",
#         "polyarithmod.h",
#         "polyarithsmallmod.h",
#         "polycore.h",
#         "publickey.h",
#         "randomgen.h",
#         "randomtostd.h",
#         "relinkeys.h",
#         "rlwe.h",
#         "scalingvariant.h",
#         "secretkey.h",
#         "serialization.h",
#         "smallmodulus.h",
#         "smallntt.h",
#         "uintarith.h",
#         "uintarithmod.h",
#         "uintarithsmallmod.h",
#         "uintcore.h",
#         "valcheck.h",
#         "ztools.h",
#     ],
# )

# cc_library(
#     name ="seal_lib",
#     srcs = ["seal/lib/libseal-3.4.a"],
#     hdrs = ["seal/seal.h"],
#     deps = [
#         "//seal:seal_includes",
#         "@zlib//:zlib",
#     ]
# )




# client_tuple_impl : Extends client_impl to use tuple <id, col1, col2>
cc_library(
    name = "client_tuple_impl",
    srcs = ["client_tuple_impl.cc"],
    hdrs = ["client_tuple_impl.h"],
    deps = [
        ":client_impl",
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_client",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        # "//crypto:elgamal",
        # "//shell:symmetric_encryption",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
        "//seal:seal_includes",
        "//seal:seal_lib",
    ],
)


# ---------------------- server setup -----------------------------
cc_library(
    name = "protocol_server",
    hdrs = ["protocol_server.h"],
    deps = [
        ":message_sink",
        ":private_join_and_compute_proto",
        "//util:status_includes",
    ],
)

cc_library(
    name = "server_impl",
    srcs = ["server_impl.cc"],
    hdrs = ["server_impl.h"],
    deps = [
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_server",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

# server_tuple_impl : Extends server_impl to use tuple <id, col1, col2>
cc_library(
    name = "server_tuple_impl",
    srcs = ["server_tuple_impl.cc"],
    hdrs = ["server_tuple_impl.h"],
    deps = [
        ":match_proto",
        ":message_sink",
        ":private_intersection_sum_proto",
        ":private_join_and_compute_proto",
        ":protocol_server",
        "//crypto:bn_util",
        "//crypto:ec_commutative_cipher",
        "//crypto:paillier",
        "//crypto:elgamal",
 #       "//shell:symmetric_encryption",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
    ],
)

# ------------------------ data generation setup ----------------
cc_library(
    name = "data_util",
    srcs = ["data_util.cc"],
    hdrs = ["data_util.h"],
    deps = [
        ":match_proto",
        "//crypto:bn_util",
        "//util:status",
        "//util:status_includes",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name = "generate_dummy_data",
    srcs = ["generate_dummy_data.cc"],
    deps = [
        ":data_util",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_google_absl//absl/base",
    ],
)

# ------------------- gRPC set up ---------------------------
cc_library(
    name = "private_join_and_compute_rpc_impl",
    srcs = ["private_join_and_compute_rpc_impl.cc"],
    hdrs = ["private_join_and_compute_rpc_impl.h"],
    deps = [
        ":message_sink",
        ":private_join_and_compute_proto",
        ":protocol_server",
        "//util:status_includes",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "server",
    srcs = ["server.cc"],
    deps = [
        ":data_util",
        ":private_join_and_compute_proto",
        ":private_join_and_compute_rpc_impl",
        ":protocol_server",
        ":server_impl",
        ":server_tuple_impl",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/memory",
    ],
)

cc_binary(
    name = "client",
    srcs = ["client.cc"],
    deps = [
        ":client_impl",
        ":client_tuple_impl",
        ":data_util",
        ":private_join_and_compute_proto",
        ":protocol_client",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_glog_glog//:glog",
        "@com_github_grpc_grpc//:grpc",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        # ":seal_lib",
        # "@zlib//:zlib",
    ],
    copts = [
        # "-Iinclude/",
        # "-std=c++17",
        "-Lseal/lib/",
    ],
    linkstatic=1,

)

# ---------------- compile option ---------------------------
# bazel build :all --incompatible_disable_deprecated_attr_params=false 
# --incompatible_depset_is_not_iterable=false 
# --incompatible_new_actions_api=false 
# --incompatible_no_support_tools_in_action_inputs=false 
# --cxxopt='-std=c++17' 
# Figuring out Bazel Build using Microsoft Source code

package(default_visibility = ["//visibility:public"])


cc_library(
    name = "seal_includes",
    hdrs = [
        "seal.h",
        "baseconverter.h",
        "batchencoder.h",
        "biguint.h",
        "blake2-impl.h",
        "blake2.h",
        "ciphertext.h",
        "ckks.h",
        "clang.h",
        "clipnormal.h",
        "common.h",
        "config.h",
        "context.h",
        "croots.h",
        "decryptor.h",
        "defines.h",
        "encryptionparams.h",
        "encryptor.h",
        "evaluator.h",
        "galoiskeys.h",
        "gcc.h",
        "globals.h",
        "hash.h",
        "hestdparms.h",
        "intarray.h",
        "intencoder.h",
        "keygenerator.h",
        "kswitchkeys.h",
        "locks.h",
        "memorymanager.h",
        "mempool.h",
        "modulus.h",
        "msvc.h",
        "numth.h",
        "plaintext.h",
        "pointer.h",
        "polyarith.h",
        "polyarithmod.h",
        "polyarithsmallmod.h",
        "polycore.h",
        "publickey.h",
        "randomgen.h",
        "randomtostd.h",
        "relinkeys.h",
        "rlwe.h",
        "scalingvariant.h",
        "secretkey.h",
        "serialization.h",
        "smallmodulus.h",
        "smallntt.h",
        "uintarith.h",
        "uintarithmod.h",
        "uintarithsmallmod.h",
        "uintcore.h",
        "valcheck.h",
        "ztools.h",
    ],
)


#https://stackoverflow.com/questions/52635860/how-can-i-include-an-already-built-static-library-a-file-using-bazel
#https://docs.bazel.build/versions/master/cpp-use-cases.html

cc_library(
    name ="seal_lib",
    srcs = ["lib/libseal-3.4.a"],
    hdrs = ["seal.h"],
    deps = [":seal_includes"]
)

cc_library(
    name = "bfv_example",
    srcs = [
        # "1_bfv_basics.cpp",
        "examples.cpp",
    ],
    hdrs = ["examples.h"],
    deps = [
        ":seal_includes",
    ],
    copts = [
        "-std=c++17",
    ],
)

cc_binary(
    name ="seal_examples",
    srcs = [
        "examples.cpp",
        "1_bfv_basics.cpp",
        ],
    deps = [
        ":bfv_example",
        ":seal_lib",
    ],
    copts = [
        # "-Iinclude/",
        "-std=c++17",
        "-Llib/",
    ],
    linkstatic=1,
)

# Implementation & Debugging details

In this code base - called Configurable Communication Module - we extend the original Private Join and Compute in the following way:
+ Implementing authentication using mTLS with Client and Server having to present certificates before data exchange begins
+ Extending the protocol to work on 2 data columns
+ Integrate [Microsoft SEAL](https://github.com/microsoft/SEAL) Fully Homomorphic Encryption Library 

## mTLS Extension
The files client.cc / server.cc were extended with the following gRPC extensions

### Client
```c++
    auto creds = grpc::SslCredentials(sslOpts);    
    stub =
      PrivateJoinAndComputeRpc::NewStub(::grpc::CreateChannel(
            FLAGS_port1, creds)); 
```

### Server
```c++
    auto creds = grpc::SslServerCredentials(sslOpts);
    builder.AddListeningPort(FLAGS_port1,creds); 
```

### Flags: for mTLS on client and server
Client flags
```c++
// switch to use MTLS
DEFINE_int32(use_mtls,0,"Swtich to use MTLS approach,dafault no MTLS");
// for local grpc
DEFINE_string(port0, "0.0.0.0:10501", "Port on which to contact server");
// for MTLS grpc
DEFINE_string(port1, "california.sjsu-mtls.com:10501", "Port on which to contact server");

```
Server flags
```c++
// switch to use MTLS
DEFINE_int32(use_mtls,0,"Swtich to use MTLS approach,dafault no MTLS");
// for local credentials
DEFINE_string(port0, "0.0.0.0:10501", "Port on which to listen");
// for certificate crendentials
DEFINE_string(port1, "california.sjsu-mtls.com:10501", "Port on which to listen");

```
### mTLS extension requirements (not in repo)
+ Client
  + client.crt
  + client.key
+ Server
  + server.crt
  + server.key
  + ca.crt
+ gRPC credentials are set up for communication
+ For local debug
  + Set up in /etc/hosts
  ``` 
  0.0.0.0    california.sjsu-mtls.com
  ```
### gRPC debugging steps
+ https://github.com/grpc/grpc/blob/master/TROUBLESHOOTING.md   
+ Debugging network process
  + export GRPC_VERBOSITY=debug 
  + export GRPC_TRACE=all

+ transport_security - traces metadata about secure channel establishment
  + export GRPC_TRACE=transport_security
  + export GRPC_TRACE=transport_security,handshaker

+ protocol error
  + export GRPC_TRACE=api,call_error

+ Unset
  + unset GRPC_VERBOSITY
  + unset GRPC_TRACE



## Protocol Extensions
+ Ref: https://developers.google.com/protocol-buffers/docs/reference/proto2-spec 


Adding another business data column: 
+ match.proto: 
```c++
message EncryptedElement {
  optional bytes element = 1;
  optional bytes associated_data_1 = 2;               //existing 
  optional bytes associated_data_2 = 3;               //extension
} 
```


+ private_intersection_sum.proto
```c++
  message ClientRoundOne {
    optional bytes public_key = 1;
    optional EncryptedSet encrypted_set = 2;
    optional EncryptedSet reencrypted_set = 3;
    optional int32 op_code_1 = 4;
    optional int32 op_code_2 = 5;
  }

  message ServerRoundTwo {
    optional int64 intersection_size = 1;
    optional bytes encrypted_sum_1 = 2;
    optional bytes encrypted_sum_2 = 3;
  }  
```

From a protobuf implementation point of view, using enumerators requires additional checks. So, while the operators (op_code_1 & op_code_2) can be implemented using enumerators, this was not done for this version of the code. 


## Extending to operators
Adding operator codes: 
+ match.proto
```c++
//  This gets applied on ONE Column only
//  Possible : 
//      1) sum(col) 
//      2) sum(col^2) 
//      3) varn(col) = sum([(x-avg(col))^2])
message OpCode {
  enum Operator {
    SUM = 0;
    SUMSQ = 1;
    VARN = 2;
  }
  Operator op = 1 [default = SUM];
}
```

+ Gettting value from enumerator
  + Ref: https://stackoverflow.com/questions/32161409/how-to-get-protobuf-enum-as-string


## Implementation Choices

Extensions and integrations were done in the following sequence
+ Adding one extra data column and adapting dummy data generation
+ Extending to mTLS using gRPC for certificate based authentication
+ Integrating Microsoft SEAL library to compute sum of squares

## Code refactoring 
Two client implementations:
+ client_impl : Original Code
+ client_tuple_impl :
  + Supports 2 data columns
  + Supports mtls
  + Supports integration to Microsoft SEAL (library to be built independently)

Two server implementations:
+ server_impl : Original Code
+ server_tuple_impl : 
  + Supports 2 data columns
  + Supports mtls
  + Supports integration to Microsoft SEAL (library to be built independently)

## Flags: for 2 column implementation
Client Flags
```c++
DEFINE_string(port, "0.0.0.0:10501", "Port on which to contact server");
DEFINE_string(client_data_file, "",
              "The file from which to read the client database.");
DEFINE_int32(
    paillier_modulus_size, 1536,
    "The bit-length of the modulus to use for Paillier encryption. The modulus "
    "will be the product of two safe primes, each of size "
    "paillier_modulus_size/2.");

//--mult_column : binary : 0 means 1 column (default), 1 means 2 columns
DEFINE_int32(multi_column,0,"Indicates 1 or 2 columns in the Client Data Set");
```
Server Flags
```c++
DEFINE_string(port, "0.0.0.0:10501", "Port on which to listen");
DEFINE_string(server_data_file, "",
              "The file from which to read the server database.");
//--mult_column : binary : 0 means 1 column (default), 1 means 2 columns
DEFINE_int32(multi_column,0,"Indicates 1 or 2 columns in the Client Data Set");

```
+ Dummy Set Generation
Ref: https://stackoverflow.com/questions/7616511/calculate-mean-and-standard-deviation-from-a-vector-of-samples-in-c-using-boos

+ The data set generation was extended to 2 columns and computing additional values. Example run and its output is 
```
bazel-bin/generate_dummy_data --server_data_file=/tmp/dummy_server_data.csv --client_data_file=/tmp/dummy_client_data.csv --server_data_size=3 --client_data_size=3 --intersection_size=2 --max_associated_value=31

Aggregated values computed using vector algo: sum = 3, sum of squares = 5, mean = 1.5, variance numerator = 0.5
Aggregated values computed using vector algo: sum = 34, sum of squares = 706, mean = 17, variance numerator = 481
Generated Server dataset of size 3, Client dataset of size 3
Passed flags passed aggregators: Operator 1 = 0, Operator 2 = 0
Intersection size = 2
Intersection aggregate 1 = 3
Intersection aggregate 2 = 34
```


## Integration SEAL BFV HE

Client
+ Given
  + Secret Key
+ Value passed
  + x = 4
+ Value returned 
  + e = h.e.(secret_key_decrypt(4))
+ Value passed to server
  + e
+ Options:
  + integrate into code
  + command line call (seal_client --encrypt=4) = e

Server
+ Given
  + Public Key
+ Value Received
  + e
+ Computation Done
  + e2 = h.e.(public_key_sq(e))
+ Value passed to client
  + e2
+ Options
  + integrate into code
  + command line call (seal_server --evalue=e) = e2
+ Server
  + square (e2) 

Client
+ Value received = e2
+ Final value
  + d = h.e.(secret_key_decrypt(e2)) = x^2
+ Options:
  + integrate into code
  + (seal_client --decrypt=e2) = x^2

## Flags: for SEAL integration
Client Flags
```c++
//enumerator value sent in protocol
DEFINE_int32(operator_1,0,"Operator One");
DEFINE_int32(operator_2,0,"Operator Two");
    // SUM = 0;
    // SUMSQ = 1;

//integrating Microsoft SEAL FHE
DEFINE_int32(use_seal,0,"Defines whether SEAL is to be used");
```
Server Flags
```c++
//integrating Microsoft SEAL FHE
DEFINE_int32(use_seal,0,"Defines whether SEAL is to be used");

```


## Testing SEAL
In this final version of implementation [Nov 2020], we achieve integration of Microsoft SEAL using a static library link and built the whole project using Bazel Build.

We used the following initializing parameters for SEAL:
```c++
  seal::EncryptionParameters parms(seal::scheme_type::BFV);
  size_t poly_modulus_degree = 4096;
  parms.set_poly_modulus_degree(poly_modulus_degree);
  parms.set_coeff_modulus(seal::CoeffModulus::BFVDefault(poly_modulus_degree));
  parms.set_plain_modulus(1024);
```

In our tests, this limits the value of x to 31 due to inherent modular arithmetic. More work needs to be done to test what modulus values can extend the range of values that can be used.


```
bazel-bin/generate_dummy_data --server_data_file=/tmp/dummy_server_data.csv --client_data_file=/tmp/dummy_client_data.csv --server_data_size=3 --client_data_size=3 --intersection_size=2 --max_associated_value=31

Aggregated values computed using vector algo: sum = 3, sum of squares = 5, mean = 1.5, variance numerator = 0.5
Aggregated values computed using vector algo: sum = 34, sum of squares = 706, mean = 17, variance numerator = 481
Generated Server dataset of size 3, Client dataset of size 3
Passed flags passed aggregators: Operator 1 = 0, Operator 2 = 0
Intersection size = 2
Intersection aggregate 1 = 3
Intersection aggregate 2 = 34
```

```
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1 --use_seal=1 --operator_2=1 --operator_1=1
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1 --use_seal=1
```
```
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1
```

## Command line for all features:

```
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1 --use_mtls=1 --use_seal=1
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1 --use_mtls=1 --use_seal=1 --operator_1=0 --operator_2=1
```
Server & Client ports can be changed if required using --port1 when using --use_mtls=1 option

The code has been tested on Ubuntu VMs representing two parties with these flags.

## Bazel Build Process

### Additional BUILD file options
+ Setting for C++17 for build
  + update .bazelrc
```
build:c++17 --cxxopt=-std=c++1z
build:c++17 --cxxopt=-stdlib=libc++
build:c++1z --cxxopt=-std=c++1z
build:c++1z --cxxopt=-stdlib=libc++
```



https://stackoverflow.com/questions/47125387/stringstream-and-binary-data
If you want to read/write binary data you can't use << or >> you need to use the std::stringstream::read and std::stringstream::write functions.
Or, pass ios::in | ios::out | ios::binary in constructor.



Log of Bazel build
-------------------
bazel build :all >& /tmp/bazel_build.log

### Bazel build errors & fixes
+ On 2020.04 Ubuntu upgrade I see the following error:
ERROR: /home/yuva/Code/private-join-and-compute/BUILD:30:1: undeclared inclusion(s) in rule '//:_private_join_and_compute_proto_only':
this rule is missing dependency declarations for the following files included by 'private_join_and_compute.pb.cc':
  '/usr/lib/gcc/x86_64-linux-gnu/9/include/stddef.h'
  '/usr/lib/gcc/x86_64-linux-gnu/9/include/stdarg.h'
  '/usr/lib/gcc/x86_64-linux-gnu/9/include/stdint.h'
  '/usr/lib/gcc/x86_64-linux-gnu/9/include/limits.h'
  '/usr/lib/gcc/x86_64-linux-gnu/9/include/syslimits.h'
  + Fix: Use bazel clean --expunge [https://stackoverflow.com/questions/48155976/bazel-undeclared-inclusions-errors-after-updating-gcc/48524741#48524741]

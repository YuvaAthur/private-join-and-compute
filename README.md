# Configurable Communication Module 
Extends [Google's Private Join & Compute](https://github.com/google/private-join-and-compute)


In this project we explore the extensions of Google's Private Join and Compute protocol in the following ways:
+ Extending PJC protocol to additional data columns and applying columnar aggregation based on supported homomorphic operators, 
+ Exploring additional operations based on RLWE homomorphic encryption schemes
+ Ensuring stronger security using mutual authentication of communicating parties using certificates, and

CMPE 295 : Master's Project : Team Warriors (Yuva, Senthil, Sanjeevi, Pradeep)

This project has been submitted to The Faculty of the College of 
Engineering, San Jose State University In Partial Fulfillment
Of the Requirements for the Degree Master of Science in Software Engineering, Dec 2020


## State Model between Client & Server in original PJC code

ID Keys: Encrypted using Communtative Cipher
+ Client & Server have their own commutative ciphers created

Business Data: 
+ Client encrypts using PrivatePallier
    + (1) create N = p*q (safe primes)
    + (2) set that in the context that is transferred between Client & Server
+ Server sums the encrypted values using PublicPallier
    + (1) create using N that is got from Client
+ Client decrypts using PrivatePallier

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

[Microsoft SEAL](https://github.com/microsoft/SEAL) library was built using CMake and the library was statically linked to this code.

For commandline integration options check [SealDev](https://github.com/vsanjeevi/SealDev)

## Building the code

Install [Bazel 0.28.0](https://docs.bazel.build/versions/0.28.0/install-ubuntu.html) 
+ This project was built & tested with this Bazel version only 

### Get Bazel
```
wget https://github.com/bazelbuild/bazel/releases/download/0.28.1/bazel-0.28.1-installer-linux-x86_64.sh
```
### Build process
```
chmod +x bazel-0.28.1-installer-linux-x86_64.sh
./bazel-0.28.1-installer-linux-x86_64.sh --user

export PATH="$PATH:$HOME/bin"
sudo apt-get install openjdk-11-jdk
git clone https://github.com/google/private-join-and-compute.git

bazel build :all --incompatible_disable_deprecated_attr_params=false --incompatible_depset_is_not_iterable=false --incompatible_new_actions_api=false --incompatible_no_support_tools_in_action_inputs=false
--cxxopt='-std=c++17'
```
### Testing base code
```
bazel-bin/generate_dummy_data --server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv

bazel-bin/generate_dummy_data \
--server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv --server_data_size=1000 \
--client_data_size=1000 --intersection_size=200 --max_associated_value=100


bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv

bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv
```
Note: Server code has been modified to continue running to support multiple protocol runs from potentially different clients

### Testing MS SEAL Integration
```
bazel-bin/generate_dummy_data \
--server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv --server_data_size=10 \
--client_data_size=10 --intersection_size=2 --max_associated_value=31

bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1 --use_seal=1 --operator_1=0 --operator_2=1

bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1 --use_seal=1
```
+ Note: (operator_1 and operator_2 have same behavior)
  + operator_1 = 0 computes SUM of the data value, 
  + operator_1 = 1 computes SUMSQ (sum of squares) of the data value,

### Testing mTLS extension

#### mTLS extension requirements (not in repo)
These certificates were generated using 
+ Client
  + client.crt
  + client.key
  + ca.crt
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

#### Generate Certificates using Vault PKI
[Vault PKI Secrets engine](https://www.vaultproject.io/docs/secrets/pki) was used to generate the above certificates

#### Run Server & Client
```
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1 --use_mtls=1 --use_seal=1
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1 --use_mtls=1 --use_seal=1 --operator_1=0 --operator_2=1
```
Server & Client ports can be changed if required using --port1 when using --use_mtls=1 option

The code has been tested on Ubuntu VMs representing two parties with these flags.

### Private Join and Compute documentation from Google 
See [Google's PJC](https://github.com/google/private-join-and-compute)

## Implementation & Debugging
Notes taken during the implementation of this project are provided in the [implementation.md](https://github.com/YuvaAthur/private-join-and-compute/blob/master/Implementation.md) file. This is has not been organized very well but gives an idea of what has been done. 

### Disclaimers
The software is provided as-is without any guarantees or warranties, express or implied.

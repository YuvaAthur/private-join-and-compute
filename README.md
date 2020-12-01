# Adapting to CMPE 295 : Master's Project

In this project we explore the extensions of Google's Private Join and Compute protocol in the following ways:
+ Extending PJC protocol to additional data columns and applying columnar aggregation based on supported homomorphic operators, 
+ Exploring additional operations based on RLWE homomorphic encryption schemes
+ Ensuring stronger security using mutual authentication of communicating parties using certificates, and

This project has been submitted to The Faculty of the College of 
Engineering, San Jose State University In Partial Fulfillment
Of the Requirements for the Degree Master of Science in Software Engineering, Dec 2020


# State Model between Client & Server in original PJC code

ID Keys: Encrypted using Communtative Cipher
+ Client & Server have their own commutative ciphers created

Business Data: 
+ Client encrypts using PrivatePallier
    + (1) create N = p*q (safe primes)
    + (2) set that in the context that is transferred between Client & Server
+ Server sums the encrypted values using PublicPallier
    + (1) create using N that is got from Client
+ Client decrypts using PrivatePallier

# Integration SEAL BFV HE
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



# Building the code

Install [Bazel 0.28.0](https://docs.bazel.build/versions/0.28.0/install-ubuntu.html) 
+ This project was built & tested with this Bazel version only 

## Get Bazel
wget https://github.com/bazelbuild/bazel/releases/download/0.28.1/bazel-0.28.1-installer-linux-x86_64.sh

## Build process

chmod +x bazel-0.28.1-installer-linux-x86_64.sh
./bazel-0.28.1-installer-linux-x86_64.sh --user

export PATH="$PATH:$HOME/bin"
sudo apt-get install openjdk-11-jdk
git clone https://github.com/google/private-join-and-compute.git

bazel build :all --incompatible_disable_deprecated_attr_params=false --incompatible_depset_is_not_iterable=false --incompatible_new_actions_api=false --incompatible_no_support_tools_in_action_inputs=false
--cxxopt='-std=c++17'

## Sample data file generation

bazel-bin/generate_dummy_data --server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv

bazel-bin/generate_dummy_data \
--server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv --server_data_size=1000 \
--client_data_size=1000 --intersection_size=200 --max_associated_value=100

## Start the server
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv


## Run Client Session
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv


## Testing MS SEAL Integration

bazel-bin/generate_dummy_data \
--server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv --server_data_size=10 \
--client_data_size=10 --intersection_size=2 --max_associated_value=31

bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1 --use_seal=1 --operator_1=0 --operator_2=1

bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1 --use_seal=1

## Testing mTLS extension

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
### Run Server & Client
```
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv --multi_column=1 --use_mtls=1 --use_seal=1
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv --multi_column=1 --use_mtls=1 --use_seal=1 --operator_1=0 --operator_2=1
```
Server & Client ports can be changed if required using --port1 when using --use_mtls=1 option

The code has been tested on Ubuntu VMs representing two parties with these flags.

# Original: Private Join and Compute documentation from Google 
Fork from [Google's PJC](https://github.com/google/private-join-and-compute)

This project contains an implementation of the "Private Join and Compute"
functionality. This functionality allows two users, each holding an input file,
to privately compute the sum of associated values for records that have common
identifiers.

In more detail, suppose a Server has a file containing the following
identifiers:

Identifiers |
----------- |
Sam         |
Ada         |
Ruby        |
Brendan     |

And a Client has a file containing the following identifiers, paired with
associated integer values:

Identifiers | Associated Values
----------- | -----------------
Ruby        | 10
Ada         | 30
Alexander   | 5
Mika        | 35

Then the Private Join and Compute functionality would allow the Client to learn
that the input files had *2* identifiers in common, and that the associated
values summed to *40*. It does this *without* revealing which specific
identifiers were in common (Ada and Ruby in the example above), or revealing
anything additional about the other identifiers in the two parties' data set.

Private Join and Compute is a variant of the well-studied Private Set
Intersection functionality. We sometimes also refer to Private Join and Compute
as Private Intersection-Sum.

## How to run the protocol

In order to run Private Join and Compute, you need to install Bazel, if you
don't have it already.
[Follow the instructions for your platform on the Bazel website.](https://docs.bazel.build/versions/master/install.html)

You also need to install Git, if you don't have it already.
[Follow the instructions for your platform on the Git website.](https://git-scm.com/book/en/v2/Getting-Started-Installing-Git)

Once you've installed Bazel and Git, open a Terminal and clone the Private Join
and Compute repository into a local folder:

```shell
git clone https://github.com/google/private-join-and-compute.git
```

Navigate into the `private-join-and-compute` folder you just created, and build
the Private Join and Compute library and dependencies using Bazel:

```bash
cd private-join-and-compute
bazel build :all
```

(All the following instructions must be run from inside the
private-join-and-compute folder.)

Next, generate some dummy data to run the protocol on:

```shell
bazel-bin/generate_dummy_data --server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv
```

This will create dummy data for the server and client at the specified
locations. You can look at the files in `/tmp/dummy_server_data.csv` and
`/tmp/dummy_client_data.csv` to see the dummy data that was generated. You can
also change the size of the dummy data generated using additional flags. For
example:

```shell
bazel-bin/generate_dummy_data \
--server_data_file=/tmp/dummy_server_data.csv \
--client_data_file=/tmp/dummy_client_data.csv --server_data_size=1000 \
--client_data_size=1000 --intersection_size=200 --max_associated_value=100
```

Once you've generated dummy data, you can start the server as follows:

```shell
bazel-bin/server --server_data_file=/tmp/dummy_server_data.csv
```

The server will load data from the specified file, and wait for a connection
from the client.

Once the server is running, you can start a client to connect to the server.
Create a new terminal and navigate to the private-join-and-compute folder. Once
there, run the following command to start the client:

```shell
bazel-bin/client --client_data_file=/tmp/dummy_client_data.csv
```

The client will connect to the server and execute the steps of the protocol
sequentially. At the end of the protocol, the client will output the
Intersection Size (the number of identifiers in common) and the Intersection Sum
(the sum of associated values). If the protocol was successful, both the server
and client will shut down.

## Caveats

Several caveats should be carefully considered before using Private Join and
Compute.

### Security Model

Our protocol has security against honest-but-curious adversaries. This means
that as long as both participants follow the protocol honestly, neither will
learn more than the size of the intersection and the intersection-sum. However,
if a participant deviates from the protocol, it is possible they could learn
more than the prescribed information. For example, they could learn the specific
identifiers in the intersection. If the underlying data is sensitive, we
recommend performing a careful risk analysis before using Private Join and
Compute, to ensure that neither party has an incentive to deviate from the
protocol. The protocol can also be supplemented with external enforcement such
as code audits to ensure that no party deviates from the protocol.

### Maliciously Chosen Inputs

We note that our protocol does not authenticate that parties use "real" input,
nor does it prevent them from arbitrarily changing their input. We suggest
careful analysis of whether any party has an incentive to lie about their
inputs. This risk can also be mitigated by external enforcement such as code
audits.

### Leakage from the Intersection-Sum.

While the Private Join and Compute functionality is supposed to reveal only the
intersection-size and intersection-sum, it is possible that the intersection-sum
itself could reveal something about which identifiers were in common.

For example, if an identifier has a very unique associated integer values, then
it may be easy to detect if that identifier was in the intersection simply by
looking at the intersection-sum. One way this could happen is if one of the
identifiers has a very large associated value compared to all other identifiers.
In that case, if the intersection-sum is large, one could reasonably infer that
that identifier was in the intersection. To mitigate this, we suggest scrubbing
inputs to remove identifiers with "outlier" values.

Another way that the intersection-sum may leak which identifiers are in the
intersection is if the intersection is too small. This could make it easier to
guess which combination of identifiers could be in the intersection in order to
yield a particular intersection-sum. To mitigate this, one could abort the
protocol if the intersection-size is below a certain threshold, or to add noise
to the output of the protocol.

(Note that these mitigations are not currently implemented in this open-source
library.)

## Disclaimers

This is not an officially supported Google product. The software is provided
as-is without any guarantees or warranties, express or implied.

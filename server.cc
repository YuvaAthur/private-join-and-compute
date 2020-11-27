/*
 * Copyright 2019 Google Inc.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*
 * Extension to server for sharing N business data columns
 * Author:          Yuva Athur
 * Created Date:    Nov. 10. 2020
 * 
*/

#include <iostream>
#include <memory>
#include <string>
#include <thread>  // NOLINT

#include <sstream>
#include <fstream>
#include <iostream>
#include <sstream>


#include "gflags/gflags.h"

#include "include/grpc/grpc_security_constants.h"
#include "include/grpcpp/grpcpp.h"
#include "include/grpcpp/security/server_credentials.h"
#include "include/grpcpp/server_builder.h"
#include "include/grpcpp/server_context.h"
#include "include/grpcpp/support/status.h"
#include "data_util.h"
#include "server_impl.h"
#include "server_tuple_impl.h"
#include "private_join_and_compute.grpc.pb.h"
#include "private_join_and_compute_rpc_impl.h"
#include "protocol_server.h"
#include "absl/memory/memory.h"

// switch to use MTLS
DEFINE_int32(use_mtls,0,"Swtich to use MTLS approach,dafault no MTLS");
// for local credentials
DEFINE_string(port0, "0.0.0.0:10501", "Port on which to listen");
// for certificate crendentials
DEFINE_string(port1, "california.sjsu-mtls.com:10501", "Port on which to listen");

DEFINE_string(server_data_file, "",
              "The file from which to read the server database.");
//--mult_column : binary : 0 means 1 column (default), 1 means 2 columns
DEFINE_int32(multi_column,0,"Indicates 1 or 2 columns in the Client Data Set");

//integrating Microsoft SEAL FHE
DEFINE_int32(use_seal,0,"Defines whether SEAL is to be used");


int RunServer() {

  std::cout << "Server: Command line " << std::endl;
  std::cout << "    + use_mtls (0: local, 1: cert) " << std::int32_t(FLAGS_use_mtls) << std::endl;
  std::cout << "    + use_seal (0: no, 1: yes) " << std::int32_t(FLAGS_use_seal) << std::endl;
  std::cout << "    + multi_column (0: 1 Column, 1: 2 Columns) " << std::int32_t(FLAGS_multi_column) << std::endl;


  std::cout << "Server: loading data... " << std::endl;
  auto maybe_server_identifiers =
      ::private_join_and_compute::ReadServerDatasetFromFile(FLAGS_server_data_file);
  if (!maybe_server_identifiers.ok()) {
    std::cerr << "RunServer: failed " << maybe_server_identifiers.status()
              << std::endl;
    return 1;
  }


  ::private_join_and_compute::Context context;
  std::unique_ptr<::private_join_and_compute::ProtocolServer> server = nullptr;

  //YAR: Set up the server implementation based on 1 or 2 columns
  if(std::int32_t(FLAGS_multi_column)){
  // 2 columns tuple implementation
  server =
      absl::make_unique<::private_join_and_compute::PrivateIntersectionSumProtocolServerTupleImpl>(
          &context, std::move(maybe_server_identifiers.value()),FLAGS_use_seal); 
  } else {
  //existing pair implementation
  server =
      absl::make_unique<::private_join_and_compute::PrivateIntersectionSumProtocolServerImpl>(
          &context, std::move(maybe_server_identifiers.value()));  
  }

  ::private_join_and_compute::PrivateJoinAndComputeRpcImpl service(std::move(server));

  ::grpc::ServerBuilder builder;

  // YAR: Choose implementation based on MTLS switch
  if(std::int32_t(FLAGS_use_mtls)){
    // Certificate based MTLS implementation
    grpc::SslServerCredentialsOptions sslOpts{};
    sslOpts.client_certificate_request = GRPC_SSL_REQUEST_AND_REQUIRE_CLIENT_CERTIFICATE_AND_VERIFY;

    auto contents = [](const std::string& filename) -> std::string {
        std::ifstream fh(filename);
        std::stringstream buffer;
        buffer << fh.rdbuf();
        fh.close();
        return buffer.str();
    };

    std::string key = contents ("server.key");
    std::string cert = contents ("server.crt");
    std::string root = contents ("ca.crt");

    grpc::SslServerCredentialsOptions::PemKeyCertPair keycert = { key,cert };

    sslOpts.pem_root_certs = root;
    sslOpts.pem_key_cert_pairs.push_back(keycert);
    auto creds = grpc::SslServerCredentials(sslOpts);
    builder.AddListeningPort(FLAGS_port1,creds);    
  } else {
    builder.AddListeningPort(FLAGS_port0,
                           ::grpc::experimental::LocalServerCredentials(
                               grpc_local_connect_type::LOCAL_TCP));
  }  
  
  builder.RegisterService(&service);
  std::unique_ptr<::grpc::Server> grpc_server(builder.BuildAndStart());


  // Run the server on a background thread.
  std::thread grpc_server_thread(
      [](::grpc::Server* grpc_server_ptr) {
        if (int32_t(FLAGS_use_mtls)){
          std::cout << "Server: listening on " << FLAGS_port1 << std::endl;
        }else{
          std::cout << "Server: listening on " << FLAGS_port0 << std::endl;
        }
        
        grpc_server_ptr->Wait();
      },
      grpc_server.get());

  while (!service.protocol_finished()) {
    // Wait for the server to be done, and then shut the server down.
  }

  // Shut down server.
  grpc_server->Shutdown();
  grpc_server_thread.join();
  std::cout << "Server completed protocol and shut down." << std::endl;

  return 0;
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  return RunServer();
}

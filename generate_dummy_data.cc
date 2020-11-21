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
 * Extension to generate_dummy_data for generating 2 data columns with 2 possible aggregators
 * Author:          Yuva Athur
 * Created Date:    Nov. 10. 2020
 * 
 * 
 *
 *  
 */

// Tool to generate dummy data for the client and server in Private Join and
// Compute.

#include "gflags/gflags.h"

#include "glog/logging.h"
#include "data_util.h"

// Flags defining the size of data to generate for the client and server, bounds
// on the associated values, and where the write the outputs.
DEFINE_int64(server_data_size, 100,
             "Number of dummy identifiers in server database.");
DEFINE_int64(
    client_data_size, 100,
    "Number of dummy identifiers and associated values in client database.");
DEFINE_int64(intersection_size, 50,
             "Number of items in the intersection. Must be less than the "
             "server and client data sizes.");
DEFINE_int64(max_associated_value, 100,
             "Dummy associated values for the client will be between 0 and "
             "this. Must be nonnegative.");
DEFINE_string(server_data_file, "",
              "The file to which to write the server database.");
DEFINE_string(client_data_file, "",
              "The file to which to write the client database.");

//always generate 2 columns - client read will take care!
//--mult_column : binary : 0 means 1 column (default), 1 means 2 columns
//DEFINE_int32(multi_column,0,"Indicates 1 or 2 columns in the Client Data Set");

//enumerator value sent in protocol
DEFINE_int32(operator_1,0,"The aggregation operation to be applied to first associated data column");
DEFINE_int32(operator_2,0,"The aggregation operation to be applied to second associated data column");
    // SUM = 0;
    // SUMSQ = 1;
    // VARN = 2;


int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  auto maybe_dummy_data = private_join_and_compute::GenerateRandomDatabases(
      FLAGS_server_data_size, FLAGS_client_data_size, FLAGS_intersection_size,
      FLAGS_max_associated_value, FLAGS_operator_1,FLAGS_operator_2);

  if (!maybe_dummy_data.ok()) {
    std::cerr << "GenerateDummyData: Error generating the dummy data: "
              << maybe_dummy_data.status() << std::endl;
    return 1;
  }

  auto dummy_data = std::move(maybe_dummy_data.value());
  auto& server_identifiers = std::get<0>(dummy_data);
  auto& client_identifiers_and_associated_values = std::get<1>(dummy_data);
  int64_t intersection_agg_1 = std::get<2>(dummy_data);
  int64_t intersection_agg_2 = std::get<3>(dummy_data);


  auto server_write_status = private_join_and_compute::WriteServerDatasetToFile(
      server_identifiers, FLAGS_server_data_file);
  if (!server_write_status.ok()) {
    std::cerr << "GenerateDummyData: Error writing server dataset: "
              << server_write_status << std::endl;
    return 1;
  }

  //Extending to tuple
  auto client_write_status = private_join_and_compute::WriteClientDatasetToFile(
    client_identifiers_and_associated_values, FLAGS_client_data_file);
  if (!client_write_status.ok()) {
    std::cerr << "GenerateDummyData: Error writing client dataset: "
            << client_write_status << std::endl;
    return 1;
  }
  

  std::cout << "Generated Server dataset of size " << FLAGS_client_data_size
            << ", Client dataset of size " << FLAGS_client_data_size
            << std::endl;
  std::cout << "Passed flags passed aggregators: Operator 1 = " << FLAGS_operator_1
            << ", Operator 2 = " << FLAGS_operator_2
            << std::endl;

  std::cout << "Intersection size = " << FLAGS_intersection_size << std::endl;
  std::cout << "Intersection aggregate 1 = " << intersection_agg_1 << std::endl;
  std::cout << "Intersection aggregate 2 = " << intersection_agg_2 << std::endl;


  return 0;
}



/**************************************************************/
/*
    auto client_write_status = private_join_and_compute::WriteClientDatasetToFile(
      client_identifiers_and_associated_values.first,
      client_identifiers_and_associated_values.second, FLAGS_client_data_file);
    if (!client_write_status.ok()) {
      std::cerr << "GenerateDummyData: Error writing client dataset: "
              << client_write_status << std::endl;
      return 1;
    }

*/
/*
 * Extension to server_impl for sharing 2 business data columns
 * Author:          Yuva Athur
 * Created Date:    Nov. 10. 2020
 * 
 * 
 * Follows the same implementation approach as PrivateIntersectionSumProtocolServerImpl
 *   Created a new class instead - inheritance is not straight forward
 *
 *  
 */

#include "server_tuple_impl.h"

#include <algorithm>

#include "crypto/paillier.h"
#include "util/status.inc"
#include "crypto/ec_commutative_cipher.h"
#include "absl/memory/memory.h"

using ::private_join_and_compute::BigNum;
using ::private_join_and_compute::ECCommutativeCipher;
using ::private_join_and_compute::PublicPaillier;

namespace private_join_and_compute {

Status PrivateIntersectionSumProtocolServerTupleImpl::setupSEAL(){
  // parms
  seal::EncryptionParameters parms(seal::scheme_type::BFV);
  size_t poly_modulus_degree = 4096;
  parms.set_poly_modulus_degree(poly_modulus_degree);
  parms.set_coeff_modulus(seal::CoeffModulus::BFVDefault(poly_modulus_degree));
  parms.set_plain_modulus(1024);

  // key_generator
  context_ = seal::SEALContext::Create(parms);

  return OkStatus();
}


StatusOr<PrivateIntersectionSumServerMessage::ServerRoundOne>
PrivateIntersectionSumProtocolServerTupleImpl::EncryptSet() {
  if (ec_cipher_ != nullptr) {
    return InvalidArgumentError("Attempted to call EncryptSet twice.");
  }
  StatusOr<std::unique_ptr<ECCommutativeCipher>> ec_cipher =
      ECCommutativeCipher::CreateWithNewKey(
          NID_X9_62_prime256v1, ECCommutativeCipher::HashType::SHA256);
  if (!ec_cipher.ok()) {
    return ec_cipher.status();
  }
  ec_cipher_ = std::move(ec_cipher.value());

  PrivateIntersectionSumServerMessage::ServerRoundOne result;
  for (const std::string& input : inputs_) {
    EncryptedElement* encrypted =
        result.mutable_encrypted_set()->add_elements();
    StatusOr<std::string> encrypted_element = ec_cipher_->Encrypt(input);
    if (!encrypted_element.ok()) {
      return encrypted_element.status();
    }
    *encrypted->mutable_element() = encrypted_element.value();
  }

  return result;
}

StatusOr<PrivateIntersectionSumServerMessage::ServerRoundTwo>
PrivateIntersectionSumProtocolServerTupleImpl::ComputeIntersection(
    const PrivateIntersectionSumClientMessage::ClientRoundOne& client_message) {
  if (ec_cipher_ == nullptr) {
    return InvalidArgumentError(
        "Called ComputeIntersection before EncryptSet.");
  }
  PrivateIntersectionSumServerMessage::ServerRoundTwo result;

  BigNum N = ctx_->CreateBigNum(client_message.public_key());
  PublicPaillier public_paillier(ctx_, N, 2);

  //set the requested operators
  op_1_ = client_message.op_code_1();
  op_2_ = client_message.op_code_2();


  std::vector<EncryptedElement> server_set, client_set, intersection;

  auto encryptedSet = client_message.encrypted_set();

  // First, we re-encrypt the client party's set, so that we can compare with
  // the re-encrypted set received from the client.
  auto maybe_client_set = EncryptClientSet(encryptedSet);

  if (!maybe_client_set.ok()) {
    return maybe_client_set.status();
  }
  client_set = std::move(maybe_client_set.value());

  for (const EncryptedElement& element :
       client_message.reencrypted_set().elements()) {
    server_set.push_back(element);
  }

  // std::set_intersection requires sorted inputs.
  std::sort(client_set.begin(), client_set.end(),
            [](const EncryptedElement& a, const EncryptedElement& b) {
              return a.element() < b.element();
            });
  std::sort(server_set.begin(), server_set.end(),
            [](const EncryptedElement& a, const EncryptedElement& b) {
              return a.element() < b.element();
            });
  std::set_intersection(
      client_set.begin(), client_set.end(), server_set.begin(),
      server_set.end(), std::back_inserter(intersection),
      [](const EncryptedElement& a, const EncryptedElement& b) {
        return a.element() < b.element();
      });

  std::cout << " Returning intersection size as " << intersection.size() << std::endl; 


  // From the intersection we compute the aggregation of the associated values, which is
  // the result we return to the client.
  auto maybe_aggregates = IntersectionAggregates(public_paillier,intersection);
  if(!maybe_aggregates.ok()){
    return maybe_aggregates.status();
  }

  auto aggregates = std::move(maybe_aggregates.value());
  if(!use_seal_){
    BigNum sum_1 = aggregates[0]; 
    BigNum sum_2 = aggregates[1]; 

    *result.mutable_encrypted_sum_1() = sum_1.ToBytes();
    *result.mutable_encrypted_sum_2() = sum_2.ToBytes();
  } else {
    std::stringstream hex_string;
    seal_sum_1_.save(hex_string); 
    *result.mutable_encrypted_sum_1() = hex_string.str();
    seal_sum_2_.save(hex_string);
    *result.mutable_encrypted_sum_2() = hex_string.str();
  }

  result.set_intersection_size(intersection.size());
  return result;
}

// YAR::Add : Refactoring the encrypting part of the client message
StatusOr<std::vector<EncryptedElement>>
PrivateIntersectionSumProtocolServerTupleImpl::EncryptClientSet(
  const private_join_and_compute::EncryptedSet encryptedSet){
  std::vector<EncryptedElement> client_set;
  for (const EncryptedElement& element : encryptedSet.elements() ){
    EncryptedElement reencrypted;

    *reencrypted.mutable_associated_data_1() = element.associated_data_1();
    *reencrypted.mutable_associated_data_2() = element.associated_data_2();

    StatusOr<std::string> reenc = ec_cipher_->ReEncrypt(element.element());
    if (!reenc.ok()) {
      return reenc.status();
    }
    *reencrypted.mutable_element() = reenc.value();
    client_set.push_back(reencrypted);
  }

  return client_set;
}

// YAR::Add : Refactoring the computation of aggregates
StatusOr<std::vector<BigNum>>
PrivateIntersectionSumProtocolServerTupleImpl::IntersectionAggregates(
  const PublicPaillier& public_paillier,
  const std::vector<EncryptedElement>& intersection){

  StatusOr<BigNum> encrypted_zero =
      public_paillier.Encrypt(ctx_->CreateBigNum(0));
  if (!encrypted_zero.ok()) {
    return encrypted_zero.status();
  }

  //reverting to simple list of 2 sums
  BigNum sum_1 = encrypted_zero.value();
  BigNum sum_2 = encrypted_zero.value();

  //Setting up SEAL Aggregation
  seal::Evaluator evaluator(context_);  

  seal::Ciphertext seal_sum_1;
  seal::Ciphertext seal_sum_2;

  seal::Ciphertext seal_data_1;
  seal::Ciphertext seal_data_2;

  std::vector<BigNum> aggregates;
  for (const EncryptedElement& element : intersection) {
    if(!op_1_) { // default is sum
      if(!use_seal_){
        sum_1 =
          public_paillier.Add(sum_1, ctx_->CreateBigNum(element.associated_data_1()));
      } else {
        std::stringstream hex_string(element.associated_data_1());
        seal_data_1.load(context_,hex_string);
        evaluator.add_inplace(seal_sum_1,seal_data_1);
      }
    } else { // sum of squares
      // if(use_seal_){
      //   std::stringstream hex_string(element.associated_data_1());
      //   seal_data_1.load(context_,hex_string);
      //   evaluator.square_inplace(seal_data_1);
      //   evaluator.add_inplace(seal_sum_1,seal_data_1);
      // }
    }
    if(!op_2_){ // default is sum
      if(!use_seal_){
        sum_2 =
          public_paillier.Add(sum_2, ctx_->CreateBigNum(element.associated_data_2()));
      } else {
        std::stringstream hex_string(element.associated_data_2());
        seal_data_2.load(context_,hex_string);
        evaluator.add_inplace(seal_sum_2,seal_data_2);
      }
    } else { // sum of squares
      // if(use_seal_){
      //   std::stringstream hex_string(element.associated_data_2());
      //   seal_data_2.load(context_,hex_string);
      //   evaluator.square_inplace(seal_data_2);
      //   evaluator.add_inplace(seal_sum_2,seal_data_2);
      // }
    }
  }

  aggregates.push_back(sum_1);
  aggregates.push_back(sum_2);

  // seal_sum_1_=seal_sum_1;
  // seal_sum_2_=seal_sum_2;


  return aggregates;
}


Status PrivateIntersectionSumProtocolServerTupleImpl::Handle(
    const ClientMessage& request,
    MessageSink<ServerMessage>* server_message_sink) {
  if (protocol_finished()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolServerImpl: Protocol is already "
        "complete.");
  }

  // Check that the message is a PrivateIntersectionSum protocol message.
  if (!request.has_private_intersection_sum_client_message()) {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolServerImpl: Received a message for the "
        "wrong protocol type");
  }
  const PrivateIntersectionSumClientMessage& client_message =
      request.private_intersection_sum_client_message();

  ServerMessage server_message;

  if (client_message.has_start_protocol_request()) {
    // Handle a protocol start message.
    auto maybe_server_round_one = EncryptSet();
    if (!maybe_server_round_one.ok()) {
      return maybe_server_round_one.status();
    }
    *(server_message.mutable_private_intersection_sum_server_message()
          ->mutable_server_round_one()) =
        std::move(maybe_server_round_one.value());
  } else if (client_message.has_client_round_one()) {
    // Handle the client round 1 message.
    auto maybe_server_round_two =
        ComputeIntersection(client_message.client_round_one());
    if (!maybe_server_round_two.ok()) {
      return maybe_server_round_two.status();
    }
    *(server_message.mutable_private_intersection_sum_server_message()
          ->mutable_server_round_two()) =
        std::move(maybe_server_round_two.value());
    // Mark the protocol as finished here.
    //YAR::Edit Keep Server on
    {
      protocol_finished_=false;
      ec_cipher_ = nullptr;     //reset ec_cipher_ for new round
    }
    //protocol_finished_ = true;
  } else {
    return InvalidArgumentError(
        "PrivateIntersectionSumProtocolServerImpl: Received a client message "
        "of an unknown type.");
  }

  return server_message_sink->Send(server_message);
}

}  // namespace private_join_and_compute

/*********************************************************************************************/
/*
    //----- begin: introducing SEAL integration using command line
    char y='6';
    std::string a = "./bazel-bin/sealexamples ";
    a += y;
    std::cout << "VALUE OF THE STRING IS " << a << std::endl;
    std::system(a.c_str());
    //------ end: introducing SEAL integration using command line
*/


/*********************************************************************************************/
/* Two associated_data() implementation
StatusOr<PrivateIntersectionSumServerMessage::ServerRoundTwo>
PrivateIntersectionSumProtocolServerImpl::ComputeIntersection(
    const PrivateIntersectionSumClientMessage::ClientRoundOne& client_message) {
  if (ec_cipher_ == nullptr) {
    return InvalidArgumentError(
        "Called ComputeIntersection before EncryptSet.");
  }
  PrivateIntersectionSumServerMessage::ServerRoundTwo result;
  BigNum N = ctx_->CreateBigNum(client_message.public_key());
  PublicPaillier public_paillier(ctx_, N, 2);

  std::vector<EncryptedElement> server_set, client_set, intersection;

  // First, we re-encrypt the client party's set, so that we can compare with
  // the re-encrypted set received from the client.
  for (const EncryptedElement& element :
       client_message.encrypted_set().elements()) {
    EncryptedElement reencrypted;
    //YAR::Edit : Two associated data elements
    *reencrypted.mutable_associated_data_1() = element.associated_data_1();
    *reencrypted.mutable_associated_data_2() = element.associated_data_2();
    StatusOr<std::string> reenc = ec_cipher_->ReEncrypt(element.element());
    if (!reenc.ok()) {
      return reenc.status();
    }
    *reencrypted.mutable_element() = reenc.value();
    client_set.push_back(reencrypted);
  }
  for (const EncryptedElement& element :
       client_message.reencrypted_set().elements()) {
    server_set.push_back(element);
  }

  // std::set_intersection requires sorted inputs.
  std::sort(client_set.begin(), client_set.end(),
            [](const EncryptedElement& a, const EncryptedElement& b) {
              return a.element() < b.element();
            });
  std::sort(server_set.begin(), server_set.end(),
            [](const EncryptedElement& a, const EncryptedElement& b) {
              return a.element() < b.element();
            });
  std::set_intersection(
      client_set.begin(), client_set.end(), server_set.begin(),
      server_set.end(), std::back_inserter(intersection),
      [](const EncryptedElement& a, const EncryptedElement& b) {
        return a.element() < b.element();
      });

  // From the intersection we compute the sum of the associated values, which is
  // the result we return to the client.
  StatusOr<BigNum> encrypted_zero =
      public_paillier.Encrypt(ctx_->CreateBigNum(0));
  if (!encrypted_zero.ok()) {
    return encrypted_zero.status();
  }
  //YAR::Edit : Two sums to be computed
  //  Using 1 value for both vectors --> Should this be 2 values?
  BigNum sum_1 = encrypted_zero.value();
  BigNum sum_2 = encrypted_zero.value();
  for (const EncryptedElement& element : intersection) {
    sum_1 =
        public_paillier.Add(sum_1, ctx_->CreateBigNum(element.associated_data_1()));
    sum_2 =
        public_paillier.Add(sum_2, ctx_->CreateBigNum(element.associated_data_2()));
  }

  *result.mutable_encrypted_sum_1() = sum_1.ToBytes();
  *result.mutable_encrypted_sum_2() = sum_2.ToBytes();
  result.set_intersection_size(intersection.size());
  return result;
}
*/
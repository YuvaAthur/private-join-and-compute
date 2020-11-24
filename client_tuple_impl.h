/*
 * Extension to client_impl for sharing 2 business data columns
 * Author:          Yuva Athur
 * Created Date:    Nov. 10. 2020
 * 
 * 
 * Follows the same implementation approach as PrivateIntersectionSumProtocolClientImpl
 *   Created a new class instead - inheritance is not straight forward
 *
 *  
 */

#ifndef OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TUPLE_IMPL_H_
#define OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TUPLE_IMPL_H_

#include "crypto/context.h"
#include "crypto/paillier.h"
#include "match.pb.h"
#include "message_sink.h"
#include "private_intersection_sum.pb.h"
#include "private_join_and_compute.pb.h"
#include "protocol_client.h"
#include "util/status.inc"
#include "crypto/ec_commutative_cipher.h"

// Microsoft SEAL Integration 
#include "seal/seal.h"


namespace private_join_and_compute {

// This class represents the "client" part of the intersection-sum protocol,
// which supplies the associated values that will be used to compute the sum.
// This is the party that will receive the sum as output.
class PrivateIntersectionSumProtocolClientTupleImpl : public ProtocolClient {
 public:
 
  //Constructor using tuple instead of pair
  PrivateIntersectionSumProtocolClientTupleImpl(
      Context* ctx, const std::tuple<std::vector<std::string>, 
      std::vector<BigNum>, std::vector<BigNum>>& table, int32_t modulus_size,
      const int32_t op_1,const int32_t op_2, const int32_t use_seal);


  // Generates the StartProtocol message and sends it on the message sink.
  Status StartProtocol(
      MessageSink<ClientMessage>* client_message_sink) override;

  // Executes the next Client round and creates a new server request, which must
  // be sent to the server unless the protocol is finished.
  //
  // If the ServerMessage is ServerRoundOne, a ClientRoundOne will be sent on
  // the message sink, containing the encrypted client identifiers and
  // associated values, and the re-encrypted and shuffled server identifiers.
  //
  // If the ServerMessage is ServerRoundTwo, nothing will be sent on
  // the message sink, and the client will internally store the intersection sum
  // and size. The intersection sum and size can be retrieved either through
  // accessors, or by calling PrintOutput.
  //
  // Fails with InvalidArgument if the message is not a
  // PrivateIntersectionSumServerMessage of the expected round, or if the
  // message is otherwise not as expected. Forwards all other failures
  // encountered.
  Status Handle(const ServerMessage& server_message,
                MessageSink<ClientMessage>* client_message_sink) override;

  // Prints the result, namely the intersection size and the intersection sum.
  Status PrintOutput() override;

  bool protocol_finished() override { return protocol_finished_; }

  // Utility functions for testing.
  int64_t intersection_size() const { return intersection_size_; }
  // Extending to 2 aggregates
  const BigNum& intersection_agg_1() const { return intersection_agg_1_; }
  const BigNum& intersection_agg_2() const { return intersection_agg_2_; }

  int32_t operator_1() const { return op_1_;}
  int32_t operator_2() const { return op_2_;}

 
 protected:
  // YAR: Microsoft SEAL integration 
  // Set up member variables 
  // Context parms is a constant between client & server
  Status setupSEAL();


  // The server sends the first message of the protocol, which contains its
  // encrypted set.  This party then re-encrypts that set and replies with the
  // reencrypted values and its own encrypted set.
  virtual StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> ReEncryptSet(
      const PrivateIntersectionSumServerMessage::ServerRoundOne&
          server_message);

  // This function will directly set the internal variables
  virtual StatusOr<PrivateIntersectionSumClientMessage::ClientRoundOne> EncryptCol();

  // After the server computes the intersection-sum, it will send it back to
  // this party for decryption, together with the intersection_size. This party
  // will decrypt and output the intersection sum and intersection size.
  
  // This function will directly set the internal variables
  virtual Status DecryptResult(
    const PrivateIntersectionSumServerMessage::ServerRoundTwo& server_message); 


  Context* ctx_;  // not owned

  // Table with 2 columns
  std::tuple<std::vector<std::string>, std::vector<BigNum>, std::vector<BigNum>> table_;


  // The Paillier private key
  BigNum p_, q_;

  // These values will hold the intersection sum and size when the protocol has
  // been completed.
  int64_t intersection_size_ = 0;
   
  // Extending to 2 sums
  BigNum intersection_agg_1_;
  BigNum intersection_agg_2_;

  //command line extension
  int32_t op_1_;
  int32_t op_2_;
  int32_t use_seal_;


  std::unique_ptr<ECCommutativeCipher> ec_cipher_;
  std::unique_ptr<PrivatePaillier> private_paillier_;

  //YAR: seal integration
  // getting linker errors on vtable of memory manager
  seal::EncryptionParameters parms_;
  std::shared_ptr<seal::SEALContext> context_;  
  // seal::KeyGenerator key_generator_; no dafault constructor
  seal::SecretKey secret_key_;
  seal::PublicKey public_key_; 

  seal::Plaintext decrypt_sum_1_,decrypt_sum_2_;

  bool protocol_finished_ = false;
};

}  // namespace private_join_and_compute

#endif  // OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_CLIENT_TUPLE_IMPL_H_

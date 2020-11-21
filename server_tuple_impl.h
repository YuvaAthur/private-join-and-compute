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

#ifndef OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_SERVER_TUPLE_IMPL_H_
#define OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_SERVER_TUPLE_IMPL_H_

#include "crypto/context.h"
#include "crypto/paillier.h"
#include "match.pb.h"
#include "message_sink.h"
#include "private_intersection_sum.pb.h"
#include "private_join_and_compute.pb.h"
#include "protocol_server.h"
#include "util/status.inc"
#include "crypto/ec_commutative_cipher.h"

namespace private_join_and_compute {

// The "server side" of the intersection-sum protocol.  This represents the
// party that will receive the size of the intersection as its output.  The
// values that will be summed are supplied by the other party; this party will
// only supply set elements as its inputs.
class PrivateIntersectionSumProtocolServerTupleImpl : public ProtocolServer {
 public:
  PrivateIntersectionSumProtocolServerTupleImpl(::private_join_and_compute::Context* ctx,
                                           std::vector<std::string> inputs)
      : ctx_(ctx), inputs_(std::move(inputs)) {}

  ~PrivateIntersectionSumProtocolServerTupleImpl() override = default;

  // Executes the next Server round and creates a response.
  //
  // If the ClientMessage is StartProtocol, a ServerRoundOne will be sent to the
  // message sink, containing the encrypted server identifiers.
  //
  // If the ClientMessage is ClientRoundOne, a ServerRoundTwo will be sent to
  // the message sink, containing the intersection size, and encrypted
  // intersection-sum.
  //
  // Fails with InvalidArgument if the message is not a
  // PrivateIntersectionSumClientMessage of the expected round, or if the
  // message is otherwise not as expected. Forwards all other failures
  // encountered.
  Status Handle(const ClientMessage& request,
                MessageSink<ServerMessage>* server_message_sink) override;

  bool protocol_finished() override { return protocol_finished_; }

  // Utility function, used for testing.
  ECCommutativeCipher* GetECCipher() { return ec_cipher_.get(); }

 protected:
  // Encrypts the server's identifiers.
  StatusOr<PrivateIntersectionSumServerMessage::ServerRoundOne> EncryptSet();

  // YAR::Add : Refactoring the encrypting part of the client message
  virtual StatusOr<std::vector<EncryptedElement>> EncryptClientSet(
    const private_join_and_compute::EncryptedSet encryptedSet);
 
  //YAR::Add : Refactoring to extend to vector of values
  virtual StatusOr<std::vector<BigNum>> IntersectionAggregates(
    const PublicPaillier& public_paillier,
    const std::vector<EncryptedElement>& intersection);

  // Computes the intersection size and encrypted intersection_sum.
  StatusOr<PrivateIntersectionSumServerMessage::ServerRoundTwo>
  ComputeIntersection(const PrivateIntersectionSumClientMessage::ClientRoundOne&
                          client_message);

  Context* ctx_;  // not owned
  std::unique_ptr<ECCommutativeCipher> ec_cipher_;

  // inputs_ will first contain the plaintext server identifiers, and later
  // contain the encrypted server identifiers.
  std::vector<std::string> inputs_;
  bool protocol_finished_ = false;

  //compute methods
  int32_t op_1_;
  int32_t op_2_;
};

}  // namespace private_join_and_compute

#endif  // OPEN_SOURCE_PRIVATE_INTERSECTION_SUM_SERVER_TUPLE_IMPL_H_

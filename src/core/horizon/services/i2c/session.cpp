#include "core/horizon/services/i2c/session.hpp"

namespace hydra::horizon::services::i2c {

DEFINE_SERVICE_COMMAND_TABLE(ISession, 10, send, 11, receive, 12,
                             executeCommandList)

result_t ISession::send(TransactionOption transaction_option,
                        InBuffer<BufferAttr::AutoSelect> in_data_buffer) {
    (void)in_data_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "transaction option: {}",
                               transaction_option);

    return RESULT_SUCCESS;
}

result_t ISession::receive(TransactionOption transaction_option,
                           OutBuffer<BufferAttr::AutoSelect> out_data_buffer) {
    (void)out_data_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "transaction option: {}",
                               transaction_option);

    return RESULT_SUCCESS;
}

result_t ISession::executeCommandList(
    InBuffer<BufferAttr::HipcPointer> in_command_buffer,
    OutBuffer<BufferAttr::AutoSelect> out_receive_buffer) {
    (void)in_command_buffer;
    (void)out_receive_buffer;
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::i2c

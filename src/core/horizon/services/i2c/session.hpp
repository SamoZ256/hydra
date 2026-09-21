#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/i2c/const.hpp"

namespace hydra::horizon::services::i2c {

enum class TransactionOption {
    StartCondition = 1,
    StopCondition = 2,
};

class ISession : public IService {
  public:
    explicit ISession(I2cDevice device_) : device{device_} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // TODO: use
    [[maybe_unused]] I2cDevice device;

    // Commands
    result_t send(TransactionOption transaction_option,
                  InBuffer<BufferAttr::AutoSelect> in_data_buffer);
    result_t receive(TransactionOption transaction_option,
                     OutBuffer<BufferAttr::AutoSelect> out_data_buffer);
    result_t
    executeCommandList(InBuffer<BufferAttr::HipcPointer> in_command_buffer,
                       OutBuffer<BufferAttr::AutoSelect> out_receive_buffer);
};

} // namespace hydra::horizon::services::i2c

ENABLE_ENUM_FORMATTING(hydra::horizon::services::i2c::TransactionOption,
                       StartCondition, "start condition", StopCondition,
                       "stop condition")

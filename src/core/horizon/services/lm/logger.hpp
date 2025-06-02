#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::lm {

struct Packet {
    std::string message;
    u32 line{0};
    std::string filename;
    std::string function;
    std::string module;
    std::string thread;
    u64 drop_count{0};
    u64 time{0};
    std::string program_name;
};

class ILogger : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    Packet packet{};

    // Commands
    result_t Log(InBuffer<BufferAttr::MapAlias> buffer); // TODO: MapAlias?
};

} // namespace hydra::horizon::services::lm

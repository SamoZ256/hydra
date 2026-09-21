#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/ssl/sf/const.hpp"

namespace hydra::horizon::services::ssl::sf {

enum class CertificateFormat : u32 {
    Pem = 1,
    Der = 2,
};

enum class InternalPki : u32 {
    None = 0,
    DeviceClientCertDefault = 1,
};

class ISslContext : public IService {
  public:
    explicit ISslContext(const SslVersion version_) : version{version_} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // TODO: use
    [[maybe_unused]] SslVersion version;

    // Commands
    result_t importServerPki(CertificateFormat cert_format,
                             InBuffer<BufferAttr::MapAlias> in_buffer,
                             u64* out_id);
    result_t registerInternalPki(InternalPki pki, u64* out_id);
};

} // namespace hydra::horizon::services::ssl::sf

ENABLE_ENUM_FORMATTING(hydra::horizon::services::ssl::sf::CertificateFormat,
                       Pem, "PEM", Der, "DER")

ENABLE_ENUM_FORMATTING(hydra::horizon::services::ssl::sf::InternalPki, None,
                       "none", DeviceClientCertDefault,
                       "device client cert default")

#include "core/horizon/services/ssl/sf/ssl_context.hpp"

namespace hydra::horizon::services::ssl::sf {

DEFINE_SERVICE_COMMAND_TABLE(ISslContext, 4, importServerPki, 8,
                             registerInternalPki)

result_t ISslContext::importServerPki(CertificateFormat cert_format,
                                      InBuffer<BufferAttr::MapAlias> in_buffer,
                                      u64* out_id) {
    (void)in_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "certificate format: {}", cert_format);

    // HACK
    *out_id = 0;
    return RESULT_SUCCESS;
}

result_t ISslContext::registerInternalPki(InternalPki pki, u64* out_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "PKI: {}", pki);

    // HACK
    *out_id = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ssl::sf

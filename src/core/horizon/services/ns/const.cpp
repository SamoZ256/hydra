#include "core/horizon/services/ns/const.hpp"

namespace hydra::horizon::services::ns {

const ApplicationTitle&
ApplicationControlProperty::GetApplicationTitle(SystemLanguage lang) const {
    u32 index = static_cast<u32>(lang);
    if (index >= sizeof_array(titles))
        index = 0;

    // Check if the language is set
    if (titles[index].IsValid())
        return titles[index];

    // Otherwise just return the first valid title
    for (const auto& title : titles) {
        if (title.IsValid())
            return title;
    }

    // Fallback to the first title
    return titles[0];
}

} // namespace hydra::horizon::services::ns

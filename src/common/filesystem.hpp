#pragma once

#include <fstream>

#include <CoreFoundation/CoreFoundation.h>

#include "common/log.hpp"
#include "common/platform.hpp"

namespace hydra {

#ifdef PLATFORM_APPLE
inline std::string GetBundleResourcePath(const std::string& filename) {
    CFBundleRef main_bundle = CFBundleGetMainBundle();
    if (main_bundle == nullptr) {
        LOG_FATAL(Common, APP_NAME " is not a bundle");
        return "";
    }

    CFStringRef cf_filename = CFStringCreateWithCString(
        nullptr, filename.c_str(), kCFStringEncodingUTF8);
    CFURLRef resource_url =
        CFBundleCopyResourceURL(main_bundle, cf_filename, nullptr, nullptr);

    if (resource_url == nullptr) {
        CFRelease(cf_filename);
        return "";
    }

    CFStringRef resource_path =
        CFURLCopyFileSystemPath(resource_url, kCFURLPOSIXPathStyle);
    std::array<char, PATH_MAX> path;
    CFStringGetCString(resource_path, path.data(), PATH_MAX,
                       kCFStringEncodingUTF8);

    CFRelease(cf_filename);
    CFRelease(resource_url);
    CFRelease(resource_path);

    return {path.data(), path.size()};
}
#endif

} // namespace hydra

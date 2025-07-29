#pragma once

#include <fstream>

#include <CoreFoundation/CoreFoundation.h>

namespace hydra {

inline std::string get_bundle_resource_path(const std::string& filename) {
    CFBundleRef main_bundle = CFBundleGetMainBundle();
    CFStringRef cf_filename = CFStringCreateWithCString(NULL, filename.c_str(),
                                                        kCFStringEncodingUTF8);
    CFURLRef resource_url =
        CFBundleCopyResourceURL(main_bundle, cf_filename, NULL, NULL);

    if (resource_url == NULL) {
        CFRelease(cf_filename);
        return "";
    }

    CFStringRef resource_path =
        CFURLCopyFileSystemPath(resource_url, kCFURLPOSIXPathStyle);
    char path[PATH_MAX];
    CFStringGetCString(resource_path, path, PATH_MAX, kCFStringEncodingUTF8);

    CFRelease(cf_filename);
    CFRelease(resource_url);
    CFRelease(resource_path);

    return std::string(path);
}

} // namespace hydra

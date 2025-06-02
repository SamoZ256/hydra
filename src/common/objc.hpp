#pragma once

#include "common/type_aliases.hpp"

#if __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace hydra {

#if !__OBJC__
typedef void* id;
#endif

} // namespace hydra

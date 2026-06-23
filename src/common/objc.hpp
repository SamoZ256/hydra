#pragma once

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#endif

namespace hydra {

#ifndef __OBJC__
using id = void*;
#endif

} // namespace hydra

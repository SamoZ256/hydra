#ifndef BRIDGING_HEADER_H
#define BRIDGING_HEADER_H

#import <QuartzCore/QuartzCore.h>

#include "core/c_api.h"

void set_layer(void* ctx, CAMetalLayer* layer) {
    void* layer_bridged = (__bridge void*)layer;

    hydra_emulation_context_set_surface(ctx, layer_bridged);
}

#endif

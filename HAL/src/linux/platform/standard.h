#include "router_hal.h"

// for online experiment, don't change
#ifdef ROUTER_PC1

static const char *interfaces[N_IFACE_ON_BOARD] = {
    "pc1r1",
    "eth2",
    "eth3",
    "eth4",
};

#elif defined(ROUTER_R1)

static const char *interfaces[N_IFACE_ON_BOARD] = {
    "r1pc1",
    "r1r2",
    "eth3",
    "eth4",
};

#elif defined(ROUTER_R2)

static const char *interfaces[N_IFACE_ON_BOARD] = {
    "r2r1",
    "r2r3",
    "eth3",
    "eth4",
};

#elif defined(ROUTER_R3)

static const char *interfaces[N_IFACE_ON_BOARD] = {
    "r3r2",
    "r3pc2",
    "eth3",
    "eth4",
};

#elif defined(ROUTER_PC2)

static const char *interfaces[N_IFACE_ON_BOARD] = {
    "pc2r3",
    "eth2",
    "eth3",
    "eth4",
};

#else
// you can customize this
// configure this to match the output of `ip a`
static const char *interfaces[N_IFACE_ON_BOARD] = {
    "r2r1",
    "eth2",
    "eth3",
    "eth4",
};
#endif

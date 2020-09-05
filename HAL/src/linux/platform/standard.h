#include "router_hal.h"

// for online experiment, don't change
#ifdef ROUTER_R1
const char *interfaces[N_IFACE_ON_BOARD] = {
    "r1pc1",
    "r1r2",
    "eth3",
    "eth4",
};

#elif defined(ROUTER_R2)

const char *interfaces[N_IFACE_ON_BOARD] = {
    "r2r1",
    "r2r3",
    "eth3",
    "eth4",
};

#elif defined(ROUTER_R3)
const char *interfaces[N_IFACE_ON_BOARD] = {
    "r3r2",
    "r3pc2",
    "eth3",
    "eth4",
};

#else
// you can customize this
// configure this to match the output of `ip a`
const char *interfaces[N_IFACE_ON_BOARD] = {
    "eth1",
    "eth2",
    "eth3",
    "eth4",
};
#endif

#include "router_hal.h"
#include "router_hal_common.h"
#include <stdio.h>

#include <ifaddrs.h>
#include <map>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <pcap.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <time.h>
#include <utility>

const char *interfaces[N_IFACE_ON_BOARD] = {
    "en0",
    "en1",
    "en2",
    "en3",
};

extern "C" {
int HAL_Init(HAL_IN int debug, HAL_IN in6_addr if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) < 0) {
    if (debugEnabled) {
      fprintf(stderr, "HAL_Init: getifaddrs failed with %s\n", strerror(errno));
    }
    return HAL_ERR_UNKNOWN;
  }

  // ref:
  // https://stackoverflow.com/questions/10593736/mac-address-from-interface-on-os-x-c
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    int index;
    if ((index = if_nametoindex(interfaces[i])) == 0) {
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: get MAC addr failed for interface %s\n",
                interfaces[i]);
      }
      continue;
    }

    int mib[6];
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    mib[5] = index;

    size_t len;

    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: get MAC addr failed for interface %s\n",
                interfaces[i]);
      }
      continue;
    }
    char *buf;

    if ((buf = (char *)malloc(len)) == NULL) {
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: get MAC addr failed for interface %s\n",
                interfaces[i]);
      }
      continue;
    }

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: get MAC addr failed for interface %s\n",
                interfaces[i]);
      }
      continue;
    }

    struct if_msghdr *ifm = (struct if_msghdr *)buf;
    struct sockaddr_dl *sdl = (struct sockaddr_dl *)(ifm + 1);
    caddr_t mac = LLADDR(sdl);
    // found
    memcpy(&interface_mac[i], mac, sizeof(ether_addr));
    ndp_table[std::pair<in6_addr, int>(if_addrs[i], i)] = interface_mac[i];
    if (debugEnabled) {
      fprintf(stderr,
              "HAL_Init: MAC addr of interface %s is "
              "%s\n",
              interfaces[i], ether_ntoa(interface_mac[i]));
    }
  }

  char error_buffer[PCAP_ERRBUF_SIZE];
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    pcap_in_handles[i] =
        pcap_open_live(interfaces[i], BUFSIZ, 1, 1, error_buffer);
    if (pcap_in_handles[i]) {
      pcap_setnonblock(pcap_in_handles[i], 1, error_buffer);
      if (debugEnabled) {
        fprintf(stderr, "HAL_Init: pcap capture enabled for %s\n",
                interfaces[i]);
      }
    } else {
      if (debugEnabled) {
        fprintf(stderr,
                "HAL_Init: pcap capture disabled for %s, either the interface "
                "does not exist or permission is denied\n",
                interfaces[i]);
      }
    }
    pcap_out_handles[i] =
        pcap_open_live(interfaces[i], BUFSIZ, 1, 0, error_buffer);
  }

  memcpy(interface_addrs, if_addrs, sizeof(interface_addrs));

  inited = true;
  return 0;
}
}
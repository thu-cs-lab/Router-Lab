#include "router_hal.h"
#include "router_hal_common.h"
#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <map>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <utility>
#include <vector>

#include "platform/standard.h"

extern "C" {
int HAL_Init(HAL_IN int debug, HAL_IN in6_addr if_addrs[N_IFACE_ON_BOARD]) {
  if (inited) {
    return 0;
  }
  debugEnabled = debug;

  // find matching interfaces and get their MAC address
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) < 0) {
    if (debugEnabled) {
      fprintf(stderr, "HAL_Init: getifaddrs failed with %s\n", strerror(errno));
    }
    return HAL_ERR_UNKNOWN;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      if (ifa->ifa_addr->sa_family == AF_PACKET &&
          strcmp(ifa->ifa_name, interfaces[i]) == 0) {
        // found
        memcpy(&interface_mac[i],
               ((struct sockaddr_ll *)ifa->ifa_addr)->sll_addr,
               sizeof(ether_addr));
        ndp_table[std::pair<in6_addr, int>(if_addrs[i], i)] = interface_mac[i];
        if (debugEnabled) {
          fprintf(stderr, "HAL_Init: found MAC addr of interface %s\n",
                  interfaces[i]);
        }

        // disable ipv6 of this interface
        // echo 1 > /proc/sys/net/ipv6/conf/if_name/disable_ipv6
        char name_buffer[64];
        sprintf(name_buffer, "/proc/sys/net/ipv6/conf/%s/disable_ipv6",
                interfaces[i]);
        FILE *fp = fopen(name_buffer, "w");
        char ch = '1';
        if (fp) {
          fwrite(&ch, 1, 1, fp);
          fclose(fp);

          if (debugEnabled) {
            fprintf(stderr, "HAL_Init: disabled ipv6 of interface %s\n",
                    interfaces[i]);
          }
        } else {
          if (debugEnabled) {
            fprintf(stderr, "HAL_Init: failed to disable ipv6 of interface %s\n",
                    interfaces[i]);
          }
        }
        break;
      }
    }
  }
  freeifaddrs(ifaddr);

  // init pcap handles
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

  // generate link local addresses with eui64
  for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
    interface_link_local_addrs[i] = eui64(interface_mac[i]);
    if (debugEnabled) {
      fprintf(stderr,
              "HAL_Init: interface %d is configured with link local addr %s\n",
              i, inet6_ntoa(interface_link_local_addrs[i]));
    }
  }

  // debug print
  if (debugEnabled) {
    for (int i = 0; i < N_IFACE_ON_BOARD; i++) {
      fprintf(stderr,
              "HAL_Init: interface %d is configured with IPv6 addr %s\n", i,
              inet6_ntoa(interface_addrs[i]));
    }
  }

  inited = true;
  return 0;
}
}

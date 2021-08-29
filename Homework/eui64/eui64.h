#ifndef __EUI64_H__
#define __EUI64_H__

#include "common.h"

/**
 * @brief 转换 MAC 地址为 IPv6 地址
 * @param mac MAC 地址
 * @return IPv6 地址
 */
in6_addr eui64(const ether_addr mac);

#endif

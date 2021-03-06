/*
 * This file is a part of plotnetcfg, a tool to visualize network config.
 * Copyright (C) 2016 Red Hat, Inc. -- Jiri Benc <jbenc@redhat.com>
 *                                     Ondrej Hlavaty <ohlavaty@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _ADDR_H
#define _ADDR_H

#include <sys/socket.h>
#include <linux/rtnetlink.h>

struct addr {
	int family;
	int prefixlen;
	char *formatted;
	void *raw;
};

int addr_init(struct addr *addr, int ai_family, int prefixlen, void *raw);
int addr_init_netlink(struct addr *dest, const struct ifaddrmsg *ifa,
		      const struct rtattr *rta);

/* dest must point to at least 16 bytes long buffer */
int addr_parse_raw(void *dest, const char *str);

void addr_destruct(struct addr *addr);

#endif

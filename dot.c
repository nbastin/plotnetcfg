/*
 * This file is a part of plotnetcfg, a tool to visualize network config.
 * Copyright (C) 2014 Red Hat, Inc. -- Jiri Benc <jbenc@redhat.com>
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

#include <stdio.h>
#include <sys/socket.h>
#include "handler.h"
#include "if.h"
#include "label.h"
#include "netns.h"
#include "utils.h"
#include "dot.h"

static void output_label(struct label *list)
{
	struct label *ptr;

	for (ptr = list; ptr; ptr = ptr->next)
		printf("\\n%s", ptr->text);
}

static void output_addresses(struct if_addr_entry *list)
{
	struct if_addr_entry *ptr;

	for (ptr = list; ptr; ptr = ptr->next) {
		printf("\\n%s", ptr->addr.formatted);
		if (ptr->peer.formatted)
			printf(" peer %s", ptr->peer.formatted);
	}
}

static void output_ifaces_pass1(struct if_entry *list)
{
	struct if_entry *ptr;

	for (ptr = list; ptr; ptr = ptr->next) {
		printf("%s [label=\"%s", ifid(ptr), ptr->if_name);
		if (ptr->driver)
			printf(" (%s)", ptr->driver);
		output_label(ptr->label);
		output_addresses(ptr->addr);
		printf("\"");
		if (ptr->flags & IF_INTERNAL)
			printf(",style=dotted");
		else if (!(ptr->flags & IF_UP))
			printf(",style=filled,fillcolor=\"grey\"");
		else if (!(ptr->flags & IF_HAS_LINK))
			printf(",style=filled,fillcolor=\"pink\"");
		else
			printf(",style=filled,fillcolor=\"darkolivegreen1\"");
		if (ptr->warnings)
			printf(",color=\"red\"");
		printf("]\n");
	}
}

static void output_ifaces_pass2(struct if_entry *list)
{
	struct if_entry *ptr;

	for (ptr = list; ptr; ptr = ptr->next) {
		if (ptr->master) {
			printf("%s -> ", ifid(ptr));
			printf("%s", ifid(ptr->master));
			if (ptr->edge_label && !ptr->link)
				printf(" [label=\"%s\"]", ptr->edge_label);
			printf("\n");
		}
		if (ptr->link) {
			printf("%s -> ", ifid(ptr->link));
			printf("%s", ifid(ptr));
			if (ptr->edge_label)
				printf(" [label=\"%s\"]", ptr->edge_label);
			printf("\n");
		}
		if (ptr->peer &&
		    (((unsigned long)ptr > (unsigned long)ptr->peer) ||
		     !ptr->peer->peer)) {
			printf("%s -> ", ifid(ptr));
			printf("%s [dir=none,style=dotted]\n", ifid(ptr->peer));
		}
	}
}

static void output_warnings(struct netns_entry *root)
{
	struct netns_entry *ns;
	int was_label = 0;

	for (ns = root; ns; ns = ns->next) {
		if (ns->warnings) {
			if (!was_label)
				printf("label=\"");
			was_label = 1;
			output_label(ns->warnings);
		}
	}
	if (was_label) {
		printf("\"\n");
		printf("fontcolor=\"red\"\n");
	}
}

void dot_output(struct netns_entry *root)
{
	struct netns_entry *ns;

	printf("digraph {\nnode [shape=box]\n");
	for (ns = root; ns; ns = ns->next) {
		if (ns->name) {
			printf("subgraph \"cluster_%s\" {\n", ns->name);
			printf("label=\"%s\"\n", ns->name);
			printf("fontcolor=\"black\"\n");
		}
		output_ifaces_pass1(ns->ifaces);
		if (ns->name)
			printf("}\n");
	}
	for (ns = root; ns; ns = ns->next) {
		output_ifaces_pass2(ns->ifaces);
	}
	output_warnings(root);
	printf("}\n");
}

// SPDX-License-Identifier: GPL-2.0

/*
 * (C) Copyright 2023 Linaro Ltd. <maxim.uvarov@linaro.org>
 */

#include <common.h>
#include <command.h>
#include <console.h>

#include <lwip/dns.h>
#include <lwip/ip_addr.h>

#include <net/ulwip.h>

static void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
	char *varname = (char *)callback_arg;
	char *ipstr = ip4addr_ntoa(ipaddr);

	if (varname)
		env_set(varname, ipstr);
	log_info("resolved %s to %s\n",  name, ipstr);
	ulwip_exit(0);
}

int ulwip_dns(char *name, char *varname)
{
	int err;
	ip_addr_t ipaddr; /* not used */
	ip_addr_t dns1;
	ip_addr_t dns2;
	char *dnsenv = env_get("dnsip");
	char *dns2env = env_get("dnsip2");

	if (!dnsenv && !dns2env) {
		log_err("nameserver is not set with dnsip and dnsip2 vars\n");
		return -ENOENT;
	}

	if (!dnsenv)
		log_warning("dnsip var is not set\n");
	if (!dns2env)
		log_warning("dnsip2 var is not set\n");

	dns_init();

	if (ipaddr_aton(dnsenv, &dns1))
		dns_setserver(0, &dns1);

	if (dns2env && ipaddr_aton(dns2env, &dns2))
		dns_setserver(1, &dns2);

	err = dns_gethostbyname(name, &ipaddr, dns_found_cb, varname);
	if (err == ERR_OK)
		dns_found_cb(name, &ipaddr, varname);

	/* convert lwIP ERR_INPROGRESS to U-Boot -EINPROGRESS */
	if (err == ERR_INPROGRESS)
		err = -EINPROGRESS;

	return err;
}

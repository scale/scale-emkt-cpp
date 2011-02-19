/* Copyright (c) 2007 Girish Venkatachalam
 *
 * Permission to use, copy, modify, and distribute
 * this software for any
 * purpose with or without fee is hereby granted,
 * provided that the above copyright notice
 * and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ldns/ldns.h>

#include "MicroDNS.h" 

vector<struct mx>
MicroDNS::MX(std::string & str_domain)
{
	ldns_resolver *res;
	ldns_rdf * domain;
	ldns_pkt *p;
	ldns_rr_list * mx;
	ldns_status s;

	p = NULL;
	mx = NULL;
	domain = NULL;
	res = NULL;


	domain = ldns_dname_new_frm_str(str_domain.c_str());
	if (!domain) {
		return NULL;
	}

	/* create a new resolver from /etc/resolv.conf */
	s = ldns_resolver_new_frm_file(&res, NULL);

	if (s != LDNS_STATUS_OK) {
		return NULL;
	}

	/* use the resolver to send a query for the mx
	 * records of the domain given on the command line
	 */
	p = ldns_resolver_query(res,
							domain,
							LDNS_RR_TYPE_MX,
							LDNS_RR_CLASS_IN,
							LDNS_RD);

	ldns_rdf_deep_free(domain);

	if (!p) {
		return NULL;
	} else {
		/* retrieve the MX records from the answer section of that
		 * packet
		 */
		mx = ldns_pkt_rr_list_by_type(p,
									  LDNS_RR_TYPE_MX,
									  LDNS_SECTION_ANSWER);
		if (!mx) {
			fprintf(stderr,
					" *** invalid answer name %s after MX query for %s\n",
					argv[1], argv[1]);
			ldns_pkt_free(p);
			ldns_resolver_deep_free(res);
			exit( EXIT_FAILURE);
		} else {
			ldns_rr_list_sort(mx);
			ldns_rr_list_print(stdout, mx);
			ldns_rr_list_deep_free(mx);
		}
	}
	ldns_pkt_free(p);
	ldns_resolver_deep_free(res);
	return 0;

}

std::string
MicroDNS::A(std::string & str_domain)
{
	ldns_resolver *res;
	ldns_rdf * domain;
	ldns_pkt *p;
	ldns_rr_list * mx;
	ldns_status s;

	p = NULL;
	mx = NULL;
	domain = NULL;
	res = NULL;


	domain = ldns_dname_new_frm_str(str_domain.c_str());
	if (!domain) {
		return NULL;
	}

	/* create a new resolver from /etc/resolv.conf */
	s = ldns_resolver_new_frm_file(&res, NULL);

	if (s != LDNS_STATUS_OK) {
		return "127.0.0.1";
	}

	/* use the resolver to send a query for the mx
	 * records of the domain given on the command line
	 */
	p = ldns_resolver_query(res,
							domain,
							LDNS_RR_TYPE_A,
							LDNS_RR_CLASS_IN,
							LDNS_RD);

	ldns_rdf_deep_free(domain);

	if (!p) {
		return "127.0.0.1";
	} else {
		/* retrieve the MX records from the answer section of that
		 * packet
		 */
		mx = ldns_pkt_rr_list_by_type(p,
									  LDNS_RR_TYPE_A,
									  LDNS_SECTION_ANSWER);
		if (!mx) {
			fprintf(stderr,
					" *** invalid answer name %s after MX query for %s\n",
					argv[1], argv[1]);
			ldns_pkt_free(p);
			ldns_resolver_deep_free(res);
			exit( EXIT_FAILURE);
		} else {
			ldns_rr_list_sort(mx);
			ldns_rr_list_print(stdout, mx);
			ldns_rr_list_deep_free(mx);
		}
	}
	ldns_pkt_free(p);
	ldns_resolver_deep_free(res);
	return 0;
}


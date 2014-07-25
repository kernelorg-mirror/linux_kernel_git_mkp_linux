/*
 * t10_pi.c - Functions for generating and verifying T10 Protection
 *	      Information.
 *
 * Copyright (C) 2007, 2008, 2014 Oracle Corporation
 * Written by: Martin K. Petersen <martin.petersen@oracle.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
 */

#include <linux/t10-pi.h>
#include <linux/blkdev.h>
#include <linux/crc-t10dif.h>
#include <net/checksum.h>

typedef __u16 (csum_fn) (void *, unsigned int);

static __u16 t10_pi_crc_fn(void *data, unsigned int len)
{
	return cpu_to_be16(crc_t10dif(data, len));
}

static __u16 t10_pi_ip_fn(void *data, unsigned int len)
{
	return ip_compute_csum(data, len);
}

/*
 * Type 1 and Type 2 protection use the same format: 16 bit guard tag,
 * 16 bit app tag, 32 bit reference tag. Type 3 does not define the ref
 * tag.
 */
static int t10_pi_generate(struct blk_integrity_iter *iter, csum_fn *fn,
			   unsigned int type)
{
	unsigned int i;

	for (i = 0 ; i < iter->data_size ; i += iter->interval) {
		struct t10_pi_tuple *pi = iter->prot_buf;

		pi->guard_tag = fn(iter->data_buf, iter->interval);
		pi->app_tag = 0;

		if (type == 1)
			pi->ref_tag = cpu_to_be32(iter->seed & 0xffffffff);
		else
			pi->ref_tag = 0;

		iter->data_buf += iter->interval;
		iter->prot_buf += sizeof(struct t10_pi_tuple);
		iter->seed++;
	}

	return 0;
}

static int t10_pi_verify(struct blk_integrity_iter *iter, csum_fn *fn,
				unsigned int type)
{
	unsigned int i;

	for (i = 0 ; i < iter->data_size ; i += iter->interval) {
		struct t10_pi_tuple *pi = iter->prot_buf;
		__u16 csum;

		switch (type) {
		case 1:
		case 2:
			if (pi->app_tag == 0xffff)
				goto next;

			if (be32_to_cpu(pi->ref_tag) !=
			    (iter->seed & 0xffffffff)) {
				pr_err("%s: ref tag error at location %lu " \
				       "(rcvd %u)\n", iter->disk_name,
				       iter->seed, be32_to_cpu(pi->ref_tag));
				return -EILSEQ;
			}
			break;
		case 3:
			if (pi->app_tag == 0xffff && pi->ref_tag == 0xffffffff)
				goto next;
			break;
		}

		csum = fn(iter->data_buf, iter->interval);

		if (pi->guard_tag != csum) {
			pr_err("%s: guard tag error at location %lu " \
			       "(rcvd %04x, data %04x)\n", iter->disk_name,
			       (unsigned long)iter->seed,
			       be16_to_cpu(pi->guard_tag), be16_to_cpu(csum));
			return -EILSEQ;
		}

next:
		iter->data_buf += iter->interval;
		iter->prot_buf += sizeof(struct t10_pi_tuple);
		iter->seed++;
	}

	return 0;
}

int t10_pi_type1_generate_crc(struct blk_integrity_iter *iter)
{
	return t10_pi_generate(iter, t10_pi_crc_fn, 1);
}
EXPORT_SYMBOL(t10_pi_type1_generate_crc);

int t10_pi_type1_generate_ip(struct blk_integrity_iter *iter)
{
	return t10_pi_generate(iter, t10_pi_ip_fn, 1);
}
EXPORT_SYMBOL(t10_pi_type1_generate_ip);

int t10_pi_type1_verify_crc(struct blk_integrity_iter *iter)
{
	return t10_pi_verify(iter, t10_pi_crc_fn, 1);
}
EXPORT_SYMBOL(t10_pi_type1_verify_crc);

int t10_pi_type1_verify_ip(struct blk_integrity_iter *iter)
{
	return t10_pi_verify(iter, t10_pi_ip_fn, 1);
}
EXPORT_SYMBOL(t10_pi_type1_verify_ip);

int t10_pi_type3_generate_crc(struct blk_integrity_iter *iter)
{
	return t10_pi_generate(iter, t10_pi_crc_fn, 3);
}
EXPORT_SYMBOL(t10_pi_type3_generate_crc);

int t10_pi_type3_generate_ip(struct blk_integrity_iter *iter)
{
	return t10_pi_generate(iter, t10_pi_ip_fn, 3);
}
EXPORT_SYMBOL(t10_pi_type3_generate_ip);

int t10_pi_type3_verify_crc(struct blk_integrity_iter *iter)
{
	return t10_pi_verify(iter, t10_pi_crc_fn, 3);
}
EXPORT_SYMBOL(t10_pi_type3_verify_crc);

int t10_pi_type3_verify_ip(struct blk_integrity_iter *iter)
{
	return t10_pi_verify(iter, t10_pi_ip_fn, 3);
}
EXPORT_SYMBOL(t10_pi_type3_verify_ip);

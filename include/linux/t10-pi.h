#ifndef _LINUX_T10_PI_H
#define _LINUX_T10_PI_H

#include <linux/types.h>
#include <linux/blkdev.h>

/*
 * T10 Protection Information tuple.
 */
struct t10_pi_tuple {
	__be16 guard_tag;	/* Checksum */
	__be16 app_tag;		/* Opaque storage */
	__be32 ref_tag;		/* Target LBA or indirect LBA */
};

extern int t10_pi_type1_generate_crc(struct blk_integrity_iter *);
extern int t10_pi_type1_verify_crc(struct blk_integrity_iter *);

extern int t10_pi_type1_generate_ip(struct blk_integrity_iter *);
extern int t10_pi_type1_verify_ip(struct blk_integrity_iter *);

extern int t10_pi_type3_generate_crc(struct blk_integrity_iter *);
extern int t10_pi_type3_verify_crc(struct blk_integrity_iter *);

extern int t10_pi_type3_generate_ip(struct blk_integrity_iter *);
extern int t10_pi_type3_verify_ip(struct blk_integrity_iter *);

struct blk_integrity t10_pi_type1_crc = {
	.name			= "T10-DIF-TYPE1-CRC",
	.generate_fn		= t10_pi_type1_generate_crc,
	.verify_fn		= t10_pi_type1_verify_crc,
	.tuple_size		= sizeof(struct t10_pi_tuple),
	.tag_size		= 0,
};

struct blk_integrity t10_pi_type1_ip = {
	.name			= "T10-DIF-TYPE1-IP",
	.generate_fn		= t10_pi_type1_generate_ip,
	.verify_fn		= t10_pi_type1_verify_ip,
	.tuple_size		= sizeof(struct t10_pi_tuple),
	.tag_size		= 0,
};

struct blk_integrity t10_pi_type3_crc = {
	.name			= "T10-DIF-TYPE3-CRC",
	.generate_fn		= t10_pi_type3_generate_crc,
	.verify_fn		= t10_pi_type3_verify_crc,
	.tuple_size		= sizeof(struct t10_pi_tuple),
	.tag_size		= 0,
};

struct blk_integrity t10_pi_type3_ip = {
	.name			= "T10-DIF-TYPE3-IP",
	.generate_fn		= t10_pi_type3_generate_ip,
	.verify_fn		= t10_pi_type3_verify_ip,
	.tuple_size		= sizeof(struct t10_pi_tuple),
	.tag_size		= 0,
};

#endif

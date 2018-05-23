/*
 * NVM Express device driver verbose errors
 * Copyright (c) 2018, Oracle and/or its affiliates
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/blkdev.h>
#include "nvme.h"

struct nvme_string_table {
	unsigned int code;
	const unsigned char *string;
};

static const struct nvme_string_table nvme_ops[] = {
	{ REQ_OP_READ			, "Read" },
	{ REQ_OP_WRITE			, "Write" },
	{ REQ_OP_FLUSH			, "Flush" },
	{ REQ_OP_DISCARD		, "Deallocate (DSM)" },
	{ REQ_OP_WRITE_ZEROES		, "Write Zeroes" },
};
#define NVME_OPS_SIZE ARRAY_SIZE(nvme_ops)

static const struct nvme_string_table nvme_errors[] = {
	{ NVME_SC_SUCCESS		, "Success" },
	{ NVME_SC_INVALID_OPCODE	, "Invalid Command Opcode" },
	{ NVME_SC_INVALID_FIELD		, "Invalid Field in Command" },
	{ NVME_SC_CMDID_CONFLICT	, "Command ID Conflict" },
	{ NVME_SC_DATA_XFER_ERROR	, "Data Transfer Error" },
	{ NVME_SC_POWER_LOSS		, "Commands Aborted due to Power Loss Notification" },
	{ NVME_SC_INTERNAL		, "Internal Error" },
	{ NVME_SC_ABORT_REQ		, "Command Abort Requested" },
	{ NVME_SC_ABORT_QUEUE		, "Command Aborted due to SQ Deletion" },
	{ NVME_SC_FUSED_FAIL		, "Command Aborted due to Failed Fused Command" },
	{ NVME_SC_FUSED_MISSING		, "Command Aborted due to Missing Fused Command" },
	{ NVME_SC_INVALID_NS		, "Invalid Namespace or Format" },
	{ NVME_SC_CMD_SEQ_ERROR		, "Command Sequence Error" },
	{ NVME_SC_SGL_INVALID_LAST	, "Invalid SGL Segment Descriptor" },
	{ NVME_SC_SGL_INVALID_COUNT	, "Invalid Number of SGL Descriptors" },
	{ NVME_SC_SGL_INVALID_DATA	, "Data SGL Length Invalid" },
	{ NVME_SC_SGL_INVALID_METADATA	, "Metadata SGL Length Invalid" },
	{ NVME_SC_SGL_INVALID_TYPE	, "SGL Descriptor Type Invalid" },
	{ NVME_SC_INVALID_CMB		, "Invalid Use of Controller Memory Buffer" },
	{ NVME_SC_INVALID_PRP_OFFSET	, "PRP Offset Invalid" },
	{ NVME_SC_ATOMIC_WRITE_EXCEEDED	, "Atomic Write Unit Exceeded" },
	{ NVME_SC_OPERATION_DENIED	, "Operation Denied" },
	{ NVME_SC_SGL_INVALID_OFFSET	, "SGL Offset Invalid" },
	{ NVME_SC_SGL_INVALID_SUBTYPE	, "Reserved" },
	{ NVME_SC_HOST_ID_INCONS_FORMAT	, "Host Identifier Inconsistent Format" },
	{ NVME_SC_KEEP_ALIVE_EXPIRED	, "Keep Alive Timeout Expired" },
	{ NVME_SC_KEEP_ALIVE_INVALID	, "Keep Alive Timeout Invalid" },
	{ NVME_SC_ABORTED_PREEMPT	, "Command Aborted due to Preempt and Abort" },
	{ NVME_SC_SANITIZE_FAILED	, "Sanitize Failed" },
	{ NVME_SC_SANITIZE_IN_PROGRESS	, "Sanitize In Progress" },
	{ NVME_SC_SGL_GRANULARITY_INV	, "SGL Data Block Granularity Invalid" },
	{ NVME_SC_CMD_UNSUP_FOR_CMB	, "Command Not Supported for Queue in CMB" },
	{ NVME_SC_LBA_RANGE		, "LBA Out of Range" },
	{ NVME_SC_CAP_EXCEEDED		, "Capacity Exceeded" },
	{ NVME_SC_NS_NOT_READY		, "Namespace Not Ready" },
	{ NVME_SC_RESERVATION_CONFLICT	, "Reservation Conflict" },
	{ NVME_SC_FORMAT_IN_PROGRESS	, "Format In Progress" },
	{ NVME_SC_CQ_INVALID		, "Completion Queue Invalid" },
	{ NVME_SC_QID_INVALID		, "Invalid Queue Identifier" },
	{ NVME_SC_QUEUE_SIZE		, "Invalid Queue Size" },
	{ NVME_SC_ABORT_LIMIT		, "Abort Command Limit Exceeded" },
	{ NVME_SC_ABORT_MISSING		, "Reserved" },
	{ NVME_SC_ASYNC_LIMIT		, "Asynchronous Event Request Limit Exceeded" },
	{ NVME_SC_FIRMWARE_SLOT		, "Invalid Firmware Slot" },
	{ NVME_SC_FIRMWARE_IMAGE	, "Invalid Firmware Image" },
	{ NVME_SC_INVALID_VECTOR	, "Invalid Interrupt Vector" },
	{ NVME_SC_INVALID_LOG_PAGE	, "Invalid Log Page" },
	{ NVME_SC_INVALID_FORMAT	, "Invalid Format" },
	{ NVME_SC_FW_NEEDS_CONV_RESET	, "Firmware Activation Requires Conventional Reset" },
	{ NVME_SC_INVALID_QUEUE		, "Invalid Queue Deletion" },
	{ NVME_SC_FEATURE_NOT_SAVEABLE	, "Feature Identifier Not Saveable" },
	{ NVME_SC_FEATURE_NOT_CHANGEABLE, "Feature Not Changeable" },
	{ NVME_SC_FEATURE_NOT_PER_NS	, "Feature Not Namespace Specific" },
	{ NVME_SC_FW_NEEDS_SUBSYS_RESET	, "Firmware Activation Requires NVM Subsystem Reset" },
	{ NVME_SC_FW_NEEDS_RESET	, "Firmware Activation Requires Reset" },
	{ NVME_SC_FW_NEEDS_MAX_TIME	, "Firmware Activation Requires Maximum Time Violation" },
	{ NVME_SC_FW_ACIVATE_PROHIBITED	, "Firmware Activation Prohibited" },
	{ NVME_SC_OVERLAPPING_RANGE	, "Overlapping Range" },
	{ NVME_SC_NS_INSUFFICENT_CAP	, "Namespace Insufficient Capacity" },
	{ NVME_SC_NS_ID_UNAVAILABLE	, "Namespace Identifier Unavailable" },
	{ NVME_SC_NS_ALREADY_ATTACHED	, "Namespace Already Attached" },
	{ NVME_SC_NS_IS_PRIVATE		, "Namespace Is Private" },
	{ NVME_SC_NS_NOT_ATTACHED	, "Namespace Not Attached" },
	{ NVME_SC_THIN_PROV_NOT_SUPP	, "Thin Provisioning Not Supported" },
	{ NVME_SC_CTRL_LIST_INVALID	, "Controller List Invalid" },
	{ NVME_SC_SELF_TEST_IN_PROGRESS	, "Device Self-test In Progress" },
	{ NVME_SC_BOOT_PARTITION_WRITE	, "Boot Partition Write Prohibited" },
	{ NVME_SC_INVALID_CONTROLLER_ID	, "Invalid Controller Identifier" },
	{ NVME_SC_INVALID_SECONDARY_CS	, "Invalid Secondary Controller State" },
	{ NVME_SC_INVALID_CONTROLLER_RES, "Invalid Number of Controller Resources" },
	{ NVME_SC_INVALID_RESOURCE_ID	, "Invalid Resource Identifier" },
	{ NVME_SC_BAD_ATTRIBUTES	, "Conflicting Attributes" },
	{ NVME_SC_INVALID_PI		, "Invalid Protection Information" },
	{ NVME_SC_READ_ONLY		, "Attempted Write to Read Only Range" },
	{ NVME_SC_ONCS_NOT_SUPPORTED	, "ONCS Not Supported" },
	{ NVME_SC_WRITE_FAULT		, "Write Fault" },
	{ NVME_SC_READ_ERROR		, "Unrecovered Read Error" },
	{ NVME_SC_GUARD_CHECK		, "End-to-end Guard Check Error" },
	{ NVME_SC_APPTAG_CHECK		, "End-to-end Application Tag Check Error" },
	{ NVME_SC_REFTAG_CHECK		, "End-to-end Reference Tag Check Error" },
	{ NVME_SC_COMPARE_FAILED	, "Compare Failure" },
	{ NVME_SC_ACCESS_DENIED		, "Access Denied" },
	{ NVME_SC_UNWRITTEN_BLOCK	, "Deallocated or Unwritten Logical Block" },
};
#define NVME_ERRORS_SIZE ARRAY_SIZE(nvme_errors)

void nvme_error_log(struct request *req)
{
	struct nvme_ns *ns = req->q->queuedata;
	struct nvme_request *nr = nvme_req(req);
	const struct nvme_string_table *entry;
	const unsigned char *op_str  = "Unknown";
	const unsigned char *err_str = "Unknown";
	unsigned int i;

	if (!ns)
		return;

	for (i = 0, entry = nvme_ops ; i < NVME_OPS_SIZE ; i++)
		if (entry[i].code == (req->cmd_flags & REQ_OP_MASK))
			op_str = entry[i].string;

	for (i = 0, entry = nvme_errors ; i < NVME_ERRORS_SIZE ; i++)
		if (entry[i].code == (nr->status & 0x7ff))
			err_str = entry[i].string;

	pr_err("%s: %s @ LBA %llu, %llu blocks, %s (sct 0x%x / sc 0x%x) %s%s\n",
	       req->rq_disk ? req->rq_disk->disk_name : "?",
	       op_str,
	       (unsigned long long)nvme_block_nr(ns, blk_rq_pos(req)),
	       (unsigned long long)blk_rq_bytes(req) >> ns->lba_shift,
	       err_str,
	       nr->status >> 8 & 7, /* Status Code Type */
	       nr->status & 0xf,    /* Status Code */
	       nr->status & NVME_SC_MORE ? "MORE " : "",
	       nr->status & NVME_SC_DNR  ? "DNR "  : "");
}
EXPORT_SYMBOL(nvme_error_log);


/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _AKD_H
#define _AKD_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#ifndef __u32
typedef uint32_t __u32;
#endif
#endif

#define AKD_DEVICE_NAME		"akd"
#define AKD_DEVICE_PATH		"/dev/" AKD_DEVICE_NAME
#define AKD_BUF_SIZE		4096
#define AKD_VERSION		0x00010000	/* 1.0.0 */

#define AKD_IOC_MAGIC		'A'

#define AKD_IOC_GET_VERSION	_IOR(AKD_IOC_MAGIC, 0x01, __u32)
#define AKD_IOC_GET_STATUS	_IOR(AKD_IOC_MAGIC, 0x02, __u32)
#define AKD_IOC_SET_STATUS	_IOW(AKD_IOC_MAGIC, 0x03, __u32)
#define AKD_IOC_CLEAR		_IO(AKD_IOC_MAGIC, 0x04)
#define AKD_IOC_GET_LEN		_IOR(AKD_IOC_MAGIC, 0x05, __u32)

enum akd_status {
	AKD_STATUS_IDLE	= 0,
	AKD_STATUS_BUSY	= 1,
	AKD_STATUS_ERROR	= 2,
};

#endif /* _AKD_H */

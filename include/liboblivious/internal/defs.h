#ifndef __LIBOBLIVIOUS_INTERNAL_DEFS_H
#define __LIBOBLIVIOUS_INTERNAL_DEFS_H

#define UNUSED __attribute__((unused))

#ifdef __cplusplus
#define LIBOBLIVIOUS_EXTERNC_BEGIN extern "C" {
#define LIBOBLIVIOUS_EXTERNC_END }
#define LIBOBLIVIOUS_EXTERNC extern "C"
#else
#define LIBOBLIVIOUS_EXTERNC_BEGIN
#define LIBOBLIVIOUS_EXTERNC_END
#define LIBOBLIVIOUS_EXTERNC
#endif

#endif

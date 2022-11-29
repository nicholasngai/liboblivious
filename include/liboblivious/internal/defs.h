#ifndef LIBOBLIVIOUS_INTERNAL_DEFS_H
#define LIBOBLIVIOUS_INTERNAL_DEFS_H

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

#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#define LIBOBLIVIOUS_CMOV
#endif

#endif

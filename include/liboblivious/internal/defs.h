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

#ifdef LIBOBLIVIOUS_CMOV
#if !defined(__GNUC__) || (!defined(__x86_64__) && !defined(__i386__))
#error "CMOV not available on this platform!"
#endif
#endif

#endif

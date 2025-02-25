/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.2.0.17) *
 *******************************************************************/
#ifndef __wrappedlibzTYPES_H_
#define __wrappedlibzTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFppi_t)(void*, void*, int32_t);
typedef int32_t (*iFpipi_t)(void*, int32_t, void*, int32_t);
typedef int32_t (*iFpippi_t)(void*, int32_t, void*, void*, int32_t);
typedef int32_t (*iFpiiiiipi_t)(void*, int32_t, int32_t, int32_t, int32_t, int32_t, void*, int32_t);

#define SUPER() ADDED_FUNCTIONS() \
	GO(deflateEnd, iFp_t) \
	GO(inflateEnd, iFp_t) \
	GO(inflate, iFpi_t) \
	GO(inflateInit_, iFppi_t) \
	GO(deflateInit_, iFpipi_t) \
	GO(inflateInit2_, iFpipi_t) \
	GO(inflateBackInit_, iFpippi_t) \
	GO(deflateInit2_, iFpiiiiipi_t)

#endif // __wrappedlibzTYPES_H_

/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.2.0.17) *
 *******************************************************************/
#ifndef __wrappedlibsndfileTYPES_H_
#define __wrappedlibsndfileTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef int32_t (*iFp_t)(void*);
typedef void* (*pFpipp_t)(void*, int32_t, void*, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(sf_close, iFp_t) \
	GO(sf_open_virtual, pFpipp_t)

#endif // __wrappedlibsndfileTYPES_H_

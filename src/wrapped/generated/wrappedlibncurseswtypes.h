/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.2.0.17) *
 *******************************************************************/
#ifndef __wrappedlibncurseswTYPES_H_
#define __wrappedlibncurseswTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef void* (*pFv_t)(void);
typedef int32_t (*iFpV_t)(void*, ...);
typedef int32_t (*iFppV_t)(void*, void*, ...);
typedef int32_t (*iFppA_t)(void*, void*, va_list);
typedef int32_t (*iFiipV_t)(int32_t, int32_t, void*, ...);
typedef int32_t (*iFpiipV_t)(void*, int32_t, int32_t, void*, ...);

#define SUPER() ADDED_FUNCTIONS() \
	GO(initscr, pFv_t) \
	GO(printw, iFpV_t) \
	GO(wprintw, iFppV_t) \
	GO(vwprintw, iFppA_t) \
	GO(mvprintw, iFiipV_t) \
	GO(mvwprintw, iFpiipV_t)

#endif // __wrappedlibncurseswTYPES_H_

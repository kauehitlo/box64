#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <dlfcn.h>

#include "box64context.h"
#include "debug.h"
#include "callback.h"
#include "librarian.h"
#include "elfs/elfloader_private.h"

/*
    This file here is for handling overriding of malloc functions

 Libraries like tcmalloc overrides all malloc/free/new/delete function and implement a custom version.
 Problem is, box64 is already loaded in memoery, and already using system libc malloc for many of it's things
 before those lib are even loaded in memory.
 Also, those override function can be define in the main executable, or in a lib loaded directly by the exectable
 or even in a lib loaded later using dlsym.

 The 2 different strategies to handle this can be defined as "Embrace" and "Exterminate" (as it cannot simply be ignored, 
 or you end up with mixing free/realloc from one lib and malloc/free from libc)

 In the "Embrace" strategy, the overriden malloc function are taken into account as soon as possible, and are used for all loaded lib, including native
 In the "Exterminate" strategy, the overriden malloc function are erased, and replaced with libc malloc as soon as they are defined.

 The advantage of "Embrace" is that the system will run the function it supposed to be using, and potential side-effect and staticaly linked functions 
 will run as intended.
 The downside of "Embrace" is that is makes it impossible to load a library with dlsym that override malloc function, especialy 
 if it loads natively wrapped function

 The advantage of "Exterminate" is that you wont be emulating basic malloc and friend, wich are used extensively in every program. Also, loading lib 
 with dlopen will not a be a problem.
 The downside of "Exterminate" is that side effect are less well controled. Staticaly linked stuff and anonymous symbols might put this strategy in trouble.

*/


#define SUPER()                 \
GO(malloc, pFL);                \
GO(free, vFp);                  \
GO(calloc, pFLL);               \
GO(realloc, pFpL);              \
GO(aligned_alloc, pFLL);        \
GO(memalign, pFLL);             \
GO(posix_memalign, iFpLL);      \
GO(pvalloc, pFL);               \
GO(valloc, pFL);                \
GO(cfree, vFp);                 \
GO(malloc_usable_size, LFp) ;   \

typedef void* (*pFL_t)  (size_t);
typedef void* (*pFLp_t) (size_t, void* p);
typedef void  (*vFp_t)  (void*);
typedef void* (*pFp_t)  (void*);
typedef size_t(*LFp_t)  (void*);
typedef void  (*vFpp_t) (void*, void*);
typedef void  (*vFpL_t) (void*, size_t);
typedef void* (*pFLL_t) (size_t, size_t);
typedef void* (*pFLLp_t)(size_t, size_t, void* p);
typedef void  (*vFpLp_t)(void*, size_t, void*);
typedef void  (*vFpLL_t)(void*, size_t, size_t);

#ifdef ANDROID
void*(*__libc_malloc)(size_t) = NULL;
void*(*__libc_realloc)(size_t, void*) = NULL;
void*(*__libc_calloc)(size_t, size_t) = NULL;
void (*__libc_free*)(void*) = NULL;
void*(*__libc_memalign)(size_t, size_t) = NULL;
#endif
size_t(*box_malloc_usable_size)(void*) = NULL;
#define GO(A, B)
SUPER()
#undef GO

int GetTID();

int go_hooking_malloc = 0;

char* box_strdup(const char* s) {
    char* ret = box_calloc(1, strlen(s)+1);
    memcpy(ret, s, strlen(s));
    return ret;
}

char* box_realpath(const char* path, char* ret)
{
    if(ret)
        return realpath(path, ret);
#ifdef PATH_MAX
    size_t path_max = PATH_MAX;
#else
    size_t path_max = pathconf(path, _PC_PATH_MAX);
    if (path_max <= 0)
    path_max = 4096;
#endif
    char tmp[path_max];
    char* p = realpath(path, tmp);
    if(!p)
        return NULL;
    return box_strdup(tmp);
}

// emulated lib/program hooked memory functions
#define GO(A, B) uintptr_t hooked_##A = 0
SUPER()
#undef GO
#define GO(A, B) elfheader_t* elf_##A = NULL
SUPER()
#undef GO

static size_t pot(size_t l) {
    size_t ret = 0;
    while (l>(1<<ret))  ++ret;
    return 1<<ret;
}

// redefining all libc memory allocation routines
EXPORT void* malloc(size_t l)
{
    if(hooked_malloc && elf_malloc && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated malloc(%zd)\n", GetTID(), l);
        void* ret = (void*)RunSafeFunction(my_context, hooked_malloc, 1, l);
        return ret;
    }
    void* ret = box_calloc(1, l);
    if(my_context) printf_log(LOG_DEBUG, "native malloc(%zu) => %p\n", l, ret);
    return ret;
}

EXPORT void free(void* p)
{
    if(hooked_free && elf_free && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated free(%x)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked_free, 1, p);
    } else
        box_free(p);
}

EXPORT void* calloc(size_t n, size_t s)
{
    if(hooked_calloc && elf_calloc && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated calloc(%zd, %zd)\n", GetTID(), n, s);
        return (void*)RunSafeFunction(my_context, hooked_calloc, 2, n, s);
    }
    void* ret = box_calloc(n, s);
    if(my_context) printf_log(LOG_DEBUG, "native calloc(%zu, %zu) => %p\n", n, s, ret);
    return ret;
}

EXPORT void* realloc(void* p, size_t s)
{
    if(hooked_realloc && elf_realloc && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated realloc(%p, %zd)\n", GetTID(), p, s);
        return (void*)RunSafeFunction(my_context, hooked_realloc, 2, p, s);
    }
    void* ret = box_realloc(p, s);
    if(my_context) printf_log(LOG_DEBUG, "native realloc(%p, %zu) => %p\n", p, s, ret);
    return ret;
}

EXPORT void* aligned_alloc(size_t align, size_t size)
{
    if(hooked_aligned_alloc && elf_aligned_alloc && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated aligned_alloc(%zd, %zd)\n", GetTID(), align, size);
        return (void*)RunSafeFunction(my_context, hooked_aligned_alloc, 2, align, size);
    }
    return box_memalign(align, size);
}

EXPORT void* memalign(size_t align, size_t size)
{
    if(hooked_memalign && elf_memalign && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated memalign(%zd, %zd)\n", GetTID(), align, size);
        return (void*)RunSafeFunction(my_context, hooked_memalign, 2, align, size);
    }
    return box_memalign(align, size);
}

EXPORT int posix_memalign(void** p, size_t align, size_t size)
{
    if(hooked_posix_memalign && elf_posix_memalign && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated posix_memalign(%p, %zd, %zd)\n", GetTID(), p, align, size);
        return (int)RunSafeFunction(my_context, hooked_posix_memalign, 3, p, align, size);
    }
    if(align%sizeof(void*) || pot(align)!=align)
        return EINVAL;
    void* ret = box_memalign(align, size);
    if(!ret)
        return ENOMEM;
    *p = ret;
    return 0;
}

EXPORT void* valloc(size_t size)
{
    if(hooked_valloc && elf_valloc && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated valloc(%zd)\n", GetTID(), size);
        return (void*)RunSafeFunction(my_context, hooked_valloc, 1, size);
    }
    return box_memalign(box64_pagesize, size);
}

EXPORT void* pvalloc(size_t size)
{
    if(hooked_pvalloc && elf_pvalloc && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated pvalloc(%zd)\n", GetTID(), size);
        return (void*)RunSafeFunction(my_context, hooked_pvalloc, 1, size);
    }
    return box_memalign(box64_pagesize, (size+box64_pagesize-1)&~(box64_pagesize-1));
}

EXPORT void cfree(void* p)
{
    if(hooked_cfree && elf_cfree && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated cfree(%p)\n", GetTID(), p);
        RunSafeFunction(my_context, hooked_cfree, 1, p);
    } else
        box_free(p);
}

EXPORT size_t malloc_usable_size(void* p)
{
    if(hooked_malloc_usable_size && elf_malloc_usable_size && go_hooking_malloc) {
        printf_log(LOG_DEBUG, "%04d|emulated malloc_usable_size(%p)\n", GetTID(), p);
        return (size_t)RunSafeFunction(my_context, hooked_malloc_usable_size, 1, p);
    } else
        return box_malloc_usable_size(p);
}

void checkHookedSymbols(lib_t *maplib, elfheader_t* h)
{
    for (size_t i=0; i<h->numDynSym; ++i) {
        const char * symname = h->DynStr+h->DynSym[i].st_name;
        int bind = ELF64_ST_BIND(h->DynSym[i].st_info);
        int type = ELF64_ST_TYPE(h->DynSym[i].st_info);
        int vis = h->DynSym[i].st_other&0x3;
        if((type==STT_FUNC) 
        && (vis==STV_DEFAULT || vis==STV_PROTECTED) && (h->DynSym[i].st_shndx!=0 && h->DynSym[i].st_shndx<=65521)) {
            uintptr_t offs = h->DynSym[i].st_value + h->delta;
            size_t sz = h->DynSym[i].st_size;
            if(bind!=STB_LOCAL && bind!=STB_WEAK) {
                #define GO(A, B) if (!hooked_##A && !strcmp(symname, #A)) {hooked_##A = offs; elf_##A = h; if(hooked_##A) printf_log(LOG_INFO, "Overriding %s to %p (%s)\n", #A, (void*)hooked_##A, h->name);}
                SUPER()
                #undef GO
            }
        }
    }
}

void init_malloc_hook() {
#ifdef ANDROID
    __libc_malloc = dlsym(RTLD_NEXT, "malloc");
    __libc_realloc = dlsym(RTLD_NEXT, "realloc");
    __libc_calloc = dlsym(RTLD_NEXT, "calloc");
    __libc_free = dlsym(RTLD_NEXT, "free");
    __libc_memalign = dlsym(RTLD_NEXT, "memalign");
#endif
    box_malloc_usable_size = dlsym(RTLD_NEXT, "malloc_usable_size");
}

#undef SUPER
/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.2.0.17) *
 *******************************************************************/
#ifndef __wrappedflacTYPES_H_
#define __wrappedflacTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef int32_t (*iFppPPPPPP_t)(void*, void*, void*, void*, void*, void*, void*, void*);
typedef int32_t (*iFpppppppppp_t)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);

#define SUPER() ADDED_FUNCTIONS() \
	GO(FLAC__metadata_chain_read_with_callbacks, iFppPPPPPP_t) \
	GO(FLAC__stream_decoder_init_stream, iFpppppppppp_t)

#endif // __wrappedflacTYPES_H_

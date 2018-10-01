/************************************************************************

    Name:     General Header

	Type:     C header file

	Desc:     Value and function types

	File:     genDef.h

	Prg:      Son Lee-Suk

************************************************************************/
#ifndef _GENDEF_H_
#define _GENDEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _OS_OSX_
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <time.h>
#include <stdint.h>

/* function type */
#define FT_PUBLIC 		extern
#define FT_PRIVATE 		static

/* primitive constant */
#ifdef CONST
#undef CONST
#define CONST 			const
#else
#define CONST 			const
#endif /* end of CONST */

#ifdef STATIC
#undef STATIC
#define STATIC 			static
#else
#define STATIC 			static
#endif /* end of STATIC */

#ifdef EXTERN
#undef EXTERN
#define EXTERN 			extern
#else
#define EXTERN 			extern
#endif /* end of EXTERN */

#ifdef VOLATILE
#undef VOLATILE
#define VOLATILE        volatile
#else
#define VOLATILE        volatile
#endif /* end of VOLATILE */

#ifdef REGISTER
#undef REGISTER
#define REGISTER        register
#else
#define REGISTER        register
#endif /* end of REGISTER */

#ifdef INLINE
#undef INLINE
#define INLINE          inline
#else
#define INLINE          inline
#endif /* end of REGISTER */

						/* basic type of value  definition */
#ifndef _OS_WIN32_
typedef void			VOID;
typedef unsigned int    UINT;
typedef signed int      SINT;
typedef unsigned short  USHORT;
typedef signed short    SSHORT;
typedef unsigned long   ULONG;
typedef signed long     SLONG;
typedef unsigned char   UCHAR;
typedef signed char     SCHAR;
typedef char            CHAR;
typedef double          DOUBLE;
typedef float           FLOT;
typedef size_t          SIZET;
typedef time_t          TIMET;
#endif
#ifdef _OS_WIN32_
#define SIEZT           size_t
#endif

#if 0
typedef unsigned char 	   U_8;
typedef signed char   	   S_8;
typedef unsigned short     U_16;
typedef signed short       S_16;
typedef unsigned int       U_32;
typedef signed int         S_32;
typedef unsigned long long U_64;
typedef long long          S_64;
#else
typedef uint8_t 	       U_8;
typedef int8_t 	           S_8;
typedef uint16_t           U_16;
typedef int16_t            S_16;
typedef uint32_t           U_32;
typedef int32_t            S_32;
typedef uint64_t           U_64;
typedef int64_t            S_64;
#endif

						/* extend type of value definition */
#ifndef _OS_WIN32_
typedef UCHAR           BOOL;       /* boolion */
typedef VOID            FUNC;       /* function */
typedef UINT            TYPE;       /* TYPE(ENUM) */
typedef UINT            SOCK;       /* socket */
#endif

						/* Return Type */
#define RT_RESULT		signed int
#define RT_LENTH		unsigned int

						/* Result code */
#define RC_OK		1
#define RC_NOK		0
#define RC_TRUE		1
#define RC_FALSE	0
#define RC_USED		1
#define RC_NOT_USED 0

						/* General Macro */
#ifdef GEN_CHK_ERR
#define GEN_CHK_ERR_RET_VOID(_stmt, _errLine){\
	    if(_stmt){\
			        _errLine;\
			        return;\
			    }\
}

#define GEN_CHK_ERR_RET(_stmt, _errLine, _ret){\
	    if(_stmt){\
			        _errLine;\
			        return _ret;\
			    }\
}
#else
#define GEN_CHK_ERR_RET_VOID(_stmt, _errLine)

#define GEN_CHK_ERR_RET(_stmt, _errLine, _ret)
#endif

#ifdef __cplusplus
}
#endif

#endif

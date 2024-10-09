#ifndef QTB_Util
#define QTB_Util

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif


#ifdef _WINDOWS
	#define qtbLog(s)	OutputDebugString(_T(s));
#else
	#define qtbLog(s)	printf(s);
#endif

//#define QTB_MEM_DBG
#ifdef QTB_MEM_DBG
#define	QTB_OVERLOAD_BLOCK		\
	int __mc[8192];

#define QTB_RAND_BAD_ALLOC(percent)	\
	if (rand() % 1000 < percent)	\
		throw std::bad_alloc();
#else
#define	QTB_OVERLOAD_BLOCK
#define QTB_RAND_BAD_ALLOC(percent)
#endif


#endif

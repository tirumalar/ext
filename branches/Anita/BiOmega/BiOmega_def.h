#ifndef _BIOMEGA_LIBRARY_DEF_
#define _BIOMEGA_LIBRARY_DEF_

#ifdef BIOMEGA_EXPORTS
#undef  BIOMEGA_DLL_EXPORTS
#define BIOMEGA_DLL_EXPORTS __declspec( dllexport )
#else // BIOMEGA_EXPORTS
#define BIOMEGA_DLL_EXPORTS
#endif

#endif // _BIOMEGA_LIBRARY_DEF_

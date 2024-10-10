#ifndef HZ_DEFINE_H
#define HZ_DEFINE_H

#ifdef _WIN32
#    ifdef LIBRARY_EXPORTS
#        define HPI extern "C" __declspec(dllexport)
#    else
#        define HPI extern "C" __declspec(dllimport)
#    endif
#elif
#    define HPI extern "C"
#endif

#endif
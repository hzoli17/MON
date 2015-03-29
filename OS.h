#ifndef PLATFORM
#ifndef PLATFORM
#ifdef _WIN64
#define PLATFORM	"Win64"
#define WINDOWS
#endif
#endif
#ifndef PLATFORM
#ifdef _WIN32
#define PLATFORM	"Win32"
#define WINDOWS
#endif
#endif
#ifdef _WIN16
#define PLATFORM	"Win16"
#define WINDOWS
#endif
#ifndef PLATFORM
#ifdef __APPLE__
#define	PLATFORM	"MacOS"
#endif
#endif
#ifndef PLATFORM
#ifdef _AIX
#define PLATFORM	"Aix"
#endif
#endif
#ifndef PLATFORM
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>
#if defined(BSD)
#define	PLATFORM	"Unix/BSD"
#endif
#endif
#endif
#ifndef PLATFORM
#ifdef __hpux
#define PLATFORM    "Unix/HP-UX"
#endif
#endif
#ifndef PLATFORM
#ifdef __linux__
#define PLATFORM	"Linux"
#endif
#endif
#ifndef PLATFORM
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define PLATFORM    "iOS/XCode emulator"
#elif TARGET_OS_IPHONE == 1
#define PLATFORM    "iOS"
#elif TARGET_OS_MAC == 1
#define PLATFORM    "MacOSX"
#endif
#endif
#endif
#ifndef PLATFORM
#if defined(__sun) && defined(__SVR4)
#define PLATFORM    "Solaris"
#endif
#endif
#ifndef PLATFORM
#ifdef __unix__
#define PLATFORM	"Unix"
#endif
#endif
#ifndef PLATFORM
#define PLATFORM	"Unkown"
#endif
#endif

#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    // Windows code (both 32/64-bit)
    #define IS_WINDOWS 1

    #ifdef _WIN64
        #define IS_WINDOWS_64 1
    #else
        #define IS_WINDOWS_32 1
    #endif

#elif defined(__APPLE__) && defined(__MACH__)
    // Apple ecosystem (Mac, iPhone, iPad, Simulator)
    #include <TargetConditionals.h>
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #define IS_MACOS 1

    #ifdef TARGET_OS_MAC
        #define IS_MACOS 1
    #endif
    
    #ifdef TARGET_OS_IPHONE
        #define IS_IOS 1
    #endif
    
    #ifdef TARGET_OS_SIMULATOR
        #define IS_SIMULATOR 1
    #endif
    
    #ifdef TARGET_CPU_ARM64
        #define IS_ARM64 1
    #elif defined(TARGET_CPU_X86_64)
        #define IS_X86_64 1
    #endif

#elif defined(__linux__)
    // Linux
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #define IS_LINUX 1

#elif defined(__ANDROID__)
    // Android
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #define IS_ANDROID 1

#elif defined(__unix__)
    // Other Unix (FreeBSD, OpenBSD, NetBSD, Solaris)
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #define IS_UNIX 1
    
    #ifdef __FreeBSD__
        #define IS_FREEBSD 1
    #elif defined(__OpenBSD__)
        #define IS_OPENBSD 1
    #elif defined(__NetBSD__)
        #define IS_NETBSD 1
    #endif

#else
    #error "Unsupported platform - this code only works on Windows, Mac, Linux, BSD, or Android"
#endif

#endif
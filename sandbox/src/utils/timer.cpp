#ifdef WIN32
#   include "../platform/win32/utils/timer.cpp.inl"
#else // PLATFORM_LINUX
#   include "../platform/linux/utils/timer.cpp.inl"
#endif

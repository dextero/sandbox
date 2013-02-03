#ifdef PLATFORM_WIN32
#	include "platform/win32/rendering/renderer.cpp.inl"
#else // PLATFORM_LINUX
#	include "platform/linux/rendering/renderer.cpp.inl"
#endif // PLATFORM_WIN32
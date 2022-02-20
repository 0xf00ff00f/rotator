#pragma once

#include <cstdio>

inline void log(const char *fmt)
{
    std::printf("%s", fmt);
}

template<typename... Args>
inline void log(const char *fmt, const Args &...args)
{
    std::printf(fmt, args...);
}

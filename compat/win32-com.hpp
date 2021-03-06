#pragma once

#ifdef _WIN32

#include "export.hpp"

#include <objbase.h>
#include <ole2.h>

enum com_type : int
{
    com_multithreaded = COINIT_MULTITHREADED,
    com_apartment = COINIT_APARTMENTTHREADED,
    com_invalid = 0,
};

bool OPENTRACK_COMPAT_EXPORT init_com_threading(com_type t = com_invalid);

#endif

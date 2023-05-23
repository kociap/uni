#pragma once

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG == 1

  #include <stdio.h>

  #define DEBUG_PRINT(...) printf(__VA_ARGS__)

#else

  #define DEBUG_PRINT(...)

#endif

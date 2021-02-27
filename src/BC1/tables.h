// rgbcx.h v1.12
// High-performance scalar BC1-5 encoders. Public Domain or MIT license (you choose - see below), written by Richard Geldreich 2020 <richgel99@gmail.com>.

#pragma once
#include <cstdint>

// This table is: 9 * (w * w), 9 * ((1.0f - w) * w), 9 * ((1.0f - w) * (1.0f - w))
// where w is [0,1/3,2/3,1]. 9 is the perfect multiplier.
static constexpr uint32_t g_weight_vals4[4] = {0x000009, 0x010204, 0x040201, 0x090000};

// multiplier is 4 for 3-color
static constexpr uint32_t g_weight_vals3[3] = {0x000004, 0x040000, 0x010101};

const uint32_t MIN_TOTAL_ORDERINGS = 1;
const uint32_t MAX_TOTAL_ORDERINGS3 = 32;

#if RGBCX_USE_SMALLER_TABLES
const uint32_t MAX_TOTAL_ORDERINGS4 = 32;
#else
const uint32_t MAX_TOTAL_ORDERINGS4 = 128;
#endif

extern const float g_midpoint5[32];
extern const float g_midpoint6[64];

const uint32_t NUM_UNIQUE_TOTAL_ORDERINGS4 = 969;
extern const uint8_t g_unique_total_orders4[NUM_UNIQUE_TOTAL_ORDERINGS4][4];

const uint32_t NUM_UNIQUE_TOTAL_ORDERINGS3 = 153;
extern const uint8_t g_unique_total_orders3[NUM_UNIQUE_TOTAL_ORDERINGS3][3];

extern const uint16_t g_best_total_orderings4[NUM_UNIQUE_TOTAL_ORDERINGS4][MAX_TOTAL_ORDERINGS4];

extern const uint8_t g_best_total_orderings3[NUM_UNIQUE_TOTAL_ORDERINGS3][32];

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright(c) 2020 Richard Geldreich, Jr.
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files(the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain(www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non - commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain.We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors.We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
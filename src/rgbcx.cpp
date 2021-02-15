// rgbcx.h v1.12
// High-performance scalar BC1-5 encoders. Public Domain or MIT license (you choose - see below), written by Richard Geldreich 2020 <richgel99@gmail.com>.

#pragma GCC diagnostic ignored "-Weverything"
#include "rgbcx.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstring>
#include <type_traits>

#include "BC1/BC1Block.h"
#include "Color.h"
#include "tables.h"
#include "util.h"

namespace rgbcx {

static const uint32_t TOTAL_ORDER_4_0_16 = 15;
static const uint32_t TOTAL_ORDER_4_1_16 = 700;
static const uint32_t TOTAL_ORDER_4_2_16 = 753;
static const uint32_t TOTAL_ORDER_4_3_16 = 515;
static uint16_t g_total_ordering4_hash[4096];
static float g_selector_factors4[NUM_UNIQUE_TOTAL_ORDERINGS4][3];

static const uint32_t TOTAL_ORDER_3_0_16 = 12;
static const uint32_t TOTAL_ORDER_3_1_16 = 15;
static const uint32_t TOTAL_ORDER_3_2_16 = 89;
static uint16_t g_total_ordering3_hash[256];
static float g_selector_factors3[NUM_UNIQUE_TOTAL_ORDERINGS3][3];

struct hist4 {
    uint8_t m_hist[4];

    hist4() { memset(m_hist, 0, sizeof(m_hist)); }

    hist4(uint32_t i, uint32_t j, uint32_t k, uint32_t l) {
        m_hist[0] = (uint8_t)i;
        m_hist[1] = (uint8_t)j;
        m_hist[2] = (uint8_t)k;
        m_hist[3] = (uint8_t)l;
    }

    inline bool operator==(const hist4 &h) const {
        if (m_hist[0] != h.m_hist[0]) return false;
        if (m_hist[1] != h.m_hist[1]) return false;
        if (m_hist[2] != h.m_hist[2]) return false;
        if (m_hist[3] != h.m_hist[3]) return false;
        return true;
    }

    inline bool any_16() const { return (m_hist[0] == 16) || (m_hist[1] == 16) || (m_hist[2] == 16) || (m_hist[3] == 16); }

    inline uint32_t lookup_total_ordering_index() const {
        if (m_hist[0] == 16)
            return TOTAL_ORDER_4_0_16;
        else if (m_hist[1] == 16)
            return TOTAL_ORDER_4_1_16;
        else if (m_hist[2] == 16)
            return TOTAL_ORDER_4_2_16;
        else if (m_hist[3] == 16)
            return TOTAL_ORDER_4_3_16;

        // Must sum to 16, so m_hist[3] isn't needed.
        return g_total_ordering4_hash[m_hist[0] | (m_hist[1] << 4) | (m_hist[2] << 8)];
    }
};

struct hist3 {
    uint8_t m_hist[3];

    hist3() { memset(m_hist, 0, sizeof(m_hist)); }

    hist3(uint32_t i, uint32_t j, uint32_t k) {
        m_hist[0] = (uint8_t)i;
        m_hist[1] = (uint8_t)j;
        m_hist[2] = (uint8_t)k;
    }

    inline bool operator==(const hist3 &h) const {
        if (m_hist[0] != h.m_hist[0]) return false;
        if (m_hist[1] != h.m_hist[1]) return false;
        if (m_hist[2] != h.m_hist[2]) return false;
        return true;
    }

    inline bool any_16() const { return (m_hist[0] == 16) || (m_hist[1] == 16) || (m_hist[2] == 16); }

    inline uint32_t lookup_total_ordering_index() const {
        if (m_hist[0] == 16)
            return TOTAL_ORDER_3_0_16;
        else if (m_hist[1] == 16)
            return TOTAL_ORDER_3_1_16;
        else if (m_hist[2] == 16)
            return TOTAL_ORDER_3_2_16;

        // Must sum to 16, so m_hist[2] isn't needed.
        return g_total_ordering3_hash[m_hist[0] | (m_hist[1] << 4)];
    }
};

struct bc1_match_entry {
    uint8_t m_hi;
    uint8_t m_lo;
    uint8_t m_e;
};

static bc1_approx_mode g_bc1_approx_mode;
static bc1_match_entry g_bc1_match5_equals_1[256], g_bc1_match6_equals_1[256];
static bc1_match_entry g_bc1_match5_half[256], g_bc1_match6_half[256];

// v0, v1 = unexpanded DXT1 endpoint values (5/6-bits)
// c0, c1 = expanded DXT1 endpoint values (8-bits)
static inline int interp_5_6_ideal(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    return (c0 * 2 + c1) / 3;
}
static inline int interp_5_6_ideal_round(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    return (c0 * 2 + c1 + 1) / 3;
}
static inline int interp_half_5_6_ideal(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    return (c0 + c1) / 2;
}

static inline int interp_5_nv(int v0, int v1) {
    assert(v0 < 32 && v1 < 32);
    return ((2 * v0 + v1) * 22) / 8;
}
static inline int interp_6_nv(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    const int gdiff = c1 - c0;
    return (256 * c0 + (gdiff / 4) + 128 + gdiff * 80) / 256;
}

static inline int interp_half_5_nv(int v0, int v1) {
    assert(v0 < 32 && v1 < 32);
    return ((v0 + v1) * 33) / 8;
}
static inline int interp_half_6_nv(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    const int gdiff = c1 - c0;
    return (256 * c0 + gdiff / 4 + 128 + gdiff * 128) / 256;
}

static inline int interp_5_6_amd(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    return (c0 * 43 + c1 * 21 + 32) >> 6;
}
static inline int interp_half_5_6_amd(int c0, int c1) {
    assert(c0 < 256 && c1 < 256);
    return (c0 + c1 + 1) >> 1;
}

static inline int interp_5(int v0, int v1, int c0, int c1, bc1_approx_mode mode) {
    // assert(scale_5_to_8(v0) == c0 && scale5To8(v1) == c1);
    switch (mode) {
        case bc1_approx_mode::cBC1NVidia:
            return interp_5_nv(v0, v1);
        case bc1_approx_mode::cBC1AMD:
            return interp_5_6_amd(c0, c1);
        default:
        case bc1_approx_mode::cBC1Ideal:
            return interp_5_6_ideal(c0, c1);
        case bc1_approx_mode::cBC1IdealRound4:
            return interp_5_6_ideal_round(c0, c1);
    }
}

static inline int interp_6(int v0, int v1, int c0, int c1, bc1_approx_mode mode) {
    (void)v0;
    (void)v1;
    // assert(scale_6_to_8(v0) == c0 && scale6To8(v1) == c1);
    switch (mode) {
        case bc1_approx_mode::cBC1NVidia:
            return interp_6_nv(c0, c1);
        case bc1_approx_mode::cBC1AMD:
            return interp_5_6_amd(c0, c1);
        default:
        case bc1_approx_mode::cBC1Ideal:
            return interp_5_6_ideal(c0, c1);
        case bc1_approx_mode::cBC1IdealRound4:
            return interp_5_6_ideal_round(c0, c1);
    }
}

static inline unsigned int interp_half_5(unsigned int v0, unsigned int v1, unsigned int c0, unsigned int c1, bc1_approx_mode mode) {
    assert(scale5To8(v0) == c0 && scale5To8(v1) == c1);
    switch (mode) {
        case bc1_approx_mode::cBC1NVidia:
            return interp_half_5_nv(v0, v1);
        case bc1_approx_mode::cBC1AMD:
            return interp_half_5_6_amd(c0, c1);
        case bc1_approx_mode::cBC1Ideal:
        case bc1_approx_mode::cBC1IdealRound4:
        default:
            return interp_half_5_6_ideal(c0, c1);
    }
}

static inline unsigned int interp_half_6(unsigned v0, unsigned v1, unsigned c0, bc1_approx_mode mode, unsigned c1) {
    (void)v0;
    (void)v1;
    assert(scale6To8(v0) == c0 && scale6To8(v1) == c1);
    switch (mode) {
        case bc1_approx_mode::cBC1NVidia:
            return interp_half_6_nv(c0, c1);
        case bc1_approx_mode::cBC1AMD:
            return interp_half_5_6_amd(c0, c1);
        case bc1_approx_mode::cBC1Ideal:
        case bc1_approx_mode::cBC1IdealRound4:
        default:
            return interp_half_5_6_ideal(c0, c1);
    }
}

static void prepare_bc1_single_color_table_half(bc1_match_entry *pTable, const uint8_t *pExpand, int size, bc1_approx_mode mode) {
    for (int i = 0; i < 256; i++) {
        int lowest_e = 256;
        for (int lo = 0; lo < size; lo++) {
            const int lo_e = pExpand[lo];

            for (int hi = 0; hi < size; hi++) {
                const int hi_e = pExpand[hi];

                const int v = (size == 32) ? interp_half_5(hi, lo, hi_e, lo_e, mode) : interp_half_6(hi, lo, hi_e, mode, lo_e);

                int e = iabs(v - i);

                // We only need to factor in 3% error in BC1 ideal mode.
                if ((mode == bc1_approx_mode::cBC1Ideal) || (mode == bc1_approx_mode::cBC1IdealRound4)) e += (iabs(hi_e - lo_e) * 3) / 100;

                // Favor equal endpoints, for lower error on actual GPU's which approximate the interpolation.
                if ((e < lowest_e) || ((e == lowest_e) && (lo == hi))) {
                    pTable[i].m_hi = static_cast<uint8_t>(hi);
                    pTable[i].m_lo = static_cast<uint8_t>(lo);

                    assert(e <= UINT8_MAX);
                    pTable[i].m_e = static_cast<uint8_t>(e);

                    lowest_e = e;
                }

            }  // hi
        }      // lo
    }
}

static void prepare_bc1_single_color_table(bc1_match_entry *pTable, const uint8_t *pExpand, int size, bc1_approx_mode mode) {
    for (int i = 0; i < 256; i++) {
        int lowest_e = 256;
        for (int lo = 0; lo < size; lo++) {
            const int lo_e = pExpand[lo];

            for (int hi = 0; hi < size; hi++) {
                const int hi_e = pExpand[hi];

                const int v = (size == 32) ? interp_5(hi, lo, hi_e, lo_e, mode) : interp_6(hi, lo, hi_e, lo_e, mode);

                int e = iabs(v - i);

                if ((mode == bc1_approx_mode::cBC1Ideal) || (mode == bc1_approx_mode::cBC1IdealRound4)) e += (iabs(hi_e - lo_e) * 3) / 100;

                // Favor equal endpoints, for lower error on actual GPU's which approximate the interpolation.
                if ((e < lowest_e) || ((e == lowest_e) && (lo == hi))) {
                    pTable[i].m_hi = static_cast<uint8_t>(hi);
                    pTable[i].m_lo = static_cast<uint8_t>(lo);

                    assert(e <= UINT8_MAX);
                    pTable[i].m_e = static_cast<uint8_t>(e);

                    lowest_e = e;
                }

            }  // hi
        }      // lo
    }
}

// This table is: 9 * (w * w), 9 * ((1.0f - w) * w), 9 * ((1.0f - w) * (1.0f - w))
// where w is [0,1/3,2/3,1]. 9 is the perfect multiplier.
static const uint32_t g_weight_vals4[4] = {0x000009, 0x010204, 0x040201, 0x090000};

// multiplier is 4 for 3-color
static const uint32_t g_weight_vals3[3] = {0x000004, 0x040000, 0x010101};

static inline void compute_selector_factors4(const hist4 &h, float &iz00, float &iz10, float &iz11) {
    uint32_t weight_accum = 0;
    for (uint32_t sel = 0; sel < 4; sel++) weight_accum += g_weight_vals4[sel] * h.m_hist[sel];

    float z00 = (float)((weight_accum >> 16) & 0xFF);
    float z10 = (float)((weight_accum >> 8) & 0xFF);
    float z11 = (float)(weight_accum & 0xFF);
    float z01 = z10;

    float det = z00 * z11 - z01 * z10;
    if (fabs(det) < 1e-8f)
        det = 0.0f;
    else
        det = (3.0f / 255.0f) / det;

    iz00 = z11 * det;
    iz10 = -z10 * det;
    iz11 = z00 * det;
}

static inline void compute_selector_factors3(const hist3 &h, float &iz00, float &iz10, float &iz11) {
    uint32_t weight_accum = 0;
    for (uint32_t sel = 0; sel < 3; sel++) weight_accum += g_weight_vals3[sel] * h.m_hist[sel];

    float z00 = (float)((weight_accum >> 16) & 0xFF);
    float z10 = (float)((weight_accum >> 8) & 0xFF);
    float z11 = (float)(weight_accum & 0xFF);
    float z01 = z10;

    float det = z00 * z11 - z01 * z10;
    if (fabs(det) < 1e-8f)
        det = 0.0f;
    else
        det = (2.0f / 255.0f) / det;

    iz00 = z11 * det;
    iz10 = -z10 * det;
    iz11 = z00 * det;
}

static bool g_initialized;

void init(bc1_approx_mode mode) {
    g_bc1_approx_mode = mode;

    uint8_t bc1_expand5[32];
    for (int i = 0; i < 32; i++) bc1_expand5[i] = static_cast<uint8_t>((i << 3) | (i >> 2));
    prepare_bc1_single_color_table(g_bc1_match5_equals_1, bc1_expand5, 32, mode);
    prepare_bc1_single_color_table_half(g_bc1_match5_half, bc1_expand5, 32, mode);

    uint8_t bc1_expand6[64];
    for (int i = 0; i < 64; i++) bc1_expand6[i] = static_cast<uint8_t>((i << 2) | (i >> 4));
    prepare_bc1_single_color_table(g_bc1_match6_equals_1, bc1_expand6, 64, mode);
    prepare_bc1_single_color_table_half(g_bc1_match6_half, bc1_expand6, 64, mode);

    for (uint32_t i = 0; i < NUM_UNIQUE_TOTAL_ORDERINGS4; i++) {
        hist4 h;
        h.m_hist[0] = (uint8_t)g_unique_total_orders4[i][0];
        h.m_hist[1] = (uint8_t)g_unique_total_orders4[i][1];
        h.m_hist[2] = (uint8_t)g_unique_total_orders4[i][2];
        h.m_hist[3] = (uint8_t)g_unique_total_orders4[i][3];

        if (!h.any_16()) {
            const uint32_t index = h.m_hist[0] | (h.m_hist[1] << 4) | (h.m_hist[2] << 8);
            assert(index < 4096);
            g_total_ordering4_hash[index] = (uint16_t)i;
        }

        compute_selector_factors4(h, g_selector_factors4[i][0], g_selector_factors4[i][1], g_selector_factors4[i][2]);
    }

    for (uint32_t i = 0; i < NUM_UNIQUE_TOTAL_ORDERINGS3; i++) {
        hist3 h;
        h.m_hist[0] = (uint8_t)g_unique_total_orders3[i][0];
        h.m_hist[1] = (uint8_t)g_unique_total_orders3[i][1];
        h.m_hist[2] = (uint8_t)g_unique_total_orders3[i][2];

        if (!h.any_16()) {
            const uint32_t index = h.m_hist[0] | (h.m_hist[1] << 4);
            assert(index < 256);
            g_total_ordering3_hash[index] = (uint16_t)i;
        }

        compute_selector_factors3(h, g_selector_factors3[i][0], g_selector_factors3[i][1], g_selector_factors3[i][2]);
    }

    g_initialized = true;
}

void encode_bc1_solid_block(void *pDst, uint32_t fr, uint32_t fg, uint32_t fb, bool allow_3color) {
    BC1Block *pDst_block = static_cast<BC1Block *>(pDst);

    uint32_t mask = 0xAA;
    int max16 = -1, min16 = 0;

    if (allow_3color) {
        const uint32_t err4 = g_bc1_match5_equals_1[fr].m_e + g_bc1_match6_equals_1[fg].m_e + g_bc1_match5_equals_1[fb].m_e;
        const uint32_t err3 = g_bc1_match5_half[fr].m_e + g_bc1_match6_half[fg].m_e + g_bc1_match5_half[fb].m_e;

        if (err3 < err4) {
            max16 = (g_bc1_match5_half[fr].m_hi << 11) | (g_bc1_match6_half[fg].m_hi << 5) | g_bc1_match5_half[fb].m_hi;
            min16 = (g_bc1_match5_half[fr].m_lo << 11) | (g_bc1_match6_half[fg].m_lo << 5) | g_bc1_match5_half[fb].m_lo;

            if (max16 > min16) std::swap(max16, min16);
        }
    }

    if (max16 == -1) {
        max16 = (g_bc1_match5_equals_1[fr].m_hi << 11) | (g_bc1_match6_equals_1[fg].m_hi << 5) | g_bc1_match5_equals_1[fb].m_hi;
        min16 = (g_bc1_match5_equals_1[fr].m_lo << 11) | (g_bc1_match6_equals_1[fg].m_lo << 5) | g_bc1_match5_equals_1[fb].m_lo;

        if (min16 == max16) {
            // Always forbid 3 color blocks
            // This is to guarantee that BC3 blocks never use punchthrough alpha (3 color) mode, which isn't supported on some (all?) GPU's.
            mask = 0;

            // Make l > h
            if (min16 > 0)
                min16--;
            else {
                // l = h = 0
                assert(min16 == max16 && max16 == 0);

                max16 = 1;
                min16 = 0;
                mask = 0x55;
            }

            assert(max16 > min16);
        }

        if (max16 < min16) {
            std::swap(max16, min16);
            mask ^= 0x55;
        }
    }

    pDst_block->SetLowColor(static_cast<uint16_t>(max16));
    pDst_block->SetHighColor(static_cast<uint16_t>(min16));
    pDst_block->selectors[0] = static_cast<uint8_t>(mask);
    pDst_block->selectors[1] = static_cast<uint8_t>(mask);
    pDst_block->selectors[2] = static_cast<uint8_t>(mask);
    pDst_block->selectors[3] = static_cast<uint8_t>(mask);
}

static const float g_midpoint5[32] = {.015686f, .047059f, .078431f, .111765f, .145098f, .176471f, .207843f, .241176f, .274510f, .305882f, .337255f,
                                      .370588f, .403922f, .435294f, .466667f, .5f,      .533333f, .564706f, .596078f, .629412f, .662745f, .694118f,
                                      .725490f, .758824f, .792157f, .823529f, .854902f, .888235f, .921569f, .952941f, .984314f, 1e+37f};
static const float g_midpoint6[64] = {.007843f, .023529f, .039216f, .054902f, .070588f, .086275f, .101961f, .117647f, .133333f, .149020f, .164706f,
                                      .180392f, .196078f, .211765f, .227451f, .245098f, .262745f, .278431f, .294118f, .309804f, .325490f, .341176f,
                                      .356863f, .372549f, .388235f, .403922f, .419608f, .435294f, .450980f, .466667f, .482353f, .500000f, .517647f,
                                      .533333f, .549020f, .564706f, .580392f, .596078f, .611765f, .627451f, .643137f, .658824f, .674510f, .690196f,
                                      .705882f, .721569f, .737255f, .754902f, .772549f, .788235f, .803922f, .819608f, .835294f, .850980f, .866667f,
                                      .882353f, .898039f, .913725f, .929412f, .945098f, .960784f, .976471f, .992157f, 1e+37f};

struct vec3F {
    float c[3];
};

static inline void compute_least_squares_endpoints4_rgb(vec3F *pXl, vec3F *pXh, int total_r, int total_g, int total_b, float iz00, float iz10, float iz11,
                                                        uint32_t s, const uint32_t r_sum[17], const uint32_t g_sum[17], const uint32_t b_sum[17]) {
    const float iz01 = iz10;

    const uint32_t f1 = g_unique_total_orders4[s][0];
    const uint32_t f2 = g_unique_total_orders4[s][0] + g_unique_total_orders4[s][1];
    const uint32_t f3 = g_unique_total_orders4[s][0] + g_unique_total_orders4[s][1] + g_unique_total_orders4[s][2];
    uint32_t uq00_r = (r_sum[f2] - r_sum[f1]) + (r_sum[f3] - r_sum[f2]) * 2 + (r_sum[16] - r_sum[f3]) * 3;
    uint32_t uq00_g = (g_sum[f2] - g_sum[f1]) + (g_sum[f3] - g_sum[f2]) * 2 + (g_sum[16] - g_sum[f3]) * 3;
    uint32_t uq00_b = (b_sum[f2] - b_sum[f1]) + (b_sum[f3] - b_sum[f2]) * 2 + (b_sum[16] - b_sum[f3]) * 3;

    float q10_r = (float)(total_r * 3 - uq00_r);
    float q10_g = (float)(total_g * 3 - uq00_g);
    float q10_b = (float)(total_b * 3 - uq00_b);

    pXl->c[0] = iz00 * (float)uq00_r + iz01 * q10_r;
    pXh->c[0] = iz10 * (float)uq00_r + iz11 * q10_r;

    pXl->c[1] = iz00 * (float)uq00_g + iz01 * q10_g;
    pXh->c[1] = iz10 * (float)uq00_g + iz11 * q10_g;

    pXl->c[2] = iz00 * (float)uq00_b + iz01 * q10_b;
    pXh->c[2] = iz10 * (float)uq00_b + iz11 * q10_b;
}

static inline bool compute_least_squares_endpoints4_rgb(const Color *pColors, const uint8_t *pSelectors, vec3F *pXl, vec3F *pXh, int total_r, int total_g,
                                                        int total_b) {
    uint32_t uq00_r = 0, uq00_g = 0, uq00_b = 0;
    uint32_t weight_accum = 0;
    for (uint32_t i = 0; i < 16; i++) {
        const uint8_t r = pColors[i][0], g = pColors[i][1], b = pColors[i][2];
        const uint8_t sel = pSelectors[i];

        weight_accum += g_weight_vals4[sel];
        uq00_r += sel * r;
        uq00_g += sel * g;
        uq00_b += sel * b;
    }

    int q10_r = total_r * 3 - uq00_r;
    int q10_g = total_g * 3 - uq00_g;
    int q10_b = total_b * 3 - uq00_b;

    float z00 = (float)((weight_accum >> 16) & 0xFF);
    float z10 = (float)((weight_accum >> 8) & 0xFF);
    float z11 = (float)(weight_accum & 0xFF);
    float z01 = z10;

    float det = z00 * z11 - z01 * z10;
    if (fabs(det) < 1e-8f) return false;

    det = (3.0f / 255.0f) / det;

    float iz00, iz01, iz10, iz11;
    iz00 = z11 * det;
    iz01 = -z01 * det;
    iz10 = -z10 * det;
    iz11 = z00 * det;

    pXl->c[0] = iz00 * (float)uq00_r + iz01 * q10_r;
    pXh->c[0] = iz10 * (float)uq00_r + iz11 * q10_r;

    pXl->c[1] = iz00 * (float)uq00_g + iz01 * q10_g;
    pXh->c[1] = iz10 * (float)uq00_g + iz11 * q10_g;

    pXl->c[2] = iz00 * (float)uq00_b + iz01 * q10_b;
    pXh->c[2] = iz10 * (float)uq00_b + iz11 * q10_b;

    return true;
}

static inline void compute_least_squares_endpoints3_rgb(vec3F *pXl, vec3F *pXh, int total_r, int total_g, int total_b, float iz00, float iz10, float iz11,
                                                        uint32_t s, const uint32_t r_sum[17], const uint32_t g_sum[17], const uint32_t b_sum[17]) {
    const float iz01 = iz10;

    // Compensates for BC1 3-color ordering, which is selector 0, 2, 1
    const uint32_t f1 = g_unique_total_orders3[s][0];
    const uint32_t f2 = g_unique_total_orders3[s][0] + g_unique_total_orders3[s][2];
    uint32_t uq00_r = (r_sum[16] - r_sum[f2]) * 2 + (r_sum[f2] - r_sum[f1]);
    uint32_t uq00_g = (g_sum[16] - g_sum[f2]) * 2 + (g_sum[f2] - g_sum[f1]);
    uint32_t uq00_b = (b_sum[16] - b_sum[f2]) * 2 + (b_sum[f2] - b_sum[f1]);

    float q10_r = (float)(total_r * 2 - uq00_r);
    float q10_g = (float)(total_g * 2 - uq00_g);
    float q10_b = (float)(total_b * 2 - uq00_b);

    pXl->c[0] = iz00 * (float)uq00_r + iz01 * q10_r;
    pXh->c[0] = iz10 * (float)uq00_r + iz11 * q10_r;

    pXl->c[1] = iz00 * (float)uq00_g + iz01 * q10_g;
    pXh->c[1] = iz10 * (float)uq00_g + iz11 * q10_g;

    pXl->c[2] = iz00 * (float)uq00_b + iz01 * q10_b;
    pXh->c[2] = iz10 * (float)uq00_b + iz11 * q10_b;
}

static inline bool compute_least_squares_endpoints3_rgb(bool use_black, const Color *pColors, const uint8_t *pSelectors, vec3F *pXl, vec3F *pXh) {
    int uq00_r = 0, uq00_g = 0, uq00_b = 0;
    uint32_t weight_accum = 0;
    int total_r = 0, total_g = 0, total_b = 0;
    for (uint32_t i = 0; i < 16; i++) {
        const uint8_t r = pColors[i][0], g = pColors[i][1], b = pColors[i][2];
        if (use_black) {
            if ((r | g | b) < 4) continue;
        }

        const uint8_t sel = pSelectors[i];
        assert(sel <= 3);
        if (sel == 3) continue;

        weight_accum += g_weight_vals3[sel];

        static const uint8_t s_tran[3] = {0, 2, 1};
        const uint8_t tsel = s_tran[sel];
        uq00_r += tsel * r;
        uq00_g += tsel * g;
        uq00_b += tsel * b;

        total_r += r;
        total_g += g;
        total_b += b;
    }

    int q10_r = total_r * 2 - uq00_r;
    int q10_g = total_g * 2 - uq00_g;
    int q10_b = total_b * 2 - uq00_b;

    float z00 = (float)((weight_accum >> 16) & 0xFF);
    float z10 = (float)((weight_accum >> 8) & 0xFF);
    float z11 = (float)(weight_accum & 0xFF);
    float z01 = z10;

    float det = z00 * z11 - z01 * z10;
    if (fabs(det) < 1e-8f) return false;

    det = (2.0f / 255.0f) / det;

    float iz00, iz01, iz10, iz11;
    iz00 = z11 * det;
    iz01 = -z01 * det;
    iz10 = -z10 * det;
    iz11 = z00 * det;

    pXl->c[0] = iz00 * (float)uq00_r + iz01 * q10_r;
    pXh->c[0] = iz10 * (float)uq00_r + iz11 * q10_r;

    pXl->c[1] = iz00 * (float)uq00_g + iz01 * q10_g;
    pXh->c[1] = iz10 * (float)uq00_g + iz11 * q10_g;

    pXl->c[2] = iz00 * (float)uq00_b + iz01 * q10_b;
    pXh->c[2] = iz10 * (float)uq00_b + iz11 * q10_b;

    return true;
}

static inline void bc1_get_block_colors4(uint32_t block_r[4], uint32_t block_g[4], uint32_t block_b[4], uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr,
                                         uint32_t hg, uint32_t hb) {
    block_r[0] = (lr << 3) | (lr >> 2);
    block_g[0] = (lg << 2) | (lg >> 4);
    block_b[0] = (lb << 3) | (lb >> 2);
    block_r[3] = (hr << 3) | (hr >> 2);
    block_g[3] = (hg << 2) | (hg >> 4);
    block_b[3] = (hb << 3) | (hb >> 2);

    if (g_bc1_approx_mode == bc1_approx_mode::cBC1Ideal) {
        block_r[1] = (block_r[0] * 2 + block_r[3]) / 3;
        block_g[1] = (block_g[0] * 2 + block_g[3]) / 3;
        block_b[1] = (block_b[0] * 2 + block_b[3]) / 3;
        block_r[2] = (block_r[3] * 2 + block_r[0]) / 3;
        block_g[2] = (block_g[3] * 2 + block_g[0]) / 3;
        block_b[2] = (block_b[3] * 2 + block_b[0]) / 3;
    } else if (g_bc1_approx_mode == bc1_approx_mode::cBC1IdealRound4) {
        block_r[1] = (block_r[0] * 2 + block_r[3] + 1) / 3;
        block_g[1] = (block_g[0] * 2 + block_g[3] + 1) / 3;
        block_b[1] = (block_b[0] * 2 + block_b[3] + 1) / 3;
        block_r[2] = (block_r[3] * 2 + block_r[0] + 1) / 3;
        block_g[2] = (block_g[3] * 2 + block_g[0] + 1) / 3;
        block_b[2] = (block_b[3] * 2 + block_b[0] + 1) / 3;
    } else if (g_bc1_approx_mode == bc1_approx_mode::cBC1AMD) {
        block_r[1] = interp_5_6_amd(block_r[0], block_r[3]);
        block_g[1] = interp_5_6_amd(block_g[0], block_g[3]);
        block_b[1] = interp_5_6_amd(block_b[0], block_b[3]);
        block_r[2] = interp_5_6_amd(block_r[3], block_r[0]);
        block_g[2] = interp_5_6_amd(block_g[3], block_g[0]);
        block_b[2] = interp_5_6_amd(block_b[3], block_b[0]);
    } else {
        block_r[1] = interp_5_nv(lr, hr);
        block_g[1] = interp_6_nv(block_g[0], block_g[3]);
        block_b[1] = interp_5_nv(lb, hb);
        block_r[2] = interp_5_nv(hr, lr);
        block_g[2] = interp_6_nv(block_g[3], block_g[0]);
        block_b[2] = interp_5_nv(hb, lb);
    }
}

static inline void bc1_get_block_colors3(uint32_t block_r[3], uint32_t block_g[3], uint32_t block_b[3], uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr,
                                         uint32_t hg, uint32_t hb) {
    block_r[0] = (lr << 3) | (lr >> 2);
    block_g[0] = (lg << 2) | (lg >> 4);
    block_b[0] = (lb << 3) | (lb >> 2);
    block_r[1] = (hr << 3) | (hr >> 2);
    block_g[1] = (hg << 2) | (hg >> 4);
    block_b[1] = (hb << 3) | (hb >> 2);

    if ((g_bc1_approx_mode == bc1_approx_mode::cBC1Ideal) || (g_bc1_approx_mode == bc1_approx_mode::cBC1IdealRound4)) {
        block_r[2] = (block_r[0] + block_r[1]) / 2;
        block_g[2] = (block_g[0] + block_g[1]) / 2;
        block_b[2] = (block_b[0] + block_b[1]) / 2;
    } else if (g_bc1_approx_mode == bc1_approx_mode::cBC1AMD) {
        block_r[2] = interp_half_5_6_amd(block_r[0], block_r[1]);
        block_g[2] = interp_half_5_6_amd(block_g[0], block_g[1]);
        block_b[2] = interp_half_5_6_amd(block_b[0], block_b[1]);
    } else {
        block_r[2] = interp_half_5_nv(lr, hr);
        block_g[2] = interp_half_6_nv(block_g[0], block_g[1]);
        block_b[2] = interp_half_5_nv(lb, hb);
    }
}

static inline void bc1_find_sels4_noerr(const Color *pSrc_pixels, uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr, uint32_t hg, uint32_t hb,
                                        uint8_t sels[16]) {
    uint32_t block_r[4], block_g[4], block_b[4];
    bc1_get_block_colors4(block_r, block_g, block_b, lr, lg, lb, hr, hg, hb);

    int ar = block_r[3] - block_r[0], ag = block_g[3] - block_g[0], ab = block_b[3] - block_b[0];

    int dots[4];
    for (uint32_t i = 0; i < 4; i++) dots[i] = (int)block_r[i] * ar + (int)block_g[i] * ag + (int)block_b[i] * ab;

    int t0 = dots[0] + dots[1], t1 = dots[1] + dots[2], t2 = dots[2] + dots[3];

    ar *= 2;
    ag *= 2;
    ab *= 2;

    static const uint8_t s_sels[4] = {3, 2, 1, 0};

    for (uint32_t i = 0; i < 16; i += 4) {
        const int d0 = pSrc_pixels[i + 0].r * ar + pSrc_pixels[i + 0].g * ag + pSrc_pixels[i + 0].b * ab;
        const int d1 = pSrc_pixels[i + 1].r * ar + pSrc_pixels[i + 1].g * ag + pSrc_pixels[i + 1].b * ab;
        const int d2 = pSrc_pixels[i + 2].r * ar + pSrc_pixels[i + 2].g * ag + pSrc_pixels[i + 2].b * ab;
        const int d3 = pSrc_pixels[i + 3].r * ar + pSrc_pixels[i + 3].g * ag + pSrc_pixels[i + 3].b * ab;

        sels[i + 0] = s_sels[(d0 <= t0) + (d0 < t1) + (d0 < t2)];
        sels[i + 1] = s_sels[(d1 <= t0) + (d1 < t1) + (d1 < t2)];
        sels[i + 2] = s_sels[(d2 <= t0) + (d2 < t1) + (d2 < t2)];
        sels[i + 3] = s_sels[(d3 <= t0) + (d3 < t1) + (d3 < t2)];
    }
}

static inline uint32_t bc1_find_sels4_fasterr(const Color *pSrc_pixels, uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr, uint32_t hg, uint32_t hb,
                                              uint8_t sels[16], uint32_t cur_err) {
    uint32_t block_r[4], block_g[4], block_b[4];
    bc1_get_block_colors4(block_r, block_g, block_b, lr, lg, lb, hr, hg, hb);

    int ar = block_r[3] - block_r[0], ag = block_g[3] - block_g[0], ab = block_b[3] - block_b[0];

    int dots[4];
    for (uint32_t i = 0; i < 4; i++) dots[i] = (int)block_r[i] * ar + (int)block_g[i] * ag + (int)block_b[i] * ab;

    int t0 = dots[0] + dots[1], t1 = dots[1] + dots[2], t2 = dots[2] + dots[3];

    ar *= 2;
    ag *= 2;
    ab *= 2;

    static const uint8_t s_sels[4] = {3, 2, 1, 0};

    uint32_t total_err = 0;

    for (uint32_t i = 0; i < 16; i += 4) {
        const int d0 = pSrc_pixels[i + 0].r * ar + pSrc_pixels[i + 0].g * ag + pSrc_pixels[i + 0].b * ab;
        const int d1 = pSrc_pixels[i + 1].r * ar + pSrc_pixels[i + 1].g * ag + pSrc_pixels[i + 1].b * ab;
        const int d2 = pSrc_pixels[i + 2].r * ar + pSrc_pixels[i + 2].g * ag + pSrc_pixels[i + 2].b * ab;
        const int d3 = pSrc_pixels[i + 3].r * ar + pSrc_pixels[i + 3].g * ag + pSrc_pixels[i + 3].b * ab;

        uint8_t sel0 = s_sels[(d0 <= t0) + (d0 < t1) + (d0 < t2)];
        uint8_t sel1 = s_sels[(d1 <= t0) + (d1 < t1) + (d1 < t2)];
        uint8_t sel2 = s_sels[(d2 <= t0) + (d2 < t1) + (d2 < t2)];
        uint8_t sel3 = s_sels[(d3 <= t0) + (d3 < t1) + (d3 < t2)];

        sels[i + 0] = sel0;
        sels[i + 1] = sel1;
        sels[i + 2] = sel2;
        sels[i + 3] = sel3;

        total_err +=
            squarei(pSrc_pixels[i + 0].r - block_r[sel0]) + squarei(pSrc_pixels[i + 0].g - block_g[sel0]) + squarei(pSrc_pixels[i + 0].b - block_b[sel0]);
        total_err +=
            squarei(pSrc_pixels[i + 1].r - block_r[sel1]) + squarei(pSrc_pixels[i + 1].g - block_g[sel1]) + squarei(pSrc_pixels[i + 1].b - block_b[sel1]);
        total_err +=
            squarei(pSrc_pixels[i + 2].r - block_r[sel2]) + squarei(pSrc_pixels[i + 2].g - block_g[sel2]) + squarei(pSrc_pixels[i + 2].b - block_b[sel2]);
        total_err +=
            squarei(pSrc_pixels[i + 3].r - block_r[sel3]) + squarei(pSrc_pixels[i + 3].g - block_g[sel3]) + squarei(pSrc_pixels[i + 3].b - block_b[sel3]);

        if (total_err >= cur_err) break;
    }

    return total_err;
}

static inline uint32_t bc1_find_sels4_check2_err(const Color *pSrc_pixels, uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr, uint32_t hg, uint32_t hb,
                                                 uint8_t sels[16], uint32_t cur_err) {
    uint32_t block_r[4], block_g[4], block_b[4];
    bc1_get_block_colors4(block_r, block_g, block_b, lr, lg, lb, hr, hg, hb);

    int dr = block_r[3] - block_r[0], dg = block_g[3] - block_g[0], db = block_b[3] - block_b[0];

    const float f = 4.0f / (float)(squarei(dr) + squarei(dg) + squarei(db) + .00000125f);

    uint32_t total_err = 0;

    for (uint32_t i = 0; i < 16; i++) {
        const int r = pSrc_pixels[i].r;
        const int g = pSrc_pixels[i].g;
        const int b = pSrc_pixels[i].b;

        int sel = (int)((float)((r - (int)block_r[0]) * dr + (g - (int)block_g[0]) * dg + (b - (int)block_b[0]) * db) * f + .5f);
        sel = clampi(sel, 1, 3);

        uint32_t err0 = squarei((int)block_r[sel - 1] - (int)r) + squarei((int)block_g[sel - 1] - (int)g) + squarei((int)block_b[sel - 1] - (int)b);
        uint32_t err1 = squarei((int)block_r[sel] - (int)r) + squarei((int)block_g[sel] - (int)g) + squarei((int)block_b[sel] - (int)b);

        int best_sel = sel;
        uint32_t best_err = err1;
        if (err0 == err1) {
            // Prefer non-interpolation
            if ((best_sel - 1) == 0) best_sel = 0;
        } else if (err0 < best_err) {
            best_sel = sel - 1;
            best_err = err0;
        }

        total_err += best_err;

        if (total_err >= cur_err) break;

        sels[i] = (uint8_t)best_sel;
    }
    return total_err;
}

static inline uint32_t bc1_find_sels4_fullerr(const Color *pSrc_pixels, uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr, uint32_t hg, uint32_t hb,
                                              uint8_t sels[16], uint32_t cur_err) {
    uint32_t block_r[4], block_g[4], block_b[4];
    bc1_get_block_colors4(block_r, block_g, block_b, lr, lg, lb, hr, hg, hb);

    uint32_t total_err = 0;

    for (uint32_t i = 0; i < 16; i++) {
        const int r = pSrc_pixels[i].r;
        const int g = pSrc_pixels[i].g;
        const int b = pSrc_pixels[i].b;

        uint32_t best_err = squarei((int)block_r[0] - (int)r) + squarei((int)block_g[0] - (int)g) + squarei((int)block_b[0] - (int)b);
        uint8_t best_sel = 0;

        for (uint32_t j = 1; (j < 4) && best_err; j++) {
            uint32_t err = squarei((int)block_r[j] - (int)r) + squarei((int)block_g[j] - (int)g) + squarei((int)block_b[j] - (int)b);
            if ((err < best_err) || ((err == best_err) && (j == 3))) {
                best_err = err;
                best_sel = (uint8_t)j;
            }
        }

        total_err += best_err;

        if (total_err >= cur_err) break;

        sels[i] = (uint8_t)best_sel;
    }
    return total_err;
}

static inline uint32_t bc1_find_sels4(uint32_t flags, const Color *pSrc_pixels, uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr, uint32_t hg, uint32_t hb,
                                      uint8_t sels[16], uint32_t cur_err) {
    uint32_t err;

    if (flags & cEncodeBC1UseFasterMSEEval)
        err = bc1_find_sels4_fasterr(pSrc_pixels, lr, lg, lb, hr, hg, hb, sels, cur_err);
    else if (flags & cEncodeBC1UseFullMSEEval)
        err = bc1_find_sels4_fullerr(pSrc_pixels, lr, lg, lb, hr, hg, hb, sels, cur_err);
    else
        err = bc1_find_sels4_check2_err(pSrc_pixels, lr, lg, lb, hr, hg, hb, sels, cur_err);

    return err;
}

static inline uint32_t bc1_find_sels3_fullerr(bool use_black, const Color *pSrc_pixels, uint32_t lr, uint32_t lg, uint32_t lb, uint32_t hr, uint32_t hg,
                                              uint32_t hb, uint8_t sels[16], uint32_t cur_err) {
    uint32_t block_r[3], block_g[3], block_b[3];
    bc1_get_block_colors3(block_r, block_g, block_b, lr, lg, lb, hr, hg, hb);

    uint32_t total_err = 0;

    for (uint32_t i = 0; i < 16; i++) {
        const int r = pSrc_pixels[i].r;
        const int g = pSrc_pixels[i].g;
        const int b = pSrc_pixels[i].b;

        uint32_t best_err = squarei((int)block_r[0] - (int)r) + squarei((int)block_g[0] - (int)g) + squarei((int)block_b[0] - (int)b);
        uint32_t best_sel = 0;

        uint32_t err1 = squarei((int)block_r[1] - (int)r) + squarei((int)block_g[1] - (int)g) + squarei((int)block_b[1] - (int)b);
        if (err1 < best_err) {
            best_err = err1;
            best_sel = 1;
        }

        uint32_t err2 = squarei((int)block_r[2] - (int)r) + squarei((int)block_g[2] - (int)g) + squarei((int)block_b[2] - (int)b);
        if (err2 < best_err) {
            best_err = err2;
            best_sel = 2;
        }

        if (use_black) {
            uint32_t err3 = squarei(r) + squarei(g) + squarei(b);
            if (err3 < best_err) {
                best_err = err3;
                best_sel = 3;
            }
        }

        total_err += best_err;
        if (total_err >= cur_err) return total_err;

        sels[i] = (uint8_t)best_sel;
    }

    return total_err;
}

static inline void precise_round_565(const vec3F &xl, const vec3F &xh, int &trial_lr, int &trial_lg, int &trial_lb, int &trial_hr, int &trial_hg,
                                     int &trial_hb) {
    trial_lr = (int)(xl.c[0] * 31.0f);
    trial_lg = (int)(xl.c[1] * 63.0f);
    trial_lb = (int)(xl.c[2] * 31.0f);

    trial_hr = (int)(xh.c[0] * 31.0f);
    trial_hg = (int)(xh.c[1] * 63.0f);
    trial_hb = (int)(xh.c[2] * 31.0f);

    if ((uint32_t)(trial_lr | trial_lb | trial_hr | trial_hb) > 31U) {
        trial_lr = ((uint32_t)trial_lr > 31U) ? (~trial_lr >> 31) & 31 : trial_lr;
        trial_hr = ((uint32_t)trial_hr > 31U) ? (~trial_hr >> 31) & 31 : trial_hr;

        trial_lb = ((uint32_t)trial_lb > 31U) ? (~trial_lb >> 31) & 31 : trial_lb;
        trial_hb = ((uint32_t)trial_hb > 31U) ? (~trial_hb >> 31) & 31 : trial_hb;
    }

    if ((uint32_t)(trial_lg | trial_hg) > 63U) {
        trial_lg = ((uint32_t)trial_lg > 63U) ? (~trial_lg >> 31) & 63 : trial_lg;
        trial_hg = ((uint32_t)trial_hg > 63U) ? (~trial_hg >> 31) & 63 : trial_hg;
    }

    trial_lr = (trial_lr + (xl.c[0] > g_midpoint5[trial_lr])) & 31;
    trial_lg = (trial_lg + (xl.c[1] > g_midpoint6[trial_lg])) & 63;
    trial_lb = (trial_lb + (xl.c[2] > g_midpoint5[trial_lb])) & 31;

    trial_hr = (trial_hr + (xh.c[0] > g_midpoint5[trial_hr])) & 31;
    trial_hg = (trial_hg + (xh.c[1] > g_midpoint6[trial_hg])) & 63;
    trial_hb = (trial_hb + (xh.c[2] > g_midpoint5[trial_hb])) & 31;
}

static inline void precise_round_565_noscale(vec3F xl, vec3F xh, int &trial_lr, int &trial_lg, int &trial_lb, int &trial_hr, int &trial_hg, int &trial_hb) {
    xl.c[0] *= 1.0f / 255.0f;
    xl.c[1] *= 1.0f / 255.0f;
    xl.c[2] *= 1.0f / 255.0f;

    xh.c[0] *= 1.0f / 255.0f;
    xh.c[1] *= 1.0f / 255.0f;
    xh.c[2] *= 1.0f / 255.0f;

    precise_round_565(xl, xh, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb);
}

static inline void bc1_encode4(BC1Block *pDst_block, int lr, int lg, int lb, int hr, int hg, int hb, const uint8_t sels[16]) {
    uint16_t lc16 = Color::Pack565Unscaled(lr, lg, lb);
    uint16_t hc16 = Color::Pack565Unscaled(hr, hg, hb);

    // Always forbid 3 color blocks
    if (lc16 == hc16) {
        uint8_t mask = 0;

        // Make l > h
        if (hc16 > 0)
            hc16--;
        else {
            // lc16 = hc16 = 0
            assert(lc16 == hc16 && hc16 == 0);

            hc16 = 0;
            lc16 = 1;
            mask = 0x55;  // select hc16
        }

        assert(lc16 > hc16);
        pDst_block->SetLowColor(static_cast<uint16_t>(lc16));
        pDst_block->SetHighColor(static_cast<uint16_t>(hc16));

        pDst_block->selectors[0] = mask;
        pDst_block->selectors[1] = mask;
        pDst_block->selectors[2] = mask;
        pDst_block->selectors[3] = mask;
    } else {
        uint8_t invert_mask = 0;
        if (lc16 < hc16) {
            std::swap(lc16, hc16);
            invert_mask = 0x55;
        }

        assert(lc16 > hc16);
        pDst_block->SetLowColor((uint16_t)lc16);
        pDst_block->SetHighColor((uint16_t)hc16);

        uint32_t packed_sels = 0;
        static const uint8_t s_sel_trans[4] = {0, 2, 3, 1};
        for (uint32_t i = 0; i < 16; i++) packed_sels |= ((uint32_t)s_sel_trans[sels[i]] << (i * 2));

        // todo: make this less silly to prevent packing and unpacking
        pDst_block->selectors[0] = (uint8_t)packed_sels ^ invert_mask;
        pDst_block->selectors[1] = (uint8_t)(packed_sels >> 8) ^ invert_mask;
        pDst_block->selectors[2] = (uint8_t)(packed_sels >> 16) ^ invert_mask;
        pDst_block->selectors[3] = (uint8_t)(packed_sels >> 24) ^ invert_mask;
    }
}

static inline void bc1_encode3(BC1Block *pDst_block, int lr, int lg, int lb, int hr, int hg, int hb, const uint8_t sels[16]) {
    uint16_t lc16 = Color::Pack565Unscaled(lr, lg, lb);
    uint16_t hc16 = Color::Pack565Unscaled(hr, hg, hb);

    bool invert_flag = false;
    if (lc16 > hc16) {
        std::swap(lc16, hc16);
        invert_flag = true;
    }

    assert(lc16 <= hc16);

    pDst_block->SetLowColor((uint16_t)lc16);
    pDst_block->SetHighColor((uint16_t)hc16);

    uint32_t packed_sels = 0;

    if (invert_flag) {
        static const uint8_t s_sel_trans_inv[4] = {1, 0, 2, 3};

        for (uint32_t i = 0; i < 16; i++) packed_sels |= ((uint32_t)s_sel_trans_inv[sels[i]] << (i * 2));
    } else {
        for (uint32_t i = 0; i < 16; i++) packed_sels |= ((uint32_t)sels[i] << (i * 2));
    }

    // todo: make this less silly to prevent packing and unpacking
    pDst_block->selectors[0] = (uint8_t)packed_sels;
    pDst_block->selectors[1] = (uint8_t)(packed_sels >> 8);
    pDst_block->selectors[2] = (uint8_t)(packed_sels >> 16);
    pDst_block->selectors[3] = (uint8_t)(packed_sels >> 24);
}

struct bc1_encode_results {
    int lr, lg, lb;
    int hr, hg, hb;
    uint8_t sels[16];
    bool m_3color;
};

static bool try_3color_block_useblack(const Color *pSrc_pixels, uint32_t flags, uint32_t &cur_err, bc1_encode_results &results) {
    int total_r = 0, total_g = 0, total_b = 0;
    int max_r = 0, max_g = 0, max_b = 0;
    int min_r = 255, min_g = 255, min_b = 255;
    int total_pixels = 0;
    for (uint32_t i = 0; i < 16; i++) {
        const int r = pSrc_pixels[i].r, g = pSrc_pixels[i].g, b = pSrc_pixels[i].b;
        if ((r | g | b) < 4) continue;

        max_r = std::max(max_r, r);
        max_g = std::max(max_g, g);
        max_b = std::max(max_b, b);
        min_r = std::min(min_r, r);
        min_g = std::min(min_g, g);
        min_b = std::min(min_b, b);
        total_r += r;
        total_g += g;
        total_b += b;

        total_pixels++;
    }

    if (!total_pixels) return false;

    int half_total_pixels = total_pixels >> 1;
    int avg_r = (total_r + half_total_pixels) / total_pixels;
    int avg_g = (total_g + half_total_pixels) / total_pixels;
    int avg_b = (total_b + half_total_pixels) / total_pixels;

    uint32_t low_c = 0, high_c = 0;

    int icov[6] = {0, 0, 0, 0, 0, 0};
    for (uint32_t i = 0; i < 16; i++) {
        int r = (int)pSrc_pixels[i].r;
        int g = (int)pSrc_pixels[i].g;
        int b = (int)pSrc_pixels[i].b;

        if ((r | g | b) < 4) continue;

        r -= avg_r;
        g -= avg_g;
        b -= avg_b;

        icov[0] += r * r;
        icov[1] += r * g;
        icov[2] += r * b;
        icov[3] += g * g;
        icov[4] += g * b;
        icov[5] += b * b;
    }

    float cov[6];
    for (uint32_t i = 0; i < 6; i++) cov[i] = (float)(icov[i]) * (1.0f / 255.0f);

    float xr = (float)(max_r - min_r);
    float xg = (float)(max_g - min_g);
    float xb = (float)(max_b - min_b);

    if (icov[2] < 0) xr = -xr;

    if (icov[4] < 0) xg = -xg;

    for (uint32_t power_iter = 0; power_iter < 4; power_iter++) {
        float r = xr * cov[0] + xg * cov[1] + xb * cov[2];
        float g = xr * cov[1] + xg * cov[3] + xb * cov[4];
        float b = xr * cov[2] + xg * cov[4] + xb * cov[5];
        xr = r;
        xg = g;
        xb = b;
    }

    float k = maximum(fabsf(xr), fabsf(xg), fabsf(xb));
    int saxis_r = 306, saxis_g = 601, saxis_b = 117;
    if (k >= 2) {
        float m = 1024.0f / k;
        saxis_r = (int)(xr * m);
        saxis_g = (int)(xg * m);
        saxis_b = (int)(xb * m);
    }

    int low_dot = INT_MAX, high_dot = INT_MIN;
    for (uint32_t i = 0; i < 16; i++) {
        int r = (int)pSrc_pixels[i].r, g = (int)pSrc_pixels[i].g, b = (int)pSrc_pixels[i].b;

        if ((r | g | b) < 4) continue;

        int dot = r * saxis_r + g * saxis_g + b * saxis_b;
        if (dot < low_dot) {
            low_dot = dot;
            low_c = i;
        }
        if (dot > high_dot) {
            high_dot = dot;
            high_c = i;
        }
    }

    int lr = scale8To5(pSrc_pixels[low_c].r);
    int lg = scale8To6(pSrc_pixels[low_c].g);
    int lb = scale8To5(pSrc_pixels[low_c].b);

    int hr = scale8To5(pSrc_pixels[high_c].r);
    int hg = scale8To6(pSrc_pixels[high_c].g);
    int hb = scale8To5(pSrc_pixels[high_c].b);

    uint8_t trial_sels[16];
    uint32_t trial_err = bc1_find_sels3_fullerr(true, pSrc_pixels, lr, lg, lb, hr, hg, hb, trial_sels, UINT32_MAX);

    if (trial_err) {
        const uint32_t total_ls_passes = flags & cEncodeBC1TwoLeastSquaresPasses ? 2 : 1;
        for (uint32_t trials = 0; trials < total_ls_passes; trials++) {
            vec3F xl, xh;
            int lr2, lg2, lb2, hr2, hg2, hb2;
            if (!compute_least_squares_endpoints3_rgb(true, pSrc_pixels, trial_sels, &xl, &xh)) {
                lr2 = g_bc1_match5_half[avg_r].m_hi;
                lg2 = g_bc1_match6_half[avg_g].m_hi;
                lb2 = g_bc1_match5_half[avg_b].m_hi;

                hr2 = g_bc1_match5_half[avg_r].m_lo;
                hg2 = g_bc1_match6_half[avg_g].m_lo;
                hb2 = g_bc1_match5_half[avg_b].m_lo;
            } else {
                precise_round_565(xl, xh, hr2, hg2, hb2, lr2, lg2, lb2);
            }

            if ((lr == lr2) && (lg == lg2) && (lb == lb2) && (hr == hr2) && (hg == hg2) && (hb == hb2)) break;

            uint8_t trial_sels2[16];
            uint32_t trial_err2 = bc1_find_sels3_fullerr(true, pSrc_pixels, lr2, lg2, lb2, hr2, hg2, hb2, trial_sels2, trial_err);

            if (trial_err2 < trial_err) {
                trial_err = trial_err2;
                lr = lr2;
                lg = lg2;
                lb = lb2;
                hr = hr2;
                hg = hg2;
                hb = hb2;
                memcpy(trial_sels, trial_sels2, sizeof(trial_sels));
            } else
                break;
        }
    }

    if (trial_err < cur_err) {
        results.m_3color = true;
        results.lr = lr;
        results.lg = lg;
        results.lb = lb;
        results.hr = hr;
        results.hg = hg;
        results.hb = hb;
        memcpy(results.sels, trial_sels, 16);

        cur_err = trial_err;

        return true;
    }

    return false;
}

static bool try_3color_block(const Color *pSrc_pixels, uint32_t flags, uint32_t &cur_err, int avg_r, int avg_g, int avg_b, int lr, int lg, int lb, int hr,
                             int hg, int hb, int total_r, int total_g, int total_b, uint32_t total_orderings_to_try, bc1_encode_results &results) {
    uint8_t trial_sels[16];
    uint32_t trial_err = bc1_find_sels3_fullerr(false, pSrc_pixels, lr, lg, lb, hr, hg, hb, trial_sels, UINT32_MAX);

    if (trial_err) {
        const uint32_t total_ls_passes = flags & cEncodeBC1TwoLeastSquaresPasses ? 2 : 1;
        for (uint32_t trials = 0; trials < total_ls_passes; trials++) {
            vec3F xl, xh;
            int lr2, lg2, lb2, hr2, hg2, hb2;
            if (!compute_least_squares_endpoints3_rgb(false, pSrc_pixels, trial_sels, &xl, &xh)) {
                lr2 = g_bc1_match5_half[avg_r].m_hi;
                lg2 = g_bc1_match6_half[avg_g].m_hi;
                lb2 = g_bc1_match5_half[avg_b].m_hi;

                hr2 = g_bc1_match5_half[avg_r].m_lo;
                hg2 = g_bc1_match6_half[avg_g].m_lo;
                hb2 = g_bc1_match5_half[avg_b].m_lo;
            } else {
                precise_round_565(xl, xh, hr2, hg2, hb2, lr2, lg2, lb2);
            }

            if ((lr == lr2) && (lg == lg2) && (lb == lb2) && (hr == hr2) && (hg == hg2) && (hb == hb2)) break;

            uint8_t trial_sels2[16];
            uint32_t trial_err2 = bc1_find_sels3_fullerr(false, pSrc_pixels, lr2, lg2, lb2, hr2, hg2, hb2, trial_sels2, trial_err);

            if (trial_err2 < trial_err) {
                trial_err = trial_err2;
                lr = lr2;
                lg = lg2;
                lb = lb2;
                hr = hr2;
                hg = hg2;
                hb = hb2;
                memcpy(trial_sels, trial_sels2, sizeof(trial_sels));
            } else
                break;
        }
    }

    if ((trial_err) && (flags & cEncodeBC1UseLikelyTotalOrderings) && (total_orderings_to_try)) {
        hist3 h;
        for (uint32_t i = 0; i < 16; i++) {
            assert(trial_sels[i] < 3);
            h.m_hist[trial_sels[i]]++;
        }

        const uint32_t orig_total_order_index = h.lookup_total_ordering_index();

        int r0, g0, b0, r3, g3, b3;
        r0 = (lr << 3) | (lr >> 2);
        g0 = (lg << 2) | (lg >> 4);
        b0 = (lb << 3) | (lb >> 2);
        r3 = (hr << 3) | (hr >> 2);
        g3 = (hg << 2) | (hg >> 4);
        b3 = (hb << 3) | (hb >> 2);

        int ar = r3 - r0, ag = g3 - g0, ab = b3 - b0;

        int dots[16];
        for (uint32_t i = 0; i < 16; i++) {
            int r = pSrc_pixels[i].r;
            int g = pSrc_pixels[i].g;
            int b = pSrc_pixels[i].b;
            int d = 0x1000000 + (r * ar + g * ag + b * ab);
            assert(d >= 0);
            dots[i] = (d << 4) + i;
        }

        std::sort(dots, dots + 16);

        uint32_t r_sum[17], g_sum[17], b_sum[17];
        uint32_t r = 0, g = 0, b = 0;
        for (uint32_t i = 0; i < 16; i++) {
            const uint32_t p = dots[i] & 15;

            r_sum[i] = r;
            g_sum[i] = g;
            b_sum[i] = b;

            r += pSrc_pixels[p].r;
            g += pSrc_pixels[p].g;
            b += pSrc_pixels[p].b;
        }

        r_sum[16] = total_r;
        g_sum[16] = total_g;
        b_sum[16] = total_b;

        const uint32_t q_total = (flags & cEncodeBC1Exhaustive) ? NUM_UNIQUE_TOTAL_ORDERINGS3 : std::min(total_orderings_to_try, MAX_TOTAL_ORDERINGS3);
        for (uint32_t q = 0; q < q_total; q++) {
            const uint32_t s = (flags & cEncodeBC1Exhaustive) ? q : g_best_total_orderings3[orig_total_order_index][q];

            int trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb;

            vec3F xl, xh;

            if ((s == TOTAL_ORDER_3_0_16) || (s == TOTAL_ORDER_3_1_16) || (s == TOTAL_ORDER_3_2_16)) {
                trial_lr = g_bc1_match5_half[avg_r].m_hi;
                trial_lg = g_bc1_match6_half[avg_g].m_hi;
                trial_lb = g_bc1_match5_half[avg_b].m_hi;

                trial_hr = g_bc1_match5_half[avg_r].m_lo;
                trial_hg = g_bc1_match6_half[avg_g].m_lo;
                trial_hb = g_bc1_match5_half[avg_b].m_lo;
            } else {
                compute_least_squares_endpoints3_rgb(&xl, &xh, total_r, total_g, total_b, g_selector_factors3[s][0], g_selector_factors3[s][1],
                                                     g_selector_factors3[s][2], s, r_sum, g_sum, b_sum);

                precise_round_565(xl, xh, trial_hr, trial_hg, trial_hb, trial_lr, trial_lg, trial_lb);
            }

            uint8_t trial_sels2[16];
            uint32_t trial_err2 =
                bc1_find_sels3_fullerr(false, pSrc_pixels, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, trial_sels2, UINT32_MAX);

            if (trial_err2 < trial_err) {
                trial_err = trial_err2;

                lr = trial_lr;
                lg = trial_lg;
                lb = trial_lb;

                hr = trial_hr;
                hg = trial_hg;
                hb = trial_hb;

                memcpy(trial_sels, trial_sels2, sizeof(trial_sels));
            }

        }  // s
    }

    if (trial_err < cur_err) {
        results.m_3color = true;
        results.lr = lr;
        results.lg = lg;
        results.lb = lb;
        results.hr = hr;
        results.hg = hg;
        results.hb = hb;
        memcpy(results.sels, trial_sels, 16);

        cur_err = trial_err;

        return true;
    }

    return false;
}

void encode_bc1(uint32_t level, void *pDst, const uint8_t *pPixels, bool allow_3color, bool allow_transparent_texels_for_black) {
    uint32_t flags = 0, total_orderings4 = 1, total_orderings3 = 1;

    static_assert(MAX_TOTAL_ORDERINGS3 >= 32, "MAX_TOTAL_ORDERINGS3 >= 32");
    static_assert(MAX_TOTAL_ORDERINGS4 >= 32, "MAX_TOTAL_ORDERINGS4 >= 32");

    switch (level) {
        case 0:
            // Faster/higher quality than stb_dxt default.
            flags = cEncodeBC1BoundingBoxInt;
            break;
        case 1:
            // Faster/higher quality than stb_dxt default. a bit higher average quality vs. mode 0.
            flags = cEncodeBC1Use2DLS;
            break;
        case 2:
            // On average mode 2 is a little weaker than modes 0/1, but it's stronger on outliers (very tough textures).
            // Slightly stronger than stb_dxt.
            flags = 0;
            break;
        case 3:
            // Slightly stronger than stb_dxt HIGHQUAL.
            flags = cEncodeBC1TwoLeastSquaresPasses;
            break;
        case 4:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1Use6PowerIters;
            break;
        default:
        case 5:
            // stb_dxt HIGHQUAL + permit 3 color (if it's enabled).
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFasterMSEEval;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            break;
        case 6:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFasterMSEEval | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            break;
        case 7:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFasterMSEEval | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 4;
            break;
        case 8:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFasterMSEEval | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 8;
            break;
        case 9:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 11;
            total_orderings3 = 3;
            break;
        case 10:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 20;
            total_orderings3 = 8;
            break;
        case 11:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 28;
            total_orderings3 = 16;
            break;
        case 12:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 32;
            total_orderings3 = 32;
            break;
        case 13:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    (20 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 32;
            total_orderings3 = 32;
            break;
        case 14:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    (32 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 32;
            total_orderings3 = 32;
            break;
        case 15:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    (32 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = ((((32 + MAX_TOTAL_ORDERINGS4) / 2) + 32) / 2);
            total_orderings3 = 32;
            break;
        case 16:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    (256 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = (32 + MAX_TOTAL_ORDERINGS4) / 2;
            total_orderings3 = 32;
            break;
        case 17:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    (256 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = MAX_TOTAL_ORDERINGS4;
            total_orderings3 = 32;
            break;
        case 18:
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    cEncodeBC1Iterative | (256 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = MAX_TOTAL_ORDERINGS4;
            total_orderings3 = 32;
            break;
        case 19:
            // This hidden mode is *extremely* slow and abuses the encoder. It's just for testing/training.
            flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use6PowerIters |
                    cEncodeBC1Exhaustive | cEncodeBC1Iterative | (256 << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
            flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) | (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
            total_orderings4 = 32;
            total_orderings3 = 32;
            break;
    }

    encode_bc1(pDst, pPixels, flags, total_orderings4, total_orderings3);
}

// Finds low and high colors to begin with
static inline void encode_bc1_pick_initial(const Color *pSrc_pixels, uint32_t flags, bool grayscale_flag, int min_r, int min_g, int min_b, int max_r,
                                           int max_g, int max_b, int avg_r, int avg_g, int avg_b, int total_r, int total_g, int total_b, int &lr, int &lg,
                                           int &lb, int &hr, int &hg, int &hb) {
    if (grayscale_flag) {
        const int fr = pSrc_pixels[0].r;

        // Grayscale blocks are a common enough case to specialize.
        if ((max_r - min_r) < 2) {
            lr = lb = hr = hb = scale8To5(fr);
            lg = hg = scale8To6(fr);
        } else {
            lr = lb = scale8To5(min_r);
            lg = scale8To6(min_r);

            hr = hb = scale8To5(max_r);
            hg = scale8To6(max_r);
        }
    } else if (flags & cEncodeBC1Use2DLS) {
        //  2D Least Squares approach from Humus's example, with added inset and optimal rounding.
        int big_chan = 0, min_chan_val = min_r, max_chan_val = max_r;
        if ((max_g - min_g) > (max_chan_val - min_chan_val)) big_chan = 1, min_chan_val = min_g, max_chan_val = max_g;

        if ((max_b - min_b) > (max_chan_val - min_chan_val)) big_chan = 2, min_chan_val = min_b, max_chan_val = max_b;

        int sum_xy_r = 0, sum_xy_g = 0, sum_xy_b = 0;
        vec3F l, h;
        if (big_chan == 0) {
            for (uint32_t i = 0; i < 16; i++) {
                const int r = pSrc_pixels[i].r, g = pSrc_pixels[i].g, b = pSrc_pixels[i].b;
                sum_xy_r += r * r, sum_xy_g += r * g, sum_xy_b += r * b;
            }

            int sum_x = total_r;
            int sum_x2 = sum_xy_r;

            float div = (float)(16 * sum_x2 - sum_x * sum_x);
            float b_y = 0.0f, b_z = 0.0f;
            if (fabs(div) > 1e-8f) {
                div = 1.0f / div;
                b_y = (16 * sum_xy_g - sum_x * total_g) * div;
                b_z = (16 * sum_xy_b - sum_x * total_b) * div;
            }

            float a_y = (total_g - b_y * sum_x) / 16.0f;
            float a_z = (total_b - b_z * sum_x) / 16.0f;

            l.c[1] = a_y + b_y * min_chan_val;
            l.c[2] = a_z + b_z * min_chan_val;

            h.c[1] = a_y + b_y * max_chan_val;
            h.c[2] = a_z + b_z * max_chan_val;

            float dg = (h.c[1] - l.c[1]);
            float db = (h.c[2] - l.c[2]);

            h.c[1] = l.c[1] + dg * (15.0f / 16.0f);
            h.c[2] = l.c[2] + db * (15.0f / 16.0f);

            l.c[1] = l.c[1] + dg * (1.0f / 16.0f);
            l.c[2] = l.c[2] + db * (1.0f / 16.0f);

            float d = (float)(max_chan_val - min_chan_val);
            float fmin_chan_val = min_chan_val + d * (1.0f / 16.0f);
            float fmax_chan_val = min_chan_val + d * (15.0f / 16.0f);

            l.c[0] = fmin_chan_val;
            h.c[0] = fmax_chan_val;
        } else if (big_chan == 1) {
            for (uint32_t i = 0; i < 16; i++) {
                const int r = pSrc_pixels[i].r, g = pSrc_pixels[i].g, b = pSrc_pixels[i].b;
                sum_xy_r += g * r, sum_xy_g += g * g, sum_xy_b += g * b;
            }

            int sum_x = total_g;
            int sum_x2 = sum_xy_g;

            float div = (float)(16 * sum_x2 - sum_x * sum_x);
            float b_x = 0.0f, b_z = 0.0f;
            if (fabs(div) > 1e-8f) {
                div = 1.0f / div;
                b_x = (16 * sum_xy_r - sum_x * total_r) * div;
                b_z = (16 * sum_xy_b - sum_x * total_b) * div;
            }

            float a_x = (total_r - b_x * sum_x) / 16.0f;
            float a_z = (total_b - b_z * sum_x) / 16.0f;

            l.c[0] = a_x + b_x * min_chan_val;
            l.c[2] = a_z + b_z * min_chan_val;

            h.c[0] = a_x + b_x * max_chan_val;
            h.c[2] = a_z + b_z * max_chan_val;

            float dr = (h.c[0] - l.c[0]);
            float db = (h.c[2] - l.c[2]);

            h.c[0] = l.c[0] + dr * (15.0f / 16.0f);
            h.c[2] = l.c[2] + db * (15.0f / 16.0f);

            l.c[0] = l.c[0] + dr * (1.0f / 16.0f);
            l.c[2] = l.c[2] + db * (1.0f / 16.0f);

            float d = (float)(max_chan_val - min_chan_val);
            float fmin_chan_val = min_chan_val + d * (1.0f / 16.0f);
            float fmax_chan_val = min_chan_val + d * (15.0f / 16.0f);

            l.c[1] = fmin_chan_val;
            h.c[1] = fmax_chan_val;
        } else {
            for (uint32_t i = 0; i < 16; i++) {
                const int r = pSrc_pixels[i].r, g = pSrc_pixels[i].g, b = pSrc_pixels[i].b;
                sum_xy_r += b * r, sum_xy_g += b * g, sum_xy_b += b * b;
            }

            int sum_x = total_b;
            int sum_x2 = sum_xy_b;

            float div = (float)(16 * sum_x2 - sum_x * sum_x);
            float b_x = 0.0f, b_y = 0.0f;
            if (fabs(div) > 1e-8f) {
                div = 1.0f / div;
                b_x = (16 * sum_xy_r - sum_x * total_r) * div;
                b_y = (16 * sum_xy_g - sum_x * total_g) * div;
            }

            float a_x = (total_r - b_x * sum_x) / 16.0f;
            float a_y = (total_g - b_y * sum_x) / 16.0f;

            l.c[0] = a_x + b_x * min_chan_val;
            l.c[1] = a_y + b_y * min_chan_val;

            h.c[0] = a_x + b_x * max_chan_val;
            h.c[1] = a_y + b_y * max_chan_val;

            float dr = (h.c[0] - l.c[0]);
            float dg = (h.c[1] - l.c[1]);

            h.c[0] = l.c[0] + dr * (15.0f / 16.0f);
            h.c[1] = l.c[1] + dg * (15.0f / 16.0f);

            l.c[0] = l.c[0] + dr * (1.0f / 16.0f);
            l.c[1] = l.c[1] + dg * (1.0f / 16.0f);

            float d = (float)(max_chan_val - min_chan_val);
            float fmin_chan_val = min_chan_val + d * (1.0f / 16.0f);
            float fmax_chan_val = min_chan_val + d * (15.0f / 16.0f);

            l.c[2] = fmin_chan_val;
            h.c[2] = fmax_chan_val;
        }

        precise_round_565_noscale(l, h, lr, lg, lb, hr, hg, hb);
    } else if (flags & cEncodeBC1BoundingBox) {
        // Algorithm from icbc.h compress_dxt1_fast()
        vec3F l, h;
        l.c[0] = min_r * (1.0f / 255.0f);
        l.c[1] = min_g * (1.0f / 255.0f);
        l.c[2] = min_b * (1.0f / 255.0f);

        h.c[0] = max_r * (1.0f / 255.0f);
        h.c[1] = max_g * (1.0f / 255.0f);
        h.c[2] = max_b * (1.0f / 255.0f);

        const float bias = 8.0f / 255.0f;
        float inset_r = (h.c[0] - l.c[0] - bias) * (1.0f / 16.0f);
        float inset_g = (h.c[1] - l.c[1] - bias) * (1.0f / 16.0f);
        float inset_b = (h.c[2] - l.c[2] - bias) * (1.0f / 16.0f);

        l.c[0] = clampf(l.c[0] + inset_r, 0.0f, 1.0f);
        l.c[1] = clampf(l.c[1] + inset_g, 0.0f, 1.0f);
        l.c[2] = clampf(l.c[2] + inset_b, 0.0f, 1.0f);

        h.c[0] = clampf(h.c[0] - inset_r, 0.0f, 1.0f);
        h.c[1] = clampf(h.c[1] - inset_g, 0.0f, 1.0f);
        h.c[2] = clampf(h.c[2] - inset_b, 0.0f, 1.0f);

        int icov_xz = 0, icov_yz = 0;
        for (uint32_t i = 0; i < 16; i++) {
            int r = (int)pSrc_pixels[i].r - avg_r;
            int g = (int)pSrc_pixels[i].g - avg_g;
            int b = (int)pSrc_pixels[i].b - avg_b;
            icov_xz += r * b;
            icov_yz += g * b;
        }

        if (icov_xz < 0) std::swap(l.c[0], h.c[0]);

        if (icov_yz < 0) std::swap(l.c[1], h.c[1]);

        precise_round_565(l, h, lr, lg, lb, hr, hg, hb);
    } else if (flags & cEncodeBC1BoundingBoxInt) {
        // Algorithm from icbc.h compress_dxt1_fast(), but converted to integer.
        int inset_r = (max_r - min_r - 8) >> 4;
        int inset_g = (max_g - min_g - 8) >> 4;
        int inset_b = (max_b - min_b - 8) >> 4;

        min_r += inset_r;
        min_g += inset_g;
        min_b += inset_b;
        if ((uint32_t)(min_r | min_g | min_b) > 255U) {
            min_r = clampi(min_r, 0, 255);
            min_g = clampi(min_g, 0, 255);
            min_b = clampi(min_b, 0, 255);
        }

        max_r -= inset_r;
        max_g -= inset_g;
        max_b -= inset_b;
        if ((uint32_t)(max_r | max_g | max_b) > 255U) {
            max_r = clampi(max_r, 0, 255);
            max_g = clampi(max_g, 0, 255);
            max_b = clampi(max_b, 0, 255);
        }

        int icov_xz = 0, icov_yz = 0;
        for (uint32_t i = 0; i < 16; i++) {
            int r = (int)pSrc_pixels[i].r - avg_r;
            int g = (int)pSrc_pixels[i].g - avg_g;
            int b = (int)pSrc_pixels[i].b - avg_b;
            icov_xz += r * b;
            icov_yz += g * b;
        }

        int x0 = min_r;
        int y0 = min_g;
        int x1 = max_r;
        int y1 = max_g;

        // swap r and g min and max to align principal axis
        if (icov_xz < 0) std::swap(x0, x1);

        if (icov_yz < 0) std::swap(y0, y1);

        lr = scale8To5(x0);
        lg = scale8To6(y0);
        lb = scale8To5(min_b);

        hr = scale8To5(x1);
        hg = scale8To6(y1);
        hb = scale8To5(max_b);
    } else {
        // Select 2 colors along the principle axis. (There must be a faster/simpler way.)
        uint32_t low_c = 0, high_c = 0;

        int icov[6] = {0, 0, 0, 0, 0, 0};
        for (uint32_t i = 0; i < 16; i++) {
            int r = (int)pSrc_pixels[i].r - avg_r;
            int g = (int)pSrc_pixels[i].g - avg_g;
            int b = (int)pSrc_pixels[i].b - avg_b;
            icov[0] += r * r;
            icov[1] += r * g;
            icov[2] += r * b;
            icov[3] += g * g;
            icov[4] += g * b;
            icov[5] += b * b;
        }

        int saxis_r = 306, saxis_g = 601, saxis_b = 117;

        float xr = (float)(max_r - min_r);
        float xg = (float)(max_g - min_g);
        float xb = (float)(max_b - min_b);

        if (icov[2] < 0) xr = -xr;

        if (icov[4] < 0) xg = -xg;

        float cov[6];
        for (uint32_t i = 0; i < 6; i++) cov[i] = (float)(icov[i]) * (1.0f / 255.0f);

        const uint32_t total_power_iters = (flags & cEncodeBC1Use6PowerIters) ? 6 : 4;
        for (uint32_t power_iter = 0; power_iter < total_power_iters; power_iter++) {
            float r = xr * cov[0] + xg * cov[1] + xb * cov[2];
            float g = xr * cov[1] + xg * cov[3] + xb * cov[4];
            float b = xr * cov[2] + xg * cov[4] + xb * cov[5];
            xr = r;
            xg = g;
            xb = b;
        }

        float k = maximum(fabsf(xr), fabsf(xg), fabsf(xb));
        if (k >= 2) {
            float m = 2048.0f / k;
            saxis_r = (int)(xr * m);
            saxis_g = (int)(xg * m);
            saxis_b = (int)(xb * m);
        }

        int low_dot = INT_MAX, high_dot = INT_MIN;

        saxis_r = (int)((uint32_t)saxis_r << 4U);
        saxis_g = (int)((uint32_t)saxis_g << 4U);
        saxis_b = (int)((uint32_t)saxis_b << 4U);

        for (uint32_t i = 0; i < 16; i += 4) {
            int dot0 = ((pSrc_pixels[i].r * saxis_r + pSrc_pixels[i].g * saxis_g + pSrc_pixels[i].b * saxis_b) & ~0xF) + i;
            int dot1 = ((pSrc_pixels[i + 1].r * saxis_r + pSrc_pixels[i + 1].g * saxis_g + pSrc_pixels[i + 1].b * saxis_b) & ~0xF) + i + 1;
            int dot2 = ((pSrc_pixels[i + 2].r * saxis_r + pSrc_pixels[i + 2].g * saxis_g + pSrc_pixels[i + 2].b * saxis_b) & ~0xF) + i + 2;
            int dot3 = ((pSrc_pixels[i + 3].r * saxis_r + pSrc_pixels[i + 3].g * saxis_g + pSrc_pixels[i + 3].b * saxis_b) & ~0xF) + i + 3;

            int min_d01 = std::min(dot0, dot1);
            int max_d01 = std::max(dot0, dot1);

            int min_d23 = std::min(dot2, dot3);
            int max_d23 = std::max(dot2, dot3);

            int min_d = std::min(min_d01, min_d23);
            int max_d = std::max(max_d01, max_d23);

            low_dot = std::min(low_dot, min_d);
            high_dot = std::max(high_dot, max_d);
        }
        low_c = low_dot & 15;
        high_c = high_dot & 15;

        lr = scale8To5(pSrc_pixels[low_c].r);
        lg = scale8To6(pSrc_pixels[low_c].g);
        lb = scale8To5(pSrc_pixels[low_c].b);

        hr = scale8To5(pSrc_pixels[high_c].r);
        hg = scale8To6(pSrc_pixels[high_c].g);
        hb = scale8To5(pSrc_pixels[high_c].b);
    }
}

static const int8_t s_adjacent_voxels[16][4] = {
    {1, 0, 0, 3},    // 0
    {0, 1, 0, 4},    // 1
    {0, 0, 1, 5},    // 2
    {-1, 0, 0, 0},   // 3
    {0, -1, 0, 1},   // 4
    {0, 0, -1, 2},   // 5
    {1, 1, 0, 9},    // 6
    {1, 0, 1, 10},   // 7
    {0, 1, 1, 11},   // 8
    {-1, -1, 0, 6},  // 9
    {-1, 0, -1, 7},  // 10
    {0, -1, -1, 8},  // 11
    {-1, 1, 0, 13},  // 12
    {1, -1, 0, 12},  // 13
    {0, -1, 1, 15},  // 14
    {0, 1, -1, 14},  // 15
};

// From icbc's high quality mode.
static inline void encode_bc1_endpoint_search(const Color *pSrc_pixels, bool any_black_pixels, uint32_t flags, bc1_encode_results &results,
                                              uint32_t cur_err) {
    int &lr = results.lr, &lg = results.lg, &lb = results.lb, &hr = results.hr, &hg = results.hg, &hb = results.hb;
    uint8_t *sels = results.sels;

    int prev_improvement_index = 0, forbidden_direction = -1;

    const int endpoint_search_rounds = (flags & cEncodeBC1EndpointSearchRoundsMask) >> cEncodeBC1EndpointSearchRoundsShift;
    for (int i = 0; i < endpoint_search_rounds; i++) {
        assert(s_adjacent_voxels[s_adjacent_voxels[i & 15][3]][3] == (i & 15));

        if (forbidden_direction == (i & 31)) continue;

        const int8_t delta[3] = {s_adjacent_voxels[i & 15][0], s_adjacent_voxels[i & 15][1], s_adjacent_voxels[i & 15][2]};

        int trial_lr = lr, trial_lg = lg, trial_lb = lb, trial_hr = hr, trial_hg = hg, trial_hb = hb;

        if ((i >> 4) & 1) {
            trial_lr = clampi(trial_lr + delta[0], 0, 31);
            trial_lg = clampi(trial_lg + delta[1], 0, 63);
            trial_lb = clampi(trial_lb + delta[2], 0, 31);
        } else {
            trial_hr = clampi(trial_hr + delta[0], 0, 31);
            trial_hg = clampi(trial_hg + delta[1], 0, 63);
            trial_hb = clampi(trial_hb + delta[2], 0, 31);
        }

        uint8_t trial_sels[16];

        uint32_t trial_err;
        if (results.m_3color) {
            trial_err = bc1_find_sels3_fullerr(((any_black_pixels) && ((flags & cEncodeBC1Use3ColorBlocksForBlackPixels) != 0)), pSrc_pixels, trial_lr,
                                               trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, trial_sels, cur_err);
        } else {
            trial_err = bc1_find_sels4(flags, pSrc_pixels, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, trial_sels, cur_err);
        }

        if (trial_err < cur_err) {
            cur_err = trial_err;

            forbidden_direction = s_adjacent_voxels[i & 15][3] | (i & 16);

            lr = trial_lr, lg = trial_lg, lb = trial_lb, hr = trial_hr, hg = trial_hg, hb = trial_hb;

            memcpy(sels, trial_sels, 16);

            prev_improvement_index = i;
        }

        if (i - prev_improvement_index > 32) break;
    }
}

void encode_bc1(void *pDst, const uint8_t *pPixels, uint32_t flags, uint32_t total_orderings_to_try, uint32_t total_orderings_to_try3) {
    assert(g_initialized);

    const Color *pSrc_pixels = (const Color *)pPixels;
    BC1Block *pDst_block = static_cast<BC1Block *>(pDst);

    int avg_r, avg_g, avg_b, min_r, min_g, min_b, max_r, max_g, max_b;

    const uint32_t fr = pSrc_pixels[0].r, fg = pSrc_pixels[0].g, fb = pSrc_pixels[0].b;

    uint32_t j;
    for (j = 15; j >= 1; --j)
        if ((pSrc_pixels[j].r != fr) || (pSrc_pixels[j].g != fg) || (pSrc_pixels[j].b != fb)) break;

    if (j == 0) {
        encode_bc1_solid_block(pDst, fr, fg, fb, (flags & (cEncodeBC1Use3ColorBlocks | cEncodeBC1Use3ColorBlocksForBlackPixels)) != 0);
        return;
    }

    int total_r = fr, total_g = fg, total_b = fb;

    max_r = fr, max_g = fg, max_b = fb;
    min_r = fr, min_g = fg, min_b = fb;

    uint32_t grayscale_flag = (fr == fg) && (fr == fb);
    uint32_t any_black_pixels = (fr | fg | fb) < 4;

    for (uint32_t i = 1; i < 16; i++) {
        const int r = pSrc_pixels[i].r, g = pSrc_pixels[i].g, b = pSrc_pixels[i].b;

        grayscale_flag &= ((r == g) && (r == b));
        any_black_pixels |= ((r | g | b) < 4);

        max_r = std::max(max_r, r);
        max_g = std::max(max_g, g);
        max_b = std::max(max_b, b);
        min_r = std::min(min_r, r);
        min_g = std::min(min_g, g);
        min_b = std::min(min_b, b);
        total_r += r;
        total_g += g;
        total_b += b;
    }

    avg_r = (total_r + 8) >> 4, avg_g = (total_g + 8) >> 4, avg_b = (total_b + 8) >> 4;

    bc1_encode_results results;
    results.m_3color = false;

    uint8_t *sels = results.sels;
    int &lr = results.lr, &lg = results.lg, &lb = results.lb, &hr = results.hr, &hg = results.hg, &hb = results.hb;
    int orig_lr = 0, orig_lg = 0, orig_lb = 0, orig_hr = 0, orig_hg = 0, orig_hb = 0;

    lr = 0, lg = 0, lb = 0, hr = 0, hg = 0, hb = 0;

    const bool needs_block_error =
        ((flags & (cEncodeBC1UseLikelyTotalOrderings | cEncodeBC1Use3ColorBlocks | cEncodeBC1UseFullMSEEval | cEncodeBC1EndpointSearchRoundsMask)) != 0) ||
        (any_black_pixels && ((flags & cEncodeBC1Use3ColorBlocksForBlackPixels) != 0));

    uint32_t cur_err = UINT32_MAX;

    if (!needs_block_error) {
        assert((flags & cEncodeBC1TryAllInitialEndponts) == 0);

        encode_bc1_pick_initial(pSrc_pixels, flags, grayscale_flag != 0, min_r, min_g, min_b, max_r, max_g, max_b, avg_r, avg_g, avg_b, total_r, total_g,
                                total_b, lr, lg, lb, hr, hg, hb);

        orig_lr = lr, orig_lg = lg, orig_lb = lb, orig_hr = hr, orig_hg = hg, orig_hb = hb;

        bc1_find_sels4_noerr(pSrc_pixels, lr, lg, lb, hr, hg, hb, sels);

        const uint32_t total_ls_passes = flags & cEncodeBC1TwoLeastSquaresPasses ? 2 : 1;
        for (uint32_t ls_pass = 0; ls_pass < total_ls_passes; ls_pass++) {
            int trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb;

            vec3F xl, xh;
            if (!compute_least_squares_endpoints4_rgb(pSrc_pixels, sels, &xl, &xh, total_r, total_g, total_b)) {
                // All selectors equal - treat it as a solid block which should always be equal or better.
                trial_lr = g_bc1_match5_equals_1[avg_r].m_hi;
                trial_lg = g_bc1_match6_equals_1[avg_g].m_hi;
                trial_lb = g_bc1_match5_equals_1[avg_b].m_hi;

                trial_hr = g_bc1_match5_equals_1[avg_r].m_lo;
                trial_hg = g_bc1_match6_equals_1[avg_g].m_lo;
                trial_hb = g_bc1_match5_equals_1[avg_b].m_lo;

                // In high/higher quality mode, let it try again in case the optimal tables have caused the sels to diverge.
            } else {
                precise_round_565(xl, xh, trial_hr, trial_hg, trial_hb, trial_lr, trial_lg, trial_lb);
            }

            if ((lr == trial_lr) && (lg == trial_lg) && (lb == trial_lb) && (hr == trial_hr) && (hg == trial_hg) && (hb == trial_hb)) break;

            bc1_find_sels4_noerr(pSrc_pixels, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, sels);

            lr = trial_lr;
            lg = trial_lg;
            lb = trial_lb;
            hr = trial_hr;
            hg = trial_hg;
            hb = trial_hb;

        }  // ls_pass
    } else {
        // calculate block error from naïve approach
        const uint32_t total_rounds = (flags & cEncodeBC1TryAllInitialEndponts) ? 2 : 1;
        for (uint32_t round = 0; round < total_rounds; round++) {
            uint32_t modified_flags = flags;
            if (round == 1) {
                modified_flags &= ~(cEncodeBC1Use2DLS | cEncodeBC1BoundingBox);
                modified_flags |= cEncodeBC1BoundingBox;
            }

            int round_lr, round_lg, round_lb, round_hr, round_hg, round_hb;
            uint8_t round_sels[16];

            encode_bc1_pick_initial(pSrc_pixels, modified_flags, grayscale_flag != 0, min_r, min_g, min_b, max_r, max_g, max_b, avg_r, avg_g, avg_b, total_r,
                                    total_g, total_b, round_lr, round_lg, round_lb, round_hr, round_hg, round_hb);

            int orig_round_lr = round_lr, orig_round_lg = round_lg, orig_round_lb = round_lb, orig_round_hr = round_hr, orig_round_hg = round_hg,
                orig_round_hb = round_hb;

            uint32_t round_err = bc1_find_sels4(flags, pSrc_pixels, round_lr, round_lg, round_lb, round_hr, round_hg, round_hb, round_sels, UINT32_MAX);

            const uint32_t total_ls_passes = flags & cEncodeBC1TwoLeastSquaresPasses ? 2 : 1;
            for (uint32_t ls_pass = 0; ls_pass < total_ls_passes; ls_pass++) {
                int trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb;

                vec3F xl, xh;
                if (!compute_least_squares_endpoints4_rgb(pSrc_pixels, round_sels, &xl, &xh, total_r, total_g, total_b)) {
                    // All selectors equal - treat it as a solid block which should always be equal or better.
                    trial_lr = g_bc1_match5_equals_1[avg_r].m_hi;
                    trial_lg = g_bc1_match6_equals_1[avg_g].m_hi;
                    trial_lb = g_bc1_match5_equals_1[avg_b].m_hi;

                    trial_hr = g_bc1_match5_equals_1[avg_r].m_lo;
                    trial_hg = g_bc1_match6_equals_1[avg_g].m_lo;
                    trial_hb = g_bc1_match5_equals_1[avg_b].m_lo;

                    // In high/higher quality mode, let it try again in case the optimal tables have caused the sels to diverge.
                } else {
                    precise_round_565(xl, xh, trial_hr, trial_hg, trial_hb, trial_lr, trial_lg, trial_lb);
                }

                if ((round_lr == trial_lr) && (round_lg == trial_lg) && (round_lb == trial_lb) && (round_hr == trial_hr) && (round_hg == trial_hg) &&
                    (round_hb == trial_hb))
                    break;

                uint8_t trial_sels[16];
                uint32_t trial_err = bc1_find_sels4(flags, pSrc_pixels, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, trial_sels, round_err);

                if (trial_err < round_err) {
                    round_lr = trial_lr;
                    round_lg = trial_lg;
                    round_lb = trial_lb;

                    round_hr = trial_hr;
                    round_hg = trial_hg;
                    round_hb = trial_hb;

                    round_err = trial_err;
                    memcpy(round_sels, trial_sels, 16);
                } else
                    break;

            }  // ls_pass

            if (round_err <= cur_err) {
                cur_err = round_err;

                lr = round_lr;
                lg = round_lg;
                lb = round_lb;
                hr = round_hr;
                hg = round_hg;
                hb = round_hb;

                orig_lr = orig_round_lr;
                orig_lg = orig_round_lg;
                orig_lb = orig_round_lb;
                orig_hr = orig_round_hr;
                orig_hg = orig_round_hg;
                orig_hb = orig_round_hb;

                memcpy(sels, round_sels, 16);
            }

        }  // round
    }

    if ((cur_err) && (flags & cEncodeBC1UseLikelyTotalOrderings)) {
        assert(needs_block_error);

        const uint32_t total_iters = (flags & cEncodeBC1Iterative) ? 2 : 1;
        for (uint32_t iter_index = 0; iter_index < total_iters; iter_index++) {
            const uint32_t orig_err = cur_err;

            hist4 h;
            for (uint32_t i = 0; i < 16; i++) {
                assert(sels[i] < 4);
                h.m_hist[sels[i]]++;
            }

            const uint32_t orig_total_order_index = h.lookup_total_ordering_index();

            int r0, g0, b0, r3, g3, b3;
            r0 = (lr << 3) | (lr >> 2);
            g0 = (lg << 2) | (lg >> 4);
            b0 = (lb << 3) | (lb >> 2);
            r3 = (hr << 3) | (hr >> 2);
            g3 = (hg << 2) | (hg >> 4);
            b3 = (hb << 3) | (hb >> 2);

            int ar = r3 - r0, ag = g3 - g0, ab = b3 - b0;

            int dots[16];
            for (uint32_t i = 0; i < 16; i++) {
                int r = pSrc_pixels[i].r;
                int g = pSrc_pixels[i].g;
                int b = pSrc_pixels[i].b;
                int d = 0x1000000 + (r * ar + g * ag + b * ab);
                assert(d >= 0);
                dots[i] = (d << 4) + i;
            }

            std::sort(dots, dots + 16);

            uint32_t r_sum[17], g_sum[17], b_sum[17];
            uint32_t r = 0, g = 0, b = 0;
            for (uint32_t i = 0; i < 16; i++) {
                const uint32_t p = dots[i] & 15;

                r_sum[i] = r;
                g_sum[i] = g;
                b_sum[i] = b;

                r += pSrc_pixels[p].r;
                g += pSrc_pixels[p].g;
                b += pSrc_pixels[p].b;
            }

            r_sum[16] = total_r;
            g_sum[16] = total_g;
            b_sum[16] = total_b;

            const uint32_t q_total =
                (flags & cEncodeBC1Exhaustive) ? NUM_UNIQUE_TOTAL_ORDERINGS4 : clampi(total_orderings_to_try, MIN_TOTAL_ORDERINGS, MAX_TOTAL_ORDERINGS4);
            for (uint32_t q = 0; q < q_total; q++) {
                const uint32_t s = (flags & cEncodeBC1Exhaustive) ? q : g_best_total_orderings4[orig_total_order_index][q];

                int trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb;

                vec3F xl, xh;

                if ((s == TOTAL_ORDER_4_0_16) || (s == TOTAL_ORDER_4_1_16) || (s == TOTAL_ORDER_4_2_16) || (s == TOTAL_ORDER_4_3_16)) {
                    trial_lr = g_bc1_match5_equals_1[avg_r].m_hi;
                    trial_lg = g_bc1_match6_equals_1[avg_g].m_hi;
                    trial_lb = g_bc1_match5_equals_1[avg_b].m_hi;

                    trial_hr = g_bc1_match5_equals_1[avg_r].m_lo;
                    trial_hg = g_bc1_match6_equals_1[avg_g].m_lo;
                    trial_hb = g_bc1_match5_equals_1[avg_b].m_lo;
                } else {
                    compute_least_squares_endpoints4_rgb(&xl, &xh, total_r, total_g, total_b, g_selector_factors4[s][0], g_selector_factors4[s][1],
                                                         g_selector_factors4[s][2], s, r_sum, g_sum, b_sum);

                    precise_round_565(xl, xh, trial_hr, trial_hg, trial_hb, trial_lr, trial_lg, trial_lb);
                }

                uint8_t trial_sels[16];

                uint32_t trial_err = bc1_find_sels4(flags, pSrc_pixels, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, trial_sels, cur_err);

                if (trial_err < cur_err) {
                    cur_err = trial_err;

                    lr = trial_lr;
                    lg = trial_lg;
                    lb = trial_lb;

                    hr = trial_hr;
                    hg = trial_hg;
                    hb = trial_hb;

                    memcpy(sels, trial_sels, 16);
                }

            }  // s

            if ((!cur_err) || (cur_err == orig_err)) break;

        }  // iter_index
    }

    if (((flags & (cEncodeBC1Use3ColorBlocks | cEncodeBC1Use3ColorBlocksForBlackPixels)) != 0) && (cur_err)) {
        if (flags & cEncodeBC1Use3ColorBlocks) {
            assert(needs_block_error);
            try_3color_block(pSrc_pixels, flags, cur_err, avg_r, avg_g, avg_b, orig_lr, orig_lg, orig_lb, orig_hr, orig_hg, orig_hb, total_r, total_g, total_b,
                             total_orderings_to_try3, results);
        }

        if ((any_black_pixels) && ((flags & cEncodeBC1Use3ColorBlocksForBlackPixels) != 0)) {
            assert(needs_block_error);
            try_3color_block_useblack(pSrc_pixels, flags, cur_err, results);
        }
    }

    if ((flags & cEncodeBC1EndpointSearchRoundsMask) && (cur_err)) {
        assert(needs_block_error);

        encode_bc1_endpoint_search(pSrc_pixels, any_black_pixels != 0, flags, results, cur_err);
    }

    if (results.m_3color)
        bc1_encode3(pDst_block, results.lr, results.lg, results.lb, results.hr, results.hg, results.hb, results.sels);
    else
        bc1_encode4(pDst_block, results.lr, results.lg, results.lb, results.hr, results.hg, results.hb, results.sels);
}

// BC3-5
void encode_bc4(void *pDst, const uint8_t *pPixels, uint32_t stride) {
    assert(g_initialized);

    uint32_t min0_v, max0_v, min1_v, max1_v, min2_v, max2_v, min3_v, max3_v;

    {
        min0_v = max0_v = pPixels[0 * stride];
        min1_v = max1_v = pPixels[1 * stride];
        min2_v = max2_v = pPixels[2 * stride];
        min3_v = max3_v = pPixels[3 * stride];
    }

    {
        uint32_t v0 = pPixels[4 * stride];
        min0_v = std::min(min0_v, v0);
        max0_v = std::max(max0_v, v0);
        uint32_t v1 = pPixels[5 * stride];
        min1_v = std::min(min1_v, v1);
        max1_v = std::max(max1_v, v1);
        uint32_t v2 = pPixels[6 * stride];
        min2_v = std::min(min2_v, v2);
        max2_v = std::max(max2_v, v2);
        uint32_t v3 = pPixels[7 * stride];
        min3_v = std::min(min3_v, v3);
        max3_v = std::max(max3_v, v3);
    }

    {
        uint32_t v0 = pPixels[8 * stride];
        min0_v = std::min(min0_v, v0);
        max0_v = std::max(max0_v, v0);
        uint32_t v1 = pPixels[9 * stride];
        min1_v = std::min(min1_v, v1);
        max1_v = std::max(max1_v, v1);
        uint32_t v2 = pPixels[10 * stride];
        min2_v = std::min(min2_v, v2);
        max2_v = std::max(max2_v, v2);
        uint32_t v3 = pPixels[11 * stride];
        min3_v = std::min(min3_v, v3);
        max3_v = std::max(max3_v, v3);
    }

    {
        uint32_t v0 = pPixels[12 * stride];
        min0_v = std::min(min0_v, v0);
        max0_v = std::max(max0_v, v0);
        uint32_t v1 = pPixels[13 * stride];
        min1_v = std::min(min1_v, v1);
        max1_v = std::max(max1_v, v1);
        uint32_t v2 = pPixels[14 * stride];
        min2_v = std::min(min2_v, v2);
        max2_v = std::max(max2_v, v2);
        uint32_t v3 = pPixels[15 * stride];
        min3_v = std::min(min3_v, v3);
        max3_v = std::max(max3_v, v3);
    }

    const uint32_t min_v = minimum(min0_v, min1_v, min2_v, min3_v);
    const uint32_t max_v = maximum(max0_v, max1_v, max2_v, max3_v);

    uint8_t *pDst_bytes = static_cast<uint8_t *>(pDst);
    pDst_bytes[0] = (uint8_t)max_v;
    pDst_bytes[1] = (uint8_t)min_v;

    if (max_v == min_v) {
        memset(pDst_bytes + 2, 0, 6);
        return;
    }

    const uint32_t delta = max_v - min_v;

    // min_v is now 0. Compute thresholds between values by scaling max_v. It's x14 because we're adding two x7 scale factors.
    const int t0 = delta * 13;
    const int t1 = delta * 11;
    const int t2 = delta * 9;
    const int t3 = delta * 7;
    const int t4 = delta * 5;
    const int t5 = delta * 3;
    const int t6 = delta * 1;

    // BC4 floors in its divisions, which we compensate for with the 4 bias.
    // This function is optimal for all possible inputs (i.e. it outputs the same results as checking all 8 values and choosing the closest one).
    const int bias = 4 - min_v * 14;

    static const uint32_t s_tran0[8] = {1U, 7U, 6U, 5U, 4U, 3U, 2U, 0U};
    static const uint32_t s_tran1[8] = {1U << 3U, 7U << 3U, 6U << 3U, 5U << 3U, 4U << 3U, 3U << 3U, 2U << 3U, 0U << 3U};
    static const uint32_t s_tran2[8] = {1U << 6U, 7U << 6U, 6U << 6U, 5U << 6U, 4U << 6U, 3U << 6U, 2U << 6U, 0U << 6U};
    static const uint32_t s_tran3[8] = {1U << 9U, 7U << 9U, 6U << 9U, 5U << 9U, 4U << 9U, 3U << 9U, 2U << 9U, 0U << 9U};

    uint64_t a0, a1, a2, a3;
    {
        const int v0 = pPixels[0 * stride] * 14 + bias;
        const int v1 = pPixels[1 * stride] * 14 + bias;
        const int v2 = pPixels[2 * stride] * 14 + bias;
        const int v3 = pPixels[3 * stride] * 14 + bias;
        a0 = s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)];
        a1 = s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)];
        a2 = s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)];
        a3 = s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)];
    }

    {
        const int v0 = pPixels[4 * stride] * 14 + bias;
        const int v1 = pPixels[5 * stride] * 14 + bias;
        const int v2 = pPixels[6 * stride] * 14 + bias;
        const int v3 = pPixels[7 * stride] * 14 + bias;
        a0 |= (uint64_t)(s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)] << 12U);
        a1 |= (uint64_t)(s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)] << 12U);
        a2 |= (uint64_t)(s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)] << 12U);
        a3 |= (uint64_t)(s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)] << 12U);
    }

    {
        const int v0 = pPixels[8 * stride] * 14 + bias;
        const int v1 = pPixels[9 * stride] * 14 + bias;
        const int v2 = pPixels[10 * stride] * 14 + bias;
        const int v3 = pPixels[11 * stride] * 14 + bias;
        a0 |= (((uint64_t)s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)]) << 24U);
        a1 |= (((uint64_t)s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)]) << 24U);
        a2 |= (((uint64_t)s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)]) << 24U);
        a3 |= (((uint64_t)s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)]) << 24U);
    }

    {
        const int v0 = pPixels[12 * stride] * 14 + bias;
        const int v1 = pPixels[13 * stride] * 14 + bias;
        const int v2 = pPixels[14 * stride] * 14 + bias;
        const int v3 = pPixels[15 * stride] * 14 + bias;
        a0 |= (((uint64_t)s_tran0[(v0 >= t0) + (v0 >= t1) + (v0 >= t2) + (v0 >= t3) + (v0 >= t4) + (v0 >= t5) + (v0 >= t6)]) << 36U);
        a1 |= (((uint64_t)s_tran1[(v1 >= t0) + (v1 >= t1) + (v1 >= t2) + (v1 >= t3) + (v1 >= t4) + (v1 >= t5) + (v1 >= t6)]) << 36U);
        a2 |= (((uint64_t)s_tran2[(v2 >= t0) + (v2 >= t1) + (v2 >= t2) + (v2 >= t3) + (v2 >= t4) + (v2 >= t5) + (v2 >= t6)]) << 36U);
        a3 |= (((uint64_t)s_tran3[(v3 >= t0) + (v3 >= t1) + (v3 >= t2) + (v3 >= t3) + (v3 >= t4) + (v3 >= t5) + (v3 >= t6)]) << 36U);
    }

    const uint64_t f = a0 | a1 | a2 | a3;

    // TODO: make this less silly by using the BC4Block class
    pDst_bytes[2] = (uint8_t)f;
    pDst_bytes[3] = (uint8_t)(f >> 8U);
    pDst_bytes[4] = (uint8_t)(f >> 16U);
    pDst_bytes[5] = (uint8_t)(f >> 24U);
    pDst_bytes[6] = (uint8_t)(f >> 32U);
    pDst_bytes[7] = (uint8_t)(f >> 40U);
}

void encode_bc3(BC3Block *pDst, const uint8_t *pPixels, uint32_t flags, uint32_t total_orderings_to_try) {
    assert(g_initialized);

    // 3-color blocks are not allowed with BC3 (on most GPU's).
    flags &= ~(cEncodeBC1Use3ColorBlocksForBlackPixels | cEncodeBC1Use3ColorBlocks);

    encode_bc4(&pDst->alpha_block, pPixels + 3, 4);
    encode_bc1(&pDst->color_block, pPixels, flags, total_orderings_to_try);
}

void encode_bc3(uint32_t level, BC3Block *pDst, const uint8_t *pPixels) {
    assert(g_initialized);

    encode_bc4(&pDst->alpha_block, pPixels + 3, 4);
    encode_bc1(level, &pDst->color_block, pPixels, false, false);
}

void encode_bc5(BC5Block *pDst, const uint8_t *pPixels, uint32_t chan0, uint32_t chan1, uint32_t stride) {
    assert(g_initialized);

    encode_bc4(&pDst->chan0_block, pPixels + chan0, stride);
    encode_bc4(&pDst->chan1_block, pPixels + chan1, stride);
}

// Returns true if the block uses 3 color punchthrough alpha mode.
bool unpack_bc1(const void *pBlock_bits, void *pPixels, bool set_alpha, bc1_approx_mode mode) {
    Color *pDst_pixels = static_cast<Color *>(pPixels);

    static_assert(sizeof(BC1Block) == 8, "sizeof(BC1Block) == 8");
    static_assert(sizeof(BC4Block) == 8, "sizeof(BC4Block) == 8");

    const BC1Block *pBlock = static_cast<const BC1Block *>(pBlock_bits);

    const uint32_t l = pBlock->GetLowColor();
    const uint32_t h = pBlock->GetHighColor();

    Color c[4];

    const int cr0 = (l >> 11) & 31;
    const int cg0 = (l >> 5) & 63;
    const int cb0 = l & 31;
    const int r0 = (cr0 << 3) | (cr0 >> 2);
    const int g0 = (cg0 << 2) | (cg0 >> 4);
    const int b0 = (cb0 << 3) | (cb0 >> 2);

    const int cr1 = (h >> 11) & 31;
    const int cg1 = (h >> 5) & 63;
    const int cb1 = h & 31;
    const int r1 = (cr1 << 3) | (cr1 >> 2);
    const int g1 = (cg1 << 2) | (cg1 >> 4);
    const int b1 = (cb1 << 3) | (cb1 >> 2);

    bool used_punchthrough = false;

    if (l > h) {
        c[0].SetRGBA(r0, g0, b0, 255);
        c[1].SetRGBA(r1, g1, b1, 255);
        switch (mode) {
            case bc1_approx_mode::cBC1Ideal:
                c[2].SetRGBA((r0 * 2 + r1) / 3, (g0 * 2 + g1) / 3, (b0 * 2 + b1) / 3, 255);
                c[3].SetRGBA((r1 * 2 + r0) / 3, (g1 * 2 + g0) / 3, (b1 * 2 + b0) / 3, 255);
                break;
            case bc1_approx_mode::cBC1IdealRound4:
                c[2].SetRGBA((r0 * 2 + r1 + 1) / 3, (g0 * 2 + g1 + 1) / 3, (b0 * 2 + b1 + 1) / 3, 255);
                c[3].SetRGBA((r1 * 2 + r0 + 1) / 3, (g1 * 2 + g0 + 1) / 3, (b1 * 2 + b0 + 1) / 3, 255);
                break;
            case bc1_approx_mode::cBC1NVidia:
                c[2].SetRGBA(interp_5_nv(cr0, cr1), interp_6_nv(g0, g1), interp_5_nv(cb0, cb1), 255);
                c[3].SetRGBA(interp_5_nv(cr1, cr0), interp_6_nv(g1, g0), interp_5_nv(cb1, cb0), 255);
                break;
            case bc1_approx_mode::cBC1AMD:
                c[2].SetRGBA(interp_5_6_amd(r0, r1), interp_5_6_amd(g0, g1), interp_5_6_amd(b0, b1), 255);
                c[3].SetRGBA(interp_5_6_amd(r1, r0), interp_5_6_amd(g1, g0), interp_5_6_amd(b1, b0), 255);
                break;
        }
    } else {
        c[0].SetRGBA(r0, g0, b0, 255);
        c[1].SetRGBA(r1, g1, b1, 255);
        switch (mode) {
            case bc1_approx_mode::cBC1Ideal:
            case bc1_approx_mode::cBC1IdealRound4:
                c[2].SetRGBA((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, 255);
                break;
            case bc1_approx_mode::cBC1NVidia:
                c[2].SetRGBA(interp_half_5_nv(cr0, cr1), interp_half_6_nv(g0, g1), interp_half_5_nv(cb0, cb1), 255);
                break;
            case bc1_approx_mode::cBC1AMD:
                c[2].SetRGBA(interp_half_5_6_amd(r0, r1), interp_half_5_6_amd(g0, g1), interp_half_5_6_amd(b0, b1), 255);
                break;
        }

        c[3].SetRGBA(0, 0, 0, 0);
        used_punchthrough = true;
    }

    if (set_alpha) {
        for (uint32_t y = 0; y < 4; y++, pDst_pixels += 4) {
            pDst_pixels[0] = c[pBlock->GetSelector(0, y)];
            pDst_pixels[1] = c[pBlock->GetSelector(1, y)];
            pDst_pixels[2] = c[pBlock->GetSelector(2, y)];
            pDst_pixels[3] = c[pBlock->GetSelector(3, y)];
        }
    } else {
        for (uint32_t y = 0; y < 4; y++, pDst_pixels += 4) {
            pDst_pixels[0].SetRGBA(c[pBlock->GetSelector(0, y)]);
            pDst_pixels[1].SetRGBA(c[pBlock->GetSelector(1, y)]);
            pDst_pixels[2].SetRGBA(c[pBlock->GetSelector(2, y)]);
            pDst_pixels[3].SetRGBA(c[pBlock->GetSelector(3, y)]);
        }
    }

    return used_punchthrough;
}

void unpack_bc4(const void *pBlock_bits, uint8_t *pPixels, uint32_t stride) {
    static_assert(sizeof(BC4Block) == 8, "sizeof(BC4Block) == 8");

    const BC4Block *pBlock = static_cast<const BC4Block *>(pBlock_bits);

    auto sel_values = BC4Block::GetValues(pBlock->GetLowAlpha(), pBlock->GetHighAlpha());

    const uint64_t selector_bits = pBlock->GetSelectorBits();

    for (uint32_t y = 0; y < 4; y++, pPixels += (stride * 4U)) {
        pPixels[0] = sel_values[pBlock->GetSelector(0, y, selector_bits)];
        pPixels[stride * 1] = sel_values[pBlock->GetSelector(1, y, selector_bits)];
        pPixels[stride * 2] = sel_values[pBlock->GetSelector(2, y, selector_bits)];
        pPixels[stride * 3] = sel_values[pBlock->GetSelector(3, y, selector_bits)];
    }
}

// Returns false if the block uses 3-color punchthrough alpha mode, which isn't supported on some GPU's for BC3.
bool unpack_bc3(const void *pBlock_bits, void *pPixels, bc1_approx_mode mode) {
    Color *pDst_pixels = static_cast<Color *>(pPixels);

    bool success = true;

    if (unpack_bc1((const uint8_t *)pBlock_bits + sizeof(BC4Block), pDst_pixels, true, mode)) success = false;

    unpack_bc4(pBlock_bits, &pDst_pixels[0].a, sizeof(Color));

    return success;
}

// writes RG
void unpack_bc5(const void *pBlock_bits, void *pPixels, uint32_t chan0, uint32_t chan1, uint32_t stride) {
    unpack_bc4(pBlock_bits, (uint8_t *)pPixels + chan0, stride);
    unpack_bc4((const uint8_t *)pBlock_bits + sizeof(BC4Block), (uint8_t *)pPixels + chan1, stride);
}

}  // namespace rgbcx

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
#pragma once
// Pure C++ (no Qt) implementation of Myers' O(ND) diff in linear space
// (middle-snake / divide & conquer). Works on sequences of integer ids.

#include <vector>
#include <algorithm>

namespace diffalgo {

struct Match
{
    int a;
    int b;
};

namespace detail {

// Finds the middle snake between a[0..n) and b[0..m).
// On success (x, y) is a split point so that both halves can be diffed independently.
inline bool bisect(const int *a, int n, const int *b, int m, int &splitX, int &splitY)
{
    const int maxD = (n + m + 1) / 2;
    const int vOffset = maxD;
    const int vLength = 2 * maxD + 2;
    std::vector<int> v1(vLength, -1), v2(vLength, -1);
    v1[vOffset + 1] = 0;
    v2[vOffset + 1] = 0;
    const int delta = n - m;
    const bool front = (delta % 2) != 0;
    int k1start = 0, k1end = 0, k2start = 0, k2end = 0;

    for (int d = 0; d < maxD; ++d) {
        for (int k1 = -d + k1start; k1 <= d - k1end; k1 += 2) {
            const int k1Offset = vOffset + k1;
            int x1;
            if (k1 == -d || (k1 != d && v1[k1Offset - 1] < v1[k1Offset + 1]))
                x1 = v1[k1Offset + 1];
            else
                x1 = v1[k1Offset - 1] + 1;
            int y1 = x1 - k1;
            while (x1 < n && y1 < m && a[x1] == b[y1]) { ++x1; ++y1; }
            v1[k1Offset] = x1;
            if (x1 > n) {
                k1end += 2;
            } else if (y1 > m) {
                k1start += 2;
            } else if (front) {
                const int k2Offset = vOffset + delta - k1;
                if (k2Offset >= 0 && k2Offset < vLength && v2[k2Offset] != -1) {
                    const int x2 = n - v2[k2Offset];
                    if (x1 >= x2) { splitX = x1; splitY = y1; return true; }
                }
            }
        }
        for (int k2 = -d + k2start; k2 <= d - k2end; k2 += 2) {
            const int k2Offset = vOffset + k2;
            int x2;
            if (k2 == -d || (k2 != d && v2[k2Offset - 1] < v2[k2Offset + 1]))
                x2 = v2[k2Offset + 1];
            else
                x2 = v2[k2Offset - 1] + 1;
            int y2 = x2 - k2;
            while (x2 < n && y2 < m && a[n - x2 - 1] == b[m - y2 - 1]) { ++x2; ++y2; }
            v2[k2Offset] = x2;
            if (x2 > n) {
                k2end += 2;
            } else if (y2 > m) {
                k2start += 2;
            } else if (!front) {
                const int k1Offset = vOffset + delta - k2;
                if (k1Offset >= 0 && k1Offset < vLength && v1[k1Offset] != -1) {
                    const int x1 = v1[k1Offset];
                    const int y1 = vOffset + x1 - k1Offset;
                    if (x1 >= n - x2) { splitX = x1; splitY = y1; return true; }
                }
            }
        }
    }
    return false;
}

inline void diffRange(const int *a, int aLo, int aHi, const int *b, int bLo, int bHi,
                      std::vector<Match> &out)
{
    // common prefix
    while (aLo < aHi && bLo < bHi && a[aLo] == b[bLo]) {
        out.push_back({aLo, bLo});
        ++aLo; ++bLo;
    }
    // common suffix
    int suf = 0;
    while (aHi - suf > aLo && bHi - suf > bLo && a[aHi - suf - 1] == b[bHi - suf - 1])
        ++suf;
    const int aEnd = aHi - suf;
    const int bEnd = bHi - suf;

    if (aLo < aEnd && bLo < bEnd) {
        const int n = aEnd - aLo, m = bEnd - bLo;
        int x = 0, y = 0;
        if (bisect(a + aLo, n, b + bLo, m, x, y)
            && !(x == 0 && y == 0) && !(x == n && y == m)) {
            diffRange(a, aLo, aLo + x, b, bLo, bLo + y, out);
            diffRange(a, aLo + x, aEnd, b, bLo + y, bEnd, out);
        }
    }
    for (int i = 0; i < suf; ++i)
        out.push_back({aEnd + i, bEnd + i});
}

} // namespace detail

// Returns the ordered list of matching (equal) element pairs (index in a, index in b).
inline std::vector<Match> computeMatches(const std::vector<int> &a, const std::vector<int> &b)
{
    std::vector<Match> out;
    out.reserve(std::min(a.size(), b.size()));
    detail::diffRange(a.data(), 0, int(a.size()), b.data(), 0, int(b.size()), out);
    return out;
}

} // namespace diffalgo

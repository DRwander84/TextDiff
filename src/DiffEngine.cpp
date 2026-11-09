#include "DiffEngine.h"
#include "DiffAlgo.h"

#include <QHash>

#include <algorithm>

namespace DiffEngine {
namespace {

struct Token
{
    int start;
    int length;
};

// Words (letters/digits/_), whitespace runs, and single punctuation characters.
bool isWordChar(QChar c)
{
    // marks (diacritics, vowel signs) and ZWNJ / ZWJ stay inside their word
    return c.isLetterOrNumber() || c == QLatin1Char('_') || c.isMark() || c == QChar(0x200C)
        || c == QChar(0x200D);
}

QVector<Token> tokenize(const QString &s)
{
    QVector<Token> tokens;
    const int n = s.size();
    int i = 0;
    while (i < n) {
        const QChar c = s.at(i);
        int j = i + 1;
        if (isWordChar(c) || (c.isHighSurrogate() && j < n && s.at(j).isLowSurrogate())) {
            while (j < n && (isWordChar(s.at(j)) || s.at(j).isLowSurrogate() || s.at(j).isHighSurrogate()))
                ++j;
        } else if (c.isSpace()) {
            while (j < n && s.at(j).isSpace())
                ++j;
        } else if (c.isHighSurrogate() && j < n && s.at(j).isLowSurrogate()) {
            ++j;
        }
        tokens.push_back({i, j - i});
        i = j;
    }
    return tokens;
}

void appendSegment(QVector<Segment> &segs, int start, int length, bool same)
{
    if (!segs.isEmpty() && segs.last().same == same
        && segs.last().start + segs.last().length == start) {
        segs.last().length += length;
    } else {
        segs.push_back({start, length, same});
    }
}

void wordDiff(const QString &a, const QString &b, QVector<Segment> &aSegs, QVector<Segment> &bSegs)
{
    const QVector<Token> ta = tokenize(a);
    const QVector<Token> tb = tokenize(b);

    QHash<QString, int> ids;
    auto idOf = [&ids](const QString &s, const Token &t) {
        const QString key = s.mid(t.start, t.length);
        auto it = ids.find(key);
        if (it == ids.end())
            it = ids.insert(key, ids.size());
        return it.value();
    };

    std::vector<int> ia, ib;
    ia.reserve(ta.size());
    ib.reserve(tb.size());
    for (const Token &t : ta) ia.push_back(idOf(a, t));
    for (const Token &t : tb) ib.push_back(idOf(b, t));

    const auto matches = diffalgo::computeMatches(ia, ib);

    // 1) words that matched are identical
    QVector<char> sameA(a.size(), 0), sameB(b.size(), 0);

    // 2) inside the gaps between matched words, compare character by character
    auto charDiff = [&](int a0, int a1, int b0, int b1) {
        if (a0 >= a1 || b0 >= b1)
            return;
        // one id per code point (surrogate pairs are never split)
        std::vector<int> ca, cb, pa, pb;
        auto collect = [](const QString &str, int from, int to, std::vector<int> &ids, std::vector<int> &pos) {
            for (int k = from; k < to; ++k) {
                pos.push_back(k);
                if (str.at(k).isHighSurrogate() && k + 1 < to && str.at(k + 1).isLowSurrogate()) {
                    ids.push_back(int(QChar::surrogateToUcs4(str.at(k), str.at(k + 1))));
                    ++k;
                } else {
                    ids.push_back(str.at(k).unicode());
                }
            }
        };
        collect(a, a0, a1, ca, pa);
        collect(b, b0, b1, cb, pb);
        auto markRun = [](QVector<char> &flags, const QString &str, int from) {
            flags[from] = 1;
            if (str.at(from).isHighSurrogate() && from + 1 < str.size() && str.at(from + 1).isLowSurrogate())
                flags[from + 1] = 1;
        };
        for (const auto &m : diffalgo::computeMatches(ca, cb)) {
            markRun(sameA, a, pa[m.a]);
            markRun(sameB, b, pb[m.b]);
        }
    };

    int prevA = 0, prevB = 0;
    for (size_t k = 0; k <= matches.size(); ++k) {
        const bool last = (k == matches.size());
        const int startA = last ? int(a.size()) : ta[matches[k].a].start;
        const int startB = last ? int(b.size()) : tb[matches[k].b].start;
        charDiff(prevA, startA, prevB, startB);
        if (!last) {
            const Token &x = ta[matches[k].a];
            const Token &y = tb[matches[k].b];
            for (int c = 0; c < x.length; ++c) sameA[x.start + c] = 1;
            for (int c = 0; c < y.length; ++c) sameB[y.start + c] = 1;
            prevA = x.start + x.length;
            prevB = y.start + y.length;
        }
    }

    // 3) merge into runs of identical / different characters
    for (int i = 0; i < a.size(); ++i)
        appendSegment(aSegs, i, 1, sameA[i]);
    for (int i = 0; i < b.size(); ++i)
        appendSegment(bSegs, i, 1, sameB[i]);
}

std::vector<int> toIds(const QStringList &lines, QHash<QString, int> &ids)
{
    std::vector<int> out;
    out.reserve(lines.size());
    for (const QString &l : lines) {
        auto it = ids.find(l);
        if (it == ids.end())
            it = ids.insert(l, ids.size());
        out.push_back(it.value());
    }
    return out;
}

} // namespace

Result compute(const QStringList &sample, const QStringList &test)
{
    Result r;
    r.sample = sample;
    r.test = test;

    QHash<QString, int> ids;
    ids.reserve(sample.size() + test.size());
    const std::vector<int> ia = toIds(sample, ids);
    const std::vector<int> ib = toIds(test, ids);
    const auto matches = diffalgo::computeMatches(ia, ib);

    r.rows.reserve(int(std::max(sample.size(), test.size())));

    int i = 0, j = 0;
    auto flushGap = [&](int aEnd, int bEnd) {
        const int na = aEnd - i;
        const int nb = bEnd - j;
        const int paired = std::min(na, nb);
        for (int k = 0; k < paired; ++k) {
            Row row;
            row.kind = RowKind::Modified;
            row.aLine = i + k;
            row.bLine = j + k;
            wordDiff(sample.at(row.aLine), test.at(row.bLine), row.aSegments, row.bSegments);
            r.rows.push_back(std::move(row));
            ++r.stats.modified;
        }
        for (int k = paired; k < na; ++k) {
            Row row;
            row.kind = RowKind::OnlyInSample;
            row.aLine = i + k;
            r.rows.push_back(std::move(row));
            ++r.stats.onlyInSample;
        }
        for (int k = paired; k < nb; ++k) {
            Row row;
            row.kind = RowKind::OnlyInTest;
            row.bLine = j + k;
            r.rows.push_back(std::move(row));
            ++r.stats.onlyInTest;
        }
    };

    for (const auto &m : matches) {
        flushGap(m.a, m.b);
        Row row;
        row.kind = RowKind::Equal;
        row.aLine = m.a;
        row.bLine = m.b;
        r.rows.push_back(std::move(row));
        ++r.stats.equal;
        i = m.a + 1;
        j = m.b + 1;
    }
    flushGap(int(sample.size()), int(test.size()));
    return r;
}

} // namespace DiffEngine

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace DiffEngine {

enum class RowKind { Equal, Modified, OnlyInSample, OnlyInTest };

// A run of characters inside a line that is either identical (same) or different.
struct Segment
{
    int start = 0;
    int length = 0;
    bool same = false;
};

// One visual row shared by both panes. aLine / bLine are 0-based line indexes (-1 = padding).
struct Row
{
    RowKind kind = RowKind::Equal;
    int aLine = -1;
    int bLine = -1;
    QVector<Segment> aSegments; // only filled for Modified rows
    QVector<Segment> bSegments;
};

struct Stats
{
    int equal = 0;
    int modified = 0;
    int onlyInSample = 0;
    int onlyInTest = 0;
};

struct Result
{
    QStringList sample;
    QStringList test;
    QVector<Row> rows;
    Stats stats;
};

// Line level diff (Myers), followed by a word level diff for every modified line pair.
Result compute(const QStringList &sample, const QStringList &test);

} // namespace DiffEngine

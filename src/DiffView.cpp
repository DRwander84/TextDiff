#include <algorithm>
#include "DiffView.h"

#include "CodeEditor.h"
#include "FilePane.h"
#include "Theme.h"

#include <QHBoxLayout>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>

namespace {
constexpr int kMargin = 150; // lines highlighted above/below the visible area
}

DiffView::DiffView(QWidget *parent)
    : QWidget(parent)
{
    m_panes[0] = new FilePane(tr("File 1"), this);
    m_panes[1] = new FilePane(tr("File 2"), this);

    m_layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(14);
    m_layout->addWidget(m_panes[0], 1);
    m_layout->addWidget(m_panes[1], 1);

    m_timer.setSingleShot(true);
    m_timer.setInterval(15);
    connect(&m_timer, &QTimer::timeout, this, &DiffView::refreshHighlights);

    for (int i = 0; i < 2; ++i) {
        CodeEditor *me = m_panes[i]->editor();
        CodeEditor *other = m_panes[1 - i]->editor();
        connect(me, &CodeEditor::viewChanged, &m_timer, qOverload<>(&QTimer::start));
        // keep both editors scrolled together
        auto link = [this](QScrollBar *from, QScrollBar *to) {
            connect(from, &QScrollBar::valueChanged, this, [this, to](int v) {
                if (m_syncing)
                    return;
                m_syncing = true;
                to->setValue(v);
                m_syncing = false;
            });
        };
        link(me->verticalScrollBar(), other->verticalScrollBar());
        link(me->horizontalScrollBar(), other->horizontalScrollBar());
    }
}

void DiffView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // narrow window: put File 2 under File 1 instead of beside it
    m_layout->setDirection(width() < 820 ? QBoxLayout::TopToBottom : QBoxLayout::LeftToRight);
}

void DiffView::setResult(const DiffEngine::Result &result)
{
    m_result = result;
    m_hasResult = true;
    m_lineRow[0].fill(-1, result.sample.size());
    m_lineRow[1].fill(-1, result.test.size());
    for (int r = 0; r < m_result.rows.size(); ++r) {
        const auto &row = m_result.rows.at(r);
        if (row.aLine >= 0)
            m_lineRow[0][row.aLine] = r;
        if (row.bLine >= 0)
            m_lineRow[1][row.bLine] = r;
    }
    refreshHighlights();
}

void DiffView::clearResult()
{
    m_hasResult = false;
    m_result = {};
    m_lineRow[0].clear();
    m_lineRow[1].clear();
    refreshHighlights();
}

void DiffView::refreshHighlights()
{
    applyHighlights(0);
    applyHighlights(1);
}

void DiffView::applyHighlights(int side)
{
    using DiffEngine::RowKind;
    CodeEditor *ed = m_panes[side]->editor();
    if (!m_hasResult) {
        ed->setExtraSelections({});
        return;
    }

    const Theme &t = theme();
    const QVector<int> &lineRow = m_lineRow[side];
    const int count = ed->blockCount();
    const int first = std::max(0, ed->firstVisibleLine() - kMargin);
    const int last = std::min({count - 1, int(lineRow.size()) - 1, ed->lastVisibleLine() + kMargin});

    QList<QTextEdit::ExtraSelection> blocks, words;
    QTextDocument *doc = ed->document();
    QTextBlock blk = doc->findBlockByNumber(first);

    auto addBlock = [&](const QTextBlock &b, const QColor &c) {
        QTextEdit::ExtraSelection s;
        s.format.setBackground(c);
        s.format.setProperty(QTextFormat::FullWidthSelection, true);
        s.cursor = QTextCursor(b);
        blocks.append(s);
    };
    auto addWord = [&](const QTextBlock &b, int start, int len, const QColor &c) {
        if (len <= 0 || start + len > b.length() - 1)
            return;
        QTextEdit::ExtraSelection s;
        s.format.setBackground(c);
        s.cursor = QTextCursor(doc);
        s.cursor.setPosition(b.position() + start);
        s.cursor.setPosition(b.position() + start + len, QTextCursor::KeepAnchor);
        words.append(s);
    };

    for (int line = first; line <= last && blk.isValid(); ++line, blk = blk.next()) {
        const int r = lineRow.at(line);
        if (r < 0)
            continue;
        const DiffEngine::Row &row = m_result.rows.at(r);
        switch (row.kind) {
        case RowKind::Equal:
            addBlock(blk, t.equalBlock);
            break;
        case RowKind::Modified:
            // mark exactly the characters that differ (no whole-line background)
            for (const DiffEngine::Segment &sg : side == 0 ? row.aSegments : row.bSegments)
                addWord(blk, sg.start, sg.length, sg.same ? t.equalSeg : t.diffSeg);
            break;
        default: // line exists only in this file
            addBlock(blk, t.diffBlock);
            addWord(blk, 0, blk.length() - 1, t.diffSeg);
            break;
        }
    }
    ed->setExtraSelections(blocks + words);
}

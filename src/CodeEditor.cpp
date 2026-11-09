#include <algorithm>
#include "CodeEditor.h"
#include "Theme.h"

#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextOption>

namespace {
class Gutter : public QWidget
{
public:
    explicit Gutter(CodeEditor *e) : QWidget(e), m_editor(e) {}
    QSize sizeHint() const override { return {m_editor->gutterWidth(), 0}; }

protected:
    void paintEvent(QPaintEvent *event) override { m_editor->paintGutter(event); }

private:
    CodeEditor *m_editor;
};
} // namespace

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_gutter(new Gutter(this))
{
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setUndoRedoEnabled(true);
    setAcceptDrops(false); // the main window handles file drops
    viewport()->setAcceptDrops(false);

    QFont f;
    f.setFamilies({"Cascadia Mono", "Consolas", "JetBrains Mono", "Vazirmatn", "Tahoma", "Segoe UI",
                   "Noto Sans Arabic", "Noto Sans", "Courier New"});
    f.setStyleHint(QFont::Monospace);
    f.setPointSize(10);
    setFont(f);
    // Every paragraph gets its own direction from its first strong character, so Persian,
    // Arabic, Hebrew... lines are right-to-left / right-aligned while the rest stays left-to-right.
    QTextOption option = document()->defaultTextOption();
    option.setTextDirection(Qt::LayoutDirectionAuto);
    document()->setDefaultTextOption(option);
    setTabStopDistance(QFontMetricsF(f).horizontalAdvance(QLatin1Char(' ')) * 4);

    connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditor::updateGutterWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditor::updateGutter);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &CodeEditor::viewChanged);
    updateGutterWidth();
}

int CodeEditor::gutterWidth() const
{
    int digits = 1;
    for (int n = std::max(1, blockCount()); n >= 10; n /= 10)
        ++digits;
    return 18 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * std::max(digits, 3);
}

void CodeEditor::updateGutterWidth()
{
    setViewportMargins(gutterWidth(), 0, 0, 0);
    m_gutter->update();
}

void CodeEditor::updateGutter(const QRect &rect, int dy)
{
    if (dy)
        m_gutter->scroll(0, dy);
    else
        m_gutter->update(0, rect.y(), m_gutter->width(), rect.height());
    if (rect.contains(viewport()->rect()))
        updateGutterWidth();
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_gutter->setGeometry(QRect(cr.left(), cr.top(), gutterWidth(), cr.height()));
    emit viewChanged();
}

int CodeEditor::firstVisibleLine() const
{
    return firstVisibleBlock().blockNumber();
}

int CodeEditor::lastVisibleLine() const
{
    QTextBlock b = firstVisibleBlock();
    qreal top = blockBoundingGeometry(b).translated(contentOffset()).top();
    const qreal bottom = viewport()->height();
    int last = b.blockNumber();
    while (b.isValid() && top <= bottom) {
        last = b.blockNumber();
        top += blockBoundingRect(b).height();
        b = b.next();
    }
    return last;
}

void CodeEditor::paintGutter(QPaintEvent *event)
{
    const Theme &t = theme();
    QPainter p(m_gutter);
    p.fillRect(event->rect(), t.gutterBg);
    p.setPen(t.dimText);
    p.setFont(font());

    QTextBlock block = firstVisibleBlock();
    int number = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top())
            p.drawText(0, top, m_gutter->width() - 8, fontMetrics().height(), Qt::AlignRight,
                       QString::number(number + 1));
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++number;
    }
}

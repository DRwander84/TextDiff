#include "WindowChrome.h"
#include "Theme.h"

#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QWindow>

#include <algorithm>

CaptionButton::CaptionButton(Kind kind, QWidget *parent)
    : QAbstractButton(parent), m_kind(kind)
{
    setAttribute(Qt::WA_Hover);
    setCursor(Qt::ArrowCursor);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(42, 32);
}

void CaptionButton::setMaximized(bool maximized)
{
    m_maximized = maximized;
    update();
}

void CaptionButton::paintEvent(QPaintEvent *)
{
    const Theme &t = theme();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const bool close = m_kind == Close;
    QColor glyph = t.text;
    if (underMouse() || isDown()) {
        QColor bg = close ? QColor(232, 17, 35, isDown() ? 200 : 255)
                          : (isDown() ? t.buttonPressed : t.buttonHover);
        p.setPen(Qt::NoPen);
        p.setBrush(bg);
        p.drawRoundedRect(QRectF(rect()).adjusted(2, 2, -2, -2), 9, 9);
        if (close)
            glyph = Qt::white;
    }

    p.setPen(QPen(glyph, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(Qt::NoBrush);
    const QPointF c = QRectF(rect()).center();
    switch (m_kind) {
    case Minimize:
        p.drawLine(QPointF(c.x() - 5, c.y()), QPointF(c.x() + 5, c.y()));
        break;
    case Maximize:
        if (m_maximized) {
            p.drawRoundedRect(QRectF(c.x() - 5, c.y() - 2, 8, 8), 1.5, 1.5);
            p.drawLine(QPointF(c.x() - 2, c.y() - 5), QPointF(c.x() + 4, c.y() - 5));
            p.drawLine(QPointF(c.x() + 4, c.y() - 5), QPointF(c.x() + 4, c.y() + 2));
        } else {
            p.drawRoundedRect(QRectF(c.x() - 5, c.y() - 5, 10, 10), 2, 2);
        }
        break;
    case Close:
        p.drawLine(QPointF(c.x() - 5, c.y() - 5), QPointF(c.x() + 5, c.y() + 5));
        p.drawLine(QPointF(c.x() - 5, c.y() + 5), QPointF(c.x() + 5, c.y() - 5));
        break;
    }
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_offset = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
        event->accept();
        return;
    }
    GlassPanel::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || !(event->buttons() & Qt::LeftButton)) {
        GlassPanel::mouseMoveEvent(event);
        return;
    }
    QWidget *w = window();
    const QPoint global = event->globalPosition().toPoint();
    if (w->isMaximized() || w->isFullScreen()) {
        // dragging a maximized window restores it under the cursor
        const double ratio = double(m_offset.x()) / std::max(1, w->width());
        const int normalWidth = w->normalGeometry().width();
        w->showNormal();
        m_offset = QPoint(int(ratio * normalWidth), 24);
    }
    w->move(global - m_offset);
    event->accept();
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    GlassPanel::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit toggleMaximizeRequested();
        event->accept();
        return;
    }
    GlassPanel::mouseDoubleClickEvent(event);
}

WindowFrame::WindowFrame(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

void WindowFrame::setResizeEnabled(bool enabled)
{
    m_resizeEnabled = enabled;
    if (!enabled)
        unsetCursor();
}

void WindowFrame::paintEvent(QPaintEvent *)
{
    // Almost invisible, but not fully transparent: on Windows, fully transparent
    // pixels of a layered window let mouse clicks fall through (we need the resize grips).
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 1));
}

Qt::Edges WindowFrame::edgesAt(const QPoint &pos) const
{
    if (!m_resizeEnabled || !layout())
        return {};
    const int m = layout()->contentsMargins().left();
    if (m <= 0)
        return {};
    const int corner = 14; // length of the corner grips along the edges
    Qt::Edges e;
    if (pos.x() < m) e |= Qt::LeftEdge;
    if (pos.x() >= width() - m) e |= Qt::RightEdge;
    if (pos.y() < m) e |= Qt::TopEdge;
    if (pos.y() >= height() - m) e |= Qt::BottomEdge;
    if (e & (Qt::LeftEdge | Qt::RightEdge)) {
        if (pos.y() < corner) e |= Qt::TopEdge;
        if (pos.y() >= height() - corner) e |= Qt::BottomEdge;
    }
    if (e & (Qt::TopEdge | Qt::BottomEdge)) {
        if (pos.x() < corner) e |= Qt::LeftEdge;
        if (pos.x() >= width() - corner) e |= Qt::RightEdge;
    }
    return e;
}

void WindowFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (m_resizing) {
        if (!(event->buttons() & Qt::LeftButton)) {
            m_resizing = false;
            return;
        }
        const QPoint d = event->globalPosition().toPoint() - m_startGlobal;
        const QSize minSize = window()->minimumSize();
        QRect g = m_startGeometry;
        if (m_edges & Qt::LeftEdge)
            g.setLeft(std::min(g.left() + d.x(), g.right() - minSize.width() + 1));
        if (m_edges & Qt::RightEdge)
            g.setRight(std::max(g.right() + d.x(), g.left() + minSize.width() - 1));
        if (m_edges & Qt::TopEdge)
            g.setTop(std::min(g.top() + d.y(), g.bottom() - minSize.height() + 1));
        if (m_edges & Qt::BottomEdge)
            g.setBottom(std::max(g.bottom() + d.y(), g.top() + minSize.height() - 1));
        window()->setGeometry(g);
        return;
    }

    const Qt::Edges e = edgesAt(event->position().toPoint());
    if (e == (Qt::LeftEdge | Qt::TopEdge) || e == (Qt::RightEdge | Qt::BottomEdge))
        setCursor(Qt::SizeFDiagCursor);
    else if (e == (Qt::RightEdge | Qt::TopEdge) || e == (Qt::LeftEdge | Qt::BottomEdge))
        setCursor(Qt::SizeBDiagCursor);
    else if (e & (Qt::LeftEdge | Qt::RightEdge))
        setCursor(Qt::SizeHorCursor);
    else if (e & (Qt::TopEdge | Qt::BottomEdge))
        setCursor(Qt::SizeVerCursor);
    else
        unsetCursor();
}

void WindowFrame::mousePressEvent(QMouseEvent *event)
{
    const Qt::Edges e = edgesAt(event->position().toPoint());
    if (event->button() == Qt::LeftButton && e) {
        m_resizing = true;
        m_edges = e;
        m_startGeometry = window()->geometry();
        m_startGlobal = event->globalPosition().toPoint();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void WindowFrame::mouseReleaseEvent(QMouseEvent *event)
{
    m_resizing = false;
    QWidget::mouseReleaseEvent(event);
}

void WindowFrame::leaveEvent(QEvent *event)
{
    unsetCursor();
    QWidget::leaveEvent(event);
}

MoveGrip::MoveGrip(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover);
    setCursor(Qt::SizeAllCursor);
    setToolTip(tr("Drag to move the window"));
    setFixedSize(20, 20);
}

void MoveGrip::paintEvent(QPaintEvent *)
{
    const Theme &t = theme();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const bool active = underMouse() || m_dragging;
    if (active) {
        p.setPen(Qt::NoPen);
        p.setBrush(t.buttonHover);
        p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 7, 7);
    }
    QColor dot = active ? t.text : t.dimText;
    p.setPen(Qt::NoPen);
    p.setBrush(dot);
    // two rows of three dots, centred
    const qreal step = 5.0;
    const QPointF c = QRectF(rect()).center();
    for (int row = 0; row < 2; ++row)
        for (int col = -1; col <= 1; ++col)
            p.drawEllipse(QPointF(c.x() + col * step, c.y() + (row - 0.5) * step), 1.1, 1.1);
}

void MoveGrip::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !window()->isMaximized() && !window()->isFullScreen()) {
        m_dragging = true;
        m_offset = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void MoveGrip::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPosition().toPoint() - m_offset);
        event->accept();
    }
}

void MoveGrip::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    update();
    QWidget::mouseReleaseEvent(event);
}

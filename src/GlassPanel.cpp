#include <algorithm>
#include "GlassPanel.h"
#include "Theme.h"

#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>

void GlassBackdrop::paintEvent(QPaintEvent *)
{
    const Theme &t = theme();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    if (m_radius > 0)
        path.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);
    else
        path.addRect(rect());
    p.setClipPath(path);

    QLinearGradient bg(0, 0, width() * 0.6, height());
    bg.setColorAt(0, t.bgTop);
    bg.setColorAt(1, t.bgBottom);
    p.fillRect(rect(), bg);

    auto blob = [&](qreal cx, qreal cy, qreal r, QColor c) {
        QRadialGradient g(QPointF(width() * cx, height() * cy), r);
        QColor edge = c;
        edge.setAlpha(0);
        g.setColorAt(0, c);
        g.setColorAt(1, edge);
        p.fillRect(rect(), g);
    };
    const qreal base = std::max(width(), height());
    blob(0.12, 0.10, base * 0.45, t.blob1);
    blob(0.90, 0.35, base * 0.42, t.blob2);
    blob(0.35, 0.98, base * 0.50, t.blob3);

    p.setClipping(false);
    if (m_radius > 0) {
        p.setPen(QPen(t.dark ? QColor(255, 255, 255, 45) : QColor(0, 0, 0, 40), 1));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    }
}

void GlassPanel::paintEvent(QPaintEvent *)
{
    const Theme &t = theme();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = QRectF(rect()).adjusted(0.75, 0.75, -0.75, -0.75);
    QPainterPath path;
    path.addRoundedRect(r, m_radius, m_radius);

    p.fillPath(path, t.glassFill);

    // top sheen, fading out: gives the "curved glass" look
    QLinearGradient sheen(0, r.top(), 0, r.top() + r.height() * 0.55);
    QColor none = t.glassSheen;
    none.setAlpha(0);
    sheen.setColorAt(0, t.glassSheen);
    sheen.setColorAt(1, none);
    p.fillPath(path, sheen);

    // rim light: bright on the top-left and bottom-right edges, dim elsewhere
    QLinearGradient rim(r.topLeft(), r.bottomRight());
    rim.setColorAt(0.0, t.borderHi);
    rim.setColorAt(0.35, t.borderLo);
    rim.setColorAt(0.65, t.borderLo);
    rim.setColorAt(1.0, t.borderHi);
    p.setPen(QPen(QBrush(rim), 1.4));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
}

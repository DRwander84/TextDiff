#pragma once

#include "GlassPanel.h"

#include <QAbstractButton>
#include <QPoint>
#include <QRect>

// Minimize / maximize(restore) / close button drawn to match the theme.
class CaptionButton : public QAbstractButton
{
    Q_OBJECT
public:
    enum Kind { Minimize, Maximize, Close };
    explicit CaptionButton(Kind kind, QWidget *parent = nullptr);
    void setMaximized(bool maximized);
    QSize sizeHint() const override { return {42, 32}; }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    Kind m_kind;
    bool m_maximized = false;
};

// The window's own title bar: drag to move, double-click to maximize / restore.
class TitleBar : public GlassPanel
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr) : GlassPanel(parent, 20) {}

signals:
    void toggleMaximizeRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    bool m_dragging = false;
    QPoint m_offset; // cursor position relative to the window's top-left
};

// Outer transparent frame that provides the resize grips of the borderless window.
class WindowFrame : public QWidget
{
    Q_OBJECT
public:
    explicit WindowFrame(QWidget *parent = nullptr);
    void setResizeEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent *) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    Qt::Edges edgesAt(const QPoint &pos) const;
    bool m_resizeEnabled = true;
    bool m_resizing = false;
    Qt::Edges m_edges;
    QRect m_startGeometry;
    QPoint m_startGlobal;
};

// Small hover handle in the bottom-right corner: press and drag it to move the window.
class MoveGrip : public QWidget
{
    Q_OBJECT
public:
    explicit MoveGrip(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_dragging = false;
    QPoint m_offset;
};

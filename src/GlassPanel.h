#pragma once

#include <QWidget>

// Window backdrop: gradient + soft colour blobs that the glass panels float over.
class GlassBackdrop : public QWidget
{
    Q_OBJECT
public:
    explicit GlassBackdrop(QWidget *parent = nullptr) : QWidget(parent) {}
    // 0 = square corners (maximized / full screen)
    void setRadius(qreal radius) { m_radius = radius; update(); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    qreal m_radius = 22;
};

// Translucent rounded "glass" card with a sheen and a light-catching border.
class GlassPanel : public QWidget
{
    Q_OBJECT
public:
    explicit GlassPanel(QWidget *parent = nullptr, qreal radius = 18) : QWidget(parent), m_radius(radius) {}

protected:
    void paintEvent(QPaintEvent *) override;

private:
    qreal m_radius;
};

#pragma once

#include <QPlainTextEdit>

// Editable text view with a line-number gutter.
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int gutterWidth() const;
    void paintGutter(QPaintEvent *event);

    int firstVisibleLine() const;
    int lastVisibleLine() const;

signals:
    void viewChanged(); // scrolled or resized

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateGutterWidth();
    void updateGutter(const QRect &rect, int dy);

    QWidget *m_gutter;
};

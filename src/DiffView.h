#pragma once

#include "DiffEngine.h"

#include <QTimer>
#include <QVector>
#include <QWidget>

class FilePane;
class QBoxLayout;

// Two editable panes whose lines/words are highlighted from a DiffEngine result.
class DiffView : public QWidget
{
    Q_OBJECT
public:
    explicit DiffView(QWidget *parent = nullptr);

    FilePane *pane(int index) const { return m_panes[index]; }

    void setResult(const DiffEngine::Result &result);
    void clearResult();
    void refreshHighlights(); // also call after a theme change

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyHighlights(int side);

    QBoxLayout *m_layout = nullptr;
    FilePane *m_panes[2] = {nullptr, nullptr};
    DiffEngine::Result m_result;
    QVector<int> m_lineRow[2]; // line index -> row index (-1 = none)
    bool m_hasResult = false;
    bool m_syncing = false;
    QTimer m_timer;
};

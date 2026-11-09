#pragma once

#include "GlassPanel.h"

class CodeEditor;
class QLabel;
class QPushButton;

// One side of the comparison: title, Open / Save / Clear buttons and an editable text area.
class FilePane : public GlassPanel
{
    Q_OBJECT
public:
    explicit FilePane(const QString &title, QWidget *parent = nullptr);

    CodeEditor *editor() const { return m_editor; }
    void setInfo(const QString &text, const QString &tooltip = {});
    void setSaveEnabled(bool enabled);

signals:
    void openClicked();
    void saveClicked();
    void saveAsClicked();
    void clearClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void elideInfo();

    QLabel *m_info;
    QPushButton *m_save;
    CodeEditor *m_editor;
    QString m_infoFull;
};

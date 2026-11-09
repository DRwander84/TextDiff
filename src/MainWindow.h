#pragma once

#include "DiffEngine.h"

#include <QFutureWatcher>
#include <QMainWindow>
#include <QTimer>

class CaptionButton;
class DiffView;
class GlassBackdrop;
class MoveGrip;
class TitleBar;
class WindowFrame;
class GlassPanel;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#endif

private:
    struct DocInfo
    {
        QString path;
        QString encoding = QStringLiteral("UTF-8");
        bool crlf = false;
    };

    void buildUi();
    void applyWindowState();
    void applyTheme(bool dark);
    void openInto(int side);
    void loadPath(int side, const QString &path);
    bool save(int side, bool saveAs = false);
    bool confirmDiscard(int side);
    void clearSide(int side);
    void swapFiles();
    void refreshPaneInfo(int side);
    void startCompare();
    void compareFinished();
    void showStats(const DiffEngine::Stats *stats);

    DiffView *m_view = nullptr;
    GlassPanel *m_statsBar = nullptr;
    QLabel *m_stats = nullptr;
    QPushButton *m_themeButton = nullptr;
    WindowFrame *m_frame = nullptr;
    GlassBackdrop *m_backdrop = nullptr;
    CaptionButton *m_maxButton = nullptr;
    QLabel *m_sub = nullptr;
    TitleBar *m_titleBar = nullptr;
    MoveGrip *m_grip = nullptr;

    DocInfo m_doc[2];
    QString m_lastDir;

    QTimer m_compareTimer;
    QFutureWatcher<DiffEngine::Result> m_watcher;
    int m_revision[2] = {0, 0};
    DiffEngine::Stats m_lastStats;
    bool m_hasStats = false;
};

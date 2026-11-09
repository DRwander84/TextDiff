#include "MainWindow.h"

#include <QtGlobal>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#endif

#include "CodeEditor.h"
#include "DiffView.h"
#include "FilePane.h"
#include "GlassPanel.h"
#include "TextLoader.h"
#include "WindowChrome.h"
#include "Theme.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <utility>

namespace {
constexpr int kFrameMargin = 10; // transparent border = resize grips + room for the rounded frame
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("TextDiff");
    setWindowIcon(QIcon(":/icons/logo-256.png"));
    // borderless window with our own title bar and rounded corners
    setWindowFlag(Qt::FramelessWindowHint, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMinimumSize(760, 520);
    resize(1040, 660);
    setAcceptDrops(true);

    buildUi();

    m_compareTimer.setSingleShot(true);
    m_compareTimer.setInterval(250); // live comparison while typing
    connect(&m_compareTimer, &QTimer::timeout, this, &MainWindow::startCompare);
    connect(&m_watcher, &QFutureWatcher<DiffEngine::Result>::finished, this, &MainWindow::compareFinished);

    QSettings settings;
    m_lastDir = settings.value("lastDir").toString();
    applyTheme(settings.value("theme", "dark").toString() != "light");
    showStats(nullptr);
}

void MainWindow::buildUi()
{
    m_frame = new WindowFrame(this);
    setCentralWidget(m_frame);
    auto *frameLayout = new QVBoxLayout(m_frame);
    frameLayout->setContentsMargins(kFrameMargin, kFrameMargin, kFrameMargin, kFrameMargin);
    frameLayout->setSpacing(0);
    m_backdrop = new GlassBackdrop(m_frame);
    frameLayout->addWidget(m_backdrop);
    auto *root = m_backdrop;

    // ---- top bar ----
    auto *top = new TitleBar(root);
    m_titleBar = top;
    auto *topLayout = new QHBoxLayout(top);
    topLayout->setContentsMargins(18, 10, 18, 10);
    topLayout->setSpacing(12);

    auto *logo = new QLabel(top);
    logo->setPixmap(QPixmap(":/icons/logo-256.png")
                        .scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    auto *titleBox = new QVBoxLayout;
    titleBox->setSpacing(0);
    auto *title = new QLabel("TextDiff", top);
    title->setObjectName("appTitle");
    auto *sub = m_sub = new QLabel(tr("Compare two texts, line by line and word by word"), top);
    sub->setObjectName("appSub");
    titleBox->addWidget(title);
    titleBox->addWidget(sub);

    auto *swapButton = new QPushButton(tr("Swap"), top);
    swapButton->setCursor(Qt::PointingHandCursor);
    swapButton->setToolTip(tr("Swap File 1 and File 2"));

    m_themeButton = new QPushButton(top);
    m_themeButton->setCursor(Qt::PointingHandCursor);
    m_themeButton->setToolTip(tr("Switch between dark and light theme"));

    topLayout->addWidget(logo);
    topLayout->addLayout(titleBox);
    topLayout->addStretch(1);
    topLayout->addWidget(swapButton);
    topLayout->addWidget(m_themeButton);
    topLayout->addSpacing(6);

    auto *minButton = new CaptionButton(CaptionButton::Minimize, top);
    m_maxButton = new CaptionButton(CaptionButton::Maximize, top);
    auto *closeButton = new CaptionButton(CaptionButton::Close, top);
    topLayout->addWidget(minButton);
    topLayout->addWidget(m_maxButton);
    topLayout->addWidget(closeButton);

    auto toggleMaximize = [this] {
        if (isMaximized() || isFullScreen())
            showNormal();
        else
            showMaximized();
    };
    connect(minButton, &QAbstractButton::clicked, this, &QWidget::showMinimized);
    connect(m_maxButton, &QAbstractButton::clicked, this, toggleMaximize);
    connect(closeButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(top, &TitleBar::toggleMaximizeRequested, this, toggleMaximize);

    // ---- stats bar (only visible while comparing) ----
    m_statsBar = new GlassPanel(root, 16);
    auto *statsLayout = new QHBoxLayout(m_statsBar);
    statsLayout->setContentsMargins(16, 6, 16, 6);
    m_stats = new QLabel(m_statsBar);
    m_stats->setObjectName("chip");
    m_stats->setTextFormat(Qt::RichText);
    m_stats->setWordWrap(true); // keeps the minimum window width small
    statsLayout->addWidget(m_stats);
    statsLayout->addStretch(1);

    m_view = new DiffView(root);

    // bottom-right handle that moves the window
    m_grip = new MoveGrip(root);
    root->installEventFilter(this);

    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);
    layout->addWidget(top);
    layout->addWidget(m_statsBar);
    layout->addWidget(m_view, 1);

    connect(swapButton, &QPushButton::clicked, this, &MainWindow::swapFiles);
    connect(m_themeButton, &QPushButton::clicked, this, [this] { applyTheme(!theme().dark); });

    for (int side = 0; side < 2; ++side) {
        FilePane *pane = m_view->pane(side);
        CodeEditor *ed = pane->editor();
        connect(pane, &FilePane::openClicked, this, [this, side] { openInto(side); });
        connect(pane, &FilePane::saveClicked, this, [this, side] { save(side); });
        connect(pane, &FilePane::saveAsClicked, this, [this, side] { save(side, true); });
        connect(pane, &FilePane::clearClicked, this, [this, side] { clearSide(side); });
        connect(ed, &QPlainTextEdit::textChanged, this, [this, side] {
            m_compareTimer.start();
            refreshPaneInfo(side);
        });
        connect(ed->document(), &QTextDocument::modificationChanged, this, [this, side](bool) {
            refreshPaneInfo(side);
        });
    }
}

void MainWindow::applyWindowState()
{
    const bool full = isMaximized() || isFullScreen();
    const int m = full ? 0 : kFrameMargin;
    m_frame->layout()->setContentsMargins(m, m, m, m);
    m_backdrop->setRadius(full ? 0 : 22);
    m_frame->setResizeEnabled(!full);
    m_maxButton->setMaximized(full);
    m_grip->setVisible(!full);
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange)
        applyWindowState();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    m_sub->setVisible(width() >= 900); // responsive: drop the subtitle on narrow windows
}

void MainWindow::applyTheme(bool dark)
{
    theme() = dark ? Theme::darkTheme() : Theme::lightTheme();
    qApp->setStyleSheet(theme().styleSheet());
    m_themeButton->setText(dark ? tr("Light") : tr("Dark"));
    centralWidget()->update();
    for (QWidget *w : centralWidget()->findChildren<QWidget *>())
        w->update();
    m_view->refreshHighlights();
    showStats(m_hasStats ? &m_lastStats : nullptr);
    QSettings().setValue("theme", dark ? "dark" : "light");
}

void MainWindow::refreshPaneInfo(int side)
{
    const DocInfo &d = m_doc[side];
    QPlainTextEdit *ed = m_view->pane(side)->editor();
    const bool modified = ed->document()->isModified();

    QString info = d.path.isEmpty() ? tr("Untitled") : QFileInfo(d.path).fileName();
    info += QStringLiteral("  (%1 lines, %2)").arg(ed->blockCount()).arg(d.encoding);
    if (modified)
        info += QStringLiteral("  \u25CF ") + tr("modified");
    m_view->pane(side)->setInfo(info, QDir::toNativeSeparators(d.path));
    m_view->pane(side)->setSaveEnabled(modified);
}

bool MainWindow::confirmDiscard(int side)
{
    QPlainTextEdit *ed = m_view->pane(side)->editor();
    if (!ed->document()->isModified())
        return true;
    const auto answer = QMessageBox::question(
        this, tr("Unsaved changes"),
        tr("File %1 has unsaved changes. Save them first?").arg(side + 1),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (answer == QMessageBox::Cancel)
        return false;
    if (answer == QMessageBox::Save)
        return save(side);
    return true;
}

void MainWindow::openInto(int side)
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open a text file into File %1").arg(side + 1), m_lastDir, tr("All files (*)"));
    if (!path.isEmpty())
        loadPath(side, path);
}

void MainWindow::loadPath(int side, const QString &path)
{
    const TextLoader::Result r = TextLoader::load(path);
    if (!r.ok) {
        QMessageBox::warning(this, tr("Cannot use this file"),
                             QStringLiteral("%1\n\n%2").arg(QDir::toNativeSeparators(path), r.error));
        return;
    }
    if (!confirmDiscard(side))
        return;

    m_doc[side] = {path, r.encoding, r.crlf};
    m_lastDir = QFileInfo(path).absolutePath();
    QSettings().setValue("lastDir", m_lastDir);

    CodeEditor *ed = m_view->pane(side)->editor();
    ed->setPlainText(r.text);
    ed->document()->setModified(false);
    ed->document()->clearUndoRedoStacks();
    refreshPaneInfo(side);
}

bool MainWindow::save(int side, bool saveAs)
{
    DocInfo &d = m_doc[side];
    CodeEditor *ed = m_view->pane(side)->editor();

    QString path = d.path;
    if (path.isEmpty() || saveAs) {
        const QString start = path.isEmpty() ? m_lastDir : path;
        path = QFileDialog::getSaveFileName(this, tr("Save File %1").arg(side + 1), start,
                                            tr("Text files (*.txt);;All files (*)"));
        if (path.isEmpty())
            return false;
    }
    QString error;
    if (!TextLoader::save(path, ed->toPlainText(), d.encoding, d.crlf, error)) {
        QMessageBox::warning(this, tr("Cannot save"),
                             QStringLiteral("%1\n\n%2").arg(QDir::toNativeSeparators(path), error));
        return false;
    }
    d.path = path;
    m_lastDir = QFileInfo(path).absolutePath();
    ed->document()->setModified(false);
    refreshPaneInfo(side);
    return true;
}

void MainWindow::clearSide(int side)
{
    CodeEditor *ed = m_view->pane(side)->editor();
    if (ed->document()->isEmpty() && m_doc[side].path.isEmpty())
        return;
    if (!confirmDiscard(side))
        return;
    m_doc[side] = DocInfo{};
    ed->clear();
    ed->document()->setModified(false);
    ed->document()->clearUndoRedoStacks();
    refreshPaneInfo(side);
}

void MainWindow::swapFiles()
{
    CodeEditor *a = m_view->pane(0)->editor();
    CodeEditor *b = m_view->pane(1)->editor();
    const QString ta = a->toPlainText(), tb = b->toPlainText();
    const bool ma = a->document()->isModified(), mb = b->document()->isModified();

    std::swap(m_doc[0], m_doc[1]);
    a->setPlainText(tb);
    b->setPlainText(ta);
    a->document()->setModified(mb);
    b->document()->setModified(ma);
    refreshPaneInfo(0);
    refreshPaneInfo(1);
}

void MainWindow::startCompare()
{
    const QString a = m_view->pane(0)->editor()->toPlainText();
    const QString b = m_view->pane(1)->editor()->toPlainText();
    m_revision[0] = m_view->pane(0)->editor()->document()->revision();
    m_revision[1] = m_view->pane(1)->editor()->document()->revision();

    if (a.isEmpty() || b.isEmpty()) {
        m_watcher.setFuture(QFuture<DiffEngine::Result>());
        m_view->clearResult();
        showStats(nullptr);
        return;
    }
    // heavy work happens on a worker thread so typing never freezes
    m_watcher.setFuture(QtConcurrent::run([a, b] {
        return DiffEngine::compute(TextLoader::splitLines(a), TextLoader::splitLines(b));
    }));
}

void MainWindow::compareFinished()
{
    if (m_watcher.future().isCanceled())
        return;
    // ignore results for text that has been edited again in the meantime
    if (m_revision[0] != m_view->pane(0)->editor()->document()->revision()
        || m_revision[1] != m_view->pane(1)->editor()->document()->revision())
        return;
    const DiffEngine::Result result = m_watcher.result();
    m_view->setResult(result);
    showStats(&result.stats);
}

void MainWindow::showStats(const DiffEngine::Stats *s)
{
    m_hasStats = (s != nullptr);
    m_statsBar->setVisible(m_hasStats);
    if (!s)
        return;
    m_lastStats = *s;

    const Theme &t = theme();
    const QString green = t.okText.name();
    const QString red = t.badText.name();
    const QString dim = t.dimText.name(QColor::HexArgb);

    const int total = s->equal + s->modified + s->onlyInSample + s->onlyInTest;
    if (s->modified == 0 && s->onlyInSample == 0 && s->onlyInTest == 0) {
        m_stats->setText(QStringLiteral("<span style='color:%1'>&#10003; %2</span>")
                             .arg(green, tr("The files are identical (%1 lines)").arg(s->equal)));
        return;
    }
    const double similarity = total ? 100.0 * s->equal / total : 100.0;
    m_stats->setText(
        QStringLiteral("<span style='color:%1'>&#9632; %2 identical</span> &nbsp;&nbsp; "
                       "<span style='color:%3'>&#9632; %4 modified</span> &nbsp;&nbsp; "
                       "<span style='color:%3'>&#9632; %5 only in File 1</span> &nbsp;&nbsp; "
                       "<span style='color:%3'>&#9632; %6 only in File 2</span> &nbsp;&nbsp; "
                       "<span>Similarity %7%</span>")
            .arg(green)
            .arg(s->equal)
            .arg(red)
            .arg(s->modified)
            .arg(s->onlyInSample)
            .arg(s->onlyInTest)
            .arg(similarity, 0, 'f', 1));
    Q_UNUSED(dim)
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QStringList files;
    for (const QUrl &url : event->mimeData()->urls()) {
        const QString p = url.toLocalFile();
        if (!p.isEmpty() && QFileInfo(p).isFile())
            files << p;
    }
    if (files.isEmpty())
        return;
    if (files.size() > 2) {
        QMessageBox::information(this, tr("Too many files"), tr("Please drop at most two files."));
        return;
    }
    event->acceptProposedAction();

    if (files.size() == 2) {
        loadPath(0, files.at(0));
        loadPath(1, files.at(1));
    } else {
        // one file: left half of the window = File 1, right half = File 2
        loadPath(event->position().x() < width() / 2.0 ? 0 : 1, files.first());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmDiscard(0) && confirmDiscard(1))
        event->accept();
    else
        event->ignore();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_backdrop && event->type() == QEvent::Resize && m_grip) {
        m_grip->move((m_backdrop->width() - m_grip->width()) / 2, 0); // centre of the top edge
        m_grip->raise();
    }
    return QMainWindow::eventFilter(watched, event);
}

#ifdef Q_OS_WIN
// Let Windows itself handle moving (title bar caption): reliable for a borderless window and
// it gives Aero Snap for free. Resizing from the edges is handled by WindowFrame.
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType != "windows_generic_MSG")
        return QMainWindow::nativeEvent(eventType, message, result);

    MSG *msg = static_cast<MSG *>(message);
    if (msg->message == WM_NCLBUTTONDBLCLK && msg->wParam == HTCAPTION) {
        if (isMaximized())
            showNormal();
        else
            showMaximized();
        *result = 0;
        return true;
    }
    if (msg->message != WM_NCHITTEST)
        return QMainWindow::nativeEvent(eventType, message, result);

    RECT wr;
    GetWindowRect(msg->hwnd, &wr);
    const qreal dpr = devicePixelRatioF();
    const QPoint p(int((GET_X_LPARAM(msg->lParam) - wr.left) / dpr),
                   int((GET_Y_LPARAM(msg->lParam) - wr.top) / dpr));

    // The transparent border around the window belongs to us (WindowFrame resizes from it):
    // stop Windows from treating its top part as caption / native border.
    if (!isMaximized() && !isFullScreen()
        && (p.x() < kFrameMargin || p.x() >= width() - kFrameMargin || p.y() < kFrameMargin
            || p.y() >= height() - kFrameMargin)) {
        *result = HTCLIENT;
        return true;
    }

    // empty parts of our own title bar act as the window caption (drag to move)
    QWidget *w = childAt(p);
    if (w && (w == m_titleBar || m_titleBar->isAncestorOf(w)) && !qobject_cast<QAbstractButton *>(w)) {
        *result = HTCAPTION;
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

#include "FilePane.h"
#include "CodeEditor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

FilePane::FilePane(const QString &title, QWidget *parent)
    : GlassPanel(parent)
{
    auto *titleLabel = new QLabel(title, this);
    titleLabel->setObjectName("paneTitle");

    m_info = new QLabel(this);
    m_info->setObjectName("paneInfo");
    m_info->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_info->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto *open = new QPushButton(tr("Open File"), this);
    open->setObjectName("primary");
    open->setCursor(Qt::PointingHandCursor);
    open->setToolTip(tr("Load a text file into this editor"));

    m_save = new QPushButton(tr("Save"), this);
    m_save->setCursor(Qt::PointingHandCursor);
    m_save->setToolTip(tr("Save the changes"));
    m_save->setEnabled(false);

    auto *clear = new QPushButton(tr("Clear"), this);
    clear->setCursor(Qt::PointingHandCursor);
    clear->setToolTip(tr("Remove all text from this editor"));

    auto *header = new QHBoxLayout;
    header->setSpacing(8);
    header->addWidget(titleLabel);
    header->addWidget(m_info, 1);
    header->addWidget(open);
    header->addWidget(m_save);
    header->addWidget(clear);

    m_editor = new CodeEditor(this);
    m_editor->setPlaceholderText(tr("Type or paste text, drop a file here, or press \"Open File\""));

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(10);
    layout->addLayout(header);
    layout->addWidget(m_editor, 1);

    connect(open, &QPushButton::clicked, this, &FilePane::openClicked);
    connect(m_save, &QPushButton::clicked, this, [this] {
        QMenu menu(this);
        QAction *save = menu.addAction(tr("Save"));
        QAction *saveAs = menu.addAction(tr("Save As..."));
        const QAction *chosen = menu.exec(m_save->mapToGlobal(QPoint(0, m_save->height())));
        if (chosen == save)
            emit saveClicked();
        else if (chosen == saveAs)
            emit saveAsClicked();
    });
    connect(clear, &QPushButton::clicked, this, &FilePane::clearClicked);
}

void FilePane::setInfo(const QString &text, const QString &tooltip)
{
    m_infoFull = text;
    m_info->setToolTip(tooltip);
    elideInfo();
}

void FilePane::setSaveEnabled(bool enabled)
{
    m_save->setEnabled(enabled);
}

void FilePane::resizeEvent(QResizeEvent *event)
{
    GlassPanel::resizeEvent(event);
    elideInfo();
}

void FilePane::elideInfo()
{
    m_info->setText(m_info->fontMetrics().elidedText(m_infoFull, Qt::ElideMiddle, m_info->width()));
}

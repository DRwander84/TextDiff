#pragma once

#include <QColor>
#include <QMap>
#include <QString>

// "Liquid glass" theme: translucent panels floating over a colourful backdrop.
struct Theme
{
    bool dark = true;

    // backdrop
    QColor bgTop, bgBottom, blob1, blob2, blob3;
    // glass
    QColor glassFill, glassSheen, borderHi, borderLo;
    // text / controls
    QColor text, dimText, accent, accentText, okText, badText;
    QColor button, buttonBorder, buttonHover, buttonPressed, buttonDisabled;
    QColor editorBg, gutterBg, selection;
    // diff colours (translucent)
    QColor equalBlock, equalSeg, diffBlock, diffSeg;

    static Theme darkTheme()
    {
        Theme t;
        t.dark = true;
        t.bgTop = QColor(58, 61, 68);
        t.bgBottom = QColor(32, 34, 39);
        t.blob1 = QColor(170, 176, 188, 55);
        t.blob2 = QColor(120, 126, 138, 50);
        t.blob3 = QColor(200, 205, 215, 30);
        t.glassFill = QColor(255, 255, 255, 22);
        t.glassSheen = QColor(255, 255, 255, 38);
        t.borderHi = QColor(255, 255, 255, 120);
        t.borderLo = QColor(255, 255, 255, 28);
        t.text = QColor(238, 241, 255);
        t.dimText = QColor(238, 241, 255, 150);
        t.accent = QColor(110, 150, 255);
        t.accentText = QColor(255, 255, 255);
        t.okText = QColor(74, 222, 128);
        t.badText = QColor(255, 120, 135);
        t.button = QColor(255, 255, 255, 28);
        t.buttonBorder = QColor(255, 255, 255, 70);
        t.buttonHover = QColor(255, 255, 255, 55);
        t.buttonPressed = QColor(255, 255, 255, 80);
        t.buttonDisabled = QColor(255, 255, 255, 10);
        t.editorBg = QColor(27, 29, 34, 238);
        t.gutterBg = QColor(255, 255, 255, 14);
        t.selection = QColor(110, 150, 255, 140);
        t.equalBlock = QColor(52, 211, 120, 42);
        t.equalSeg = QColor(52, 211, 120, 115);
        t.diffBlock = QColor(255, 80, 100, 42);
        t.diffSeg = QColor(255, 80, 100, 125);
        return t;
    }

    static Theme lightTheme()
    {
        Theme t;
        t.dark = false;
        t.bgTop = QColor(228, 231, 236);
        t.bgBottom = QColor(190, 195, 204);
        t.blob1 = QColor(255, 255, 255, 170);
        t.blob2 = QColor(160, 168, 182, 80);
        t.blob3 = QColor(255, 255, 255, 110);
        t.glassFill = QColor(255, 255, 255, 120);
        t.glassSheen = QColor(255, 255, 255, 150);
        t.borderHi = QColor(255, 255, 255, 235);
        t.borderLo = QColor(120, 130, 170, 60);
        t.text = QColor(27, 33, 64);
        t.dimText = QColor(27, 33, 64, 150);
        t.accent = QColor(61, 109, 242);
        t.accentText = QColor(255, 255, 255);
        t.okText = QColor(21, 128, 61);
        t.badText = QColor(185, 28, 28);
        t.button = QColor(255, 255, 255, 140);
        t.buttonBorder = QColor(255, 255, 255, 220);
        t.buttonHover = QColor(255, 255, 255, 200);
        t.buttonPressed = QColor(255, 255, 255, 235);
        t.buttonDisabled = QColor(255, 255, 255, 60);
        t.editorBg = QColor(246, 247, 249, 240);
        t.gutterBg = QColor(120, 130, 170, 22);
        t.selection = QColor(61, 109, 242, 110);
        t.equalBlock = QColor(40, 180, 100, 48);
        t.equalSeg = QColor(40, 180, 100, 115);
        t.diffBlock = QColor(240, 60, 80, 42);
        t.diffSeg = QColor(240, 60, 80, 105);
        return t;
    }

    static QString css(const QColor &c)
    {
        return QStringLiteral("rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alpha());
    }

    QString styleSheet() const
    {
        QString s = QStringLiteral(R"(
QWidget { color: @text@; font-family: "Segoe UI Variable", "Segoe UI", "Inter", sans-serif; font-size: 10pt; }
QLabel { background: transparent; }
QLabel#appTitle { font-size: 17pt; font-weight: 700; }
QLabel#appSub { color: @dim@; }
QLabel#paneTitle { font-weight: 700; font-size: 11pt; }
QLabel#paneInfo { color: @dim@; }
QLabel#chip { font-weight: 600; padding: 2px 6px; }
QPushButton { background: @btn@; border: 1px solid @btnBorder@; border-radius: 10px; padding: 7px 16px; font-weight: 600; }
QPushButton:hover { background: @btnHover@; }
QPushButton:pressed { background: @btnPressed@; }
QPushButton:disabled { color: @dim@; background: @btnDisabled@; border: 1px solid transparent; }
QPushButton#primary { background: @accent@; border: 1px solid @btnBorder@; color: @accentText@; }
QPushButton#primary:hover { background: @accentHover@; }
QPushButton#primary:disabled { color: @dim@; background: @btnDisabled@; border: 1px solid transparent; }
QPlainTextEdit { background: @editorBg@; border: 1px solid @btnBorder@; border-radius: 12px; selection-background-color: @selection@; padding: 2px; }
QScrollBar:vertical { background: transparent; width: 12px; margin: 2px; }
QScrollBar:horizontal { background: transparent; height: 12px; margin: 2px; }
QScrollBar::handle { background: @btnBorder@; border-radius: 5px; min-height: 30px; min-width: 30px; }
QScrollBar::handle:hover { background: @dim@; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }
QToolTip { background: @editorBg@; color: @text@; border: 1px solid @btnBorder@; padding: 4px; }
QMessageBox, QDialog { background: @editorBg@; }
)");
        QColor accentHover = accent;
        accentHover = accentHover.lighter(115);
        const QMap<QString, QColor> map = {
            {"text", text}, {"dim", dimText}, {"btn", button}, {"btnBorder", buttonBorder},
            {"btnHover", buttonHover}, {"btnPressed", buttonPressed}, {"btnDisabled", buttonDisabled},
            {"accent", accent}, {"accentHover", accentHover}, {"accentText", accentText},
            {"editorBg", editorBg}, {"selection", selection},
        };
        for (auto it = map.begin(); it != map.end(); ++it)
            s.replace('@' + it.key() + '@', css(it.value()));
        return s;
    }
};

inline Theme &theme()
{
    static Theme t = Theme::darkTheme();
    return t;
}

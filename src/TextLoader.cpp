#include "TextLoader.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStringConverter>

#include <algorithm>

namespace TextLoader {
namespace {

constexpr qint64 kMaxFileSize = 256LL * 1024 * 1024;

bool looksBinary(const QByteArray &data)
{
    const int n = int(std::min<qsizetype>(data.size(), 64 * 1024));
    if (n == 0)
        return false;
    int control = 0;
    for (int i = 0; i < n; ++i) {
        const uchar c = uchar(data.at(i));
        if (c == 0)
            return true;
        if (c < 0x20 && c != '\t' && c != '\n' && c != '\r' && c != '\f' && c != 0x1B)
            ++control;
    }
    return control * 100 > n; // more than 1% control characters
}

QString normalise(QString text, bool &crlf)
{
    if (!text.isEmpty() && text.at(0) == QChar(0xFEFF))
        text.remove(0, 1);
    crlf = text.contains(QStringLiteral("\r\n"));
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    return text;
}

} // namespace

Result load(const QString &path)
{
    Result r;
    const QFileInfo info(path);
    if (!info.isFile()) {
        r.error = QObject::tr("This is not a file.");
        return r;
    }
    if (info.size() > kMaxFileSize) {
        r.error = QObject::tr("The file is too large (limit is 256 MB).");
        return r;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        r.error = QObject::tr("Cannot open the file: %1").arg(f.errorString());
        return r;
    }
    const QByteArray data = f.readAll();

    QString text;
    if (const auto bom = QStringConverter::encodingForData(data)) {
        // UTF-8 / UTF-16 / UTF-32 with BOM
        QStringDecoder dec(*bom);
        text = dec(data);
        r.encoding = *bom == QStringConverter::Utf8
            ? QStringLiteral("UTF-8 (BOM)")
            : QString::fromLatin1(QStringConverter::nameForEncoding(*bom));
    } else {
        if (looksBinary(data)) {
            r.error = QObject::tr("This file does not look like a text file.");
            return r;
        }
        QStringDecoder utf8(QStringConverter::Utf8, QStringConverter::Flag::Stateless);
        text = utf8(data);
        if (utf8.hasError()) {
            QStringDecoder legacy(QStringConverter::System);
            text = legacy(data);
            r.encoding = QStringLiteral("System");
        } else {
            r.encoding = QStringLiteral("UTF-8");
        }
    }

    r.text = normalise(std::move(text), r.crlf);
    r.ok = true;
    return r;
}

QStringList splitLines(const QString &text)
{
    QStringList lines = text.split(QLatin1Char('\n'), Qt::KeepEmptyParts);
    if (!lines.isEmpty() && lines.last().isEmpty())
        lines.removeLast();
    return lines;
}

bool save(const QString &path, const QString &text, const QString &encoding, bool crlf, QString &error)
{
    QString out = text;
    if (crlf)
        out.replace(QLatin1Char('\n'), QStringLiteral("\r\n"));

    QByteArray bytes;
    if (encoding == QLatin1String("System")) {
        QStringEncoder enc(QStringConverter::System);
        bytes = enc(out);
        if (enc.hasError()) {
            error = QObject::tr("The text contains characters the original encoding cannot store.");
            return false;
        }
    } else {
        const bool utf8Bom = encoding == QLatin1String("UTF-8 (BOM)");
        const QByteArray name = (utf8Bom ? QStringLiteral("UTF-8") : encoding).toLatin1();
        auto enc = QStringConverter::encodingForName(name.constData());
        const QStringConverter::Encoding e = enc ? *enc : QStringConverter::Utf8;
        const bool bom = utf8Bom || e != QStringConverter::Utf8;
        QStringEncoder encoder(e, bom ? QStringConverter::Flag::WriteBom : QStringConverter::Flag::Default);
        bytes = encoder(out);
    }

    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() || !f.commit()) {
        error = f.errorString();
        return false;
    }
    return true;
}

} // namespace TextLoader

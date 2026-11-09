#pragma once

#include <QString>
#include <QStringList>

namespace TextLoader {

struct Result
{
    bool ok = false;
    QString error;
    QString text;      // normalised: only '\n' line breaks, no BOM
    QString encoding;  // "UTF-8", "UTF-8 (BOM)", "UTF-16LE", "System", ...
    bool crlf = false; // file used Windows line endings
};

// Reads a file, verifies it contains text (not binary) and decodes it.
Result load(const QString &path);

// Writes text back using the given encoding / line ending style. Returns false and sets error on failure.
bool save(const QString &path, const QString &text, const QString &encoding, bool crlf, QString &error);

// Splits on '\n'; a trailing newline does not create an extra empty line.
QStringList splitLines(const QString &text);

} // namespace TextLoader

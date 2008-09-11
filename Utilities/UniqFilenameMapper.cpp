
#include "Utilities/UniqFilenameMapper.h"

#include <QFileInfo>

Utilities::UniqFilenameMapper::UniqFilenameMapper() {
    /* nop */
}

Utilities::UniqFilenameMapper::UniqFilenameMapper(const QString &target)
    : _targetDirectory(target) {
    /* nop */
}


void Utilities::UniqFilenameMapper::reset() {
    _uniqFiles.clear();
    _origToUniq.clear();
}

bool Utilities::UniqFilenameMapper::fileClashes(const QString &file) {
    return _uniqFiles.contains(file)
        || (!_targetDirectory.isNull() && QFileInfo(file).exists());
}

QString Utilities::UniqFilenameMapper::uniqNameFor(const QString& filename) {
    if (_origToUniq.contains(filename))
        return _origToUniq[filename];

    const QString base = QFileInfo(filename).baseName();
    const QString ext = QFileInfo(filename).completeSuffix();

    QString uniqFile;
    int i = 0;
    do {
        uniqFile = (i == 0)
            ? QString::fromAscii("%1.%2").arg(base).arg(ext)
            : QString::fromAscii("%1-%2.%3").arg(base).arg(i).arg(ext);
        if (!_targetDirectory.isNull()) {
            uniqFile = QString::fromAscii("%1/%2")
                .arg(_targetDirectory).arg(uniqFile);
        }
        ++i;
    }
    while (fileClashes(uniqFile));

    _origToUniq.insert(filename, uniqFile);
    _uniqFiles.insert(uniqFile);
    return uniqFile;
}

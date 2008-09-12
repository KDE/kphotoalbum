
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

    const QString extension = QFileInfo(filename).completeSuffix();
    QString base = QFileInfo(filename).baseName();
    if (!_targetDirectory.isNull()) {
        base = QString::fromAscii("%1/%2")
            .arg(_targetDirectory).arg(base);
    }
    
    QString uniqFile;
    int i = 0;
    do {
        uniqFile = (i == 0)
            ? QString::fromAscii("%1.%2").arg(base).arg(extension)
            : QString::fromAscii("%1-%2.%3").arg(base).arg(i).arg(extension);
        ++i;
    }
    while (fileClashes(uniqFile));

    _origToUniq.insert(filename, uniqFile);
    _uniqFiles.insert(uniqFile);
    return uniqFile;
}

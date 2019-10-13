#include "PathMapper.h"
#include "PathMappingDialog.h"
#include <QFileInfo>
#include <QSettings>

static QString escape(QString path)
{
    return path.replace("/", "-+-+-+-");
}

static QString unescape(QString path)
{
    return path.replace("-+-+-+-", "/");
}

PathMapper::PathMapper()
{
    QSettings settings("KPhotoAlbum");
    settings.beginGroup("PathMappings");
    for (auto key : settings.childKeys()) {
        m_mappings.append({ unescape(key), settings.value(key).toString() });
    }
}

bool PathMapper::configurePath(const QString &linuxPath, const QString &hostPath)
{
    PathMappingDialog dialog;
    dialog.setLinuxPath(linuxPath);
    dialog.setHostPath(hostPath);

    if (dialog.exec() == QDialog::Accepted) {
        removeMapping(linuxPath, hostPath);
        addMapping(dialog.linuxPath(), dialog.hostPath());
        emit setupChanged();
        return true;
    } else
        return false;
}

PathMapper &PathMapper::instance()
{
    static PathMapper instance;
    return instance;
}

QString PathMapper::map(const QString linuxPath)
{
    for (const Mapping &item : m_mappings) {
        if (linuxPath.startsWith(item.linuxPath))
            return item.hostPath + linuxPath.mid(item.linuxPath.length());
    }

    const QString path = QFileInfo(linuxPath).path();
    bool ok = configurePath(path, path);
    if (ok)
        return map(linuxPath);
    else
        return {};
}

void PathMapper::removeMapping(const QString &linuxPath, const QString &hostPath)
{
    m_mappings.removeOne({ linuxPath, hostPath });
    QSettings settings("KPhotoAlbum");
    settings.beginGroup("PathMappings");
    settings.remove(escape(linuxPath));
    emit setupChanged();
}

void PathMapper::addMapping(const QString &linuxPath, const QString &hostPath)
{
    m_mappings.append({ linuxPath, hostPath });
    QSettings settings("KPhotoAlbum");
    settings.beginGroup("PathMappings");
    settings.setValue(escape(linuxPath), hostPath);
}

QVector<PathMapper::Mapping> PathMapper::mappings() const
{
    return m_mappings;
}

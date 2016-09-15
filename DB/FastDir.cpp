#include "FastDir.h"

#include <QFile>

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

DB::FastDir::FastDir(const QString &path)
  : m_path(path)
{
}


QStringList DB::FastDir::entryList() const
{
    QStringList answer;
    DIR *dir;
    dirent *file;
    QByteArray path = QFile::encodeName(m_path);
    dir = opendir( path.constData() );
    if ( !dir )
        return answer; // cannot read the directory

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    // ZaJ (2016-03-23): while porting to Qt5/KF5, this code-path is disabled on my system
    //     I don't want to touch this right now since I can't verify correctness in any way.
    union dirent_buf {
        struct KDE_struct_dirent mt_file;
        char b[sizeof(struct dirent) + MAXNAMLEN + 1];
    } *u = new union dirent_buf;
    while ( readdir_r(dir, &(u->mt_file), &file ) == 0 && file )
#else
    // FIXME: use 64bit versions of readdir and dirent?
    while ( (file = readdir(dir)) )
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS
        answer.append(QFile::decodeName(file->d_name));
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    delete u;
#endif
    (void) closedir(dir);
    return answer;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "FastDir.h"
#include <kde_file.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

DB::FastDir::FastDir(const QString &path)
  : _path(path)
{
}


QStringList DB::FastDir::entryList() const
{
    QStringList answer;
    DIR *dir;
    dirent *file;
    dir = opendir( QFile::encodeName(_path) );
    if ( !dir )
    return answer; // cannot read the directory

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    union dirent_buf {
    struct KDE_struct_dirent mt_file;
    char b[sizeof(struct dirent) + MAXNAMLEN + 1];
    } *u = new union dirent_buf;
    while ( readdir_r(dir, &(u->mt_file), &file ) == 0 && file )
#else
    while ( (file = KDE_readdir(dir)) )
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS
    answer.append(QFile::decodeName(file->d_name));
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    delete u;
#endif
    (void) closedir(dir);
    return answer;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

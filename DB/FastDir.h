#ifndef FASTDIR_H
#define FASTDIR_H
#include <QStringList>
#include <QString>

namespace DB
{
/**
   FastDir is used in place of QDir because QDir stat()s every file in
   the directory, even if we tell it not to restrict anything.  When
   scanning for new images, we don't want to look at files we already
   have in our database, and we also don't want to look at files whose
   names indicate that we don't care about them.  So what we do is
   simply read the names from the directory and let the higher layers
   decide what to do with them.

   On my sample database with ~20,000 images, this improves the time
   to rescan for images on a cold system from about 100 seconds to
   about 3 seconds.

   -- Robert Krawitz, rlk@alum.mit.edu 2007-07-22
*/
class FastDir
{
public:
    FastDir(const QString &path);
    QStringList entryList() const;
private:
    const QString _path;
};

}

#endif /* FASTDIR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

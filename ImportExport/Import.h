// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMPORTEXPORT_IMPORT_H
#define IMPORTEXPORT_IMPORT_H

#include <QObject>
#include <QUrl>

class QTemporaryFile;

class KJob;
namespace KIO
{
class Job;
}

namespace ImportExport
{

class Import : public QObject
{
    Q_OBJECT

public:
    static void imageImport();
    static void imageImport(const QUrl &url);

private Q_SLOTS:
    void downloadKimJobCompleted(KJob *);
    void data(KIO::Job *, const QByteArray &);

private:
    void exec(const QString &fileName);
    void downloadUrl(const QUrl &url);

private:
    Import();
    ~Import() override;
    QTemporaryFile *m_tmp;
    QUrl m_kimFileUrl;
};

}

#endif // IMPORTEXPORT_IMPORT_H

// vi:expandtab:tabstop=4 shiftwidth=4:

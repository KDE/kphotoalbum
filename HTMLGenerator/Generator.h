/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef HTMLGENERATOR_GENERATOR_H
#define HTMLGENERATOR_GENERATOR_H

#include "Setup.h"

#include <DB/CategoryPtr.h>
#include <DB/ImageInfoPtr.h>
#include <ImageManager/ImageClientInterface.h>
#include <Utilities/UniqFilenameMapper.h>

#include <QEventLoop>
#include <QPointer>
#include <QProgressDialog>
#include <QString>
#include <QTemporaryDir>

namespace HTMLGenerator
{

class Generator : public QProgressDialog, private ImageManager::ImageClientInterface
{
    Q_OBJECT

public:
    Generator(const Setup &setup, QWidget *parent);
    ~Generator() override;
    void generate();

protected slots:
    void slotCancelGenerate();
    void showBrowser();

protected:
    bool generateIndexPage(int width, int height);
    bool generateContentPage(int width, int height,
                             const DB::FileName &prevInfo, const DB::FileName &current, const DB::FileName &nextInfo);
    bool linkIndexFile();
    QString populateDescription(QList<DB::CategoryPtr> categories, const DB::ImageInfoPtr info);

public:
    QString namePage(int width, int height, const DB::FileName &fileName);
    QString nameImage(const DB::FileName &fileName, int size);

    QString createImage(const DB::FileName &id, int size);
    QString createVideo(const DB::FileName &fileName);

    QString kimFileName(bool relative);
    bool writeToFile(const QString &fileName, const QString &str);
    int calculateSteps();
    void getThemeInfo(QString *baseDir, QString *name, QString *author);

    void pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image) override;
    int maxImageSize();
    void minImageSize(int &width, int &height);

private:
    Setup m_setup;
    int m_waitCounter;
    int m_total;
    QTemporaryDir m_tempDirHandle;
    QDir m_tempDir;
    Utilities::UniqFilenameMapper m_filenameMapper;
    QSet<QPair<DB::FileName, int>> m_generatedFiles;
    DB::FileNameSet m_copiedVideos;
    bool m_hasEnteredLoop;
    QPointer<QEventLoop> m_eventLoop;
    QString m_avconv;
};

}

#endif /* HTMLGENERATOR_GENERATOR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

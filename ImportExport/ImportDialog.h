/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORT_H
#define IMPORT_H

#include "ImportMatcher.h"
#include "ImportSettings.h"

#include <KAssistantDialog>
#include <QUrl>

class QTemporaryFile;
class QLineEdit;

namespace DB
{
class ImageInfo;
}

namespace ImportExport
{
class ImportMatcher;
class ImageRow;
class KimFileReader;
class MD5CheckPage;

/**
 * This is the wizard that configures the import process
 */
class ImportDialog : public KAssistantDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(QWidget *parent);
    // prevent hiding of base class method:
    using KAssistantDialog::exec;
    bool exec(KimFileReader *kimFileReader, const QUrl &kimFilePath);
    ImportSettings settings();

protected:
    friend class ImageRow;

    void setupPages();
    bool readFile(const QByteArray &data);
    void createIntroduction();
    void createImagesPage();
    void createDestination();
    void createCategoryPages();
    ImportMatcher *createCategoryPage(const QString &myCategory, const QString &otherCategory);
    void selectImage(bool on);
    DB::ImageInfoList selectedImages() const;
    void possiblyAddMD5CheckPage();

protected slots:
    void slotEditDestination();
    void updateNextButtonState();
    void next() override;
    void slotSelectAll();
    void slotSelectNone();
    void slotHelp();

signals:
    void failedToCopy(QStringList files);

private:
    DB::ImageInfoList m_images;
    QLineEdit *m_destinationEdit;
    KPageWidgetItem *m_destinationPage;
    KPageWidgetItem *m_categoryMatcherPage;
    KPageWidgetItem *m_dummy;
    ImportMatcher *m_categoryMatcher;
    ImportMatchers m_matchers;
    QList<ImageRow *> m_imagesSelect;
    QTemporaryFile *m_tmp;
    bool m_externalSource;
    QUrl m_kimFile;
    bool m_hasFilled;
    QUrl m_baseUrl;
    KimFileReader *m_kimFileReader;
    MD5CheckPage *m_md5CheckPage;
};
}

#endif /* IMPORT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

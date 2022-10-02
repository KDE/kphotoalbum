// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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

protected Q_SLOTS:
    void slotEditDestination();
    void updateNextButtonState();
    void next() override;
    void slotSelectAll();
    void slotSelectNone();
    void slotHelp();

Q_SIGNALS:
    void failedToCopy(QStringList files);

private:
    DB::ImageInfoList m_images;
    QLineEdit *m_destinationEdit = nullptr;
    KPageWidgetItem *m_destinationPage = nullptr;
    KPageWidgetItem *m_categoryMatcherPage = nullptr;
    KPageWidgetItem *m_dummy = nullptr;
    ImportMatcher *m_categoryMatcher = nullptr;
    ImportMatchers m_matchers;
    QList<ImageRow *> m_imagesSelect;
    QTemporaryFile *m_tmp = nullptr;
    bool m_externalSource = false;
    QUrl m_kimFile;
    bool m_hasFilled = false;
    QUrl m_baseUrl;
    KimFileReader *m_kimFileReader = nullptr;
    MD5CheckPage *m_md5CheckPage = nullptr;
};
}

#endif /* IMPORT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

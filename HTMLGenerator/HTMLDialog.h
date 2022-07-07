// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HTMLGENERATOR_HTMLDIALOG_H
#define HTMLGENERATOR_HTMLDIALOG_H

#include <kpabase/FileNameList.h>

#include <KComboBox>
#include <KPageDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTextEdit;

namespace HTMLGenerator
{
class ImageSizeCheckBox;
class Generator;
class Setup;

class HTMLDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit HTMLDialog(QWidget *parent);
    // prevent hiding of base class method:
    using KPageDialog::exec;
    int exec(const DB::FileNameList &list);

protected slots:
    void slotOk();
    void selectDir();
    void displayThemeDescription(int);
    void slotUpdateOutputLabel();
    void slotSuggestOutputDir();

protected:
    bool event(QEvent *event) override;
    bool checkVars();
    Setup setup() const;
    QList<ImageSizeCheckBox *> activeResolutions() const;
    QString activeSizes() const;
    QString includeSelections() const;
    void populateThemesCombo();
    void createContentPage();
    void createLayoutPage();
    void createDestinationPage();

private:
    QLineEdit *m_title;
    QLineEdit *m_baseDir;
    QLineEdit *m_destURL;
    QLineEdit *m_outputDir;
    QLabel *m_outputLabel;
    QCheckBox *m_openInBrowser;
    QLineEdit *m_copyright;
    QCheckBox *m_date;
    QSpinBox *m_thumbSize;
    QTextEdit *m_description;
    QSpinBox *m_numOfCols;
    QCheckBox *m_generateKimFile;
    QCheckBox *m_inlineMovies;
    QCheckBox *m_html5Video;
    QCheckBox *m_html5VideoGenerate;
    QMap<int, QString> m_themes;
    KComboBox *m_themeBox;
    QLabel *m_themeInfo;
    QStringList m_themeAuthors;
    QStringList m_themeDescriptions;
    QMap<QString, QCheckBox *> m_whatToIncludeMap;
    QList<ImageSizeCheckBox *> m_sizeCheckBoxes;
    DB::FileNameList m_list;
};
}

#endif /* HTMLGENERATOR_HTMLDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

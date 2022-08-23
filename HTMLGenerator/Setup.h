// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HTMLGENERATOR_SETUP_H
#define HTMLGENERATOR_SETUP_H

#include <kpabase/FileNameList.h>

#include <QList>
#include <QMap>
#include <QString>

namespace HTMLGenerator
{
class ImageSizeCheckBox;

class Setup
{
public:
    Setup();

    void setTitle(const QString &title);
    QString title() const;

    void setBaseDir(const QString &baseDir);
    QString baseDir() const;

    void setBaseURL(const QString &baseURL);
    QString baseURL() const;

    void setDestURL(const QString &destURL);
    QString destURL() const;

    void setOutputDir(const QString &outputDir);
    QString outputDir() const;

    void setThumbSize(int thumbSize);
    int thumbSize() const;

    void setCopyright(const QString &copyright);
    QString copyright() const;

    void setDate(bool date);
    bool date() const;

    void setDescription(const QString &description);
    QString description() const;

    void setNumOfCols(int numOfCols);
    int numOfCols() const;

    void setGenerateKimFile(bool generateKimFile);
    bool generateKimFile() const;

    void setThemePath(const QString &theme);
    QString themePath() const;

    void setIncludeCategory(const QString &category, bool include);
    bool includeCategory(const QString &category) const;

    void setResolutions(const QList<ImageSizeCheckBox *> &sizes);
    const QList<HTMLGenerator::ImageSizeCheckBox *> &activeResolutions() const;

    void setImageList(const DB::FileNameList &files);
    DB::FileNameList imageList() const;

    void setInlineMovies(bool inlineMovie);
    bool inlineMovies() const;

    void setHtml5Video(bool html5Video);
    bool html5Video() const;

    void setHtml5VideoGenerate(bool html5VideoGenerate);
    bool html5VideoGenerate() const;

private:
    QString m_title;
    QString m_baseDir;
    QString m_baseURL;
    QString m_destURL;
    QString m_outputDir;
    int m_thumbSize = -1;
    QString m_copyright;
    bool m_date = false;
    QString m_description;
    int m_numOfCols = 0;
    bool m_generateKimFile = false;
    QString m_theme;
    QMap<QString, bool> m_includeCategory;
    QList<ImageSizeCheckBox *> m_resolutions;
    DB::FileNameList m_images;
    bool m_inlineMovies = false;
    bool m_html5Video = false;
    bool m_html5VideoGenerate = false;
};

}

#endif /* HTMLGENERATOR_SETUP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

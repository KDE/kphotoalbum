/* Copyright (C) 2014-2015 Tobias Leupold <tobias.leupold@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef RECOGNIZER_H
#define RECOGNIZER_H

// Qt includes
#include <QObject>
#include <QPair>
#include <QMap>
#include <QImage>
#include <QSqlDatabase>

// libkface includes
#include <KFace/RecognitionDatabase>

namespace KFaceIface
{

// KFaceIface classes
class Identity;

}

namespace FaceManagement
{

class Recognizer : public QObject
{
    Q_OBJECT

public:
    static FaceManagement::Recognizer* instance();
    Recognizer();
    ~Recognizer();
    QPair<QString, QString> recognizeFace(const QImage& image);
    void trainRecognitionDatabase(QPair<QString, QString>& tagData, const QImage& image);
    void changeIdentityName(QString category, QString oldTagName, QString newTagName);
    QMap<QString, QStringList> allParsedIdentities();
    void deleteTag(QString category, QString tag);
    void deleteTags(QList<QPair<QString, QString>>& tagsToDelete);
    void eraseDatabase();
    void updateCategoryName(QString oldName, QString newName);
    void deleteCategory(QString category);
    int getHistogramCount(const QString category, const QString tag) const;

private: // Variables
    static FaceManagement::Recognizer* s_instance;
    KFaceIface::RecognitionDatabase m_recognitionDatabase;

private: // Functions
    void deleteIdentities(QList<KFaceIface::Identity>& identitiesToDelete);
    QPair<QString, QString> parseIdentity(KFaceIface::Identity identity);
    QString identityString(QPair<QString, QString> tagData) const;
    QString identityString(QString category, QString tag) const;
    QSqlDatabase m_recognitiondb;
};

}

#endif // RECOGNIZER_H

// vi:expandtab:tabstop=4 shiftwidth=4:

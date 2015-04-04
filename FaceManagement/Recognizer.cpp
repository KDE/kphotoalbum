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

// Qt includes
#include <QDebug>
#include <QSqlQuery>

// Local includes
#include "Recognizer.h"
#include "Settings/SettingsData.h"

using namespace KFaceIface;

FaceManagement::Recognizer* FaceManagement::Recognizer::s_instance = nullptr;

FaceManagement::Recognizer* FaceManagement::Recognizer::instance()
{
    if (! s_instance) {
        s_instance = new FaceManagement::Recognizer();
    }
    return s_instance;
}

FaceManagement::Recognizer::Recognizer()
{
    QString imageDirectory = Settings::SettingsData::instance()->imageDirectory();
    m_recognitionDatabase = RecognitionDatabase::addDatabase(imageDirectory);
    m_recognitiondb = QSqlDatabase::addDatabase(QString::fromUtf8("QSQLITE"),
                                                QString::fromUtf8("recognition"));
    m_recognitiondb.setDatabaseName(imageDirectory + QString::fromUtf8("recognition.db"));
    m_recognitiondb.open();
}

FaceManagement::Recognizer::~Recognizer()
{
    m_recognitiondb.close();
}

QPair<QString, QString> FaceManagement::Recognizer::recognizeFace(const QImage& image)
{
    if (! m_recognitionDatabase.isNull() && ! m_recognitionDatabase.allIdentities().isEmpty()) {
        Identity identity = m_recognitionDatabase.recognizeFace(image);
        if (! identity.isNull()) {
            return parseIdentity(identity);
        }
    }
    return QPair<QString, QString>();
}

void FaceManagement::Recognizer::trainRecognitionDatabase(
    QPair<QString, QString> &tagData, const QImage& image
)
{
    // Assemble an ID string for this tag
    QString fullNameString = identityString(tagData);

    // Check if we have an identity for this tag
    Identity identity = m_recognitionDatabase.findIdentity(
        QString::fromUtf8("fullName"), fullNameString
    );
    if (identity.isNull()) {
        // Add a new identity for this tag
        QMap<QString, QString> attributes;
        attributes[QString::fromUtf8("fullName")] = fullNameString;
        identity = m_recognitionDatabase.addIdentity(attributes);
    }

    // Train the database
    m_recognitionDatabase.train(identity, image, QString::fromUtf8("KPhotoAlbum"));
}

void FaceManagement::Recognizer::changeIdentityName(
    QString category, QString oldTagName, QString newTagName
)
{
    // Assemble the old ID string for this tag
    QString fullNameString = identityString(category, oldTagName);

    // Check if we have an identity for this tag
    Identity identity = m_recognitionDatabase.findIdentity(
        QString::fromUtf8("fullName"), fullNameString
    );
    if (identity.isNull()) {
        // We don't have this tag in the recognition database, so nothing has to be done
        return;
    }

    // Assemble the new ID string for this tag
    fullNameString = identityString(category, newTagName);
    QMap<QString, QString> attributes;
    attributes[QString::fromUtf8("fullName")] = fullNameString;

    // Update the recognition database
    m_recognitionDatabase.setIdentityAttributes(identity.id(), attributes);
}

QMap<QString, QStringList> FaceManagement::Recognizer::allParsedIdentities()
{
    QMap<QString, QStringList> parsedIdentities;
    QString identityFullName;
    QStringList identityParts;
    QList<Identity> allIdentities = m_recognitionDatabase.allIdentities();
    QPair<QString, QString> parsedIdentity;

    for (int i = 0; i < allIdentities.size(); ++i) {
        parsedIdentity = parseIdentity(allIdentities.at(i));
        parsedIdentities[parsedIdentity.first] << parsedIdentity.second;
    }

    return parsedIdentities;
}

void FaceManagement::Recognizer::deleteTag(QString category, QString tag)
{
    QList<QPair<QString, QString>> tagsToDelete;
    tagsToDelete << QPair<QString, QString>(category, tag);
    deleteTags(tagsToDelete);
}

void FaceManagement::Recognizer::deleteTags(QList<QPair<QString, QString>>& tagsToDelete)
{
    QList<Identity> identitiesToDelete;
    for (int i = 0; i < tagsToDelete.size(); ++i) {
        identitiesToDelete << m_recognitionDatabase.findIdentity(
            QString::fromUtf8("fullName"),
            identityString(tagsToDelete.at(i))
        );
    }

    deleteIdentities(identitiesToDelete);
}

void FaceManagement::Recognizer::deleteIdentities(QList<Identity>& identitiesToDelete)
{
    for (int i = 0; i < identitiesToDelete.size(); ++i) {
        m_recognitionDatabase.deleteIdentity(identitiesToDelete.at(i));
    }
}

void FaceManagement::Recognizer::eraseDatabase()
{
    QList<Identity> allIdentities = m_recognitionDatabase.allIdentities();
    deleteIdentities(allIdentities);
}

void FaceManagement::Recognizer::updateCategoryName(QString oldName, QString newName)
{
    QList<Identity> allIdentities = m_recognitionDatabase.allIdentities();
    if (allIdentities.size() == 0) {
        return;
    }

    // Check all identities for the outdated category name
    QPair<QString, QString> parsedIdentity;
    QString fullNameString;
    for (int i = 0; i < allIdentities.size(); ++i) {
        parsedIdentity = parseIdentity(allIdentities.at(i));

        if (parsedIdentity.first == oldName) {
            // Assemble an updated "attributes" map
            fullNameString = identityString(newName, parsedIdentity.second);
            QMap<QString, QString> attributes;
            attributes[QString::fromUtf8("fullName")] = fullNameString;

            // Update the recognition database
            m_recognitionDatabase.setIdentityAttributes(allIdentities.at(i).id(), attributes);
        }
    }
}

void FaceManagement::Recognizer::deleteCategory(QString category)
{
    QList<Identity> allIdentities = m_recognitionDatabase.allIdentities();
    if (allIdentities.size() == 0) {
        return;
    }

    QList<Identity> identitiesToDelete;

    // Check all identities for the respective category name
    QPair<QString, QString> parsedIdentity;
    for (int i = 0; i < allIdentities.size(); ++i) {
        parsedIdentity = parseIdentity(allIdentities.at(i));

        if (parsedIdentity.first == category) {
            identitiesToDelete << allIdentities.at(i);
        }
    }

    deleteIdentities(identitiesToDelete);
}

QPair<QString, QString> FaceManagement::Recognizer::parseIdentity(Identity identity)
{
    QStringList tagParts = identity.attributesMap()[QString::fromUtf8("fullName")].split(
        QString::fromUtf8("-/-")
    );
    for (QString& part : tagParts) {
        part.replace(QString::fromUtf8("//"), QString::fromUtf8("/"));
    }
    return QPair<QString, QString>(tagParts[0], tagParts[1]);
}

QString FaceManagement::Recognizer::identityString(QPair<QString, QString> tagData) const
{
    QString fullNameString = tagData.first.replace(
        QString::fromUtf8("/"), QString::fromUtf8("//")
    );
    fullNameString += QString::fromUtf8("-/-");
    fullNameString += tagData.second.replace(QString::fromUtf8("/"), QString::fromUtf8("//"));
    return fullNameString;
}

QString FaceManagement::Recognizer::identityString(QString category, QString tag) const
{
    QString fullNameString = category.replace(QString::fromUtf8("/"), QString::fromUtf8("//"));
    fullNameString += QString::fromUtf8("-/-");
    fullNameString += tag.replace(QString::fromUtf8("/"), QString::fromUtf8("//"));
    return fullNameString;
}

int FaceManagement::Recognizer::getHistogramCount(const QString category, const QString tag) const
{
    int id = m_recognitionDatabase.findIdentity(
        QString::fromUtf8("fullName"),
        identityString(QPair<QString, QString>(category, tag))
    ).id();
    QSqlQuery query(m_recognitiondb);
    query.prepare(QString::fromUtf8("SELECT COUNT(identity) "
                                    "FROM OpenCVLBPHistograms "
                                    "WHERE identity = :id"));
    query.bindValue(QString::fromUtf8(":id"), id);
    query.exec();

    if (query.next()) {
        return query.value(0).toInt();
    } else {
        return 0;
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

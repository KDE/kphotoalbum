/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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

// Local includes
#include "Settings/SettingsData.h"
#include "Recognizer.h"
#include "config-kpa-kface.h"

using namespace KFaceIface;

FaceManagement::Recognizer *FaceManagement::Recognizer::m_instance = nullptr;

FaceManagement::Recognizer * FaceManagement::Recognizer::instance()
{
    if (! m_instance) {
        m_instance = new FaceManagement::Recognizer();
    }
    return m_instance;
}

FaceManagement::Recognizer::Recognizer()
{
#ifdef HAVE_KFACE
    m_recognitionDatabase = RecognitionDatabase::addDatabase(
        Settings::SettingsData::instance()->imageDirectory()
    );
#endif
}

FaceManagement::Recognizer::~Recognizer()
{
}

QPair<QString, QString> FaceManagement::Recognizer::recognizeFace(const QImage &image)
{
#ifdef HAVE_KFACE
    if (! m_recognitionDatabase.isNull() && ! m_recognitionDatabase.allIdentities().isEmpty()) {
        Identity identity = m_recognitionDatabase.recognizeFace(image);
        if (! identity.isNull()) {
            return parseIdentity(identity);
        }
    }
#else
    Q_UNUSED(image);
#endif
    return QPair<QString, QString>();
}

void FaceManagement::Recognizer::trainRecognitionDatabase(QPair<QString, QString> &tagData,
                                                          const QImage &image)
{
#ifdef HAVE_KFACE
    // Assemble an ID string for this tag
    QString fullNameString = identityString(tagData);

    // Check if we have an identity for this tag
    Identity identity = m_recognitionDatabase.findIdentity(
        QString::fromLatin1("fullName"), fullNameString
    );
    if (identity.isNull()) {
        // Add a new identity for this tag
        QMap<QString, QString> attributes;
        attributes[QString::fromLatin1("fullName")] = fullNameString;
        identity = m_recognitionDatabase.addIdentity(attributes);
    }

    // Train the database
    m_recognitionDatabase.train(identity, image, QString::fromLatin1("KPhotoAlbum"));
#else
    Q_UNUSED(tagData);
    Q_UNUSED(image);
#endif
}

void FaceManagement::Recognizer::changeIdentityName(QString category,
                                                    QString oldTagName, QString newTagName)
{
#ifdef HAVE_KFACE
    // Assemble the old ID string for this tag
    QString fullNameString = identityString(category, oldTagName);

    // Check if we have an identity for this tag
    Identity identity = m_recognitionDatabase.findIdentity(
        QString::fromLatin1("fullName"), fullNameString
    );
    if (identity.isNull()) {
        // We don't have this tag in the recognition database, so nothing has to be done
        return;
    }

    // Assemble the new ID string for this tag
    fullNameString = identityString(category, newTagName);
    QMap<QString, QString> attributes;
    attributes[QString::fromLatin1("fullName")] = fullNameString;

    // Update the recognition database
    m_recognitionDatabase.setIdentityAttributes(identity.id(), attributes);
#else
    Q_UNUSED(category);
    Q_UNUSED(oldTagName);
    Q_UNUSED(newTagName);
#endif
}

QMap<QString, QStringList> FaceManagement::Recognizer::allParsedIdentities()
{
    QMap<QString, QStringList> parsedIdentities;
#ifdef HAVE_KFACE
    QString identityFullName;
    QStringList identityParts;
    QList<Identity> allIdentities = m_recognitionDatabase.allIdentities();

    QPair<QString, QString> parsedIdentity;

    for (int i = 0; i < allIdentities.size(); ++i) {
        parsedIdentity = parseIdentity(allIdentities.at(i));
        parsedIdentities[parsedIdentity.first] << parsedIdentity.second;
    }
#endif
    return parsedIdentities;
}

void FaceManagement::Recognizer::deleteTag(QString category, QString tag)
{
#ifdef HAVE_KFACE
    QList<QPair<QString, QString>> tagsToDelete;
    tagsToDelete << QPair<QString, QString>(category, tag);
    deleteTags(tagsToDelete);
#else
    Q_UNUSED(category);
    Q_UNUSED(tag);
#endif
}

void FaceManagement::Recognizer::deleteTags(QList<QPair<QString, QString>> &tagsToDelete)
{
#ifdef HAVE_KFACE
    QList<Identity> identitiesToDelete;
    for (int i = 0; i < tagsToDelete.size(); ++i) {
        identitiesToDelete << m_recognitionDatabase.findIdentity(
            QString::fromLatin1("fullName"),
            identityString(tagsToDelete.at(i))
        );
    }

    deleteIdentities(identitiesToDelete);
#else
    Q_UNUSED(tagsToDelete);
#endif
}

#ifdef HAVE_KFACE
void FaceManagement::Recognizer::deleteIdentities(QList<Identity> &identitiesToDelete)
{
    for (int i = 0; i < identitiesToDelete.size(); ++i) {
        m_recognitionDatabase.deleteIdentity(identitiesToDelete.at(i));
    }
}
#endif

void FaceManagement::Recognizer::eraseDatabase()
{
#ifdef HAVE_KFACE
    QList<Identity> allIdentities = m_recognitionDatabase.allIdentities();
    deleteIdentities(allIdentities);
#endif
}

void FaceManagement::Recognizer::updateCategoryName(QString oldName, QString newName)
{
#ifdef HAVE_KFACE
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
            attributes[QString::fromLatin1("fullName")] = fullNameString;

            // Update the recognition database
            m_recognitionDatabase.setIdentityAttributes(allIdentities.at(i).id(), attributes);
        }
    }
#else
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
#endif
}

void FaceManagement::Recognizer::deleteCategory(QString category)
{
#ifdef HAVE_KFACE
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
#else
    Q_UNUSED(category);
#endif
}

#ifdef HAVE_KFACE
QPair<QString, QString> FaceManagement::Recognizer::parseIdentity(Identity identity)
{
    QStringList tagParts = identity.attributesMap()[QString::fromLatin1("fullName")].split(
        QString::fromLatin1("-/-")
    );
    for (QString &part : tagParts) {
        part.replace(QString::fromLatin1("//"), QString::fromLatin1("/"));
    }
    return QPair<QString, QString>(tagParts[0], tagParts[1]);
}

QString FaceManagement::Recognizer::identityString(QPair<QString, QString> tagData) const
{
    QString fullNameString = tagData.first.replace(
        QString::fromLatin1("/"), QString::fromLatin1("//")
    );
    fullNameString += QString::fromLatin1("-/-");
    fullNameString += tagData.second.replace(QString::fromLatin1("/"), QString::fromLatin1("//"));
    return fullNameString;
}

QString FaceManagement::Recognizer::identityString(QString category, QString tag) const
{
    QString fullNameString = category.replace(QString::fromLatin1("/"), QString::fromLatin1("//"));
    fullNameString += QString::fromLatin1("-/-");
    fullNameString += tag.replace(QString::fromLatin1("/"), QString::fromLatin1("//"));
    return fullNameString;
}
#endif

// vi:expandtab:tabstop=4 shiftwidth=4:

// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2023 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Database.h"

#include "DatabaseElement.h"

#include <kpabase/Logging.h>
#include <kpabase/UIDelegate.h>

#include <KLocalizedString>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>

using namespace Exif;

namespace
{
// schema version; bump it up whenever the database schema changes
constexpr int DB_VERSION = 3;
const Database::ElementList elements(int since = 0)
{
    static Database::ElementList elms;
    static int sinceDBVersion[DB_VERSION] {};

    if (elms.count() == 0) {
        elms.append(new RationalExifElement("Exif.Photo.FocalLength"));
        elms.append(new RationalExifElement("Exif.Photo.ExposureTime"));

        elms.append(new RationalExifElement("Exif.Photo.ApertureValue"));
        elms.append(new RationalExifElement("Exif.Photo.FNumber"));
        // elms.append( new RationalExifElement( "Exif.Photo.FlashEnergy" ) );

        elms.append(new IntExifElement("Exif.Photo.Flash"));
        elms.append(new IntExifElement("Exif.Photo.Contrast"));
        elms.append(new IntExifElement("Exif.Photo.Sharpness"));
        elms.append(new IntExifElement("Exif.Photo.Saturation"));
        elms.append(new IntExifElement("Exif.Image.Orientation"));
        elms.append(new IntExifElement("Exif.Photo.MeteringMode"));
        elms.append(new IntExifElement("Exif.Photo.ISOSpeedRatings"));
        elms.append(new IntExifElement("Exif.Photo.ExposureProgram"));

        elms.append(new StringExifElement("Exif.Image.Make"));
        elms.append(new StringExifElement("Exif.Image.Model"));
        // gps info has been added in database schema version 2:
        sinceDBVersion[1] = elms.size();
        elms.append(new IntExifElement("Exif.GPSInfo.GPSVersionID")); // actually a byte value
        elms.append(new RationalExifElement("Exif.GPSInfo.GPSAltitude"));
        elms.append(new IntExifElement("Exif.GPSInfo.GPSAltitudeRef")); // actually a byte value
        elms.append(new StringExifElement("Exif.GPSInfo.GPSMeasureMode"));
        elms.append(new RationalExifElement("Exif.GPSInfo.GPSDOP"));
        elms.append(new RationalExifElement("Exif.GPSInfo.GPSImgDirection"));
        elms.append(new RationalExifElement("Exif.GPSInfo.GPSLatitude"));
        elms.append(new StringExifElement("Exif.GPSInfo.GPSLatitudeRef"));
        elms.append(new RationalExifElement("Exif.GPSInfo.GPSLongitude"));
        elms.append(new StringExifElement("Exif.GPSInfo.GPSLongitudeRef"));
        elms.append(new RationalExifElement("Exif.GPSInfo.GPSTimeStamp"));
        // lens info has been added in database schema version 3:
        sinceDBVersion[2] = elms.size();
        elms.append(new LensExifElement());
    }

    // query only for the newly added stuff:
    if (since > 0)
        return elms.mid(sinceDBVersion[since]);

    return elms;
}

bool isSQLiteDriverAvailable()
{
#ifdef QT_NO_SQL
    return false;
#else
    return QSqlDatabase::isDriverAvailable(QString::fromLatin1("QSQLITE"));
#endif
}

constexpr QFileDevice::Permissions FILE_PERMISSIONS { QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther };
}

class Database::DatabasePrivate
{
public:
    DatabasePrivate(Database *q, const QString &exifDBFile, DB::UIDelegate &uiDelegate);
    ~DatabasePrivate();

    bool isOpen() const;
    bool isUsable() const;
    int DBFileVersion() const;

    QString getFileName() const;

protected:
    Database *q_ptr;
    Q_DECLARE_PUBLIC(Database)

    enum DBSchemaChangeType { SchemaChanged,
                              SchemaAndDataChanged };
    void openDatabase();
    void populateDatabase();
    void updateDatabase();
    void createMetadataTable(DBSchemaChangeType change);
    static QString connectionName();
    bool insert(const DB::FileName &filename, Exiv2::ExifData);
    bool insert(const QList<DBExifInfo> &);

private:
    mutable bool m_isFailed = false;
    DB::UIDelegate &m_ui;
    QSqlDatabase m_db;
    const QString m_fileName;
    bool m_isOpen = false;
    bool m_doUTF8Conversion = false;
    QSqlQuery *m_insertTransaction = nullptr;
    QString m_queryString;

    void showErrorAndFail(QSqlQuery &query) const;
    void showErrorAndFail(const QString &errorMessage, const QString &technicalInfo) const;
    void init();
    QSqlQuery *getInsertQuery();
    void concludeInsertQuery(QSqlQuery *);
};

Database::DatabasePrivate::DatabasePrivate(Database *q, const QString &exifDBFile, DB::UIDelegate &uiDelegate)
    : q_ptr(q)
    , m_ui(uiDelegate)
    , m_db(QSqlDatabase::addDatabase(QString::fromLatin1("QSQLITE"), QString::fromLatin1("exif")))
    , m_fileName(exifDBFile)
{
    init();
}

void Exif::Database::DatabasePrivate::init()
{
    Q_Q(Database);
    if (!q->isAvailable())
        return;

    m_isFailed = false;
    m_insertTransaction = nullptr;
    const bool dbExists = QFile::exists(m_fileName);

    openDatabase();

    if (!isOpen())
        return;

    if (!dbExists) {
        QFile::setPermissions(m_fileName, FILE_PERMISSIONS);
        populateDatabase();
    } else
        updateDatabase();
}

/**
 * @brief show and error message for the failed \p query and disable the Exif database.
 * The database is closed because at this point we can not trust the data inside.
 * @param query
 */
void Database::DatabasePrivate::showErrorAndFail(QSqlQuery &query) const
{
    const QString txt = i18n("<p>There was an error while accessing the Exif search database. "
                             "The error is likely due to a broken database file.</p>"
                             "<p>To fix this problem run Maintenance->Recreate Exif Search database.</p>"
                             "<hr/>"
                             "<p>For debugging: the command that was attempted to be executed was:<br/>%1</p>"
                             "<p>The error message obtained was:<br/>%2</p>",
                             query.lastQuery(), query.lastError().text());

    const QString technicalInfo = QString::fromUtf8("Error running query: %1\n Error was: %2")
                                      .arg(query.lastQuery(), query.lastError().text());
    showErrorAndFail(txt, technicalInfo);
}

void Database::DatabasePrivate::showErrorAndFail(const QString &errorMessage, const QString &technicalInfo) const
{
    m_ui.information(DB::LogMessage { ExifLog(), technicalInfo }, errorMessage, i18n("Error in Exif database"), QString::fromLatin1("sql_error_in_exif_DB"));
    // disable exif db for now:
    m_isFailed = true;
}

Exif::Database::Database(const QString &sqliteFileName, DB::UIDelegate &uiDelegate)
    : d_ptr(new DatabasePrivate(this, sqliteFileName, uiDelegate))
{
}

Database::~Database()
{
    delete d_ptr;
}

void Exif::Database::DatabasePrivate::openDatabase()
{
    m_db.setDatabaseName(m_fileName);

    m_isOpen = m_db.open();
    if (!m_isOpen) {
        const QString txt = i18n("<p>There was an error while opening the Exif search database.</p> "
                                 "<p>To fix this problem run Maintenance->Recreate Exif Search database.</p>"
                                 "<hr/>"
                                 "<p>The error message obtained was:<br/>%1</p>",
                                 m_db.lastError().text());
        const QString logMsg = QString::fromUtf8("Could not open Exif search database! "
                                                 "Error was: %1")
                                   .arg(m_db.lastError().text());
        showErrorAndFail(txt, logMsg);
        return;
    }
    // If SQLite in Qt has Unicode feature, it will convert queries to
    // UTF-8 automatically. Otherwise we should do the conversion to
    // be able to store any Unicode character.
    m_doUTF8Conversion = !m_db.driver()->hasFeature(QSqlDriver::Unicode);
}

Exif::Database::DatabasePrivate::~DatabasePrivate()
{
    // We have to close the database before destroying the QSqlDatabase object,
    // otherwise Qt screams and kittens might die (see QSqlDatabase's
    // documentation)
    if (m_db.isOpen())
        m_db.close();
}

bool Exif::Database::DatabasePrivate::isOpen() const
{
    return m_isOpen && !m_isFailed;
}

void Exif::Database::DatabasePrivate::populateDatabase()
{
    createMetadataTable(SchemaAndDataChanged);
    QStringList attributes;
    const auto allElements = elements();
    for (const DatabaseElement *element : allElements) {
        attributes.append(element->createString());
    }

    QSqlQuery query(QString::fromLatin1("create table if not exists exif (filename string PRIMARY KEY, %1 )")
                        .arg(attributes.join(QString::fromLatin1(", "))),
                    m_db);
    if (!query.exec())
        showErrorAndFail(query);
}

void Exif::Database::DatabasePrivate::updateDatabase()
{
    if (m_db.tables().isEmpty()) {
        const QString txt = i18n("<p>The Exif search database is corrupted and has no data.</p> "
                                 "<p>To fix this problem run Maintenance->Recreate Exif Search database.</p>");
        const QString logMsg = QString::fromUtf8("Database open but empty!");
        showErrorAndFail(txt, logMsg);
        return;
    }

    const int version = DBFileVersion();
    if (m_isFailed)
        return;
    if (version < DBVersion()) {
        // on the next update, we can just query the DB Version
        createMetadataTable(SchemaChanged);
    }
    // update schema
    if (version < DBVersion()) {
        QSqlQuery query(m_db);
        for (const DatabaseElement *e : elements(version)) {
            query.prepare(QString::fromLatin1("alter table exif add column %1")
                              .arg(e->createString()));
            if (!query.exec())
                showErrorAndFail(query);
        }
    }
}

void Exif::Database::DatabasePrivate::createMetadataTable(DBSchemaChangeType change)
{
    QSqlQuery query(m_db);
    query.prepare(QString::fromLatin1("create table if not exists settings (keyword TEXT PRIMARY KEY, value TEXT) without rowid"));
    if (!query.exec()) {
        showErrorAndFail(query);
        return;
    }

    query.prepare(QString::fromLatin1("insert or replace into settings (keyword, value) values('DBVersion','%1')").arg(Database::DBVersion()));
    if (!query.exec()) {
        showErrorAndFail(query);
        return;
    }

    if (change == SchemaAndDataChanged) {
        query.prepare(QString::fromLatin1("insert or replace into settings (keyword, value) values('GuaranteedDataVersion','%1')").arg(Database::DBVersion()));
        if (!query.exec())
            showErrorAndFail(query);
    }
}

bool Database::add(const DB::FileName &filename, Exiv2::ExifData data)
{
    if (!isUsable())
        return false;

    Q_D(Database);
    // we might as well rename insert() to add()
    return d->insert(filename, data);
}

bool Exif::Database::add(const DB::FileName &fileName)
{
    if (!isUsable())
        return false;

    try {
        const auto image = Exiv2::ImageFactory::open(fileName.absolute().toLocal8Bit().data());
        Q_ASSERT(image.get() != nullptr);
        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        Q_D(Database);
        return d->insert(fileName, exifData);
    } catch (...) {
        qCWarning(ExifLog, "Error while reading exif information from %s", qPrintable(fileName.absolute()));
        return false;
    }
}

bool Exif::Database::add(const DB::FileNameList &list)
{
    if (!isUsable())
        return false;

    QList<DBExifInfo> map;

    for (const DB::FileName &fileName : list) {
        try {
            const auto image = Exiv2::ImageFactory::open(fileName.absolute().toLocal8Bit().data());
            Q_ASSERT(image.get() != nullptr);
            image->readMetadata();
            map << DBExifInfo(fileName, image->exifData());
        } catch (...) {
            qCWarning(ExifLog, "Error while reading exif information from %s", qPrintable(fileName.absolute()));
        }
    }
    Q_D(Database);
    d->insert(map);
    return true;
}

void Exif::Database::remove(const DB::FileName &fileName)
{
    if (!isUsable())
        return;

    Q_D(Database);
    QSqlQuery query(d->m_db);
    query.prepare(QString::fromLatin1("DELETE FROM exif WHERE fileName=?"));
    query.bindValue(0, fileName.absolute());
    if (!query.exec())
        d->showErrorAndFail(query);
}

void Exif::Database::remove(const DB::FileNameList &list)
{
    if (!isUsable())
        return;

    Q_D(Database);
    d->m_db.transaction();
    QSqlQuery query(d->m_db);
    query.prepare(QString::fromLatin1("DELETE FROM exif WHERE fileName=?"));
    for (const DB::FileName &fileName : list) {
        query.bindValue(0, fileName.absolute());
        if (!query.exec()) {
            d->m_db.rollback();
            d->showErrorAndFail(query);
            return;
        }
    }
    d->m_db.commit();
}

QSqlQuery *Exif::Database::DatabasePrivate::getInsertQuery()
{
    if (!isUsable())
        return nullptr;
    if (m_insertTransaction)
        return m_insertTransaction;
    if (m_queryString.isEmpty()) {
        QStringList formalList;
        const Database::ElementList elms = elements();
        for (const DatabaseElement *e : elms) {
            formalList.append(e->queryString());
        }
        m_queryString = QString::fromLatin1("INSERT OR REPLACE into exif values (?, %1) ").arg(formalList.join(QString::fromLatin1(", ")));
    }
    QSqlQuery *query = new QSqlQuery(m_db);
    if (query)
        query->prepare(m_queryString);
    return query;
}

void Exif::Database::DatabasePrivate::concludeInsertQuery(QSqlQuery *query)
{
    if (m_insertTransaction)
        return;
    m_db.commit();
    delete query;
}

bool Exif::Database::startInsertTransaction()
{
    if (!isUsable())
        return false;

    Q_D(Database);
    Q_ASSERT(d->m_insertTransaction == nullptr);
    d->m_insertTransaction = d->getInsertQuery();
    d->m_db.transaction();
    return (d->m_insertTransaction != nullptr);
}

bool Exif::Database::commitInsertTransaction()
{
    if (!isUsable())
        return false;

    Q_D(Database);
    if (d->m_insertTransaction) {
        d->m_db.commit();
        delete d->m_insertTransaction;
        d->m_insertTransaction = nullptr;
    } else
        qCWarning(ExifLog, "Trying to commit transaction, but no transaction is active!");
    return true;
}

bool Exif::Database::abortInsertTransaction()
{
    if (!isUsable())
        return false;

    Q_D(Database);
    if (d->m_insertTransaction) {
        d->m_db.rollback();
        delete d->m_insertTransaction;
        d->m_insertTransaction = nullptr;
    } else
        qCWarning(ExifLog, "Trying to abort transaction, but no transaction is active!");
    return true;
}

bool Exif::Database::DatabasePrivate::insert(const DB::FileName &filename, Exiv2::ExifData data)
{
    if (!isUsable())
        return false;

    QSqlQuery *query = getInsertQuery();
    query->bindValue(0, filename.absolute());
    int i = 1;
    for (const DatabaseElement *e : elements()) {
        query->bindValue(i++, e->valueFromExif(data));
    }

    bool status = query->exec();
    if (!status)
        showErrorAndFail(*query);
    concludeInsertQuery(query);
    return status;
}

bool Exif::Database::DatabasePrivate::insert(const QList<DBExifInfo> &map)
{
    if (!isUsable())
        return false;

    QSqlQuery *query = getInsertQuery();
    // not a const reference because DatabaseElement::valueFromExif uses operator[] on the exif datum
    for (DBExifInfo elt : map) {
        query->bindValue(0, elt.first.absolute());
        int i = 1;
        for (const DatabaseElement *e : elements()) {
            query->bindValue(i++, e->valueFromExif(elt.second));
        }

        if (!query->exec()) {
            showErrorAndFail(*query);
        }
    }
    concludeInsertQuery(query);
    return true;
}

QString Exif::Database::DatabasePrivate::getFileName() const
{
    return m_fileName;
}

bool Exif::Database::isAvailable()
{
    return isSQLiteDriverAvailable();
}

int Exif::Database::DatabasePrivate::DBFileVersion() const
{
    // previous to KPA 4.6, there was no metadata table:
    if (!m_db.tables().contains(QString::fromLatin1("settings")))
        return 1;

    QSqlQuery query(QString::fromLatin1("SELECT value FROM settings WHERE keyword = 'DBVersion'"), m_db);
    if (!query.exec())
        showErrorAndFail(query);

    if (query.first()) {
        return query.value(0).toInt();
    }
    return 0;
}

int Exif::Database::DBFileVersion() const
{
    Q_D(const Database);
    return d->DBFileVersion();
}

int Exif::Database::DBFileVersionGuaranteed() const
{
    Q_D(const Database);

    // previous to KPA 4.6, there was no metadata table:
    if (!d->m_db.tables().contains(QString::fromLatin1("settings")))
        return 0;

    QSqlQuery query(QString::fromLatin1("SELECT value FROM settings WHERE keyword = 'GuaranteedDataVersion'"), d->m_db);
    if (!query.exec())
        d->showErrorAndFail(query);

    if (query.first()) {
        return query.value(0).toInt();
    }
    return 0;
}

int Exif::Database::DBVersion()
{
    return DB_VERSION;
}

bool Exif::Database::DatabasePrivate::isUsable() const
{
    return isSQLiteDriverAvailable() && isOpen();
}
bool Exif::Database::isUsable() const
{
    Q_D(const Database);
    return d->isUsable();
}

bool Exif::Database::readFields(const DB::FileName &fileName, ElementList &fields) const
{
    if (!isUsable())
        return false;

    bool foundIt = false;
    QStringList fieldList;
    for (const DatabaseElement *e : fields) {
        fieldList.append(e->columnName());
    }

    Q_D(const Database);
    QSqlQuery query(d->m_db);
    // the query returns a single value, so we don't need the overhead for random access:
    query.setForwardOnly(true);

    query.prepare(QString::fromLatin1("select %1 from exif where filename=?")
                      .arg(fieldList.join(QString::fromLatin1(", "))));
    query.bindValue(0, fileName.absolute());

    if (!query.exec()) {
        d->showErrorAndFail(query);
    }
    if (query.next()) {
        // file in exif db -> write back results
        int i = 0;
        for (DatabaseElement *e : fields) {
            e->setValue(query.value(i++));
        }
        foundIt = true;
    }
    return foundIt;
}

DB::FileNameSet Exif::Database::filesMatchingQuery(const QString &queryStr) const
{
    if (!isUsable())
        return DB::FileNameSet();

    DB::FileNameSet result;
    Q_D(const Database);
    QSqlQuery query(queryStr, d->m_db);

    if (!query.exec())
        d->showErrorAndFail(query);

    else {
        if (d->m_doUTF8Conversion)
            while (query.next())
                result.insert(DB::FileName::fromAbsolutePath(QString::fromUtf8(query.value(0).toByteArray())));
        else
            while (query.next())
                result.insert(DB::FileName::fromAbsolutePath(query.value(0).toString()));
    }

    return result;
}

QList<QPair<QString, QString>> Exif::Database::cameras() const
{
    QList<QPair<QString, QString>> result;

    if (!isUsable())
        return result;

    Q_D(const Database);
    QSqlQuery query(QString::fromLatin1("SELECT DISTINCT Exif_Image_Make, Exif_Image_Model FROM exif"), d->m_db);
    if (!query.exec()) {
        d->showErrorAndFail(query);
    } else {
        while (query.next()) {
            QString make = query.value(0).toString();
            QString model = query.value(1).toString();
            if (!make.isEmpty() && !model.isEmpty())
                result.append(qMakePair(make, model));
        }
    }

    return result;
}

QList<QString> Exif::Database::lenses() const
{
    QList<QString> result;

    if (!isUsable())
        return result;

    Q_D(const Database);
    QSqlQuery query(QString::fromLatin1("SELECT DISTINCT Exif_Photo_LensModel FROM exif"), d->m_db);
    if (!query.exec()) {
        d->showErrorAndFail(query);
    } else {
        while (query.next()) {
            QString lens = query.value(0).toString();
            if (!lens.isEmpty())
                result.append(lens);
        }
    }

    return result;
}

int Database::size() const
{
    if (!isUsable())
        return 0;

    Q_D(const Database);
    QSqlQuery query(QLatin1String("SELECT count(*) FROM exif"), d->m_db);
    int result = 0;
    if (!query.exec()) {
        d->showErrorAndFail(query);
    } else {
        if (query.first()) {
            result = query.value(0).toInt();
        }
    }
    return result;
}

void Exif::Database::recreate(const DB::FileNameList &allImageFiles, DB::AbstractProgressIndicator &progressIndicator)
{
    progressIndicator.setMinimum(0);
    progressIndicator.setMaximum(allImageFiles.size());

    Q_D(Database);
    // We create a backup of the current database in case
    // the user presse 'cancel' or there is any error. In that case
    // we want to go back to the original DB.

    const QString origBackup = d->getFileName() + QLatin1String(".bak");
    d->m_db.close();

    QDir().remove(origBackup);
    QDir().rename(d->getFileName(), origBackup);
    d->init();

    // using a transaction here removes a *huge* overhead on the insert statements
    startInsertTransaction();
    int i = 0;
    for (const auto &fileName : allImageFiles) {
        progressIndicator.setValue(i++);
        add(fileName);
        if (i % 10) {
            auto app = QCoreApplication::instance();
            if (app)
                app->processEvents();
        }
        if (progressIndicator.wasCanceled())
            break;
    }

    // PENDING(blackie) We should count the amount of files that did not succeeded and warn the user.
    if (progressIndicator.wasCanceled()) {
        abortInsertTransaction();
        d->m_db.close();
        QDir().remove(d->getFileName());
        QDir().rename(origBackup, d->getFileName());
        d->init();
    } else {
        commitInsertTransaction();
        QDir().remove(origBackup);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

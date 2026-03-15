// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include <KLocalizedString>

#include "TestImageDate.h"

#include "ImageDate.h"

void KPATest::TestImageDate::initTestCase()
{
    KLocalizedString::setApplicationDomain("kphotoalbum");
}

void KPATest::TestImageDate::testParseDateString()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    QVERIFY(!DB::parseDateString(QString(), true).isNull()); // TODO: should be null?
    QVERIFY(!DB::parseDateString(QString(), false).isNull()); // TODO: should be null?

    const auto currentYear = QDate::currentDate().year();

    QCOMPARE(DB::parseDateString(QString::fromLatin1("Dec"), true), QDate(currentYear, 12, 1));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("Dec"), false), QDate(currentYear, 12, 31));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("December"), true), QDate(currentYear, 12, 1));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("December"), false), QDate(currentYear, 12, 31));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("2021"), true), QDate(2021, 1, 1));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("2021"), false), QDate(2021, 12, 31));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3. Feb. 82"), true), QDate(1982, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3. Feb. 82"), false), QDate(1982, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3. Feb. 12"), true), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3. Feb. 12"), false), QDate(2012, 2, 3));

    // Test supported delimiters.
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3-2-12"), true), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3-2-12"), false), QDate(2012, 2, 3));
    // The parseDateString() comment said commas are supported but they aren't.
    QVERIFY(DB::parseDateString(QString::fromLatin1("3,2,12"), true).isNull());
    QVERIFY(DB::parseDateString(QString::fromLatin1("3,2,12"), false).isNull());
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3.2.12"), true), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3.2.12"), false), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3 2 12"), true), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3 2 12"), false), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3/2/12"), true), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("3/2/12"), false), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("03-02-12"), true), QDate(2012, 2, 3));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("03-02-12"), false), QDate(2012, 2, 3));

    // Year < 50 is assumed to be 20xx
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-49"), true), QDate(2049, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-49"), false), QDate(2049, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-50"), true), QDate(1950, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-50"), false), QDate(1950, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-1949"), true), QDate(1949, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-1949"), false), QDate(1949, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-2049"), true), QDate(2049, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-2049"), false), QDate(2049, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-2050"), true), QDate(2050, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-2050"), false), QDate(2050, 10, 27));

    // Partial dates.
    QCOMPARE(DB::parseDateString(QString::fromLatin1("03-2020"), true), QDate(2020, 1, 1)); // TODO: bug
    QCOMPARE(DB::parseDateString(QString::fromLatin1("03-2020"), false), QDate(2020, 12, 31)); // TODO: bug
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-"), true), QDate(currentYear, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10-"), false), QDate(currentYear, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10"), true), QDate(currentYear, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-10"), false), QDate(currentYear, 10, 27));
    QCOMPARE(DB::parseDateString(QString::fromLatin1("27-"), true), QDate(currentYear, 1, 1)); // TODO: bug
    QCOMPARE(DB::parseDateString(QString::fromLatin1("5"), true), QDate(currentYear, 1, 1)); // TODO: bug
    QCOMPARE(DB::parseDateString(QString::fromLatin1("42"), true), QDate(currentYear, 1, 1)); // TODO: bug

    // Time is not supported.
    QVERIFY(!DB::parseDateString(QString::fromLatin1("13-03-2020 01:02:03"), true).isValid());
    QVERIFY(!DB::parseDateString(QString::fromLatin1("01:03:03"), true).isValid());

    QVERIFY(!DB::parseDateString(QString::fromLatin1("foo"), true).isValid());
    QVERIFY(!DB::parseDateString(QString::fromLatin1("foo"), false).isValid());
}

void KPATest::TestImageDate::testDefaultConstructor()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const DB::ImageDate id;

    QVERIFY(id.isNull());
    QVERIFY(!id.start().isValid());
    QVERIFY(!id.end().isValid());
    QVERIFY(!id.isFuzzy());
    QCOMPARE(id.toString(), QString());
    QVERIFY(id.hasValidTime()); // TODO: should be false if start() returns null?

    const DB::ImageDate today(QDate::currentDate());

    QVERIFY(id == DB::ImageDate());
    QCOMPARE(id != DB::ImageDate(), false);
    QVERIFY(id != today);
    QCOMPARE(id != DB::ImageDate(), false);
    QCOMPARE(id < DB::ImageDate(), false);
    QVERIFY(id < today);
    QVERIFY(id <= DB::ImageDate());
    QVERIFY(id <= today);

    QCOMPARE(id.isIncludedIn(today), DB::ImageDate::MatchType::NoMatch);
    QVERIFY(!id.includes(QDate::currentDate()));

    DB::ImageDate id2;
    id2.extendTo(id);
    QCOMPARE(id2, id);

    DB::ImageDate id3;
    id3.extendTo(today);
    QCOMPARE(id3, today);
}

void KPATest::TestImageDate::testFileReadConstructor()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    // Arguments <= 0 are invalid.
    QCOMPARE(DB::ImageDate(-1, -1, -1, -1, -1, -1, -1, -1, -1), DB::ImageDate());
    QCOMPARE(DB::ImageDate(0, 0, 0, 0, 0, 0, 0, 0, 0), DB::ImageDate());

    // If the first argument (yearFrom) is invalid, the rest are all ignored.
    QCOMPARE(DB::ImageDate(-1, 12, 15, 2020, 12, 25, 4, 12, 34), DB::ImageDate());

    // If any of the other *From are invalid, the rest of *From are ignored.
    QCOMPARE(DB::ImageDate(2020, -1, 25, 2020, 12, 25, 4, 12, 34), DB::ImageDate(QDate(2020, 1, 1), QDate(2020, 12, 25)));
    QCOMPARE(DB::ImageDate(2020, 12, -1, 2020, 12, 25, 4, 12, 34), DB::ImageDate(QDate(2020, 12, 1), QDate(2020, 12, 25)));
    QCOMPARE(DB::ImageDate(2020, 12, 25, -1, 12, 25, 4, 12, 34), DB::ImageDate(QDateTime(QDate(2020, 12, 25), QTime(4, 12, 34))));

    QCOMPARE(DB::ImageDate(2020, 12, 25, 2020, 12, 25, 4, 12, 34), DB::ImageDate(QDateTime(QDate(2020, 12, 25), QTime(4, 12, 34))));
    QCOMPARE(DB::ImageDate(2020, 1, 31, 2020, 12, 31, 4, 12, 34), DB::ImageDate(QDateTime(QDate(2020, 1, 31), QTime(4, 12, 34)), QDateTime(QDate(2020, 12, 31), QTime(23, 59, 59))));

    QCOMPARE(DB::ImageDate(2020, -1, -1, -1, -1, -1, -1, -1, -1), DB::ImageDate(QDate(2020, 1, 1), QDate(2020, 12, 31)));

    QCOMPARE(DB::ImageDate(2020, 3, -1, -1, -1, -1, -1, -1, -1), DB::ImageDate(QDate(2020, 3, 1), QDate(2020, 3, 31)));
    QCOMPARE(DB::ImageDate(2020, 3, 7, -1, -1, -1, -1, -1, -1), DB::ImageDate(QDate(2020, 3, 7), QDate(2020, 3, 7)));
    QCOMPARE(DB::ImageDate(2020, 3, 7, 2021, -1, -1, -1, -1, -1), DB::ImageDate(QDate(2020, 3, 7), QDate(2021, 12, 31)));
    QCOMPARE(DB::ImageDate(2020, 3, 7, 2021, 2, -1, -1, -1, -1), DB::ImageDate(QDate(2020, 3, 7), QDate(2021, 2, 28)));
    QCOMPARE(DB::ImageDate(2020, 3, 7, 2021, 2, 14, -1, -1, -1), DB::ImageDate(QDate(2020, 3, 7), QDate(2021, 2, 14)));
    QCOMPARE(DB::ImageDate(2020, 3, 7, 2021, 2, 14, 5, -1, -1), DB::ImageDate(QDateTime(QDate(2020, 3, 7), QTime(5, 0, 0)), QDateTime(QDate(2021, 2, 14), QTime(23, 59, 59))));
    QCOMPARE(DB::ImageDate(2020, 3, 7, 2021, 2, 14, 15, 30, -1), DB::ImageDate(QDateTime(QDate(2020, 3, 7), QTime(15, 30, 0)), QDateTime(QDate(2021, 2, 14), QTime(23, 59, 59))));
}

void KPATest::TestImageDate::testFastDateTimeConstructorWithInvalidDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const auto invalid = Utilities::FastDateTime(QDate(), QTime());
    const DB::ImageDate id = DB::ImageDate(invalid, invalid);
    QVERIFY(id.isNull());
    QCOMPARE(id.start(), invalid);
    QCOMPARE(id.end(), invalid);
    QVERIFY(!id.isFuzzy());
    QCOMPARE(id.toString(), QString());
    QVERIFY(id.hasValidTime()); // Because m_start == m_end.
}

void KPATest::TestImageDate::testQDateConstructorWithInvalidDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const DB::ImageDate id = DB::ImageDate(QDate(), QDate());
    // This constructor always forces the times.
    QCOMPARE(id.start(), Utilities::FastDateTime(QDate(), QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(QDate(), QTime(23, 59, 59)));
    QVERIFY(!id.isFuzzy());
    QVERIFY(!id.isNull()); // Because m_start is not null.
    QCOMPARE(id.toString(), QString());
    QVERIFY(id.hasValidTime()); // Because the times are forced.
}

void KPATest::TestImageDate::testQDateQTimeConstructorWithInvalidDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const DB::ImageDate id = DB::ImageDate(QDate(), QDate(), QTime());
    // This constructor always forces the times if the time argument is invalid.
    QCOMPARE(id.start(), Utilities::FastDateTime(QDate(), QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(QDate(), QTime(23, 59, 59)));
    QVERIFY(!id.isFuzzy());
    QVERIFY(!id.isNull()); // Because m_start is not null.
    QCOMPARE(id.toString(), QString());
    QVERIFY(id.hasValidTime()); // Because the times are forced.
}

void KPATest::TestImageDate::testFastDateTimeConstructorWithOneValidDate()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const auto date = QDate(2025, 4, 22);
    const auto time_msec = QTime(3, 4, 5, 6); // Note the msecs.
    const auto time_no_msec = QTime(3, 4, 5);
    const auto start = Utilities::FastDateTime(date, time_msec);
    const auto invalid = Utilities::FastDateTime(QDate(), QTime());
    const DB::ImageDate id = DB::ImageDate(start, invalid);
    // msec were zeroed in the constructor:
    QVERIFY(id.start() == Utilities::FastDateTime(date, time_no_msec));
    QVERIFY(id.end() == invalid);
    QVERIFY(id.isFuzzy()); // Because start != end
    QCOMPARE(id.toString(false), QString::fromLatin1("22. Apr 2025 - "));
    QCOMPARE(id.toString(true), QString::fromLatin1("22. Apr 2025 03:04 - "));
    QVERIFY(!id.hasValidTime()); // Because start != end
}

void KPATest::TestImageDate::testQDateConstructorWithOneValidDate()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate date = QDate(2025, 4, 22);
    const DB::ImageDate id = DB::ImageDate(date, QDate());
    // This constructor always forces the times.
    QCOMPARE(id.start(), Utilities::FastDateTime(date, QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(QDate(), QTime(23, 59, 59)));
    QVERIFY(id.isFuzzy()); // Because start != end
    QCOMPARE(id.toString(false), QString::fromLatin1("22. Apr 2025 - "));
    QCOMPARE(id.toString(true), QString::fromLatin1("22. Apr 2025 - "));
    QVERIFY(!id.hasValidTime()); // Because start != end
}

void KPATest::TestImageDate::testQDateQTimeConstructorWithOneValidDateAndInvalidTime()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate date = QDate(2025, 4, 22);
    const DB::ImageDate id = DB::ImageDate(date, QDate(), QTime());
    // This constructor always forces the times.
    QCOMPARE(id.start(), Utilities::FastDateTime(date, QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(date, QTime(23, 59, 59)));
    QVERIFY(id.isFuzzy()); // Because start != end
    QCOMPARE(id.toString(false), QString::fromLatin1("22. Apr 2025"));
    QCOMPARE(id.toString(true), QString::fromLatin1("22. Apr 2025"));
    QVERIFY(!id.hasValidTime()); // Because start != end
}

void KPATest::TestImageDate::testQDateQTimeConstructorWithOneValidDateAndValidTime()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate date = QDate(2025, 4, 22);
    const QTime time_msec = QTime(4, 3, 2, 1); // Note the msecs.
    const QTime time_no_msec = QTime(4, 3, 2);
    const DB::ImageDate id = DB::ImageDate(date, QDate(), time_msec);
    // The msecs are dropped.
    QCOMPARE(id.start(), Utilities::FastDateTime(date, time_no_msec));
    QCOMPARE(id.end(), Utilities::FastDateTime(date, time_no_msec));
    QVERIFY(!id.isFuzzy()); // Because start == end.
    QCOMPARE(id.toString(false), QString::fromLatin1("22. Apr 2025"));
    QCOMPARE(id.toString(true), QString::fromLatin1("22. Apr 2025 04:03:02"));
    QVERIFY(id.hasValidTime());
}

void KPATest::TestImageDate::testFastDateTimeConstructorWithTwoIdenticalDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const auto date = QDate(2025, 4, 22);
    const auto time = QTime(3, 4, 56);
    const auto start = Utilities::FastDateTime(date, time);
    const DB::ImageDate id = DB::ImageDate(start, start);
    QVERIFY(id.start() == start);
    QVERIFY(id.end() == start);
    QVERIFY(!id.isFuzzy()); // Because start == end.
    QCOMPARE(id.toString(false), QString::fromLatin1("22. Apr 2025"));
    QCOMPARE(id.toString(true), QString::fromLatin1("22. Apr 2025 03:04:56"));
    QVERIFY(id.hasValidTime());
}

void KPATest::TestImageDate::testQDateConstructorWithTwoIdenticalDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate date = QDate(2026, 1, 5);
    const DB::ImageDate id = DB::ImageDate(date, date);
    // This constructor always forces the times.
    QCOMPARE(id.start(), Utilities::FastDateTime(date, QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(date, QTime(23, 59, 59)));
    QVERIFY(id.isFuzzy()); // Because start != end due to different times.
    QCOMPARE(id.toString(), QString::fromLatin1("5. Jan 2026"));
    QVERIFY(!id.hasValidTime()); // Because start != end due to different times.
}

void KPATest::TestImageDate::testQDateQTimeConstructorWithTwoIdenticalDatesAndInvalidTime()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate date = QDate(2026, 1, 5);
    const DB::ImageDate id = DB::ImageDate(date, date, QTime());
    // This constructor always forces the times if the time is invalid.
    QCOMPARE(id.start(), Utilities::FastDateTime(date, QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(date, QTime(23, 59, 59)));
    QVERIFY(id.isFuzzy()); // Because start != end due to different times.
    QCOMPARE(id.toString(), QString::fromLatin1("5. Jan 2026"));
    QVERIFY(!id.hasValidTime()); // Because start != end due to different times.
}

void KPATest::TestImageDate::testQDateQTimeConstructorWithTwoDatesAndValidTime()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate start = QDate(2026, 1, 5);
    const QDate end = QDate(2026, 1, 6);
    const QTime time = QTime(9, 8, 7);
    const DB::ImageDate id = DB::ImageDate(end, start, time); // Note they are swapped!
    // Note supplied time is ignored:
    QCOMPARE(id.start(), Utilities::FastDateTime(start, QTime(23, 59, 59))); // TODO: time?
    // Note end is ignored:
    QCOMPARE(id.end(), Utilities::FastDateTime(end, QTime(0, 0, 0))); // TODO: time?
    QVERIFY(id.isFuzzy()); // Because start != end.
    QCOMPARE(id.toString(false), QString::fromLatin1("5. Jan 2026 - 6. Jan 2026"));
    QCOMPARE(id.toString(true), QString::fromLatin1("5. Jan 2026 23:59 - 6. Jan 2026 00:00"));
    QVERIFY(!id.hasValidTime()); // Because start != end.
}

void KPATest::TestImageDate::testFastDateTimeConstructorWithTwoValidDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const auto start = Utilities::FastDateTime(QDate(2025, 4, 22), QTime(3, 4, 5));
    const auto end = Utilities::FastDateTime(QDate(2025, 4, 30), QTime(12, 13, 14));
    const DB::ImageDate id = DB::ImageDate(end, start); // Note they are swapped!
    QVERIFY(id.start() == start);
    QVERIFY(id.end() == end);
    QVERIFY(id.isFuzzy()); // Because start != end.
    QCOMPARE(id.toString(false), QString::fromLatin1("22. Apr 2025 - 30. Apr 2025"));
    QCOMPARE(id.toString(true), QString::fromLatin1("22. Apr 2025 03:04 - 30. Apr 2025 12:13"));
    QVERIFY(!id.hasValidTime()); // Because start != end.
}

void KPATest::TestImageDate::testQDateConstructorWithTwoValidDates()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate start = QDate(2025, 1, 1);
    const QDate end = QDate(2025, 12, 31);
    const DB::ImageDate id = DB::ImageDate(end, start); // Note they are swapped!
    // This constructor always results in a valid time.
    QCOMPARE(id.start(), Utilities::FastDateTime(start, QTime(0, 0, 0)));
    QCOMPARE(id.end(), Utilities::FastDateTime(end, QTime(23, 59, 59)));
    QVERIFY(id.isFuzzy()); // Because start != end
    QCOMPARE(id.toString(), QString::fromLatin1("2025"));
    QVERIFY(!id.hasValidTime()); // Because start != end
}

void KPATest::TestImageDate::testExtendToAndIncludes()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    const QDate start = QDate(2025, 1, 1);
    const QDate end = QDate(2025, 12, 31);
    const DB::ImageDate id = DB::ImageDate(start, end);

    const QDate between = QDate(2025, 6, 21);

    QVERIFY(between != start);
    QVERIFY(between != end);
    QVERIFY(start < between);
    QVERIFY(between < end);

    DB::ImageDate idBetween(between, between);
    QVERIFY(idBetween.start() != id.start());
    QVERIFY(idBetween.end() != id.end());
    QVERIFY(idBetween != id);
    QCOMPARE(id.isIncludedIn(idBetween), DB::ImageDate::MatchType::Overlap);
    QCOMPARE(idBetween.isIncludedIn(id), DB::ImageDate::MatchType::IsContained);
    QVERIFY(id.includes(between));
    QVERIFY(!idBetween.includes(start));
    QVERIFY(!idBetween.includes(end));

    idBetween.extendTo(id);

    QVERIFY(idBetween == id);
    QVERIFY(idBetween.start() == id.start());
    QVERIFY(idBetween.end() == id.end());
    QVERIFY(idBetween.includes(start));
    QVERIFY(idBetween.includes(end));
    QCOMPARE(id.isIncludedIn(idBetween), DB::ImageDate::MatchType::IsContained);
    QCOMPARE(idBetween.isIncludedIn(id), DB::ImageDate::MatchType::IsContained);

    const QDate before = QDate(2024, 12, 31);

    QVERIFY(before < start);
    QVERIFY(before < end);

    DB::ImageDate idBefore(before, before);
    QVERIFY(idBefore.start() != id.start());
    QVERIFY(idBefore.end() != id.end());
    QVERIFY(idBefore != id);
    QCOMPARE(id.isIncludedIn(idBefore), DB::ImageDate::MatchType::NoMatch);
    QVERIFY(!id.includes(before));
    QCOMPARE(idBefore.isIncludedIn(id), DB::ImageDate::MatchType::NoMatch);
    QVERIFY(!idBefore.includes(start));
    QVERIFY(!idBefore.includes(end));

    idBefore.extendTo(id);

    QVERIFY(idBefore != id);
    QVERIFY(idBefore.start() < id.start());
    QVERIFY(idBefore.end() == id.end());
    QVERIFY(idBefore.includes(before));
    QVERIFY(idBetween.includes(start));
    QVERIFY(idBetween.includes(end));
    QCOMPARE(id.isIncludedIn(idBefore), DB::ImageDate::MatchType::IsContained);
    QCOMPARE(idBefore.isIncludedIn(id), DB::ImageDate::MatchType::Overlap);

    const QDate after = QDate(2026, 1, 1);

    QVERIFY(after > start);
    QVERIFY(after > end);

    DB::ImageDate idAfter(after, after);
    QVERIFY(idAfter.start() != id.start());
    QVERIFY(idAfter.end() != id.end());
    QVERIFY(idAfter != id);
    QCOMPARE(id.isIncludedIn(idAfter), DB::ImageDate::MatchType::NoMatch);
    QVERIFY(!id.includes(after));
    QCOMPARE(idAfter.isIncludedIn(id), DB::ImageDate::MatchType::NoMatch);
    QVERIFY(!idAfter.includes(start));
    QVERIFY(!idAfter.includes(end));

    idAfter.extendTo(id);

    QVERIFY(idAfter != id);
    QVERIFY(idAfter.start() == id.start());
    QVERIFY(idAfter.end() > id.end());
    QVERIFY(idAfter.includes(after));
    QVERIFY(idAfter.includes(start));
    QVERIFY(idAfter.includes(end));
    QCOMPARE(id.isIncludedIn(idAfter), DB::ImageDate::MatchType::IsContained);
    QCOMPARE(idAfter.isIncludedIn(id), DB::ImageDate::MatchType::Overlap);
}

void KPATest::TestImageDate::testToString()
{
    const auto restoreDefaultLocale = qScopeGuard([prior = QLocale()]() {
        QLocale::setDefault(prior);
    });
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    QCOMPARE(DB::ImageDate().toString(false), QString());
    QCOMPARE(DB::ImageDate().toString(true), QString());

    const QDate d1 = QDate(2005, 1, 1);

    QCOMPARE(DB::ImageDate(d1).toString(false), QString::fromLatin1("1. Jan 2005"));
    QCOMPARE(DB::ImageDate(d1).toString(true), QString::fromLatin1("1. Jan 2005"));

    const QTime t1 = QTime(10, 20, 30);

    QCOMPARE(DB::ImageDate(d1, d1).toString(false), QString::fromLatin1("1. Jan 2005"));
    QCOMPARE(DB::ImageDate(d1, d1).toString(true), QString::fromLatin1("1. Jan 2005"));

    QCOMPARE(DB::ImageDate(d1, d1, t1).toString(false), QString::fromLatin1("1. Jan 2005"));
    // Time is only formatted when start == end and the time is not the first second of the day.
    QCOMPARE(DB::ImageDate(d1, d1, t1).toString(true), QString::fromLatin1("1. Jan 2005 10:20:30"));

    const QTime t2 = QTime(0, 0, 0);

    QCOMPARE(DB::ImageDate(d1, d1, t2).toString(false), QString::fromLatin1("1. Jan 2005"));
    // Time is not formatted because it is the first second of the day.
    QCOMPARE(DB::ImageDate(d1, d1, t2).toString(true), QString::fromLatin1("1. Jan 2005"));

    const QDate d2 = QDate(2005, 12, 31);

    QCOMPARE(DB::ImageDate(d1, d2).toString(false), QString::fromLatin1("2005"));
    QCOMPARE(DB::ImageDate(d1, d2).toString(true), QString::fromLatin1("2005"));

    const QDate d3 = QDate(2006, 12, 31);

    QCOMPARE(DB::ImageDate(d1, d3).toString(false), QString::fromLatin1("2005 - 2006"));
    QCOMPARE(DB::ImageDate(d1, d3).toString(true), QString::fromLatin1("2005 - 2006"));

    QCOMPARE(DB::ImageDate(d1, d3, t1).toString(false), QString::fromLatin1("2005 - 2006"));
    // Time is ignored.
    QCOMPARE(DB::ImageDate(d1, d3, t1).toString(true), QString::fromLatin1("2005 - 2006"));

    QCOMPARE(DB::ImageDate(d1, d2, t1).toString(false), QString::fromLatin1("2005"));
    // Time is ignored.
    QCOMPARE(DB::ImageDate(d1, d2, t1).toString(true), QString::fromLatin1("2005"));

    const QDate d4 = QDate(2005, 1, 7);

    QCOMPARE(DB::ImageDate(d1, d4).toString(false), QString::fromLatin1("1. Jan 2005 - 7. Jan 2005"));
    QCOMPARE(DB::ImageDate(d1, d4).toString(true), QString::fromLatin1("1. Jan 2005 - 7. Jan 2005"));

    QCOMPARE(DB::ImageDate(d1, d4, t1).toString(false), QString::fromLatin1("1. Jan 2005 - 7. Jan 2005"));
    // Time is ignored.
    QCOMPARE(DB::ImageDate(d1, d4, t1).toString(true), QString::fromLatin1("1. Jan 2005 - 7. Jan 2005"));

    const QDate d5 = QDate(2005, 1, 31);

    QCOMPARE(DB::ImageDate(d1, d5).toString(false), QString::fromLatin1("Jan 2005"));
    QCOMPARE(DB::ImageDate(d1, d5).toString(true), QString::fromLatin1("Jan 2005"));

    QCOMPARE(DB::ImageDate(d1, d5, t1).toString(false), QString::fromLatin1("Jan 2005"));
    // Time is ignored.
    QCOMPARE(DB::ImageDate(d1, d5, t1).toString(true), QString::fromLatin1("Jan 2005"));

    const QDate d6 = QDate(2006, 2, 28);

    QCOMPARE(DB::ImageDate(d1, d6).toString(false), QString::fromLatin1("Jan 2005 - Feb 2006"));
    QCOMPARE(DB::ImageDate(d1, d6).toString(true), QString::fromLatin1("Jan 2005 - Feb 2006"));

    QCOMPARE(DB::ImageDate(d1, d6, t1).toString(false), QString::fromLatin1("Jan 2005 - Feb 2006"));
    // Time is ignored.
    QCOMPARE(DB::ImageDate(d1, d6, t1).toString(true), QString::fromLatin1("Jan 2005 - Feb 2006"));

    const auto start = Utilities::FastDateTime(d1, t1);
    const auto end = Utilities::FastDateTime(d1, QTime(20, 30, 40));

    QCOMPARE(DB::ImageDate(start, end).toString(false), QString::fromLatin1("1. Jan 2005"));
    // Time is reported (no seconds) because the range is less than one day.
    QCOMPARE(DB::ImageDate(start, end).toString(true), QString::fromLatin1("1. Jan 2005 10:20 - 1. Jan 2005 20:30"));

    const auto startOfDay = Utilities::FastDateTime(d1, QTime(0, 0, 0));
    const auto endOfDay = Utilities::FastDateTime(d1, QTime(23, 59, 59));

    QCOMPARE(DB::ImageDate(start, endOfDay).toString(false), QString::fromLatin1("1. Jan 2005"));
    QCOMPARE(DB::ImageDate(start, endOfDay).toString(true), QString::fromLatin1("1. Jan 2005 10:20 - 1. Jan 2005 23:59"));
    QCOMPARE(DB::ImageDate(startOfDay, end).toString(false), QString::fromLatin1("1. Jan 2005"));
    QCOMPARE(DB::ImageDate(startOfDay, end).toString(true), QString::fromLatin1("1. Jan 2005 00:00 - 1. Jan 2005 20:30"));
    QCOMPARE(DB::ImageDate(startOfDay, endOfDay).toString(false), QString::fromLatin1("1. Jan 2005"));
    // Time is not reported because the range times are start of day and end of day.
    QCOMPARE(DB::ImageDate(startOfDay, endOfDay).toString(true), QString::fromLatin1("1. Jan 2005"));
}

QTEST_MAIN(KPATest::TestImageDate)

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TestImageDate.cpp"

// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#include <KLocalizedString>

#include "TestTimespan.h"

#include "Timespan.h"

void KPATest::TestTimespan::initTestCase()
{
    KLocalizedString::setApplicationDomain("kphotoalbum");
}

void KPATest::TestTimespan::testAge()
{
    const auto birthDate = QDate(2000, 1, 1);

    // Invalid image date: TODO: the ImageDate isn't actually invalid?
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(QDate())), QString());
    // Exact image date before birth date:
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addDays(-1))), QString());
    // Image fuzzy date before birth date:
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addDays(-2), birthDate.addDays(-1))), QString());
    // Image fuzzy date spanning birth date:
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addDays(-1), birthDate.addDays(1))), QString::fromLatin1(" (up to 1 day)"));
    // Exact image date months after birth date:
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addMonths(1))), QString::fromLatin1(" (1 month)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addMonths(2))), QString::fromLatin1(" (2 months)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addMonths(11))), QString::fromLatin1(" (11 months)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addMonths(12))), QString::fromLatin1(" (1 year)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addMonths(13))), QString::fromLatin1(" (1 year, 1 month)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addMonths(14))), QString::fromLatin1(" (1 year, 2 months)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addYears(2).addMonths(11))), QString::fromLatin1(" (2 years, 11 months)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addYears(3))), QString::fromLatin1(" (3)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addYears(3).addMonths(1))), QString::fromLatin1(" (3)"));
    // Image fuzzy date after birth date:
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addYears(3), birthDate.addYears(4))), QString::fromLatin1(" (3 to 4)"));
    QCOMPARE(Timespan::age(birthDate, DB::ImageDate(birthDate.addYears(3).addMonths(1), birthDate.addYears(3).addMonths(2))), QString::fromLatin1(" (3)"));
}

void KPATest::TestTimespan::testAgo()
{
    // TimeSpan::ago() calculates relative to the current date.
    const auto today = QDate::currentDate();

    // Image start date after current date:
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(1))), QString());
    // Image end date after current date:
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-1), today.addDays(1))), QString::fromLatin1(" (yesterday)")); // TODO: bug

    QCOMPARE(Timespan::ago(DB::ImageDate(today)), QString::fromLatin1(" (today)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-1))), QString::fromLatin1(" (yesterday)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-2))), QString::fromLatin1(" (2 days ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-3))), QString::fromLatin1(" (3 days ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-7))), QString::fromLatin1(" (7 days ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-3), today.addDays(-2))), QString::fromLatin1(" (2 days to 3 days ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-14))), QString::fromLatin1(" (2 weeks ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-15))), QString::fromLatin1(" (ca. 2 weeks ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-28))), QString::fromLatin1(" (4 weeks ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-29))), QString::fromLatin1(" (ca. 4 weeks ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-49))), QString::fromLatin1(" (7 weeks ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-52))), QString::fromLatin1(" (ca. 7 weeks ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addDays(-53))), QString::fromLatin1(" (ca. 1 month ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addMonths(-2).addDays(1))), QString::fromLatin1(" (ca. 2 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addMonths(-2))), QString::fromLatin1(" (2 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addMonths(-2).addDays(-1))), QString::fromLatin1(" (ca. 2 months ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addMonths(-3), today.addMonths(-2))), QString::fromLatin1(" (2 months to 3 months ago)"));

    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1))), QString::fromLatin1(" (1 year ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-1))), QString::fromLatin1(" (1 year and 1 month ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-2))), QString::fromLatin1(" (1 year and 2 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-2).addDays(-1))), QString::fromLatin1(" (1 year and 2 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-2).addDays(-22))), QString::fromLatin1(" (1 year and 2 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-2).addDays(-23))), QString::fromLatin1(" (1 year and 3 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-2).addDays(-24))), QString::fromLatin1(" (1 year and 3 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-11))), QString::fromLatin1(" (1 year and 11 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-11).addDays(-21))), QString::fromLatin1(" (1 year and 11 months ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-1).addMonths(-11).addDays(-22))), QString::fromLatin1(" (2 years ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-2))), QString::fromLatin1(" (2 years ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-50), today.addYears(-49))), QString::fromLatin1(" (49 years to 50 years ago)"));
    QCOMPARE(Timespan::ago(DB::ImageDate(today.addYears(-50).addMonths(-5), today.addYears(-49).addMonths(5))), QString::fromLatin1(" (48 years and 7 months to 50 years and 5 months ago)"));
}

QTEST_MAIN(KPATest::TestTimespan)

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TestTimespan.cpp"

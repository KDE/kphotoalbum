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
    QFETCH(DB::ImageDate, imageDate);
    QFETCH(QDate, referenceDate);
    QFETCH(QString, result);

    QCOMPARE(Timespan::ago(imageDate, referenceDate), result);
}

void KPATest::TestTimespan::testAgo_data()
{
    QTest::addColumn<DB::ImageDate>("imageDate");
    QTest::addColumn<QDate>("referenceDate");
    QTest::addColumn<QString>("result");

    const auto today = QDate::currentDate();

    QTest::newRow("Image start date after current date") << DB::ImageDate(today.addDays(1)) << today << QString();
    QTest::newRow("Image end date after current date")
        << DB::ImageDate(today.addDays(-1), today.addDays(1))
        << today
        << QString::fromLatin1(" (yesterday)"); // TODO: bug

    QTest::newRow("today")
        << DB::ImageDate(today)
        << today
        << QString::fromLatin1(" (today)");
    QTest::newRow("yesterday")
        << DB::ImageDate(today.addDays(-1))
        << today
        << QString::fromLatin1(" (yesterday)");

    for (int days = 2; days <= 13; days++) {
        QTest::addRow("%d days ago", days)
            << DB::ImageDate(today.addDays(-days))
            << today
            << QString::fromLatin1(" (%1 days ago)").arg(days);

        QTest::addRow("%d to %d days ago", days - 1, days)
            << DB::ImageDate(today.addDays(-(days - 1)), today.addDays(-days))
            << today
            << QString::fromLatin1(" (%1 days to %2 days ago)").arg(days - 1).arg(days);
    }

    for (int weeks = 2; weeks <= 7; weeks++) {
        QTest::addRow("exactly %d weeks ago", weeks)
            << DB::ImageDate(today.addDays(-(weeks * 7)))
            << today
            << QString::fromLatin1(" (%1 weeks ago)").arg(weeks);
        if (weeks > 2) {
            // 2 weeks -1 day = 13 days
            QTest::addRow("%d weeks -1 day ago", weeks)
                << DB::ImageDate(today.addDays(-(weeks * 7) + 1))
                << today
                << QString::fromLatin1(" (ca. %1 weeks ago)").arg(weeks);
        }
        QTest::addRow("%d weeks +1 day ago", weeks)
            << DB::ImageDate(today.addDays(-(weeks * 7) - 1))
            << today
            << QString::fromLatin1(" (ca. %1 weeks ago)").arg(weeks);
    }

    // QTest::newRow("53 days ago = ~1 month")
    //     << DB::ImageDate(today.addDays(-53))
    //     << today
    //     << QString::fromLatin1(" (ca. 1 month ago)"); // TODO why? this actually returns 8 weeks ago, which seems much more sensible...

    for (int months = 2; months <= 11; months++) {
        QTest::addRow("exactly %d months ago", months)
            << DB::ImageDate(today.addMonths(-months))
            << today
            << QString::fromLatin1(" (%1 months ago)").arg(months);
        QTest::addRow("%d months +1 day ago", months)
            << DB::ImageDate(today.addMonths(-months).addDays(1))
            << today
            << QString::fromLatin1(" (ca. %1 months ago)").arg(months);
        QTest::addRow("%d months -1 day ago", months)
            << DB::ImageDate(today.addMonths(-months).addDays(-1))
            << today
            << QString::fromLatin1(" (ca. %1 months ago)").arg(months);

        QTest::addRow("%d to %d months ago", months - 1, months)
            << DB::ImageDate(today.addMonths(-(months - 1)), today.addMonths(-months))
            << today
            << QString::fromLatin1(" (%1 months to %2 months ago)").arg(months - 1).arg(months);
    }

    QTest::newRow("1 year ago")
        << DB::ImageDate(today.addYears(-1))
        << today
        << QString::fromLatin1(" (1 year ago)");
    // test more than one lifetime, but testing every year would be overkill
    for (int years = 2; years < 150; years += 7) {
        QTest::addRow("%d years ago", years)
            << DB::ImageDate(today.addYears(-years))
            << today
            << QString::fromLatin1(" (%1 years ago)").arg(years);

        QTest::addRow("%d years, 5 months, 21 days ago (round to same year)", years)
            << DB::ImageDate(today.addYears(-(years)).addMonths(5).addDays(21))
            << today
            << QString::fromLatin1(" (%1 years ago)").arg(years);

        QTest::addRow("%d years, 11 months, 22 days ago (round to next year)", years - 1)
            << DB::ImageDate(today.addYears(-(years - 1)).addMonths(-11).addDays(-22))
            << today
            << QString::fromLatin1(" (%1 years ago)").arg(years);

        QTest::addRow("%d to %d years ago", years, years + 1)
            << DB::ImageDate(today.addYears(-years), today.addYears(-(years + 1)))
            << today
            << QString::fromLatin1(" (%1 years to %2 years ago)").arg(years).arg(years + 1);
    }
    // unstable test:
    // QTest::addRow("") << DB::ImageDate(today.addYears(-1).addMonths(-11).addDays(-21)) << today << QString::fromLatin1(" (1 year and 11 months ago)");
    // QTest::addRow("") << DB::ImageDate(today.addYears(-1).addMonths(-11).addDays(-21)) << today << QString::fromLatin1(" (2 years ago)");

    QTest::newRow("1 year, 1 month ago")
        << DB::ImageDate(today.addYears(-1).addMonths(-1))
        << today
        << QString::fromLatin1(" (1 year and 1 month ago)");
    for (int months = 2; months <= 11; months++) {
        QTest::addRow("1 year, %d months ago", months)
            << DB::ImageDate(today.addYears(-1).addMonths(-months))
            << today
            << QString::fromLatin1(" (1 year and %1 months ago)").arg(months);

        QTest::addRow("1 year, %d months, 21 days ago (round down to neares month)", months)
            << DB::ImageDate(today.addYears(-1).addMonths(-months).addDays(-21))
            << today
            << QString::fromLatin1(" (1 year and %1 months ago)").arg(months);

        QTest::addRow("1 year, %d months, 22 days (round up to next month)", months)
            << DB::ImageDate(today.addYears(-1).addMonths(-months).addDays(-22))
            << today
            << QString::fromLatin1(" (1 year and %1 months ago)").arg(months + 1);
    }
    QTest::newRow("48 years, 7 months to 50 years, 5 months ago")
        << DB::ImageDate(today.addYears(-50).addMonths(-5), today.addYears(-49).addMonths(5))
        << today
        << QString::fromLatin1(" (48 years and 7 months to 50 years and 5 months ago)");
}

QTEST_MAIN(KPATest::TestTimespan)

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TestTimespan.cpp"

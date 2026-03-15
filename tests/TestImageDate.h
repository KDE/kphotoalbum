// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATEST_IMAGE_DATE_H
#define KPATEST_IMAGE_DATE_H

#include <QtTest/QTest>

namespace KPATest
{
class TestImageDate : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testDefaultConstructor();
    void testFileReadConstructor();
    void testFastDateTimeConstructorWithInvalidDates();
    void testFastDateTimeConstructorWithOneValidDate();
    void testFastDateTimeConstructorWithTwoIdenticalDates();
    void testFastDateTimeConstructorWithTwoValidDates();
    void testQDateConstructorWithInvalidDates();
    void testQDateConstructorWithOneValidDate();
    void testQDateConstructorWithTwoIdenticalDates();
    void testQDateConstructorWithTwoValidDates();
    void testQDateQTimeConstructorWithInvalidDates();
    void testQDateQTimeConstructorWithOneValidDateAndInvalidTime();
    void testQDateQTimeConstructorWithOneValidDateAndValidTime();
    void testQDateQTimeConstructorWithTwoIdenticalDatesAndInvalidTime();
    void testQDateQTimeConstructorWithTwoDatesAndValidTime();
    void testExtendToAndIncludes();
    void testParseDateString();
    void testToString();

private:
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:

// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef KPATEST_IMAGE_DATE_H
#define KPATEST_IMAGE_DATE_H

#include <QtTest/QTest>

namespace KPATest
{
class TestTimespan : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testAge();
    void testAgo();
    void testAgo_data();
};
}

#endif

// vi:expandtab:tabstop=4 shiftwidth=4:

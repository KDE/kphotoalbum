/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef DATEVIEWHANDLER_H
#define DATEVIEWHANDLER_H
#include <Utilities/FastDateTime.h>
#include <QString>

namespace DateBar
{

class ViewHandler
{
public:
    virtual ~ViewHandler() {}
    virtual void init(const Utilities::FastDateTime &startDate) = 0;
    virtual bool isMajorUnit(int unit) = 0;
    virtual bool isMidUnit(int unit);
    virtual QString text(int unit) = 0;
    virtual Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) = 0;
    virtual QString unitText() const = 0;

protected:
    Utilities::FastDateTime m_startDate;
};

class DecadeViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    bool isMidUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class YearViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    bool isMidUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class MonthViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class WeekViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class DayViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    bool isMidUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class HourViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    bool isMidUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class TenMinuteViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    bool isMidUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

class MinuteViewHandler : public ViewHandler
{
public:
    void init(const Utilities::FastDateTime &startDate) override;
    bool isMajorUnit(int unit) override;
    bool isMidUnit(int unit) override;
    QString text(int unit) override;
    Utilities::FastDateTime date(int unit, Utilities::FastDateTime reference = Utilities::FastDateTime()) override;
    QString unitText() const override;
};

}

#endif /* DATEVIEWHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>
  
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

#ifndef READVIDEOLENGTHJOB_H
#define READVIDEOLENGTHJOB_H

#include "JobInterface.h"
#include <DB/FileName.h>

namespace BackgroundTasks
{

class ReadVideoLengthJob : public JobInterface
{
    Q_OBJECT

public:
    ReadVideoLengthJob( const DB::FileName& fileName );
    OVERRIDE void execute();
    OVERRIDE JobInfo info() const;

private slots:
    void lengthFound(int);
    void unableToDetermindLength();

private:
    DB::FileName m_fileName;
};

}
#endif // READVIDEOLENGTHJOB_H

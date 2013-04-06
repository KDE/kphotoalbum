// This file was copied from git://anongit.kde.org/scratch/dakon/cmake-cxx11
/* Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
   Copyright 2012 Andreas Weis

   Distributed under the OSI-approved BSD License (the "License");
   see accompanying file Copyright.txt for details.

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
*/
#include <vector>

class seq {
public:
    seq(std::initializer_list<int> list);

    int length() const;
private:
    std::vector<int> m_v;
};

seq::seq(std::initializer_list<int> list)
    : m_v(list)
{
}

int seq::length() const
{
    return m_v.size();
}

int main(void)
{
    seq a = {18, 20, 2, 0, 4, 7};

    return (a.length() == 6) ? 0 : 1;
}

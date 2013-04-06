// This file was copied from git://anongit.kde.org/scratch/dakon/cmake-cxx11
/* Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
   Copyright 2012 Andreas Weis

   Distributed under the OSI-approved BSD License (the "License");
   see accompanying file Copyright.txt for details.

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
*/
class base {
public:
    virtual int foo(int a)
     { return 4 + a; }
    int bar(int a) final
     { return a - 2; }
};

class sub final : public base {
public:
    virtual int foo(int a) override
     { return 8 + 2 * a; };
};

int main(void)
{
    base b;
    sub s;

    return (b.foo(2) * 2 == s.foo(2)) ? 0 : 1;
}

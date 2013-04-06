// This file was copied from git://anongit.kde.org/scratch/dakon/cmake-cxx11
/* Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
   Copyright 2012 Andreas Weis

   Distributed under the OSI-approved BSD License (the "License");
   see accompanying file Copyright.txt for details.

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
*/
int Accumulate()
{
	return 0;
}

template<typename T, typename... Ts>
int Accumulate(T v, Ts... vs)
{
	return v + Accumulate(vs...);
}

template<int... Is>
int CountElements()
{
	return sizeof...(Is);
}

int main()
{
	int acc = Accumulate(1, 2, 3, 4, -5);
	int count = CountElements<1,2,3,4,5>();
	return ((acc == 5) && (count == 5)) ? 0 : 1;
}

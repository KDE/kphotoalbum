// This file was copied from git://anongit.kde.org/scratch/dakon/cmake-cxx11
/* Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
   Copyright 2012 Andreas Weis

   Distributed under the OSI-approved BSD License (the "License");
   see accompanying file Copyright.txt for details.

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
*/
int main()
{
	auto i = 5;
	auto f = 3.14159f;
	auto d = 3.14159;
	bool ret = (
		(sizeof(f) < sizeof(d)) &&
		(sizeof(i) == sizeof(int))
	);
	return ret ? 0 : 1;
}

// This file was copied from git://anongit.kde.org/scratch/dakon/cmake-cxx11
/* Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
   Copyright 2012 Andreas Weis

   Distributed under the OSI-approved BSD License (the "License");
   see accompanying file Copyright.txt for details.

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
*/
struct foo {
	char bar;
	int baz;
};

int main(void)
{
	bool ret = (
		(sizeof(foo::bar) == 1) &&
		(sizeof(foo::baz) >= sizeof(foo::bar)) &&
		(sizeof(foo) >= sizeof(foo::bar) + sizeof(foo::baz))
	);
	return ret ? 0 : 1;
}

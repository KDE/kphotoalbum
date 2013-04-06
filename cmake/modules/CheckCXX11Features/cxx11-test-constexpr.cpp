// This file was copied from git://anongit.kde.org/scratch/dakon/cmake-cxx11
/* Copyright 2011,2012 Rolf Eike Beer <eike@sf-mail.de>
   Copyright 2012 Andreas Weis

   Distributed under the OSI-approved BSD License (the "License");
   see accompanying file Copyright.txt for details.

   This software is distributed WITHOUT ANY WARRANTY; without even the
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the License for more information.
*/
constexpr int square(int x)
{
	return x*x;
}

constexpr int the_answer()
{
	return 42;
}

int main()
{
	int test_arr[square(3)];
	bool ret = (
		(square(the_answer()) == 1764) &&
		(sizeof(test_arr)/sizeof(test_arr[0]) == 9)
	);
	return ret ? 0 : 1;
}

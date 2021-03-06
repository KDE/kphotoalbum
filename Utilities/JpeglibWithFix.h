/*
  SPDX-FileCopyrightText: 2007-2010 Tuomas Suutari <thsuut@utu.fi>

  SPDX-License-Identifier: GPL-2.0-or-later

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#ifndef UTILITIES_JPEGLIBWITHFIX_H
#define UTILITIES_JPEGLIBWITHFIX_H
// FIXME: Hacky work-around for HAVE_STDLIB_H redifiniton warning.

// Some systems (like mine) have HAVE_STDLIB_H defined in jconfig.h
// which is included from jpeglib.h, but then it's also defined in our
// config.h, so GCC complains about HAVE_STDLIB_H redifinition. Here
// is my hacky fix for it. If you know a better way please let me
// know.  -- Tuomas

#ifdef HAVE_STDLIB_H
#undef HAVE_STDLIB_H
#define HAVE_STDLIB_H_WAS_DEFINED
#endif

// clang-format off
extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
}
// clang-format on

#ifdef HAVE_STDLIB_H_WAS_DEFINED
#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H
#endif
#endif

#undef HAVE_STDLIB_H_WAS_DEFINED

#endif /* UTILITIES_JPEGLIBWITHFIX_H */
// vi:expandtab:tabstop=4 shiftwidth=4:

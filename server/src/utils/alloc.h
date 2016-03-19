/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ALLOC_H_
#define _ALLOC_H_

#include <stdio.h>
#include <stdlib.h>

#define __bad_alloc() \
  (__extension__ ({ \
    fprintf(stderr, "Bad new in %s, line %d.\n", __FILE__, __LINE__); \
    exit(EXIT_FAILURE); \
  }))

#define __pnew(FUN, PARAMS) \
  (__extension__ ({ \
    void *__p = FUN PARAMS; \
    if (!__p) \
      __bad_alloc(); \
    __p; \
  }))

#define __pmalloc(P, FUN, PARAMS) \
  while (((P) = FUN PARAMS) == NULL) \
    __bad_alloc()

#define pnew(TYPE) __pnew(malloc, (sizeof(TYPE)))
#define pnew0(TYPE) __pnew(calloc, (1, sizeof(TYPE)))

#define pmalloc(P, SIZE) __pmalloc(P, malloc, (SIZE))
#define pmalloc0(P, SIZE) __pmalloc(P, calloc, (1, SIZE))
#define prealloc(P, SIZE) __prealloc(P, realloc, (P, SIZE))

#endif /* _ALLOC_H_ INCLUDED */

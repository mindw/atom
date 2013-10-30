/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "pythonhelpers.h"

#define ALIAS_GET_FLAG 0x1
#define ALIAS_SET_FLAG 0x2


typedef struct {
    PyObject_HEAD
    PyObject* name;
    PyObject* attr;
    bool observable;
    int flags;
} Alias;


extern PyTypeObject Alias_Type;


int
import_alias();

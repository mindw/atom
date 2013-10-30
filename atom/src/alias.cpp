/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include <sstream>
#include "alias.h"
#include "packagenaming.h"


using namespace PythonHelpers;


class FlagGuard
{

public:

    FlagGuard( Alias* alias, int flag ) : m_alias( alias ), m_flag( flag )
    {
        alias->flags |= flag;
    }

    ~FlagGuard()
    {
        m_alias->flags &= ~m_flag;
    }

private:

    Alias* m_alias;
    int m_flag;
};


static void
alias_error( const char* prefix )
{
    PyObject* ptype;
    PyObject* pvalue;
    PyObject* ptraceback;
    PyErr_Fetch( &ptype, &pvalue, &ptraceback );
    PyObjectPtr ptype_ptr( ptype );
    PyObjectPtr pvalue_ptr( pvalue );
    PyObjectPtr ptraceback_ptr( ptraceback );
    std::stringstream stream;
    stream << prefix << " - ";
    if( pvalue )
    {
        PyObjectPtr pystr( PyObject_Str( pvalue ) );
        if( !pystr )
            return;
        stream << PyString_AS_STRING( pystr.get() );
    }
    pvalue_ptr = PyString_FromString( stream.str().c_str() );
    if( !pvalue_ptr )
        return;
    PyErr_Restore(
        ptype_ptr.release(),
        pvalue_ptr.release(),
        ptraceback_ptr.release()
    );
}


static PyObject*
alias_get_error()
{
    alias_error( "error while getting alias" );
    return 0;
}


static int
alias_set_error()
{
    alias_error( "error while setting alias" );
    return -1;
}


static PyObject*
Alias_new( PyTypeObject* type, PyObject* args, PyObject* kwargs )
{
    PyObject* name;
    PyObject* attr = Py_None;
    PyObject* pybool = 0;
    static char *kwlist[] = { "name", "attr", "observable", 0 };
    if( !PyArg_ParseTupleAndKeywords( args, kwargs, "S|SO!:__new__",
        kwlist, &name, &attr, &PyBool_Type, &pybool ) )
        return 0;
    PyObject* self = PyType_GenericNew( type, 0, 0 );
    if( !self )
        return 0;
    Alias* alias = reinterpret_cast<Alias*>( self );
    alias->name = newref( name );
    alias->attr = newref( attr );
    alias->observable = pybool == Py_True ? true : false;
    return self;
}


static void
Alias_dealloc( Alias* self )
{
    Py_CLEAR( self->name );
    Py_CLEAR( self->attr );
    self->ob_type->tp_free( pyobject_cast( self ) );
}


static PyObject*
Alias__get__( Alias* self, PyObject* object, PyObject* type )
{
    if( !object )
        return newref( pyobject_cast( self ) );
    if( self->flags & ALIAS_GET_FLAG )
    {
        PyErr_SetString(PyExc_RuntimeError, "recursion detected");
        return 0;
    }
    FlagGuard guard( self, ALIAS_GET_FLAG );
    PyObjectPtr other( PyObject_GetAttr( object, self->name ) );
    if( !other )
        return alias_get_error();
    if( self->attr == Py_None )
        return other.release();
    PyObjectPtr value( PyObject_GetAttr( other.get(), self->attr ) );
    if( !value )
        return alias_get_error();
    return value.release();
}


static int
Alias__set__( Alias* self, PyObject* object, PyObject* value )
{
    if( self->flags & ALIAS_SET_FLAG )
    {
        PyErr_SetString(PyExc_RuntimeError, "recursion detected");
        return -1;
    }
    FlagGuard guard( self, ALIAS_SET_FLAG );
    if( self->attr == Py_None )
    {
        if( PyObject_SetAttr( object, self->name, value ) < 0 )
            return alias_set_error();
        return 0;
    }
    PyObjectPtr other( PyObject_GetAttr( object, self->name ) );
    if( !other )
        return alias_set_error();
    if( PyObject_SetAttr( other.get(), self->attr, value ) < 0 )
        return alias_set_error();
    return 0;
}


static PyObject*
Alias_get_name( Alias* self, void* ctxt )
{
    return newref( self->name );
}


static PyObject*
Alias_get_attr( Alias* self, void* ctxt )
{
    return newref( self->attr );
}


static PyObject*
Alias_get_observable( Alias* self, void* ctxt )
{
    return newref( self->observable ? Py_True : Py_False );
}


static PyGetSetDef
Alias_getset[] = {
    { "name", ( getter )Alias_get_name, 0,
      "Get the name of the alias" },
    { "attr", ( getter )Alias_get_attr, 0,
      "Get the attr for the alias." },
    { "observable", ( getter )Alias_get_observable, 0,
      "Get the observable flag for the alias." },
    { 0 } // sentinel
};


PyTypeObject Alias_Type = {
    PyObject_HEAD_INIT( 0 )
    0,                                      /* ob_size */
    PACKAGE_TYPENAME( "Alias" ),            /* tp_name */
    sizeof( Alias ),                        /* tp_basicsize */
    0,                                      /* tp_itemsize */
    (destructor)Alias_dealloc,              /* tp_dealloc */
    (printfunc)0,                           /* tp_print */
    (getattrfunc)0,                         /* tp_getattr */
    (setattrfunc)0,                         /* tp_setattr */
    (cmpfunc)0,                             /* tp_compare */
    (reprfunc)0,                            /* tp_repr */
    (PyNumberMethods*)0,                    /* tp_as_number */
    (PySequenceMethods*)0,                  /* tp_as_sequence */
    (PyMappingMethods*)0,                   /* tp_as_mapping */
    (hashfunc)0,                            /* tp_hash */
    (ternaryfunc)0,                         /* tp_call */
    (reprfunc)0,                            /* tp_str */
    (getattrofunc)0,                        /* tp_getattro */
    (setattrofunc)0,                        /* tp_setattro */
    (PyBufferProcs*)0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    0,                                      /* Documentation string */
    (traverseproc)0,                        /* tp_traverse */
    (inquiry)0,                             /* tp_clear */
    (richcmpfunc)0,                         /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)0,                         /* tp_iter */
    (iternextfunc)0,                        /* tp_iternext */
    (struct PyMethodDef*)0,                 /* tp_methods */
    (struct PyMemberDef*)0,                 /* tp_members */
    Alias_getset,                           /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    (descrgetfunc)Alias__get__,             /* tp_descr_get */
    (descrsetfunc)Alias__set__,             /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    (allocfunc)PyType_GenericAlloc,         /* tp_alloc */
    (newfunc)Alias_new,                     /* tp_new */
    (freefunc)PyObject_Del,                 /* tp_free */
    (inquiry)0,                             /* tp_is_gc */
    0,                                      /* tp_bases */
    0,                                      /* tp_mro */
    0,                                      /* tp_cache */
    0,                                      /* tp_subclasses */
    0,                                      /* tp_weaklist */
    (destructor)0                           /* tp_del */
};


int
import_alias()
{
    if( PyType_Ready( &Alias_Type ) < 0 )
        return -1;
    return 0;
}

/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#include "aliasguard.h"
#include "member.h"
#include "memberchange.h"


using namespace PythonHelpers;


bool
Member::check_context( DelAttr::Mode mode, PyObject* context )
{
    switch( mode )
    {
        case DelAttr::Delegate:
            if( !Member::TypeCheck( context ) )
            {
                py_expected_type_fail( context, "Member" );
                return false;
            }
            break;
        case DelAttr::Alias:
            if( !PyTuple_CheckExact( context ) )
            {
                py_expected_type_fail( context, "2-tuple of str" );
                return false;
            }
            if( PyTuple_GET_SIZE( context ) != 2 )
            {
                py_expected_type_fail( context, "2-tuple of str" );
                return false;
            }
            if( !PyString_Check( PyTuple_GET_ITEM( context, 0 ) ) )
            {
                py_expected_type_fail( context, "2-tuple of str" );
                return false;
            }
            if( !PyString_Check( PyTuple_GET_ITEM( context, 1 ) ) )
            {
                py_expected_type_fail( context, "2-tuple of str" );
                return false;
            }
            break;
        default:
            break;
    }
    return true;
}


static int
no_op_handler( Member* member, CAtom* atom )
{
    return 0;
}


static PyObject*
deleted_args( CAtom* atom, Member* member, PyObject* value )
{
    PyTuplePtr argsptr( PyTuple_New( 1 ) );
    if( !argsptr )
        return 0;
    PyObjectPtr changeptr( MemberChange::deleted( atom, member, value ) );
    if( !changeptr )
        return 0;
    argsptr.initialize( 0, changeptr );
    return argsptr.release();
}


static int
slot_handler( Member* member, CAtom* atom )
{
    if( member->index >= atom->get_slot_count() )
    {
        py_no_attr_fail( pyobject_cast( atom ), PyString_AsString( member->name ) );
        return -1;
    }
    PyObjectPtr valueptr( atom->get_slot( member->index ) );
    if( !valueptr )
        return 0;
    atom->set_slot( member->index, 0 );
    if( atom->get_notifications_enabled() )
    {
        PyObjectPtr argsptr;
        if( member->has_observers() )
        {
            argsptr = deleted_args( atom, member, valueptr.get() );
            if( !argsptr )
                return -1;
            if( !member->notify( atom, argsptr.get(), 0 ) )
                return -1;
        }
        if( atom->has_observers( member->name ) )
        {
            if( !argsptr )
            {
                argsptr = deleted_args( atom, member, valueptr.get() );
                if( !argsptr )
                    return -1;
            }
            if( !atom->notify( member->name, argsptr.get(), 0 ) )
                return -1;
        }
    }
    return 0;
}


static int
constant_handler( Member* member, CAtom* atom )
{
    py_type_fail( "cannot delete the value of a constant member" );
    return -1;
}


static int
read_only_handler( Member* member, CAtom* atom )
{
    py_type_fail( "cannot delete the value of a read only member" );
    return -1;
}


static int
event_handler( Member* member, CAtom* atom )
{
    py_type_fail( "cannot delete the value of an event" );
    return -1;
}


static int
signal_handler( Member* member, CAtom* atom )
{
    py_type_fail( "cannot delete the value of a signal" );
    return -1;
}


static int
delegate_handler( Member* member, CAtom* atom )
{
    Member* delegate = member_cast( member->delattr_context );
    return delegate->delattr( atom );
}


static int
alias_handler( Member* member, CAtom* atom )
{
    if( AliasGuard::guarded( member, atom ) )
    {
        py_runtime_fail( "alias recursion detected" );
        return -1;
    }
    AliasGuard guard( member, atom );
    PyObject* name = PyTuple_GET_ITEM( member->getattr_context, 0 );
    PyObject* attr = PyTuple_GET_ITEM( member->getattr_context, 1 );
    PyObjectPtr other( PyObject_GetAttr( pyobject_cast( atom ), name ) );
    if( !other )
        return -1;
    return PyObject_SetAttr( other.get(), attr, 0 );
}


typedef int
( *handler )( Member* member, CAtom* atom );


static handler
handlers[] = {
    no_op_handler,
    slot_handler,
    constant_handler,
    read_only_handler,
    event_handler,
    signal_handler,
    delegate_handler,
    alias_handler,
};


int
Member::delattr( CAtom* atom )
{
    if( get_delattr_mode() >= sizeof( handlers ) )
        return no_op_handler( this, atom );
    return handlers[ get_delattr_mode() ]( this, atom );
}

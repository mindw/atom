/*-----------------------------------------------------------------------------
| Copyright (c) 2013, Nucleic Development Team.
|
| Distributed under the terms of the Modified BSD License.
|
| The full license is in the file COPYING.txt, distributed with this software.
|----------------------------------------------------------------------------*/
#pragma once
#include "member.h"
#include "catom.h"


class AliasGuard
{

public:

    static bool guarded( Member* member, CAtom* atom )
    {
        return member->alias_guard() && atom->alias_guard();
    }

    AliasGuard( Member* member, CAtom* atom ) : m_member( member ), m_atom( atom )
    {
        if( !m_member->alias_guard() )
        {
            m_member->set_alias_guard( true );
            m_owns_member = true;
        }
        if( !m_atom->alias_guard() )
        {
            m_atom->set_alias_guard( true );
            m_owns_atom = true;
        }
    }

    ~AliasGuard()
    {
        if( m_owns_member )
            m_member->set_alias_guard( false);
        if( m_owns_atom )
            m_atom->set_alias_guard( false );
    }

private:

    Member* m_member;
    CAtom* m_atom;
    bool m_owns_member;
    bool m_owns_atom;
};

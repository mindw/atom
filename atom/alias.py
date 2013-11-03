#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import CAtom, Member, GetAttr, SetAttr, DelAttr, atomref


class DynamicAliasObserver(object):
    """ An object which helps implement alias observer behavior.

    This class is an implementation detail of the framework, it should
    not be used directly by user code.

    """
    __slots__ = ('name', 'obref')

    def __init__(self, name, obref):
        self.name = name
        self.obref = obref

    def __nonzero__(self):
        return bool(self.obref)

    def __call__(self, change):
        if self.obref:
            ob = self.obref()
            member = ob.get_member(self.name)
            member.notify(ob, change)
            ob.notify(member.name, change)


class StaticAliasObserver(object):
    """ An object which helps implement alias observer behavior.

    This class is an implementation detail of the framework, it should
    not be used directly by user code.

    """
    __slots__ = 'name'

    def __init__(self, name):
        self.name = name

    def __call__(self, change):
        new = None
        ctype = change['type']
        if ctype == 'create' or ctype == 'update':
            new = change['value']
        owner = change['object']
        name = self.name
        alias = owner.get_member(name)
        handler = alias.get_slot(owner)
        if handler is not None:
            handler.obref = None
        if isinstance(new, CAtom):
            obref = atomref(owner)
            new.observe(alias.attr, DynamicAliasObserver(name, obref))


class Alias(Member):
    """ A member which aliases an attribute on a contained object.

    """
    __slots__ = 'observable'

    def __init__(self, target, attr, observable=False):
        """ Initialize an Alias.

        Parameters
        ----------
        target : str
            The name of the attribute on the current object which
            holds the target object to be aliased.

        attr : str
            The name of the attribute of interest on the aliased
            object.

        observable : bool, optional
            Whether or not the alias is observable. The default is False
            as observing an alias requires more overhead than standard
            observers.

        """
        context = (target, attr)
        self.set_getattr_mode(GetAttr.Alias, context)
        self.set_setattr_mode(SetAttr.Alias, context)
        self.set_delattr_mode(DelAttr.Alias, context)
        self.set_needs_storage(observable)
        self.observable = observable

    @property
    def target(self):
        """ Get the name of the target object attribute.

        Returns
        -------
        result : str
            The name of the target object attribute.

        """
        return self.getattr_mode[1][0]

    @property
    def attr(self):
        """ Get the name of the target alias attribute.

        Returns
        -------
        result : str
            The name of the target alias attribute.

        """
        return self.getattr_mode[1][1]

    def clone(self):
        """ Create a clone of the alias member.

        Returns
        -------
        result : Alias
            A clone of the alias.

        """
        clone = super(Alias, self).clone()
        clone.observable = self.observable
        return clone

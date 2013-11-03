#------------------------------------------------------------------------------
# Copyright (c) 2013, Nucleic Development Team.
#
# Distributed under the terms of the Modified BSD License.
#
# The full license is in the file COPYING.txt, distributed with this software.
#------------------------------------------------------------------------------
from .catom import Member, GetAttr, SetAttr, DelAttr


class Alias(Member):
    """ A member which aliases an attribute on a contained object.

    """
    __slots__ = 'observable'

    def __init__(self, name, attr, observable=False):
        """ Initialize an Alias.

        Parameters
        ----------
        name : str
            The name of the attribute on the current object which
            holds the aliased object.

        attr : str
            The name of the attribute of interest on the aliased
            object.

        observable : bool, optional
            Whether or not the alias is observable. The default is False
            as observing an alias requires more overhead than standard
            observers.

        """
        context = (name, attr)
        self.set_getattr_mode(GetAttr.Alias, context)
        self.set_setattr_mode(SetAttr.Alias, context)
        self.set_delattr_mode(DelAttr.Alias, context)
        self.set_needs_storage(False)
        self.observable = observable

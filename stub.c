/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "3d_def.h"


//
// Offsets of variables in data segment.
//

const int OBJLIST_OFFSET = 0xFFFF - MAXACTORS;
const int STATOBJLIST_OFFSET = 0xFFFF - MAXSTATS;
const int DOOROBJLIST_OFFSET = 0xFFFF - MAXDOORS;


void ogl_update_screen();


objtype *ui16_to_actor(uint16_t value)//
{
    int index = value - OBJLIST_OFFSET;

    if (index < 0) {
        return NULL;
    }

    if (index >= MAXACTORS) {
        return NULL;
    }

    return &objlist[index];
}

uint16_t actor_to_ui16(const objtype *actor)//
{
    intptr_t index = actor - objlist;

    if (index < 0) {
        return 0;
    }

    if (index >= MAXACTORS) {
        return 0;
    }

    return (uint16_t)(index + OBJLIST_OFFSET);
}

statobj_t* ui16_to_static_object(uint16_t value)//
{
    int index = value - STATOBJLIST_OFFSET;

    if (index < 0) {
        return NULL;
    }

    if (index >= MAXSTATS) {
        return NULL;
    }

    return &statobjlist[index];
}

uint16_t static_object_to_ui16(const statobj_t* static_object)//
{
    intptr_t index = static_object - statobjlist;

    if (index < 0) {
        return 0;
    }

    if (index >= MAXSTATS) {
        return 0;
    }

    return (uint16_t)(index + STATOBJLIST_OFFSET);
}

doorobj_t* ui16_to_door_object(uint16_t value)//
{
    int index = value - DOOROBJLIST_OFFSET;

    if (index < 0) {
        return NULL;
    }

    if (index >= MAXDOORS) {
        return NULL;
    }

    return &doorobjlist[index];
}

uint16_t door_object_to_ui16(const doorobj_t* door_object)//
{
    intptr_t index = door_object - doorobjlist;

    if (index < 0) {
        return 0;
    }

    if (index >= MAXDOORS) {
        return 0;
    }

    return (uint16_t)(index + DOOROBJLIST_OFFSET);
}

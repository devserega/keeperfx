/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_physics.c
 *     Implementation of physics functions used for things.
 * @par Purpose:
 *     Functions to move things, with acceleration, speed and bouncing/sliding
 *     on walls.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 02 Mar 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_physics.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_data.h"
#include "thing_stats.h"
#include "thing_list.h"
#include "creature_control.h"
#include "map_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_slide_thing_against_wall_at(struct Thing *thing, struct Coord3d *pos, long a3);
DLLIMPORT void _DK_bounce_thing_off_wall_at(struct Thing *thing, struct Coord3d *pos, long a3);
DLLIMPORT long _DK_creature_cannot_move_directly_to(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_thing_in_wall_at(const struct Thing *thing, const struct Coord3d *pos);
DLLIMPORT long _DK_get_thing_height_at(const struct Thing *thing, const struct Coord3d *pos);
DLLIMPORT long _DK_get_thing_height_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long a3);
DLLIMPORT long _DK_thing_in_wall_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long a3);
/******************************************************************************/


/******************************************************************************/
void slide_thing_against_wall_at(struct Thing *thing, struct Coord3d *pos, long a3)
{
    _DK_slide_thing_against_wall_at(thing, pos, a3); return;
}

void bounce_thing_off_wall_at(struct Thing *thing, struct Coord3d *pos, long a3)
{
    _DK_bounce_thing_off_wall_at(thing, pos, a3); return;
}

void remove_relevant_forces_from_thing_after_slide(struct Thing *thing, struct Coord3d *pos, long a3)
{
    switch ( a3 )
    {
    case 1:
        thing->pos_2C.x.val = 0;
        break;
    case 2:
        thing->pos_2C.y.val = 0;
        break;
    case 3:
        thing->pos_2C.x.val = 0;
        thing->pos_2C.y.val = 0;
        break;
    case 4:
        thing->pos_2C.z.val = 0;
        break;
    case 5:
        thing->pos_2C.x.val = 0;
        thing->pos_2C.z.val = 0;
        break;
    case 6:
        thing->pos_2C.y.val = 0;
        thing->pos_2C.z.val = 0;
        break;
    case 7:
        thing->pos_2C.x.val = 0;
        thing->pos_2C.y.val = 0;
        thing->pos_2C.z.val = 0;
        break;
    }
}

TbBool positions_equivalent(const struct Coord3d *pos_a, const struct Coord3d *pos_b)
{
    if (pos_a->x.val != pos_b->x.val)
        return false;
    if (pos_a->y.val != pos_b->y.val)
        return false;
    if (pos_a->z.val != pos_b->z.val)
        return false;
    return true;
}

void creature_set_speed(struct Thing *thing, long speed)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if (speed < -256)
    {
        cctrl->move_speed = -256;
    } else
    if (speed > 256)
    {
        cctrl->move_speed = 256;
    } else
    {
        cctrl->move_speed = speed;
    }
    cctrl->flgfield_1 |= CCFlg_Unknown40;
}

long creature_cannot_move_directly_to(struct Thing *thing, struct Coord3d *pos)
{
    return _DK_creature_cannot_move_directly_to(thing, pos);
}

/** Retrieves planned next position for given thing, without collision detection.
 *  Just adds thing velocity to current position and does some clipping. Nothing fancy.
 * @param pos The position to be set.
 * @param thing Source thing which position and velocity is used.
 * @return Gives true if values were in map coords range, false if they were
 *  outside map area and had to be corrected.
 */
TbBool get_thing_next_position(struct Coord3d *pos, const struct Thing *thing)
{
    // Don't clip the Z coord - clipping would make impossible to hit base ground (ie. water drip over water)
    return set_coords_add_velocity(pos, &thing->mappos, &thing->velocity, MapCoord_ClipX|MapCoord_ClipY);
}

long get_thing_height_at(const struct Thing *thing, const struct Coord3d *pos)
{
  SYNCDBG(18,"Starting");
  return _DK_get_thing_height_at(thing, pos);
}

long get_thing_height_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long radius)
{
    return _DK_get_thing_height_at_with_radius(thing, pos, radius);
}

long thing_in_wall_at(const struct Thing *thing, const struct Coord3d *pos)
{
    return _DK_thing_in_wall_at(thing, pos);
}

long thing_in_wall_at_with_radius(const struct Thing *thing, const struct Coord3d *pos, unsigned long radius)
{
    return _DK_thing_in_wall_at_with_radius(thing, pos, radius);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
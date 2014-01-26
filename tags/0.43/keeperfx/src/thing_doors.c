/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_doors.c
 *     Door things support functions.
 * @par Purpose:
 *     Functions to create and operate on door things.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_doors.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_objects.h"
#include "thing_list.h"
#include "thing_stats.h"
#include "config_terrain.h"
#include "ariadne.h"
#include "gui_topmsg.h"
#include "game_legacy.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char const build_door_angle[] = {-1, -1, -1, -1, -1, 0, -1, 0, -1, -1, 1, 1, -1, 0, 1, -1 };
/* Obsolete - use DoorConfigStats instead
const short door_names[] = {
    201, 590, 591, 592, 593, 0,
};
*/
/******************************************************************************/
DLLIMPORT void _DK_lock_door(struct Thing *doortng);
DLLIMPORT struct Thing *_DK_create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4);
DLLIMPORT long _DK_destroy_door(struct Thing *doortng);
DLLIMPORT long _DK_process_door(struct Thing *doortng);
DLLIMPORT long _DK_check_door_should_open(struct Thing *doortng);
DLLIMPORT unsigned char _DK_find_door_of_type(unsigned long a1, unsigned char a2);
/******************************************************************************/


/******************************************************************************/
struct Thing *create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4)
{
  return _DK_create_door(pos, a1, a2, a3, a4);
}

TbBool remove_key_on_door(struct Thing *thing)
{
  struct Thing *keytng;
  keytng = find_base_thing_on_mapwho(TCls_Object, 44, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
  if (thing_is_invalid(keytng))
    return false;
  delete_thing_structure(keytng, 0);
  return true;
}

TbBool add_key_on_door(struct Thing *thing)
{
  struct Thing *keytng;
  keytng = create_object(&thing->mappos, 44, thing->owner, 0);
  if (thing_is_invalid(keytng))
    return false;
  keytng->mappos.x.stl.pos = 128;
  keytng->mappos.y.stl.pos = 128;
  keytng->mappos.z.stl.num = 4;
  return true;
}

void unlock_door(struct Thing *thing)
{
  thing->byte_18 = 0;
  game.field_14EA4B = 1;
  update_navigation_triangulation(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1,
    thing->mappos.x.stl.num+1, thing->mappos.y.stl.num+1);
  pannel_map_update(thing->mappos.x.stl.num-1, thing->mappos.y.stl.num-1, 3, 3);
  if (!remove_key_on_door(thing))
    WARNMSG("Cannot remove keyhole when unlocking door.");
}

void lock_door(struct Thing *doortng)
{
    struct DoorStats *dostat;
    long stl_x,stl_y;
    //_DK_lock_door(thing);
    dostat = &door_stats[doortng->model][doortng->door.orientation];
    stl_x = doortng->mappos.x.stl.num;
    stl_y = doortng->mappos.y.stl.num;
    doortng->active_state = DorSt_Unknown02;
    doortng->word_16 = 0;
    doortng->byte_18 = 1;
    game.field_14EA4B = 1;
    place_animating_slab_type_on_map(dostat->field_0, 0, stl_x, stl_y, doortng->owner);
    update_navigation_triangulation(stl_x-1,  stl_y-1, stl_x+1,stl_y+1);
    pannel_map_update(stl_x-1, stl_y-1, 3, 3);
    if (!add_key_on_door(doortng)) {
        WARNMSG("Cannot create a keyhole when locking a door.");
    }
}

long destroy_door(struct Thing *thing)
{
  return _DK_destroy_door(thing);
}

TbBool thing_is_deployed_door(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (thing->class_id != TCls_Door)
        return false;
    return true;
}

TbBool door_can_stand(struct Thing *thing)
{
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    unsigned int wall_flags;
    long slb_x,slb_y;
    int i;
    wall_flags = 0;
    for (i = 0; i < 4; i++)
    {
        wall_flags *= 2;
        slb_x = subtile_slab_fast(thing->mappos.x.stl.num) + (int)small_around[i].delta_x;
        slb_y = subtile_slab_fast(thing->mappos.y.stl.num) + (int)small_around[i].delta_y;
        slb = get_slabmap_block(slb_x,slb_y);
        slbattr = get_slab_attrs(slb);
      if ((slbattr->category == SlbAtCtg_FortifiedWall) || (slb->kind == SlbT_ROCK)
          || (slb->kind == SlbT_EARTH) || (slb->kind == SlbT_TORCHDIRT)
          || (slb->kind == SlbT_GOLD) || (slb->kind == SlbT_GEMS))
        wall_flags |= 0x01;
    }
    // The array needs to have 2^4 = 16 values
    return (build_door_angle[wall_flags] != -1);
}

TbBool check_door_should_open(struct Thing *thing)
{
    struct Thing *openertng;
    //return _DK_check_door_should_open(thing);
    // If doors are locked, never should open
    if (thing->byte_18 != 0)
    {
        return false;
    }
    openertng = get_creature_near_and_owned_by_or_allied_with(thing->mappos.x.val, thing->mappos.y.val, 5, thing->owner);
    if (thing_is_invalid(openertng))
    {
        return false;
    }
    return true;
}

long process_door_open(struct Thing *thing)
{
    // If doors are locked, delay to closing = 0
    if (thing->byte_18)
        thing->byte_15 = 0;
    if ( check_door_should_open(thing) )
    {
        thing->byte_15 = 10;
        return 0;
    }
    if (thing->byte_15 > 0)
    {
        thing->byte_15--;
        return 0;
    }
    thing->active_state = DorSt_Unknown04;
    thing_play_sample(thing, 92, 100, 0, 3, 0, 2, 256);
    return 1;
}

long process_door_closed(struct Thing *thing)
{
    if ( !check_door_should_open(thing) )
      return 0;
    thing->active_state = DorSt_Unknown03;
    thing_play_sample(thing, 91, 100, 0, 3, 0, 2, 256);
    return 1;
}

long process_door_opening(struct Thing *thing)
{
    struct DoorStats *dostat;
    int new_h,old_h,delta_h;
    int slbparam;
    dostat = &door_stats[thing->model][thing->door.orientation];
    old_h = (thing->word_16 / 256);
    delta_h = dostat->field_6;
    slbparam = dostat->field_0;
    if (thing->word_16+delta_h < 768)
    {
        thing->word_16 += delta_h;
    } else
    {
        thing->active_state = DorSt_Unknown01;
        thing->byte_15 = 10;
        thing->word_16 = 768;
    }
    new_h = (thing->word_16 / 256);
    if (new_h != old_h)
      place_animating_slab_type_on_map(slbparam, new_h, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner);
    return 1;
}

long process_door_closing(struct Thing *thing)
{
    struct DoorStats *dostat;
    int new_h,old_h,delta_h;
    int slbparam;
    old_h = (thing->word_16 / 256);
    dostat = &door_stats[thing->model][thing->door.orientation];
    delta_h = dostat->field_6;
    slbparam = dostat->field_0;
    if ( check_door_should_open(thing) )
    {
        thing->active_state = DorSt_Unknown03;
        thing_play_sample(thing, 91, 100, 0, 3, 0, 2, 256);
    }
    if (thing->word_16 > delta_h)
    {
        thing->word_16 -= delta_h;
    } else
    {
        thing->active_state = DorSt_Unknown02;
        thing->word_16 = 0;
    }
    new_h = (thing->word_16 / 256);
    if (new_h != old_h)
      place_animating_slab_type_on_map(slbparam, new_h, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->owner);
    return 1;
}

TngUpdateRet process_door(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    TRACE_THING(thing);
    //return _DK_process_door(thing);
    if ( !door_can_stand(thing) || (thing->health < 0) )
    {
        thing->health = -1;
        destroy_door(thing);
        return TUFRet_Deleted;
    }
    if ((thing->door.orientation > 1) || (thing->door.orientation < 0))
    {
        ERRORLOG("Invalid %s (index %d) orientation %d",thing_model_name(thing),(int)thing->index,(int)thing->door.orientation);
        thing->door.orientation &= 1;
    }
    switch ( thing->active_state )
    {
    case DorSt_Unknown01:
        process_door_open(thing);
        break;
    case DorSt_Unknown02:
        process_door_closed(thing);
        break;
    case DorSt_Unknown03:
        process_door_opening(thing);
        break;
    case DorSt_Unknown04:
        process_door_closing(thing);
        break;
    default:
        ERRORLOG("Invalid %s state %d",thing_model_name(thing),(int)thing->active_state);
        thing->active_state = DorSt_Unknown04;
        break;
    }
    return TUFRet_Modified;
}

unsigned char find_door_of_type(ThingModel model, PlayerNumber owner)
{
    struct Thing *thing;
    long i;
    unsigned long k;
   //return _DK_find_door_of_type(a1, a2);
    k = 0;
    i = game.thing_lists[TngList_Doors].index;
    while (i > 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
            break;
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner == owner) && (thing->model == model))
            return true;
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
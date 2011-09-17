/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_data.c
 *     Rooms support functions.
 * @par Purpose:
 *     Functions to create and maintain the game rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Apr 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_data.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "config_creature.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "thing_effects.h"
#include "room_jobs.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "gui_topmsg.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_delete_room_structure(struct Room *room);
DLLIMPORT struct Room * _DK_find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, signed char plyr_idx, signed char rkind, unsigned char a4);
DLLIMPORT long _DK_claim_room(struct Room *room,struct Thing *claimtng);
DLLIMPORT long _DK_claim_enemy_room(struct Room *room,struct Thing *claimtng);
/******************************************************************************/
void count_slabs(struct Room *room);
void count_gold_slabs_with_efficiency(struct Room *room);
void count_gold_hoardes_in_room(struct Room *room);
void count_slabs_div2(struct Room *room);
void count_books_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_slabs_with_efficiency(struct Room *room);
void count_crates_in_room(struct Room *room);
void count_workers_in_room(struct Room *room);
void count_bodies_in_room(struct Room *room);
void count_capacity_in_garden(struct Room *room);
void count_food_in_room(struct Room *room);
void count_lair_occupants(struct Room *room);
/******************************************************************************/

RoomKind look_through_rooms[] = {
    RoK_DUNGHEART, RoK_TREASURE, RoK_LAIR,      RoK_GARDEN,
    RoK_LIBRARY,   RoK_TRAINING, RoK_WORKSHOP,  RoK_SCAVENGER,
    RoK_PRISON,    RoK_TEMPLE,   RoK_TORTURE,   RoK_GRAVEYARD,
    RoK_BARRACKS,  RoK_BRIDGE,   RoK_GUARDPOST, RoK_ENTRANCE,
    RoK_DUNGHEART, RoK_UNKN17,};

struct RoomData room_data[] = {
  { 0,  0, NULL,                    NULL,                   NULL,                  0, 0, 0, 201, 201},
  {14,  0, count_slabs,             NULL,                   NULL,                  0, 0, 0, 598, 614},
  {16, 57, count_gold_slabs_with_efficiency, count_gold_hoardes_in_room, NULL,     1, 0, 0, 599, 615},
  {18, 61, count_slabs_div2,        count_books_in_room,    count_workers_in_room, 0, 0, 0, 600, 616},
  {20, 65, count_slabs_with_efficiency, NULL,               NULL,                  1, 0, 0, 601, 617},
  {22, 63, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 602, 619},
  {24, 67, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 603, 618},
  {26,  0, NULL,                    NULL,                   NULL,                  0, 0, 0, 604, 620},
  {28, 75, count_slabs_div2,        count_crates_in_room,   count_workers_in_room, 0, 0, 0, 605, 621},
  {30, 77, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 613, 629},
  {32, 73, count_slabs_div2,        NULL,                   NULL,                  1, 0, 0, 612, 628},
  {34, 71, count_slabs_div2,        count_bodies_in_room,   NULL,                  0, 0, 0, 606, 622},
  {40, 69, count_slabs_div2,        NULL,                   NULL,                  0, 0, 0, 607, 623},
  {36, 59, count_capacity_in_garden, count_food_in_room,    NULL,                  1, 0, 0, 608, 624},
  {38, 79, count_slabs_with_efficiency, count_lair_occupants, NULL,                1, 0, 0, 609, 625},
  {51, 81, NULL,                    NULL,                   NULL,                  0, 0, 0, 610, 626},
  {53, 83, count_slabs,             NULL,                   NULL,                  0, 0, 0, 611, 627},
};

struct RoomInfo room_info[] = {
  { 0,  0,  0},
  { 0,  0,  0},
  {29, 57,  0},
  {33, 61,  0},
  {37, 65,  0},
  {35, 63,  0},
  {39, 67, 85},
  { 0,  0,  0},
  {47, 75, 86},
  {49, 77,156},
  {45, 73,155},
  {43, 71, 45},
  {41, 69,  0},
  {31, 59,  0},
  {51, 79,  0},
  {53, 81,  0},
  {55, 83,  0},
};

struct AroundLByte const room_spark_offset[] = {
  {-256,  256},
  {-256,    0},
  {-256, -256},
  {-256, -256},
  {   0, -256},
  { 256, -256},
  { 256, -256},
  { 256,    0},
  { 256,  256},
  { 256,  256},
  {   0,  256},
  {-256,  256},
};

struct Around const small_around[] = {
  { 0,-1},
  { 1, 0},
  { 0, 1},
  {-1, 0},
};

struct Around const my_around_eight[] = {
  { 0,-1},
  { 1,-1},
  { 1, 0},
  { 1, 1},
  { 0, 1},
  {-1, 1},
  {-1, 0},
  {-1,-1},
};

short const around_map[] = {-257, -256, -255, -1, 0, 1, 255, 256, 257};

unsigned char const slabs_to_centre_peices[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,
  1,  1,  1,  2,  2,  2,  3,  4,  4,
  4,  5,  6,  6,  6,  7,  8,  9,  9,
  9, 10, 11, 12, 12, 12, 13, 14, 15,
 16, 16, 16, 17, 18, 19, 20, 20, 20,
 21, 22, 23, 24, 25,
};

/**
 * Should contain values encoded with get_subtile_number(). */
const unsigned short small_around_pos[] = {
  0xFF00, 0x0001, 0x0100, 0xFFFF,
};

struct Around const mid_around[] = {
  { 0,  0},
  { 0, -1},
  { 1,  0},
  { 0,  1},
  {-1,  0},
  {-1, -1},
  { 1, -1},
  {-1,  1},
  { 1,  1},
};

unsigned short const room_effect_elements[] = { 55, 56, 57, 58, 0, 0 };
const short slab_around[] = { -85, 1, 85, -1 };
/******************************************************************************/
DLLIMPORT unsigned char _DK_find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT void _DK_count_gold_slabs_with_efficiency(struct Room *room);
DLLIMPORT void _DK_count_gold_hoardes_in_room(struct Room *room);
DLLIMPORT void _DK_count_books_in_room(struct Room *room);
DLLIMPORT void _DK_count_workers_in_room(struct Room *room);
DLLIMPORT void _DK_count_crates_in_room(struct Room *room);
DLLIMPORT void _DK_count_workers_in_room(struct Room *room);
DLLIMPORT void _DK_count_bodies_in_room(struct Room *room);
DLLIMPORT void _DK_count_food_in_room(struct Room *room);
DLLIMPORT void _DK_count_lair_occupants(struct Room *room);
DLLIMPORT short _DK_room_grow_food(struct Room *room);
DLLIMPORT void _DK_set_room_capacity(struct Room *room, long capac);
DLLIMPORT void _DK_set_room_efficiency(struct Room *room);
DLLIMPORT struct Room *_DK_link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind);
DLLIMPORT struct Room *_DK_create_room(unsigned char a1, unsigned char a2, unsigned short a3, unsigned short a4);
DLLIMPORT void _DK_create_room_flag(struct Room *room);
DLLIMPORT struct Room *_DK_allocate_free_room_structure(void);
DLLIMPORT unsigned short _DK_i_can_allocate_free_room_structure(void);
DLLIMPORT struct Room *_DK_find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2);
DLLIMPORT long _DK_create_workshop_object_in_workshop_room(long a1, long a2, long a3);
DLLIMPORT unsigned char _DK_find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos);
DLLIMPORT struct Room* _DK_find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing,
    signed char a2, signed char a3, unsigned char a4, long a5);
DLLIMPORT struct Room* _DK_find_room_with_spare_capacity(unsigned char a1, signed char a2, long a3);
DLLIMPORT short _DK_delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3);
DLLIMPORT long _DK_calculate_room_efficiency(struct Room *room);
DLLIMPORT void _DK_kill_room_slab_and_contents(unsigned char a1, unsigned char a2, unsigned char a3);
DLLIMPORT void _DK_free_room_structure(struct Room *room);
DLLIMPORT void _DK_reset_creatures_rooms(struct Room *room);
DLLIMPORT void _DK_replace_room_slab(struct Room *room, long a2, long a3, unsigned char a4, unsigned char a5);
DLLIMPORT struct Room *_DK_place_room(unsigned char a1, unsigned char a2, unsigned short a3, unsigned short a4);
DLLIMPORT struct Room *_DK_find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char a2, char a3, unsigned char a4);
DLLIMPORT struct Room * _DK_pick_random_room(PlayerNumber plyr_idx, int kind);
/******************************************************************************/
struct Room *room_get(long room_idx)
{
  if ((room_idx < 1) || (room_idx > ROOMS_COUNT))
    return &game.rooms[0];
  return &game.rooms[room_idx];
}

struct Room *subtile_room_get(long stl_x, long stl_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_for_subtile(stl_x,stl_y);
  if (slabmap_block_invalid(slb))
    return INVALID_ROOM;
  return room_get(slb->room_index);
}

struct Room *slab_room_get(long slb_x, long slb_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_block(slb_x,slb_y);
  if (slabmap_block_invalid(slb))
    return INVALID_ROOM;
  return room_get(slb->room_index);
}

TbBool room_is_invalid(const struct Room *room)
{
  if (room == NULL)
    return true;
  if (room == INVALID_ROOM)
    return true;
  return (room <= &game.rooms[0]);
}

TbBool room_exists(const struct Room *room)
{
  if (room_is_invalid(room))
    return false;
  return ((room->field_0 & 0x01) != 0);
}

struct RoomData *room_data_get_for_kind(RoomKind rkind)
{
  if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[rkind];
}

struct RoomData *room_data_get_for_room(const struct Room *room)
{
  if ((room->kind < 1) || (room->kind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[room->kind];
}

struct RoomStats *room_stats_get_for_kind(RoomKind rkind)
{
    if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
        return &game.room_stats[0];
    return &game.room_stats[rkind];
}

struct RoomStats *room_stats_get_for_room(const struct Room *room)
{
    if ((room->kind < 1) || (room->kind > ROOM_TYPES_COUNT))
        return &game.room_stats[0];
    return &game.room_stats[room->kind];
}

long get_room_look_through(RoomKind rkind)
{
  const long arr_length = sizeof(look_through_rooms)/sizeof(look_through_rooms[0]);
  long i;
  for (i=0; i < arr_length; i++)
  {
    if (look_through_rooms[i] == rkind)
      return i;
  }
  return -1;
}

long get_room_slabs_count(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    long i;
    long count;
    dungeon = get_players_num_dungeon(plyr_idx);
    count = 0;
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        count += room->slabs_count;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

long get_room_kind_used_capacity_fraction(PlayerNumber plyr_idx, RoomKind room_kind)
{
    struct Dungeon * dungeon;
    struct Room * room;
    int used_capacity;
    int total_capacity;
    long i;
    unsigned long k;
    dungeon = get_dungeon(plyr_idx);
    total_capacity = 0;
    used_capacity = 0;
    i = dungeon->room_kind[room_kind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        used_capacity += room->used_capacity;
        total_capacity += room->total_capacity;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    if (total_capacity <= 0) {
        return 0;
    }
    return (used_capacity * 256) / total_capacity;
}

long get_player_rooms_count(PlayerNumber plyr_idx, RoomKind rkind)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned long k;
  long i;
  // note that we can't get_players_num_dungeon() because players
  // may be uninitialized yet when this is called.
  dungeon = get_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return 0;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return k;
}

void set_room_capacity(struct Room *room, long capac)
{
    SYNCDBG(7,"Starting");
    _DK_set_room_capacity(room, capac);
}

void set_room_efficiency(struct Room *room)
{
  _DK_set_room_efficiency(room);
}

void count_slabs(struct Room *room)
{
  room->total_capacity = room->slabs_count;
}

void count_gold_slabs_with_efficiency(struct Room *room)
{
  _DK_count_gold_slabs_with_efficiency(room);
}

void count_gold_hoardes_in_room(struct Room *room)
{
  _DK_count_gold_hoardes_in_room(room);
}

void count_slabs_div2(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = ((count/256) >> 1);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;

}

void count_books_in_room(struct Room *room)
{
  _DK_count_books_in_room(room);
}

void count_workers_in_room(struct Room *room)
{
  _DK_count_workers_in_room(room);
}

void count_slabs_with_efficiency(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = (count/256);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_crates_in_room(struct Room *room)
{
  _DK_count_crates_in_room(room);
}

void count_bodies_in_room(struct Room *room)
{
  _DK_count_bodies_in_room(room);
}

void count_capacity_in_garden(struct Room *room)
{
  unsigned long count;
  count = room->slabs_count * ((long)room->efficiency);
  count = (count/256);
  if (count <= 1)
    count = 1;
  room->total_capacity = count;
}

void count_food_in_room(struct Room *room)
{
  _DK_count_food_in_room(room);
}

void count_lair_occupants(struct Room *room)
{
  _DK_count_lair_occupants(room);
}


void delete_room_structure(struct Room *room)
{
    struct Dungeon *dungeon;
    struct Room *secroom;
    unsigned short *wptr;
    //_DK_delete_room_structure(room); return;
    if (room_is_invalid(room))
    {
        WARNLOG("Attempt to delete invalid room");
        return;
    }
    if ((room->field_0 & 0x01) != 0)
    {
      if (room->owner != game.neutral_player_num)
      {
          dungeon = get_players_num_dungeon(room->owner);
          wptr = &dungeon->room_kind[room->kind];
          if (room->index == *wptr)
          {
              *wptr = room->next_of_owner;
              secroom = room_get(room->next_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->prev_of_owner = 0;
          }
          else
          {
              secroom = room_get(room->next_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->prev_of_owner = room->prev_of_owner;
              secroom = room_get(room->prev_of_owner);
              if (!room_is_invalid(secroom))
                  secroom->next_of_owner = room->next_of_owner;
          }
      }
      memset(room, 0, sizeof(struct Room));
    }
}

void delete_all_room_structures(void)
{
    struct Room *room;
    long i;
    for (i=1; i < ROOMS_COUNT; i++)
    {
        room = &game.rooms[i];
        delete_room_structure(room);
    }
}

struct Room *link_adjacent_rooms_of_type(unsigned char owner, long x, long y, RoomKind rkind)
{
    struct Thing *thing;
    SlabCodedCoords central_slbnum;
    struct SlabMap *slb;
    struct RoomData *rdata;
    struct Room *linkroom;
    struct Room *room;
    long stl_x,stl_y;
    long i,n;
    unsigned long k;
    // TODO: rework! may lead to hang on map borders
    //return _DK_link_adjacent_rooms_of_type(owner, x, y, rkind);
    central_slbnum = get_slab_number(map_to_slab[x],map_to_slab[y]);
    // Localize the room to be merged with other rooms
    linkroom = INVALID_ROOM;
    for (n = 0; n < 4; n++)
    {
        stl_x = x + 3 * (long)small_around[n].delta_x;
        stl_y = y + 3 * (long)small_around[n].delta_y;
        room = subtile_room_get(stl_x,stl_y);
        if ( !room_is_invalid(room) )
        {
          if ( (room->owner == owner) && (room->kind == rkind) )
          {
              // Add the central slab to room which was found
              room->total_capacity = 0;
              slb = get_slabmap_direct(room->field_39);
              slb->next_in_room = central_slbnum;
              slb = get_slabmap_direct(central_slbnum);
              slb->next_in_room = 0;
              room->field_39 = central_slbnum;
              linkroom = room;
              break;
          }
        }
    }
    if ( room_is_invalid(linkroom) )
    {
        return NULL;
    }
    for (n++; n < 4; n++)
    {
        stl_x = x + 3 * (long)small_around[n].delta_x;
        stl_y = y + 3 * (long)small_around[n].delta_y;
        room = subtile_room_get(stl_x,stl_y);
        if ( !room_is_invalid(room) )
        {
          if ( (room->owner == owner) && (room->kind == rkind) )
          {
              if (room != linkroom)
              {
                  slb = get_slabmap_direct(linkroom->field_39);
                  slb->next_in_room = room->slabs_list;
                  linkroom->field_39 = room->field_39;
                  linkroom->slabs_count = 0;
                  k = 0;
                  i = room->slabs_list;
                  while (i != 0)
                  {
                      // Per room tile code
                      linkroom->slabs_count++;
                      slb->room_index = linkroom->index;
                      // Per room tile code ends
                      i = get_next_slab_number_in_room(i);
                      k++;
                      if (k > room->slabs_count)
                      {
                          ERRORLOG("Room slabs list length exceeded when sweeping");
                          break;
                      }
                  }
                  rdata = room_data_get_for_kind(linkroom->kind);
                  if (rdata->ofsfield_3 != NULL)
                  {
                      rdata->ofsfield_3(linkroom);
                  }
                  k = 0;
                  while (room->creatures_list != 0)
                  {
                      thing = thing_get(room->creatures_list);
                      if (thing_is_invalid(thing))
                      {
                          ERRORLOG("Jump to invalid creature %d detected",(int)room->creatures_list);
                          break;
                      }
                      // Per creature code
                      remove_creature_from_specific_room(thing, room);
                      add_creature_to_work_room(thing, linkroom);
                      // Per creature code ends
                      k++;
                      if (k > THINGS_COUNT)
                      {
                          ERRORLOG("Infinite loop detected when sweeping creatures list");
                          break;
                      }
                  }
                  delete_room_flag(room);
                  free_room_structure(room);
              }
          }
        }
    }
    return linkroom;
}

void count_room_slabs(struct Room *room)
{
    struct SlabMap *slb;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    room->slabs_count = 0;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        slb = get_slabmap_block(slb_x,slb_y);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        room->slabs_count++;
        slb->room_index = room->index;
        // Per room tile code ends
        k++;
        if (k >= map_tiles_x*map_tiles_y)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
}

/** Returns coordinates of slab at mass centre of given room.
 *  Note that the slab returned may not be pat of the room - it is possible
 *   that the room is just surrounding the spot.
 * @param mass_x
 * @param mass_y
 * @param room
 */
void get_room_mass_centre_coords(long *mass_x, long *mass_y, const struct Room *room)
{
    struct SlabMap *slb;
    unsigned long tot_x,tot_y;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    tot_x = 0;
    tot_y = 0;
    k = 0;
    i = room->slabs_list;
    while (i > 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        slb = get_slabmap_block(slb_x,slb_y);
        if (slabmap_block_invalid(slb))
        {
          ERRORLOG("Jump to invalid item when sweeping Slabs.");
          break;
        }
        // Per room tile code
        tot_x += slb_x;
        tot_y += slb_y;
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    if (room->slabs_count > 1) {
        *mass_x = tot_x / room->slabs_count;
        *mass_y = tot_y / room->slabs_count;
    } else
    if (room->slabs_count > 0) {
        *mass_x = tot_x;
        *mass_y = tot_y;
    } else {
        *mass_x = map_tiles_x / 2;
        *mass_y = map_tiles_y / 2;
    }
}


void update_room_central_tile_position(struct Room *room)
{
    struct MapOffset *sstep;
    struct SlabMap *slb;
    long mass_x,mass_y;
    long cx,cy;
    long i;
    get_room_mass_centre_coords(&mass_x, &mass_y, room);
    for (i=0; i < 256; i++)
    {
        sstep = &spiral_step[i];
        cx = 3 * (mass_x + (long)sstep->h) + 1;
        cy = 3 * (mass_y + (long)sstep->v) + 1;
        slb = get_slabmap_for_subtile(cx,cy);
        if (slabmap_block_invalid(slb))
            continue;
        if (slb->room_index == room->index)
        {
            room->central_stl_x = cx;
            room->central_stl_y = cy;
            return;
        }
    }
    room->central_stl_x = map_tiles_x / 2;
    room->central_stl_y = map_tiles_y / 2;
    ERRORLOG("Cannot find position to place an ensign.");
}

void add_room_to_global_list(struct Room *room)
{
    struct Room *nxroom;
    // There is only one global list of rooms - the list of entrances
    if (room->kind == RoK_ENTRANCE)
    {
      if ((game.entrance_room_id > 0) && (game.entrance_room_id < ROOMS_COUNT))
      {
        room->next_of_kind = game.entrance_room_id;
        nxroom = room_get(game.entrance_room_id);
        nxroom->prev_of_kind = room->index;
      }
      game.entrance_room_id = room->index;
      game.entrances_count++;
    }
}

TbBool add_room_to_players_list(struct Room *room, long plyr_idx)
{
    struct Dungeon *dungeon;
    struct Room *nxroom;
    long nxroom_id;
    if (plyr_idx == game.neutral_player_num)
        return false;
    if (room->kind >= ROOM_TYPES_COUNT)
    {
        ERRORLOG("Room no %d has invalid kind",(int)room->index);
        return false;
    }
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    dungeon = get_dungeon(plyr_idx);
    nxroom_id = dungeon->room_kind[room->kind];
    nxroom = room_get(nxroom_id);
    if (room_is_invalid(nxroom))
    {
        room->next_of_owner = 0;
    } else
    {
        room->next_of_owner = nxroom_id;
        nxroom->prev_of_owner = room->index;
    }
    dungeon->room_kind[room->kind] = room->index;
    dungeon->room_slabs_count[room->kind]++;
    return true;
}

struct Room *prepare_new_room(unsigned char owner, unsigned char rkind, unsigned short x, unsigned short y)
{
    struct SlabMap *slb;
    struct Room *room;
    long slb_x,slb_y;
    long i;
    if ( !i_can_allocate_free_room_structure() )
    {
        ERRORDBG(2,"Cannot allocate any more rooms.");
        erstat_inc(ESE_NoFreeRooms);
        return NULL;
    }
    room = allocate_free_room_structure();
    room->owner = owner;
    room->kind = rkind;
    add_room_to_global_list(room);
    add_room_to_players_list(room, owner);
    slb_x = map_to_slab[x%(map_subtiles_x+1)];
    slb_y = map_to_slab[y%(map_subtiles_y+1)];
    i = get_slab_number(slb_x, slb_y);
    room->slabs_list = i;
    room->field_39 = i;
    slb = get_slabmap_direct(i);
    slb->next_in_room = 0;
    return room;
}

struct Room *create_room(unsigned char owner, unsigned char rkind, unsigned short x, unsigned short y)
{
    struct Room *room;
    SYNCDBG(7,"Starting");
    // room = _DK_create_room(owner, rkind, x, y); return room;
    // Try linking the new room slab to existing room
    room = link_adjacent_rooms_of_type(owner, x, y, rkind);
    if (room_is_invalid(room))
    {
        room = prepare_new_room(owner, rkind, x, y);
        if (room_is_invalid(room))
            return INVALID_ROOM;
        count_room_slabs(room);
        update_room_central_tile_position(room);
        create_room_flag(room);
    } else
    {
        count_room_slabs(room);
        update_room_central_tile_position(room);
    }
    SYNCDBG(7,"Done");
    return room;
}

void create_room_flag(struct Room *room)
{
    struct Thing *thing;
    struct Coord3d pos;
    long x,y;
    //_DK_create_room_flag(room);
    x = 3 * slb_num_decode_x(room->slabs_list) + 1;
    y = 3 * slb_num_decode_y(room->slabs_list) + 1;
    SYNCDBG(7,"Starting for %s at (%ld,%ld)",room_code_name(room->kind),x,y);
    if ( (room->kind != RoK_DUNGHEART) && (room->kind != RoK_ENTRANCE)
      && (room->kind != RoK_GUARDPOST) && (room->kind != RoK_BRIDGE) )
    {
        pos.z.val = 2 << 8;
        pos.x.val = x << 8;
        pos.y.val = y << 8;
        thing = find_base_thing_on_mapwho(TCls_Object, 25, x, y);
        if (thing_is_invalid(thing))
        {
            thing = create_object(&pos, 25, room->owner, -1);
        }
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Cannot create room flag");
            return;
        }
        thing->word_13 = room->index;
    }
}

void delete_room_flag(struct Room *room)
{
    struct Thing *thing;
    long stl_x,stl_y;
    stl_x = 3 * slb_num_decode_x(room->slabs_list) + 1;
    stl_y = 3 * slb_num_decode_y(room->slabs_list) + 1;
    if ((room->kind != RoK_DUNGHEART) && (room->kind != RoK_ENTRANCE))
    {
        thing = find_base_thing_on_mapwho(TCls_Object, 25, stl_x, stl_y);
        if (!thing_is_invalid(thing))
          delete_thing_structure(thing, 0);
    }
}

struct Room *allocate_free_room_structure(void)
{
  return _DK_allocate_free_room_structure();
}

unsigned short i_can_allocate_free_room_structure(void)
{
  unsigned short ret = _DK_i_can_allocate_free_room_structure();
  if (ret == 0)
      SYNCDBG(3,"No slot for next room");
  return ret;
}

RoomKind slab_to_room_type(SlabType slab_type)
{
  switch (slab_type)
  {
  case SlbT_ENTRANCE:
      return RoK_ENTRANCE;
  case SlbT_TREASURE:
      return RoK_TREASURE;
  case SlbT_LIBRARY:
      return RoK_LIBRARY;
  case SlbT_PRISON:
      return RoK_PRISON;
  case SlbT_TORTURE:
      return RoK_TORTURE;
  case SlbT_TRAINING:
      return RoK_TRAINING;
  case SlbT_DUNGHEART:
      return RoK_DUNGHEART;
  case SlbT_WORKSHOP:
      return RoK_WORKSHOP;
  case SlbT_SCAVENGER:
      return RoK_SCAVENGER;
  case SlbT_TEMPLE:
      return RoK_TEMPLE;
  case SlbT_GRAVEYARD:
      return RoK_GRAVEYARD;
  case SlbT_GARDEN:
      return RoK_GARDEN;
  case SlbT_LAIR:
      return RoK_LAIR;
  case SlbT_BARRACKS:
      return RoK_BARRACKS;
  case SlbT_BRIDGE:
      return RoK_BRIDGE;
  case SlbT_GUARDPOST:
      return RoK_GUARDPOST;
  default:
      return RoK_NONE;
  }
}

void reinitialise_treaure_rooms(void)
{
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned int i,k,n;
  for (n=0; n < DUNGEONS_COUNT; n++)
  {
    dungeon = get_dungeon(n);
    i = dungeon->room_kind[RoK_TREASURE];
    k = 0;
    while (i != 0)
    {
      room = room_get(i);
      if (room_is_invalid(room))
      {
        ERRORLOG("Jump to invalid room detected");
        break;
      }
      i = room->next_of_owner;
      set_room_capacity(room, 1);
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
  }
}

TbBool initialise_map_rooms(void)
{
  struct SlabMap *slb;
  struct Room *room;
  unsigned long x,y;
  RoomKind rkind;
  SYNCDBG(7,"Starting");
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = get_slabmap_block(x, y);
      rkind = slab_to_room_type(slb->kind);
      if (rkind > 0)
        room = create_room(slabmap_owner(slb), rkind, 3*x+1, 3*y+1);
      else
        room = NULL;
      if (room != NULL)
      {
        set_room_efficiency(room);
        set_room_capacity(room, 0);
      }
    }
  return true;
}

short room_grow_food(struct Room *room)
{
  return _DK_room_grow_food(room);
}

long calculate_room_widespread_factor(const struct Room *room)
{
    long nslabs,npieces;
    long i;
    nslabs = room->slabs_count;
    i = nslabs;
    if (i >= sizeof(slabs_to_centre_peices)/sizeof(slabs_to_centre_peices[0]))
        i = sizeof(slabs_to_centre_peices)/sizeof(slabs_to_centre_peices[0]) - 1;
    npieces = slabs_to_centre_peices[i];
    return 2 * (npieces + 4 * nslabs);
}

/** Calculates summary of efficiency score from all slabs in room.
 *
 * @param room Source room.
 * @return The efficiency score summary.
 */
long calculate_cummulative_room_slabs_effeciency(const struct Room *room)
{
    long score;
    long i;
    unsigned long k;
    score = 0;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        // Per room tile code
        score += calculate_effeciency_score_for_room_slab(i, room->owner);
        // Per room tile code ends
        i = get_next_slab_number_in_room(i); // It would be better to have this before per-tile block, but we need old value
        k++;
        if (k > room->slabs_count)
        {
          ERRORLOG("Room slabs list length exceeded when sweeping");
          break;
        }
    }
    return score;
}

long calculate_room_efficiency(const struct Room *room)
{
    long nslabs,score,widespread,effic;
    long expected_base;
    //return _DK_calculate_room_efficiency(room);
    nslabs = room->slabs_count;
    if (nslabs <= 0)
    {
        ERRORLOG("Room %s index %d seems to have no slabs.",room_code_name(room->kind),(int)room->index);
        return 0;
    }
    if (nslabs == 1) {
        expected_base = 0;
    } else {
        expected_base = 4 * (nslabs - 1);
    }
    widespread = calculate_room_widespread_factor(room);
    score = calculate_cummulative_room_slabs_effeciency(room);
    if (score <= expected_base) {
        effic = 0;
    } else
    if (widespread <= expected_base) {
        effic = ROOM_EFFICIENCY_MAX;
    } else
    {
        effic = ((score - expected_base) << 8) / (widespread - expected_base);
    }
    if (effic > ROOM_EFFICIENCY_MAX)
        effic = ROOM_EFFICIENCY_MAX;
    return effic;
}

void update_room_efficiency(struct Room *room)
{
    room->efficiency = calculate_room_efficiency(room);
}

TbBool update_room_contents(struct Room *room)
{
  struct RoomData *rdata;
  rdata = room_data_get_for_room(room);
  if (rdata->ofsfield_7 != NULL)
  {
    rdata->ofsfield_7(room);
  }
  if (rdata->offfield_B != NULL)
  {
    rdata->offfield_B(room);
  }
  return true;
}

void init_room_sparks(struct Room *room)
{
    struct SlabMap *slb;
    struct SlabMap *sibslb;
    long slb_x,slb_y;
    unsigned long k;
    long i;
    if (room->kind == RoK_DUNGHEART) {
        return;
    }
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        slb = get_slabmap_block(slb_x, slb_y);
        sibslb = get_slabmap_block(slb_x, slb_y-1);
        if (sibslb->room_index != slb->room_index)
        {
            room->field_43 = 1;
            room->field_44 = 0;
            room->field_41 = i;
        }
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
}

TbBool create_effects_on_room_slabs(struct Room *room, long effkind, long effrange, long effowner)
{
    long slb_x,slb_y;
    unsigned long k;
    long i;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        struct Coord3d pos;
        long effect_kind;
        pos.x.val = subtile_coord_center(3*slb_x+1);
        pos.y.val = subtile_coord_center(3*slb_y+1);
        pos.z.val = subtile_coord_center(1);
        effect_kind = effkind;
        if (effrange > 0)
            effect_kind += ACTION_RANDOM(effrange);
        create_effect(&pos, effect_kind, effowner);
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    return true;
}

/**
 * Clears digging operations for given player on slabs making up given room.
 *
 * @param room The room whose slabs are to be affected.
 * @param plyr_idx Player index whose dig tag shall be cleared.
 */
TbBool clear_dig_on_room_slabs(struct Room *room, long plyr_idx)
{
    long slb_x,slb_y;
    unsigned long k;
    long i;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        clear_slab_dig(slb_x, slb_y, plyr_idx);
        // Per room tile code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
    return true;
}

TbBool find_random_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_random_valid_position_for_thing_in_room(thing, room, pos);
}

struct Room *find_room_with_spare_room_item_capacity(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    //return _DK_find_room_with_spare_room_item_capacity(a1, a2);
    if ((rkind < 0) || (rkind >= ROOM_TYPES_COUNT))
        return NULL;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
        return NULL;
    k = 0;
    i = dungeon->room_kind[rkind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->capacity_used_for_storage < room->total_capacity) {
            return room;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return NULL;
}

/**
 * Searches for room of given kind and owner which has no less than given spare capacity.
 * @param owner
 * @param rkind
 * @param spare
 * @return
 * @note Function find_room_with_spare_room_capacity() should also redirect to this one.
 */
struct Room *find_room_with_spare_capacity(unsigned char owner, signed char rkind, long spare)
{
    struct Dungeon *dungeon;
    if ((rkind < 0) || (rkind >= ROOM_TYPES_COUNT))
        return NULL;
    dungeon = get_dungeon(owner);
    if (dungeon_invalid(dungeon))
        return NULL;
    return find_room_with_spare_capacity_starting_with(dungeon->room_kind[rkind], spare);
}

struct Room *find_room_with_spare_capacity_starting_with(long room_idx, long spare)
{
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    k = 0;
    i = room_idx;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->used_capacity + spare <= room->total_capacity)
        {
            return room;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return NULL;
}

struct Room *find_room_with_most_spare_capacity_starting_with(long room_idx,long *total_spare_cap)
{
    long max_spare_cap,loc_total_spare_cap,delta;
    struct Room *max_spare_room;
    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    loc_total_spare_cap = 0;
    max_spare_room = NULL;
    max_spare_cap = 0;
    k = 0;
    i = room_idx;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->total_capacity > room->used_capacity)
        {
          delta = room->total_capacity - room->used_capacity;
          loc_total_spare_cap += delta;
          if (max_spare_cap < delta)
          {
            max_spare_cap = delta;
            max_spare_room = room;
          }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    if (total_spare_cap != NULL)
       (*total_spare_cap) = loc_total_spare_cap;
    return max_spare_room;
}

TbBool find_first_valid_position_for_thing_in_room(struct Thing *thing, struct Room *room, struct Coord3d *pos)
{
    return _DK_find_first_valid_position_for_thing_in_room(thing, room, pos);
}

struct Room *find_nearest_room_for_thing_with_spare_capacity(struct Thing *thing, signed char owner, signed char rkind, unsigned char nav_no_owner, long spare)
{
    struct Dungeon *dungeon;
    struct Room *nearoom;
    long neardistance,distance;
    struct Coord3d pos;

    struct Room *room;
    unsigned long k;
    int i;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(owner);
    nearoom = NULL;
    neardistance = LONG_MAX;
    k = 0;
    i = dungeon->room_kind[rkind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        // Compute simplified distance - without use of mul or div
        distance = abs(thing->mappos.x.stl.num - room->central_stl_x)
                 + abs(thing->mappos.y.stl.num - room->central_stl_y);
        if ((neardistance > distance) && (room->used_capacity + spare <= room->total_capacity))
        {
            if (find_first_valid_position_for_thing_in_room(thing, room, &pos))
            {
                if ((thing->class_id != TCls_Creature)
                  || creature_can_navigate_to(thing, &pos, nav_no_owner))
                {
                  neardistance = distance;
                  nearoom = room;
                }
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return nearoom;
}

/**
 * Counts all room of given kind and owner where the creature can navigate to.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_no_owner
 * @return
 */
long count_rooms_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    struct Coord3d pos;
    long count;
    SYNCDBG(18,"Starting");
    dungeon = get_dungeon(owner);
    count = 0;
    k = 0;
    i = dungeon->room_kind[kind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        pos.x.val = get_subtile_center_pos(room->central_stl_x);
        pos.y.val = get_subtile_center_pos(room->central_stl_y);
        pos.z.val = 256;
        if ((room->used_capacity > 0) && creature_can_navigate_to(thing, &pos, nav_no_owner))
        {
            count++;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

/**
 * Gives a room of given kind and owner where the creature can navigate to.
 * Counts all possible rooms, then selects one and returns it.
 * @param thing
 * @param owner
 * @param kind
 * @param nav_no_owner
 * @return
 */
struct Room *find_random_room_creature_can_navigate_to(struct Thing *thing, unsigned char owner, signed char kind, unsigned char nav_no_owner)
{
    struct Dungeon *dungeon;
    struct Room *room;
    unsigned long k;
    int i;
    struct Coord3d pos;
    long count,selected;
    SYNCDBG(18,"Starting");
    count = count_rooms_creature_can_navigate_to(thing, owner, kind, nav_no_owner);
    if (count < 1)
        return NULL;
    dungeon = get_dungeon(owner);
    selected = ACTION_RANDOM(count);
    k = 0;
    i = dungeon->room_kind[kind];
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        pos.x.val = get_subtile_center_pos(room->central_stl_x);
        pos.y.val = get_subtile_center_pos(room->central_stl_y);
        pos.z.val = 256;
        if ((room->used_capacity > 0) && creature_can_navigate_to(thing, &pos, nav_no_owner))
        {
            if (selected > 0)
            {
                selected--;
            } else
            {
                return room;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return NULL;
}

/**
 * Searches for players room of given kind which center is closest to given position.
 * Computes geometric distance - does not include any map obstacles in computations.
 *
 * @param plyr_idx Player of which room distance we want.
 * @param rkind Room kind of which all rooms are to be checked.
 * @param pos Position to be closest to.
 * @param room_distance Output variable which returns the closest distance, in map coords.
 * @return
 */
struct Room *find_room_nearest_to_position(PlayerNumber plyr_idx, RoomKind rkind, const struct Coord3d *pos, long *room_distance)
{
    struct Dungeon *dungeon;
    struct Room *room;
    struct Room *near_room;
    long i;
    unsigned long k;
    long distance,near_distance;
    long delta_x,delta_y;
    dungeon = get_dungeon(plyr_idx);
    near_distance = LONG_MAX;
    near_room = INVALID_ROOM;
    i = dungeon->room_kind[rkind];
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        delta_x = (room->central_stl_x << 8) - (long)pos->x.val;
        delta_y = (room->central_stl_y << 8) - (long)pos->y.val;
        distance = LbDiagonalLength(abs(delta_x), abs(delta_y));
        if (distance < near_distance)
        {
            near_room = room;
            near_distance = distance;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    *room_distance = near_distance;
    return near_room;
}


//TODO CREATURE_AI try to make sure the creature will do proper activity in the room.
//TODO CREATURE_AI try to select lair far away from CTA and enemies
struct Room *get_room_of_given_kind_for_thing(struct Thing *thing, struct Dungeon *dungeon, RoomKind rkind)
{
  struct Room *room;
  struct Room *retroom;
  struct CreatureControl *cctrl;
  struct CreatureStats *crstat;
  long retdist,dist,pay;
  unsigned long k;
  long i;
  retdist = LONG_MAX;
  retroom = NULL;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i != 0)
  {
    room = room_get(i);
    if (room_is_invalid(room))
    {
      ERRORLOG("Jump to invalid room detected");
      break;
    }
    i = room->next_of_owner;
    // Per-room code
    if (room->kind == RoK_TREASURE)
    {
        cctrl = creature_control_get_from_thing(thing);
        crstat = creature_stats_get_from_thing(thing);
        dungeon = get_dungeon(thing->owner);
        if (dungeon->tortured_creatures[thing->model] )
        {
          pay = compute_creature_max_pay(crstat->pay,cctrl->explevel) / 2;
        } else
        {
          pay = compute_creature_max_pay(crstat->pay,cctrl->explevel);
        }
        if (room->capacity_used_for_storage > pay)
          continue;
    }
    dist =  abs(thing->mappos.y.stl.num - room->central_stl_y);
    dist += abs(thing->mappos.x.stl.num - room->central_stl_x);
    if (retdist > dist)
    {
      retdist = dist;
      retroom = room;
    }
    // Per-room code ends
    k++;
    if (k > ROOMS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return retroom;
}

struct Room * find_random_room_for_thing_with_spare_room_item_capacity(struct Thing *thing, signed char plyr_idx, signed char rkind, unsigned char a4)
{
    return _DK_find_random_room_for_thing_with_spare_room_item_capacity(thing, plyr_idx, rkind, a4);
}

long create_workshop_object_in_workshop_room(long plyr_idx, long tng_class, long tng_kind)
{
    struct Coord3d pos;
    struct Thing *thing;
    struct Room *room;
    struct Dungeon *dungeon;
    //return _DK_create_workshop_object_in_workshop_room(plyr_idx, tng_class, tng_kind);
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    switch (tng_class)
    {
    case TCls_Trap:
        thing = create_object(&pos, trap_to_object[tng_kind], plyr_idx, -1);
        break;
    case TCls_Door:
        thing = create_object(&pos, door_to_object[tng_kind], plyr_idx, -1);
        break;
    default:
        thing = INVALID_THING;
        ERRORLOG("No known workshop crate can represent %s model %d",thing_class_code_name(tng_class),(int)tng_kind);
        break;
    }
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Could not create workshop crate thing");
        return 0;
    }
    room = find_random_room_for_thing_with_spare_room_item_capacity(thing, plyr_idx, RoK_WORKSHOP, 0);
    if (room_is_invalid(room))
    {
        ERRORLOG("No room for thing");
        delete_thing_structure(thing, 0);
        return 0;
    }
    if ( !find_random_valid_position_for_thing_in_room_avoiding_object(thing, room, &pos) )
    {
        ERRORLOG("Could not find room for thing");
        delete_thing_structure(thing, 0);
        return 0;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    move_thing_in_map(thing, &pos);
    room->used_capacity++;
    room->capacity_used_for_storage++;
    dungeon = get_players_num_dungeon(plyr_idx);
    switch (tng_class)
    {
    case TCls_Trap:
        if ( !dungeon->trap_placeable[tng_kind] ) {
            event_create_event(thing->mappos.x.val, thing->mappos.y.val, TCls_Trap, plyr_idx, tng_kind);
        }
        break;
    case TCls_Door:
        if ( !dungeon->door_placeable[tng_kind] ) {
          event_create_event(thing->mappos.x.val, thing->mappos.y.val, TCls_Door, plyr_idx, tng_kind);
        }
        break;
    default:
        break;
    }
    create_effect(&pos, TngEff_Unknown56, thing->owner);
    thing_play_sample(thing, 89, 100, 0, 3, 0, 2, 256);
    return 1;
}

short delete_room_slab_when_no_free_room_structures(long a1, long a2, unsigned char a3)
{
    SYNCDBG(8,"Starting");
    return _DK_delete_room_slab_when_no_free_room_structures(a1, a2, a3);
}

void kill_room_slab_and_contents(unsigned char a1, unsigned char a2, unsigned char a3)
{
  _DK_kill_room_slab_and_contents(a1, a2, a3);
}

void free_room_structure(struct Room *room)
{
  _DK_free_room_structure(room);
}

void reset_creatures_rooms(struct Room *room)
{
  _DK_reset_creatures_rooms(room);
}

void replace_room_slab(struct Room *room, MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char owner, unsigned char a5)
{
    struct SlabMap *slb;
    unsigned short wlbflag;
    //_DK_replace_room_slab(room, slb_x, slb_y, owner, a5);
    if (room->kind == RoK_BRIDGE)
    {
        slb = get_slabmap_block(slb_x, slb_y);
        wlbflag = slabmap_wlb(slb);
        if (wlbflag == 0x01)
        {
          place_slab_type_on_map(12, 3 * slb_x, 3 * slb_y, game.neutral_player_num, 0);
        } else
        if (wlbflag == 0x02)
        {
            place_slab_type_on_map(13, 3 * slb_x, 3 * slb_y, game.neutral_player_num, 0);
        } else
        {
            ERRORLOG("WLB flags seem damaged for slab (%ld,%ld).",(long)slb_x,(long)slb_y);
            place_slab_type_on_map(13, 3 * slb_x, 3 * slb_y, game.neutral_player_num, 0);
        }
    } else
    {
        if ( a5 )
        {
            place_slab_type_on_map(10, 3 * slb_x, 3 * slb_y, game.neutral_player_num, 0);
        } else
        {
            place_slab_type_on_map(11, 3 * slb_x, 3 * slb_y, owner, 0);
            increase_dungeon_area(owner, 1);
        }
    }
}

struct Room *place_room(unsigned char owner, RoomKind rkind, unsigned short stl_x, unsigned short stl_y)
{
    struct Room *room;
    struct RoomData *rdata;
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    long slb_x, slb_y;
    long i;
    //return _DK_place_room(owner, rkind, stl_x, stl_y);
    game.field_14EA4B = 1;
    if (subtile_coords_invalid(stl_x, stl_y))
        return INVALID_ROOM;
    slb_x = map_to_slab[stl_x];
    slb_y = map_to_slab[stl_y];
    slb = get_slabmap_block(slb_x,slb_y);
    if (slb->room_index > 0)
    {
      delete_room_slab(slb_x, slb_y, 0);
    } else
    {
        decrease_dungeon_area(owner, 1);
        increase_room_area(owner, 1);
    }

    room = create_room(owner, rkind, stl_x, stl_y);
    if (room_is_invalid(room))
        return INVALID_ROOM;
    // Make sure we have first subtile
    stl_x = slab_starting_subtile(stl_x);
    stl_y = slab_starting_subtile(stl_y);
    // Update slab type on map
    rdata = room_data_get_for_room(room);
    i = get_slab_number(slb_x, slb_y);
    if ((rkind == RoK_GUARDPOST) || (rkind == RoK_BRIDGE))
    {
      delete_room_slabbed_objects(i);
      place_animating_slab_type_on_map(rdata->numfield_0, 0, stl_x, stl_y, owner);
    } else
    {
      delete_room_slabbed_objects(i);
      place_slab_type_on_map(rdata->numfield_0, stl_x, stl_y, owner, 0);
    }
    SYNCDBG(7,"Updating efficiency");
    do_slab_efficiency_alteration(slb_x, slb_y);
    update_room_efficiency(room);
    set_room_capacity(room,0);
    if (owner != game.neutral_player_num)
    {
        dungeon = get_dungeon(owner);
        dungeon->lvstats.rooms_constructed++;
    }
    pannel_map_update(stl_x, stl_y, 3, 3);
    return room;
}

struct Room *find_nearest_room_for_thing_with_spare_item_capacity(struct Thing *thing, char a2, char a3, unsigned char a4)
{
    return _DK_find_nearest_room_for_thing_with_spare_item_capacity(thing, a2, a3, a4);
}

struct Room * pick_random_room(PlayerNumber plyr_idx, RoomKind rkind)
{
    return _DK_pick_random_room(plyr_idx, rkind);
}

TbBool remove_item_from_room_capacity(struct Room *room)
{
    if ( (room->used_capacity <= 0) || (room->capacity_used_for_storage <= 0) )
    {
        ERRORLOG("Room %s index %d does not contain item to remove",room_code_name(room->kind),(int)room->index);
        return false;
    }
    room->used_capacity--;
    room->capacity_used_for_storage--;
    return true;
}

TbBool add_item_to_room_capacity(struct Room *room)
{
    if (room->used_capacity >= room->total_capacity)
    {
        return false;
    }
    room->used_capacity++;
    room->capacity_used_for_storage++;
    return true;
}

long claim_room(struct Room *room,struct Thing *claimtng)
{
    return _DK_claim_room(room,claimtng);
}

long claim_enemy_room(struct Room *room,struct Thing *claimtng)
{
    return _DK_claim_enemy_room(room,claimtng);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
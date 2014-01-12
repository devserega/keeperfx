/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_data.c
 *     Player data structures definitions.
 * @par Purpose:
 *     Defines functions for player-related structures support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_data.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "engine_redraw.h"
#include "frontend.h"
#include "thing_objects.h"
#include "power_hand.h"

/******************************************************************************/
/******************************************************************************/
unsigned short player_colors_map[] = {0, 1, 2, 3, 4, 5, 0, 0, 0, };

TbPixel player_path_colours[]  = {131, 90, 163, 181,  20,   4, };
TbPixel player_room_colours[]  = {132, 92, 164, 183,  21, 132, };
TbPixel player_flash_colours[] = {133, 94, 167, 142,  31,  15, };
TbPixel player_highlight_colours[] = {31, 31, 31, 31,  31,  31, };

unsigned short const player_cubes[] = {0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C7, 0x00C6 };

long neutral_player_number = NEUTRAL_PLAYER;
long hero_player_number = HERO_PLAYER;
struct PlayerInfo bad_player;
/******************************************************************************/
DLLIMPORT void _DK_set_player_state(struct PlayerInfo *player, unsigned char a1, long a2);
DLLIMPORT void _DK_set_player_mode(struct PlayerInfo *player, long val);
DLLIMPORT void _DK_reset_player_mode(struct PlayerInfo *player, unsigned char a2);
/******************************************************************************/
struct PlayerInfo *get_player_f(long plyr_idx,const char *func_name)
{
    if ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT))
        return &game.players[plyr_idx];
    ERRORMSG("%s: Tried to get non-existing player %d!",func_name,(int)plyr_idx);
    return INVALID_PLAYER;
}

TbBool player_invalid(const struct PlayerInfo *player)
{
    if (player == INVALID_PLAYER)
        return true;
    return (player < &game.players[0]);
}

/**
 * Returns if the given player is in use on current map.
 * @param player The player to check.
 * @return True if the player is in use, false otherwise.
 */
TbBool player_exists(const struct PlayerInfo *player)
{
    if (player_invalid(player))
        return false;
    return ((player->field_0 & RoF_Allocated) != 0);
}

TbBool is_my_player(const struct PlayerInfo *player)
{
    struct PlayerInfo *myplyr;
    myplyr = &game.players[my_player_number%PLAYERS_COUNT];
    return (player == myplyr);
}

TbBool is_my_player_number(PlayerNumber plyr_num)
{
    struct PlayerInfo *myplyr;
    myplyr = &game.players[my_player_number%PLAYERS_COUNT];
    return (plyr_num == myplyr->id_number);
}

/**
 * Informs if player plyr1_idx considers player plyr2_idx as enemy.
 * Note that if the players are not enemies, it doesn't necessarily mean they're friends.
 * @param origin_plyr_idx Index of the player who asks for an enemy.
 * @param check_plyr_idx Index of the player who could be enemy.
 * @return True if the players are enemies; false otherwise.
 */
TbBool players_are_enemies(long origin_plyr_idx, long check_plyr_idx)
{
    struct PlayerInfo *origin_player,*check_player;
    // Player can't be his own enemy
    if (origin_plyr_idx == check_plyr_idx)
        return false;
    // And neutral player can't be enemy
    if ((origin_plyr_idx == game.neutral_player_num) || (check_plyr_idx == game.neutral_player_num))
        return false;
    origin_player = get_player(origin_plyr_idx);
    check_player = get_player(check_plyr_idx);
    // Inactive or invalid players are not enemies, as long as they're not heroes
    // (heroes are normally NOT existing keepers)
    if (!player_exists(origin_player) && (origin_plyr_idx != game.hero_player_num))
        return false;
    if (!player_exists(check_player) && (check_plyr_idx != game.hero_player_num))
        return false;
    // And if they're valid, living players - get result from alliances table
    return ((origin_player->allied_players & (1<<check_plyr_idx)) == 0);
}

/**
 * Informs if players plyr1_idx and plyr2_idx are mutual allies.
 * If the players are not mutual allies, one side can still consider they're friends.
 * @param plyr1_idx Index of the first player.
 * @param plyr2_idx Index of the second player.
 * @return True if the players are mutual allies; false otherwise.
 */
TbBool players_are_mutual_allies(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx)
{
    struct PlayerInfo *player1,*player2;
    // Player is always his own ally
    if (plyr1_idx == plyr2_idx)
        return true;
    // And neutral player can't be allied
    if ((plyr1_idx == game.neutral_player_num) || (plyr2_idx == game.neutral_player_num))
        return false;
    player1 = get_player(plyr1_idx);
    player2 = get_player(plyr2_idx);
    // Inactive or invalid players are not allies
    if (!player_exists(player1))
        return false;
    if (!player_exists(player2))
        return false;
    return ((player1->allied_players & (1<<plyr2_idx)) != 0)
        && ((player2->allied_players & (1<<plyr1_idx)) != 0);
}

/**
 * Informs if players plyr1_idx and plyr2_idx creatures are tolerating each other.
 * This is similar to mutual alliance, but differs in conditions on nonexisting and neutral player.
 * @param plyr1_idx Index of the first player.
 * @param plyr2_idx Index of the second player.
 * @return True if the players creatures are tolerating each other; false otherwise.
 */
TbBool players_creatures_tolerate_each_other(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx)
{
    struct PlayerInfo *player1,*player2;
    // Player is always tolerating fellow creatures
    if (plyr1_idx == plyr2_idx)
        return true;
    // And neutral player creatures are like fellow creatures
    if ((plyr1_idx == game.neutral_player_num) || (plyr2_idx == game.neutral_player_num))
        return true;
    player1 = get_player(plyr1_idx);
    player2 = get_player(plyr2_idx);
    // Check if we're allied
    return ((player1->allied_players & (1<<plyr2_idx)) != 0)
        && ((player2->allied_players & (1<<plyr1_idx)) != 0);
}

TbBool player_allied_with(const struct PlayerInfo *player, PlayerNumber ally_idx)
{
    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
    {
        WARNLOG("Tried to get non-existing player!");
        return false;
    }
    return ((player->allied_players & (1<<ally_idx)) != 0);
}

/**
 * Checks if given player is either friendly to origin player or defeated.
 * @param check_plyr_idx
 * @param origin_plyr_idx
 * @return
 */
TbBool player_is_friendly_or_defeated(PlayerNumber check_plyr_idx, PlayerNumber origin_plyr_idx)
{
    struct PlayerInfo *player;
    struct PlayerInfo *win_player;
    // Handle neutral player at first, because we can't get PlayerInfo nor Dungeon for it
    if ((origin_plyr_idx == game.neutral_player_num) || (check_plyr_idx == game.neutral_player_num))
        return true;
    player = get_player(check_plyr_idx);
    win_player = get_player(origin_plyr_idx);
    if (player_exists(player))
    {
        if ( (!player_allied_with(win_player, check_plyr_idx)) || (!player_allied_with(player, origin_plyr_idx)) )
        {
            if (player_has_heart(check_plyr_idx))
              return false;
        }
    }
    return true;
}

void clear_players(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = &game.players[i];
        LbMemorySet(player, 0, sizeof(struct PlayerInfo));
        player->id_number = PLAYERS_COUNT;
    }
    LbMemorySet(&bad_player, 0, sizeof(struct PlayerInfo));
    bad_player.id_number = PLAYERS_COUNT;
    game.hero_player_num = hero_player_number;
    game.field_14E495 = 0;
    game.game_kind = GKind_NetworkGame;
}

void toggle_ally_with_player(long plyridx, unsigned int allyidx)
{
    struct PlayerInfo *player;
    player = get_player(plyridx);
    if (player_invalid(player))
        return;
    player->allied_players ^= (1 << allyidx);
}

TbBool set_ally_with_player(PlayerNumber plyridx, PlayerNumber ally_idx, TbBool state)
{
    struct PlayerInfo *player;
    player = get_player(plyridx);
    if (player_invalid(player))
        return false;
    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
        return false;
    if (state)
        player->allied_players |= (1 << ally_idx);
    else
        player->allied_players &= ~(1 << ally_idx);
    return true;
}

void set_player_state(struct PlayerInfo *player, short nwrk_state, long chosen_kind)
{
  struct Thing *thing;
  struct Coord3d pos;
  //_DK_set_player_state(player, nwrk_state, chosen_kind);
  // Selecting the same state again - update only 2nd parameter
  if (player->work_state == nwrk_state)
  {
    switch ( player->work_state )
    {
    case PSt_BuildRoom:
        player->chosen_room_kind = chosen_kind;
        break;
    case PSt_PlaceTrap:
        player->chosen_trap_kind = chosen_kind;
        break;
    case PSt_PlaceDoor:
        player->chosen_door_kind = chosen_kind;
        break;
    }
    return;
  }
  player->continue_work_state = player->work_state;
  player->work_state = nwrk_state;
  if (is_my_player(player))
    game.field_14E92E = 0;
  if ((player->work_state != PSt_Unknown12) && (player->work_state != PSt_Unknown15)
     && (player->work_state != PSt_CtrlDirect) && (player->work_state != PSt_CtrlPassngr))
  {
    player->controlled_thing_idx = 0;
  }
  switch (player->work_state)
  {
  case PSt_CtrlDungeon:
      player->field_4A4 = 1;
      break;
  case PSt_BuildRoom:
      player->chosen_room_kind = chosen_kind;
      break;
  case PSt_Unknown5:
      create_power_hand(player->id_number);
      break;
  case PSt_Slap:
      pos.x.val = 0;
      pos.y.val = 0;
      pos.z.val = 0;
      thing = create_object(&pos, 37, player->id_number, -1);
      if (thing_is_invalid(thing))
      {
        player->hand_thing_idx = 0;
        break;
      }
      player->hand_thing_idx = thing->index;
      set_power_hand_graphic(player->id_number, 785, 256);
      place_thing_in_limbo(thing);
      break;
  case PSt_PlaceTrap:
      player->chosen_trap_kind = chosen_kind;
      break;
  case PSt_PlaceDoor:
      player->chosen_door_kind = chosen_kind;
      break;
  default:
      break;
  }
}

void set_player_mode(struct PlayerInfo *player, long nview)
{
  long i;
  if (player->view_type == nview)
    return;
  player->view_type = nview;
  player->field_0 &= ~0x08;
  if (is_my_player(player))
  {
    game.numfield_D &= 0xF7;
    game.numfield_D |= 0x01;
    if (is_my_player(player))
      stop_all_things_playing_samples();
  }
  switch (player->view_type)
  {
  case PVT_DungeonTop:
      i = 2;
      if (player->field_4B5 == 5)
      {
        set_engine_view(player, 2);
        i = 5;
      }
      set_engine_view(player, i);
      if (is_my_player(player))
        toggle_status_menu((game.numfield_C & 0x40) != 0);
      if ((game.numfield_C & 0x20) != 0)
        setup_engine_window(status_panel_width, 0, MyScreenWidth, MyScreenHeight);
      else
        setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
      break;
  case PVT_CreatureContrl:
  case PVT_CreaturePasngr:
      set_engine_view(player, 1);
      if (is_my_player(player))
        game.numfield_D &= ~0x01;
      setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
      break;
  case PVT_MapScreen:
      player->continue_work_state = player->work_state;
      set_engine_view(player, 3);
      break;
  case PVT_MapFadeIn:
      set_player_instance(player, PI_MapFadeTo, 0);
      break;
  case PVT_MapFadeOut:
      set_player_instance(player, PI_MapFadeFrom, 0);
      break;
  }
}

void reset_player_mode(struct PlayerInfo *player, unsigned short nmode)
{
  //_DK_reset_player_mode(player, nmode);
  player->view_type = nmode;
  switch (nmode)
  {
    case 1:
      player->work_state = player->continue_work_state;
      if (player->field_4B5 == 5)
        set_engine_view(player, 5);
      else
        set_engine_view(player, 2);
      if (is_my_player(player))
        game.numfield_D &= ~0x01;
      break;
    case 2:
    case 3:
      player->work_state = player->continue_work_state;
      set_engine_view(player, 1);
      if (is_my_player(player))
        game.numfield_D |= 0x01;
      break;
    case 4:
      player->work_state = player->continue_work_state;
      set_engine_view(player, 3);
      if (is_my_player(player))
        game.numfield_D &= ~0x01;
      break;
    default:
      break;
  }
}
/******************************************************************************/

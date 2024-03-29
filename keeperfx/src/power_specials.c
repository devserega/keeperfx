/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_specials.c
 *     power_specials support functions.
 * @par Purpose:
 *     Functions to power_specials.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "power_specials.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "thing_creature.h"
#include "thing_navigate.h"
#include "thing_effects.h"
#include "config_terrain.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "creature_control.h"
#include "creature_states.h"
#include "power_hand.h"
#include "game_saves.h"
#include "game_merge.h"
#include "slab_data.h"
#include "map_blocks.h"
#include "spdigger_stack.h"
#include "thing_corpses.h"
#include "thing_objects.h"
#include "front_simple.h"
#include "frontend.h"
#include "frontmenu_ingame_map.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_make_safe(struct PlayerInfo *player);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Makes a bonus level for current SP level visible on the land map screen.
 */
TbBool activate_bonus_level(struct PlayerInfo *player)
{
  TbBool result;
  LevelNumber sp_lvnum;
  SYNCDBG(5,"Starting");
  set_flag_byte(&game.flags_font,FFlg_unk02,true);
  sp_lvnum = get_loaded_level_number();
  result = set_bonus_level_visibility_for_singleplayer_level(player, sp_lvnum, true);
  if (!result)
    ERRORLOG("No Bonus level assigned to level %d",(int)sp_lvnum);
  set_flag_byte(&game.numfield_C,0x02,false);
  return result;
}

void multiply_creatures_in_dungeon_list(struct Dungeon *dungeon, long list_start)
{
    struct Thing *thing;
    struct Thing *tncopy;
    struct CreatureControl *cctrl;
    unsigned long k;
    int i;
    k = 0;
    i = list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        tncopy = create_creature(&thing->mappos, thing->model, dungeon->owner);
        if (thing_is_invalid(tncopy))
        {
            WARNLOG("Can't create a copy of creature");
            break;
        }
        set_creature_level(tncopy, cctrl->explevel);
        tncopy->health = thing->health;
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
}

void multiply_creatures(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  dungeon = get_players_dungeon(player);
  // Copy 'normal' creatures
  multiply_creatures_in_dungeon_list(dungeon, dungeon->creatr_list_start);
  // Copy 'special worker' creatures
  multiply_creatures_in_dungeon_list(dungeon, dungeon->digger_list_start);
}

void increase_level(struct PlayerInfo *player)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    dungeon = get_dungeon(player->id_number);
    // Increase level of normal creatures
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
          ERRORLOG("Jump to invalid creature detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        creature_increase_level(thing);
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
    // Increase level of special workers
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
          ERRORLOG("Jump to invalid creature detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        creature_increase_level(thing);
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping creatures list");
          break;
        }
    }
}

TbBool steal_hero(struct PlayerInfo *player, struct Coord3d *pos)
{
    //TODO CONFIG creature models dependency; put them in config files
    static ThingModel skip_steal_models[] = {6, 7};
    static ThingModel prefer_steal_models[] = {3, 12};
    struct Thing *herotng;
    herotng = INVALID_THING;
    int heronum;
    struct Dungeon *herodngn;
    struct CreatureControl *cctrl;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    herodngn = get_players_num_dungeon(game.hero_player_num);
    k = 0;
    if (herodngn->num_active_creatrs > 0) {
        heronum = ACTION_RANDOM(herodngn->num_active_creatrs);
        i = herodngn->creatr_list_start;
        SYNCDBG(4,"Selecting random creature %d out of %d heroes",(int)heronum,(int)herodngn->num_active_creatrs);
    } else {
        heronum = 0;
        i = 0;
        SYNCDBG(4,"No heroes on map, skipping selection");
    }
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        TbBool heroallow;
        heroallow = true;
        ThingModel skipidx;
        for (skipidx=0; skipidx < sizeof(skip_steal_models)/sizeof(skip_steal_models[0]); skipidx++)
        {
            if (thing->model == skip_steal_models[skipidx]) {
                heroallow = false;
            }
        }
        if (heroallow) {
            herotng = thing;
        }
        // If we've reached requested hero number, return either current hero on previously selected one
        if ((heronum <= 0) && thing_is_creature(herotng)) {
            break;
        }
        heronum--;
        if (i == 0) {
            i = herodngn->creatr_list_start;
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    if (!thing_is_invalid(herotng))
    {
        move_thing_in_map(herotng, pos);
        change_creature_owner(herotng, player->id_number);
        SYNCDBG(3,"Converted %s to owner %d",thing_model_name(herotng),(int)player->id_number);
    }
    else
    {
        i = ACTION_RANDOM(sizeof(prefer_steal_models)/sizeof(prefer_steal_models[0]));
        struct Thing *creatng;
        creatng = create_creature(pos, prefer_steal_models[i], player->id_number);
        if (thing_is_invalid(creatng))
            return false;
        SYNCDBG(3,"Created %s owner %d",thing_model_name(creatng),(int)player->id_number);
    }
    return true;
}

void make_safe(struct PlayerInfo *player)
{
    //_DK_make_safe(player);
    unsigned char *areamap;
    areamap = (unsigned char *)scratch;
    MapSlabCoord slb_x, slb_y;
    // Prepare the array to remember which slabs were already taken care of
    for (slb_y=0; slb_y < map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < map_tiles_x; slb_x++)
        {
            SlabCodedCoords slb_num;
            struct SlabMap *slb;
            slb_num = get_slab_number(slb_x, slb_y);
            slb = get_slabmap_direct(slb_num);
            struct SlabAttr *slbattr;
            slbattr = get_slab_attrs(slb);
            if ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0)
                areamap[slb_num] = 0x01;
            else
                areamap[slb_num] = 0x00;
        }
    }
    {
        const struct Coord3d *center_pos;
        center_pos = dungeon_get_essential_pos(player->id_number);
        slb_x = subtile_slab_fast(center_pos->x.stl.num);
        slb_y = subtile_slab_fast(center_pos->y.stl.num);
        SlabCodedCoords slb_num;
        slb_num = get_slab_number(slb_x, slb_y);
        areamap[slb_num] |= 0x02;
    }

    unsigned int list_cur, list_len;
    PlayerNumber plyr_idx;
    plyr_idx = player->id_number;
    SlabCodedCoords *slblist;
    slblist = (SlabCodedCoords *)(scratch + map_tiles_x*map_tiles_y);
    list_len = 0;
    list_cur = 0;
    while (list_cur <= list_len)
    {
        SlabCodedCoords slb_num;

        if (slb_x > 0)
        {
            slb_num = get_slab_number(slb_x-1, slb_y);
            if ((areamap[slb_num] & 0x01) != 0)
            {
                areamap[slb_num] |= 0x02;
                struct SlabMap *slb;
                slb = get_slabmap_direct(slb_num);
                struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                if ((slbattr->category == SlbAtCtg_FriableDirt) && slab_by_players_land(plyr_idx, slb_x-1, slb_y))
                {
                    unsigned char pretty_type;
                    pretty_type = choose_pretty_type(plyr_idx, slb_x-1, slb_y);
                    place_slab_type_on_map(pretty_type, slab_subtile(slb_x-1,0), slab_subtile(slb_y,0), plyr_idx, 1);
                }
            } else
            if ((areamap[slb_num] & 0x02) == 0)
            {
                areamap[slb_num] |= 0x02;
                slblist[list_len] = slb_num;
                list_len++;
            }
        }
        if (slb_x < map_tiles_x-1)
        {
            slb_num = get_slab_number(slb_x+1, slb_y);
            if ((areamap[slb_num] & 0x01) != 0)
            {
                areamap[slb_num] |= 0x02;
                struct SlabMap *slb;
                slb = get_slabmap_direct(slb_num);
                struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                if ((slbattr->category == SlbAtCtg_FriableDirt) &&  slab_by_players_land(plyr_idx, slb_x+1, slb_y))
                {
                    unsigned char pretty_type;
                    pretty_type = choose_pretty_type(plyr_idx, slb_x+1, slb_y);
                    place_slab_type_on_map(pretty_type, slab_subtile(slb_x+1,0), slab_subtile(slb_y,0), plyr_idx, 1u);
                }
            } else
            if ((areamap[slb_num] & 0x02) == 0)
            {
                areamap[slb_num] |= 0x02;
                slblist[list_len] = slb_num;
                list_len++;
            }
        }
        if (slb_y > 0)
        {
            slb_num = get_slab_number(slb_x, slb_y-1);
            if ((areamap[slb_num] & 0x01) != 0)
            {
                areamap[slb_num] |= 0x02;
                struct SlabMap *slb;
                slb = get_slabmap_direct(slb_num);
                struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                if ((slbattr->category == SlbAtCtg_FriableDirt) && slab_by_players_land(plyr_idx, slb_x, slb_y-1))
                {
                    unsigned char pretty_type;
                    pretty_type = choose_pretty_type(plyr_idx, slb_x, slb_y - 1);
                    place_slab_type_on_map(pretty_type, slab_subtile(slb_x,0), slab_subtile(slb_y-1,0), plyr_idx, 1u);
                }
            } else
            if ((areamap[slb_num] & 0x02) == 0)
            {
                areamap[slb_num] |= 0x02;
                slblist[list_len] = slb_num;
                list_len++;
            }
        }
        if (slb_y < map_tiles_y-1)
        {
            slb_num = get_slab_number(slb_x, slb_y+1);
            if ((areamap[slb_num] & 0x01) != 0)
            {
                areamap[slb_num] |= 0x02;
                struct SlabMap *slb;
                slb = get_slabmap_direct(slb_num);
                struct SlabAttr *slbattr;
                slbattr = get_slab_attrs(slb);
                if ((slbattr->category == SlbAtCtg_FriableDirt) && slab_by_players_land(plyr_idx, slb_x, slb_y+1))
                {
                    unsigned char pretty_type;
                    pretty_type = choose_pretty_type(plyr_idx, slb_x, slb_y+1);
                    place_slab_type_on_map(pretty_type, slab_subtile(slb_x,0), slab_subtile(slb_y+1,0), plyr_idx, 1u);
                }
            } else
            if ((areamap[slb_num] & 0x02) == 0)
            {
                areamap[slb_num] |= 0x02;
                slblist[list_len] = slb_num;
                list_len++;
            }
        }

        slb_x = slb_num_decode_x(slblist[list_cur]);
        slb_y = slb_num_decode_y(slblist[list_cur]);
        list_cur++;
    }
    pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
}

void activate_dungeon_special(struct Thing *cratetng, struct PlayerInfo *player)
{
  SYNCDBG(6,"Starting");
  short used;
  struct Coord3d pos;
  int spkindidx;

  // Gathering data which we'll need if the special is used and disposed.
  memcpy(&pos,&cratetng->mappos,sizeof(struct Coord3d));
  spkindidx = cratetng->model - 86;
  used = 0;
  if (thing_exists(cratetng) && is_dungeon_special(cratetng))
  {
    switch (cratetng->model)
    {
        case 86:
          reveal_whole_map(player);
          remove_events_thing_is_attached_to(cratetng);
          used = 1;
          delete_thing_structure(cratetng, 0);
          break;
        case 87:
          start_resurrect_creature(player, cratetng);
          break;
        case 88:
          start_transfer_creature(player, cratetng);
          break;
        case 89:
          if (steal_hero(player, &cratetng->mappos))
          {
            remove_events_thing_is_attached_to(cratetng);
            used = 1;
            delete_thing_structure(cratetng, 0);
          }
          break;
        case 90:
          multiply_creatures(player);
          remove_events_thing_is_attached_to(cratetng);
          used = 1;
          delete_thing_structure(cratetng, 0);
          break;
        case 91:
          increase_level(player);
          remove_events_thing_is_attached_to(cratetng);
          used = 1;
          delete_thing_structure(cratetng, 0);
          break;
        case 92:
          make_safe(player);
          remove_events_thing_is_attached_to(cratetng);
          used = 1;
          delete_thing_structure(cratetng, 0);
          break;
        case 93:
          activate_bonus_level(player);
          remove_events_thing_is_attached_to(cratetng);
          used = 1;
          delete_thing_structure(cratetng, 0);
          break;
        default:
          ERRORLOG("Invalid dungeon special (Model %d)", (int)cratetng->model);
          break;
      }
      if ( used )
      {
        if (is_my_player(player))
          output_message(special_desc[spkindidx].speech_msg, 0, true);
        create_special_used_effect(&pos, player->id_number);
      }
  }
}

void resurrect_creature(struct Thing *boxtng, PlayerNumber owner, ThingModel crmodel, unsigned char crlevel)
{
    struct Thing *creatng;
    if (!thing_exists(boxtng) || (box_thing_to_special(boxtng) != SpcKind_Resurrect) ) {
        ERRORMSG("Invalid resurrect box object!");
        return;
    }
    creatng = create_creature(&boxtng->mappos, crmodel, owner);
    if (!thing_is_invalid(creatng))
    {
        init_creature_level(creatng, crlevel);
        if (is_my_player_number(owner))
          output_message(SMsg_CommonAcknowledge, 0, true);
    }
    create_special_used_effect(&boxtng->mappos, owner);
    remove_events_thing_is_attached_to(boxtng);
    force_any_creature_dragging_owned_thing_to_drop_it(boxtng);
    if ((gameadd.classic_bugs_flags & ClscBug_ResurrectForever) == 0) {
        remove_item_from_dead_creature_list(get_players_num_dungeon(owner), crmodel, crlevel);
    }
    delete_thing_structure(boxtng, 0);
}

void transfer_creature(struct Thing *boxtng, struct Thing *transftng, unsigned char plyr_idx)
{
    SYNCDBG(7,"Starting");
    struct CreatureControl *cctrl;
    if (!thing_exists(boxtng) || (box_thing_to_special(boxtng) != SpcKind_TrnsfrCrtr) ) {
        ERRORMSG("Invalid transfer box object!");
        return;
    }
    // Check if 'things' are correct
    if (!thing_exists(transftng) || !thing_is_creature(transftng) || (transftng->owner != plyr_idx)) {
        ERRORMSG("Invalid transfer creature thing!");
        return;
    }

    cctrl = creature_control_get_from_thing(transftng);
    set_transfered_creature(plyr_idx, transftng->model, cctrl->explevel);
    remove_thing_from_power_hand_list(transftng, plyr_idx);
    kill_creature(transftng, INVALID_THING, -1, CrDed_NoEffects|CrDed_NotReallyDying);
    create_special_used_effect(&boxtng->mappos, plyr_idx);
    remove_events_thing_is_attached_to(boxtng);
    force_any_creature_dragging_owned_thing_to_drop_it(boxtng);
    delete_thing_structure(boxtng, 0);
    if (is_my_player_number(plyr_idx))
      output_message(SMsg_CommonAcknowledge, 0, true);
}

void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  if (dungeon->num_active_creatrs != 0)
  {
    if (is_my_player(player))
    {
      dungeon_special_selected = thing->index;
      transfer_creature_scroll_offset = 0;
      turn_off_menu(GMnu_DUNGEON_SPECIAL);
      turn_on_menu(GMnu_TRANSFER_CREATURE);
    }
  }
}

void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(player->id_number);
    if (dungeon->dead_creatures_count != 0)
    {
        if (is_my_player(player))
        {
          dungeon_special_selected = thing->index;
          resurrect_creature_scroll_offset = 0;
          turn_off_menu(GMnu_DUNGEON_SPECIAL);
          turn_on_menu(GMnu_RESURRECT_CREATURE);
        }
    }
}

TbBool create_transferred_creature_on_level(void)
{
    struct Thing *thing;
    struct Coord3d *pos;
    if (game.intralvl_transfered_creature.model > 0)
    {
        thing = get_player_soul_container(my_player_number);
        pos = &(thing->mappos);
        thing = create_creature(pos, game.intralvl_transfered_creature.model, 5);
        if (thing_is_invalid(thing))
          return false;
        init_creature_level(thing, game.intralvl_transfered_creature.explevel);
        clear_transfered_creature();
        return true;
    }
    return false;
}
/******************************************************************************/

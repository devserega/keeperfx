/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_entrance.c
 *     Entrance maintain functions.
 * @par Purpose:
 *     Functions to create and use entrances.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_entrance.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "room_data.h"
#include "room_lair.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "player_utils.h"
#include "thing_data.h"
#include "thing_navigate.h"
#include "creature_states.h"
#include "config_creature.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_process_entrance_generation(void);
DLLIMPORT struct Thing *_DK_create_creature_at_entrance(struct Room * room, unsigned short crmodel);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_creature_at_entrance(struct Room * room, ThingModel crkind)
{
    //return _DK_create_creature_at_entrance(room, crtr_kind);
    struct Thing *creatng;
    struct Coord3d pos;
    pos.x.val = room->central_stl_x;
    pos.y.val = room->central_stl_y;
    pos.z.val = subtile_coord(1,0);
    creatng = create_creature(&pos, crkind, room->owner);
    if (thing_is_invalid(creatng)) {
        ERRORLOG("Cannot create creature %s for player %d entrance",creature_code_name(crkind),(int)room->owner);
        return INVALID_THING;
    }
    mark_creature_joined_dungeon(creatng);
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        ERRORLOG("Cannot find a valid place in player %d entrance to create creature %s",(int)room->owner,creature_code_name(crkind));
        delete_thing_structure(creatng, 0);
        return INVALID_THING;
    }
    move_thing_in_map(creatng, &pos);
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(room->owner);
        dungeon->lvstats.field_4++;
        dungeon->lvstats.field_8++;
        dungeon->lvstats.field_88 = crkind;
    }
    struct Thing *heartng;
    heartng = get_player_soul_container(room->owner);
    TRACE_THING(heartng);
    if (!thing_is_invalid(heartng))
    {
        if (setup_person_move_to_position(creatng, heartng->mappos.x.stl.num, heartng->mappos.y.stl.num, 0)) {
            creatng->continue_state = CrSt_CreaturePresentToDungeonHeart;
        } else {
            heartng = INVALID_THING;
        }
    }
    if (thing_is_invalid(heartng))
    {
        set_start_state(creatng);
    }
    return creatng;
}

/** Checks if an entrance shall now generate next creature.
 *
 * @return Gives true if an entrance shall generate, false otherwise.
 */
TbBool generation_due_in_game(void)
{
    if (game.generate_speed == -1)
        return true;
    return ( (game.play_gameturn-game.entrance_last_generate_turn) >= game.generate_speed );
}

TbBool generation_due_for_dungeon(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    if ( (game.armageddon_cast_turn == 0) || (game.armageddon.count_down + game.armageddon_cast_turn > game.play_gameturn) )
    {
        if ( (dungeon->turns_between_entrance_generation != -1) &&
             (game.play_gameturn - dungeon->last_entrance_generation_gameturn >= dungeon->turns_between_entrance_generation) ) {
            return true;
        }
    }
    return false;
}

TbBool generation_available_to_dungeon(const struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    if (!dungeon_has_room(dungeon, RoK_ENTRANCE))
        return false;
    return ((long)dungeon->num_active_creatrs < (long)dungeon->max_creatures_attracted);
}

long calculate_attractive_room_quantity(RoomKind room_kind, PlayerNumber plyr_idx, int crmodel)
{
    struct Dungeon * dungeon;
    long used_fraction;
    long slabs_count;

    dungeon = get_dungeon(plyr_idx);
    slabs_count = get_room_slabs_count(plyr_idx, room_kind);

    switch (room_kind)
    {
    case RoK_LAIR:
        // Add one attractiveness per 2 unused slabs in the room
        used_fraction = get_room_kind_used_capacity_fraction(plyr_idx, room_kind);
        return (slabs_count * (256-used_fraction)) / 256 / 2 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_DUNGHEART:
    case RoK_BRIDGE:
        // Add one attractiveness per 9 slabs of such room
        return slabs_count / 9 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_ENTRANCE:
    case RoK_LIBRARY:
    case RoK_PRISON:
    case RoK_TORTURE:
    case RoK_TRAINING:
    case RoK_SCAVENGER:
    case RoK_TEMPLE:
    case RoK_GRAVEYARD:
    case RoK_BARRACKS:
    case RoK_GUARDPOST:
        // Add one attractiveness per 3 slabs of such room
        return slabs_count / 3 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_WORKSHOP:
    case RoK_GARDEN:
        // Add one attractiveness per 4 slabs of such room
        return slabs_count / 4 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_TREASURE:
        // Add one attractiveness per 3 used slabs in the room
        used_fraction = get_room_kind_used_capacity_fraction(plyr_idx, room_kind);
        return (slabs_count * used_fraction) / 256 / 3;
    case RoK_NONE:
    default:
        return 0;
    }
}

long calculate_excess_attraction_for_creature(ThingModel crkind, PlayerNumber plyr_idx)
{
    struct CreatureStats * stats;

    SYNCDBG(11, "Starting");

    stats = creature_stats_get(crkind);
    long excess_attraction = 0;
    int i;
    for (i=0; i < 3; i++)
    {
        RoomKind room_kind;
        room_kind = stats->entrance_rooms[i];
        if ((room_kind != RoK_NONE) && (stats->entrance_slabs_req[i] > 0)) {
            // First room adds fully to attraction, second adds only 1/2, third adds 1/3
            excess_attraction += calculate_attractive_room_quantity(room_kind, plyr_idx, crkind) / (i+1);
        }
    }
    return excess_attraction;
}

TbBool creature_will_generate_for_dungeon(const struct Dungeon * dungeon, ThingModel crkind)
{
    struct CreatureStats * stats;
    int i;

    SYNCDBG(11, "Starting for creature kind %s", creature_code_name(crkind));

    if (game.pool.crtr_kind[crkind] <= 0) {
        return false;
    }

    // Not allowed creatures can never be attracted
    if (!dungeon->creature_allowed[crkind]) {
        return false;
    }

    // Enabled creatures don't need additional conditions to be met
    if (dungeon->creature_force_enabled[crkind] > dungeon->creature_models_joined[crkind]) {
        return true;
    }

    // Typical way is to allow creatures which meet attraction conditions
    stats = creature_stats_get(crkind);

    // Check if we've got rooms of enough size for attraction
    for (i = 0; i < 3; ++i)
    {
        RoomKind room_kind;
        int slabs_count;
        room_kind = stats->entrance_rooms[i];

        if (room_kind != RoK_NONE) {
            slabs_count = get_room_slabs_count(dungeon->owner, room_kind);

            if (slabs_count < stats->entrance_slabs_req[i]) {
                return false;
            }
        }
    }

    return true;
}

TbBool remove_creature_from_generate_pool(ThingModel crtr_kind)
{
    if (game.pool.crtr_kind[crtr_kind] <= 0) {
        WARNLOG("Could not remove creature %s from the creature pool",creature_code_name(crtr_kind));
        return false;
    }
    game.pool.crtr_kind[crtr_kind]--;
    return true;
}

int calculate_creature_to_generate_for_dungeon(struct Dungeon * dungeon)
{
    long cum_freq; //cumulative frequency
    long gen_count;
    long crtr_freq[CREATURE_TYPES_COUNT];
    long rnd;
    long score;
    long i;

    SYNCDBG(9,"Starting");

    cum_freq = 0;
    gen_count = 0;
    crtr_freq[0] = 0;
    for (i = 1; i < CREATURE_TYPES_COUNT; ++i) {
        if (creature_will_generate_for_dungeon(dungeon, i))
        {
            struct CreatureStats *crstat;
            crstat = creature_stats_get(i);

            gen_count += 1;

            score = (long)crstat->entrance_score
                + calculate_excess_attraction_for_creature(i, dungeon->owner);
            if (score < 1) {
                score = 1;
            }
            cum_freq += score;
            crtr_freq[i] = cum_freq;
        }
        else {
            crtr_freq[i] = 0;
        }
    }

    // Select a creature kind to generate based on score we've got for every kind
    // Scores define a chance of being generated.
    if (gen_count > 0) {
        if (cum_freq > 0) {
            rnd = ACTION_RANDOM(cum_freq);

            i = 1;
            while (rnd >= crtr_freq[i]) {
                ++i;
                if (i >= CREATURE_TYPES_COUNT) {
                    ERRORLOG("Internal problem; got outside of cummulative range.");
                    return 0;
                }
            }

            return i;
        }
        else {
            ERRORLOG("Bad configuration; creature available but no scores for randomization.");
        }
    }

    return 0;
}

TbBool generate_creature_at_random_entrance(struct Dungeon * dungeon, ThingModel crtr_kind)
{
    struct Room * room;

    SYNCDBG(9,"Starting");

    room = pick_random_room(dungeon->owner, RoK_ENTRANCE);
    if (room_is_invalid(room))
    {
        ERRORLOG("Could not get a random entrance for player %d",(int)dungeon->owner);
        return false;
    }
    struct Thing *creatng;
    creatng = create_creature_at_entrance(room, crtr_kind);
    if (thing_is_invalid(creatng)) {
        return false;
    }
    remove_creature_from_generate_pool(crtr_kind);
    return true;
}

void generate_creature_for_dungeon(struct Dungeon * dungeon)
{
    ThingModel crkind;
    long lair_space;
    struct CreatureStats *crstat;

    SYNCDBG(9,"Starting");

    crkind = calculate_creature_to_generate_for_dungeon(dungeon);
    crstat = creature_stats_get(crkind);

    if (crkind > 0) {
        lair_space = calculate_free_lair_space(dungeon);
        if ((long)crstat->pay > dungeon->total_money_owned)
        {
            if (is_my_player_number(dungeon->owner)) {
                output_message(SMsg_GoldLow, MESSAGE_DELAY_TREASURY, true);
            }
        } else
        if (lair_space > 0)
        {
            generate_creature_at_random_entrance(dungeon, crkind);
        } else
        if (lair_space == 0)
        {
            generate_creature_at_random_entrance(dungeon, crkind);

            if (dungeon_has_room(dungeon, RoK_LAIR))
            {
                event_create_event_or_update_nearby_existing_event(0, 0,
                    EvKind_NoMoreLivingSet, dungeon->owner, 0);
                output_message_room_related_from_computer_or_player_action(dungeon->owner, RoK_LAIR, OMsg_RoomTooSmall);
            } else
            {
                output_message_room_related_from_computer_or_player_action(dungeon->owner, RoK_LAIR, OMsg_RoomNeeded);
            }
        }
    }
}

void process_entrance_generation(void)
{
    struct PlayerInfo *plyr;
    struct Dungeon *dungeon;
    long i;
    SYNCDBG(8,"Starting");
    //_DK_process_entrance_generation();

    if (generation_due_in_game())
    {
        if (game.armageddon_cast_turn == 0) {
            update_dungeons_scores();
            update_dungeon_generation_speeds();
            game.entrance_last_generate_turn = game.play_gameturn;
        }
    }

    for (i = 0; i < PLAYERS_COUNT; i++)
    {
        plyr = get_player(i);
        if (!player_exists(plyr)) {
            continue;
        }
        if ((plyr->field_2C == 1) && (plyr->victory_state != VicS_LostLevel) )
        {
            dungeon = get_players_dungeon(plyr);
            if (generation_due_for_dungeon(dungeon))
            {
                if (generation_available_to_dungeon(dungeon)) {
                    generate_creature_for_dungeon(dungeon);
                }
                dungeon->last_entrance_generation_gameturn = game.play_gameturn;
            }
            dungeon->field_1485 = 0;
        }
    }
}
/******************************************************************************/

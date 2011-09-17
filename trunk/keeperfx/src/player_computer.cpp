/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer.c
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include <limits.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"
#include "bflib_math.h"

#include "config.h"
#include "creature_states.h"
#include "magic.h"
#include "keeperfx.hpp"
#include "skirmish_ai.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_compplayer_file[]="keepcompp.cfg";

const struct NamedCommand compp_common_commands[] = {
  {"COMPUTERASSISTS", 1},
  {"PROCESSESCOUNT",  2},
  {"CHECKSCOUNT",     3},
  {"EVENTSCOUNT",     4},
  {"COMPUTERSCOUNT",  5},
  {NULL,              0},
  };

const struct NamedCommand compp_process_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PARAMS",          4},
  {"MNEMONIC",        5},
  {NULL,              0},
  };

const struct NamedCommand compp_check_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PARAMS",          4},
  {"MNEMONIC",        5},
  {NULL,              0},
  };

const struct NamedCommand compp_event_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PROCESS",         4},
  {"PARAMS",          5},
  {"MNEMONIC",        6},
  {NULL,              0},
  };

const struct NamedCommand compp_computer_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"PROCESSES",       3},
  {"CHECKS",          4},
  {"EVENTS",          5},
  {NULL,              0},
  };

ComputerType computer_assist_types[] = { 6, 7, 8, 9 };
unsigned short computer_types[] = { 201, 201, 201, 201, 201, 201, 729, 730, 731, 732 };

/******************************************************************************/
DLLIMPORT void _DK_setup_computer_players2(void);
DLLIMPORT void _DK_process_computer_player2(unsigned long plridx);
DLLIMPORT void _DK_setup_a_computer_player(unsigned long a1, long a2);
DLLIMPORT struct ComputerTask *_DK_computer_setup_build_room(struct Computer2 *comp, unsigned short a2, long a3, long a4, long a5);
DLLIMPORT void _DK_process_computer_players2(void);
DLLIMPORT long _DK_set_next_process(struct Computer2 *comp);
DLLIMPORT struct ComputerProcess * _DK_find_best_process(struct Computer2 *comp);
DLLIMPORT void _DK_computer_check_events(struct Computer2 *comp);
DLLIMPORT long _DK_process_checks(struct Computer2 *comp);
DLLIMPORT long _DK_process_tasks(struct Computer2 *comp);
DLLIMPORT long _DK_get_computer_money_less_cost(struct Computer2 *comp);

DLLIMPORT long _DK_computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_build_all_rooms _DK_computer_check_build_all_rooms
DLLIMPORT long _DK_computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_any_room_continue _DK_computer_setup_any_room_continue
DLLIMPORT long _DK_computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_any_room _DK_computer_check_any_room
DLLIMPORT long _DK_computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_any_room _DK_computer_setup_any_room
DLLIMPORT long _DK_computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_dig_to_entrance _DK_computer_check_dig_to_entrance
DLLIMPORT long _DK_computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_dig_to_entrance _DK_computer_setup_dig_to_entrance
DLLIMPORT long _DK_computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_dig_to_gold _DK_computer_check_dig_to_gold
DLLIMPORT long _DK_computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_dig_to_gold _DK_computer_setup_dig_to_gold
DLLIMPORT long _DK_computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_sight_of_evil _DK_computer_check_sight_of_evil
DLLIMPORT long _DK_computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_sight_of_evil _DK_computer_setup_sight_of_evil
DLLIMPORT long _DK_computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_process_sight_of_evil _DK_computer_process_sight_of_evil
DLLIMPORT long _DK_computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_attack1 _DK_computer_check_attack1
DLLIMPORT long _DK_computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_setup_attack1 _DK_computer_setup_attack1
DLLIMPORT long _DK_computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_completed_attack1 _DK_computer_completed_attack1
DLLIMPORT long _DK_computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_check_safe_attack _DK_computer_check_safe_attack
DLLIMPORT long _DK_computer_process_task(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_process_task _DK_computer_process_task
DLLIMPORT long _DK_computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_completed_build_a_room _DK_computer_completed_build_a_room
DLLIMPORT long _DK_computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_paused_task _DK_computer_paused_task
DLLIMPORT long _DK_computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process);
//#define computer_completed_task _DK_computer_completed_task

DLLIMPORT long _DK_computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);
DLLIMPORT long _DK_count_creatures_for_defend_pickup(struct Computer2 *comp);
DLLIMPORT long _DK_computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos);
DLLIMPORT long _DK_computer_able_to_use_magic(struct Computer2 *comp, long a2, long a3, long a4);
DLLIMPORT long _DK_check_call_to_arms(struct Computer2 *comp);
DLLIMPORT long _DK_computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlook);

DLLIMPORT long _DK_computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
DLLIMPORT long _DK_computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
DLLIMPORT long _DK_computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
DLLIMPORT long _DK_computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);

DLLIMPORT extern struct ComputerProcess _DK_BuildAllRooms3x3;
#define BuildAllRooms3x3 _DK_BuildAllRooms3x3
DLLIMPORT extern struct ComputerProcess _DK_BuildAllRooms4x4;
#define BuildAllRooms4x4 _DK_BuildAllRooms4x4
DLLIMPORT extern struct ComputerProcess _DK_BuildPrisonRoom;
#define BuildPrisonRoom _DK_BuildPrisonRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTortureRoom;
#define BuildTortureRoom _DK_BuildTortureRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildScavengerRoom;
#define BuildScavengerRoom _DK_BuildScavengerRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTempleRoom;
#define BuildTempleRoom _DK_BuildTempleRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildGraveyardRoom;
#define BuildGraveyardRoom _DK_BuildGraveyardRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildBarrackRoom;
#define BuildBarrackRoom _DK_BuildBarrackRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTreasureRoom;
#define BuildTreasureRoom _DK_BuildTreasureRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildResearchRoom;
#define BuildResearchRoom _DK_BuildResearchRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildHatcheryRoom;
#define BuildHatcheryRoom _DK_BuildHatcheryRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildLairRoom;
#define BuildLairRoom _DK_BuildLairRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTrainingRoom;
#define BuildTrainingRoom _DK_BuildTrainingRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildWorkshopRoom;
#define BuildWorkshopRoom _DK_BuildWorkshopRoom
DLLIMPORT extern struct ComputerProcess _DK_DigToEntrance;
#define DigToEntrance _DK_DigToEntrance
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldForMoney;
#define DigToGoldForMoney _DK_DigToGoldForMoney
DLLIMPORT extern struct ComputerProcess _DK_BuildTreasureRoom4x4;
#define BuildTreasureRoom4x4 _DK_BuildTreasureRoom4x4
DLLIMPORT extern struct ComputerProcess _DK_BuildLairRoom4x4;
#define BuildLairRoom4x4 _DK_BuildLairRoom4x4
DLLIMPORT extern struct ComputerProcess _DK_DigToCloseGoldForMoney;
#define DigToCloseGoldForMoney _DK_DigToCloseGoldForMoney
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldGreedy;
#define DigToGoldGreedy _DK_DigToGoldGreedy
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldGreedy2;
#define DigToGoldGreedy2 _DK_DigToGoldGreedy2
DLLIMPORT extern struct ComputerProcess _DK_ComputerSightOfEvil;
#define ComputerSightOfEvil _DK_ComputerSightOfEvil
DLLIMPORT extern struct ComputerProcess _DK_ComputerSightOfEvilScare;
#define ComputerSightOfEvilScare _DK_ComputerSightOfEvilScare
DLLIMPORT extern struct ComputerProcess _DK_ComputerAttack1;
#define ComputerAttack1 _DK_ComputerAttack1
DLLIMPORT extern struct ComputerProcess _DK_ComputerSafeAttack;
#define ComputerSafeAttack _DK_ComputerSafeAttack

/******************************************************************************/
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process);
long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process);
long computer_process_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process);
long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process);

/*TODO enable ComputerProcess structs when there are no references to those in DLL
struct ComputerProcess BuildAllRooms3x3 = {
  "BUILD ALL ROOM 3x3", 0, 3, 3, 0, -1, computer_check_build_all_rooms,
  computer_setup_any_room_continue, computer_process_task,
  computer_completed_build_a_room, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildAllRooms4x4 = {
  "BUILD ALL ROOM 4x4", 0, 4, 4, 0, -1, computer_check_build_all_rooms,
  computer_setup_any_room_continue, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildPrisonRoom = {
  "BUILD A PRISON ROOM", 0, 3, 4, 4, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTortureRoom = {
  "BUILD A TORTURE ROOM", 0, 3, 4, 5, 4, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildScavengerRoom = {
  "BUILD A SCAVENGER ROOM", 0, 3, 3, 9, 4, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTempleRoom = {
  "BUILD A TEMPLE ROOM", 0, 3, 3, 10, 8, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildGraveyardRoom = {
  "BUILD A GRAVEYARD ROOM", 0, 4, 5, 11, 5, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildBarrackRoom = {
  "BUILD A BARRACK ROOM", 0, 3, 4, 12, 6, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTreasureRoom = {
  "BUILD A TREASURE ROOM", 10, 5, 5, 2, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildResearchRoom = {
  "BUILD A RESEARCH ROOM", 0, 5, 5, 3, 2, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildHatcheryRoom = {
  "BUILD A HATCHERY", 0, 6, 5, 13, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildLairRoom = {
 "BUILD A LAIR ROOM", 0, 5, 5, 14, 7, computer_check_any_room,
 computer_setup_any_room, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTrainingRoom = {
  "BUILD A TRAINING ROOM", 0, 4, 5, 6, 14, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildWorkshopRoom = {
 "BUILD A WORKSHOP ROOM", 0, 6, 6, 8, 3, computer_check_any_room,
 computer_setup_any_room, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToEntrance = {
 "DIG TO AN ENTRANCE", 0, 1700, 0, 0, 0, computer_check_dig_to_entrance,
 computer_setup_dig_to_entrance, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldForMoney = {
 "DIG TO GOLD", 0, 10999, 150, 7, 0, computer_check_dig_to_gold,
 computer_setup_dig_to_gold, computer_process_task,
 computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildTreasureRoom4x4 = {
  "BUILD A TREASURE ROOM 4x4", 10, 4, 4, 2, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess BuildLairRoom4x4 = {
  "BUILD A LAIR ROOM 4x4", 0, 4, 4, 14, 7, computer_check_any_room,
  computer_setup_any_room, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToCloseGoldForMoney = {
 "DIG TO CLOSE GOLD", 0, 30999, 500, 5, 71, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldGreedy = {
 "DIG TO GREEDY GOLD", 0, 40999, 400, 7, 900, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess DigToGoldGreedy2 = {
  "DIG TO GREEDY GOLD2", 0, 40999, 50, 7, 900, computer_check_dig_to_gold,
  computer_setup_dig_to_gold, computer_process_task,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSightOfEvil = {
  "SIGHT OF EVIL", 0, 8, 64, 1500, 0, computer_check_sight_of_evil,
  computer_setup_sight_of_evil, computer_process_sight_of_evil,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSightOfEvilScare = {
  "SIGHT OF EVIL SCARE", 0, 8, 10, 5000, 0, computer_check_sight_of_evil,
  computer_setup_sight_of_evil, computer_process_sight_of_evil,
  computer_completed_task, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerAttack1 = {
  "ATTACK PLAN 1", 0, 55, 6, 80, 0, computer_check_attack1,
  computer_setup_attack1, computer_process_task,
  computer_completed_attack1, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};

struct ComputerProcess ComputerSafeAttack = {
  "ATTACK SAFE ATTACK", 0, 25, 4, 80, 0, computer_check_safe_attack,
  computer_setup_attack1, computer_process_task,
  computer_completed_attack1, computer_paused_task,
  0, 0, 0, 0, 0, 0, 0};
*/

struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  &BuildTreasureRoom},
  {RoK_LAIR,      &BuildLairRoom},
  {RoK_GARDEN,    &BuildHatcheryRoom},
  {RoK_LIBRARY,   &BuildResearchRoom},
  {RoK_TRAINING,  &BuildTrainingRoom},
  {RoK_WORKSHOP,  &BuildWorkshopRoom},
  {RoK_SCAVENGER, &BuildScavengerRoom},
  {RoK_PRISON,    &BuildPrisonRoom},
  {RoK_TEMPLE,    &BuildTempleRoom},
  {RoK_TORTURE,   &BuildTortureRoom},
  {RoK_GRAVEYARD, &BuildGraveyardRoom},
  {RoK_BARRACKS,  &BuildBarrackRoom},
  {-1,            NULL},
};

char const event_pay_day_text[] = "EVENT PAY DAY";
char const event_save_imps_text[] = "EVENT SAVE IMPS";
char const event_check_room_text[] = "EVENT CHECK ROOMS FULL";
char const event_magic_foe_text[] = "EVENT MAGIC FOE";
char const event_check_fighters_text[] = "EVENT CHECK FIGHTERS";
char const event_fight_test_text[] = "EVENT FIGHT TEST";
char const event_fight_text[] = "EVENT FIGHT";
char const event_living_space_full_text[] = "EVENT LIVING SPACE FULL";
char const event_treasure_room_full_text[] = "EVENT TREASURE ROOM FULL";
char const event_heart_under_attack_text[] = "EVENT HEART UNDER ATTACK";
char const event_room_attack_text[] = "EVENT ROOM ATTACK";
char const event_dungeon_breach_text[] = "EVENT DUNGEON BREACH";

char const check_money_text[] = "CHECK MONEY";
char const check_expand_room_text[] = "CHECK EXPAND ROOM";
char const check_avail_trap_text[] = "CHECK AVAILIABLE TRAP";
char const check_neutral_places_text[] = "CHECK FOR NEUTRAL PLACES";
char const check_avail_door_text[] = "CHECK AVAILIABLE DOOR";
char const check_enemy_entrances_text[] = "CHECK FOR ENEMY ENTRANCES";
char const check_for_slap_imp_text[] = "CHECK FOR SLAP IMP";
char const check_for_speed_up_text[] = "CHECK FOR SPEED UP";
char const check_for_quick_attack_text[] = "CHECK FOR QUICK ATTACK";
char const check_to_pretty_text[] = "CHECK TO PRETTY";
char const check_enough_imps_text[] = "CHECK FOR ENOUGH IMPS";
char const move_creature_to_train_text[] = "MOVE CREATURE TO TRAINING";
char const move_creature_to_best_text[] = "MOVE CREATURE TO BEST ROOM";
char const computer_check_hates_text[] = "COMPUTER CHECK HATES";

long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);

long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_breach(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event);

struct ComputerProcessMnemonic computer_process_config_list[] = {
  {"Unused", NULL,},
  {"", &BuildAllRooms3x3,},
  {"", &BuildAllRooms4x4,},
  {"", &BuildPrisonRoom,},
  {"", &BuildTortureRoom,},
  {"", &BuildScavengerRoom,},
  {"", &BuildTempleRoom,},
  {"", &BuildGraveyardRoom,},
  {"", &BuildBarrackRoom,},
  {"", &BuildTreasureRoom,},
  {"", &BuildResearchRoom,},
  {"", &BuildHatcheryRoom,},
  {"", &BuildLairRoom,},
  {"", &BuildTrainingRoom,},
  {"", &BuildWorkshopRoom,},
  {"", &DigToEntrance,},
  {"", &DigToGoldForMoney,},
  {"", &BuildTreasureRoom4x4,},
  {"", &BuildLairRoom4x4,},
  {"", &DigToCloseGoldForMoney,},
  {"", &DigToGoldGreedy,},
  {"", &DigToGoldGreedy2,},
  {"", &ComputerSightOfEvil,},
  {"", &ComputerSightOfEvilScare,},
  {"", &ComputerAttack1,},
  {"", &ComputerSafeAttack,},
};

const struct NamedCommand computer_process_func_type[] = {
  {"check_build_all_rooms",   1,},
  {"setup_any_room_continue", 2,},
  {"check_any_room",          3,},
  {"setup_any_room",          4,},
  {"check_dig_to_entrance",   5,},
  {"setup_dig_to_entrance",   6,},
  {"check_dig_to_gold",       7,},
  {"setup_dig_to_gold",       8,},
  {"check_sight_of_evil",     9,},
  {"setup_sight_of_evil",    10,},
  {"process_sight_of_evil",  11,},
  {"check_attack1",          12,},
  {"setup_attack1",          13,},
  {"completed_attack1",      14,},
  {"check_safe_attack",      15,},
  {"process_task",           16,},
  {"completed_build_a_room", 17,},
  {"paused_task",            18,},
  {"completed_task",         19,},
  {"none",                   20,},
  {NULL,                      0,},
};

Comp_Process_Func computer_process_func_list[] = {
  NULL,
  computer_check_build_all_rooms,
  computer_setup_any_room_continue,
  computer_check_any_room,
  computer_setup_any_room,
  computer_check_dig_to_entrance,
  computer_setup_dig_to_entrance,
  computer_check_dig_to_gold,
  computer_setup_dig_to_gold,
  computer_check_sight_of_evil,
  computer_setup_sight_of_evil,
  computer_process_sight_of_evil,
  computer_check_attack1,
  computer_setup_attack1,
  computer_completed_attack1,
  computer_check_safe_attack,
  computer_process_task,
  computer_completed_build_a_room,
  computer_paused_task,
  computer_completed_task,
  NULL,
  NULL,
};

ComputerName computer_check_names[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheck computer_checks[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheckMnemonic computer_check_config_list[COMPUTER_CHECKS_TYPES_COUNT];

const struct NamedCommand computer_check_func_type[] = {
  {"checks_hates",            1,},
  {"check_move_to_best_room", 2,},
  {"check_move_to_room",      3,},
  {"check_no_imps",           4,},
  {"check_for_pretty",        5,},
  {"check_for_quick_attack",  6,},
  {"check_for_accelerate",    7,},
  {"check_slap_imps",         8,},
  {"check_enemy_entrances",   9,},
  {"check_for_place_door",   10,},
  {"check_neutral_places",   11,},
  {"check_for_place_trap",   12,},
  {"check_for_expand_room",  13,},
  {"check_for_money",        14,},
  {"none",                   15,},
  {NULL,                      0,},
};

Comp_Check_Func computer_check_func_list[] = {
  NULL,
  computer_checks_hates,
  computer_check_move_creatures_to_best_room,
  computer_check_move_creatures_to_room,
  computer_check_no_imps,
  computer_check_for_pretty,
  computer_check_for_quick_attack,
  computer_check_for_accelerate,
  computer_check_slap_imps,
  computer_check_enemy_entrances,
  computer_check_for_place_door,
  computer_check_neutral_places,
  computer_check_for_place_trap,
  computer_check_for_expand_room,
  computer_check_for_money,
  NULL,
  NULL,
};

ComputerName computer_event_names[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEvent computer_events[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEventMnemonic computer_event_config_list[COMPUTER_EVENTS_TYPES_COUNT];

const struct NamedCommand computer_event_test_func_type[] = {
  {"event_battle_test",       1,},
  {"event_check_fighters",    2,},
  {"event_attack_magic_foe",  3,},
  {"event_check_rooms_full",  4,},
  {"event_check_imps_danger", 5,},
  {"none",                    6,},
  {NULL,                      0,},
};

Comp_EvntTest_Func computer_event_test_func_list[] = {
  NULL,
  computer_event_battle_test,
  computer_event_check_fighters,
  computer_event_attack_magic_foe,
  computer_event_check_rooms_full,
  computer_event_check_imps_in_danger,
  NULL,
  NULL,
};

const struct NamedCommand computer_event_func_type[] = {
  {"event_battle",            1,},
  {"event_find_link",         2,},
  {"event_check_payday",      3,},
  {"none",                    4,},
  {NULL,                      0,},
};

Comp_Event_Func computer_event_func_list[] = {
  NULL,
  computer_event_battle,
  computer_event_find_link,
  computer_event_check_payday,
  NULL,
  NULL,
};

ComputerName ComputerProcessListsNames[COMPUTER_PROCESS_LISTS_COUNT];
struct ComputerProcessTypes ComputerProcessLists[COMPUTER_PROCESS_LISTS_COUNT];
/******************************************************************************/

int get_computer_process_config_list_index_prc(struct ComputerProcess *process)
{
  int i;
  const int arr_size = sizeof(computer_process_config_list)/sizeof(computer_process_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (computer_process_config_list[i].process == process)
      return i;
  }
  return 0;
}

int get_computer_process_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = sizeof(computer_process_config_list)/sizeof(computer_process_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (stricmp(computer_process_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_check_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (stricmp(computer_check_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_event_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    if (strcasecmp(computer_event_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

struct ComputerProcessTypes *get_computer_process_type_template(long cpt_idx)
{
    if ((cpt_idx < 0) || (cpt_idx >= COMPUTER_PROCESS_LISTS_COUNT))
        cpt_idx = 0;
  return &ComputerProcessLists[cpt_idx];
}

TbBool computer_type_clear_processes(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_PROCESSES_COUNT; i++)
  {
    cpt->processes[i] = NULL;
  }
  return true;
}

int computer_type_add_process(struct ComputerProcessTypes *cpt, struct ComputerProcess *process)
{
  int i;
  for (i=0; i<COMPUTER_PROCESSES_COUNT; i++)
  {
      if (cpt->processes[i] == NULL)
      {
        cpt->processes[i] = process;
        return i;
      }
  }
  return -1;
}

short computer_type_clear_checks(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_CHECKS_COUNT; i++)
  {
    LbMemorySet(&cpt->checks[i], 0, sizeof(struct ComputerCheck));
  }
  return true;
}

int computer_type_add_check(struct ComputerProcessTypes *cpt, struct ComputerCheck *check)
{
  int i;
  for (i=0; i<COMPUTER_CHECKS_COUNT; i++)
  {
      if (cpt->checks[i].name == NULL)
      {
        LbMemoryCopy(&cpt->checks[i], check, sizeof(struct ComputerCheck));
        return i;
      }
  }
  return -1;
}

short computer_type_clear_events(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_EVENTS_COUNT; i++)
  {
    LbMemorySet(&cpt->events[i], 0, sizeof(struct ComputerEvent));
  }
  return true;
}

int computer_type_add_event(struct ComputerProcessTypes *cpt, struct ComputerEvent *event)
{
  int i;
  for (i=0; i<COMPUTER_EVENTS_COUNT; i++)
  {
      if (cpt->events[i].name == NULL)
      {
        LbMemoryCopy(&cpt->events[i], event, sizeof(struct ComputerEvent));
        return i;
      }
  }
  return -1;
}

short init_computer_process_lists(void)
{
  struct ComputerProcessTypes *cpt;
  int i;
  for (i=0; i<COMPUTER_PROCESS_LISTS_COUNT; i++)
  {
    cpt = &ComputerProcessLists[i];
    LbMemorySet(cpt, 0, sizeof(struct ComputerProcessTypes));
    LbMemorySet(ComputerProcessListsNames[i], 0, LINEMSG_SIZE);
  }
  for (i=0; i<COMPUTER_PROCESS_LISTS_COUNT-1; i++)
  {
    cpt = &ComputerProcessLists[i];
    cpt->name = ComputerProcessListsNames[i];
    computer_type_clear_processes(cpt);
    computer_type_clear_checks(cpt);
  }
  return true;
}

short parse_computer_player_common_blocks(char *buf,long len)
{
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
    return 0;
  }
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // COMPUTERASSISTS
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 2: // PROCESSESCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 3: // CHECKSCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 4: // EVENTSCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 5: // COMPUTERSCOUNT
//TODO make it work when AI structures from DLL will no longer be used
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
  return 1;
}

short parse_computer_player_process_blocks(char *buf,long len)
{
  struct ComputerProcess *process;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variable
  char block_buf[32];
  char word_buf[32];
  const int arr_size = sizeof(computer_process_config_list)/sizeof(computer_process_config_list[0]);
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"process%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    process = computer_process_config_list[i].process;
    process->parent = NULL;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_process_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          //For now, let's leave default names.
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_10 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_14 = k;
            n++;
          }
          if (n < 5)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_check = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_setup = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_task = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_complete = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_pause = computer_process_func_list[k];
              n++;
          }
          if (n < 5)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_30 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_34 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_38 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_3C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_40 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_44 = k;
            n++;
          }
          if (n < 6)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 5: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_process_config_list[i].name,sizeof(computer_process_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short parse_computer_player_check_blocks(char *buf,long len)
{
  struct ComputerCheck *check;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  char word_buf[32];
  // Initialize the checks array
  const int arr_size = sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]);
  for (i=0; i < arr_size; i++)
  {
    check = &computer_checks[i];
    computer_check_config_list[i].name[0] = '\0';
    computer_check_config_list[i].check = check;
    check->name = computer_check_names[i];
    LbMemorySet(computer_check_names[i], 0, LINEMSG_SIZE);
  }
  strcpy(computer_check_names[0],"INCORRECT CHECK");
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"check%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    check = computer_check_config_list[i].check;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_check_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,check->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->flags = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->turns_interval = k;
            n++;
          }
          if (n < 2)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_check_func_type);
          if (k > 0)
          {
              check->func = computer_check_func_list[k];
              n++;
          }
          if (n < 1)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param1 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param2 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param3 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->turns_last = k;
            n++;
          }
          if (n < 4)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 5: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_check_config_list[i].name,sizeof(computer_check_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short parse_computer_player_event_blocks(char *buf,long len)
{
  struct ComputerEvent *cevent;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  char word_buf[32];
  // Initialize the events array
  const int arr_size = sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]);
  for (i=0; i < arr_size; i++)
  {
    cevent = &computer_events[i];
    computer_event_config_list[i].name[0] = '\0';
    computer_event_config_list[i].event = cevent;
    cevent->name = computer_event_names[i];
    LbMemorySet(computer_event_names[i], 0, LINEMSG_SIZE);
  }
  strcpy(computer_event_names[0],"INCORRECT EVENT");
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"event%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    cevent = computer_event_config_list[i].event;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_event_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,cevent->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->cetype = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->test_interval = k;
            n++;
          }
          if (n < 3)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_event_func_type);
          if (k > 0)
          {
              cevent->func_event = computer_event_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_event_test_func_type);
          if (k > 0)
          {
              cevent->func_test = computer_event_test_func_list[k];
              n++;
          }
          if (n < 2)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PROCESS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_process_config_list_index_mnem(word_buf);
            if (k > 0)
            {
              cevent->process = computer_process_config_list[k].process;
            } else
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","PROCESS",word_buf);
            }
          }
          break;
      case 5: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->param1 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->param2 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->param3 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->last_test_gameturn = k;
            n++;
          }
          if (n < 4)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 6: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_event_config_list[i].name,sizeof(computer_event_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short write_computer_player_check_to_log(struct ComputerCheck *check)
{
  JUSTMSG("[checkXX]");
  JUSTMSG("Name = %s",check->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %d %d",check->flags,check->turns_interval);
  JUSTMSG("Functions = %x",check->func);
  JUSTMSG("Params = %d %d %d %d",check->param1,check->param2,check->param3,check->turns_last);
  return true;
}

short write_computer_player_event_to_log(struct ComputerEvent *event)
{
  JUSTMSG("[eventXX]");
  JUSTMSG("Name = %s",event->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %d %d %d",event->cetype,event->field_8,event->test_interval);
  JUSTMSG("Functions = %x %x",event->func_event,event->func_test);
  JUSTMSG("Params = %d %d %d %d",event->param1,event->param2,event->param3,event->last_test_gameturn);
  return true;
}

short parse_computer_player_computer_blocks(char *buf,long len)
{
  struct ComputerProcessTypes *cpt;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variable
  char block_buf[32];
  char word_buf[32];
  const int arr_size = sizeof(ComputerProcessLists)/sizeof(ComputerProcessLists[0])-1;
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"computer%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    cpt = &ComputerProcessLists[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_computer_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,cpt->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_10 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_14 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_18 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_1C = k;
            n++;
          }
          if (n < 7)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // PROCESSES
          computer_type_clear_processes(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_process_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","PROCESSES",word_buf);
              continue;
            }
            n = computer_type_add_process(cpt, computer_process_config_list[k].process);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","PROCESSES",word_buf);
          }
          break;
      case 4: // CHECKS
          computer_type_clear_checks(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_check_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","CHECKS",word_buf);
              continue;
            }
            n = computer_type_add_check(cpt, computer_check_config_list[k].check);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","CHECKS",word_buf);
          }
          break;
      case 5: // EVENTS
          computer_type_clear_events(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_event_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","EVENTS",word_buf);
              continue;
            }
            n = computer_type_add_event(cpt, computer_event_config_list[k].event);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","EVENTS",word_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short load_computer_player_config(void)
{
  const char *fname;
  char *buf;
  long len;
  init_computer_process_lists();
  // Load the config file
  fname = prepare_file_path(FGrp_FxData,keeper_compplayer_file);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    ERRORLOG("Computer Player file \"%s\" doesn't exist or is too small.",keeper_compplayer_file);
    return false;
  }
  if (len > 65536)
  {
    ERRORLOG("Computer Player file \"%s\" is too large.",keeper_compplayer_file);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    parse_computer_player_common_blocks(buf,len);
    parse_computer_player_process_blocks(buf,len);
    parse_computer_player_check_blocks(buf,len);
    parse_computer_player_event_blocks(buf,len);
    parse_computer_player_computer_blocks(buf,len);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  // Hack to synchronize local structure with the one inside DLL.
  // Remove when it's not needed anymore.
  LbMemoryCopy(_DK_ComputerProcessLists,ComputerProcessLists,13*sizeof(struct ComputerProcessTypes));
  return 1;
}

void shut_down_process(struct Computer2 *comp, struct ComputerProcess *process)
{
  Comp_Process_Func callback;
  if (process != NULL)
  {
    set_flag_dword(&process->field_44, 0x0008, true);
    set_flag_dword(&process->field_44, 0x0020, false);
    process->field_34 = game.play_gameturn;
    callback = process->func_complete;
    if (callback != NULL)
      callback(comp, process);
  }
}

long computer_process_index(const struct Computer2 *comp, const struct ComputerProcess *process)
{
    long i;
    i = ((char *)process - (char *)&comp->processes[0]);
    if ( (i < 0) || (i > COMPUTER_PROCESSES_COUNT*sizeof(struct ComputerProcess)) )
        return 0;
    return i / sizeof(struct ComputerProcess);
}

void suspend_process(struct Computer2 *comp, struct ComputerProcess *process)
{
    if (process != NULL)
    {
        process->field_44 &= ~0x20;
        process->field_38 = 0;
        process->field_3C = game.play_gameturn;
        process->field_34 = game.play_gameturn;
    }
}

void reset_process(struct Computer2 *comp, struct ComputerProcess *process)
{
  if (process != NULL)
  {
    process->field_3C = 0;
    process->field_38 = 0;
    set_flag_dword(&process->field_44, 0x0020, false);
    process->field_34 = game.play_gameturn;
  }
}

long get_computer_money_less_cost(struct Computer2 *comp)
{
  return _DK_get_computer_money_less_cost(comp);
}

long count_creatures_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4)
{
    //TODO COMPUTER_EVENT_BREACH needs this function; may be also used somewhere else - not sure
    return 0;
}

struct ComputerTask *computer_setup_build_room(struct Computer2 *comp, unsigned short rkind, long a3, long a4, long a5)
{
  return _DK_computer_setup_build_room(comp, rkind, a3, a4, a5);
}

long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process)
{
  //return _DK_computer_setup_any_room(comp, process);
  struct ComputerTask *task;
  long i;
  task = computer_setup_build_room(comp, process->field_10, process->field_8, process->field_C, process->field_14);
  if (task != NULL)
  {
    set_flag_dword(&process->field_44, 0x0020, true);
    i = (long)((char *)process - (char *)&comp->processes[0]) / sizeof(struct ComputerProcess);
    if ((i < 0) || (i > COMPUTER_PROCESSES_COUNT))
    {
      ERRORLOG("Process \"%s\" is outside of Computer Player.",process->name);
      i = COMPUTER_PROCESSES_COUNT;
    }
    task->field_8C = i;
    shut_down_process(comp, process);
    return 2;
  }
  if (process->field_8 > process->field_C)
  {
    if (process->field_8 <= 2)
      return 0;
    process->field_8--;
  } else
  {
    if (process->field_C <= 2)
      return 0;
    process->field_C--;
  }
  reset_process(comp, process);
  return 2;
}

long computer_setup_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process)
{
    return _DK_computer_setup_dig_to_entrance(comp, process);
}

void setup_dig_to(struct ComputerDig *cdig, const struct Coord3d startpos, const struct Coord3d endpos)
{
    memset(cdig,0,sizeof(struct ComputerDig));
    cdig->pos_gold.x.val = startpos.x.val;
    cdig->pos_gold.y.val = startpos.y.val;
    cdig->pos_gold.z.val = startpos.z.val;
    cdig->pos_E.x.val = startpos.x.val;
    cdig->pos_E.y.val = startpos.y.val;
    cdig->pos_E.z.val = startpos.z.val;
    cdig->pos_14.x.val = endpos.x.val;
    cdig->pos_14.y.val = endpos.y.val;
    cdig->pos_14.z.val = endpos.z.val;
    cdig->distance = LONG_MAX;
    cdig->field_2C = 1;
    cdig->pos_20.x.val = 0;
    cdig->pos_20.y.val = 0;
    cdig->pos_20.z.val = 0;
    cdig->field_54 = 0;
}

long computer_finds_nearest_room_to_gold_lookup(const struct Dungeon *dungeon, const struct GoldLookup *gldlook, struct Room **nearroom)
{
    struct Room *room;
    long rkind;
    long distance,min_distance;
    struct Coord3d gold_pos;
    *nearroom = INVALID_ROOM;
    gold_pos.x.val = 0;
    gold_pos.y.val = 0;
    gold_pos.z.val = 0;
    gold_pos.x.stl.num = gldlook->x_stl_num;
    gold_pos.y.stl.num = gldlook->y_stl_num;
    min_distance = LONG_MAX;
    distance = LONG_MAX;
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        room = find_room_nearest_to_position(dungeon->owner, rkind, &gold_pos, &distance);
        if (!room_is_invalid(room))
        {
            distance >>= 8; // Convert to subtiles
            // Decrease the value by gold area radius
            distance -= (gldlook->field_E >> 3);
            // We can accept longer distances if digging directly to treasure room
            if (room->kind == RoK_TREASURE)
                distance -= TREASURE_ROOM_PREFERENCE_WHILE_DIGGING_GOLD;
            if (min_distance > distance)
            {
                *nearroom = room;
                min_distance = distance;
            }
        }
    }
    return min_distance;
}

long computer_finds_nearest_task_to_gold(const struct Computer2 *comp, const struct GoldLookup *gldlook, struct ComputerTask ** near_task)
{
    struct Coord3d task_pos;
    long i;
    unsigned long k;
    struct ComputerTask *ctask;
    long distance,min_distance;
    long delta_x,delta_y;
    task_pos.x.val = 0;
    task_pos.y.val = 0;
    task_pos.z.val = 0;
    task_pos.x.stl.num = gldlook->x_stl_num;
    task_pos.y.stl.num = gldlook->y_stl_num;
    min_distance = LONG_MAX;
    i = comp->task_idx;
    k = 0;
    while (i != 0)
    {
        ctask = get_computer_task(i);
        if (computer_task_invalid(ctask))
        {
            ERRORLOG("Jump to invalid task detected");
            break;
        }
        i = ctask->next_task;
        // Per-task code
        if ( ((ctask->flags & 0x01) != 0) && ((ctask->flags & 0x02) != 0) )
        {
            delta_x = (long)ctask->pos_64.x.val - (long)task_pos.x.val;
            delta_y = (long)ctask->pos_64.y.val - (long)task_pos.y.val;
            distance = LbDiagonalLength(abs(delta_x), abs(delta_y));
            distance >>= 8; // Convert to subtiles
            distance -= (gldlook->field_E >> 3);
            if (min_distance > distance)
            {
                *near_task = ctask;
                min_distance = distance;
            }
        }
        // Per-task code ends
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping tasks list");
            break;
        }
    }
    return min_distance;
}

/**
 * Finds nearest place to start digging gold from, and the target GoldLookup to be digged.
 *
 * @param comp Computer player which considers starting the digging.
 * @param pos Resurns position to start digging from.
 * @param gldlookref Returns reference to GoldLookup containing coords of the place to dig to.
 * @return Lower or equal 0 on failure, positive if gold digging is ready to go.
 */
long computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlookref)
{
    struct Dungeon *dungeon;
    struct GoldLookup *gldlook;
    struct GoldLookup *gldlooksel;
    struct Coord3d locpos;
    struct Coord3d *spos;
    long dig_distance;
    long lookups_checked;
    long i;
    //return _DK_computer_finds_nearest_room_to_gold(comp, pos, gldlookref);
    dungeon = comp->dungeon;
    gldlooksel = NULL;
    *gldlookref = gldlooksel;
    locpos.x.val = 0;
    locpos.y.val = 0;
    locpos.z.val = 0;
    spos = &locpos;
    lookups_checked = 0;
    dig_distance = LONG_MAX;
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
        gldlook = &game.gold_lookup[i];
        if ((gldlook->field_0 & 0x01) == 0)
            continue;
        if ((gldlook->plyrfield_1[dungeon->owner] & 0x03) != 0)
            continue;
        SYNCDBG(8,"Searching for place to reach (%d,%d)",(int)gldlook->x_stl_num,(int)gldlook->y_stl_num);
        lookups_checked++;
        struct Room *room = INVALID_ROOM;
        long new_dist;
        new_dist = computer_finds_nearest_room_to_gold_lookup(dungeon, gldlook, &room);
        if (dig_distance > new_dist)
        {
            locpos.x.val = (room->central_stl_x << 8);
            locpos.y.val = (room->central_stl_y << 8);
            locpos.z.val = (1 << 8);
            spos = &locpos;
            dig_distance = new_dist;
            gldlooksel = gldlook;
            SYNCDBG(8,"Distance from room at (%d,%d) is %d",(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
        }
        struct ComputerTask *ctask = NULL;
        new_dist = computer_finds_nearest_task_to_gold(comp, gldlook, &ctask);
        if (dig_distance > new_dist)
        {
            spos = &ctask->pos_64;
            dig_distance = new_dist;
            gldlooksel = gldlook;
            SYNCDBG(8,"Distance from task at (%d,%d) is %d",(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
        }
    }
    if (gldlooksel == NULL)
    {
        SYNCDBG(8,"Checked %d lookups, but no gold to dig found",(int)lookups_checked);
        if (lookups_checked == 0)
        {
            return -1;
        } else
        {
            return 0;
        }
    }
    SYNCDBG(8,"Best digging start to reach (%d,%d) is on subtile (%d,%d); distance is %d",(int)gldlooksel->x_stl_num,(int)gldlooksel->y_stl_num,(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
    *gldlookref = gldlooksel;
    pos->x.val = spos->x.val;
    pos->y.val = spos->y.val;
    pos->z.val = spos->z.val;
    if (dig_distance < 1)
        dig_distance = 1;
    if (dig_distance > LONG_MAX)
        dig_distance = LONG_MAX;
    return dig_distance;
}

long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process)
{
    struct GoldLookup *gldlook;
    struct Dungeon *dungeon;
    struct ComputerTask *ctask;
    struct Coord3d startpos;
    struct Coord3d endpos;
    unsigned long dig_distance;
    unsigned long max_distance;
    struct Coord3d * posptr;
    long digres;
    SYNCDBG(18,"Starting");
    //return _DK_computer_setup_dig_to_gold(comp, process);
    dig_distance = 0;
    dungeon = comp->dungeon;
    gldlook = 0;
    digres = computer_finds_nearest_room_to_gold(comp, &startpos, &gldlook);
    if (digres == -1)
    {
        process->field_44 |= 0x04;
        SYNCDBG(8,"Can't find nearest room to gold; will refresh gold map");
        return 0;
    }
    if (digres <= 0)
    {
        SYNCDBG(8,"Finding gold to dig didn't worked out");
        return 0;
    }
    max_distance = game.play_gameturn / process->field_C + process->field_14;
    if (digres > max_distance)
    {
        SYNCDBG(8,"Gold is out of distance (%lu > %lu)",digres,max_distance);
        return 4;
    }
    endpos.x.val = 0;
    endpos.y.val = 0;
    endpos.z.val = 0;
    endpos.x.stl.num = 3 * (gldlook->x_stl_num / 3);
    endpos.y.stl.num = 3 * (gldlook->y_stl_num / 3);
    startpos.x.stl.pos = 0;
    startpos.y.stl.pos = 0;
    endpos.z.val = 0;
    startpos.x.stl.num = 3 * (startpos.x.stl.num / 3);
    startpos.y.stl.num = 3 * (startpos.y.stl.num / 3);
    if ( comp->field_20 )
    {
        struct ComputerDig cdig;
        // Setup the digging on dummy ComputerDig, just to compute distance
        setup_dig_to(&cdig, startpos, endpos);
        while ( 1 )
        {
            digres = tool_dig_to_pos2(comp, &cdig, true, 1);
            if (digres != 0)
              break;
            dig_distance++;
        }
        if ( (digres != -1) && (digres != -5) )
        {
            SYNCDBG(8,"Dig evaluation didn't worked out, code %d",digres);
            gldlook->plyrfield_1[dungeon->owner] |= 0x02;
            return 0;
        }
        if (dig_distance > max_distance)
        {
            SYNCDBG(8,"Gold is out of evaluation distance (%lu > %lu)",digres,max_distance);
            return 0;
        }
        SYNCDBG(8,"Dig evaluation distance %d, result %d",dig_distance,digres);
    }
    ctask = get_free_task(comp, 0);
    if (ctask == NULL)
    {
        SYNCDBG(8,"No free task; won't dig");
        return 4;
    }
    posptr = &ctask->pos_70;
    posptr->x.val = startpos.x.val;
    posptr->y.val = startpos.y.val;
    posptr->z.val = startpos.z.val;
    ctask->ttype = CTT_DigToGold;
    ctask->pos_76.x.val = endpos.x.val;
    ctask->pos_76.y.val = endpos.y.val;
    ctask->pos_76.z.val = endpos.z.val;
    ctask->long_86 = process->field_10;
    ctask->flags |= 0x04;
    posptr->x.stl.num = 3 * (posptr->x.stl.num / 3);
    posptr->y.stl.num = 3 * (posptr->y.stl.num / 3);
    ctask->field_8C = computer_process_index(comp, process);
    ctask->word_80 = gold_lookup_index(gldlook);
    gldlook->plyrfield_1[dungeon->owner] |= 0x01;
    // Setup the digging
    endpos.x.val = ctask->pos_76.x.val;
    endpos.y.val = ctask->pos_76.y.val;
    endpos.z.val = ctask->pos_76.z.val;
    startpos.x.val = posptr->x.val;
    startpos.y.val = posptr->y.val;
    startpos.z.val = posptr->z.val;
    setup_dig_to(&ctask->dig, startpos, endpos);
    process->func_complete(comp, process);
    suspend_process(comp, process);
    comp->field_0 = 2;
    return 2;
}

long computer_setup_any_room_continue(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_any_room_continue(comp, process);
}

long computer_setup_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_sight_of_evil(comp, process);
}

long computer_setup_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_setup_attack1(comp, process);
}

long computer_check_build_all_rooms(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_build_all_rooms(comp, process);
}

long computer_check_any_room(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_any_room(comp, process);
}

long computer_check_dig_to_entrance(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_dig_to_entrance(comp, process);
}

/**
 * Check for gold digging.
 * This function address is compared in computer_check_for_money(); but it is already rewritten.
 */
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process)
{
    //return _DK_computer_check_dig_to_gold(comp, process);
    // If we have treasure room
    if ( !dungeon_has_room(comp->dungeon, RoK_TREASURE) )
    {
        SYNCDBG(18,"Computer player model %d won't dig for gold - no treasure room.",(int)comp->model);
        return 4;
    }
    // And we're lacking money
    if ( process->field_8 <= get_computer_money_less_cost(comp) )
    {
        SYNCDBG(18,"Computer player model %d won't dig for gold - has over %d gold.",(int)comp->model,(int)process->field_8);
        return 4;
    }
    // And we're not already digging for gold
    if ( get_task_in_progress(comp, CTT_DigToGold) )
    {
        SYNCDBG(18,"Computer player model %d is already digging for gold.",(int)comp->model);
        return 4;
    }
    // Then do dig for gold
    SYNCDBG(8,"Computer player model %d is going to start digging for gold.",(int)comp->model);
    return 1;
}

long computer_check_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_sight_of_evil(comp, process);
}

long computer_check_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_attack1(comp, process);
}

long computer_check_safe_attack(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_check_safe_attack(comp, process);
}

long computer_process_sight_of_evil(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_process_sight_of_evil(comp, process);
}

long computer_process_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  return 0;
}

long computer_paused_task(struct Computer2 *comp, struct ComputerProcess *process)
{
  comp->field_0 = 2;
  return 0;
}

long computer_completed_task(struct Computer2 *comp, struct ComputerProcess *process)
{
    SYNCDBG(8,"Completed process \"%s\"",process->name);
    process->field_34 = game.play_gameturn;
    comp->field_0 = 2;
    return 0;
}

long computer_completed_attack1(struct Computer2 *comp, struct ComputerProcess *process)
{
  return _DK_computer_completed_attack1(comp, process);
}

long computer_completed_build_a_room(struct Computer2 *comp, struct ComputerProcess *process)
{
  process->field_44 &= 0xFFFFFFF7u;
  comp->field_0 = 2;
  return 0;
}

long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_checks_hates(comp, check);
}

long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_move_creatures_to_best_room(comp, check);
}

long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_move_creatures_to_room(comp, check);
}

long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_no_imps(comp, check);
}

long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_pretty(comp, check);
}

long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_quick_attack(comp, check);
}

struct Thing *computer_check_creatures_in_room_for_accelerate(struct Computer2 *comp, struct Room *room)
{
    struct Dungeon *dungeon;
    struct StateInfo *stati;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    long i,n;
    dungeon = comp->dungeon;
    i = room->creatures_list;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      cctrl = creature_control_get_from_thing(thing);
      if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
      {
        ERRORLOG("Jump to invalid creature %ld detected",i);
        break;
      }
      i = cctrl->next_in_room;
      // Per creature code
      if (!thing_affected_by_spell(thing, 11))
      {
          if (thing->active_state == CrSt_MoveToPosition)
              n = thing->continue_state;
          else
              n = thing->active_state;
          stati = get_thing_state_info_num(n);
          if (stati->state_type == 1)
          {
              if (try_game_action(comp, dungeon->owner, GA_Unk21, 8, 0, 0, thing->index, 0) > 0)
              {
                  return thing;
              }
          }
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(struct Computer2 *comp, RoomKind rkind)
{
    struct Dungeon *dungeon;
    struct Room *room;
    struct Thing *thing;
    long i;
    unsigned long k;
    if ((rkind < 1) || (rkind > ROOM_TYPES_COUNT))
    {
        ERRORLOG("Invalid room kind %d",(int)rkind);
        return INVALID_THING;
    }
    dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return INVALID_THING;
    }
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
      thing = computer_check_creatures_in_room_for_accelerate(comp, room);
      if (!thing_is_invalid(thing))
          return thing;
      // Per-room code ends
      k++;
      if (k > ROOMS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping rooms list");
        break;
      }
    }
    return INVALID_THING;
}

long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check)
{
    static RoomKind workers_in_rooms[] = {RoK_LIBRARY,RoK_LIBRARY,RoK_WORKSHOP,RoK_TRAINING,RoK_SCAVENGER};
    struct Thing *thing;
    long i,n;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_accelerate(comp, check);
    if (computer_able_to_use_magic(comp, 11, 8, 3) != 1)
    {
        return 4;
    }
    n = check->param2 % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    if (n <= 0)
        n = ACTION_RANDOM(sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    for (i=0; i < sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]); i++)
    {
        thing = computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(comp, workers_in_rooms[n]);
        if (!thing_is_invalid(thing))
        {
            SYNCDBG(8,"Cast on thing %d",(int)thing->index);
            return 1;
        }
        n = (n+1) % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    }
    return 4;
}

long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_slap_imps(comp, check);
}

long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_enemy_entrances(comp, check);
}

long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_place_door(comp, check);
}

long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_neutral_places(comp, check);
}

long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_place_trap(comp, check);
}

long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    return _DK_computer_check_for_expand_room(comp, check);
}

long computer_pick_trainig_or_scavenging_creatures_and_place_on_room(struct Computer2 *comp, struct Room *room, long thing_idx, long tasks_limit)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long new_tasks;
    unsigned long k;
    long i;
    new_tasks = 0;
    // Sweep through creatures list
    i = thing_idx;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
        ERRORLOG("Jump to invalid thing detected");
        break;
      }
      cctrl = creature_control_get_from_thing(thing);
      i = cctrl->players_next_creature_idx;
      // Per creature code
      if (creature_is_training(thing) || creature_is_scavengering(thing)) // originally, only CrSt_Training and CrSt_Scavengering were accepted
      {
        if (!create_task_move_creature_to_pos(comp, thing, room->central_stl_x, room->central_stl_y))
          break;
        new_tasks++;
        if (new_tasks >= tasks_limit)
          break;
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return new_tasks;
}

/** Picks creatures and workers from given dungeon who are doing expensive jobs, then places them on lair.
 *
 * @param comp Computer player who controls the target dungeon.
 * @param tasks_limit Max amount of computer tasks to create.
 * @return Amount of new computer tasks created.
 */
long computer_pick_expensive_job_creatures_and_place_on_lair(struct Computer2 *comp, long tasks_limit)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long new_tasks;
    dungeon = comp->dungeon;
    room = room_get(dungeon->room_kind[RoK_LAIR]);
    new_tasks = 0;
    // If we don't have lair, then don't even bother
    if (room_is_invalid(room))
      return new_tasks;
    // Sweep through creatures list
    new_tasks += computer_pick_trainig_or_scavenging_creatures_and_place_on_room(comp, room, dungeon->creatr_list_start, tasks_limit);
    if (new_tasks >= tasks_limit)
        return new_tasks;
    // Sweep through workers list
    new_tasks += computer_pick_trainig_or_scavenging_creatures_and_place_on_room(comp, room, dungeon->digger_list_start, tasks_limit-new_tasks);
    return new_tasks;
}

long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check)
{
    long money;
    struct ComputerProcess *cproc;
    struct Dungeon *dungeon;
    long ret;
    long i;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_money(comp, check);
    ret = 4;
    // Try creating digging for gold process
    money = get_computer_money_less_cost(comp);
    if ((check->param3 > money) || (check->param2 > money))
    {
      for (i=0; i <= COMPUTER_PROCESSES_COUNT; i++)
      {
          cproc = &comp->processes[i];
          if ((cproc->field_44 & 0x02) != 0)
              break;
          if (cproc->func_check == computer_check_dig_to_gold)
          {
            cproc->field_4++;
            if (game.play_gameturn - cproc->field_3C > 20)
              cproc->field_3C = 0;
          }
      }
    }

    // Try selling traps and doors
    dungeon = comp->dungeon;
    if (dungeon->field_14B8 > dungeon->total_money_owned)
    {
      if (dungeon->room_kind[RoK_WORKSHOP] != 0)
      {
        if (get_task_in_progress(comp, CTT_SellTrapsAndDoors) == NULL)
        {
          if (create_task_sell_traps_and_doors(comp, 3*dungeon->field_14B8/2))
          {
            ret = 1;
          }
        }
      }
    }
    if (3*dungeon->field_14B8/2 <= dungeon->total_money_owned)
      return ret;

    // Move creatures away from rooms which costs a lot
    if (computer_pick_expensive_job_creatures_and_place_on_lair(comp, 3) > 0)
        ret = 1;
    return ret;
}

long count_creatures_for_defend_pickup(struct Computer2 *comp)
{
    return _DK_count_creatures_for_defend_pickup(comp);
}

long computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos)
{
    return _DK_computer_find_non_solid_block(comp, pos);
}

long computer_able_to_use_magic(struct Computer2 *comp, long a2, long a3, long a4)
{
    return _DK_computer_able_to_use_magic(comp, a2, a3, a4);
}

long check_call_to_arms(struct Computer2 *comp)
{
    return _DK_check_call_to_arms(comp);
}

long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
    long creatrs_def, creatrs_num;
    struct Coord3d pos;
    //return _DK_computer_event_battle(comp, cevent, event);
    pos.x.stl.num = event->mappos_x;
    pos.x.stl.pos = 0;
    pos.y.stl.num = event->mappos_y;
    pos.y.stl.pos = 0;
    pos.z.val = 0;
    if ((pos.x.val <= 0) || (pos.y.val <= 0))
        return false;
    creatrs_def = count_creatures_for_defend_pickup(comp);
    creatrs_num = creatrs_def * (long)cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0))
        creatrs_num = 1;
    if (creatrs_num <= 0)
        return false;
    if (!computer_find_non_solid_block(comp, &pos))
        return false;
    if (!get_task_in_progress(comp, CTT_MoveCreaturesToDefend) || ((cevent->param2 & 0x02) != 0))
    {
        return create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2);
    } else
    if (computer_able_to_use_magic(comp, 6, 1, 1) == 1)
    {
        if (!get_task_in_progress(comp, CTT_MagicCallToArms) || ((cevent->param2 & 0x02) != 0))
        {
            if ( check_call_to_arms(comp) )
            {
                return create_task_magic_call_to_arms(comp, &pos, creatrs_num);
            }
        }
    }
    return false;
}

long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
  return _DK_computer_event_find_link(comp, cevent, event);
}

long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_battle_test(comp, cevent);
}

long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_fighters(comp, cevent);
}

long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_attack_magic_foe(comp, cevent);
}

long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_rooms_full(comp, cevent);
}

long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent)
{
  return _DK_computer_event_check_imps_in_danger(comp, cevent);
}

long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
  return _DK_computer_event_check_payday(comp, cevent, event);
}

long computer_event_breach(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event)
{
    //TODO COMPUTER_EVENT_BREACH is remade from beta; make it work (if it's really needed)
    struct ComputerTask *ctask;
    struct Coord3d pos;
    long i,count;

    //TODO COMPUTER_EVENT_BREACH check why mappos_x and mappos_y isn't used normally
    pos.x.val = ((event->mappos_x & 0xFF) << 8);
    pos.y.val = (((event->mappos_x >> 8) & 0xFF) << 8);
    if ((pos.x.val <= 0) || (pos.y.val <= 0))
    {
        return 0;
    }
    count = count_creatures_for_pickup(comp, &pos, 0, cevent->param2);
    i = count * cevent->param1 / 100;
    if ((i <= 0) && (count > 0))
    {
        i = 1;
    }
    if (i <= 0)
    {
        return 4;
    }
    if (!computer_find_non_solid_block(comp, &pos))
    {
        return 4;
    }
    ctask = get_free_task(comp, 1);
    if (computer_task_invalid(ctask))
    {
        return 4;
    }
    ctask->ttype = CTT_MoveCreaturesToDefend;
    ctask->pos_76.x.val = pos.x.val;
    ctask->pos_76.y.val = pos.y.val;
    ctask->pos_76.z.val = pos.z.val;
    ctask->field_7C = i;
    ctask->field_70 = cevent->param2;
    ctask->field_A = game.play_gameturn;
    ctask->field_5C = game.play_gameturn;
    ctask->field_60 = comp->field_34;
    return 1;
}

void setup_a_computer_player(unsigned short plyridx, long comp_model)
{
  struct ComputerProcessTypes *cproctype;
  struct ComputerProcess *process;
  struct ComputerProcess *newproc;
  struct ComputerCheck *check;
  struct ComputerCheck *newchk;
  struct ComputerEvent *event;
  struct ComputerEvent *newevnt;
  struct Comp2_UnkStr1 *unkptr;
  struct Computer2 *comp;
  long i;
  //_DK_setup_a_computer_player(plyridx, comp_model); return;
  comp = &game.computer[plyridx];
  LbMemorySet(comp, 0, sizeof(struct Computer2));
  cproctype = &ComputerProcessLists[comp_model];
  comp->dungeon = get_players_num_dungeon(plyridx);
  comp->model = comp_model;
  comp->field_18 = cproctype->field_C;
  comp->field_14 = cproctype->field_8;
  comp->field_30 = cproctype->field_10;
  comp->field_2C = cproctype->field_14;
  comp->field_20 = cproctype->field_18;
  comp->field_C = 1;
  comp->field_0 = 2;

  for (i=0; i < 5; i++)
  {
    unkptr = &comp->unkarr_A10[i];
    if (i == plyridx)
      unkptr->field_6 = 0x80000000;
    else
      unkptr->field_6 = 0;
  }
  comp->field_1C = cproctype->field_4;

  for (i=0; i < COMPUTER_PROCESSES_COUNT; i++)
  {
    process = cproctype->processes[i];
    newproc = &comp->processes[i];
    if ((process == NULL) || (process->name == NULL))
    {
      newproc->name = NULL;
      break;
    }
    // Modifying original ComputerProcessTypes structure - I don't like it!
    if (process->func_setup == computer_setup_any_room)
    {
      if (process->field_14 >= 0)
        process->field_14 = get_room_look_through(process->field_14);
    }
    LbMemoryCopy(newproc, process, sizeof(struct ComputerProcess));
    newproc->parent = process;
  }
  newproc = &comp->processes[i];
  newproc->field_44 |= 0x02;

  for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
  {
    check = &cproctype->checks[i];
    newchk = &comp->checks[i];
    if ((check == NULL) || (check->name == NULL))
    {
      newchk->name = NULL;
      break;
    }
    LbMemoryCopy(newchk, check, sizeof(struct ComputerCheck));
  }
  // Note that we don't have special, empty check at end of array
  // The check with 0x02 flag identifies end of active checks
  // (the check with 0x02 flag is invalid - only previous checks are in use)
  //newchk = &comp->checks[i];
  newchk->flags |= 0x02;

  for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
  {
    event = &cproctype->events[i];
    newevnt = &comp->events[i];
    if ((event == NULL) || (event->name == NULL))
    {
      newevnt->name = NULL;
      break;
    }
    LbMemoryCopy(newevnt, event, sizeof(struct ComputerEvent));
  }
}

struct ComputerProcess * find_best_process(struct Computer2 *comp)
{
    return _DK_find_best_process(comp);
}

long set_next_process(struct Computer2 *comp)
{
    struct ComputerProcess *process;
    long chkres;
    //return _DK_set_next_process(comp);
    chkres = 0;
    process = find_best_process(comp);
    if (process != NULL)
    {
        SYNCDBG(8,"Checking \"%s\"",process->name);
        chkres = process->func_check(comp, process);
        if (chkres == 1)
        {
            comp->ongoing_process = computer_process_index(comp, process); // This should give index of the process
            SYNCDBG(8,"Setting up process %d",(int)comp->ongoing_process);
            chkres = process->func_setup(comp, process);
            if ( chkres == 1 )
            {
                process->field_30 = game.play_gameturn;
                comp->field_0 = 3;
            }
        }
        if (chkres == 4)
        {
            process->field_3C = game.play_gameturn;
            process->field_38 = 0;
        }
        if (chkres == 0)
        {
            process->field_3C = 0;
            process->field_38 = game.play_gameturn;
        }
    }
    if (chkres != 1)
    {
        SYNCDBG(17,"No new process");
        comp->ongoing_process = 0;
    } else
    {
        SYNCDBG(7,"Undertaking new process");
    }
    return chkres;
}

void computer_check_events(struct Computer2 *comp)
{
    struct Dungeon * dungeon;
    struct ComputerEvent * cevent;
    struct Event * event;
    long i,n;
    SYNCDBG(17,"Starting");
    //_DK_computer_check_events(comp);
    dungeon = comp->dungeon;
    for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        cevent = &comp->events[i];
        if (cevent->name == NULL)
            break;
        switch (cevent->cetype)
        {
        case 0:
            for (n=0; n < EVENTS_COUNT; n++)
            {
                event = &game.event[n];
                if ( ((event->field_0 & 0x01) != 0) &&
                      (event->owner == dungeon->owner) &&
                      (event->kind == cevent->field_8) )
                {
                    if (cevent->func_event(comp, cevent, event) == 1)
                        cevent->last_test_gameturn = game.play_gameturn;
                }
            }
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            if ((cevent->last_test_gameturn + cevent->test_interval) <= (long)game.play_gameturn)
            {
                if (cevent->func_test(comp,cevent) == 1)
                    ; // nothing done with this "if" - hmm... could be intentional, or not.
                cevent->last_test_gameturn = game.play_gameturn;
            }
            break;
        default:
            ERRORLOG("Unhandled Computer Event Type %d",(int)cevent->cetype);
            break;
        }
    }
}

TbBool process_checks(struct Computer2 *comp)
{
    struct ComputerCheck *ccheck;
    long delta;
    long i;
    SYNCDBG(17,"Starting");
    //return _DK_process_checks(comp);
    for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        ccheck = &comp->checks[i];
        if (comp->tasks_did <= 0)
            break;
        if ((ccheck->flags & 0x02) != 0)
            break;
        if ((ccheck->flags & 0x01) == 0)
        {
            delta = (game.play_gameturn - ccheck->turns_last);
            if ((delta > ccheck->turns_interval) && (ccheck->func != NULL))
            {
                SYNCDBG(18,"Executing check %ld",i);
                ccheck->func(comp, ccheck);
                ccheck->turns_last = game.play_gameturn;
            }
        }
    }
    return true;
}

TbBool process_processes_and_task(struct Computer2 *comp)
{
  struct ComputerProcess *process;
  Comp_Process_Func callback;
  int i;
  SYNCDBG(17,"Starting");
  for (i=comp->tasks_did; i > 0; i--)
  {
    if (comp->tasks_did <= 0)
        return false;
    if ((game.play_gameturn % comp->field_18) == 0)
        process_tasks(comp);
    switch (comp->field_0)
    {
    case 1:
        comp->gameturn_wait--;
        if (comp->gameturn_wait <= 0)
        {
          comp->gameturn_wait = comp->gameturn_delay;
          set_next_process(comp);
        }
        break;
    case 2:
        set_next_process(comp);
        break;
    case 3:
        if ((comp->ongoing_process > 0) && (comp->ongoing_process <= COMPUTER_PROCESSES_COUNT))
        {
          process = &comp->processes[comp->ongoing_process];
          callback = process->func_task;
          if (callback != NULL)
            callback(comp,process);
        } else
        {
          ERRORLOG("No Process %d for a computer player",(int)comp->ongoing_process);
          comp->field_0 = 1;
        }
        break;
    default:
        ERRORLOG("Invalid task state %d",(int)comp->field_0);
        break;
    }
  }
  return true;
}

void process_computer_player2(unsigned long plyr_idx)
{
  struct Computer2 *comp;
  SYNCDBG(7,"Starting for player %lu",plyr_idx);
  //_DK_process_computer_player2(plyr_idx);
  if (plyr_idx >= PLAYERS_COUNT)
      return;
  comp = &game.computer[plyr_idx];
  if ((comp->field_14 != 0) && (comp->field_2C <= game.play_gameturn))
    comp->tasks_did = 1;
  else
    comp->tasks_did = 0;
  if (comp->tasks_did <= 0)
    return;
  computer_check_events(comp);
  process_checks(comp);
  process_processes_and_task(comp);
  if ((comp->tasks_did < 0) || (comp->tasks_did > 1))
    ERRORLOG("Computer performed %d tasks instead of up to one",(int)comp->tasks_did);
}

struct ComputerProcess *computer_player_find_process_by_func_setup(long plyr_idx,Comp_Process_Func func_setup)
{
  struct ComputerProcess *process;
  struct Computer2 *comp;
  comp = &(game.computer[plyr_idx]);
  process = &comp->processes[0];
  while ((process->field_44 & 0x02) == 0)
  {
    if (process->func_setup == func_setup)
    {
        return process;
    }
    process++;
  }
  return NULL;
}

TbBool computer_player_demands_gold_check(long plyr_idx)
{
  struct ComputerProcess *dig_process;
  dig_process = computer_player_find_process_by_func_setup(plyr_idx,computer_setup_dig_to_gold);
  // If this computer player has no gold digging process
  if (dig_process == NULL)
  {
      SYNCDBG(18,"Player %d has no digging ability.",(int)plyr_idx);
      return false;
  }
  if ((dig_process->field_44 & 0x04) == 0)
  {
      SYNCDBG(18,"Player %d isn't interested in digging.",(int)plyr_idx);
      return false;
  }
  SYNCDBG(8,"Player %d wants to start digging.",(int)plyr_idx);
  // If the computer player needs to dig for gold
  if (gameadd.turn_last_checked_for_gold+5000 < game.play_gameturn)
  {
      set_flag_dword(&dig_process->field_44, 0x04, false);
      return true;
  }
  return false;
}

void process_computer_players2(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    TbBool needs_gold_check;
    int i;
    //_DK_process_computer_players2();
    needs_gold_check = false;
#ifdef PETTER_AI
    SAI_run_shared();
#endif
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (!player_exists(player))
          continue;
        dungeon = get_players_dungeon(player);
        if (((player->field_0 & 0x40) != 0) || ((dungeon->computer_enabled & 0x01) != 0))
        {
          if (player->field_2C == 1)
          {
#ifdef PETTER_AI
            SAI_run_for_player(i);
#else
            process_computer_player2(i);
            if (computer_player_demands_gold_check(i))
            {
              needs_gold_check = true;
            }
#endif
          }
        }
    }
    if (needs_gold_check)
    {
      SYNCDBG(0,"Computer players demand gold check.");
      gameadd.turn_last_checked_for_gold = game.play_gameturn;
      check_map_for_gold();
    } else
    if (gameadd.turn_last_checked_for_gold > game.play_gameturn)
    {
      gameadd.turn_last_checked_for_gold = 0;
    }
}

void setup_computer_players2(void)
{
  struct PlayerInfo *player;
  int i;
  gameadd.turn_last_checked_for_gold = game.play_gameturn;
  check_map_for_gold();
  for (i=0; i < COMPUTER_TASKS_COUNT; i++)
  {
    LbMemorySet(&game.computer_task[i], 0, sizeof(struct ComputerTask));
  }
#ifdef PETTER_AI
  SAI_init_for_map();
#endif
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player))
    {
      if (player->field_2C == 1)
      {
#ifdef PETTER_AI
        SAI_init_for_player(i);
#else
        setup_a_computer_player(i, 7);
#endif
      }
    }
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
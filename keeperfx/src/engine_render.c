/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_render.c
 *     Rendering the 3D view functions.
 * @par Purpose:
 *     Functions for displaying drawlist elements on screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Mar 2009 - 20 May 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_render.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_render.h"
#include "bflib_heapmgr.h"

#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "engine_textures.h"
#include "engine_redraw.h"
#include "creature_graphics.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "creature_states_gardn.h"
#include "creature_states_lair.h"
#include "thing_stats.h"
#include "game_lghtshdw.h"
#include "game_heap.h"
#include "kjm_input.h"
#include "gui_draw.h"
#include "front_simple.h"
#include "frontend.h"
#include "vidmode.h"
#include "vidfade.h"
#include "config_settings.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "game_legacy.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_fastview_mapwho(struct Camera *cam, struct JontySpr *outbuf);
DLLIMPORT void _DK_draw_stripey_line(long pos_x, long pos_z, long start_y, long end_y, unsigned char scale);
DLLIMPORT long _DK_convert_world_coord_to_front_view_screen_coord(struct Coord3d *pos, struct Camera *cam, long *x, long *y, long *z);
DLLIMPORT void _DK_rotate_base_axis(struct M33 *matx, short pos_z, unsigned char start_y);
DLLIMPORT void _DK_fill_in_points_perspective(long pos_x, long pos_z, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_cluedo(long pos_x, long pos_z, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_isometric(long pos_x, long pos_z, struct MinMax *mm);
DLLIMPORT void _DK_find_gamut(void);
DLLIMPORT void _DK_frame_wibble_generate(void);
DLLIMPORT void _DK_setup_rotate_stuff(long pos_x, long pos_z, long start_y, long end_y, long scale, long a6, long a7, long a8);
DLLIMPORT void _DK_do_a_trig_gourad_tr(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short plane_end, long scale);
DLLIMPORT void _DK_do_a_trig_gourad_bl(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short plane_end, long scale);
DLLIMPORT void _DK_do_map_who(short stl_x);
DLLIMPORT void _DK_fiddle_half_gamut(long y, long pos_y, long floor_x, long floor_y);
DLLIMPORT long _DK_do_a_plane_of_engine_columns_sub5(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3);
DLLIMPORT void _DK_do_a_gpoly_gourad_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end, int a5);
DLLIMPORT void _DK_do_a_gpoly_unlit_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end);
DLLIMPORT void _DK_do_a_gpoly_unlit_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end);
DLLIMPORT void _DK_do_a_gpoly_gourad_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short plane_end, int a5);
DLLIMPORT long _DK_find_closest_lights(struct Coord3d *pos, struct NearestLights *nlgt);
DLLIMPORT void _DK_create_shadows(struct Thing *thing, struct EngineCoord *ecor, struct Coord3d *pos);
DLLIMPORT void _DK_create_status_box(struct Thing *thing, struct EngineCoord *ecor);
/******************************************************************************/
unsigned short shield_offset[] = {
 0x0,  0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x118, 0x80,
 0x80, 0x100,  0x80,  0x80, 0x100, 0x100, 0x138,  0x80,  0x80, 0x138,  0x80,  0x80, 0x100,  0x80, 0x80, 0x100,
};
struct SideOri sideoris[] = {
    { 0,  1,  2,  3},
    { 0,  0,  3,  2},
    { 1,128,  2,  1},
    { 0,  3,128,  2},
    { 3,  0,  1,  0},
    { 3,  2,  1,  0},
    {128, 3,  0,  1},
    { 2,  0,  1,  2},
    { 3,  0,  0,  1},
    { 0,  3,  2,128},
};

long const x_offs[] =  { 0, 1, 1, 0};
long const y_offs[] =  { 0, 0, 1, 1};
long const x_step1[] = { 0,-1, 0, 1};
long const y_step1[] = { 1, 0,-1, 0};
long const x_step2[] = { 1, 0,-1, 0};
long const y_step2[] = { 0, 1, 0,-1};
long const orient_table_xflip[] =  {0, 0, 1, 1};
long const orient_table_yflip[] =  {0, 1, 1, 0};
long const orient_table_rotate[] = {0, 1, 0, 1};
long const orient_to_mapU1[] = { 0x00, 0x1F0000, 0x1F0000, 0x00 };
long const orient_to_mapU2[] = { 0x1F0000, 0x1F0000, 0x00, 0x00 };
long const orient_to_mapU3[] = { 0x1F0000, 0x00, 0x00, 0x1F0000 };
long const orient_to_mapU4[] = { 0x00, 0x00, 0x1F0000, 0x1F0000 };
long const orient_to_mapV1[] = { 0x00, 0x00, 0x1F0000, 0x1F0000 };
long const orient_to_mapV2[] = { 0x00, 0x1F0000, 0x1F0000, 0x00 };
long const orient_to_mapV3[] = { 0x1F0000, 0x1F0000, 0x00, 0x00 };
long const orient_to_mapV4[] = { 0x1F0000, 0x00, 0x00, 0x1F0000 };

unsigned char const height_masks[] = {
  0, 1, 2, 2, 3, 3, 3, 3,
  4, 4, 4, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  6, 6, 6, 6, 6, 6, 6, 6,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  7, 7, 7, 7, 7, 7, 7, 7,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
};

int water_wibble_angle = 0;
//unsigned char temp_cluedo_mode;
unsigned long render_problems;
long render_prob_kind;
long sp_x,sp_y,sp_dx,sp_dy;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
void do_map_who(short tnglist_idx);
/******************************************************************************/
void get_floor_pointed_at(long x, long y, long *floor_x, long *floor_y)
{
    long long ofs_x,ofs_y;
    long long sor_hp,sor_hn,sor_vp,sor_vn;
    long long der_hp,der_hn,der_vp,der_vn;
    if ( (vert_offset[1] == 0) && (hori_offset[1] == 0) )
    {
        *floor_x = 0;
        *floor_y = 0;
        return;
    }
    ofs_x = (long long)x - (long long)x_init_off;
    ofs_y = (long long)y - (long long)y_init_off;
    sor_vp = (((long long)vert_offset[1] * ofs_x) / 2LL);
    sor_vn = (((long long)vert_offset[0] * ofs_y) / 2LL);
    der_vp = ((long long)hori_offset[0] * (long long)vert_offset[1]) / 8LL;
    der_vn = ((long long)vert_offset[0] * (long long)hori_offset[1]) / 8LL;
    sor_hp = (((long long)hori_offset[1] * ofs_x) / 2LL);
    sor_hn = (((long long)hori_offset[0] * ofs_y) / 2LL);
    der_hp = ((long long)vert_offset[0] * (long long)hori_offset[1]) / 8LL;
    der_hn = ((long long)hori_offset[0] * (long long)vert_offset[1]) / 8LL;
    *floor_y = ((sor_vp-sor_vn) / ((der_vp-der_vn)>>8)) >> 2;
    *floor_x = ((sor_hp-sor_hn) / ((der_hp-der_hn)>>8)) >> 2;
}

long compute_cells_away(void)
{
    long half_width,half_height;
    long xmin,ymin,xmax,ymax;
    long xcell,ycell;
    struct PlayerInfo *player;
    long ncells_a;
    player = get_my_player();
    half_width = (player->engine_window_width >> 1);
    half_height = (player->engine_window_height >> 1);
    xcell = ((half_width<<1) + (half_width>>4))/pixel_size - player->engine_window_x/pixel_size;
    ycell = ((8 * high_offset[1]) >> 8) - (half_width>>4)/pixel_size - player->engine_window_y/pixel_size;
    get_floor_pointed_at(xcell, ycell, &xmax, &ymax);
    xcell = (half_width)/pixel_size - player->engine_window_x/pixel_size;
    ycell = (half_height)/pixel_size - player->engine_window_y/pixel_size;
    get_floor_pointed_at(xcell, ycell, &xmin, &ymin);
    xcell = abs(ymax - ymin);
    ycell = abs(xmax - xmin);
    if (ycell >= xcell)
        ncells_a = ycell + (xcell >> 1);
    else
        ncells_a = xcell + (ycell >> 1);
    ncells_a += 2;
    if (ncells_a > max_i_can_see)
      ncells_a = max_i_can_see;
    return ncells_a;
}

void init_coords_and_rotation(struct EngineCoord *origin,struct M33 *matx)
{
    origin->x = 0;
    origin->y = 0;
    origin->z = 0;
    matx->r[0].v[0] = 0x4000u;
    matx->r[0].v[1] = 0;
    matx->r[0].v[2] = 0;
    matx->r[1].v[0] = 0;
    matx->r[1].v[1] = 0x4000u;
    matx->r[1].v[2] = 0;
    matx->r[2].v[0] = 0;
    matx->r[2].v[1] = 0;
    matx->r[2].v[2] = 0x4000u;
}

void update_fade_limits(long ncells_a)
{
    fade_max = (ncells_a << 8);
    fade_scaler = (ncells_a << 8);
    fade_way_out = (ncells_a + 1) << 8;
    fade_min = (768 * ncells_a) / 4;
    split_1 = (split1at << 8);
    split_2 = (split2at << 8);
}

void update_normal_shade(struct M33 *matx)
{
    normal_shade_left = matx->r[2].v[0];
    normal_shade_right = -matx->r[2].v[0];
    normal_shade_back = -matx->r[2].v[2];
    normal_shade_front = matx->r[2].v[2];
    if (normal_shade_front < 0)
      normal_shade_front = 0;
    if (normal_shade_back < 0)
      normal_shade_back = 0;
    if (normal_shade_left < 0)
      normal_shade_left = 0;
    if (normal_shade_right < 0)
      normal_shade_right = 0;
}

void update_engine_settings(struct PlayerInfo *player)
{
    engine_player_number = player->id_number;
    player_bit = (1 << engine_player_number);
    switch (settings.field_0)
    {
    case 0:
        split1at = 4;
        split2at = 3;
        break;
    case 1:
        split1at = 3;
        split2at = 2;
        break;
    case 2:
        split1at = 2;
        split2at = 1;
        break;
    case 3:
    default:
        split1at = 0;
        split2at = 0;
        break;
    }
    me_pointed_at = NULL;
    me_distance = 100000000;
    max_i_can_see = get_creature_can_see_subtiles();
    if (lens_mode != 0)
      temp_cluedo_mode = 0;
    else
      temp_cluedo_mode = settings.video_cluedo_mode;
    thing_pointed_at = NULL;
}

/**
 * Sets the reserved amount of poly pool entries.
 * Entries which are reserved won't be filled by standard rendering items, even if the queue is full.
 * @param nitems
 */
void poly_pool_end_reserve(int nitems)
{
    poly_pool_end = &poly_pool[sizeof(poly_pool)-(nitems*sizeof(struct BasicUnk13)-1)];
}

TbBool is_free_space_in_poly_pool(int nitems)
{
    return (getpoly+(nitems*sizeof(struct BasicUnk13)) <= poly_pool_end);
}

void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx, long zoom)
{
    long factor_w,factor_h;
    long inp_x,inp_y,inp_z;
    long long out_x,out_y;
    inp_x = epos->x;
    inp_y = epos->y;
    inp_z = epos->z;
    out_x = (inp_z * matx->r[0].v[2] + (inp_y + matx->r[0].v[0]) * (inp_x + matx->r[0].v[1]) - matx->r[0].v[3] - inp_x * inp_y) >> 14;
    epos->x = out_x;
    out_y = (inp_z * matx->r[1].v[2] + (inp_y + matx->r[1].v[0]) * (inp_x + matx->r[1].v[1]) - matx->r[1].v[3] - inp_x * inp_y) >> 14;
    epos->y = out_y;
    epos->z = (inp_z * matx->r[2].v[2] + (inp_y + matx->r[2].v[0]) * (inp_x + matx->r[2].v[1]) - matx->r[2].v[3] - inp_x * inp_y) >> 14;
    factor_w = (long)view_width_over_2 + (zoom * out_x >> 16);
    epos->view_width = factor_w;
    factor_h = (long)view_height_over_2 - (zoom * out_y >> 16);
    epos->view_height = factor_h;
    if (factor_w < 0)
    {
        epos->field_8 |= 0x0008;
    } else
    if (vec_window_width <= factor_w)
    {
        epos->field_8 |= 0x0010;
    }
    if (factor_h < 0)
    {
        epos->field_8 |= 0x0020;
    } else
    if (factor_h >= vec_window_height)
    {
        epos->field_8 |= 0x0040;
    }
    epos->field_8 |= 0x0400;
}

void base_vec_normalisation(struct M33 *matx, unsigned char a2)
{
    struct M31 *vec;
    vec = &matx->r[a2];
    long rv0, rv1, rv2, rvlen;
    rv0 = vec->v[0];
    rv1 = vec->v[1];
    rv2 = vec->v[2];
    rvlen = LbSqrL(rv0 * rv0 + rv1 * rv1 + rv2 * rv2);
    vec->v[0] = (rv0 << 14) / rvlen;
    vec->v[1] = (rv1 << 14) / rvlen;
    vec->v[2] = (rv2 << 14) / rvlen;
}

void vec_cross_prod(struct M31 *outvec, const struct M31 *vec2, const struct M31 *vec3)
{
    outvec->v[0] = vec3->v[2] * vec2->v[1] - vec3->v[1] * vec2->v[2];
    outvec->v[1] = vec3->v[0] * vec2->v[2] - vec3->v[2] * vec2->v[0];
    outvec->v[2] = vec3->v[1] * vec2->v[0] - vec3->v[0] * vec2->v[1];
}

void matrix_transform(struct M31 *outvec, const struct M33 *matx, const struct M31 *vec2)
{
    outvec->v[0] = matx->r[0].v[2] * vec2->v[2] + matx->r[0].v[0] * vec2->v[0] + matx->r[0].v[1] * vec2->v[1];
    outvec->v[1] = matx->r[1].v[2] * vec2->v[2] + matx->r[1].v[0] * vec2->v[0] + matx->r[1].v[1] * vec2->v[1];
    outvec->v[2] = matx->r[2].v[2] * vec2->v[2] + matx->r[2].v[1] * vec2->v[1] + matx->r[2].v[0] * vec2->v[0];
}

void rotate_base_axis(struct M33 *matx, short angle, unsigned char axis)
{
    //_DK_rotate_base_axis(matx, a2, a3); return;

    unsigned char scor0, scor1, scor2;
    switch (axis)
    {
    case 1:
        scor0 = 1;
        scor1 = 2;
        scor2 = 0;
        break;
    case 2:
        scor0 = 0;
        scor1 = 2;
        scor2 = 1;
        break;
    case 3:
        scor0 = 0;
        scor1 = 1;
        scor2 = 2;
        break;
    default:
        ERRORLOG("Bad axis");
        scor0 = 0;
        scor1 = 1;
        scor2 = 2;
        break;
    }

    struct M33 matt;
    {
#define TRIG_LIMIT (1 << (LbFPMath_TrigmBits - 2))
        int angle_sin, angle_cos;
        angle_sin = LbSinL(angle) >> 2;
        angle_cos = LbCosL(angle) >> 2;
        long val0, val1, val2;
        val0 = matx->r[scor2].v[0];
        val2 = matx->r[scor2].v[2];
        val1 = matx->r[scor2].v[1];
        long shf0, shf1, shf2;
        long mag0, mag1, mag2;
        matt.r[0].v[0] = (val0 * val0 >> 14) + (angle_cos * (TRIG_LIMIT - (val0 * val0 >> 14)) >> 14);
        matt.r[1].v[1] = (val1 * val1 >> 14) + (angle_cos * (TRIG_LIMIT - (val1 * val1 >> 14)) >> 14);
        matt.r[2].v[2] = (val2 * val2 >> 14) + (angle_cos * (TRIG_LIMIT - (val2 * val2 >> 14)) >> 14);
        mag2 = (TRIG_LIMIT - angle_cos) * (val1 * val0 >> 14) >> 14;
        shf2 = angle_sin * val2 >> 14;
        mag1 = (TRIG_LIMIT - angle_cos) * (val0 * val2 >> 14) >> 14;
        shf1 = angle_sin * val1 >> 14;
        mag0 = (TRIG_LIMIT - angle_cos) * (val1 * val2 >> 14) >> 14;
        shf0 = angle_sin * val0 >> 14;
        matt.r[0].v[1] = mag2 - shf2;
        matt.r[0].v[2] = mag1 + shf1;
        matt.r[1].v[2] = mag0 - shf0;
        matt.r[1].v[0] = mag2 + shf2;
        matt.r[2].v[0] = mag1 - shf1;
        matt.r[2].v[1] = mag0 + shf0;
#undef TRIG_LIMIT
    }

    struct M31 locvec;
    matrix_transform(&locvec, &matt, &matx->r[scor0]);
    matx->r[scor0].v[0] = locvec.v[0] >> 14;
    matx->r[scor0].v[1] = locvec.v[1] >> 14;
    matx->r[scor0].v[2] = locvec.v[2] >> 14;
    matrix_transform(&locvec, &matt, &matx->r[scor1]);
    matx->r[scor1].v[0] = locvec.v[0] >> 14;
    matx->r[scor1].v[1] = locvec.v[1] >> 14;
    matx->r[scor1].v[2] = locvec.v[2] >> 14;
    base_vec_normalisation(matx, 2);

    vec_cross_prod(&locvec, &matx->r[2], &matx->r[0]);
    matx->r[1].v[0] = locvec.v[0] >> 14;
    matx->r[1].v[1] = locvec.v[1] >> 14;
    matx->r[1].v[2] = locvec.v[2] >> 14;
    base_vec_normalisation(matx, 1);

    vec_cross_prod(&locvec, &matx->r[1], &matx->r[2]);
    matx->r[0].v[0] = locvec.v[0] >> 14;
    matx->r[0].v[1] = locvec.v[1] >> 14;
    matx->r[0].v[2] = locvec.v[2] >> 14;
    base_vec_normalisation(matx, 0);

    matx->r[0].v[3] = matx->r[0].v[0] * matx->r[0].v[1];
    matx->r[1].v[3] = matx->r[1].v[0] * matx->r[1].v[1];
    matx->r[2].v[3] = matx->r[2].v[0] * matx->r[2].v[1];
}

void fill_in_points_perspective(long bstl_x, long bstl_y, struct MinMax *mm)
{
    //_DK_fill_in_points_perspective(bstl_x, bstl_y, mm); return;
    if ((bstl_y < 0) || (bstl_y > map_subtiles_y-1)) {
        return;
    }
    long mmin, mmax;
    mmin = min(mm[0].min,mm[1].min);
    mmax = max(mm[0].max,mm[1].max);
    if (mmin + bstl_x < 1)
      mmin = 1 - bstl_x;
    if (mmax + bstl_x > map_subtiles_y)
      mmax = map_subtiles_y - bstl_x;
    MapSubtlCoord stl_x, stl_y;
    stl_y = bstl_y;
    stl_x = mmin + bstl_x;
    apos += subtile_coord(mmin,0);
    struct EngineCol *ecol;
    ecol = &front_ec[mmin + 31];
    unsigned long mask_unrev;
    {
        struct Column *col;
        col = get_column(game.unrevealed_column_idx);
        mask_unrev = col->solidmask + 65536;
    }
    struct Map *mapblk;
    struct Column *col;
    unsigned long pfulmask_or, pfulmask_and;
    {
        unsigned long mask_cur, mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x-1, stl_y+1);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x-1, stl_y);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        pfulmask_or = mask_cur | mask_yp;
        pfulmask_and = mask_cur & mask_yp;
    }

    int wib_x, wib_y, wib_v;
    wib_y = (stl_y + 1) & 3;
    int idxx;
    for (idxx=mmax-mmin+1; idxx > 0; idxx--)
    {
        unsigned long mask_cur, mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x, stl_y);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        unsigned long nfulmask_or, nfulmask_and;
        nfulmask_or = mask_cur | mask_yp;
        nfulmask_and = mask_cur & mask_yp;
        unsigned long fulmask_or, fulmask_and;
        fulmask_or = nfulmask_or | pfulmask_or;
        fulmask_and = nfulmask_and & pfulmask_and;
        pfulmask_or = nfulmask_or;
        pfulmask_and = nfulmask_and;
        int lightness;
        lightness = 0;
        if ((fulmask_or & 0x10000) == 0)
            lightness = game.lish.subtile_lightness[get_subtile_number(stl_x, stl_y+1)];
        long hmin, hmax;
        hmax = height_masks[fulmask_or & 0xff];
        hmin = floor_height[fulmask_and & 0xff];
        struct EngineCoord *ecord;
        ecord = &ecol->cors[hmin];
        long hpos;
        hpos = subtile_coord(hmin,0) - view_alt;
        wib_x = stl_x & 3;
        struct WibbleTable *wibl;
        wibl = &wibble_table[32 * wib_v + wib_x + (wib_y << 2)];
        int idxh;
        for (idxh = hmax-hmin+1; idxh > 0; idxh--)
        {
            ecord->x = apos + wibl->field_0;
            ecord->y = hpos + wibl->field_4;
            ecord->z = bpos + wibl->field_8;
            ecord->field_8 = 0;
            lightness += wibl->field_C;
            if (lightness < 0)
                lightness = 0;
            if (lightness > 16128)
                lightness = 16128;
            ecord->field_A = lightness;
            wibl += 2;
            hpos += COORD_PER_STL;
            rotpers(ecord, &camera_matrix);
            ecord++;
        }
        wibl -= 2;
        // Set ceiling
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        hpos = subtile_coord(get_mapblk_filled_subtiles(mapblk),0) - view_alt;
        if (wib_v == 2)
          wibl = &wibble_table[wib_x + 2 * (hmax + 2 * wib_y - hmin) + 32];
        ecord = &ecol->cors[8];
        {
            ecord->x = apos + wibl->field_0;
            ecord->y = hpos + wibl->field_4;
            ecord->z = bpos + wibl->field_8;
            ecord->field_8 = 0;
            // Use lightness from last cube
            ecord->field_A = lightness;
            rotpers(ecord, &camera_matrix);
        }
        stl_x++;
        ecol++;
        apos += COORD_PER_STL;
    }
}

void fill_in_points_cluedo(long bstl_x, long bstl_y, struct MinMax *mm)
{
    //_DK_fill_in_points_cluedo(bstl_x, bstl_y, mm);
    if ((bstl_y < 0) || (bstl_y > map_subtiles_y-1)) {
        return;
    }
    long mmin, mmax;
    mmin = min(mm[0].min,mm[1].min);
    mmax = max(mm[0].max,mm[1].max);
    if (mmin + bstl_x < 1) {
        mmin = 1 - bstl_x;
    }
    if (mmax + bstl_x > map_subtiles_y) {
        mmax = map_subtiles_y - bstl_x;
    }
    if (mmax < mmin) {
        return;
    }
    MapSubtlCoord stl_x, stl_y;
    stl_y = bstl_y;
    stl_x = mmin + bstl_x;
    apos += (mmin << 8);
    struct EngineCol *ecol;
    ecol = &front_ec[mmin + 31];
    unsigned long mask_unrev;
    {
        struct Column *col;
        col = get_column(game.unrevealed_column_idx);
        mask_unrev = (col->solidmask & 3) + 65536;
    }
    struct Map *mapblk;
    struct Column *col;
    unsigned long pfulmask_or, pfulmask_and;
    {
        unsigned long mask_cur, mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x-1, stl_y+1);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
            if ((mask_cur >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_cur &= 3;
            }
        }
        mapblk = get_map_block_at(stl_x-1, stl_y);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
            if ((mask_yp >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_yp &= 3;
            }
        }
        pfulmask_or = mask_cur | mask_yp;
        pfulmask_and = mask_cur & mask_yp;
    }
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    const struct Camera *cam;
    cam = myplyr->acamera;
    long view_z;
    int zoom;
    long eview_w, eview_h, eview_z;
    int hview_y, hview_z;
    zoom = cam->zoom / pixel_size;
    view_z = object_origin.z + (cells_away << 8)
        + ((bpos * camera_matrix.r[2].v[2]
         + (apos + camera_matrix.r[2].v[1]) * (camera_matrix.r[2].v[0] - view_alt)
          - camera_matrix.r[2].v[3]
          - apos * -view_alt) >> 14);
    eview_w = (view_width_over_2 + (zoom
          * (object_origin.x
           + ((bpos * camera_matrix.r[0].v[2]
            + (apos + camera_matrix.r[0].v[1]) * (camera_matrix.r[0].v[0] - view_alt)
             - camera_matrix.r[0].v[3]
             - apos * -view_alt) >> 14)) >> 16)) << 8;
    hview_y = (view_height_over_2 - (zoom
          * (object_origin.y
           + ((bpos * camera_matrix.r[1].v[2]
            + (apos + camera_matrix.r[1].v[1]) * (camera_matrix.r[1].v[0] - view_alt)
             - camera_matrix.r[1].v[3]
             - apos * -view_alt) >> 14)) >> 16)) << 8;
    hview_z = (abs(view_z) >> 1);
    if (hview_z < 32) {
        hview_z = 0;
    } else
    if (hview_z >= 11232) {
        hview_z = 11232;
    }
    int dview_w, dview_h, dview_z;
    int dhview_y, dhview_z;

    dview_w = zoom * camera_matrix.r[0].v[0] >> 14;
    dhview_y = -(zoom * camera_matrix.r[1].v[0]) >> 14;
    dhview_z = camera_matrix.r[2].v[0] >> 7;
    dview_h = -(zoom * camera_matrix.r[1].v[1]) >> 14;
    dview_z = camera_matrix.r[2].v[1] >> 7;
    int wib_x, wib_y, wib_v;
    wib_y = (stl_y + 1) & 3;
    int idxx;
    for (idxx=mmax-mmin+1; idxx > 0; idxx--)
    {
        unsigned long mask_cur, mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
            if ((mask_cur >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_cur &= 3;
            }
        }
        mapblk = get_map_block_at(stl_x, stl_y);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
            if ((mask_yp >= 8) && ((mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((col->bitfields & 0xE) == 0)) {
                mask_yp &= 3;
            }
        }
        unsigned long nfulmask_or, nfulmask_and;
        nfulmask_or = mask_cur | mask_yp;
        nfulmask_and = mask_cur & mask_yp;
        unsigned long fulmask_or, fulmask_and;
        fulmask_or = nfulmask_or | pfulmask_or;
        fulmask_and = nfulmask_and & pfulmask_and;
        pfulmask_or = nfulmask_or;
        pfulmask_and = nfulmask_and;
        int lightness;
        lightness = 0;
        if ((fulmask_or & 0x10000) == 0)
            lightness = game.lish.subtile_lightness[get_subtile_number(stl_x, stl_y+1)];

        long hmin, hmax;
        hmax = height_masks[fulmask_or & 0xff];
        hmin = floor_height[fulmask_and & 0xff];
        struct EngineCoord *ecord;
        ecord = &ecol->cors[hmin];
        wib_x = stl_x & 3;
        struct WibbleTable *wibl;
        wibl = &wibble_table[32 * wib_v + wib_x + (wib_y << 2)];
        long *randmis;
        randmis = &randomisors[(stl_x + 17 * (stl_y + 1)) & 0xff];
        eview_h = dview_h * hmin + hview_y;
        eview_z = dview_z * hmin + hview_z;
        int idxh;
        for (idxh = hmax-hmin+1; idxh > 0; idxh--)
        {
            ecord->view_width = (eview_w + wibl->field_10) >> 8;
            ecord->view_height = (eview_h + wibl->field_14) >> 8;
            ecord->z = eview_z;
            ecord->field_8 = 0;
            lightness += *randmis;
            if (lightness < 0)
                lightness = 0;
            if (lightness > 16128)
                lightness = 16128;
            ecord->field_A = lightness;
            if (ecord->z < 32) {
                ecord->z = 0;
            } else
            if (ecord->z >= 11232) {
                ecord->z = 11232;
            }
            if (ecord->view_width < 0) {
                ecord->field_8 |= 0x08;
            } else
            if (ecord->view_width >= vec_window_width) {
                ecord->field_8 |= 0x10;
            }
            if (ecord->view_height < 0) {
                ecord->field_8 |= 0x20;
            } else
            if (ecord->view_height >= vec_window_height) {
                ecord->field_8 |= 0x40;
            }

            wibl += 2;
            ecord++;
            randmis++;
            eview_h += dview_h;
            eview_z += dview_z;
        }
        stl_x++;
        ecol++;
        apos += 256;
        eview_w += dview_w;
        hview_y += dhview_y;
        hview_z += dhview_z;
    }
}

void fill_in_points_isometric(long bstl_x, long bstl_y, struct MinMax *mm)
{
    //_DK_fill_in_points_isometric(bstl_x, bstl_y, mm);
    if ((bstl_y < 0) || (bstl_y > map_subtiles_y-1)) {
        return;
    }
    long mmin, mmax;
    TbBool clip_min, clip_max;
    mmin = min(mm[0].min,mm[1].min);
    mmax = max(mm[0].max,mm[1].max);
    clip_min = false;
    clip_max = false;
    if (mmin + bstl_x < 1) {
        clip_min = true;
        mmin = 1 - bstl_x;
    }
    if (mmax + bstl_x > map_subtiles_y) {
        clip_max = true;
        mmax = map_subtiles_y - bstl_x;
    }
    if (mmax < mmin) {
        return;
    }
    MapSubtlCoord stl_x, stl_y;
    stl_y = bstl_y;
    stl_x = mmin + bstl_x;
    TbBool lim_min, lim_max;
    lim_min = (stl_y <= 0);
    lim_max = (stl_y >= map_subtiles_y-1);
    TbBool clip;
    clip = clip_min | clip_max | lim_max | lim_min;
    apos += (mmin << 8);
    struct EngineCol *ecol;
    ecol = &front_ec[mmin + 31];
    unsigned long mask_unrev;
    {
        struct Column *col;
        col = get_column(game.unrevealed_column_idx);
        mask_unrev = col->solidmask + 65536;
    }
    struct Map *mapblk;
    struct Column *col;
    unsigned long pfulmask_or, pfulmask_and;
    {
        unsigned long mask_cur, mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x-1, stl_y+1);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x-1, stl_y);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        if (clip)
        {
            if (clip_min || lim_min)
                mask_cur = 0;
            if (clip_min || lim_max)
                mask_yp = 0;
        }
        pfulmask_or = mask_cur | mask_yp;
        pfulmask_and = mask_cur & mask_yp;
    }
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    const struct Camera *cam;
    cam = myplyr->acamera;
    long hpos;
    long view_x, view_y, view_z;
    int zoom;
    int hview_z;
    zoom = cam->zoom / pixel_size;
    hpos = -view_alt * apos;
    view_x = view_width_over_2 + (zoom
         * (object_origin.x
          + ((bpos * camera_matrix.r[0].v[2]
           + (apos + camera_matrix.r[0].v[1]) * (camera_matrix.r[0].v[0] - view_alt)
            - hpos - camera_matrix.r[0].v[3]) >> 14)) >> 16);
    view_y = view_height_over_2 - (zoom
         * (object_origin.y
          + ((bpos * camera_matrix.r[1].v[2]
           + (apos + camera_matrix.r[1].v[1]) * (camera_matrix.r[1].v[0] - view_alt)
            - hpos - camera_matrix.r[1].v[3]) >> 14)) >> 16);
    view_z = object_origin.z + (cells_away << 8)
        + ((bpos * camera_matrix.r[2].v[2]
         + (apos + camera_matrix.r[2].v[1]) * (camera_matrix.r[2].v[0] - view_alt)
          - hpos - camera_matrix.r[2].v[3]) >> 14);
    hview_z = (abs(view_z) >> 1);
    if (hview_z < 32) {
        hview_z = 0;
    } else
    if (hview_z >= 11232) {
        hview_z = 11232;
    }
    long eview_w, eview_h, eview_z;
    long hview_y;
    long *randmis;
    int dview_w, dview_h, dview_z;
    int dhview_y, dhview_z;

    eview_w = view_x << 8;
    hview_y = view_y << 8;
    dview_w = zoom * camera_matrix.r[0].v[0] >> 14;
    dhview_y = -(zoom * camera_matrix.r[1].v[0]) >> 14;
    dhview_z = camera_matrix.r[2].v[0] >> 7;
    dview_h = -(zoom * camera_matrix.r[1].v[1]) >> 14;
    dview_z = camera_matrix.r[2].v[1] >> 7;
    int wib_x, wib_y, wib_v;
    wib_y = (stl_y + 1) & 3;
    int idxx;
    for (idxx=mmax-mmin+1; idxx > 0; idxx--)
    {
        unsigned long mask_cur, mask_yp;
        mask_cur = mask_unrev;
        mask_yp = mask_unrev;
        mapblk = get_map_block_at(stl_x, stl_y+1);
        wib_v = get_mapblk_wibble_value(mapblk);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_cur = col->solidmask;
        }
        mapblk = get_map_block_at(stl_x, stl_y);
        if (map_block_revealed_bit(mapblk, player_bit)) {
            col = get_map_column(mapblk);
            mask_yp = col->solidmask;
        }
        if (clip)
        {
            if (clip_max && (idxx == 1)) {
                mask_cur = 0;
                mask_yp = 0;
            }
            if (lim_min)
                mask_cur = 0;
            if (lim_max)
                mask_yp = 0;
        }
        unsigned long nfulmask_or, nfulmask_and;
        nfulmask_or = mask_cur | mask_yp;
        nfulmask_and = mask_cur & mask_yp;
        unsigned long fulmask_or, fulmask_and;
        fulmask_or = nfulmask_or | pfulmask_or;
        fulmask_and = nfulmask_and & pfulmask_and;
        pfulmask_or = nfulmask_or;
        pfulmask_and = nfulmask_and;
        int lightness;
        lightness = 0;
        if ((fulmask_or & 0x10000) == 0)
            lightness = game.lish.subtile_lightness[get_subtile_number(stl_x, stl_y+1)];
        long hmin, hmax;
        hmax = height_masks[fulmask_or & 0xff];
        hmin = floor_height[fulmask_and & 0xff];
        struct EngineCoord *ecord;
        ecord = &ecol->cors[hmin];
        wib_x = stl_x & 3;
        struct WibbleTable *wibl;
        wibl = &wibble_table[32 * wib_v + wib_x + (wib_y << 2)];
        eview_h = dview_h * hmin + hview_y;
        eview_z = dview_z * hmin + hview_z;
        randmis = &randomisors[(stl_x + 17 * (stl_y+1)) & 0xff] + hmin;
        int idxh;
        for (idxh = hmax-hmin+1; idxh > 0; idxh--)
        {
            ecord->view_width = (eview_w + wibl->field_10) >> 8;
            ecord->view_height = (eview_h + wibl->field_14) >> 8;
            ecord->z = eview_z;
            ecord->field_8 = 0;
            lightness += 4 * (*randmis & 0xff) - 512;
            if (lightness < 0)
                lightness = 0;
            if (lightness > 15872)
                lightness = 15872;
            ecord->field_A = lightness;
            if (ecord->z < 32) {
                ecord->z = 0;
            } else
            if (ecord->z >= 11232) {
                ecord->z = 11232;
            }
            if (ecord->view_width < 0) {
                ecord->field_8 |= 0x08;
            } else
            if (ecord->view_width >= vec_window_width) {
                ecord->field_8 |= 0x10;
            }
            if (ecord->view_height < 0) {
                ecord->field_8 |= 0x20;
            } else
            if (ecord->view_height >= vec_window_height) {
                ecord->field_8 |= 0x40;
            }
            wibl += 2;
            ecord++;
            randmis++;
            eview_h += dview_h;
            eview_z += dview_z;
        }
        stl_x++;
        ecol++;
        apos += 256;
        hview_z += dhview_z;
        eview_w += dview_w;
        hview_y += dhview_y;
    }
}

void frame_wibble_generate(void)
{
    //_DK_frame_wibble_generate(); return;
    int i;
    struct WibbleTable *wibl;
    wibl = &wibble_table[64];
    for (i = 0; i < 16; i++)
    {
        unsigned short angle;
        int osc;
        angle = water_wibble_angle + ((i & 0xFFFC) * ((i & 3) + 1) << 7);
        osc = LbSinL(angle);
        wibl->field_4 = osc >> 11;
        wibl->field_C = osc >> 6;
        wibl++;
    }
    water_wibble_angle = (water_wibble_angle + 46) & 0x7FF;
    int zoom;
    {
        struct PlayerInfo *myplyr;
        myplyr = get_my_player();
        const struct Camera *cam;
        cam = myplyr->acamera;
        zoom = cam->zoom / pixel_size;
    }

    int zm00, zm02;
    int zm10, zm11, zm12;
    zm00 = zoom * camera_matrix.r[0].v[0] >> 14;
    zm02 = zoom * camera_matrix.r[0].v[2] >> 14;
    zm10 = zoom * camera_matrix.r[1].v[0] >> 14;
    zm12 = zoom * camera_matrix.r[1].v[2] >> 14;
    zm11 = zoom * camera_matrix.r[1].v[1] >> 14;

    wibl = &wibble_table[32];
    for (i=64; i > 0; i--)
    {
        wibl->field_10 =   ((zm00 * wibl->field_0) >> 8)
                         + ((zm02 * wibl->field_8) >> 8);
        wibl->field_14 = -(((zm12 * wibl->field_8) >> 8)
                         + ((zm10 * wibl->field_0) >> 8)
                         + ((zm11 * wibl->field_4) >> 8));
        wibl++;
    }
}

void setup_rotate_stuff(long x, long y, long z, long fade_max, long fade_min, long zoom, long map_angle, long map_roll)
{
    //_DK_setup_rotate_stuff(x, y, z, fade_max, fade_min, zoom, map_angle, map_roll);
    view_width_over_2 = vec_window_width / 2;
    view_height_over_2 = vec_window_height / 2;
    map_x_pos = x;
    map_y_pos = y;
    map_z_pos = z;
    thelens = zoom;
    spr_map_angle = map_angle;
    lfade_min = fade_min;
    lfade_max = fade_max;
    fade_mmm = fade_max - fade_min;
}

void create_box_coords(struct EngineCoord *coord, long x, long z, long y)
{
    coord->x = x;
    coord->z = z;
    coord->field_8 = 0;
    coord->y = y;
    rotpers(coord, &camera_matrix);
}

void do_perspective_rotation(long x, long y, long z)
{
    struct PlayerInfo *player;
    struct EngineCoord epos;
    long zoom;
    long engine_w,engine_h;
    player = get_my_player();
    zoom = camera_zoom / pixel_size;
    engine_w = player->engine_window_width/pixel_size;
    engine_h = player->engine_window_height/pixel_size;
    epos.x = -x;
    epos.y = 0;
    epos.z = y;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    x_init_off = epos.view_width;
    y_init_off = epos.view_height;
    depth_init_off = epos.z;
    epos.x = 65536;
    epos.y = 0;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    hori_offset[0] = epos.view_width - (engine_w >> 1);
    hori_offset[1] = epos.view_height - (engine_h >> 1);
    hori_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 0;
    epos.z = -65536;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    vert_offset[0] = epos.view_width - (engine_w >> 1);
    vert_offset[1] = epos.view_height - (engine_h >> 1);
    vert_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 65536;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix, zoom);
    high_offset[0] = epos.view_width - (engine_w >> 1);
    high_offset[1] = epos.view_height - (engine_h >> 1);
    high_offset[2] = epos.z;
}

void find_gamut(void)
{
    SYNCDBG(19,"Starting");
    //_DK_find_gamut(); return;
    {
        long cell_cur, cell_lim;
        struct MinMax *mml;
        struct MinMax *mmr;
        cell_lim = cells_away + 1;
        mml = &minmaxs[31];
        mmr = &minmaxs[31];
        for (cell_cur = 0; cell_cur < cell_lim; cell_cur++)
        {
            long dist;
            dist = LbSqrL(cell_lim * cell_lim - cell_cur * cell_cur);
            mmr->max = dist;
            mml->max = dist;
            dist = -mmr->max;
            mmr->min = dist;
            mml->min = dist;
            mmr++;
            mml--;
        }
    }
    if (lens_mode == 0) {
        return;
    }

    int angle_sin, angle_cos;
    angle_sin = LbSinL(cam_map_angle);
    angle_cos = LbCosL(cam_map_angle);
    int cells_w, cells_h;
    cells_h = 6 * angle_cos >> 16;
    cells_w = -6 * angle_sin >> 16;
    int scr_w1, scr_h1, scr_w2, scr_h2;
    long screen_dist;
    screen_dist = (lbDisplay.PhysicalScreenWidth << 7) / lens;
    scr_w1 = cells_w + ((screen_dist * angle_cos - (angle_sin << 8)) >> 16);
    scr_h1 = cells_h + (((angle_cos << 8) + screen_dist * angle_sin) >> 16);
    scr_w2 = cells_w + ((-screen_dist * angle_cos - (angle_sin << 8)) >> 16);
    scr_h2 = cells_h + (((angle_cos << 8) - screen_dist * angle_sin) >> 16);
    int mbase, delta;
    struct MinMax *mm;
    int cell_curr;
    if (scr_h1 < cells_h)
    {
        delta = ((scr_w1 - cells_w) << 8) / (scr_h1 - cells_h);
        mm = &minmaxs[-cells_away + 31];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if (mm->max > nlimit)
                mm->max = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    if (scr_h1 > cells_h)
    {
        delta = ((scr_w1 - cells_w) << 8) / (scr_h1 - cells_h);
        mm = &minmaxs[-cells_away + 31];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if (mm->min < nlimit)
                mm->min = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    {
        if (scr_w1 <= cells_w)
        {
            mm = &minmaxs[cells_h + 31];
            for (cell_curr = cells_h; cell_curr >= -cells_away; cell_curr--)
            {
                mm->max = 0;
                mm->min = 0;
                mm--;
            }
        } else
        {
            mm = &minmaxs[cells_h + 31];
            for (cell_curr = cells_h; cell_curr <= cells_away; cell_curr++)
            {
                mm->max = 0;
                mm->min = 0;
                mm++;
            }
        }
    }

    if (scr_h2 < cells_h)
    {
        delta = ((scr_w2 - cells_w) << 8) / (scr_h2 - cells_h);
        mm = &minmaxs[-cells_away + 31];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if ( mm->min < nlimit )
              mm->min = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    if (scr_h2 > cells_h)
    {
        delta = ((scr_w2 - cells_w) << 8) / (scr_h2 - cells_h);
        mm = &minmaxs[-cells_away + 31];
        mbase = delta * (-cells_away - cells_h);
        for (cell_curr = -cells_away; cell_curr <= cells_away; cell_curr++)
        {
            int nlimit;
            nlimit = cells_w + (mbase >> 8);
            if (mm->max > nlimit)
              mm->max = nlimit;
            mm++;
            mbase += delta;
        }
    } else
    {
        if (cells_w <= scr_w2)
        {
            mm = &minmaxs[cells_h + 31];
            for ( ; cells_h >= -cells_away; cells_h--)
            {
                mm->max = 0;
                mm->min = 0;
                mm--;
            }
        } else
        {
            mm = &minmaxs[cells_h + 31];
            for ( ; cells_away >= cells_h; cells_h++)
            {
                mm->max = 0;
                mm->min = 0;
                mm++;
            }
        }
    }
}

void fiddle_half_gamut(long start_stl_x, long start_stl_y, long step, long a4)
{
    //_DK_fiddle_half_gamut(a1, a2, a3, a4);
    long end_stl_x;
    long stl_xc, stl_xp, stl_xn;

    end_stl_x = start_stl_x + minmaxs[32].min;
    for (stl_xc=start_stl_x; 1; stl_xc--)
    {
        if (stl_xc < end_stl_x) {
            stl_xc = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xc, start_stl_y);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xp=start_stl_x; 1; stl_xp--)
    {
        if (stl_xp < end_stl_x) {
            stl_xp = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xp, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xn=start_stl_x; 1; stl_xn--)
    {
        if (stl_xn < end_stl_x) {
            stl_xn = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xn, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    long stl_x_min;
    stl_x_min = 0;
    TbBool set_x_min;
    set_x_min = false;
    if ((stl_xc != -4000) && (stl_xp != -4000) && (stl_xn != -4000))
    {
        stl_x_min = min(min(stl_xn, stl_xp), stl_xc);
        set_x_min = true;
        minmaxs[32].min = stl_x_min - start_stl_x;
    }

    end_stl_x = start_stl_x + minmaxs[32].max;
    for (stl_xc=start_stl_x; 1; stl_xc++)
    {
        if (stl_xc > end_stl_x) {
            stl_xc = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xc, start_stl_y);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xp=start_stl_x; 1; stl_xp++)
    {
        if (stl_xp > end_stl_x) {
            stl_xp = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xp, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    for (stl_xn=start_stl_x; 1; stl_xn++)
    {
        if (stl_xn > end_stl_x) {
            stl_xn = -4000;
            break;
        }
        struct Map *mapblk;
        mapblk = get_map_block_at(stl_xn, start_stl_y-1);
        if  ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
            break;
        }
    }
    long stl_x_max;
    stl_x_max = 0;
    TbBool set_x_max;
    set_x_max = false;
    if ((stl_xc != -4000) && (stl_xp != -4000) && (stl_xn != -4000))
    {
        stl_x_max = max(max(stl_xn, stl_xp), stl_xc);
        set_x_max = true;
        minmaxs[32].max = stl_x_max - start_stl_x + 1;
    }

    struct MinMax *mm;
    long stl_y;
    stl_y = start_stl_y + step;
    mm = &minmaxs[step + 32];
    long n;
    for (n=1; n < a4; n++)
    {
        if (mm->max <= mm->min)
        {
            long i;
            for (i=a4-n; i > 0; i--)
            {
                mm->min = 0;
                mm->max = 0;
                mm += step;
            }
            break;
        }
        long stl_x_min_limit;
        stl_x_min_limit = start_stl_x + mm->min;
        if (!set_x_min || (stl_x_min < stl_x_min_limit)) {
            stl_x_min = stl_x_min_limit;
        }
        long stl_x_max_limit;
        stl_x_max_limit = start_stl_x + mm->max;
        if (!set_x_max || (stl_x_max > stl_x_max_limit)) {
            stl_x_max = stl_x_max_limit;
        }

        /* The variable needs to be volatile to disallow changing it to float during optimisations.
         * Changing it to float would lead to conditions like "if (delta_y != 1)" not working.
         */
        volatile long delta_y;
        delta_y = abs(stl_y - start_stl_y);
        long rect_factor;

        TbBool set_x_min_rect;
        if (delta_y != 1) {
            rect_factor = (stl_x_min - start_stl_x) / (delta_y - 1);
        } else {
            rect_factor = 1;
        }
        if (rect_factor - 1 <= 0) {
            set_x_min_rect = false;
        } else {
            set_x_min_rect = true;
            stl_x_min = rect_factor + stl_x_min - 1;
        }

        long stl_x;
        long stl_x_lc_min;

        for (stl_x=stl_x_min-1; stl_x < stl_x_max_limit; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x+1, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
              break;
        }
        stl_x_lc_min = stl_x;

        if ( set_x_min_rect
          || stl_x_lc_min > stl_x_min
          || (get_map_block_at(stl_x_lc_min, stl_y)->flags & SlbAtFlg_Blocking) )
        {
            long stl_tmp;
            stl_tmp = stl_x_min - start_stl_x;
            stl_x_min = stl_x_lc_min;
            set_x_min = true;
            mm->min = stl_tmp - 1;
        }
        else
        {
          if (delta_y != 1) {
              rect_factor = (stl_x_lc_min - start_stl_x) / (delta_y - 1);
          } else {
              rect_factor = 1;
          }
          long stl_x_min_sublim;
          if ((delta_y == 1) || (stl_x_min + rect_factor - 1 < stl_x_min_limit))
          {
              set_x_min = false;
              stl_x_min_sublim = stl_x_min_limit;
          } else
          {
              stl_x_min += rect_factor - 1;
              set_x_min = true;
              stl_x_min_sublim = stl_x_min;
              mm->min = stl_x_min - start_stl_x - 1;
          }
          for (stl_x=stl_x_min; stl_x >= stl_x_min_sublim; stl_x--)
          {
              struct Map *mapblk;
              mapblk = get_map_block_at(stl_x, stl_y);
              if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                  stl_x_min = stl_x;
                  set_x_min = true;
                  mm->min = stl_x - start_stl_x - 1;
                  break;
              }
          }
        }

        TbBool set_x_max_rect;
        if (delta_y != 1) {
            rect_factor = (stl_x_max - start_stl_x) / (delta_y - 1);
        } else {
            rect_factor = 1;
        }
        if (rect_factor + 1 >= 0) {
            set_x_max_rect = false;
        } else {
            set_x_max_rect = true;
            stl_x_max += rect_factor + 1;
        }

        for (stl_x=stl_x_max+1; stl_x > stl_x_min_limit; stl_x--)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x-1, stl_y);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
              break;
        }
        stl_x_lc_min = stl_x;

        if ( set_x_max_rect
          || stl_x_lc_min < stl_x_max
          || (get_map_block_at(stl_x_lc_min, stl_y)->flags & SlbAtFlg_Blocking) )
        {
            long stl_tmp;
            stl_tmp = stl_x_max - start_stl_x;
            stl_x_max = stl_x_lc_min;
            mm->max = stl_tmp + 2;
            set_x_max = true;
        }
        else
        {
          set_x_max = 0;
          if (delta_y != 1) {
              rect_factor = (stl_x_lc_min - start_stl_x) / (delta_y - 1);
          } else {
              rect_factor = 1;
          }

          long stl_x_max_sublim;
          if ((delta_y == 1) || (stl_x_max + rect_factor + 1 > start_stl_x + mm->max))
          {
              stl_x_max_sublim = start_stl_x + mm->max;
          } else
          {
              stl_x_max += rect_factor + 1;
              set_x_max = true;
              stl_x_max_sublim = stl_x_max;
              mm->max = stl_x_max - start_stl_x + 2;
          }
          for (stl_x=stl_x_max; stl_x <= stl_x_max_sublim; stl_x++)
          {
              struct Map *mapblk;
              mapblk = get_map_block_at(stl_x, stl_y);
              if ((mapblk->flags & SlbAtFlg_Blocking) != 0) {
                  set_x_max = true;
                  stl_x_max = stl_x;
                  mm->max = stl_x - start_stl_x + 2;
                  break;
              }
          }
        }

        if (mm->min < -cells_away)
            mm->min = -cells_away;
        if (mm->max > cells_away)
            mm->max = cells_away;
        if (mm->min >= mm->max)
        {
            long i;
            for (i=a4-n; i > 0; i--)
            {
                mm->min = 0;
                mm->max = 0;
                mm += step;
            }
            break;
        }
        mm += step;
        stl_y += step;
    }
}

void fiddle_gamut_find_limits(long *floor_x, long *floor_y, long ewwidth, long ewheight, long ewzoom)
{
    long len_01,len_02,len_13,len_23;
    long tmp_y,tmp_x;
    long i;
    get_floor_pointed_at(ewwidth + ewzoom, -ewzoom, &floor_y[2], &floor_x[2]);
    get_floor_pointed_at(ewwidth + ewzoom, ewheight + ewzoom, &floor_y[1], &floor_x[1]);
    get_floor_pointed_at(-ewzoom, ewheight + ewzoom, &floor_y[0], &floor_x[0]);
    get_floor_pointed_at(-ewzoom, -ewzoom, &floor_y[3], &floor_x[3]);
    // Get the value with lowest X coord into [0]
    for (i=1; i < 4; i++)
    {
        tmp_y = floor_y[i];
        if (floor_y[0] > tmp_y)
        {
          tmp_x = floor_x[i];
          floor_x[i] = floor_x[0];
          floor_x[0] = tmp_x;
          floor_y[i] = floor_y[0];
          floor_y[0] = tmp_y;
        }
    }
    // Get the value with highest X coord into [3]
    for (i=0; i < 3; i++)
    {
        tmp_y = floor_y[i];
        if (floor_y[3] < tmp_y)
        {
          tmp_x = floor_x[i];
          floor_x[i] = floor_x[3];
          floor_x[3] = tmp_x;
          floor_y[i] = floor_y[3];
          floor_y[3] = tmp_y;
        }
    }
    // Between values with medicore X, place the lowest Y first
    if (floor_x[1] > floor_x[2])
    {
        tmp_x = floor_x[1];
        tmp_y = floor_y[1];
        floor_x[1] = floor_x[2];
        floor_x[2] = tmp_x;
        floor_y[1] = floor_y[2];
        floor_y[2] = tmp_y;
    }

    // Lengths of X vectors
    len_01 = abs(floor_y[1] - floor_y[0]);
    len_13 = abs(floor_y[3] - floor_y[1]);
    len_02 = abs(floor_y[2] - floor_y[0]);
    len_23 = abs(floor_y[3] - floor_y[2]);
    // Update points according to both coordinates
    if ( (floor_x[1] > floor_x[0]) && (len_01 < len_13) )
    {
        tmp_x = floor_x[1];
        floor_y[1] = floor_y[0];
        floor_x[1] = floor_x[0];
        floor_x[0] = tmp_x;
    }
    if ( (floor_x[1] > floor_x[3]) && (len_13 < len_01) )
    {
        tmp_x = floor_x[1];
        floor_y[1] = floor_y[3];
        floor_x[1] = floor_x[3];
        floor_x[3] = tmp_x;
    }
    if ( (floor_x[2] < floor_x[0]) && (len_02 < len_23) )
    {
        tmp_x = floor_x[2];
        floor_y[2] = floor_y[0];
        floor_x[2] = floor_x[0];
        floor_x[0] = tmp_x;
    }
    if ( (floor_x[2] < floor_x[3]) && (len_23 < len_02) )
    {
        tmp_x = floor_x[2];
        floor_x[2] = floor_x[3];
        floor_x[3] = tmp_x;
        floor_y[2] = floor_y[3];
    }
}

void fiddle_gamut_set_base(long *floor_x, long *floor_y, long pos_x, long pos_y)
{
    floor_x[0] -= pos_x;
    floor_x[1] -= pos_x;
    floor_y[0] += 32 - pos_y;
    floor_x[2] -= pos_x;
    floor_y[1] += 32 - pos_y;
    floor_y[2] += 32 - pos_y;
    floor_x[3] -= pos_x;
    floor_y[3] += 32 - pos_y;
}

void fiddle_gamut_set_minmaxes(long *floor_x, long *floor_y, long max_tiles)
{
    struct MinMax *mm;
    long mlimit,bormul,bormuh,borinc;
    short bordec;
    long midx;
    midx = 0;
    mlimit = floor_y[0];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        mm->min = 0;
        mm->max = 0;
    }
    if (floor_y[1] <= floor_y[0])
        borinc = floor_x[0];
    else
        borinc = ((floor_x[1] - floor_x[0]) << 16) / (floor_y[1] - floor_y[0]);

    bormul = (floor_x[0] << 16);
    if (floor_y[0] < 0)
        bormul -= floor_y[0] * borinc;

    mlimit = floor_y[1];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16);
        if (bordec < -max_tiles)
            mm->min = -max_tiles;
        else
            mm->min = bordec;
        bormul += borinc;
    }

    bormul = floor_x[1] << 16;
    if (floor_y[1] < floor_y[3])
      borinc = ((floor_x[3] - floor_x[1]) << 16) / (floor_y[3] - floor_y[1]);

    mlimit = floor_y[3];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    if (midx < 0) {
        bormul -= midx * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16);
        if (bordec < -max_tiles)
            mm->min = -max_tiles;
        else
            mm->min = bordec;
        bormul += borinc;
    }
    midx = floor_y[0];
    if (floor_y[2] > floor_y[0])
        borinc = ((floor_x[2] - floor_x[0]) << 16) / (floor_y[2] - floor_y[0]);
    mlimit = floor_y[2];
    if (mlimit > MINMAX_LENGTH-1)
        mlimit = MINMAX_LENGTH-1;
    bormuh = (floor_x[0] << 16);
    if (midx < 0) {
        bormuh -= floor_y[0] * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormuh >> 16) + 1;
        if (bordec > max_tiles)
            mm->max = max_tiles;
        else
            mm->max = bordec;
        bormuh += borinc;
    }

    bormul = floor_x[2] << 16;
    if (floor_y[2] < floor_y[3])
      borinc = ((floor_x[3] - floor_x[2]) << 16) / (floor_y[3] - floor_y[2]);
    mlimit = floor_y[3];
    if (mlimit > MINMAX_LENGTH-1)
      mlimit = MINMAX_LENGTH-1;
    if ( midx < 0 ) {
        bormul -= midx * borinc;
        midx = 0;
    }

    for (; midx < mlimit; midx++)
    {
        mm = &minmaxs[midx];
        bordec = (bormul >> 16) + 1;
        if (bordec > max_tiles)
            mm->max = max_tiles;
        else
            mm->max = bordec;
        bormul += borinc;
    }
    for (; midx <= MINMAX_LENGTH-1; midx++)
    {
        mm = &minmaxs[midx];
        mm->min = 0;
        mm->max = 0;
    }
}

/** Prepares limits for tiles to be rendered.
 *
 * @param pos_x
 * @param pos_y
 */
void fiddle_gamut(long pos_x, long pos_y)
{
    struct PlayerInfo *player;
    long ewwidth,ewheight,ewzoom;
    long floor_x[4];
    long floor_y[4];
    player = get_my_player();
    switch (player->view_mode)
    {
    case PVM_CreatureView:
        fiddle_half_gamut(pos_x, pos_y, 1, cells_away);
        fiddle_half_gamut(pos_x, pos_y, -1, cells_away + 2);
        break;
    case PVM_IsometricView:
        // Retrieve coordinates on limiting map points
        ewwidth = player->engine_window_width / pixel_size;
        ewheight = player->engine_window_height / pixel_size - ((8 * high_offset[1]) >> 8);
        ewzoom = (768 * (camera_zoom/pixel_size)) >> 17;
        fiddle_gamut_find_limits(floor_x, floor_y, ewwidth, ewheight, ewzoom);
        // Place the area at proper base coords
        fiddle_gamut_set_base(floor_x, floor_y, pos_x, pos_y);
        fiddle_gamut_set_minmaxes(floor_x, floor_y, 30);
        break;
    }
}

void create_line_element(long a1, long a2, long a3, long a4, long bckt_idx, TbPixel color)
{
    struct BasicUnk13 *poly;
    if (bckt_idx >= BUCKETS_COUNT)
        bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
        bckt_idx = 0;
    poly = (struct BasicUnk13 *)getpoly;
    getpoly += sizeof(struct BasicUnk13);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_ClippedLine;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
        poly->p.field_0 = a1 / pixel_size;
        poly->p.field_4 = a2 / pixel_size;
        poly->p.field_8 = a3 / pixel_size;
        poly->p.field_C = a4 / pixel_size;
    }
    poly->p.field_10 = color;
}

void create_line_segment(struct EngineCoord *start, struct EngineCoord *end, TbPixel color)
{
    struct BasicUnk13 *poly;
    long bckt_idx;
    if (!is_free_space_in_poly_pool(1))
        return;
    // Get bucket index
    bckt_idx = (start->z+end->z)/2 / 16 - 2;
    if (bckt_idx >= BUCKETS_COUNT)
        bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
        bckt_idx = 0;
    // Add to bucket
    poly = (struct BasicUnk13 *)getpoly;
    getpoly += sizeof(struct BasicUnk13);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_ClippedLine;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    // Fill parameters
    if (pixel_size > 0)
    {
        poly->p.field_0 = start->view_width;
        poly->p.field_4 = start->view_height;
        poly->p.field_8 = end->view_width;
        poly->p.field_C = end->view_height;
    }
    poly->p.field_10 = color;
}

void create_line_const_xz(long pos_x, long pos_z, long start_y, long end_y)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_y;
    create_box_coords(&start, pos_x, start_y, pos_z);
    for (pos_y = start_y+256; pos_y <= end_y; pos_y+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_line_const_xy(long pos_x, long pos_y, long start_z, long end_z)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_z;
    create_box_coords(&start, pos_x, pos_y, start_z);
    for (pos_z = start_z+256; pos_z <= end_z; pos_z+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_line_const_yz(long pos_y, long pos_z, long start_x, long end_x)
{
    struct EngineCoord end;
    struct EngineCoord start;
    long pos_x;
    create_box_coords(&start, start_x, pos_y, pos_z);
    for (pos_x = start_x+256; pos_x <= end_x; pos_x+=256)
    {
        create_box_coords(&end, pos_x, pos_y, pos_z);
        create_line_segment(&start, &end, map_volume_box.color);
        memcpy(&start, &end, sizeof(struct EngineCoord));
    }
}

void create_map_volume_box(long x, long y, long z)
{
    long box_xs,box_xe;
    long box_ys,box_ye;
    long box_zs,box_ze;
    long i;

    box_xs = map_volume_box.field_3 - x;
    box_ys = y - map_volume_box.field_7;
    box_ye = y - map_volume_box.field_F;
    box_xe = map_volume_box.field_B - x;

    if ( temp_cluedo_mode )
        box_ze = 512 - z;
    else
        box_ze = 1280 - z;

    box_zs = (map_volume_box.field_13 << 8) - z;
    if ( box_zs >= box_ze )
      box_zs = box_ze;

    if ( box_xe < box_xs )
    {
        i = map_volume_box.field_3;
        box_xs = map_volume_box.field_B - x;
        box_xe = map_volume_box.field_3 - x;
        map_volume_box.field_3 = map_volume_box.field_B;
        map_volume_box.field_B = i;
    }

    if ( box_ye < box_ys )
    {
        i = map_volume_box.field_7;
        box_ys = y - map_volume_box.field_F;
        box_ye = y - map_volume_box.field_7;
        map_volume_box.field_7 = map_volume_box.field_F;
        map_volume_box.field_F = i;
    }

    // Draw top rectangle
    create_line_const_yz(box_ye, box_zs, box_xs, box_xe);
    create_line_const_yz(box_ys, box_zs, box_xs, box_xe);
    create_line_const_xz(box_xs, box_zs, box_ys, box_ye);
    create_line_const_xz(box_xe, box_zs, box_ys, box_ye);
    // Vertical lines which connect the rectangles
    create_line_const_xy(box_xs, box_ys, box_zs, box_ze);
    create_line_const_xy(box_xe, box_ys, box_zs, box_ze);
    create_line_const_xy(box_xe, box_ye, box_zs, box_ze);
    create_line_const_xy(box_xs, box_ye, box_zs, box_ze);
    // Bottom rectangle
    create_line_const_yz(box_ye, box_ze, box_xs, box_xe);
    create_line_const_yz(box_ys, box_ze, box_xs, box_xe);
    create_line_const_xz(box_xs, box_ze, box_ys, box_ye);
    create_line_const_xz(box_xe, box_ze, box_ys, box_ye);
}

void do_a_trig_gourad_tr(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short a4, long a5)
{
    _DK_do_a_trig_gourad_tr(ep1, ep2, ep3, a4, a5);
}

void do_a_trig_gourad_bl(struct EngineCoord *ep1, struct EngineCoord *ep2, struct EngineCoord *ep3, short a4, long a5)
{
    _DK_do_a_trig_gourad_bl(ep1, ep2, ep3, a4, a5);
}

long find_closest_lights(struct Coord3d *pos, struct NearestLights *nlgt)
{
    return _DK_find_closest_lights(pos, nlgt);
}

void create_shadows(struct Thing *thing, struct EngineCoord *ecor, struct Coord3d *pos)
{
    _DK_create_shadows(thing, ecor, pos); return;
}

void create_status_box(struct Thing *thing, struct EngineCoord *ecor)
{
    _DK_create_status_box(thing, ecor); return;
}

void do_a_plane_of_engine_columns_perspective(long stl_x, long stl_y, long plane_start, long plane_end)
{
    struct Column *blank_colmn;
    struct Column *colmn;
    struct Map *mapblk;
    struct Map *sib_mapblk;
    struct Column *sib_colmn;
    unsigned short textr_idx,height_bit;
    unsigned long center_block_idx;
    long fepos,bepos,ecpos;
    long clip_start,clip_end;
    struct CubeAttribs *texturing;
    unsigned short *cubenum_ptr;
    long i,n;
    if ((stl_y <= 0) || (stl_y >= 255))
        return;
    clip_start = plane_start;
    if (stl_x + plane_start < 1)
        clip_start = 1 - stl_x;
    clip_end = plane_end;
    if (stl_x + plane_end > 255)
        clip_end = 255 - stl_x;
    struct EngineCol *bec;
    struct EngineCol *fec;
    bec = &back_ec[clip_start + 31];
    fec = &front_ec[clip_start + 31];
    blank_colmn = get_column(game.unrevealed_column_idx);
    center_block_idx = clip_start + stl_x + (stl_y << 8);
    for (i = clip_end-clip_start; i > 0; i--)
    {
        mapblk = get_map_block_at_pos(center_block_idx);
        colmn = blank_colmn;
        if (map_block_revealed_bit(mapblk, player_bit) )
        {
            n = get_mapwho_thing_index(mapblk);
            if (n != 0)
                do_map_who(n);
            colmn = get_map_column(mapblk);
        }
        // Retrieve solidmasks for surrounding area
        unsigned short solidmsk_center,solidmsk_top,solidmsk_bottom,solidmsk_left,solidmsk_right;
        solidmsk_center = colmn->solidmask;
        solidmsk_top = blank_colmn->solidmask;
        solidmsk_right = blank_colmn->solidmask;
        solidmsk_bottom = blank_colmn->solidmask;
        solidmsk_left = blank_colmn->solidmask;
        sib_mapblk = get_map_block_at_pos(center_block_idx-256);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_top = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx+256);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_bottom = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx-1);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_left = sib_colmn->solidmask;
        }
        sib_mapblk = get_map_block_at_pos(center_block_idx+1);
        if (map_block_revealed_bit(sib_mapblk, player_bit) ) {
            sib_colmn = get_map_column(sib_mapblk);
            solidmsk_right = sib_colmn->solidmask;
        }
        bepos = 0;
        fepos = 0;
        cubenum_ptr = &colmn->cubes[0];
        height_bit = 1;
        while (height_bit <= solidmsk_center)
        {
            texturing = &game.cubes_data[*cubenum_ptr];
            if ((solidmsk_center & height_bit) != 0)
            {
              if ((solidmsk_top & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_0];
                  do_a_trig_gourad_tr(&bec[1].cors[bepos+1], &bec[0].cors[bepos+1], &bec[0].cors[bepos],   textr_idx, normal_shade_back);
                  do_a_trig_gourad_bl(&bec[0].cors[bepos],   &bec[1].cors[bepos],   &bec[1].cors[bepos+1], textr_idx, normal_shade_back);
              }
              if ((solidmsk_bottom & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_2];
                  do_a_trig_gourad_tr(&fec[0].cors[fepos+1], &fec[1].cors[fepos+1], &fec[1].cors[fepos],   textr_idx, normal_shade_front);
                  do_a_trig_gourad_bl(&fec[1].cors[fepos],   &fec[0].cors[fepos],   &fec[0].cors[fepos+1], textr_idx, normal_shade_front);
              }
              if ((solidmsk_left & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_3];
                  do_a_trig_gourad_tr(&bec[0].cors[bepos+1], &fec[0].cors[fepos+1], &fec[0].cors[fepos],   textr_idx, normal_shade_left);
                  do_a_trig_gourad_bl(&fec[0].cors[fepos],   &bec[0].cors[bepos],   &bec[0].cors[bepos+1], textr_idx, normal_shade_left);
              }
              if ((solidmsk_right & height_bit) == 0)
              {
                  textr_idx = texturing->texture_id[sideoris[0].field_1];
                  do_a_trig_gourad_tr(&fec[1].cors[fepos+1], &bec[1].cors[bepos+1], &bec[1].cors[bepos],   textr_idx, normal_shade_right);
                  do_a_trig_gourad_bl(&bec[1].cors[bepos],   &fec[1].cors[fepos],   &fec[1].cors[fepos+1], textr_idx, normal_shade_right);
              }
            }
            bepos++; fepos++;
            cubenum_ptr++;
            height_bit = height_bit << 1;
        }

        ecpos = floor_height[solidmsk_center];
        if (ecpos > 0)
        {
            cubenum_ptr = &colmn->cubes[ecpos-1];
            texturing = &game.cubes_data[*cubenum_ptr];
            textr_idx = texturing->texture_id[4];
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);
        } else
        {
            ecpos = 0;
            textr_idx = colmn->baseblock;
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);
        }
        // For tiles which have solid columns at top, draw them
        ecpos = lintel_top_height[solidmsk_center];
        if (ecpos > 0)
        {
            cubenum_ptr = &colmn->cubes[ecpos-1];
            texturing = &game.cubes_data[*cubenum_ptr];
            textr_idx = texturing->texture_id[4];
            do_a_trig_gourad_tr(&bec[0].cors[ecpos], &bec[1].cors[ecpos], &fec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&fec[1].cors[ecpos], &fec[0].cors[ecpos], &bec[0].cors[ecpos], textr_idx, -1);

            ecpos =  lintel_bottom_height[solidmsk_center];
            textr_idx = texturing->texture_id[5];
            do_a_trig_gourad_tr(&fec[0].cors[ecpos], &fec[1].cors[ecpos], &bec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&bec[1].cors[ecpos], &bec[0].cors[ecpos], &fec[0].cors[ecpos], textr_idx, -1);
        }
        // Draw the universal ceiling on top of the columns
        ecpos = 8;
        {
            textr_idx = floor_to_ceiling_map[colmn->baseblock];
            do_a_trig_gourad_tr(&fec[0].cors[ecpos], &fec[1].cors[ecpos], &bec[1].cors[ecpos], textr_idx, -1);
            do_a_trig_gourad_bl(&bec[1].cors[ecpos], &bec[0].cors[ecpos], &fec[0].cors[ecpos], textr_idx, -1);
        }
        bec++;
        fec++;
        center_block_idx++;
    }
}

long do_a_plane_of_engine_columns_sub5(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3)
{
    return _DK_do_a_plane_of_engine_columns_sub5(ec1, ec2, ec3);
}

void do_a_gpoly_gourad_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4, int a5)
{
    _DK_do_a_gpoly_gourad_tr(ec1, ec2, ec3, a4, a5); return;
}

void do_a_gpoly_unlit_tr(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4)
{
    _DK_do_a_gpoly_unlit_tr(ec1, ec2, ec3, a4); return;
}

void do_a_gpoly_unlit_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4)
{
    _DK_do_a_gpoly_unlit_bl(ec1, ec2, ec3, a4); return;
}

void do_a_gpoly_gourad_bl(struct EngineCoord *ec1, struct EngineCoord *ec2, struct EngineCoord *ec3, short a4, int a5)
{
    _DK_do_a_gpoly_gourad_bl(ec1, ec2, ec3, a4, a5); return;
}

void do_a_plane_of_engine_columns_cluedo(long stl_x, long stl_y, long plane_start, long plane_end)
{
    if ((stl_y < 1) || (stl_y > 254)) {
        return;
    }
    long xaval, xbval;
    xaval = plane_start;
    if (stl_x + plane_start < 1) {
        xaval = 1 - stl_x;
    }
    xbval = plane_end;
    if (stl_x + plane_end > 255) {
        xbval = 255 - stl_x;
    }
    int xidx, xdelta;
    xdelta = xbval - xaval;
    const struct Column *unrev_colmn;
    unrev_colmn = get_column(game.unrevealed_column_idx);
    for (xidx=0; xidx < xdelta; xidx++)
    {
        struct Map *cur_mapblk;
        cur_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y);
        // Get solidmasks of sibling columns
        unsigned short solidmsk_cur_raw, solidmsk_cur, solidmsk_back, solidmsk_front, solidmsk_left, solidmsk_right;
        solidmsk_cur_raw = unrev_colmn->solidmask;
        solidmsk_cur = unrev_colmn->solidmask & 3;
        solidmsk_back = unrev_colmn->solidmask & 3;
        solidmsk_right = unrev_colmn->solidmask & 3;
        solidmsk_front = unrev_colmn->solidmask & 3;
        solidmsk_left = unrev_colmn->solidmask & 3;
        // Get column to be drawn
        const struct Column *cur_colmn;
        cur_colmn = unrev_colmn;
        if (map_block_revealed_bit(cur_mapblk, player_bit))
        {
            long i;
            i = get_mapwho_thing_index(cur_mapblk);
            if (i > 0) {
              do_map_who(i);
            }
            cur_colmn = get_map_column(cur_mapblk);
            solidmsk_cur_raw = cur_colmn->solidmask;
            solidmsk_cur = solidmsk_cur_raw;
            if (solidmsk_cur >= (1<<3))
            {
                if (((cur_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((cur_colmn->bitfields & 0xE) == 0)) {
                    solidmsk_cur &= 3;
                }
            }
        }
        struct Map *sib_mapblk;
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y - 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_back = colmn->solidmask;
            if (solidmsk_back >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_back &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y + 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_front = colmn->solidmask;
            if (solidmsk_front >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_front &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx - 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_left = colmn->solidmask;
            if (solidmsk_left >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_left &= 3;
                }
            }
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx + 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_right = colmn->solidmask;
            if (solidmsk_right >= (1<<3))
            {
                if (((sib_mapblk->flags & (SlbAtFlg_IsDoor|SlbAtFlg_IsRoom)) == 0) && ((colmn->bitfields & 0xE) == 0)) {
                    solidmsk_right &= 3;
                }
            }
        }

        struct EngineCol *bec;
        struct EngineCol *fec;
        bec = &back_ec[xaval + 31 + xidx];
        fec = &front_ec[xaval + 31 + xidx];
        unsigned short mask;
        int ncor;
        for (mask=1,ncor=0; mask <= solidmsk_cur; mask*=2,ncor++)
        {
            unsigned short textr_id;
            struct CubeAttribs *cubed;
            cubed = &game.cubes_data[cur_colmn->cubes[ncor]];
            if ((mask & solidmsk_cur) == 0)
            {
                continue;
            }
            if ((mask & solidmsk_back) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_0];
                do_a_gpoly_gourad_tr(&bec[1].cors[ncor+1], &bec[0].cors[ncor+1], &bec[0].cors[ncor],   textr_id, normal_shade_back);
                do_a_gpoly_gourad_bl(&bec[0].cors[ncor],   &bec[1].cors[ncor],   &bec[1].cors[ncor+1], textr_id, normal_shade_back);
            }
            if ((solidmsk_front & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_2];
                do_a_gpoly_gourad_tr(&fec[0].cors[ncor+1], &fec[1].cors[ncor+1], &fec[1].cors[ncor],   textr_id, normal_shade_front);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor],   &fec[0].cors[ncor],   &fec[0].cors[ncor+1], textr_id, normal_shade_front);
            }
            if ((solidmsk_left & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_3];
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor+1], &fec[0].cors[ncor+1], &fec[0].cors[ncor],   textr_id, normal_shade_left);
                do_a_gpoly_gourad_bl(&fec[0].cors[ncor],   &bec[0].cors[ncor],   &bec[0].cors[ncor+1], textr_id, normal_shade_left);
            }
            if ((solidmsk_right & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_1];
                do_a_gpoly_gourad_tr(&fec[1].cors[ncor+1], &bec[1].cors[ncor+1], &bec[1].cors[ncor],   textr_id, normal_shade_right);
                do_a_gpoly_gourad_bl(&bec[1].cors[ncor],   &fec[1].cors[ncor],   &fec[1].cors[ncor+1], textr_id, normal_shade_right);
            }
        }

        ncor = floor_height[solidmsk_cur];
        if ((ncor > 0) && (ncor <= COLUMN_STACK_HEIGHT))
        {
            int ncor_raw;
            ncor_raw = floor_height[solidmsk_cur_raw];
            if ((cur_mapblk->flags & (SlbAtFlg_Unk80|SlbAtFlg_Unk04)) == 0)
            {
                if ((ncor_raw > 0) && (ncor_raw <= COLUMN_STACK_HEIGHT))
                {
                    struct CubeAttribs * cubed;
                    cubed = &game.cubes_data[cur_colmn->cubes[ncor_raw-1]];
                    do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
                    do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
                }
            } else
            if ((cur_mapblk->flags & SlbAtFlg_Valuable) != 0)
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 579);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 579);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 578);
            }
        } else
        {
            if ((cur_mapblk->flags & SlbAtFlg_Unk04) == 0)
            {
                do_a_gpoly_gourad_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], cur_colmn->baseblock, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], cur_colmn->baseblock, -1);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], 578);
            }
        }
        ncor = lintel_top_height[solidmsk_cur];
        if ((ncor > 0) && (ncor <= COLUMN_STACK_HEIGHT))
        {
            struct CubeAttribs * cubed;
            cubed = &game.cubes_data[cur_colmn->cubes[ncor-1]];
            do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
            do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
        }
    }
}

void do_a_plane_of_engine_columns_isometric(long stl_x, long stl_y, long plane_start, long plane_end)
{
    if ((stl_y < 1) || (stl_y > 254)) {
        return;
    }
    long xaval, xbval;
    TbBool xaclip, xbclip;
    xaval = plane_start;
    xaclip = 0;
    xbclip = 0;
    if (stl_x + plane_start < 1) {
        xaclip = 1;
        xaval = 1 - stl_x;
    }
    xbval = plane_end;
    if (stl_x + plane_end > map_subtiles_x) {
        xbclip = 1;
        xbval = map_subtiles_x - stl_x;
    }
    int xidx, xdelta;
    xdelta = xbval - xaval;
    const struct Column *unrev_colmn;
    unrev_colmn = get_column(game.unrevealed_column_idx);
    for (xidx=0; xidx < xdelta; xidx++)
    {
        struct Map *cur_mapblk;
        cur_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y);
        // Get column to be drawn
        const struct Column *cur_colmn;
        cur_colmn = unrev_colmn;
        if (map_block_revealed_bit(cur_mapblk, player_bit))
        {
            long i;
            i = get_mapwho_thing_index(cur_mapblk);
            if (i > 0) {
              do_map_who(i);
            }
            cur_colmn = get_map_column(cur_mapblk);
        }
        // Get solidmasks of sibling columns
        unsigned short solidmsk_cur, solidmsk_back, solidmsk_front, solidmsk_left, solidmsk_right;
        solidmsk_cur = cur_colmn->solidmask;
        solidmsk_back = unrev_colmn->solidmask;
        solidmsk_right = unrev_colmn->solidmask;
        solidmsk_front = unrev_colmn->solidmask;
        solidmsk_left = unrev_colmn->solidmask;
        struct Map *sib_mapblk;
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y - 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_back = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx, stl_y + 1);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_front = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx - 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_left = colmn->solidmask;
        }
        sib_mapblk = get_map_block_at(stl_x + xaval + xidx + 1, stl_y);
        if (map_block_revealed_bit(sib_mapblk, player_bit)) {
            struct Column *colmn;
            colmn = get_map_column(sib_mapblk);
            solidmsk_right = colmn->solidmask;
        }
        if ( xaclip || xbclip || (stl_y <= 1) || (stl_y >= 254))
        {
            if (xaclip && (xidx == 0)) {
                solidmsk_left = 0;
            }
            if (xbclip && (xdelta - xidx == 1)) {
                solidmsk_right = 0;
            }
            if (stl_y <= 1) {
                solidmsk_back = 0;
            }
            if (stl_y >= 254) {
                solidmsk_front = 0;
            }
        }

        struct EngineCol *bec;
        struct EngineCol *fec;
        bec = &back_ec[xaval + 31 + xidx];
        fec = &front_ec[xaval + 31 + xidx];
        unsigned short mask;
        int ncor;
        for (mask=1,ncor=0; mask <= solidmsk_cur; mask*=2,ncor++)
        {
            unsigned short textr_id;
            struct CubeAttribs *cubed;
            cubed = &game.cubes_data[cur_colmn->cubes[ncor]];
            if ((mask & solidmsk_cur) == 0)
            {
                continue;
            }
            if ((mask & solidmsk_back) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_0];
                do_a_gpoly_gourad_tr(&bec[1].cors[ncor+1], &bec[0].cors[ncor+1], &bec[0].cors[ncor],   textr_id, normal_shade_back);
                do_a_gpoly_gourad_bl(&bec[0].cors[ncor],   &bec[1].cors[ncor],   &bec[1].cors[ncor+1], textr_id, normal_shade_back);
            }
            if ((solidmsk_front & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_2];
                do_a_gpoly_gourad_tr(&fec[0].cors[ncor+1], &fec[1].cors[ncor+1], &fec[1].cors[ncor],   textr_id, normal_shade_front);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor],   &fec[0].cors[ncor],   &fec[0].cors[ncor+1], textr_id, normal_shade_front);
            }
            if ((solidmsk_left & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_3];
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor+1], &fec[0].cors[ncor+1], &fec[0].cors[ncor],   textr_id, normal_shade_left);
                do_a_gpoly_gourad_bl(&fec[0].cors[ncor],   &bec[0].cors[ncor],   &bec[0].cors[ncor+1], textr_id, normal_shade_left);
            }
            if ((solidmsk_right & mask) == 0)
            {
                textr_id = cubed->texture_id[sideoris[0].field_1];
                do_a_gpoly_gourad_tr(&fec[1].cors[ncor+1], &bec[1].cors[ncor+1], &bec[1].cors[ncor],   textr_id, normal_shade_right);
                do_a_gpoly_gourad_bl(&bec[1].cors[ncor],   &fec[1].cors[ncor],   &fec[1].cors[ncor+1], textr_id, normal_shade_right);
            }
        }

        ncor = floor_height[solidmsk_cur];
        if (ncor > 0)
        {
            if ((cur_mapblk->flags & (SlbAtFlg_Unk80|SlbAtFlg_Unk04)) == 0)
            {
                struct CubeAttribs * cubed;
                cubed = &game.cubes_data[*(short *)((char *)&cur_colmn->baseblock + 2 * ncor + 1)];
                do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
            } else
            if ((cur_mapblk->flags & SlbAtFlg_Valuable) != 0)
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 579);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 579);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], 578);
            }
        } else
        {
            if ((cur_mapblk->flags & SlbAtFlg_Unk04) == 0)
            {
                do_a_gpoly_gourad_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], cur_colmn->baseblock, -1);
                do_a_gpoly_gourad_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], cur_colmn->baseblock, -1);
            } else
            {
                do_a_gpoly_unlit_tr(&bec[0].cors[0], &bec[1].cors[0], &fec[1].cors[0], 578);
                do_a_gpoly_unlit_bl(&fec[1].cors[0], &fec[0].cors[0], &bec[0].cors[0], 578);
            }
        }
        ncor = lintel_top_height[solidmsk_cur];
        if (ncor > 0)
        {
            struct CubeAttribs * cubed;
            cubed = &game.cubes_data[*(short *)((char *)&cur_colmn->baseblock + 2 * ncor + 1)];
            do_a_gpoly_gourad_tr(&bec[0].cors[ncor], &bec[1].cors[ncor], &fec[1].cors[ncor], cubed->texture_id[4], -1);
            do_a_gpoly_gourad_bl(&fec[1].cors[ncor], &fec[0].cors[ncor], &bec[0].cors[ncor], cubed->texture_id[4], -1);
        }
    }
}

void draw_map_volume_box(long cor1_x, long cor1_y, long cor2_x, long cor2_y, long a5, unsigned char color)
{
    map_volume_box.visible = 1;
    map_volume_box.field_3 = cor1_x & 0xFFFF00;
    map_volume_box.field_7 = cor1_y & 0xFF00;
    map_volume_box.field_B = cor2_x & 0xFFFF00;
    map_volume_box.field_13 = a5;
    map_volume_box.field_F = cor2_y & 0xFFFF00;
    map_volume_box.color = color;
}

void draw_fastview_mapwho(struct Camera *cam, struct JontySpr *spr)
{
    _DK_draw_fastview_mapwho(cam, spr);
}

void draw_engine_number(struct Number *num)
{
    struct PlayerInfo *player;
    unsigned short flg_mem;
    struct TbSprite *spr;
    long val,ndigits;
    long w,h,pos_x;
    flg_mem = lbDisplay.DrawFlags;
    player = get_my_player();
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    spr = &button_sprite[71];
    w = spr->SWidth;
    h = spr->SHeight;
    if ((player->acamera->viewType == CAMERA_VIEW_EMPTY) || (player->acamera->viewType == CAMERA_VIEW_PARCHMENT))
    {
        // Count digits to be displayed
        ndigits=0;
        for (val = num->lvl; val > 0; val /= 10)
            ndigits++;
        if (ndigits > 0)
        {
            // Show the digits
            pos_x = w*(ndigits-1)/2 + num->x;
            for (val = num->lvl; val > 0; val /= 10)
            {
                spr = &button_sprite[(val%10) + 71];
                LbSpriteDrawScaled(pos_x, num->y - h, spr, w, h);
                pos_x -= w;
            }
        }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void draw_engine_room_flagpole(struct RoomFlag *rflg)
{
    struct Room *room;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    room = room_get(rflg->lvl);
    if (!room_exists(room) || !room_can_have_ensign(room->kind)) {
        return;
    }
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    const struct Camera *cam;
    cam = myplyr->acamera;
    if ((cam->viewType == CAMERA_VIEW_EMPTY) || (cam->viewType == CAMERA_VIEW_PARCHMENT))
    {
        if ( settings.field_8 )
        {
            int scale;
            int deltay;
            int height;
            scale = cam->zoom;
            if (cam->viewType == 5)
              scale = 4094;
            deltay = (scale << 7 >> 13)*units_per_pixel/16;
            height = (2 * (71 * scale) >> 13);
            LbDrawBox(rflg->x,
                      rflg->y - deltay,
                      (4 * units_per_pixel + 8) / 16,
                      (height * units_per_pixel + 8) / 16,
                      colours[3][1][0]);
            LbDrawBox(rflg->x + 2 * units_per_pixel / 16,
                      rflg->y - deltay,
                      (2 * units_per_pixel + 8) / 16,
                      (height * units_per_pixel + 8) / 16,
                      colours[1][0][0]);
        }
    }
}

/**
 * Selects index of a sprite used to show creature health flower.
 * @param thing
 */
unsigned short choose_health_sprite(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    HitPoints health, maxhealth;
    int color_idx;
    health = thing->health;
    maxhealth = cctrl->max_health;
    color_idx = (thing->owner % 5);
    if (is_neutral_thing(thing)) {
        color_idx = game.play_gameturn & 3;
    }
    if ((maxhealth <= 0) || (health <= 0))
    {
        return 88 + (8*color_idx);
    } else
    if (health >= maxhealth)
    {
        return 88 + (8*color_idx) - 7;
    } else
    {
        return 88 + (8*color_idx) - (8 * health / maxhealth);
    }
}

void draw_status_sprites(long scrpos_x, long scrpos_y, struct Thing *thing, long zoom)
{
    struct PlayerInfo *myplyr;
    const struct Camera *mycam;
    unsigned short flg_mem;
    myplyr = get_my_player();

    flg_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = 0;

    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    if ((game.flags_cd & MFlg_NoHeroHealthFlower) != 0)
    {
      if ( myplyr->thing_under_hand != thing->index )
      {
        cctrl->field_43 = game.play_gameturn;
        return;
      }
      cctrl->field_47 = 40;
    }

    short health_spridx,state_spridx;
    signed short anger_spridx;

    anger_spridx = 0;
    health_spridx = 0;
    state_spridx = 0;

    CrtrExpLevel exp;
    exp = min(cctrl->explevel,9);
    mycam = myplyr->acamera;
    if ((mycam->viewType == CAMERA_VIEW_EMPTY) || (mycam->viewType == CAMERA_VIEW_PARCHMENT))
    {
      health_spridx = choose_health_sprite(thing);
      if (is_my_player_number(thing->owner))
      {
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        cctrl = creature_control_get_from_thing(thing);
        if (cctrl->field_43 - game.play_gameturn != -1)
        {
            cctrl->field_47 = 0;
        } else
        if (cctrl->field_47 < 40)
        {
            cctrl->field_47++;
        }
        cctrl->field_43 = game.play_gameturn;
        if (cctrl->field_47 == 40)
        {
            struct StateInfo *stati;
            stati = get_creature_state_with_task_completion(thing);
            if ( !stati->field_23 )
            {
                if (anger_is_creature_livid(thing))
                {
                    stati = &states[CrSt_CreatureLeavingDungeon];
                } else
                if (creature_is_called_to_arms(thing))
                {
                    stati = &states[CrSt_ArriveAtCallToArms];
                } else
                if (creature_is_at_alarm(thing))
                {
                    stati = &states[CrSt_ArriveAtAlarm];
                } else
                if ( anger_is_creature_angry(thing) )
                {
                    stati = &states[CrSt_PersonSulkAtLair];
                } else
                if (hunger_is_creature_hungry(thing))
                {
                    stati = &states[CrSt_CreatureArrivedAtGarden];
                } else
                if (creature_requires_healing(thing))
                {
                    stati = &states[CrSt_CreatureSleep];
                } else
                if (cctrl->field_48)
                {
                    stati = &states[CrSt_CreatureWantsSalary];
                } else
                {
                    stati = get_creature_state_with_task_completion(thing);
                }
                if ((*(short *)&stati->field_26 == 1) || (thing_pointed_at == thing))
                  state_spridx = stati->sprite_idx;
                switch ( anger_get_creature_anger_type(thing) )
                {
                case AngR_NotPaid:
                    anger_spridx = 52;
                    break;
                case AngR_Hungry:
                    anger_spridx = 59;
                    break;
                case AngR_NoLair:
                    anger_spridx = 54;
                    break;
                case AngR_Other:
                    anger_spridx = 55;
                    break;
                default:
                    break;
                }
            }
        }
      }
    }
    int h_add;
    h_add = 0;
    int w, h;
    struct TbSprite *spr;
    int bs_units_per_px;
    spr = &button_sprite[70];
    bs_units_per_px = 17 * units_per_pixel / spr->SHeight;
    if ( state_spridx || anger_spridx )
    {
        spr = &button_sprite[70];
        w = (zoom * spr->SWidth * bs_units_per_px/16) >> 13;
        h = (zoom * spr->SHeight * bs_units_per_px/16) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
    }
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR8;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    if (((game.play_gameturn & 4) == 0) && (anger_spridx > 0))
    {
        spr = &button_sprite[anger_spridx];
        w = (zoom * spr->SWidth * bs_units_per_px/16) >> 13;
        h = (zoom * spr->SHeight * bs_units_per_px/16) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
        spr = &button_sprite[state_spridx];
        h_add += spr->SHeight * bs_units_per_px/16;
    } else
    if ( state_spridx )
    {
        spr = &button_sprite[state_spridx];
        w = (zoom * spr->SWidth * bs_units_per_px/16) >> 13;
        h = (zoom * spr->SHeight * bs_units_per_px/16) >> 13;
        LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h, spr, w, h);
        h_add += h;
    }
    if ((thing->word_17 > 0) && (health_spridx > 0) && ((game.play_gameturn & 1) != 0))
    {
        int flash_owner;
        if (is_neutral_thing(thing)) {
            flash_owner = game.play_gameturn & 3;
        } else {
            flash_owner = thing->owner;
        }
        spr = &button_sprite[health_spridx];
        w = (zoom * spr->SWidth * bs_units_per_px/16) >> 13;
        h = (zoom * spr->SHeight * bs_units_per_px/16) >> 13;
        LbSpriteDrawScaledOneColour(scrpos_x - w / 2, scrpos_y - h - h_add, spr, w, h, player_flash_colours[flash_owner]);
    }
    else
    {
      if ( (myplyr->thing_under_hand == thing->index)
        || ((myplyr->id_number != thing->owner) && !creature_is_invisible(thing))
        || (cctrl->combat_flags != 0)
        || (thing->word_17 > 0)
        || (mycam->viewType == CAMERA_VIEW_ISOMETRIC))
      {
          if (health_spridx > 0) {
              spr = &button_sprite[health_spridx];
              w = (zoom * spr->SWidth * bs_units_per_px/16) >> 13;
              h = (zoom * spr->SHeight * bs_units_per_px/16) >> 13;
              LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h - h_add, spr, w, h);
          }
          spr = &button_sprite[184 + exp];
          w = (zoom * spr->SWidth * bs_units_per_px/16) >> 13;
          h = (zoom * spr->SHeight * bs_units_per_px/16) >> 13;
          LbSpriteDrawScaled(scrpos_x - w / 2, scrpos_y - h - h_add, spr, w, h);
      }
    }
    lbDisplay.DrawFlags = flg_mem;
}

void draw_iso_only_fastview_mapwho(struct Camera *cam, struct JontySpr *spr)
{
    if (cam->viewType == 5)
      draw_fastview_mapwho(cam, spr);
}

#define ROOM_FLAG_PROGRESS_BAR_WIDTH 10
void draw_room_flag_top(long x, long y, int units_per_px, const struct Room *room)
{
    unsigned long flg_mem;
    flg_mem = lbDisplay.DrawFlags;
    int bar_fill;
    int bar_empty;
    struct TbSprite *spr;
    int ps_units_per_px;
    spr = &gui_panel_sprites[303];
    ps_units_per_px = 36*units_per_px/spr->SHeight;
    LbSpriteDrawScaled(x, y, spr, spr->SWidth * ps_units_per_px / 16, spr->SHeight * ps_units_per_px / 16);
    struct RoomConfigStats *roomst;
    roomst = &slab_conf.room_cfgstats[room->kind];
    int barpos_x;
    barpos_x = x + spr->SWidth * ps_units_per_px / 16 - (8 * units_per_px - 8) / 16;
    spr = &gui_panel_sprites[roomst->medsym_sprite_idx];
    LbSpriteDrawResized(x - 2*units_per_px/16, y - 4*units_per_px/16, ps_units_per_px, spr);
    bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH;
    bar_empty = 0;
    if (room->slabs_count > 0)
    {
        bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH * room->health / compute_room_max_health(room->slabs_count, room->efficiency);
        bar_empty = ROOM_FLAG_PROGRESS_BAR_WIDTH - bar_fill;
    }
    int bar_width, bar_height;
    bar_width = (2 * bar_empty * units_per_px + 8) / 16;
    // Compute height in a way which will assure covering whole bar area
    bar_height = (5 * units_per_px - 8) / 16;
    LbDrawBox(barpos_x - bar_width, y +  (8 * units_per_px + 8) / 16, bar_width, bar_height, colours[0][0][0]);
    bar_empty = 0;
    if (room->total_capacity > 0)
    {
        bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH * room->used_capacity / room->total_capacity;
        bar_empty = ROOM_FLAG_PROGRESS_BAR_WIDTH - bar_fill;
    }
    bar_width = (2 * bar_empty * units_per_px + 8) / 16;
    LbDrawBox(barpos_x - bar_width, y + (16 * units_per_px + 8) / 16, bar_width, bar_height, colours[0][0][0]);
    bar_empty = 0;
    {
        bar_fill = ROOM_FLAG_PROGRESS_BAR_WIDTH * room->efficiency / ROOM_EFFICIENCY_MAX;
        bar_empty = ROOM_FLAG_PROGRESS_BAR_WIDTH - bar_fill;
    }
    bar_width = (2 * bar_empty * units_per_px + 8) / 16;
    LbDrawBox(barpos_x - bar_width, y + (24 * units_per_px + 8) / 16, bar_width, bar_height, colours[0][0][0]);
    lbDisplay.DrawFlags = flg_mem;
}
#undef ROOM_FLAG_PROGRESS_BAR_WIDTH

void draw_engine_room_flag_top(struct RoomFlag *rflg)
{
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    struct Room *room;
    room = room_get(rflg->lvl);
    if (!room_exists(room) || !room_can_have_ensign(room->kind)) {
        return;
    }
    struct PlayerInfo *myplyr;
    myplyr = get_my_player();
    const struct Camera *cam;
    cam = myplyr->acamera;

    if ((cam->viewType == 2) || (cam->viewType == 5))
    {
        if (settings.field_8)
        {
            int scale;
            int deltay;
            scale = cam->zoom;
            if (cam->viewType == 5)
                scale = 4094;
            deltay = (scale << 7 >> 13)*units_per_pixel/16;
            draw_room_flag_top(rflg->x, rflg->y - deltay, units_per_pixel, room);
        }
    }
}

void draw_stripey_line(long a1, long a2, long a3, long a4, unsigned char a5)
{
    _DK_draw_stripey_line(a1, a2, a3, a4, a5);
}

void draw_clipped_line(long x1, long y1, long x2, long y2, TbPixel color)
{
    struct PlayerInfo *player;
    if ((x1 >= 0) || (x2 >= 0))
    {
      if ((y1 >= 0) || (y2 >= 0))
      {
        player = get_my_player();
        if ((x1 < player->engine_window_width) || (x2 < player->engine_window_width))
        {
          if ((y1 < player->engine_window_width) || (y2 < player->engine_window_width))
          {
            draw_stripey_line(x1, y1, x2, y2, color);
          }
        }
      }
    }
}

void draw_map_who(struct RotoSpr *spr)
{
    // empty
}

void draw_unkn09(struct BasicUnk09 *unk09)
{
    struct XYZ coord_a,coord_b,coord_c,coord_d,coord_e;
    struct PolyPoint point_a,point_b,point_c,point_d,point_e,
        point_f,point_g,point_h,point_i,point_j,point_k,point_l;
    vec_map = block_ptrs[unk09->block];
    switch (unk09->subtype)
    {
    case 0:
        vec_mode = VM_Unknown5;
        draw_gpoly(&unk09->p1,&unk09->p2,&unk09->p3);
        break;
    case 1:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&unk09->p1, &point_a, &unk09->p3);
        draw_gpoly(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 2:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&unk09->p1, &unk09->p2, &point_a);
        draw_gpoly(&unk09->p1, &point_a, &unk09->p3);
        break;
    case 3:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        draw_gpoly(&unk09->p1, &unk09->p2, &point_a);
        draw_gpoly(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 4:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p1, &point_a, &point_c);
        draw_gpoly(&point_a, &unk09->p2, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_c, &point_b, &unk09->p3);
        break;
    case 5:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_44) >> 1;
        coord_b.y = (coord_a.y + unk09->field_48) >> 1;
        coord_b.z = (coord_a.z + unk09->field_4C) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_b.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_50) >> 1;
        coord_c.y = (coord_a.y + unk09->field_54) >> 1;
        coord_c.z = (coord_a.z + unk09->field_58) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_c.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p1, &point_b, &unk09->p3);
        draw_gpoly(&point_b, &point_a, &unk09->p3);
        draw_gpoly(&point_a, &point_c, &unk09->p3);
        draw_gpoly(&point_c, &unk09->p2, &unk09->p3);
        break;
    case 6:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_50) >> 1;
        coord_b.y = (coord_a.y + unk09->field_54) >> 1;
        coord_b.z = (coord_a.z + unk09->field_58) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_c.y = (coord_a.y + unk09->field_60) >> 1;
        coord_c.z = (coord_a.z + unk09->field_64) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        point_c.field_10 = (point_a.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p1, &unk09->p2, &point_b);
        draw_gpoly(&unk09->p1, &point_b, &point_a);
        draw_gpoly(&unk09->p1, &point_a, &point_c);
        draw_gpoly(&unk09->p1, &point_c, &unk09->p3);
        break;
    case 7:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_b.y = (coord_a.y + unk09->field_60) >> 1;
        coord_b.z = (coord_a.z + unk09->field_64) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        point_b.field_10 = (point_a.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_44) >> 1;
        coord_c.y = (coord_a.y + unk09->field_48) >> 1;
        coord_c.z = (coord_a.z + unk09->field_4C) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        draw_gpoly(&unk09->p2, &unk09->p3, &point_b);
        draw_gpoly(&unk09->p2, &point_b, &point_a);
        draw_gpoly(&unk09->p2, &point_a, &point_c);
        draw_gpoly(&unk09->p2, &point_c, &unk09->p1);
        break;
    case 8:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_d.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_e.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&unk09->p1, &point_d, &point_c);
        draw_gpoly(&point_d, &point_a, &point_c);
        draw_gpoly(&point_a, &point_e, &point_b);
        draw_gpoly(&point_e, &unk09->p2, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_c, &point_b, &unk09->p3);
        break;
    case 9:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_d.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_d.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        point_d.field_10 = (point_b.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_e.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_e.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        point_e.field_10 = (point_b.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&unk09->p1, &point_a, &point_c);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&point_a, &unk09->p2, &point_d);
        draw_gpoly(&point_a, &point_d, &point_b);
        draw_gpoly(&point_c, &point_b, &point_e);
        draw_gpoly(&point_c, &point_e, &unk09->p3);
        break;
    case 10:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_d.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_d.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        point_d.field_10 = (point_c.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_e.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_e.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        point_e.field_10 = (point_c.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_e, &point_e);
        draw_gpoly(&point_a, &unk09->p2, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
        draw_gpoly(&unk09->p1, &point_a, &point_e);
        draw_gpoly(&point_e, &point_a, &point_c);
        draw_gpoly(&point_c, &point_b, &point_d);
        draw_gpoly(&point_d, &point_b, &unk09->p3);
        break;
    case 11:
        vec_mode = VM_Unknown5;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        point_a.field_10 = (unk09->p2.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        point_b.field_10 = (unk09->p3.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        point_c.field_10 = (unk09->p3.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        point_d.field_10 = (point_a.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        point_e.field_10 = (point_a.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_e, &point_e);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_f.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_f.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        point_f.field_10 = (point_b.field_10 + unk09->p2.field_10) >> 1;
        perspective(&coord_d, &point_f);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_g.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_g.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        point_g.field_10 = (point_b.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_e, &point_g);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_h.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_h.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        point_h.field_10 = (point_c.field_10 + unk09->p3.field_10) >> 1;
        perspective(&coord_d, &point_h);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_i.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_i.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        point_i.field_10 = (point_c.field_10 + unk09->p1.field_10) >> 1;
        perspective(&coord_e, &point_i);
        coord_d.x = (coord_a.x + coord_c.x) >> 1;
        coord_d.y = (coord_a.y + coord_c.y) >> 1;
        coord_d.z = (coord_a.z + coord_c.z) >> 1;
        point_j.field_8 = (point_a.field_8 + point_c.field_8) >> 1;
        point_j.field_C = (point_a.field_C + point_c.field_C) >> 1;
        point_j.field_10 = (point_a.field_10 + point_c.field_10) >> 1;
        perspective(&coord_d, &point_j);
        coord_e.x = (coord_a.x + coord_b.x) >> 1;
        coord_e.y = (coord_a.y + coord_b.y) >> 1;
        coord_e.z = (coord_a.z + coord_b.z) >> 1;
        point_k.field_8 = (point_a.field_8 + point_b.field_8) >> 1;
        point_k.field_C = (point_a.field_C + point_b.field_C) >> 1;
        point_k.field_10 = (point_a.field_10 + point_b.field_10) >> 1;
        perspective(&coord_e, &point_k);
        coord_d.x = (coord_b.x + coord_c.x) >> 1;
        coord_d.y = (coord_b.y + coord_c.y) >> 1;
        coord_d.z = (coord_b.z + coord_c.z) >> 1;
        point_l.field_8 = (point_b.field_8 + point_c.field_8) >> 1;
        point_l.field_C = (point_b.field_C + point_c.field_C) >> 1;
        point_l.field_10 = (point_b.field_10 + point_c.field_10) >> 1;
        perspective(&coord_d, &point_l);
        draw_gpoly(&unk09->p1, &point_d, &point_i);
        draw_gpoly(&point_d, &point_a, &point_j);
        draw_gpoly(&point_a, &point_e, &point_k);
        draw_gpoly(&point_e, &unk09->p2, &point_f);
        draw_gpoly(&point_d, &point_j, &point_i);
        draw_gpoly(&point_a, &point_k, &point_j);
        draw_gpoly(&point_e, &point_f, &point_k);
        draw_gpoly(&point_i, &point_j, &point_c);
        draw_gpoly(&point_j, &point_k, &point_l);
        draw_gpoly(&point_k, &point_f, &point_b);
        draw_gpoly(&point_j, &point_l, &point_c);
        draw_gpoly(&point_k, &point_b, &point_l);
        draw_gpoly(&point_c, &point_l, &point_h);
        draw_gpoly(&point_l, &point_b, &point_g);
        draw_gpoly(&point_l, &point_g, &point_h);
        draw_gpoly(&point_h, &point_g, &unk09->p3);
        break;
    case 12:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        trig(&unk09->p1, &unk09->p2, &unk09->p3);
        break;
    case 13:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        trig(&unk09->p1, &point_a, &unk09->p3);
        trig(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 14:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_a, &point_a);
        trig(&unk09->p1, &unk09->p2, &point_a);
        trig(&unk09->p1, &point_a, &unk09->p3);
        break;
    case 15:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        trig(&unk09->p1, &unk09->p2, &point_a);
        trig(&point_a, &unk09->p2, &unk09->p3);
        break;
    case 16:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p1, &point_a, &point_c);
        trig(&point_a, &unk09->p2, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&point_c, &point_b, &unk09->p3);
        break;
    case 17:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_44) >> 1;
        coord_b.y = (coord_a.y + unk09->field_48) >> 1;
        coord_b.z = (coord_a.z + unk09->field_4C) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_50) >> 1;
        coord_c.y = (coord_a.y + unk09->field_54) >> 1;
        coord_c.z = (coord_a.z + unk09->field_58) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p1, &point_b, &unk09->p3);
        trig(&point_b, &point_a, &unk09->p3);
        trig(&point_a, &point_c, &unk09->p3);
        trig(&point_c, &unk09->p2, &unk09->p3);
        break;
    case 18:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_50) >> 1;
        coord_b.y = (coord_a.y + unk09->field_54) >> 1;
        coord_b.z = (coord_a.z + unk09->field_58) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_c.y = (coord_a.y + unk09->field_60) >> 1;
        coord_c.z = (coord_a.z + unk09->field_64) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p1, &unk09->p2, &point_b);
        trig(&unk09->p1, &point_b, &point_a);
        trig(&unk09->p1, &point_a, &point_c);
        trig(&unk09->p1, &point_c, &unk09->p3);
        break;
    case 19:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_a.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_a.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (coord_a.x + unk09->field_5C) >> 1;
        coord_b.y = (coord_a.y + unk09->field_60) >> 1;
        coord_b.z = (coord_a.z + unk09->field_64) >> 1;
        point_b.field_8 = (point_a.field_8 + unk09->p3.field_8) >> 1;
        point_b.field_C = (point_a.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (coord_a.x + unk09->field_44) >> 1;
        coord_c.y = (coord_a.y + unk09->field_48) >> 1;
        coord_c.z = (coord_a.z + unk09->field_4C) >> 1;
        point_c.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_c.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        trig(&unk09->p2, &unk09->p3, &point_b);
        trig(&unk09->p2, &point_b, &point_a);
        trig(&unk09->p2, &point_a, &point_c);
        trig(&unk09->p2, &point_c, &unk09->p1);
        break;
    case 20:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_e, &point_e);
        trig(&unk09->p1, &point_d, &point_c);
        trig(&point_d, &point_a, &point_c);
        trig(&point_a, &point_e, &point_b);
        trig(&point_e, &unk09->p2, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&point_c, &point_b, &unk09->p3);
        break;
    case 21:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_d.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_d.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_e.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_e.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_e, &point_e);
        trig(&unk09->p1, &point_a, &point_c);
        trig(&point_a, &point_b, &point_c);
        trig(&point_a, &unk09->p2, &point_d);
        trig(&point_a, &point_d, &point_b);
        trig(&point_c, &point_b, &point_e);
        trig(&point_c, &point_e, &unk09->p3);
        break;
    case 22:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_d.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_d.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_e.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_e.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_e, &point_e);
        trig(&point_a, &unk09->p2, &point_b);
        trig(&point_a, &point_b, &point_c);
        trig(&unk09->p1, &point_a, &point_e);
        trig(&point_e, &point_a, &point_c);
        trig(&point_c, &point_b, &point_d);
        trig(&point_d, &point_b, &unk09->p3);
        break;
    case 23:
        vec_mode = VM_Unknown7;
        vec_colour = (unk09->p3.field_10 + unk09->p2.field_10 + unk09->p1.field_10) / 3 >> 16;
        coord_a.x = (unk09->field_50 + unk09->field_44) >> 1;
        coord_a.y = (unk09->field_54 + unk09->field_48) >> 1;
        coord_a.z = (unk09->field_4C + unk09->field_58) >> 1;
        point_a.field_8 = (unk09->p1.field_8 + unk09->p2.field_8) >> 1;
        point_a.field_C = (unk09->p2.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_a, &point_a);
        coord_b.x = (unk09->field_50 + unk09->field_5C) >> 1;
        coord_b.y = (unk09->field_54 + unk09->field_60) >> 1;
        coord_b.z = (unk09->field_64 + unk09->field_58) >> 1;
        point_b.field_8 = (unk09->p3.field_8 + unk09->p2.field_8) >> 1;
        point_b.field_C = (unk09->p3.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_b, &point_b);
        coord_c.x = (unk09->field_44 + unk09->field_5C) >> 1;
        coord_c.y = (unk09->field_60 + unk09->field_48) >> 1;
        coord_c.z = (unk09->field_64 + unk09->field_4C) >> 1;
        point_c.field_8 = (unk09->p1.field_8 + unk09->p3.field_8) >> 1;
        point_c.field_C = (unk09->p3.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_c, &point_c);
        coord_d.x = (coord_a.x + unk09->field_44) >> 1;
        coord_d.y = (coord_a.y + unk09->field_48) >> 1;
        coord_d.z = (coord_a.z + unk09->field_4C) >> 1;
        point_d.field_8 = (point_a.field_8 + unk09->p1.field_8) >> 1;
        point_d.field_C = (point_a.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_d, &point_d);
        coord_e.x = (coord_a.x + unk09->field_50) >> 1;
        coord_e.y = (coord_a.y + unk09->field_54) >> 1;
        coord_e.z = (coord_a.z + unk09->field_58) >> 1;
        point_e.field_8 = (point_a.field_8 + unk09->p2.field_8) >> 1;
        point_e.field_C = (point_a.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_e, &point_e);
        coord_d.x = (coord_b.x + unk09->field_50) >> 1;
        coord_d.y = (coord_b.y + unk09->field_54) >> 1;
        coord_d.z = (coord_b.z + unk09->field_58) >> 1;
        point_f.field_8 = (point_b.field_8 + unk09->p2.field_8) >> 1;
        point_f.field_C = (point_b.field_C + unk09->p2.field_C) >> 1;
        perspective(&coord_d, &point_f);
        coord_e.x = (coord_b.x + unk09->field_5C) >> 1;
        coord_e.y = (coord_b.y + unk09->field_60) >> 1;
        coord_e.z = (coord_b.z + unk09->field_64) >> 1;
        point_g.field_8 = (point_b.field_8 + unk09->p3.field_8) >> 1;
        point_g.field_C = (point_b.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_e, &point_g);
        coord_d.x = (coord_c.x + unk09->field_5C) >> 1;
        coord_d.y = (coord_c.y + unk09->field_60) >> 1;
        coord_d.z = (coord_c.z + unk09->field_64) >> 1;
        point_h.field_8 = (point_c.field_8 + unk09->p3.field_8) >> 1;
        point_h.field_C = (point_c.field_C + unk09->p3.field_C) >> 1;
        perspective(&coord_d, &point_h);
        coord_e.x = (coord_c.x + unk09->field_44) >> 1;
        coord_e.y = (coord_c.y + unk09->field_48) >> 1;
        coord_e.z = (coord_c.z + unk09->field_4C) >> 1;
        point_i.field_8 = (point_c.field_8 + unk09->p1.field_8) >> 1;
        point_i.field_C = (point_c.field_C + unk09->p1.field_C) >> 1;
        perspective(&coord_e, &point_i);
        coord_d.x = (coord_a.x + coord_c.x) >> 1;
        coord_d.y = (coord_a.y + coord_c.y) >> 1;
        coord_d.z = (coord_a.z + coord_c.z) >> 1;
        point_j.field_8 = (point_a.field_8 + point_c.field_8) >> 1;
        point_j.field_C = (point_a.field_C + point_c.field_C) >> 1;
        perspective(&coord_d, &point_j);
        coord_e.x = (coord_a.x + coord_b.x) >> 1;
        coord_e.y = (coord_a.y + coord_b.y) >> 1;
        coord_e.z = (coord_a.z + coord_b.z) >> 1;
        point_k.field_8 = (point_a.field_8 + point_b.field_8) >> 1;
        point_k.field_C = (point_a.field_C + point_b.field_C) >> 1;
        perspective(&coord_e, &point_k);
        coord_d.x = (coord_b.x + coord_c.x) >> 1;
        coord_d.y = (coord_b.y + coord_c.y) >> 1;
        coord_d.z = (coord_b.z + coord_c.z) >> 1;
        point_l.field_8 = (point_b.field_8 + point_c.field_8) >> 1;
        point_l.field_C = (point_b.field_C + point_c.field_C) >> 1;
        perspective(&coord_d, &point_l);
        trig(&unk09->p1, &point_d, &point_i);
        trig(&point_d, &point_a, &point_j);
        trig(&point_a, &point_e, &point_k);
        trig(&point_e, &unk09->p2, &point_f);
        trig(&point_d, &point_j, &point_i);
        trig(&point_a, &point_k, &point_j);
        trig(&point_e, &point_f, &point_k);
        trig(&point_i, &point_j, &point_c);
        trig(&point_j, &point_k, &point_l);
        trig(&point_k, &point_f, &point_b);
        trig(&point_j, &point_l, &point_c);
        trig(&point_k, &point_b, &point_l);
        trig(&point_c, &point_l, &point_h);
        trig(&point_l, &point_b, &point_g);
        trig(&point_l, &point_g, &point_h);
        trig(&point_h, &point_g, &unk09->p3);
        break;
    default:
        render_problems++;
        render_prob_kind = unk09->b.kind;
        break;
    }
}
void display_drawlist(void)
{
    struct PlayerInfo *player;
    const struct Camera *cam;
    union {
      struct BasicQ *b;
      struct BasicUnk00 *unk00;
      struct BasicUnk01 *unk01;
      struct BasicUnk02 *unk02;
      struct BasicUnk03 *unk03;
      struct BasicUnk04 *unk04;
      struct BasicUnk05 *unk05;
      struct BasicUnk06 *unk06;
      struct BasicUnk07 *unk07;
      struct RotoSpr *rotSpr;
      struct BasicUnk09 *unk09;
      struct BasicUnk10 *unk10;
      struct JontySpr *jonSpr;
      struct KeeperSpr *keepSpr;
      struct BasicUnk13 *unk13;
      struct BasicUnk14 *unk14;
      struct TexturedQuad *txquad;
      struct Number *number;
      struct RoomFlag *roomFlg;
    } item;
    long bucket_num;
    struct PolyPoint point_a,point_b,point_c;
    SYNCDBG(9,"Starting");
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    render_problems = 0;
    thing_pointed_at = 0;
    for (bucket_num = BUCKETS_COUNT-1; bucket_num > 0; bucket_num--)
    {
      for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
      {
        //JUSTLOG("%d",(int)item.b->kind);
        switch ( item.b->kind )
        {
        case QK_PolyTriangle:
          vec_mode = VM_Unknown5;
          vec_map = block_ptrs[item.unk00->block];
          draw_gpoly(&item.unk00->p1, &item.unk00->p2, &item.unk00->p3);
          break;
        case QK_PolyTriangleSimp:
          vec_mode = VM_Unknown7;
          vec_colour = ((item.unk01->p3.field_10 + item.unk01->p2.field_10 + item.unk01->p1.field_10)/3) >> 16;
          vec_map = block_ptrs[item.unk01->block];
          trig(&item.unk01->p1, &item.unk01->p2, &item.unk01->p3);
          break;
        case QK_PolyMode0:
          vec_mode = VM_Unknown0;
          vec_colour = item.unk02->colour;
          point_a.field_0 = item.unk02->x1;
          point_a.field_4 = item.unk02->y1;
          point_b.field_0 = item.unk02->x2;
          point_b.field_4 = item.unk02->y2;
          point_c.field_0 = item.unk02->x3;
          point_c.field_4 = item.unk02->y3;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case QK_PolyMode4:
          vec_mode = VM_Unknown4;
          vec_colour = item.unk03->colour;
          point_a.field_0 = item.unk03->x1;
          point_a.field_4 = item.unk03->y1;
          point_b.field_0 = item.unk03->x2;
          point_b.field_4 = item.unk03->y2;
          point_c.field_0 = item.unk03->x3;
          point_c.field_4 = item.unk03->y3;
          point_a.field_10 = item.unk03->vf1 << 16;
          point_b.field_10 = item.unk03->vf2 << 16;
          point_c.field_10 = item.unk03->vf3 << 16;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case QK_TrigMode2:
          vec_mode = VM_Unknown2;
          point_a.field_0 = item.unk04->x1;
          point_a.field_4 = item.unk04->y1;
          point_b.field_0 = item.unk04->x2;
          point_b.field_4 = item.unk04->y2;
          point_c.field_0 = item.unk04->x3;
          point_c.field_4 = item.unk04->y3;
          point_a.field_8 = item.unk04->uf1 << 16;
          point_a.field_C = item.unk04->vf1 << 16;
          point_b.field_8 = item.unk04->uf2 << 16;
          point_b.field_C = item.unk04->vf2 << 16;
          point_c.field_8 = item.unk04->uf3 << 16;
          point_c.field_C = item.unk04->vf3 << 16;
          trig(&point_a, &point_b, &point_c);
          break;
        case QK_PolyMode5:
          vec_mode = VM_Unknown5;
          point_a.field_0 = item.unk05->x1;
          point_a.field_4 = item.unk05->y1;
          point_b.field_0 = item.unk05->x2;
          point_b.field_4 = item.unk05->y2;
          point_c.field_0 = item.unk05->x3;
          point_c.field_4 = item.unk05->y3;
          point_a.field_8 = item.unk05->uf1 << 16;
          point_a.field_C = item.unk05->vf1 << 16;
          point_b.field_8 = item.unk05->uf2 << 16;
          point_b.field_C = item.unk05->vf2 << 16;
          point_c.field_8 = item.unk05->uf3 << 16;
          point_c.field_C = item.unk05->vf3 << 16;
          point_a.field_10 = item.unk05->wf1 << 16;
          point_b.field_10 = item.unk05->wf2 << 16;
          point_c.field_10 = item.unk05->wf3 << 16;
          draw_gpoly(&point_a, &point_b, &point_c);
          break;
        case QK_TrigMode3:
          vec_mode = VM_Unknown3;
          point_a.field_0 = item.unk06->x1;
          point_a.field_4 = item.unk06->y1;
          point_b.field_0 = item.unk06->x2;
          point_b.field_4 = item.unk06->y2;
          point_c.field_0 = item.unk06->x3;
          point_c.field_4 = item.unk06->y3;
          point_a.field_8 = item.unk06->uf1 << 16;
          point_a.field_C = item.unk06->vf1 << 16;
          point_b.field_8 = item.unk06->uf2 << 16;
          point_b.field_C = item.unk06->vf2 << 16;
          point_c.field_8 = item.unk06->uf3 << 16;
          point_c.field_C = item.unk06->vf3 << 16;
          trig(&point_a, &point_b, &point_c);
          break;
        case QK_TrigMode6:
          vec_mode = VM_Unknown6;
          point_a.field_0 = item.unk07->x1;
          point_a.field_4 = item.unk07->y1;
          point_b.field_0 = item.unk07->x2;
          point_b.field_4 = item.unk07->y2;
          point_c.field_0 = item.unk07->x3;
          point_c.field_4 = item.unk07->y3;
          point_a.field_8 = item.unk07->uf1 << 16;
          point_a.field_C = item.unk07->vf1 << 16;
          point_b.field_8 = item.unk07->uf2 << 16;
          point_b.field_C = item.unk07->vf2 << 16;
          point_c.field_8 = item.unk07->uf3 << 16;
          point_c.field_C = item.unk07->vf3 << 16;
          point_a.field_10 = item.unk07->wf1 << 16;
          point_b.field_10 = item.unk07->wf2 << 16;
          point_c.field_10 = item.unk07->wf3 << 16;
          trig(&point_a, &point_b, &point_c);
          break;
        case QK_RotableSprite:
          draw_map_who(item.rotSpr);
          break;
        case QK_Unknown9:
          draw_unkn09(item.unk09);
          break;
        case QK_Unknown10:
          vec_mode = VM_Unknown0;
          vec_colour = item.unk10->field_6;
          draw_gpoly(&item.unk10->p1, &item.unk10->p2, &item.unk10->p3);
          break;
        case QK_JontySprite:
          draw_jonty_mapwho(item.jonSpr);
          break;
        case QK_KeeperSprite:
          draw_keepsprite_unscaled_in_buffer(item.keepSpr->field_5C, item.keepSpr->field_58, item.keepSpr->field_5E, scratch);
          vec_map = scratch;
          vec_mode = VM_Unknown10;
          vec_colour = item.keepSpr->p1.field_10;
          trig(&item.keepSpr->p1, &item.keepSpr->p2, &item.keepSpr->p3);
          trig(&item.keepSpr->p1, &item.keepSpr->p3, &item.keepSpr->p4);
          break;
        case QK_ClippedLine:
          draw_clipped_line(item.unk13->p.field_0,item.unk13->p.field_4,item.unk13->p.field_8,item.unk13->p.field_C,item.unk13->p.field_10);
          break;
        case QK_StatusSprites:
          player = get_my_player();
          cam = player->acamera;
          if (cam != NULL)
          {
              if ((cam->viewType == 2) || (cam->viewType == 5)) {
                  // Status sprites grow smaller slower than zoom
                  int status_zoom;
                  status_zoom = (camera_zoom+CAMERA_ZOOM_MAX)/2;
                  draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, status_zoom*16/units_per_pixel);
              }
          }
          break;
        case QK_IntegerValue:
          draw_engine_number(item.number);
          break;
        case QK_RoomFlagPole:
          draw_engine_room_flagpole(item.roomFlg);
          break;
        case QK_JontyISOSprite:
          player = get_my_player();
          cam = player->acamera;
          if (cam != NULL)
          {
            if (cam->viewType == 2)
              draw_jonty_mapwho(item.jonSpr);
          }
          break;
        case QK_RoomFlagTop:
          draw_engine_room_flag_top(item.roomFlg);
          break;
        default:
          render_problems++;
          render_prob_kind = item.b->kind;
          break;
        }
      }
    }
    if (render_problems > 0)
      WARNLOG("Encoured %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
}

void prepare_draw_plane_of_engine_columns(long aposc, long bposc, long xcell, long ycell, struct MinMax *mm)
{
    apos = aposc;
    bpos = bposc;
    back_ec = &ecs1[0];
    front_ec = &ecs2[0];
    if (lens_mode != 0)
    {
        fill_in_points_perspective(xcell, ycell, mm);
    } else
    if (settings.video_cluedo_mode)
    {
        fill_in_points_cluedo(xcell, ycell, mm);
    } else
    {
        fill_in_points_isometric(xcell, ycell, mm);
    }
}

/**
 * Draws single plane of engine columns.
 *
 * @param aposc
 * @param bposc
 * @param xcell
 * @param ycell
 */
void draw_plane_of_engine_columns(long aposc, long bposc, long xcell, long ycell, struct MinMax *mm)
{
    struct EngineCol *ec;
    ec = front_ec;
    front_ec = back_ec;
    back_ec = ec;
    apos = aposc;
    bpos = bposc;
    if (lens_mode != 0)
    {
        fill_in_points_perspective(xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_perspective(xcell, ycell, mm->min, mm->max);
        }
    } else
    if ( settings.video_cluedo_mode )
    {
        fill_in_points_cluedo(xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_cluedo(xcell, ycell, mm->min, mm->max);
        }
    } else
    {
        fill_in_points_isometric(xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_isometric(xcell, ycell, mm->min, mm->max);
        }
    }
}

/**
 * Draws rectangular area of engine columns.
 * @param aposc
 * @param bposc
 * @param xcell
 * @param ycell
 */
void draw_view_map_plane(long aposc, long bposc, long xcell, long ycell)
{
    struct MinMax *mm;
    long i;
    i = 31-cells_away;
    if (i < 0)
        i = 0;
    mm = &minmaxs[i];
    prepare_draw_plane_of_engine_columns(aposc, bposc, xcell, ycell, mm);
    for (i = 2*cells_away-1; i > 0; i--)
    {
        ycell++;
        bposc -= (map_subtiles_y+1);
        mm++;
        draw_plane_of_engine_columns(aposc, bposc, xcell, ycell, mm);
    }
}

void draw_view(struct Camera *cam, unsigned char a2)
{
    long zoom_mem;
    long x,y,z;
    long xcell,ycell;
    long i;
    long aposc,bposc;
    SYNCDBG(9,"Starting");
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    zoom_mem = cam->zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam->zoom = camera_zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    getpoly = poly_pool;
    LbMemorySet(buckets, 0, sizeof(buckets));
    LbMemorySet(poly_pool, 0, sizeof(poly_pool));
    if (map_volume_box.visible) {
        poly_pool_end_reserve(14);
    } else {
        poly_pool_end_reserve(4);
    }
    i = lens_mode;
    if ((i < 0) || (i >= PERS_ROUTINES_COUNT))
        i = 0;
    perspective = perspective_routines[i];
    rotpers = rotpers_routines[i];
    update_fade_limits(cells_away);
    init_coords_and_rotation(&object_origin,&camera_matrix);
    rotate_base_axis(&camera_matrix, cam->orient_a, 2);
    update_normal_shade(&camera_matrix);
    rotate_base_axis(&camera_matrix, -cam->orient_b, 1);
    rotate_base_axis(&camera_matrix, -cam->orient_c, 3);
    cam_map_angle = cam->orient_a;
    map_roll = cam->orient_c;
    map_tilt = -cam->orient_b;
    x = cam->mappos.x.val;
    y = cam->mappos.y.val;
    z = cam->mappos.z.val;
    frame_wibble_generate();
    view_alt = z;
    if (lens_mode != 0)
    {
        cells_away = max_i_can_see;
        update_fade_limits(cells_away);
        fade_range = (fade_max - fade_min) >> 8;
        setup_rotate_stuff(x, y, z, fade_max, fade_min, lens, cam_map_angle, map_roll);
    } else
    {
        fade_min = 1000000;
        setup_rotate_stuff(x, y, z, fade_max, fade_min, camera_zoom/pixel_size, cam_map_angle, map_roll);
        do_perspective_rotation(x, y, z);
        cells_away = compute_cells_away();
    }
    xcell = (x >> 8);
    aposc = -(x & 0xFF);
    bposc = (cells_away << 8) + (y & 0xFF);
    ycell = (y >> 8) - (cells_away+1);
    find_gamut();
    fiddle_gamut(xcell, ycell + (cells_away+1));
    draw_view_map_plane(aposc, bposc, xcell, ycell);
    if (map_volume_box.visible) {
        poly_pool_end_reserve(0);
        create_map_volume_box(x, y, z);
    }
    cam->zoom = zoom_mem;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    display_drawlist();
    map_volume_box.visible = 0;
    SYNCDBG(9,"Finished");
}

void clear_fast_bucket_list(void)
{
    getpoly = poly_pool;
    LbMemorySet(buckets, 0, sizeof(buckets));
}

void draw_texturedquad_block(struct TexturedQuad *txquad)
{
    if (!UseFastBlockDraw)
    {
        struct PolyPoint point_a;
        struct PolyPoint point_b;
        struct PolyPoint point_c;
        struct PolyPoint point_d;
        vec_mode = VM_Unknown5;
        switch (txquad->field_2A)
        {
        case 0:
            vec_map = block_ptrs[578];
            break;
        case 1:
            vec_map = block_ptrs[579];
            break;
        case 3:
        default:
            vec_map = block_ptrs[txquad->field_6];
            break;
        }
        point_a.field_0 = (txquad->field_A >> 8) / pixel_size;
        point_a.field_4 = (txquad->field_E >> 8) / pixel_size;
        point_a.field_8 = orient_to_mapU1[txquad->field_5];
        point_a.field_C = orient_to_mapV1[txquad->field_5];
        point_a.field_10 = txquad->field_1A;
        point_d.field_0 = ((txquad->field_12 + txquad->field_A) >> 8) / pixel_size;
        point_d.field_4 = (txquad->field_E >> 8) / pixel_size;
        point_d.field_8 = orient_to_mapU2[txquad->field_5];
        point_d.field_C = orient_to_mapV2[txquad->field_5];
        point_d.field_10 = txquad->field_1E;
        point_b.field_0 = ((txquad->field_12 + txquad->field_A) >> 8) / pixel_size;
        point_b.field_4 = ((txquad->field_16 + txquad->field_E) >> 8) / pixel_size;
        point_b.field_8 = orient_to_mapU3[txquad->field_5];
        point_b.field_C = orient_to_mapV3[txquad->field_5];
        point_b.field_10 = txquad->field_22;
        point_c.field_0 = (txquad->field_A >> 8) / pixel_size;
        point_c.field_4 = ((txquad->field_16 + txquad->field_E) >> 8) / pixel_size;
        point_c.field_8 = orient_to_mapU4[txquad->field_5];
        point_c.field_C = orient_to_mapV4[txquad->field_5];
        point_c.field_10 = txquad->field_26;
        draw_gpoly(&point_a, &point_d, &point_b);
        draw_gpoly(&point_a, &point_b, &point_c);
    } else
    {
        struct GtBlock gtb;
        switch (txquad->field_2A)
        {
        case 0:
            gtb.field_0 = block_ptrs[578];
            gtb.field_C = txquad->field_1A >> 16;
            gtb.field_10 = txquad->field_1E >> 16;
            gtb.field_18 = txquad->field_22 >> 16;
            gtb.field_14 = txquad->field_26 >> 16;
            break;
        case 1:
            gtb.field_0 = block_ptrs[579];
            gtb.field_C = txquad->field_1A >> 16;
            gtb.field_10 = txquad->field_1E >> 16;
            gtb.field_18 = txquad->field_22 >> 16;
            gtb.field_14 = txquad->field_26 >> 16;
            break;
        case 3:
            gtb.field_0 = block_ptrs[txquad->field_6];
            gtb.field_C = txquad->field_1A >> 16;
            gtb.field_10 = txquad->field_1E >> 16;
            gtb.field_18 = txquad->field_22 >> 16;
            gtb.field_14 = txquad->field_26 >> 16;
            break;
        default:
            gtb.field_0 = block_ptrs[txquad->field_6];
            break;
        }
        gtb.field_4 = (txquad->field_A >> 8) / pixel_size;
        gtb.field_8 = (txquad->field_E >> 8) / pixel_size;
        gtb.field_1C = orient_table_xflip[txquad->field_5];
        gtb.field_20 = orient_table_yflip[txquad->field_5];
        gtb.field_24 = orient_table_rotate[txquad->field_5];
        gtb.field_28 = (txquad->field_12 >> 8) / pixel_size >> 5;
        gtb.field_2C = (txquad->field_16 >> 8) / pixel_size >> 4;
        gtblock_draw(&gtb);
    }
}

void display_fast_drawlist(struct Camera *cam)
{
    int bucket_num;
    union {
      struct BasicQ *b;
      struct BasicUnk00 *unk00;
      struct BasicUnk01 *unk01;
      struct BasicUnk02 *unk02;
      struct BasicUnk03 *unk03;
      struct BasicUnk04 *unk04;
      struct BasicUnk05 *unk05;
      struct BasicUnk06 *unk06;
      struct BasicUnk07 *unk07;
      struct RotoSpr *rotSpr;
      struct BasicUnk09 *unk09;
      struct BasicUnk10 *unk10;
      struct JontySpr *jonSpr;
      struct KeeperSpr *unk12;
      struct BasicUnk13 *unk13;
      struct BasicUnk14 *unk14;
      struct TexturedQuad *txquad;
      struct Number *number;
      struct RoomFlag *roomFlg;
    } item;
    // Color rendering array pointers used by draw_keepersprite()
    render_fade_tables = pixmap.fade_tables;
    render_ghost = pixmap.ghost;
    render_alpha = (unsigned char *)&alpha_sprite_table;
    render_problems = 0;
    thing_pointed_at = 0;
    for (bucket_num = BUCKETS_COUNT-1; bucket_num >= 0; bucket_num--)
    {
        for (item.b = buckets[bucket_num]; item.b != NULL; item.b = item.b->next)
        {
            switch (item.b->kind)
            {
            case QK_JontySprite:
                draw_fastview_mapwho(cam, item.jonSpr);
                break;
            case QK_ClippedLine:
                draw_clipped_line(item.unk13->p.field_0,item.unk13->p.field_4,item.unk13->p.field_8,item.unk13->p.field_C,item.unk13->p.field_10);
                break;
            case QK_StatusSprites:
                if (pixel_size == 1)
                    draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, 48*256);
                else
                    draw_status_sprites(item.unk14->field_C, item.unk14->field_10, item.unk14->thing, 16*256);
                break;
            case QK_TextureQuad:
                draw_texturedquad_block(item.txquad);
                break;
            case QK_IntegerValue:
                draw_engine_number(item.number);
                break;
            case QK_RoomFlagPole:
                draw_engine_room_flagpole(item.roomFlg);
                break;
            case QK_JontyISOSprite:
                draw_iso_only_fastview_mapwho(cam, item.jonSpr);
                break;
            case QK_RoomFlagTop:
                draw_engine_room_flag_top(item.roomFlg);
                break;
            default:
                render_problems++;
                render_prob_kind = item.b->kind;
                break;
            }
        }
    } // end for(bucket_num...
    if (render_problems > 0) {
        WARNLOG("Encoured %lu rendering problems; last was with poly kind %ld",render_problems,render_prob_kind);
    }
}

long convert_world_coord_to_front_view_screen_coord(struct Coord3d *pos, struct Camera *cam, long *x, long *y, long *z)
{
    return _DK_convert_world_coord_to_front_view_screen_coord(pos, cam, x, y, z);
}

void add_unkn11_to_polypool(struct Thing *thing, long scr_x, long scr_y, long a4, long bckt_idx)
{
    struct JontySpr *poly;
    if (bckt_idx >= BUCKETS_COUNT)
        bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
        bckt_idx = 0;
    poly = (struct JontySpr *)getpoly;
    getpoly += sizeof(struct JontySpr);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_JontySprite;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    poly->thing = thing;
    if (pixel_size > 0)
    {
        poly->scr_x = scr_x / pixel_size;
        poly->scr_y = scr_y / pixel_size;
    }
    poly->field_14 = a4;
}

void add_unkn18_to_polypool(struct Thing *thing, long scr_x, long scr_y, long a4, long bckt_idx)
{
    struct JontySpr *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct JontySpr *)getpoly;
    getpoly += sizeof(struct JontySpr);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_JontyISOSprite;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    poly->thing = thing;
    if (pixel_size > 0)
    {
      poly->scr_x = scr_x / pixel_size;
      poly->scr_y = scr_y / pixel_size;
    }
    poly->field_14 = a4;
}

void create_status_box_element(struct Thing *thing, long a2, long a3, long a4, long bckt_idx)
{
    struct BasicUnk14 *poly;
    if (bckt_idx >= BUCKETS_COUNT) {
      bckt_idx = BUCKETS_COUNT-1;
    } else
    if (bckt_idx < 0) {
      bckt_idx = 0;
    }
    poly = (struct BasicUnk14 *)getpoly;
    getpoly += sizeof(struct BasicUnk14);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_StatusSprites;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    poly->thing = thing;
    if (pixel_size > 0)
    {
        poly->field_C = a2 / pixel_size;
        poly->field_10 = a3 / pixel_size;
    }
    poly->field_14 = a4;
}

void create_fast_view_status_box(struct Thing *thing, long x, long y)
{
    create_status_box_element(thing, x, y - (shield_offset[thing->model]+thing->clipbox_size_yz) / 12, y, 1);
}

void add_textruredquad_to_polypool(long x, long y, long texture_idx, long a7, long a8, long lightness, long a9, long bckt_idx)
{
    struct TexturedQuad *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct TexturedQuad *)getpoly;
    getpoly += sizeof(struct TexturedQuad);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_TextureQuad;
    buckets[bckt_idx] = (struct BasicQ *)poly;

    poly->field_6 = texture_idx;
    poly->field_A = x;
    poly->field_E = y;
    poly->field_12 = a7;
    poly->field_16 = a7;
    poly->field_5 = a8;
    poly->field_1A = lightness;
    poly->field_1E = lightness;
    poly->field_22 = lightness;
    poly->field_26 = lightness;
    poly->field_2A = a9;
}

void add_lgttextrdquad_to_polypool(long x, long y, long texture_idx, long a6, long a7, long a8, long lg0, long lg1, long lg2, long lg3, long bckt_idx)
{
    struct TexturedQuad *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct TexturedQuad *)getpoly;
    getpoly += sizeof(struct TexturedQuad);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_TextureQuad;
    buckets[bckt_idx] = (struct BasicQ *)poly;

    poly->field_6 = texture_idx;
    poly->field_A = x;
    poly->field_E = y;
    poly->field_12 = a6;
    poly->field_16 = a7;
    poly->field_5 = a8;
    poly->field_1A = lg0;
    poly->field_1E = lg1;
    poly->field_22 = lg2;
    poly->field_26 = lg3;
    poly->field_2A = 3;
}

void add_unkn16_to_polypool(long x, long y, long lvl, long bckt_idx)
{
    struct Number *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct Number *)getpoly;
    getpoly += sizeof(struct Number);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_IntegerValue;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
      poly->x = x / pixel_size;
      poly->y = y / pixel_size;
    }
    poly->lvl = lvl;
}

void add_room_flag_pole_to_polypool(long x, long y, long room_idx, long bckt_idx)
{
    struct RoomFlag *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct RoomFlag *)getpoly;
    getpoly += sizeof(struct RoomFlag);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_RoomFlagPole;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
      poly->x = x / pixel_size;
      poly->y = y / pixel_size;
    }
    poly->lvl = room_idx;
}

void add_room_flag_top_to_polypool(long x, long y, long room_idx, long bckt_idx)
{
    struct RoomFlag *poly;
    if (bckt_idx >= BUCKETS_COUNT)
      bckt_idx = BUCKETS_COUNT-1;
    else
    if (bckt_idx < 0)
      bckt_idx = 0;
    poly = (struct RoomFlag *)getpoly;
    getpoly += sizeof(struct RoomFlag);
    poly->b.next = buckets[bckt_idx];
    poly->b.kind = QK_RoomFlagTop;
    buckets[bckt_idx] = (struct BasicQ *)poly;
    if (pixel_size > 0)
    {
      poly->x = x / pixel_size;
      poly->y = y / pixel_size;
    }
    poly->lvl = room_idx;
}

void prepare_lightness_intensity_array(long stl_x, long stl_y, long *arrp, long base_lightness)
{
    long i,n;
    n = 4 * stl_x + 17 * stl_y;
    for (i=0; i < 9; i++)
    {
        long rndi,nval;
        if ((base_lightness <= 256) || (base_lightness > 15872))
        {
            nval = base_lightness;
        } else
        {
            rndi = randomisors[n&RANDOMISORS_MASK];
            n++;
            nval = 32 * (rndi & 0x3F) + base_lightness - 256;
        }
        *arrp = nval << 8;
        arrp++;
    }
}

void draw_element(struct Map *map, long lightness, long stl_x, long stl_y, long pos_x, long pos_y, long a7, unsigned char a8, long *ymax)
{
    struct PlayerInfo *myplyr;
    TbBool sibrevealed[3][3];
    struct CubeAttribs *unkstrcp;
    struct Map *mapblk;
    long lightness_arr[4][9];
    long bckt_idx;
    long cube_itm,delta_y;
    long tc; // top cube index
    long x,y;
    long i;
    myplyr = get_my_player();
    cube_itm = (a8 + 2) & 3;
    delta_y = (a7 << 7) / 256;
    bckt_idx = myplyr->engine_window_height - (pos_y >> 8) + 64;
    // Check if there's enough place to draw
    if (!is_free_space_in_poly_pool(8))
      return;

    // Prepare light intensity array

    for (y=0; y < 3; y++)
        for (x=0; x < 3; x++)
        {
            sibrevealed[y][x] = subtile_revealed(stl_x+x-1, stl_y+y-1, myplyr->id_number);
        }

    i = 0;
    if (sibrevealed[0][1] && sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[0][0])
        i = lightness;
    prepare_lightness_intensity_array(stl_x,stl_y,lightness_arr[(-a8) & 3],i);

    i = 0;
    if (sibrevealed[0][1] && sibrevealed[0][2] && sibrevealed[1][2] && sibrevealed[1][1])
        i = get_subtile_lightness(&game.lish,stl_x+1,stl_y);
    prepare_lightness_intensity_array(stl_x+1,stl_y,lightness_arr[(1-a8) & 3],i);

    i = 0;
    if (sibrevealed[1][0] && sibrevealed[1][1] && sibrevealed[2][0] && sibrevealed[2][1])
        i = get_subtile_lightness(&game.lish,stl_x,stl_y+1);
    prepare_lightness_intensity_array(stl_x,stl_y+1,lightness_arr[(-1-a8) & 3],i);

    i = 0;
    if (sibrevealed[2][2] && sibrevealed[1][2] && sibrevealed[1][1] && sibrevealed[2][1])
        i = get_subtile_lightness(&game.lish,stl_x+1,stl_y+1);
    prepare_lightness_intensity_array(stl_x+1,stl_y+1,lightness_arr[(-2-a8) & 3],i);

    // Get column to be drawn on the current subtile

    struct Column *col;
    if (map_block_revealed_bit(map, player_bit))
      i = get_mapblk_column_index(map);
    else
      i = game.unrevealed_column_idx;
    col = get_column(i);
    mapblk = get_map_block_at(stl_x, stl_y);

    // Draw the columns base block

    if (*ymax > pos_y)
    {
      if ((col->baseblock != 0) && (col->cubes[0] == 0))
      {
          *ymax = pos_y;
          if ((mapblk->flags & SlbAtFlg_Unk04) != 0)
          {
              add_textruredquad_to_polypool(pos_x, pos_y, col->baseblock, a7, 0,
                  2097152, 0, bckt_idx);
          } else
          {
              add_lgttextrdquad_to_polypool(pos_x, pos_y, col->baseblock, a7, a7, 0,
                  lightness_arr[0][0], lightness_arr[1][0], lightness_arr[2][0], lightness_arr[3][0], bckt_idx);
          }
      }
    }

    // Draw the columns cubes

    y = a7 + pos_y;
    unkstrcp = NULL;
    for (tc=0; tc < COLUMN_STACK_HEIGHT; tc++)
    {
      if (col->cubes[tc] == 0)
        break;
      y -= delta_y;
      unkstrcp = &game.cubes_data[col->cubes[tc]];
      if (*ymax > y)
      {
        *ymax = y;
        add_lgttextrdquad_to_polypool(pos_x, y, unkstrcp->texture_id[cube_itm], a7, delta_y, 0,
            lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
      }
    }

    if (unkstrcp != NULL)
    {
      i = y - a7;
      if (*ymax > i)
      {
        *ymax = i;
        if ((mapblk->flags & SlbAtFlg_Unk80) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a8,
              2097152, 1, bckt_idx);
        } else
        if ((mapblk->flags & SlbAtFlg_Unk04) != 0)
        {
          add_textruredquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a8,
              2097152, 0, bckt_idx);
        } else
        {
          add_lgttextrdquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a7, a8,
              lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
        }
      }
    }

    // If there are still some solid cubes higher than tc
    if ((get_column_ceiling_filled_subtiles(col) != 0) && (col->solidmask > (1 << tc)))
    {
        // Find any top cube separated by empty space
        for (;tc < COLUMN_STACK_HEIGHT; tc++)
        {
            if (col->cubes[tc] != 0)
              break;
            y -= delta_y;
        }

        for (;tc < COLUMN_STACK_HEIGHT; tc++)
        {
            if (col->cubes[tc] == 0)
              break;
            y -= delta_y;
            unkstrcp = &game.cubes_data[col->cubes[tc]];
            if (*ymax > y)
            {
              add_lgttextrdquad_to_polypool(pos_x, y, unkstrcp->texture_id[cube_itm], a7, delta_y, 0,
                  lightness_arr[3][tc+1], lightness_arr[2][tc+1], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
            }
        }
        if (unkstrcp != NULL)
        {
          i = y - a7;
          if (*ymax > i)
          {
            add_lgttextrdquad_to_polypool(pos_x, i, unkstrcp->texture_id[4], a7, a7, a8,
                lightness_arr[0][tc], lightness_arr[1][tc], lightness_arr[2][tc], lightness_arr[3][tc], bckt_idx);
          }
        }
    }

}

unsigned short get_thing_shade(struct Thing *thing)
{
    MapSubtlCoord stl_x,stl_y;
    long lgh[2][2]; // the dimensions are lgh[y][x]
    long shval;
    long fract_x,fract_y;
    stl_x = thing->mappos.x.stl.num;
    stl_y = thing->mappos.y.stl.num;
    fract_x = thing->mappos.x.stl.pos;
    fract_y = thing->mappos.y.stl.pos;
    lgh[0][0] = get_subtile_lightness(&game.lish,stl_x,  stl_y);
    lgh[0][1] = get_subtile_lightness(&game.lish,stl_x+1,stl_y);
    lgh[1][0] = get_subtile_lightness(&game.lish,stl_x,  stl_y+1);
    lgh[1][1] = get_subtile_lightness(&game.lish,stl_x+1,stl_y+1);
    shval = (fract_x
        * (lgh[0][1] + (fract_y * (lgh[1][1] - lgh[0][1]) >> 8)
        - (lgh[0][0] + (fract_y * (lgh[1][0] - lgh[0][0]) >> 8))) >> 8)
        + (lgh[0][0] + (fract_y * (lgh[1][0] - lgh[0][0]) >> 8));
    if (shval < MINIMUM_LIGHTNESS)
    {
        shval += (MINIMUM_LIGHTNESS>>2);
        if (shval > MINIMUM_LIGHTNESS)
            shval = MINIMUM_LIGHTNESS;
    } else
    {
        // Max lightness value - make sure it won't exceed our limits
        if (shval > 64*256+255)
            shval = 64*256+255;
    }
    return shval;
}

void lock_keepersprite(unsigned short kspr_idx)
{
    int frame_num,frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->rotable) {
        frame_count = 5 * kspr_arr->frames;
    } else {
        frame_count = kspr_arr->frames;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        struct HeapMgrHandle *hmhndl;
        hmhndl = heap_handle[kspr_idx+frame_num];
        if (hmhndl != NULL) {
            hmhndl->flags |= 0x02;
        }
    }
}

void unlock_keepersprite(unsigned short kspr_idx)
{
    int frame_num,frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->rotable) {
        frame_count = 5 * kspr_arr->frames;
    } else {
        frame_count = kspr_arr->frames;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        struct HeapMgrHandle *hmhndl;
        hmhndl = heap_handle[kspr_idx+frame_num];
        if (hmhndl != NULL) {
            hmhndl->flags &= ~0x02;
        }
    }
}

long load_single_frame(unsigned short kspr_idx)
{
    long nlength;
    struct HeapMgrHandle *nitem;
    int i;
    nlength = creature_table[kspr_idx+1].foffset - creature_table[kspr_idx].foffset;
    for (i=0; i < 100; i++)
    {
        nitem = heapmgr_add_item(graphics_heap, nlength);
        if (nitem != NULL) {
            break;
        }
        long idfreed;
        idfreed = heapmgr_free_oldest(graphics_heap);
        if (idfreed < 0)
        {
            // If can't free anything more, try to defrag existing items
            heapmgr_complete_defrag(graphics_heap);
            nitem = heapmgr_add_item(graphics_heap, nlength);
            if (nitem != NULL) {
                break;
            }
            // Cannot free and cannot defrag - we can't do anything more
            ERRORLOG("Unable to find freeable item");
            return 0;
        }
        heap_handle[idfreed] = NULL;
        keepsprite[idfreed] = NULL;
    }
    if (nitem == NULL) {
        ERRORLOG("Unable to make room for new item on heap");
        return 0;
    }
    if (!read_heap_item(nitem, creature_table[kspr_idx].foffset, nlength))
    {
        ERRORLOG("Load Fail On KeepSprite %d",(int)kspr_idx);
        return 0;
    }
    keepsprite[kspr_idx] = (unsigned char **)&nitem->buf;
    heap_handle[kspr_idx] = nitem;
    nitem->idx = kspr_idx;
    return 1;
}

long load_keepersprite_if_needed(unsigned short kspr_idx)
{
    int frame_num,frame_count;
    struct KeeperSprite *kspr_arr;
    kspr_arr = &creature_table[kspr_idx];
    if (kspr_arr->rotable) {
        frame_count = 5 * kspr_arr->frames;
    } else {
        frame_count = kspr_arr->frames;
    }
    for (frame_num=0; frame_num < frame_count; frame_num++)
    {
        struct HeapMgrHandle **hmhndl;
        hmhndl = &heap_handle[kspr_idx+frame_num];
        if ((*hmhndl) != NULL)
        {
            heapmgr_make_newest(graphics_heap, *hmhndl);
        } else
        {
            if ( !load_single_frame(kspr_idx+frame_num) )
            {
                return 0;
            }
            (*hmhndl)->flags |= 0x02;
        }
    }
    return 1;
}

long heap_manage_keepersprite(unsigned short kspr_idx)
{
    long result;
    lock_keepersprite(kspr_idx);
    result = load_keepersprite_if_needed(kspr_idx);
    unlock_keepersprite(kspr_idx);
    return result;
}

void draw_keepersprite(long x, long y, long w, long h, long kspr_idx)
{
    struct TbSprite sprite;
    long cut_w,cut_h;
    TbSpriteData *kspr_item;
    if ((kspr_idx < 0) || (kspr_idx >= KEEPSPRITE_LENGTH)) {
        WARNDBG(9,"Invalid KeeperSprite %ld at (%ld,%ld) size (%ld,%ld) alpha %d",kspr_idx,x,y,w,h,(int)EngineSpriteDrawUsingAlpha);
        return;
    }
    SYNCDBG(17,"Drawing %ld at (%ld,%ld) size (%ld,%ld) alpha %d",kspr_idx,x,y,w,h,(int)EngineSpriteDrawUsingAlpha);
    cut_w = w;
    cut_h = h - water_source_cutoff;
    if (cut_h <= 0) {
        return;
    }
    kspr_item = keepsprite[kspr_idx];
    sprite.SWidth = cut_w;
    sprite.SHeight = cut_h;
    if (kspr_item != NULL) {
        sprite.Data = *kspr_item;
    } else {
        sprite.Data = NULL;
    }
    if (sprite.Data == NULL) {
        WARNDBG(9,"Unallocated KeeperSprite %ld can't be drawn at (%ld,%ld)",kspr_idx,x,y);
        return;
    }
    if ( EngineSpriteDrawUsingAlpha ) {
        DrawAlphaSpriteUsingScalingData(x, y, &sprite);
    } else {
        LbSpriteDrawUsingScalingData(x, y, &sprite);
    }
    SYNCDBG(18,"Finished");
}

void set_thing_pointed_at(struct Thing *thing)
{
    if (thing_pointed_at == NULL) {
        thing_pointed_at = thing;
    }
}

void draw_single_keepersprite_omni_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    src_dy = (long)kspr->field_7;
    src_dx = (long)kspr->field_6;
    x = src_dx - (long)kspr->field_A - (long)kspr->field_4;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(x, y, kspr->field_4, kspr->field_5, kspr_idx);
}

void draw_single_keepersprite_omni(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    src_dy = (long)kspr->field_7;
    src_dx = (long)kspr->field_6;
    x = kspr->field_A;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x;
        sp_y = kspos_y;
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(x, y, kspr->field_4, kspr->field_5, kspr_idx);
}

void draw_single_keepersprite_xflip(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    SYNCDBG(18,"Starting");
    src_dy = (long)kspr->field_5;
    src_dx = (long)kspr->field_4;
    x = (long)kspr->field_6 - (long)kspr->field_A - src_dx;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
      if ( (pointer_x >= sp_x) && (pointer_x <= sp_dx + sp_x) )
      {
          if ( (pointer_y >= sp_y) && (pointer_y <= sp_dy + sp_y) )
          {
              set_thing_pointed_at(thing_being_displayed);
          }
      }
    }
    draw_keepersprite(0, 0, kspr->field_4, kspr->field_5, kspr_idx);
    SYNCDBG(18,"Finished");
}

void draw_single_keepersprite(long kspos_x, long kspos_y, struct KeeperSprite *kspr, long kspr_idx, long scale)
{
    long x,y;
    long src_dy,src_dx;
    SYNCDBG(18,"Starting");
    src_dy = (long)kspr->field_5;
    src_dx = (long)kspr->field_4;
    x = kspr->field_A;
    y = kspr->field_B;
    if ( EngineSpriteDrawUsingAlpha )
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        SetAlphaScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    } else
    {
        sp_x = kspos_x + ((scale * x) >> 5);
        sp_y = kspos_y + ((scale * y) >> 5);
        sp_dy = (src_dy * scale) >> 5;
        sp_dx = (src_dx * scale) >> 5;
        LbSpriteSetScalingData(sp_x, sp_y, src_dx, src_dy, sp_dx, sp_dy);
    }
    if ( thing_being_displayed_is_creature )
    {
        if ( (pointer_x >= x) && (pointer_x <= sp_dx + x) )
        {
            if ( (pointer_y >= y) && (pointer_y <= sp_dy + y) )
            {
                set_thing_pointed_at(thing_being_displayed);
            }
        }
    }
    draw_keepersprite(0, 0, kspr->field_4, kspr->field_5, kspr_idx);
    SYNCDBG(18,"Finished");
}

void process_keeper_sprite(short x, short y, unsigned short kspr_base, short kspr_frame, unsigned char sprgroup, long scale)
{
    struct KeeperSprite *creature_sprites;
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct KeeperSprite *kspr;
    long kspr_idx,draw_idx;
    short dim_ow,dim_oh,dim_th,dim_tw;
    long scaled_x,scaled_y;
    TbBool needs_xflip;
    long long lltemp;
    long sprite_group,sprite_delta,cutoff;
    SYNCDBG(17,"At (%d,%d) opts %d %d %d %d",(int)x,(int)y,(int)kspr_base,(int)kspr_frame,(int)sprgroup,(int)scale);
    player = get_my_player();

    if ( ((kspr_frame & 0x7FF) <= 1151) || ((kspr_frame & 0x7FF) >= 1919) )
        needs_xflip = 0;
    else
        needs_xflip = 1;
    if ( needs_xflip )
      lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    else
      lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
    sprite_group = sprgroup;
    lltemp = 4 - ((((long)kspr_frame + 128) & 0x7FF) >> 8);
    sprite_delta = llabs(lltemp);
    kspr_idx = keepersprite_index(kspr_base);
    global_scaler = scale;
    creature_sprites = keepersprite_array(kspr_base);
    scaled_x = ((scale * (long)creature_sprites->field_C) >> 5) + (long)x;
    scaled_y = ((scale * (long)creature_sprites->field_E) >> 5) + (long)y;
    SYNCDBG(17,"Scaled (%d,%d)",(int)scaled_x,(int)scaled_y);
    if (thing_is_invalid(thing_being_displayed))
    {
        water_y_offset = 0;
        water_source_cutoff = 0;
    } else
    if ( (thing_being_displayed->movement_flags & (TMvF_IsOnWater|TMvF_IsOnLava|TMvF_Unknown04)) == 0)
    {
        water_y_offset = 0;
        water_source_cutoff = 0;
    } else
    {
        cutoff = 6;
        if ( (thing_being_displayed->movement_flags & TMvF_Unknown04) != 0 )
        {
            get_keepsprite_unscaled_dimensions(thing_being_displayed->anim_sprite, thing_being_displayed->move_angle_xy, thing_being_displayed->field_48, &dim_ow, &dim_oh, &dim_tw, &dim_th);
            cctrl = creature_control_get_from_thing(thing_being_displayed);
            lltemp = dim_oh * (48 - (long)cctrl->word_9A);
            cutoff = ((((lltemp >> 24) & 0x1F) + (long)lltemp) >> 5) / 2;
        }
        if (player->view_mode == 1)
        {
            water_source_cutoff = cutoff;
            water_y_offset = (2 * scale * cutoff) >> 5;
        } else
        {
            water_source_cutoff = 2 * cutoff;
            water_y_offset = (scale * cutoff) >> 5;
        }
    }
    scaled_y += water_y_offset;
    if (creature_sprites->rotable == 0)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &creature_sprites[sprite_group];
        draw_idx = sprite_group + kspr_idx;
        if ( needs_xflip )
        {
            draw_single_keepersprite_omni_xflip(scaled_x, scaled_y, kspr, draw_idx, scale);
        } else
        {
            draw_single_keepersprite_omni(scaled_x, scaled_y, kspr, draw_idx, scale);
        }
    } else
    if (creature_sprites->rotable == 2)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &creature_sprites[sprite_group + sprite_delta * (long)creature_sprites->frames];
        draw_idx = sprite_group + sprite_delta * (long)kspr->frames + kspr_idx;
        if ( needs_xflip )
        {
            draw_single_keepersprite_xflip(scaled_x, scaled_y, kspr, draw_idx, scale);
        } else
        {
            draw_single_keepersprite(scaled_x, scaled_y, kspr, draw_idx, scale);
        }
    }
}

void process_keeper_speedup_sprite(struct JontySpr *jspr, long angle, long scale)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    long transp2;
    unsigned short graph_id2;
    unsigned long nframe2;
    long add_x,add_y;
    thing = jspr->thing;
    player = get_my_player();
    switch (thing->model)
    {
    case 28:
        graph_id2 = 112;
        if (player->view_type == PVT_DungeonTop)
        {
          add_x = scale >> 3;
          add_y = (scale >> 2) - scale;
        } else
        {
          add_x = scale * LbSinL(angle) >> 20;
          add_y = -(LbCosL(angle) * (scale + (scale >> 1))) >> 16;
        }
        transp2 = scale / 2;
        break;
    case 2:
        graph_id2 = 113;
        if (player->view_type == PVT_DungeonTop)
        {
            add_x = 0;
            add_y = 3 * scale >> 3;
        } else
        {
            add_x = (scale * LbSinL(angle)) >> 20;
            add_y = (scale * LbCosL(angle)) >> 20;
        }
        transp2 = 2 * scale / 3;
        break;
    default:
        graph_id2 = 113;
        if (player->view_type == PVT_DungeonTop)
        {
            add_x = (scale >> 2) / 3;
            add_y = (scale >> 1) / 3;
        } else
        {
            add_x = scale * LbSinL(angle) >> 20;
            add_y = (-(LbCosL(angle) * (scale + (scale >> 1))) >> 16) / 3;
        }
        transp2 = scale / 3;
        break;
    }
    EngineSpriteDrawUsingAlpha = 0;
    nframe2 = (thing->index + game.play_gameturn) % keepersprite_frames(graph_id2);
    process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->field_48, scale);
    EngineSpriteDrawUsingAlpha = 1;
    process_keeper_sprite(jspr->scr_x+add_x, jspr->scr_y+add_y, graph_id2, angle, nframe2, transp2);
}

void prepare_jonty_remap_and_scale(long *scale, const struct JontySpr *jspr)
{
    long i;
    struct Thing *thing;
    long shade,shade_factor,fade;
    thing = jspr->thing;
    if (lens_mode == 0)
    {
        fade = 65536;
        if ((thing->field_4F & 0x02) == 0)
            i = get_thing_shade(thing);
        else
            i = MINIMUM_LIGHTNESS;
        shade = i;
    } else
    if (jspr->field_14 <= lfade_min)
    {
        fade = jspr->field_14;
        if ((thing->field_4F & 0x02) == 0)
            i = get_thing_shade(thing);
        else
            i = MINIMUM_LIGHTNESS;
        shade = i;
    } else
    if (jspr->field_14 < lfade_max)
    {
        fade = jspr->field_14;
        if ((thing->field_4F & 0x02) == 0)
            i = get_thing_shade(thing);
        else
            i = MINIMUM_LIGHTNESS;
        shade = i * (long long)(lfade_max - fade) / fade_mmm;
    } else
    {
        fade = jspr->field_14;
        shade = 0;
    }
    shade_factor = shade >> 8;
    *scale = (thelens * (long)thing->sprite_size) / fade;
    if ((thing->field_4F & 0x0C) != 0)
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        shade_factor = thing->field_51;
        lbSpriteReMapPtr = &pixmap.ghost[256 * shade_factor];
    } else
    if (shade_factor == 32)
    {
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
    } else
    {
        lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
        lbSpriteReMapPtr = &pixmap.fade_tables[256 * shade_factor];
    }
}

void draw_jonty_mapwho(struct JontySpr *jspr)
{
    unsigned short flg_mem;
    unsigned char alpha_mem;
    struct PlayerInfo *player;
    struct Thing *thing;
    long angle,scale;
    flg_mem = lbDisplay.DrawFlags;
    alpha_mem = EngineSpriteDrawUsingAlpha;
    thing = jspr->thing;
    player = get_my_player();
    if (keepersprite_rotable(thing->anim_sprite))
      angle = thing->move_angle_xy - spr_map_angle;
    else
      angle = thing->move_angle_xy;
    prepare_jonty_remap_and_scale(&scale, jspr);
    EngineSpriteDrawUsingAlpha = 0;
    switch (thing->field_4F & (0x10|0x20))
    {
    case 0x10:
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR8;
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
        break;
    case 0x20:
        lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
        lbDisplay.DrawFlags &= ~Lb_TEXT_UNDERLNSHADOW;
        break;
    case 0x30:
        EngineSpriteDrawUsingAlpha = 1;
        break;
    }

    if ((thing->class_id == TCls_Creature)
     || (thing->class_id == TCls_Object)
     || (thing->class_id == TCls_DeadCreature))
    {
        if ((player->thing_under_hand == thing->index) && (game.play_gameturn & 2))
        {
            if (player->acamera->viewType == CAMERA_VIEW_EMPTY)
          {
              lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
              lbSpriteReMapPtr = white_pal;
          }
        } else
        if ((thing->field_4F & 0x80) != 0)
        {
            lbDisplay.DrawFlags |= Lb_TEXT_UNDERLNSHADOW;
            lbSpriteReMapPtr = red_pal;
            thing->field_4F &= ~0x80u;
        }
        thing_being_displayed_is_creature = 1;
        thing_being_displayed = thing;
    } else
    {
        thing_being_displayed_is_creature = 0;
        thing_being_displayed = NULL;
    }

    if (thing->anim_sprite >= CREATURE_FRAMELIST_LENGTH)
    {
        ERRORLOG("Invalid graphic Id %d from model %d, class %d", (int)thing->anim_sprite, (int)thing->model, (int)thing->class_id);
    } else
    {
        switch (thing->class_id)
        {
        case TCls_Object:
            //TODO CONFIG object model dependency, move to config
            if ((thing->model == 2) || (thing->model == 4) || (thing->model == 28))
            {
                process_keeper_speedup_sprite(jspr, angle, scale);
                break;
            }
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->field_48, scale);
            break;
        case TCls_Trap:
            //TODO CONFIG trap model dependency, make config option instead
            if ((thing->model != 1) && (player->id_number != thing->owner) && (thing->byte_18 == 0))
            {
                break;
            }
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->field_48, scale);
            break;
        default:
            process_keeper_sprite(jspr->scr_x, jspr->scr_y, thing->anim_sprite, angle, thing->field_48, scale);
            break;
        }
    }
    lbDisplay.DrawFlags = flg_mem;
    EngineSpriteDrawUsingAlpha = alpha_mem;
}

/** Fills solid area of the sprite in target buffer with color 255.
 *
 * @param sprdata Sprite data.
 * @param outbuf Output buffer to be filled.
 * @param lines_max Max lines to be written into output buffer.
 * @param scanln Length of scanline (length of line in output buffer).
 */
void sprite_to_sbuff(const TbSpriteData sprdata, unsigned char *outbuf, int lines_max, int scanln)
{
    unsigned char *out_lnstart;
    unsigned char *out;
    int cval;
    const unsigned char *sprd;
    int i;
    sprd = sprdata;
    out = outbuf;
    out_lnstart = outbuf;
    cval = 0;
    while (lines_max > 0)
    {
      while ( 1 )
      {
          // Skip transparent area
          while ( 1 )
          {
              cval = *(char *)sprd;
              sprd++;
              if (cval >= 0)
                break;
              cval = -cval;
              out += cval;
          }
          if (cval == 0)
            break;
          sprd += cval;
          // Fill area per-byte until we get 32bit-aligned position
          while ((unsigned int)out & 3)
          {
              *out = 0xFF;
              out++;
              cval--;
              if (cval <= 0)
                  break;
          }
          // Now fill the area faster - by writing 32-bit values
          for (i = (cval >> 2); i > 0; i--)
          {
              *(unsigned long *)out = 0xFFFFFFFF;
              out += 4;
          }
          // Fill the last unaligned bytes
          cval &= 3;
          while (cval > 0)
          {
            *out = 0xFF;
            out++;
            cval--;
          }
      }
      out_lnstart += scanln;
      out = out_lnstart;
      lines_max--;
    }
}

/** Fills solid area of the sprite in target buffer with color 255, x-flipping the sprite.
 *
 * @param sprdata Sprite data.
 * @param outbuf Output buffer to be filled.
 * @param lines_max Max lines to be written into output buffer.
 * @param scanln Length of scanline (length of line in output buffer).
 */
void sprite_to_sbuff_xflip(const TbSpriteData sprdata, unsigned char *outbuf, int lines_max, int scanln)
{
    unsigned char *out_lnstart;
    unsigned char *out;
    int cval;
    const unsigned char *sprd;
    int i;
    sprd = sprdata;
    out = outbuf;
    out_lnstart = outbuf;
    cval = 0;
    while (lines_max > 0)
    {
      while ( 1 )
      {
          // Skip transparent area
          while ( 1 )
          {
              cval = *(char *)sprd;
              sprd++;
              if (cval >= 0)
                  break;
              cval = -cval;
              out -= cval;
          }
          if (cval == 0)
            break;
          sprd += cval;
          out++;
          // Fill area per-byte until we get 32bit-aligned position
          while ((unsigned long)out & 3)
          {
              out--;
              *out = 0xFF;
              cval--;
              if (cval <= 0)
                  break;
          }
          // Now fill the area faster - by writing 32-bit values
          for (i = (cval >> 2); i > 0; i--)
          {
              out -= 4;
              *(unsigned long *)out = 0xFFFFFFFF;
          }
          out--;
          // Fill the last unaligned bytes
          cval &= 3;
          while (cval > 0)
          {
              *out = 0xFF;
              out--;
              cval--;
          }
      }
      out_lnstart += scanln;
      out = out_lnstart;
      lines_max--;
    }
}

void draw_keepsprite_unscaled_in_buffer(unsigned short kspr_n, short angle, unsigned char a3, unsigned char *outbuf)
{
    struct KeeperSprite *kspr_arr;
    unsigned long kspr_idx;
    struct KeeperSprite *kspr;
    unsigned int keepsprite_id;
    unsigned char *tmpbuf;
    int skip_w,skip_h;
    int fill_w,fill_h;
    TbBool flip_range;
    short quarter;
    int i;
    if ( ((angle & 0x7FF) <= 1151) || ((angle & 0x7FF) >= 1919) )
        flip_range = false;
    else
        flip_range = true;
    i = ((angle + 128) & 0x7FF);
    quarter = abs(4 - (i >> 8)); // i is restricted by "&" so (i>>8) is 0..7
    kspr_idx = keepersprite_index(kspr_n);
    kspr_arr = keepersprite_array(kspr_n);
    if (kspr_arr->rotable == 0)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        keepsprite_id = a3 + kspr_idx;
        kspr = &kspr_arr[a3];
        fill_w = kspr->field_6;
        fill_h = kspr->field_7;
        if ( flip_range )
        {
            tmpbuf = outbuf;
            skip_w = kspr->field_6 - kspr->field_A;
            skip_h = kspr->field_B;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff_xflip(*keepsprite[keepsprite_id], &outbuf[256 * skip_h + skip_w], kspr->field_5, 256);
        } else
        {
            tmpbuf = outbuf;
            skip_w = kspr->field_A;
            skip_h = kspr->field_B;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff(*keepsprite[keepsprite_id], &outbuf[256 * skip_h + skip_w], kspr->field_5, 256);
        }
    } else
    if (kspr_arr->rotable == 2)
    {
        if (!heap_manage_keepersprite(kspr_idx))
        {
            return;
        }
        kspr = &kspr_arr[a3 + quarter * kspr_arr->frames];
        fill_w = kspr->field_4;
        fill_h = kspr->field_5;
        keepsprite_id = a3 + quarter * kspr->frames + kspr_idx;
        if ( flip_range )
        {
            tmpbuf = outbuf;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff_xflip(*keepsprite[keepsprite_id], &outbuf[kspr->field_4], kspr->field_5, 256);
        } else
        {
            tmpbuf = outbuf;
            for (i = fill_h; i > 0; i--)
            {
                LbMemorySet(tmpbuf, 0, fill_w);
                tmpbuf += 256;
            }
            sprite_to_sbuff(*keepsprite[keepsprite_id], &outbuf[0], kspr->field_5, 256);
        }
    }
}

void update_frontview_pointed_block(unsigned long laaa, unsigned char qdrant, long w, long h, long qx, long qy)
{
    TbGraphicsWindow ewnd;
    struct Column *colmn;
    unsigned long mask;
    struct Map *mapblk;
    long pos_x,pos_y;
    long slb_x,slb_y;
    long point_a,point_b,delta;
    long i;
    SYNCDBG(16,"Starting");
    store_engine_window(&ewnd,1);
    point_a = (((GetMouseX() - ewnd.x) << 8) - qx) << 8;
    point_b = (((GetMouseY() - ewnd.y) << 8) - qy) << 8;
    delta = (laaa << 7) / 256 << 8;
    for (i=0; i < 8; i++)
    {
        pos_x = (point_a / laaa) * x_step2[qdrant] + (point_b / laaa) * x_step1[qdrant] + (w << 8);
        pos_y = (point_a / laaa) * y_step2[qdrant] + (point_b / laaa) * y_step1[qdrant] + (h << 8);
        slb_x = (pos_x >> 8) + x_offs[qdrant];
        slb_y = (pos_y >> 8) + y_offs[qdrant];
        mapblk = get_map_block_at(slb_x, slb_y);
        if (!map_block_invalid(mapblk))
        {
          if (i == 0)
          {
            floor_pointed_at_x = slb_x;
            floor_pointed_at_y = slb_y;
            block_pointed_at_x = slb_x;
            block_pointed_at_y = slb_y;
            pointed_at_frac_x = pos_x & 0xFF;
            pointed_at_frac_y = pos_y & 0xFF;
            me_pointed_at = mapblk;
          } else
          {
            colmn = get_map_column(mapblk);
            mask = colmn->solidmask;
            if ( (1 << (i-1)) & mask )
            {
              pointed_at_frac_x = pos_x & 0xFF;
              pointed_at_frac_y = pos_y & 0xFF;
              block_pointed_at_x = slb_x;
              block_pointed_at_y = slb_y;
              me_pointed_at = mapblk;
            }
            if (((temp_cluedo_mode)  && (i == 2))
             || ((!temp_cluedo_mode) && (i == 5)))
            {
              top_pointed_at_frac_x = pos_x & 0xFF;
              top_pointed_at_frac_y = pos_y & 0xFF;
              top_pointed_at_x = slb_x;
              top_pointed_at_y = slb_y;
            }
          }
        }
        point_b += delta;
    }
}

void create_frontview_map_volume_box(struct Camera *cam, unsigned char stl_width)
{
    struct Coord3d pos;
    long coord_x,coord_y,coord_z;
    unsigned char orient;
    long i;
    long slb_width,depth;
    long vstart,vend;
    long delta[4];

    pos.y.val = map_volume_box.field_7;
    pos.x.val = map_volume_box.field_3;
    pos.z.val = 5*256;
    orient = ((unsigned int)(cam->orient_a + 256) >> 9) & 0x03;
    convert_world_coord_to_front_view_screen_coord(&pos, cam, &coord_x, &coord_y, &coord_z);
    depth = (5 - map_volume_box.field_13) * ((long)stl_width << 7) / 256;
    slb_width = STL_PER_SLB * (long)stl_width;
    switch ( orient )
    {
    case 1:
        coord_y -= slb_width;
        coord_z += slb_width;
        break;
    case 2:
        coord_x -= slb_width;
        coord_y -= slb_width;
        coord_z += slb_width;
        break;
    case 3:
        coord_x -= slb_width;
        break;
    }
    vstart = 0;
    coord_z -= (stl_width >> 1);
    vend = stl_width;
    delta[0] = 0;
    delta[1] = slb_width;
    delta[2] = depth;
    delta[3] = slb_width + depth;
    // Draw a horizonal line element for every subtile
    for (i=3; i > 0; i--)
    {
      if (!is_free_space_in_poly_pool(4))
        break;
      create_line_element(coord_x + vstart,    coord_y + delta[0],  coord_x + vend,      coord_y + delta[0], coord_z,             map_volume_box.color);
      create_line_element(coord_x + vstart,    coord_y + delta[1],  coord_x + vend,      coord_y + delta[1], coord_z - slb_width, map_volume_box.color);
      create_line_element(coord_x + vstart,    coord_y + delta[2],  coord_x + vend,      coord_y + delta[2], coord_z,             map_volume_box.color);
      create_line_element(coord_x + vstart,    coord_y + delta[3],  coord_x + vend,      coord_y + delta[3], coord_z - slb_width, map_volume_box.color);
      vend += stl_width;
      vstart += stl_width;
    }
    // Now the rectangles at left and right
    for (i=3; i > 0; i--)
    {
      if (!is_free_space_in_poly_pool(4))
        break;
      create_line_element(coord_x,             coord_y + delta[0],  coord_x,             coord_y + delta[1], coord_z - delta[0], map_volume_box.color);
      create_line_element(coord_x + slb_width, coord_y + delta[0],  coord_x + slb_width, coord_y + delta[1], coord_z - delta[0], map_volume_box.color);
      create_line_element(coord_x,             coord_y + delta[2],  coord_x,             coord_y + delta[3], coord_z - delta[0], map_volume_box.color);
      create_line_element(coord_x + slb_width, coord_y + delta[2],  coord_x + slb_width, coord_y + delta[3], coord_z - delta[0], map_volume_box.color);
      delta[0] += stl_width;
      delta[2] += stl_width;
      delta[3] += stl_width;
      delta[1] += stl_width;
    }
}

void do_map_who_for_thing(struct Thing *thing)
{
    int bckt_idx;
    struct EngineCoord ecor;
    struct NearestLights nearlgt;
    switch (thing->field_50 >> 2)
    {
    case 2:
        ecor.x = ((long)thing->mappos.x.val - map_x_pos);
        ecor.z = (map_y_pos - (long)thing->mappos.y.val);
        ecor.field_8 = 0;
        ecor.y = ((long)thing->field_60 - map_z_pos);
        if (thing_is_creature(thing) && ((thing->movement_flags & TMvF_Unknown04) == 0))
        {
            int count, i;
            count = find_closest_lights(&thing->mappos, &nearlgt);
            for (i=0; i < count; i++) {
                create_shadows(thing, &ecor, &nearlgt.coord[i]);
            }
        }
        ecor.y = thing->mappos.z.val - map_z_pos;
        if ( thing->class_id == 5 )
            create_status_box(thing, &ecor);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end)
        {
          if ( lens_mode )
            bckt_idx = (ecor.z - 64) / 16;
          else
            bckt_idx = (ecor.z - 64) / 16 - 6;
          add_unkn11_to_polypool(thing, ecor.view_width, ecor.view_height, ecor.z, bckt_idx);
        }
        break;
    case 3:
        ecor.x = ((long)thing->mappos.x.val - map_x_pos);
        ecor.z = (map_y_pos - (long)thing->mappos.y.val);
        ecor.field_8 = 0;
        ecor.y = ((long)thing->mappos.z.val - map_z_pos);
        memcpy(&object_origin, &ecor, sizeof(struct EngineCoord));
        object_origin.x = 0;
        object_origin.y = 0;
        object_origin.z = 0;
        break;
    case 4:
        ecor.x = ((long)thing->mappos.x.val - map_x_pos);
        ecor.z = (map_y_pos - (long)thing->mappos.y.val);
        ecor.y = ((long)thing->mappos.z.val - map_z_pos);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end)
        {
            add_unkn16_to_polypool(ecor.view_width, ecor.view_height, thing->long_13, 1);
        }
        break;
    case 5:
        ecor.x = ((long)thing->mappos.x.val - map_x_pos);
        ecor.z = (map_y_pos - (long)thing->mappos.y.val);
        ecor.y = ((long)thing->mappos.z.val - map_z_pos);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end)
        {
            if (game.play_gameturn - thing->long_15 == 1)
            {
              if (thing->byte_19 < 40)
                thing->byte_19++;
            } else
            {
                thing->byte_19 = 0;
            }
            thing->long_15 = game.play_gameturn;
            if (thing->byte_19 == 40)
            {
                bckt_idx = (ecor.z - 64) / 16 - 6;
                add_room_flag_pole_to_polypool(ecor.view_width, ecor.view_height, thing->roomflag.room_idx, bckt_idx);
                if (getpoly < poly_pool_end)
                {
                    add_room_flag_top_to_polypool(ecor.view_width, ecor.view_height, thing->roomflag.room_idx, 1);
                }
            }
        }
        break;
    case 6:
        ecor.x = (thing->mappos.x.val - map_x_pos);
        ecor.z = (map_y_pos - thing->mappos.y.val);
        ecor.y = (thing->mappos.z.val - map_z_pos);
        rotpers(&ecor, &camera_matrix);
        if (getpoly < poly_pool_end) {
            add_unkn18_to_polypool(thing, ecor.view_width, ecor.view_height, ecor.z, 1);
        }
        break;
    default:
        break;
    }
}

void do_map_who(short tnglist_idx)
{
    //_DK_do_map_who(a1); return;
    long i;
    unsigned long k;
    k = 0;
    i = tnglist_idx;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if ((thing->field_4F & TF4F_Unknown01) == 0)
        {
            do_map_who_for_thing(thing);
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

void draw_frontview_thing_on_element(struct Thing *thing, struct Map *map, struct Camera *cam)
{
    long cx,cy,cz;
    if ((thing->field_4F & TF4F_Unknown01) != 0)
        return;
    switch ( (thing->field_50 >> 2) )
    {
    case 2:
        convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            add_unkn11_to_polypool(thing, cx, cy, cy, cz-3);
            if ((thing->class_id == TCls_Creature) && is_free_space_in_poly_pool(1))
            {
              create_fast_view_status_box(thing, cx, cy);
            }
        }
        break;
    case 4:
        convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            add_unkn16_to_polypool(cx, cy, thing->creature.gold_carried, 1);
        }
        break;
    case 5:
        convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
          if (game.play_gameturn - thing->long_15 != 1)
          {
              thing->byte_19 = 0;
          } else
          if (thing->byte_19 < 40)
          {
              thing->byte_19++;
          }
          thing->long_15 = game.play_gameturn;
          if (thing->byte_19 == 40)
          {
              add_room_flag_pole_to_polypool(cx, cy, thing->roomflag.room_idx, cz-3);
              if (is_free_space_in_poly_pool(1))
              {
                  add_room_flag_top_to_polypool(cx, cy, thing->roomflag.room_idx, 1);
              }
          }
        }
        break;
    case 6:
        convert_world_coord_to_front_view_screen_coord(&thing->mappos,cam,&cx,&cy,&cz);
        if (is_free_space_in_poly_pool(1))
        {
            add_unkn18_to_polypool(thing, cx, cy, cy, cz-3);
        }
        break;
    default:
        break;
    }
}

void draw_frontview_things_on_element(struct Map *map, struct Camera *cam)
{
    struct Thing *thing;
    long i;
    unsigned long k;
    k = 0;
    i = get_mapwho_thing_index(map);
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        draw_frontview_thing_on_element(thing, map, cam);
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

void draw_frontview_engine(struct Camera *cam)
{
    long zoom_mem;
    struct PlayerInfo *player;
    TbGraphicsWindow grwnd;
    TbGraphicsWindow ewnd;
    unsigned char qdrant;
    long px,py,qx,qy;
    long w,h;
    long pos_x,pos_y;
    MapSubtlCoord stl_x,stl_y;
    long lim_x,lim_y;
    long cam_x,cam_y;
    long long zoom,lbbb;
    long i;
    SYNCDBG(9,"Starting");
    player = get_my_player();
    if (cam->zoom > 65536)
        cam->zoom = 65536;
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    zoom_mem = cam->zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    cam->zoom = camera_zoom;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    UseFastBlockDraw = (camera_zoom == 65536);
    LbScreenStoreGraphicsWindow(&grwnd);
    store_engine_window(&ewnd,pixel_size);
    LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    gtblock_set_clipping_window(lbDisplay.GraphicsWindowPtr, ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    setup_vecs(lbDisplay.GraphicsWindowPtr, NULL, lbDisplay.GraphicsScreenWidth, ewnd.width, ewnd.height);
    engine_player_number = player->id_number;
    player_bit = (1 << player->id_number);
    clear_fast_bucket_list();
    store_engine_window(&ewnd,1);
    setup_engine_window(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    qdrant = ((unsigned int)(cam->orient_a + 256) >> 9) & 0x03;
    zoom = camera_zoom >> 3;
    w = (ewnd.width << 16) / zoom >> 1;
    h = (ewnd.height << 16) / zoom >> 1;
    cam_x = cam->mappos.x.val;
    cam_y = cam->mappos.y.val;
    switch (qdrant)
    {
    case 0:
        px = ((cam_x - w) >> 8);
        py = ((cam_y - h) >> 8);
        lbbb = cam_x - (px << 8);
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = cam_y - (py << 8);
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        break;
    case 1:
        px = ((cam_x + h) >> 8);
        py = ((cam_y - w) >> 8);
        lbbb = cam_y - (py << 8);
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = (px << 8) - cam_x;
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        px--;
        break;
    case 2:
        px = ((cam_x + w) >> 8) + 1;
        py = ((cam_y + h) >> 8);
        lbbb = (px << 8) - cam_x;
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = (py << 8) - cam_y;
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        px--;
        py--;
        break;
    case 3:
        px = ((cam_x - h) >> 8);
        py = ((cam_y + w) >> 8) + 1;
        lbbb = (py << 8) - cam_y;
        qx = (ewnd.width << 7)  - ((zoom * lbbb) >> 8);
        lbbb = cam_x - (px << 8);
        qy = (ewnd.height << 7) - ((zoom * lbbb) >> 8);
        py--;
        break;
    default:
        ERRORLOG("Illegal quadrant, %d.",qdrant);
        LbScreenLoadGraphicsWindow(&grwnd);
        return;
    }

    update_frontview_pointed_block(zoom, qdrant, px, py, qx, qy);
    if (map_volume_box.visible)
        create_frontview_map_volume_box(cam, (zoom >> 8) & 0xFF);
    map_volume_box.visible = 0;

    h = (8 * (zoom + 32 * ewnd.height) - qy) / zoom;
    w = (8 * (zoom + 32 * ewnd.height) - qy) / zoom;
    qy += zoom * h;
    px += x_step1[qdrant] * w;
    stl_x = x_step1[qdrant] * w + px;
    stl_y = y_step1[qdrant] * h + py;
    py += y_step1[qdrant] * h;
    lim_x = ewnd.width << 8;
    lim_y = -zoom;
    SYNCDBG(19,"Range (%ld,%ld) to (%ld,%ld), quadrant %d",px,py,qx,qy,(int)qdrant);
    for (pos_x=qx; pos_x < lim_x; pos_x += zoom)
    {
        i = (ewnd.height << 8);
        // Initialize the stl_? which will be swept by second loop
        if (x_step1[qdrant] != 0)
          stl_x = px;
        else
          stl_y = py;
        for (pos_y=qy; pos_y > lim_y; pos_y -= zoom)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            if (!map_block_invalid(mapblk))
            {
                if (get_mapblk_column_index(mapblk) > 0)
                {
                    draw_element(mapblk, game.lish.subtile_lightness[get_subtile_number(stl_x,stl_y)], stl_x, stl_y, pos_x, pos_y, zoom, qdrant, &i);
                }
                if ( subtile_revealed(stl_x, stl_y, player->id_number) )
                {
                    draw_frontview_things_on_element(mapblk, cam);
                }
            }
            stl_x -= x_step1[qdrant];
            stl_y -= y_step1[qdrant];
        }
        stl_x += x_step2[qdrant];
        stl_y += y_step2[qdrant];
    }

    display_fast_drawlist(cam);
    LbScreenLoadGraphicsWindow(&grwnd);
    cam->zoom = zoom_mem;//TODO [zoom] remove when all cam->zoom will be changed to camera_zoom
    SYNCDBG(9,"Finished");
}
/******************************************************************************/

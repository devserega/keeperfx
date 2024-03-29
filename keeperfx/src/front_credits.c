/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_credits.c
 *     Credits displaying routines.
 * @par Purpose:
 *     Functions to show and maintain credits screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 May 2009 - 20 Jun 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "front_credits.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"
#include "bflib_keybrd.h"
#include "gui_frontbtns.h"
#include "frontend.h"
#include "config_strings.h"
#include "config_campaigns.h"

/******************************************************************************/
void frontcredits_draw(void)
{
    struct CreditsItem *credit;
    TbBool did_draw;
    int fontid;
    long i;
    long h;
    const char *text;
    credits_offset -= credits_scroll_speed;
    frontend_copy_background();

    lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE | Lb_TEXT_HALIGN_CENTER;
    LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
    fontid = 1;
    LbTextSetFont(frontend_font[fontid]);
    h = credits_offset;
    did_draw = h > 0;
    for (i = 0; campaign.credits[i].kind != CIK_None; i++)
    {
        if (h >= lbDisplay.PhysicalScreenHeight)
          break;
        credit = &campaign.credits[i];
        if (credit->font != fontid)
        {
          fontid = credit->font;
          LbTextSetFont(frontend_font[fontid]);
        }
        int ln_height;
        ln_height = LbTextLineHeight() * units_per_pixel / 16;
        if (h > -ln_height)
        {
            switch (credit->kind)
            {
            case CIK_StringId:
                text = get_string(credit->num);
                break;
            case CIK_DirectText:
                text = credit->str;
                break;
            default:
                text = "";
                break;
            }
            LbTextDrawResized(0, h, units_per_pixel, text);
            did_draw = 1;
        }
        h += ln_height + 2 * units_per_pixel / 16;
    }
    if (!did_draw)
    {
        credits_end = 1;
        credits_offset = lbDisplay.PhysicalScreenHeight;
    }
}

void frontcredits_input(void)
{
    int fontid;
    credits_scroll_speed = 1 * units_per_pixel / 16;
    fontid = 1;
    int speed;
    if ( lbKeyOn[KC_DOWN] )
    {
        LbTextSetFont(frontend_font[fontid]);
        speed = LbTextLineHeight() * units_per_pixel / 16;
        credits_scroll_speed = speed;
    } else
    if ((lbKeyOn[KC_UP]) && (credits_offset <= 0))
    {
        LbTextSetFont(frontend_font[fontid]);
        speed = -LbTextLineHeight() * units_per_pixel / 16;
        if (speed <= credits_offset)
          speed = credits_offset;
        credits_scroll_speed = speed;
    }
}

/******************************************************************************/

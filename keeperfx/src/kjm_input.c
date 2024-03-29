/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file kjm_input.c
 *     Keyboard-Joypad-Mouse input routines.
 * @par Purpose:
 *     Allows reading state of input devices.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "kjm_input.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_math.h"

#include "config_settings.h"
#include "config_strings.h"
#include "frontend.h"
#include "frontmenu_ingame_map.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** Initialization array, used to create array which stores index of text name of keyboard keys. */
struct KeyToStringInit key_to_string_init[] = {
  {KC_A,  -65},
  {KC_B,  -66},
  {KC_C,  -67},
  {KC_D,  -68},
  {KC_E,  -69},
  {KC_F,  -70},
  {KC_G,  -71},
  {KC_H,  -72},
  {KC_I,  -73},
  {KC_J,  -74},
  {KC_K,  -75},
  {KC_L,  -76},
  {KC_M,  -77},
  {KC_N,  -78},
  {KC_O,  -79},
  {KC_P,  -80},
  {KC_Q,  -81},
  {KC_R,  -82},
  {KC_S,  -83},
  {KC_T,  -84},
  {KC_U,  -85},
  {KC_V,  -86},
  {KC_W,  -87},
  {KC_X,  -88},
  {KC_Y,  -89},
  {KC_Z,  -90},
  {KC_F1,  GUIStr_KeyF1},
  {KC_F2,  GUIStr_KeyF2},
  {KC_F3,  GUIStr_KeyF3},
  {KC_F4,  GUIStr_KeyF4},
  {KC_F5,  GUIStr_KeyF5},
  {KC_F6,  GUIStr_KeyF6},
  {KC_F7,  GUIStr_KeyF7},
  {KC_F8,  GUIStr_KeyF8},
  {KC_F9,  GUIStr_KeyF9},
  {KC_F10, GUIStr_KeyF10},
  {KC_F11, GUIStr_KeyF11},
  {KC_F12, GUIStr_KeyF12},
  {KC_CAPITAL, GUIStr_KeyCapsLock},
  {KC_LSHIFT,  GUIStr_KeyLeftShift},
  {KC_RSHIFT,  GUIStr_KeyRightShift},
  {KC_LCONTROL, GUIStr_KeyLeftControl},
  {KC_RCONTROL, GUIStr_KeyRightControl},
  {KC_RETURN,  GUIStr_KeyReturn},
  {KC_BACK,    GUIStr_KeyBackspace},
  {KC_INSERT,  GUIStr_KeyInsert},
  {KC_DELETE,  GUIStr_KeyDelete},
  {KC_HOME,    GUIStr_KeyHome},
  {KC_END,     GUIStr_KeyEnd},
  {KC_PGUP,    GUIStr_KeyPageUp},
  {KC_PGDOWN,  GUIStr_KeyPageDown},
  {KC_NUMLOCK, GUIStr_KeyNumLock},
  {KC_DIVIDE,  GUIStr_KeyNumSlash},
  {KC_MULTIPLY, GUIStr_KeyNumMul},
  {KC_NUMPADENTER, GUIStr_KeyNumEnter},
  {KC_DECIMAL, GUIStr_KeyNumDelete},
  {KC_NUMPAD0, GUIStr_KeyNum0},
  {KC_NUMPAD1, GUIStr_KeyNum1},
  {KC_NUMPAD2, GUIStr_KeyNum2},
  {KC_NUMPAD3, GUIStr_KeyNum3},
  {KC_NUMPAD4, GUIStr_KeyNum4},
  {KC_NUMPAD5, GUIStr_KeyNum5},
  {KC_NUMPAD6, GUIStr_KeyNum6},
  {KC_NUMPAD7, GUIStr_KeyNum7},
  {KC_NUMPAD8, GUIStr_KeyNum8},
  {KC_NUMPAD9, GUIStr_KeyNum9},
  {KC_UP,     GUIStr_KeyUp},
  {KC_DOWN,   GUIStr_KeyDown},
  {KC_LEFT,   GUIStr_KeyLeft},
  {KC_RIGHT,  GUIStr_KeyRight},
  {  0,     0},
};
/******************************************************************************/
/**
 * Returns X position of mouse cursor on screen.
 */
long GetMouseX(void)
{
  long result;
  result = lbDisplay.MMouseX * (long)pixel_size;
  return result;
}

/**
 * Returns Y position of mouse cursor on screen.
 */
long GetMouseY(void)
{
  long result;
  result = lbDisplay.MMouseY * (long)pixel_size;
  return result;
}

short is_mouse_pressed_leftbutton(void)
{
  return lbDisplay.LeftButton;
}

short is_mouse_pressed_rightbutton(void)
{
  return lbDisplay.RightButton;
}

short is_mouse_pressed_lrbutton(void)
{
  return (lbDisplay.LeftButton || lbDisplay.RightButton);
}

void clear_mouse_pressed_lrbutton(void)
{
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
}

void update_left_button_released(void)
{
  left_button_released = 0;
  left_button_double_clicked = 0;
  if ( lbDisplay.LeftButton )
  {
    left_button_held = 1;
    left_button_held_x = GetMouseX();
    left_button_held_y = GetMouseY();
  }
  if (left_button_held)
  {
    if (!lbDisplay.MLeftButton)
    {
      left_button_released = 1;
      left_button_held = 0;
      left_button_released_x = GetMouseX();
      left_button_released_y = GetMouseY();
      if ( left_button_click_space_count < 5 )
      {
        left_button_double_clicked = 1;
        left_button_double_clicked_x = left_button_released_x;
        left_button_double_clicked_y = left_button_released_y;
      }
      left_button_click_space_count = 0;
    }
  } else
  {
    if (left_button_click_space_count < LONG_MAX)
      left_button_click_space_count++;
  }
}

void update_right_button_released(void)
{
  right_button_released = 0;
  right_button_double_clicked = 0;
  if (lbDisplay.RightButton)
  {
    right_button_held = 1;
    right_button_held_x = GetMouseX();
    right_button_held_y = GetMouseY();
  }
  if ( right_button_held )
  {
    if ( !lbDisplay.MRightButton )
    {
      right_button_released = 1;
      right_button_held = 0;
      right_button_released_x = GetMouseX();
      right_button_released_y = GetMouseY();
      if (right_button_click_space_count < 5)
      {
        right_button_double_clicked = 1;
        right_button_double_clicked_x = right_button_released_x;
        right_button_double_clicked_y = right_button_released_y;
      }
      right_button_click_space_count = 0;
    }
  } else
  {
    if (right_button_click_space_count < LONG_MAX)
      right_button_click_space_count++;
  }
}

void update_left_button_clicked(void)
{
  left_button_clicked = lbDisplay.LeftButton;
  left_button_clicked_x = lbDisplay.MouseX * (long)pixel_size;
  left_button_clicked_y = lbDisplay.MouseY * (long)pixel_size;
}

void update_right_button_clicked(void)
{
  right_button_clicked = lbDisplay.RightButton;
  right_button_clicked_x = lbDisplay.MouseX * (long)pixel_size;
  right_button_clicked_y = lbDisplay.MouseY * (long)pixel_size;
}

void update_mouse(void)
{
  update_left_button_released();
  update_right_button_released();
  update_left_button_clicked();
  update_right_button_clicked();
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
}

/**
 * Checks if a specific key is pressed.
 * @param key Code of the key to check.
 * @param kmodif Key modifier flags required.
 */
short is_key_pressed(TbKeyCode key, TbKeyMods kmodif)
{
  if ((kmodif == KMod_DONTCARE) || (kmodif == key_modifiers))
    return lbKeyOn[key];
  return 0;
}

/**
 * Converts keyboard key code into ASCII character.
 * @param key Code of the key being pressed.
 * @param kmodif Key modifier flags.
 * @note Key modifier can't be KMod_DONTCARE in this function.
 */
unsigned short key_to_ascii(TbKeyCode key, TbKeyMods kmodif)
{
  if ((key<0) || (key>=128))
    return 0;
  if (kmodif & KMod_SHIFT)
    return lbInkeyToAsciiShift[key];
  return lbInkeyToAscii[key];
}

/**
 * Clears the marking that a specific key is pressed.
 */
void clear_key_pressed(long key)
{
  if ((key<0) || (key>=sizeof(lbKeyOn)))
    return;
  lbKeyOn[key] = 0;
  if (key == lbInkey)
    lbInkey = KC_UNASSIGNED;
}

/**
 * Set key modifiers based on the pressed key codes.
 */
void update_key_modifiers(void)
{
  unsigned short key_mods=0;
  if ( lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT] )
    key_mods |= KMod_SHIFT;
  if ( lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL] )
    key_mods |= KMod_CONTROL;
  if ( lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT] )
    key_mods |= KMod_ALT;
  key_modifiers = key_mods;
}

long set_game_key(long key_id, unsigned char key, unsigned int mods)
{
    if (!key_to_string[key]) {
      return 0;
    }
    // Rotate & speed - allow only lone modifiers
    if (key_id == 4 || key_id == 5)
    {
        if ((mods & KMod_SHIFT) || (mods & KMod_CONTROL))
        {
            int ncode;
            if (mods & KMod_SHIFT) {
                ncode = 42;
            } else
            if (mods & KMod_CONTROL) {
                ncode = 29;
            } else {
                ncode = 29;
            }
            struct GameKey  *kbk;
            kbk = &settings.kbkeys[((unsigned int)(key_id - 4) < 1) + 4];
            if (kbk->code != ncode)
            {
                kbk = &settings.kbkeys[key_id];
                if (mods & KMod_SHIFT)
                {
                    kbk->code = 42;
                } else
                if (mods & KMod_CONTROL)
                {
                    kbk->code = 29;
                }
                kbk->mods = 0;
            }
            return 1;
        } else
        {
            return 0;
        }
    }
    // Possess & query - allow lone modifiers and normal keys
    if (key_id == 27 || key_id == 28)
    {
        if ((mods & KMod_SHIFT) || (mods & KMod_CONTROL))
        {
            int ncode;
            if (mods & KMod_SHIFT) {
                ncode = 42;
            } else
            if (mods & KMod_CONTROL) {
                ncode = 29;
            } else {
                ncode = 29;
            }
            struct GameKey  *kbk;
            kbk = &settings.kbkeys[((unsigned int)(key_id - 27) < 1) + 27];
            if (kbk->code != ncode)
            {
                kbk = &settings.kbkeys[key_id];
                if (mods & KMod_SHIFT) {
                    kbk->code = 42;
                } else
                if (mods & KMod_CONTROL) {
                    kbk->code = 29;
                }
                kbk->mods = 0;
            }
            return 1;
        } else
        {
            long i;
            struct GameKey  *kbk;
            for (i = 0; i < GAME_KEYS_COUNT; i++)
            {
                kbk = &settings.kbkeys[i];
                if ((i != key_id) && (kbk->code == key) && (kbk->mods == mods)) {
                    return 0;
                }
            }
            kbk = &settings.kbkeys[key_id];
            kbk->code = key;
            kbk->mods = 0;
            return 1;
        }
    }
    // Just ignore these keystrokes
    if ( key == 42 || key == 54 || key == 29 || key == 157 )
    {
        return 0;
    }
    // The normal keys - allow a key alone, or with one modifier
    {
        if (((mods & KMod_SHIFT) && (mods & KMod_CONTROL))
         || ((mods & KMod_SHIFT) && (mods & KMod_ALT))
         || ((mods & KMod_CONTROL) && (mods & KMod_ALT))) {
            return 0;
        }
        struct GameKey *kbk;
        long i;
        for (i = 0; i < GAME_KEYS_COUNT; i++)
        {
            kbk = &settings.kbkeys[i];
            if ((i != key_id) && (kbk->code == key) && (kbk->mods == mods)) {
                return 0;
            }
        }
        kbk = &settings.kbkeys[key_id];
        kbk->code = key;
        kbk->mods = mods & (KMod_SHIFT|KMod_CONTROL|KMod_ALT);
        return 1;
    }
}

void define_key_input(void)
{
  if (lbInkey == KC_ESCAPE)
  {
    defining_a_key = 0;
    lbInkey = KC_UNASSIGNED;
  } else
  if (lbInkey != KC_UNASSIGNED)
  {
      update_key_modifiers();
      if ( set_game_key(defining_a_key_id, lbInkey, key_modifiers) )
        defining_a_key = 0;
      lbInkey = KC_UNASSIGNED;
  }
}

/**
 * Fills the array of keyboard key names.
 */
void init_key_to_strings(void)
{
    struct KeyToStringInit *ktsi;
    long k;
    LbMemorySet(key_to_string, 0, sizeof(key_to_string));
    for (ktsi = &key_to_string_init[0]; ktsi->chr != 0; ktsi++)
    {
      k = ktsi->chr;
      key_to_string[k] = ktsi->str_idx;
    }
}

/**
 * Returns if the mouse is over "pannel map" - the circular minimap area on top left.
 * @param x Pannel map circle start X coordinate.
 * @param y Pannel map circle start Y coordinate.
 * @return
 */
TbBool mouse_is_over_pannel_map(ScreenCoord x, ScreenCoord y)
{
    long cmx,cmy;
    long px,py;
    cmx = GetMouseX();
    cmy = GetMouseY();
    int units_per_px;
    units_per_px = (16*status_panel_width + 140/2) / 140;
    px = (cmx-(x+PANNEL_MAP_RADIUS*units_per_px/16));
    py = (cmy-(y+PANNEL_MAP_RADIUS*units_per_px/16));
    return (LbSqrL(px*px + py*py) < PANNEL_MAP_RADIUS*units_per_px/16);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif

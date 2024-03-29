/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_mouse.cpp
 *     Mouse related routines.
 * @par Purpose:
 *     Pointer position, movement and cursor support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     12 Feb 2008 - 26 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_mouse.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_video.h"
#include "bflib_memory.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_mshandler.hpp"
#include "bflib_keybrd.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*

struct mouse_buffer mbuffer;
struct mouse_info minfo;
char ptr[4096];
unsigned int redraw_active=0;
unsigned int mouse_initialised=0;
short volatile mouse_mickey_x;
short volatile mouse_mickey_y;
long volatile mouse_dx;
long volatile mouse_dy;
unsigned long mouse_pos_change_saved;
struct DevInput joy;
*/

// Whether we want to relative mouse mode, when this is on, mouse will be trapped in game window.
volatile TbBool lbUseRelativeMouseMode = true;
volatile TbDisplayStructEx lbDisplayEx;
/******************************************************************************/
TbResult LbMouseChangeSpriteAndHotspot(struct TbSprite *pointerSprite, long hot_x, long hot_y)
{
#if (BFDEBUG_LEVEL > 18)
  if (pointerSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)pointerSprite->SWidth,(int)pointerSprite->SHeight,pointerSprite);
#endif
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseSpriteAndOffset(pointerSprite, hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetup(struct TbSprite *pointerSprite)
{
  TbResult ret;
  long x,y;
  if (lbMouseSpriteInstalled)
  {
      LbMouseSuspend();
  }
  y = (lbDisplay.MouseWindowHeight + lbDisplay.MouseWindowY) / 2;
  x = (lbDisplay.MouseWindowWidth + lbDisplay.MouseWindowX) / 2;
  pointerHandler.Install();
  lbMouseOffline = true;
  lbMouseSpriteInstalled = true;
  LbMouseSetWindow(0, 0, LbScreenWidth(), LbScreenHeight());

  ret = Lb_SUCCESS;
  
  if ((LbMouseSetPosition(x, y) != Lb_SUCCESS) ||
      (LbMouseChangeSprite(pointerSprite) != Lb_SUCCESS))
  {
      ret = Lb_FAIL;
  }

  lbMouseSpriteInstalled = (ret == Lb_SUCCESS);
  lbMouseOffline = false;
  return ret;
}

TbResult LbMouseSetPointerHotspot(long hot_x, long hot_y)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseSpriteOffset(hot_x, hot_y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetPosition(long x, long y)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMousePosition(x, y))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseChangeSprite(struct TbSprite *pointerSprite)
{
#if (BFDEBUG_LEVEL > 18)
  if (pointerSprite == NULL)
    SYNCLOG("Setting to %s","NONE");
  else
    SYNCLOG("Setting to %dx%d, data at %p",(int)pointerSprite->SWidth,(int)pointerSprite->SHeight,pointerSprite);
#endif
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseSprite(pointerSprite))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

void GetPointerHotspot(long *hot_x, long *hot_y)
{
  struct TbPoint *hotspot;
  hotspot = pointerHandler.GetMouseSpriteOffset();
  if (hotspot == NULL)
    return;
  *hot_x = hotspot->x;
  *hot_y = hotspot->y;
}

TbResult LbMouseIsInstalled(void)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.IsInstalled())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseSetWindow(long x, long y, long width, long height)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.SetMouseWindow(x, y, width, height))
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseOnMove(struct TbPoint dstPos)
{
    if ((!lbMouseSpriteInstalled) || (lbMouseOffline))
    {
        return Lb_FAIL;
    }

    if (!pointerHandler.SetMousePosition(dstPos.x, dstPos.y))
    {
        return Lb_FAIL;
    }

   // SDL_WarpMouseInWindow(lbScreenWindow, lbDisplay.MMouseX, lbDisplay.MMouseY);
    return Lb_SUCCESS;
}

/** Converts mouse coordinates into relative and scaled coordinates.
 *
 * @param pos Pointer to the structure with source point, and where result is placed.
 */
TbPoint ScaleMouseMove(struct TbPoint posDelta)
{
    TbPoint scaledMouseMove;

    if (!lbUseRelativeMouseMode)
    {
        ERRORLOG("Mouse ratio is only valid in relative mouse mode.");
        return scaledMouseMove;
    }

    // Scale coordinate
    scaledMouseMove.x = (long)((float)(posDelta.x)* (float)lbDisplay.MouseMoveRatio / DEFAULT_MOUSE_MOVE_RATIO);
    scaledMouseMove.y = (long)((float)(posDelta.y)* (float)lbDisplay.MouseMoveRatio / DEFAULT_MOUSE_MOVE_RATIO);

    return scaledMouseMove;
}

TbResult LbMouseSuspend(void)
{
  if (!lbMouseSpriteInstalled)
    return Lb_FAIL;
  if (!pointerHandler.Release())
    return Lb_FAIL;
  return Lb_SUCCESS;
}

TbResult LbMouseOnBeginSwap(void)
{
    if (!pointerHandler.PointerBeginSwap())
        return Lb_FAIL;
    return Lb_SUCCESS;
}

TbResult LbMouseOnEndSwap(void)
{
    if (!pointerHandler.PointerEndSwap())
        return Lb_FAIL;
    return Lb_SUCCESS;
}

void _get_mouse_state(TbPoint *positionDelta, TbPoint *destination)
{
    // TODO HeM error handling.
    int x, y;
    struct TbPoint scaledMove;

    if (lbUseRelativeMouseMode)
    {
        SDL_GetRelativeMouseState(&x, &y);

        positionDelta->x = x;
        positionDelta->y = y;

        // Scale the Delta to Sprite position.
        scaledMove = ScaleMouseMove(*positionDelta);
        positionDelta->x = scaledMove.x;
        positionDelta->y = scaledMove.y;

        destination->x = lbDisplay.MMouseX + positionDelta->x;
        destination->y = lbDisplay.MMouseY + positionDelta->y;
    }
    else
    {
        SDL_GetMouseState(&x, &y);

        positionDelta->x = x - lbDisplay.MMouseX;
        positionDelta->y = y - lbDisplay.MMouseY;

        destination->x = x;
        destination->y = y;
    }
}

void mouseControl(unsigned int action)
{
    struct TbPoint mousePosDelta;
    struct TbPoint dstPos;

    bool isCtrlDown = lbInkeyFlags & KMod_CONTROL;

    // Used instead of lpDisplay.LButton when ctrl is pressed.
    static bool isCtrlAndLeftButtonDown = false;
    static bool isCtrlAndRightButtonDown = false;

    _get_mouse_state(&mousePosDelta, &dstPos);

    switch ( action )
    {
    case MActn_MOUSEMOVE:
        // Drag to move camera when ctrl is pressed, or there is nothing else to do.
        if (isCtrlAndLeftButtonDown /*|| (lbDisplay.MLeftButton && lbDisplayEx.isPowerHandNothingTodoLeftClick)*/)
        {
            lbDisplayEx.cameraMoveX += mousePosDelta.x * lbDisplayEx.cameraMoveRatioX;
            lbDisplayEx.cameraMoveY += mousePosDelta.y * lbDisplayEx.cameraMoveRatioY;
            //SYNCLOG("SET X MOVE %d", lbDisplayEx.cameraMoveX);
        }
        // Right drag to rotate camera when ctrl is pressed, or there is nothing else to do.
        else if (isCtrlAndRightButtonDown /*|| (lbDisplay.MRightButton && lbDisplayEx.isPowerHandNothingTodoRightClick)*/)
        {   
            // TODO HeM recalculate rotate angle to be more responsive to mouse movement.
            lbDisplayEx.cameraRotateAngle += mousePosDelta.x / 3;
        }

        // Normal mouse move
        LbMouseOnMove(dstPos);

        break;
    case MActn_LBUTTONDOWN:
        lbDisplay.MLeftButton = 1;
        LbMouseOnMove(dstPos);
        if (!lbDisplay.LeftButton && !isCtrlAndLeftButtonDown)
        {
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 0;

            if (!isCtrlDown)
            {
                lbDisplay.LeftButton = 1;
            }
            else
            {
                isCtrlAndLeftButtonDown = true;
            }
        }
        break;
    case MActn_LBUTTONUP:
        lbDisplay.MLeftButton = 0;
        LbMouseOnMove(dstPos);
        if ( !lbDisplay.RLeftButton )
        {
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RLeftButton = 1;

            isCtrlAndLeftButtonDown = false;
        }
        break;
    case MActn_RBUTTONDOWN:
        lbDisplay.MRightButton = 1;
        LbMouseOnMove(dstPos);
        if (!lbDisplay.RightButton  && !isCtrlAndRightButtonDown)
        {
            lbDisplay.MouseX = lbDisplay.MMouseX;
            lbDisplay.MouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 0;

            if (!isCtrlDown)
            {
                lbDisplay.RightButton = 1;
            }
            else
            {
                isCtrlAndRightButtonDown = true;
            }
        }
        break;
    case MActn_RBUTTONUP:
        lbDisplay.MRightButton = 0;
        LbMouseOnMove(dstPos);
        if ( !lbDisplay.RRightButton )
        {
            lbDisplay.RMouseX = lbDisplay.MMouseX;
            lbDisplay.RMouseY = lbDisplay.MMouseY;
            lbDisplay.RRightButton = 1;

            isCtrlAndRightButtonDown = false;
        }
        break;
    case MActn_WHEELUP:
        // Zooms in when wheel up.
        lbDisplayEx.wheelUp = true;
        break;
    case MActn_WHEELDOWN:
        // Zooms out when wheel down.
        lbDisplayEx.wheelDown = true;
        break;
    default:
        break;
    }
}

/**
 * Changes mouse movement ratio.
 * Note that this function can be run even before mouse setup. Still, the factor
 *  will be reset during the installation - so use it after LbMouseSetup().
 *
 * @param ratio_x Movement ratio in X direction; 256 means unchanged ratio from OS.
 * @param ratio_y Movement ratio in Y direction; 256 means unchanged ratio from OS.
 * @return Lb_SUCCESS if the ratio values were of correct range and have been set.
 */
TbResult LbMouseChangeMoveRatio(long ratio_x, long ratio_y)
{
    if ((ratio_x < -8192) || (ratio_x > 8192) || (ratio_x == 0))
        return Lb_FAIL;
    if ((ratio_y < -8192) || (ratio_y > 8192) || (ratio_y == 0))
        return Lb_FAIL;
    SYNCLOG("New ratio %ldx%ld", ratio_x , ratio_y);
    // Currently we don't have two ratio factors, so let's store an average
    lbDisplay.MouseMoveRatio = (ratio_x + ratio_y)/2;
    //TODO INPUT Separate mouse ratios in X and Y direction when lbDisplay from DLL will no longer be used.
    //minfo.XMoveRatio = ratio_x;
    //minfo.YMoveRatio = ratio_y;
    return Lb_SUCCESS;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif

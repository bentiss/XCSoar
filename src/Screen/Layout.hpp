/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2015 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_SCREEN_LAYOUT_HPP
#define XCSOAR_SCREEN_LAYOUT_HPP

#include "Screen/Point.hpp"
#include "Asset.hpp"
#include "Compiler.h"

namespace Layout
{
  extern bool landscape;

  /**
   * Fixed-point scaling factor, fractional part is 10 bits (factor
   * 1024).
   */
  extern unsigned scale_1024;
  extern unsigned scale;

  /**
   * Fixed-point scaling factor for on-screen objects which don't grow
   * linearly with the screen resolution.
   */
  extern unsigned small_scale;

  extern unsigned pen_width_scale;

  /**
   * Recommended padding from Window boundary to text.
   */
  extern unsigned text_padding;

  extern unsigned minimum_control_height, maximum_control_height;

  extern unsigned hit_radius;

  /**
   * Initializes the screen layout information provided by this
   * namespace.
   *
   * @param screen_size the size of the screen in pixels
   */
  void Initialize(PixelSize screen_size);

  /**
   * Is scaling supported by this platform?
   */
  gcc_const
  static inline bool
  ScaleSupported()
  {
    return !IsAltair();
  }

  /**
   * Is scaling enabled currently?
   */
  gcc_const
  static inline bool
  ScaleEnabled()
  {
    return ScaleSupported() && scale_1024 > 1024;
  }

  gcc_const
  static inline int
  Scale(int x)
  {
    if (!ScaleSupported())
      return x;

    return (x * int(scale_1024)) >> 10;
  }

  gcc_const
  static inline unsigned
  Scale(unsigned x)
  {
    if (!ScaleSupported())
      return x;

    return (x * scale_1024) >> 10;
  }

#ifdef USE_GDI
  gcc_const
  static inline int
  Scale(PixelScalar x)
  {
    return Scale(int(x));
  }

  gcc_const
  static inline int
  Scale(UPixelScalar x)
  {
    return Scale(unsigned(x));
  }
#endif

  gcc_const
  static inline int
  FastScale(int x)
  {
    if (!ScaleSupported())
      return x;

    return x * int(scale);
  }

  gcc_const
  static inline unsigned
  FastScale(unsigned x)
  {
    if (!ScaleSupported())
      return x;

    return x * scale;
  }

#ifdef USE_GDI
  gcc_const
  static inline int
  FastScale(PixelScalar x)
  {
    return FastScale(int(x));
  }

  gcc_const
  static inline int
  FastScale(UPixelScalar x)
  {
    return FastScale(unsigned(x));
  }
#endif

  gcc_const
  static inline int
  SmallScale(int x)
  {
    if (!ScaleSupported())
      return x;

    return (x * (int)small_scale) >> 10;
  }

  gcc_const
  static inline unsigned
  ScalePenWidth(unsigned width)
  {
    if (!ScaleSupported())
      return width;

    return (width * pen_width_scale) >> 10;
  }

  /**
   * Scale a vertical dimension value according to the aspect ratio of
   * the display, to work around non-square pixels.  An ellipsis with
   * the pixel width "x" and the pixel height "ScaleY(x)" shall be a
   * circle.
   */
  gcc_const
  static inline int
  ScaleY(int y)
  {
    return y;
  }

  gcc_const
  static inline unsigned
  GetTextPadding()
  {
    if (!ScaleSupported())
      return 2;

    return text_padding;
  }

  /**
   * Returns the minimum height of an dialog control.
   */
  gcc_pure
  static inline unsigned
  GetMinimumControlHeight()
  {
    if (IsAltair())
      return 22;

    return minimum_control_height;
  }

  /**
   * Returns the maximum useful height of a dialog control.
   */
  gcc_pure
  static inline unsigned
  GetMaximumControlHeight()
  {
    if (IsAltair())
      return 22;

    return maximum_control_height;
  }

  /**
   * Returns the radius (in pixels) of the hit circle around map
   * items.
   */
  gcc_pure
  static inline unsigned
  GetHitRadius()
  {
    if (!HasPointer())
      return 0;

    return hit_radius;
  }
}

#endif

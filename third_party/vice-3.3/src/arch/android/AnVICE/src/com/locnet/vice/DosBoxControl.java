/*
 * DosBoxControl.java
 *
 * Written by
 *  Locnet <android.locnet@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

package com.locnet.vice;

public class DosBoxControl {
    public static native void nativeMouse(int x, int y, int down_x, int down_y, int action, int button);
    public static native int nativeKey(int keyCode, int down, int ctrl, int alt, int shift);
    public static native void nativeJoystick(int x, int y, int action, int button);

    //return true to clear modifier
    public static boolean sendNativeKey(int keyCode, boolean down, boolean ctrl, boolean alt, boolean shift) {
        boolean result = false;

        if (nativeKey(keyCode, (down) ? 1 : 0, (ctrl) ? 1 : 0, (alt) ? 1 : 0, (shift) ? 1 : 0) != 0) {
            if (!down) {
                result = true;
            }
        }
        return result;
    }
}

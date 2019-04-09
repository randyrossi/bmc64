/*
 * ROMImportView.java
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

package org.ab.uae;

import com.locnet.vice.R;

import java.io.File;
import java.util.ArrayList;

import org.ab.nativelayer.ImportFileView;

import android.app.Application;

public class RomImportView extends ImportFileView {

    public RomImportView() {
        super(new String [] {} );
        virtualDir = false;
    }

    private static final long serialVersionUID = -8756086087950123786L;

    @Override
    public ArrayList<String> getExtendedList(Application application, File file) {
        return null;
    }

    @Override
    public String getExtra2(int position) {
        return null;
    }

    @Override
    public int getIcon(int position) {
        return R.drawable.file;
    }
}

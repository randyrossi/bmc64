/*
 * ImportFileView.java
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

package org.ab.nativelayer;

import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;

import android.app.Application;

public abstract class ImportFileView implements Serializable {

    private static final long serialVersionUID = 6009558385710619718L;
    private ArrayList<String> extensions;
    protected boolean virtualDir = true;

    public ImportFileView(String ext []) {
        if (ext != null && ext.length > 0) {
            extensions = new ArrayList<String>();
            for (int i = 0; i < ext.length; i++) {
                extensions.add(ext[i]);
            }
        }
    }

    public ArrayList<String> getExtensions() {
        return extensions;
    }

    public abstract ArrayList<String> getExtendedList(Application application, File file);

    public abstract String getExtra2(int position);

    public abstract int getIcon(int position);
}

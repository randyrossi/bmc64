/*
 * ImportView.java
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

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.locnet.vice.PreConfig;
import com.locnet.vice.R;

public class ImportView extends ListActivity {
    /**
     * text we use for the parent directory
     */
    private final static String PARENT_DIR = "..";
    /**
     * Currently displayed files
     */
    private final List<String> currentFiles = new ArrayList<String>();
    /**
     * Currently displayed directory
     */
    private File currentDir = null;
    private ImportFileView importView;

    @Override
    public void onCreate(final Bundle icicle) {
        super.onCreate(icicle);

        // go to the root directory
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        String last_dir = sp.getString("last_dir", "/sdcard");
        importView = (ImportFileView) getIntent().getSerializableExtra("import");
        String specificDir = (String) getIntent().getStringExtra("specificDir");
        if (specificDir != null && new File(specificDir).exists()) {
            showDirectory(specificDir);
        } else {
            showDirectory(last_dir);
        }
    }

    private File currentSelectedFile;

    @Override
    protected void onListItemClick(final ListView l, final View v, final int position, final long id) {
        if (position == 0 && PARENT_DIR.equals(this.currentFiles.get(0))) {
            if (currentSelectedFile != null) {
                currentSelectedFile = null;
                showDirectory(this.currentDir.getAbsolutePath());
            } else {
                showDirectory(this.currentDir.getParent());
            }
        } else {
            if (currentSelectedFile != null) {
                String extra = this.currentFiles.get(position);
                if (importView != null) {
                    String extra2 = importView.getExtra2(position);
                    if (extra2 != null) {
                        extra = extra2;
                    }
                }
                selectFile(currentSelectedFile, extra, position);
            }

            final File file = new File(this.currentFiles.get(position));

            if (file.isDirectory()) {
                currentSelectedFile = null;
                showDirectory(file.getAbsolutePath());
            } else {
                if (importView != null && importView.virtualDir) {
                    currentSelectedFile = file;
                    this.currentFiles.clear();
                    this.currentFiles.add(PARENT_DIR);
                    ArrayList<String> list = importView.getExtendedList(getApplication(), currentSelectedFile);
                    if (list != null) {
                        this.currentFiles.addAll(list);
                    }
                    showList();
                } else if (file.exists()) {
                    selectFile(file, null, 0);
                }
            }
        }
    }
    private void selectFile(File file, String extra1, int extra2) {
        final Intent extras = new Intent();
        extras.putExtra("currentFile", file.getAbsolutePath());
        if (extra1 != null) {
            extras.putExtra("extra1", extra1);
        }
        extras.putExtra("extra2", extra2);
        setResult(RESULT_OK, extras);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        Editor e = sp.edit();
        e.putString("last_dir", file.getParent());
        e.commit();
        finish();
    }

    /**
     * Show the contents of a given directory as a selectable list
     *
     * @param path      the directory to display
     */
    private void showDirectory(final String path) {
        // we clear any old content and add an entry to get up one level
        this.currentFiles.clear();
        this.currentDir = new File(path);
        if (this.currentDir.getParentFile() != null) {
            this.currentFiles.add(PARENT_DIR);
        }

        // get all directories and relevant files in the given path
        final File[] files = this.currentDir.listFiles();
        final ArrayList<String> sorted = new ArrayList<String>();
        if (files != null) {
            for (final File file : files) {
                final String name = file.getAbsolutePath();
                if (file.isDirectory()) {
                    sorted.add(name);
                } else {
                    if (importView != null) {
                        String ext = null;
                        if (name.length() > 3) {
                            ext = name.substring(name.length() - 3).toLowerCase();
                        }
                        if (importView.getExtensions() == null || importView.getExtensions().contains(ext)) {
                            sorted.add(name);
                        }
                    } else {
                        sorted.add(name);
                    }
                }
            }
        }
        Collections.sort(sorted, String.CASE_INSENSITIVE_ORDER);
        this.currentFiles.addAll(sorted);

        showList();
    }

    private void showList() {
        // display these images
        final Context context = this;

        ArrayAdapter<String> filenamesAdapter = new ArrayAdapter<String>(this, R.layout.file_row, this.currentFiles) {
            @Override
            public View getView(final int position, final View convertView, final ViewGroup parent) {
                return new IconifiedTextLayout(context, getItem(position), position);
            }
        };

        setListAdapter(filenamesAdapter);
    }

    // new layout displaying a text and an associated image
    class IconifiedTextLayout extends LinearLayout {

        public IconifiedTextLayout(final Context context, final String path, final int position) {
            super(context);

            setOrientation(HORIZONTAL);

            // determine icon to display
            final ImageView imageView = new ImageView(context);
            final File file = new File(path);

            if (position == 0 && PARENT_DIR.equals(path)) {
                imageView.setImageResource(R.drawable.folder);
            } else {
                if (file.exists()) {
                    if (file.isDirectory()) {
                        imageView.setImageResource(R.drawable.folder);
                    } else {
                        if ((importView.getExtensions() != null) || file.getName().equalsIgnoreCase(PreConfig.KERNAL_NAME) || file.getName().equalsIgnoreCase(PreConfig.BASIC_NAME) || file.getName().equalsIgnoreCase(PreConfig.CHARGEN_NAME)) {
                            imageView.setImageResource(R.drawable.icon32);
                        } else {
                            imageView.setImageResource(R.drawable.file);
                        }
                    }
                } else {
                    int r = R.drawable.icon;

                    if (importView != null) {
                        r = importView.getIcon(position);
                    }
                    imageView.setImageResource(r);
                }
            }
            imageView.setPadding(0, 3, 5, 3);

            // create view for the directory name
            final TextView textView = new TextView(context);

            textView.setText(file.getName());
            textView.setTextSize(18);

            addView(imageView, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
            addView(textView, new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
        }
    }
}

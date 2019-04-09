/*
 * DosBoxAudio.java
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

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class DosBoxAudio
{
    private DosBoxLauncher mParent = null;
    private boolean mAudioRunning = true;
    private AudioTrack mAudio = null;
    private int mChannelCount;

    public short[] mAudioBuffer = null;

    private DosBoxAudioThread mAudioThread = null;

    class DosBoxAudioThread extends Thread {
        public void run() {
            while (mAudioRunning) {
                if (mParent.mDosBoxThread.mDosBoxRunning && (mParent.mAudioDevice.mAudioBuffer != null)) {
                    long now = System.currentTimeMillis();
                    boolean bufferUnderRun = false;
                    synchronized(mAudioBuffer) {
                        int size = DosBoxLauncher.nativeAudioWriteBuffer(mParent.mAudioDevice.mAudioBuffer);
                        if (size > 0) {
                            mParent.callbackAudioWriteBuffer(size);
                        } else {
                            bufferUnderRun = true;
                        }
                    }
                    if (bufferUnderRun) {
                        try {
                            Thread.sleep(10);
                        }
                        catch (InterruptedException e) {
                        }
                        continue;
                    }
                    try {
                        long sleepTime = (mParent.mAudioDevice.mAudioMinUpdateInterval - (System.currentTimeMillis() - now)) / 2;
                        if (sleepTime < 10) {
                            sleepTime = 10;
                        }
                        Thread.sleep(sleepTime);
                    }
                    catch (InterruptedException e) {
                    }
                } else {
                    try {
                        Thread.sleep(1000);
                    }
                    catch (InterruptedException e) {
                    }
                }
            }
        }
    }

    DosBoxAudio(DosBoxLauncher context) {
        mParent = context;
        mAudioThread = new DosBoxAudioThread();
    }

    public int initAudio(int rate, int channels, int encoding, int bufSize) {
        if (mAudio != null) {
            synchronized(mAudioBuffer) {	//2011-09-07, try fixing bug on loading savestate
                mAudio.release();
                mAudio = null;
                mAudioBuffer = null;
            }
        }
        if (mAudio == null) {
            int bufSize2 = bufSize;

            channels = (channels == 1) ? AudioFormat.CHANNEL_CONFIGURATION_MONO : AudioFormat.CHANNEL_CONFIGURATION_STEREO;
            encoding = (encoding == 1) ? AudioFormat.ENCODING_PCM_16BIT : AudioFormat.ENCODING_PCM_8BIT;

            if (AudioTrack.getMinBufferSize(rate, channels, encoding) > bufSize) {
                bufSize2 = AudioTrack.getMinBufferSize(rate, channels, encoding);
                bufSize2 = Math.max(bufSize2, bufSize << 3);
            }

            mAudioMinUpdateInterval = 1000 * (bufSize >> 1) / ((channels == AudioFormat.CHANNEL_CONFIGURATION_MONO) ? 1 : 2) / rate;

            mAudioBuffer = new short[bufSize >> 1];
            mAudio = new AudioTrack(AudioManager.STREAM_MUSIC, rate, channels, encoding, bufSize2, AudioTrack.MODE_STREAM);
            mAudio.pause();

            mChannelCount = mAudio.getChannelCount();

            if (!mAudioThread.isAlive()) {
                mAudioThread.start();
            }
            return bufSize;
        }
        if (mAudioBuffer != null) {
            return bufSize;
        } else {
            return 0;
        }
    }

    public void shutDownAudio() {
        if (mAudio != null) {
            mAudio.stop();
            mAudio.release();
            mAudio = null;
        }
        mAudioBuffer = null;
    }

    private long mLastWriteBufferTime = 0;
    public int mAudioMinUpdateInterval = 50;

    public void AudioWriteBuffer(int size) {
        if ((mAudioBuffer != null) && mAudioRunning) {
            long now = System.currentTimeMillis();
            if ((!mParent.mTurboOn) || ((now - mLastWriteBufferTime) > mAudioMinUpdateInterval)) {
                if (size > 0) {
                    writeSamples(mAudioBuffer, size * mChannelCount);
                }
                mLastWriteBufferTime = now;
            }
        }
    }

    public void setRunning() {
        mAudioRunning = !mAudioRunning;
        if (!mAudioRunning) {
            mAudio.pause();
        }

        if (mAudioRunning) {
            mAudioThread.start();
        }
    }

    public void writeSamples(short[] samples, int size) {
        if (mAudioRunning) {
            if (mAudio != null) {
                mAudio.write(samples, 0, size);

                if (mAudio.getPlayState() != AudioTrack.PLAYSTATE_PLAYING) {
                    play();
                }
            }
        }
    }

    public void play() {
        if (mAudio != null) {
            mAudio.play();
        }
    }

    public void pause() {
        if (mAudio != null) {
            mAudio.pause();
        }
    }
}

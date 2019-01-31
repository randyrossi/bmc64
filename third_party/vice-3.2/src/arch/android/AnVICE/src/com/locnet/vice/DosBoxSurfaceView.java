/*
 * DosBoxSurfaceView.java
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

import java.nio.Buffer;
import java.nio.ByteBuffer;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.os.Handler;
import android.os.Message;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

public class DosBoxSurfaceView extends SurfaceView implements SurfaceHolder.Callback {
    public final static int MOUSE_ACTION_DOWN = 0;
    public final static int MOUSE_ACTION_UP = 1;
    public final static int MOUSE_ACTION_MOVE = 2;

    private final static int DEFAULT_WIDTH = 640;	//800;
    private final static int DEFAULT_HEIGHT = 400;	//600; 

    private final static int ONSCREEN_BUTTON_WIDTH = 3;
    private final static int ONSCREEN_BUTTON_HEIGHT = 5;

    private final static int BUTTON_TAP_TIME_MIN = 10;
    private final static int BUTTON_TAP_TIME_MAX = 300;	
    public final static int BUTTON_TAP_DELAY = 200;	
    public final static int BUTTON_JOYSTICK_TAP_DELAY = 50;	
    private final static int BUTTON_REPEAT_DELAY = 100;	

    public final static int INPUT_MODE_MOUSE = 1;
    public final static int INPUT_MODE_SCROLL = 2;
    public final static int INPUT_MODE_JOYSTICK = 3;
    public final static int INPUT_MODE_HYBRID_MOUSE = 4;
    public final static int INPUT_MODE_PEN = 5;

    public final static int SCALE_MODE_CUSTOM = 1;
    public final static int SCALE_MODE_FIT_SCREEN = 2;
    public final static int SCALE_MODE_FIT_WIDTH = 3;
    public final static int SCALE_MODE_FIT_HEIGHT = 4;
    public final static int SCALE_MODE_FULLSCREEN = 5;

    private DosBoxLauncher mParent = null;	
    private boolean mSurfaceViewRunning = false;
    public DosBoxVideoThread mVideoThread = null;
    public KeyHandler mKeyHandler = null;
    public Buffer mVideoBuffer = null;		

    public int mInputMode = INPUT_MODE_MOUSE;
    public boolean mScale = true;
    public boolean mShowInfo = false;
    private int mAbsoluteMouseButton = 0;
    private boolean mAbsoluteMouseClicked = false;
    private boolean mDisableAbsoluteClick = false;

    int mContextMenu = 0;

    Bitmap mBitmap = null;
    private Paint mBitmapPaint = null;
    private Paint mTextPaint = null;
    private Rect mSrcRect = new Rect();
    private Rect mDstRect = new Rect();
    private Rect mDirtyRect = new Rect();	
    private Rect mScreenRect = new Rect();

    int mSrc_width = 0;
    int mSrc_height = 0;	

    private int mDirtyCount = 0;
    private int mScroll_x = 0;
    private int mScroll_y = 0;	

    Boolean mDirty = false;
    int mStartLine = 0;
    int mEndLine = 0;

    boolean mModifierCtrl = false;
    boolean mModifierAlt = false;
    boolean mModifierShift = false;

    class KeyHandler extends Handler {
        boolean mReCheck = false;

        @Override
        public void handleMessage (Message msg) {
            switch (msg.what) {
                case DosBoxLauncher.SPLASH_TIMEOUT_MESSAGE:
                    setBackgroundResource(0);

                    //locnet, 2011-05-30, initialize numlock to on
                    break;
                case DosBoxLauncher.SHOW_IM_PICKER_MESSAGE:
                    mButtonDown = ONSCREEN_BUTTON_SHOW_IM_PICKER;
                    DosBoxMenuUtility.doShowInputMethodPicker(mParent);
                    break;
                case DosBoxLauncher.TURBO_MESSAGE:
                    mButtonDown = ONSCREEN_BUTTON_TURBO;
                    mParent.mTurboOn = true;
                    DosBoxLauncher.nativeSetOption(DosBoxMenuUtility.DOSBOX_OPTION_ID_TURBO_ON, 1, null);
                    break;
                case DosBoxLauncher.RESET_JOY_BUTTON_MESSAGE:
                    DosBoxControl.nativeJoystick(0, 0, 1, 0);
                    break;
                case DosBoxLauncher.RESET_JOY_DIRECTION_MESSAGE:
                    DosBoxControl.nativeJoystick(0, 0, 2, -1);
                    break;
                default:
                    switch (msg.what) {
                        default:
                            if (DosBoxControl.sendNativeKey(msg.what, false, mModifierCtrl, mModifierAlt, mModifierShift)) {
                                mModifierCtrl = false;
                                mModifierAlt = false;
                                mModifierShift = false;
                            }
                    }
                    break;
            }
        }
    }

    class DosBoxVideoThread extends Thread {
        private static final int UPDATE_INTERVAL = 20;	//40;
        private static final int UPDATE_INTERVAL_MIN = 10;	//20;
        private static final int RESET_INTERVAL = 100;

        private boolean mVideoRunning = false;

        private long startTime = 0;
        private int frameCount = 0;

        void setRunning(boolean running) {
            mVideoRunning = running;
        }

        public void run() {
            mVideoRunning = true;
            while (mVideoRunning) {
                if (mSurfaceViewRunning) {
                    long curTime = System.currentTimeMillis();

                    if (frameCount > RESET_INTERVAL) {
                        frameCount = 0;
                    }

                    if (frameCount == 0) {
                        startTime = curTime - UPDATE_INTERVAL;
                    }

                    frameCount++;

                    synchronized (mDirty) {
                        if (mDirty) {
                            VideoRedraw(mBitmap, mSrc_width, mSrc_height, mStartLine, mEndLine);
                            mDirty = false;
                        }
                    }

                    //locnet, 2012-01-20, add option for more smooth video
                    try {
                        if (!mParent.mPrefBetterVideoOn) {
                            long nextUpdateTime = startTime + (frameCount + 1) * UPDATE_INTERVAL;
                            long sleepTime = nextUpdateTime - System.currentTimeMillis();
                            Thread.sleep(Math.max(sleepTime, UPDATE_INTERVAL_MIN));
                        } else {
                            Thread.sleep(1);
                        }
                    }
                    catch (InterruptedException e) {
                    }
                } else {
                    try {
                        frameCount = 0;
                        Thread.sleep(1000);
                    }
                    catch (InterruptedException e) {
                    }
                }
            }
        }
    }

    public DosBoxSurfaceView(DosBoxLauncher context) {
        super(context);
        initSurfaceView(context);
    }

    private void initSurfaceView(DosBoxLauncher context) {
        mParent = context;

        mBitmapPaint = new Paint();
        mBitmapPaint.setFilterBitmap(true);		

        mTextPaint = new Paint();
        mTextPaint.setTextSize(15 * getResources().getDisplayMetrics().density);
        mTextPaint.setTypeface(Typeface.DEFAULT_BOLD);
        mTextPaint.setTextAlign(Paint.Align.CENTER);
        mTextPaint.setStyle(Paint.Style.FILL);
        mTextPaint.setSubpixelText(false);

        mBitmap = Bitmap.createBitmap(DEFAULT_WIDTH, DEFAULT_HEIGHT, Bitmap.Config.RGB_565);

        //locnet, 2011-04-28, support 2.1 or below
        mVideoBuffer = ByteBuffer.allocateDirect(DEFAULT_WIDTH * DEFAULT_HEIGHT * 2);

        mVideoThread = new DosBoxVideoThread();
        mKeyHandler = new KeyHandler(); 				

        // Receive keyboard events
        setFocusableInTouchMode(true);
        setFocusable(true);

        getHolder().addCallback(this);
        getHolder().setFormat(PixelFormat.RGB_565);
        getHolder().setKeepScreenOn(true);

        setJoystickRect();
    }

    public void shutDown() {
        mBitmap = null;
        mVideoThread = null;
        mKeyHandler = null;		
    }

    public void VideoRedraw(Bitmap bitmap, int src_width, int src_height, int startLine, int endLine) {
        if (!mSurfaceViewRunning || (bitmap == null) || (src_width <= 0) || (src_height <= 0)) {
            return;
        }

        SurfaceHolder surfaceHolder = getHolder();
        Canvas canvas = null;

        try {
            synchronized (surfaceHolder) {
                int dst_width = getWidth();
                int dst_height = getHeight();
                boolean isDirty = false;
                boolean isLandscape = (dst_width > dst_height);

                if (mShowInfo || (mParent.mPrefAlwaysShowJoystick && (mInputMode == INPUT_MODE_JOYSTICK))) {
                    mDirtyCount = 0;
                }

                if (mDirtyCount < 3) {
                    mDirtyCount++;
                    isDirty =  true;
                    startLine = 0;
                    endLine = src_height;
                }

                if (mScale) {
                    int tmp = src_width * dst_height /src_height;

                    if (isLandscape) {
                        switch (mParent.mPrefScaleMode) {
                            case SCALE_MODE_FIT_WIDTH:
                                dst_height = src_height * dst_width / src_width;
                                break;
                            case SCALE_MODE_FIT_HEIGHT:
                                dst_width = src_width * dst_height /src_height;
                                break;
                            case SCALE_MODE_FULLSCREEN:
                                break;
                            case SCALE_MODE_FIT_SCREEN:
                            case SCALE_MODE_CUSTOM:
                            default:
                                if (tmp < dst_width) {
                                    dst_width = tmp;
                                } else if (tmp > dst_width) {
                                    dst_height = src_height * dst_width /src_width;
                                }
                                if (mParent.mPrefScaleMode != SCALE_MODE_FIT_SCREEN) {
                                    dst_width = dst_width * mParent.mPrefScaleFactor / 100;
                                    dst_height = dst_height * mParent.mPrefScaleFactor / 100;
                                }
                                break;
                        }
                    } else {
                        if (tmp < dst_width) {
                            dst_width = tmp;
                        } else if (tmp > dst_width) {
                            dst_height = src_height * dst_width / src_width;
                        }
                    }

                    int tmpX = (getWidth() - dst_width) / 2;
                    int tmpY = 0;

                    if ((isLandscape) && (mParent.mPrefScaleMode == SCALE_MODE_FIT_WIDTH) && (dst_height > getHeight())) {
                        tmpY = (getHeight() - dst_height) / 2;
                    }

                    mSrcRect.set(0, 0, src_width, src_height);
                    mDstRect.set(0, 0, dst_width, dst_height);
                    mDstRect.offset(tmpX, tmpY);

                    //locnet, 2011-08-08, screen glitch (amberstart)
                    mDirtyRect.set(0, startLine * dst_height / src_height - 1, dst_width, endLine * dst_height / src_height + 1);
                    if (mDirtyRect.top < 0) {
                        mDirtyRect.top = 0;
                    }

                    //locnet, 2011-04-21, a strip on right side not updated
                    mDirtyRect.offset(tmpX, tmpY);

                    //locnet, 2011-06-10, for absolute mouse
                    mScreenRect.set(mDstRect);
                } else {
                    if ((mScroll_x + src_width) < dst_width) {
                        mScroll_x = dst_width - src_width;
                    }

                    if ((mScroll_y + src_height) < dst_height) {
                        mScroll_y = dst_height - src_height;
                    }

                    mScroll_x = Math.min(mScroll_x, 0);
                    mScroll_y = Math.min(mScroll_y, 0);

                    mSrcRect.set(-mScroll_x, Math.max(-mScroll_y, startLine), Math.min(dst_width - mScroll_x, src_width), Math.min(Math.min(dst_height - mScroll_y, src_height), endLine));

                    dst_width = mSrcRect.width();
                    dst_height = mSrcRect.height();

                    mDstRect.set(0, mSrcRect.top + mScroll_y, dst_width, mSrcRect.top + mScroll_y + dst_height);

                    mDstRect.offset((getWidth() - dst_width) / 2, 0);

                    mDirtyRect.set(mDstRect);

                    //locnet, 2011-06-10, for absolute mouse
                    mScreenRect.set(0, 0, src_width, src_height);
                    mScreenRect.offset(mScroll_x, mScroll_y);
                    mScreenRect.offset((getWidth() - dst_width) / 2, 0);
                }

                if (isDirty) {
                    canvas = surfaceHolder.lockCanvas(null);
                    canvas.drawColor(0xff202020);
                } else {
                    canvas = surfaceHolder.lockCanvas(mDirtyRect);
                }

                //locnet, 2011-04-28, support 2.1 or below
                if (mVideoBuffer != null) {
                    mVideoBuffer.position(0);
                    //locnet, 2012-01-23, ensure buffer is correct
                    if (bitmap.getWidth() * bitmap.getHeight() * 2 == mVideoBuffer.remaining()) {
                        bitmap.copyPixelsFromBuffer(mVideoBuffer);
                    }
                }

                if (mScale) {
                    canvas.drawBitmap(bitmap, mSrcRect, mDstRect, (mParent.mPrefScaleFilterOn) ? mBitmapPaint : null);
                } else {
                    canvas.drawBitmap(bitmap, mSrcRect, mDstRect, null);
                }

                int screen_width = getWidth();
                int screen_height = getHeight();

                if ((mShowInfo || mParent.mPrefAlwaysShowJoystick) && (mInputMode == INPUT_MODE_JOYSTICK)) {
                    drawButton(canvas, mJoystickRect.left, mJoystickRect.top, mJoystickRect.right, mJoystickRect.bottom, "[+]");
                    drawButton(canvas, screen_width / ONSCREEN_BUTTON_WIDTH, 0, screen_width * 2 / ONSCREEN_BUTTON_WIDTH, screen_height / ONSCREEN_BUTTON_HEIGHT, "Btn 2");
                    drawButton(canvas, screen_width * 2 / ONSCREEN_BUTTON_WIDTH, 0, screen_width, screen_height / ONSCREEN_BUTTON_HEIGHT, "Btn 1");
                }

                if (mShowInfo) {
                    drawButton(canvas, 0, 0, screen_width / ONSCREEN_BUTTON_WIDTH, screen_height / ONSCREEN_BUTTON_HEIGHT, "Special");

                    if (mInputMode != INPUT_MODE_JOYSTICK) {
                        drawButton(canvas, screen_width / ONSCREEN_BUTTON_WIDTH, 0, screen_width * 2 / ONSCREEN_BUTTON_WIDTH, screen_height / ONSCREEN_BUTTON_HEIGHT, "Btn 1");
                        drawButton(canvas, screen_width * 2 / ONSCREEN_BUTTON_WIDTH, 0, screen_width, screen_height / ONSCREEN_BUTTON_HEIGHT, "Btn 2");
                    }

                    drawButton(canvas, 0, screen_height * (ONSCREEN_BUTTON_HEIGHT - 1) / ONSCREEN_BUTTON_HEIGHT, screen_width / ONSCREEN_BUTTON_WIDTH, screen_height, "Hide");
                    drawButton(canvas, screen_width / ONSCREEN_BUTTON_WIDTH, screen_height * (ONSCREEN_BUTTON_HEIGHT - 1) / ONSCREEN_BUTTON_HEIGHT, screen_width * 2 / ONSCREEN_BUTTON_WIDTH, screen_height, "Menu");
                    drawButton(canvas, screen_width * 2 / ONSCREEN_BUTTON_WIDTH, screen_height * (ONSCREEN_BUTTON_HEIGHT - 1) / ONSCREEN_BUTTON_HEIGHT, screen_width, screen_height, "Keyboard");
                }
            }
        }
        finally {
            if (canvas != null) {
                surfaceHolder.unlockCanvasAndPost(canvas);
            }
        }

        surfaceHolder = null;
    }

    RectF mButtonRect = new RectF();
    Rect mJoystickRect = new Rect(0, 0, 1, 1);

    void setJoystickRect() {
        int screen_width = getWidth();
        int screen_height = getHeight();
        int width;
        int height;
        int bottom;

        if (isLandscape(this)) {
            width = screen_height / 2;
        } else {
            width = screen_width / 2;
        }

        if (mParent.mPrefLargeJoystick) {
            width = screen_width;
        }

        if (mParent.mPrefOnscreenButtonOn) {
            bottom = screen_height * (ONSCREEN_BUTTON_HEIGHT - 1) / ONSCREEN_BUTTON_HEIGHT;
            height = screen_height * (ONSCREEN_BUTTON_HEIGHT - 2) / ONSCREEN_BUTTON_HEIGHT;
        } else {
            bottom = screen_height;
            height = screen_height * (ONSCREEN_BUTTON_HEIGHT - 1) / ONSCREEN_BUTTON_HEIGHT;
        }

        if (!mParent.mPrefLargeJoystick) {
            height = width;
        }

        mJoystickRect.set(0, bottom - height, width, bottom);
    }

    void drawButton(Canvas canvas, int left, int top, int right, int bottom, String text) {
        int x = (right + left) / 2;
        int y = (bottom + top) / 2;

        mTextPaint.setColor(0x80ffffff);
        mButtonRect.set(left, top, right, bottom);
        mButtonRect.inset(5, 5);
        canvas.drawRoundRect(mButtonRect, 5, 5, mTextPaint);

        mTextPaint.setColor(0x80000000);
        mTextPaint.setAntiAlias(true);
        canvas.drawText(text, x, y + 10, mTextPaint);
        mTextPaint.setAntiAlias(false);
    }

    private static final int MOUSEMOVE_MIN = 5;
    private float down_x = -1;
    private float down_y = -1;
    private int mButtonDown = -1;

    private final static int ONSCREEN_BUTTON_SPECIAL_KEY = 3; 
    private final static int ONSCREEN_BUTTON_KEYBOARD = 4; 
    private final static int ONSCREEN_BUTTON_MENU = 5; 
    private final static int ONSCREEN_BUTTON_INFO = 6; 
    private final static int ONSCREEN_BUTTON_TURBO = 98; 
    private final static int ONSCREEN_BUTTON_SHOW_IM_PICKER = 97; 

    @Override
    public boolean onTrackballEvent(final MotionEvent event) {
        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_DOWN, 0);
                break;
            case MotionEvent.ACTION_UP:
                DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_UP, 0);
                break;
            case MotionEvent.ACTION_MOVE:
                {
                    int cur_x = (int)(event.getX() * 10);
                    int cur_y = (int)(event.getY() * 10);

                    down_x = 0;
                    down_y = 0;

                    if (mParent.mPrefMouseSensitivity != DosBoxMenuUtility.DEFAULT_MOUSE_SENSITIVITY) {
                        float mouseScale = mParent.mMouseScale;
                        DosBoxControl.nativeMouse((int)(cur_x * mouseScale), (int)(cur_y * mouseScale), -1024, -1024, MOUSE_ACTION_MOVE, -1);
                    } else {
                        DosBoxControl.nativeMouse(cur_x, cur_y, -1024, -1024, MOUSE_ACTION_MOVE, -1);
                    }

                    try {
                        Thread.sleep(mParent.mPrefFastMouseOn ? 20 : 100);
                    }
                    catch (InterruptedException e) {
                    }
                }
                break;
        }
        return true;
    }

    private int getButton(int x, int y) {
        int button = -1;

        if (y < getHeight() / ONSCREEN_BUTTON_HEIGHT) {
            button = (int)(x * ONSCREEN_BUTTON_WIDTH / getWidth());

            if (button == 0) {
                button = ONSCREEN_BUTTON_SPECIAL_KEY;
            } else if ((button >= 1) && (button <= 2)) {
                button--;
            } else {
                button = -1;
            }
        } else if (y > (getHeight() * (ONSCREEN_BUTTON_HEIGHT - 1) / ONSCREEN_BUTTON_HEIGHT)) {
            button =  (int)(x * ONSCREEN_BUTTON_WIDTH / getWidth());
            switch (button) {
                case 0:
                    button = ONSCREEN_BUTTON_INFO;
                    break;
                case 1:
                    button = ONSCREEN_BUTTON_MENU;
                    break;
                case 2:
                    button = ONSCREEN_BUTTON_KEYBOARD;
                    break;
                default:
                    button = -1;
                    break;
            }
        }

        if (isPenMode(mInputMode) && (isLandscape(this) || (!mParent.mPrefOnscreenButtonOn))) {
            button = -1;
        }

        if ((!mParent.mPrefOnscreenButtonOn) && ((button < 0) || (button > 2))) {
            button = -1;
        }

        return button;
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {
        int action = event.getAction();

        switch (action & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_POINTER_DOWN:
            case MotionEvent.ACTION_POINTER_UP:
                handleUpDownEvent(event);
                break;
            case MotionEvent.ACTION_DOWN:
                {
                    down_x = event.getX();
                    down_y = event.getY();

                    mButtonDown = getButton((int)down_x, (int)down_y);

                    if (mButtonDown >= 0) {
                        if ((mButtonDown >= 0) && (mButtonDown <= 2)) {
                            switch (mInputMode) {
                                case INPUT_MODE_MOUSE:
                                case INPUT_MODE_HYBRID_MOUSE:
                                case INPUT_MODE_PEN:
                                    DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_DOWN, mButtonDown);
                                    break;
                                case INPUT_MODE_JOYSTICK:
                                    mButtonDown = (mButtonDown == 0) ? 1 : 0;
                                    DosBoxControl.nativeJoystick(0, 0, 0, mButtonDown);
                                    break;
                            }
                        } else {
                            if (mButtonDown == ONSCREEN_BUTTON_KEYBOARD) {
                                mKeyHandler.removeMessages(DosBoxLauncher.SHOW_IM_PICKER_MESSAGE);
                                mKeyHandler.sendEmptyMessageDelayed(DosBoxLauncher.SHOW_IM_PICKER_MESSAGE, BUTTON_TAP_TIME_MAX);
                            } else if (mButtonDown == ONSCREEN_BUTTON_SPECIAL_KEY) {
                                mKeyHandler.removeMessages(DosBoxLauncher.TURBO_MESSAGE);
                                mKeyHandler.sendEmptyMessageDelayed(DosBoxLauncher.TURBO_MESSAGE, BUTTON_TAP_TIME_MAX);
                            }
                        }
                    }

                    if (mButtonDown < 0) {
                        switch (mInputMode) {
                            case INPUT_MODE_JOYSTICK:
                                handleJoystickMove(event.getX(), event.getY());
                                break;
                            case INPUT_MODE_PEN:
                                if (!mDisableAbsoluteClick) {
                                    boolean inScreen = handleAbsoluteMouseMove(event);

                                    if (inScreen) {
                                        try {
                                            Thread.sleep(BUTTON_TAP_DELAY);
                                        }
                                        catch (InterruptedException e) {
                                        }
                                        DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_DOWN, mAbsoluteMouseButton);
                                        mAbsoluteMouseClicked = true;
                                    }
                                }
                                break;
                        }
                    }
                }
                break;
            case MotionEvent.ACTION_UP:
                {
                    if (mButtonDown >= 0) {
                        long diff = event.getEventTime() - event.getDownTime();
                        switch (mButtonDown) {
                            case ONSCREEN_BUTTON_SHOW_IM_PICKER:
                                break;
                            case ONSCREEN_BUTTON_TURBO:
                                mParent.mTurboOn = false;
                                DosBoxLauncher.nativeSetOption(DosBoxMenuUtility.DOSBOX_OPTION_ID_TURBO_ON, 0, null);
                                break;
                            case ONSCREEN_BUTTON_SPECIAL_KEY:
                                mKeyHandler.removeMessages(DosBoxLauncher.TURBO_MESSAGE);
                                if ((diff > BUTTON_TAP_TIME_MIN) && (diff < BUTTON_TAP_TIME_MAX)) {
                                    mContextMenu = DosBoxMenuUtility.CONTEXT_MENU_SPECIAL_KEYS;
                                    mParent.openContextMenu(this);
                                }
                                break;
                            case ONSCREEN_BUTTON_KEYBOARD:
                                mKeyHandler.removeMessages(DosBoxLauncher.SHOW_IM_PICKER_MESSAGE);
                                if ((diff > BUTTON_TAP_TIME_MIN) && (diff < BUTTON_TAP_TIME_MAX)) {
                                    DosBoxMenuUtility.doShowKeyboard(mParent);
                                }
                                break;
                            case ONSCREEN_BUTTON_MENU:
                                if ((diff > BUTTON_TAP_TIME_MIN) && (diff < BUTTON_TAP_TIME_MAX)) {
                                    mParent.openOptionsMenu();
                                }
                                break;
                            case ONSCREEN_BUTTON_INFO:
                                if ((diff > BUTTON_TAP_TIME_MIN) && (diff < BUTTON_TAP_TIME_MAX)) {
                                    mShowInfo = !mShowInfo;
                                    DosBoxMenuUtility.doShowHideInfo(mParent, mShowInfo);
                                }
                                break;
                            default:
                                switch (mInputMode) {
                                    case INPUT_MODE_MOUSE:
                                    case INPUT_MODE_HYBRID_MOUSE:
                                    case INPUT_MODE_PEN:
                                        {
                                            if (diff < BUTTON_TAP_DELAY) {
                                                try {
                                                    Thread.sleep(BUTTON_TAP_DELAY - diff);
                                                }
                                                catch (InterruptedException e) {
                                                }
                                            }
                                            DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_UP, mButtonDown);
                                        }
                                        break;
                                    case INPUT_MODE_JOYSTICK:
                                        {
                                            if (diff < BUTTON_JOYSTICK_TAP_DELAY) {
                                                try {
                                                    Thread.sleep(BUTTON_JOYSTICK_TAP_DELAY - diff);
                                                }
                                                catch (InterruptedException e) {
                                                }
                                            }
                                            DosBoxControl.nativeJoystick(0, 0, 1, mButtonDown);
                                        }
                                        break;
                                }
                                break;						
                        }
                        mButtonDown = -1;
                    } else {
                        switch (mInputMode) {
                            case INPUT_MODE_PEN:
                                if (!mDisableAbsoluteClick) {
                                    long diff = event.getEventTime() - event.getDownTime();
                                    if (diff < BUTTON_TAP_DELAY) {
                                        try {
                                            Thread.sleep(BUTTON_TAP_DELAY - diff);
                                        }
                                        catch (InterruptedException e)
                                        {
                                        }
                                    }
                                    DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_UP, mAbsoluteMouseButton);
                                    break;
                                }
                                //fall to trackpad mode
                            case INPUT_MODE_MOUSE:
                            case INPUT_MODE_HYBRID_MOUSE:
                                {
                                    //emulate mouse click
                                    long diff = event.getEventTime() - event.getDownTime();

                                    if ((diff > BUTTON_TAP_TIME_MIN) && (diff < BUTTON_TAP_TIME_MAX)) {
                                        boolean inScreen = false;

                                        if (mInputMode == INPUT_MODE_HYBRID_MOUSE) {
                                            inScreen = handleAbsoluteMouseMove(event);

                                            if (inScreen) {
                                                try {
                                                    Thread.sleep(BUTTON_TAP_DELAY);
                                                }
                                                catch (InterruptedException e) {
                                                }
                                            }
                                        }

                                        if ((mParent.mPrefTapClickOn || ((mInputMode == INPUT_MODE_HYBRID_MOUSE) && inScreen))) {
                                            DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_DOWN, 0);

                                            try {
                                                Thread.sleep(BUTTON_TAP_DELAY);
                                            }
                                            catch (InterruptedException e) {
                                            }
                                            DosBoxControl.nativeMouse(0, 0, -1, -1, MOUSE_ACTION_UP, 0);
                                        }
                                    }
                                }
                                break;
                            case INPUT_MODE_JOYSTICK:
                                DosBoxControl.nativeJoystick(0, 0, 2, -1);
                                break;
                        }
                    }
                }
                break;
            case MotionEvent.ACTION_MOVE:
                switch (mInputMode) {
                    case INPUT_MODE_SCROLL:
                        mScroll_x += (int)(event.getX() - down_x);
                        mScroll_y += (int)(event.getY() - down_y);

                        down_x = event.getX();
                        down_y = event.getY();

                        forceRedraw();
                        break;
                    case INPUT_MODE_JOYSTICK:
                        if (mButtonDown == -1) {
                            handleJoystickMove(event.getX(), event.getY());
                        }
                        try {
                            Thread.sleep(100);
                        }
                        catch (InterruptedException e) {
                        }
                        break;
                    case INPUT_MODE_PEN:
                        if (!mDisableAbsoluteClick) {
                            if (mButtonDown == -1) {
                                handleAbsoluteMouseMove(event);
                            }
                            try {
                                Thread.sleep(100);
                            }
                            catch (InterruptedException e) {
                            }
                            break;
                        }
                        //fall to trackpad mode
                    case INPUT_MODE_MOUSE:
                    case INPUT_MODE_HYBRID_MOUSE:
                    default:
                        int cur_x = (int)event.getX();
                        int cur_y = (int)event.getY();

                        if ((Math.abs(cur_x - down_x) > MOUSEMOVE_MIN) || (Math.abs(cur_y - down_y) > MOUSEMOVE_MIN)) {
                            if (mParent.mPrefMouseSensitivity != DosBoxMenuUtility.DEFAULT_MOUSE_SENSITIVITY) {
                                float mouseScale = mParent.mMouseScale;
                                DosBoxControl.nativeMouse((int)(cur_x * mouseScale), (int)(cur_y * mouseScale), (int)(down_x * mouseScale), (int)(down_y * mouseScale), MOUSE_ACTION_MOVE, -1);
                            } else {
                                DosBoxControl.nativeMouse(cur_x, cur_y, (int)down_x, (int)down_y, MOUSE_ACTION_MOVE, -1);
                            }
                        }

                        try {
                            Thread.sleep(mParent.mPrefFastMouseOn ? 20 : 100);
                        }
                        catch (InterruptedException e) {
                        }
                        break;
                }
                break;
        }
        return true;
    }

    private boolean handleAbsoluteMouseMove(MotionEvent event) {
        boolean inScreen = false;

        float abs_x = (event.getX() - mScreenRect.left) / mScreenRect.width();
        float abs_y = (event.getY() - mScreenRect.top) / mScreenRect.height();

        if ((abs_x >= 0) && (abs_x <= 1) && (abs_y >= 0) && (abs_y <= 1)) {
            inScreen = true;

            DosBoxControl.nativeMouse(0, 0, (int)(abs_x * 1000), (int)(abs_y * 1000), MOUSE_ACTION_DOWN, -1);
        }
        return inScreen;
    }

    static private boolean isLandscape(View v) {
        return (v.getWidth() > v.getHeight());
    }

    static private boolean isPenMode(int inputMode) {
        return ((inputMode == INPUT_MODE_PEN) || (inputMode == INPUT_MODE_HYBRID_MOUSE));
    }

    private void handleUpDownEvent(MotionEvent event) {
        switch (mInputMode) {
            case INPUT_MODE_JOYSTICK:
                int action = event.getAction();

                boolean down = ((action & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_POINTER_DOWN);
                final int pointerIndex = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                //locnet, 2011-08-05, fix multi-touch problem
                int id = pointerIndex;
                float x = event.getX(id);
                float y = event.getY(id);

                int mButtonDown2 = getButton((int)x, (int)y);

                if ((mButtonDown2 >= 0) && (mButtonDown2 <= 2)) {
                    mButtonDown2 = (mButtonDown2 == 0) ? 1 : 0;

                    if (down) {
                        DosBoxControl.nativeJoystick(0, 0, 0, mButtonDown2);
                    } else {
                        long diff = event.getEventTime() - event.getDownTime();
                        if (diff < BUTTON_JOYSTICK_TAP_DELAY) {
                            try {
                                Thread.sleep(BUTTON_JOYSTICK_TAP_DELAY - diff);
                            }
                            catch (InterruptedException e) {
                            }
                        }
                        DosBoxControl.nativeJoystick(0, 0, 1, mButtonDown2);
                    }
                }
                if (mButtonDown2 < 0) {
                    switch (mInputMode) {
                        case INPUT_MODE_JOYSTICK:
                            if (down) {
                                handleJoystickMove(x, y);
                            } else {
                                DosBoxControl.nativeJoystick(0, 0, 2, -1);
                            }
                            break;
                    }
                }
                break;
        }
    }

    private void handleJoystickMove(float x, float y) {
        x = (x / mJoystickRect.width()) * 2 - 1.0f;

        int min_y = mJoystickRect.top;
        int max_y = mJoystickRect.bottom;

        if (y < min_y) {
            y = min_y;
        }
        if (y > max_y) {
            y = max_y;
        }

        y = (y - min_y) / (max_y - min_y) * 2 - 1.0f;

        if ((x > -1) && (x < 1) && (y > -1) && (y < 1)) {
            DosBoxControl.nativeJoystick((int)(x * 256), (int)(y * 256), 2, -1);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, final KeyEvent event) {
        return handleKey(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, final KeyEvent event) {
        return handleKey(keyCode, event);
    }

    //locnet, 2011-11-12, fix keyboard problem on device like Bionic
    InputConnection mInputConnection = null;

    @Override
    public boolean onCheckIsTextEditor() {
        return true;
    }

    @Override
    public InputConnection onCreateInputConnection (EditorInfo outAttrs) {
        InputConnection result;

        result = super.onCreateInputConnection(outAttrs);
        outAttrs.inputType = InputType.TYPE_NULL;

        if (result == null) {
            if (mInputConnection == null) {
                mInputConnection = new BaseInputConnection(this, false);
            }
            result = mInputConnection;
        }
        return result;
    }

    static int curJoyX = 0, curJoyY = 0;

    private boolean handleKey(int keyCode, final KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                if (event.getAction() == KeyEvent.ACTION_UP) {
                    DosBoxMenuUtility.doConfirmQuit(mParent);
                    return true;
                }
                break;
            case KeyEvent.KEYCODE_VOLUME_UP:
                if (mParent.mPrefHardkeyOn) {
                    switch (mInputMode) {
                        case INPUT_MODE_MOUSE:
                        case INPUT_MODE_HYBRID_MOUSE:
                            DosBoxControl.nativeMouse(0, 0, -1, -1, (event.getAction() == KeyEvent.ACTION_DOWN) ? MOUSE_ACTION_DOWN : MOUSE_ACTION_UP, 0);
                            return true;
                        case INPUT_MODE_PEN:
                            mDisableAbsoluteClick = (event.getAction() == KeyEvent.ACTION_DOWN) ? true : false;
                            return true;
                        case INPUT_MODE_JOYSTICK:
                            DosBoxControl.nativeJoystick(0, 0, (event.getAction() == KeyEvent.ACTION_DOWN) ? 0 : 1, 0);
                            return true;
                    }
                }
                break;
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                if (mParent.mPrefHardkeyOn) {
                    switch (mInputMode) {
                        case INPUT_MODE_MOUSE:
                        case INPUT_MODE_HYBRID_MOUSE:
                            DosBoxControl.nativeMouse(0, 0, -1, -1, (event.getAction() == KeyEvent.ACTION_DOWN) ? MOUSE_ACTION_DOWN : MOUSE_ACTION_UP, 1);
                            return true;
                        case INPUT_MODE_PEN:
                            //2011-07-21, locnet, add toggle behaviour
                            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                                if (event.getRepeatCount() == 0) {
                                    mAbsoluteMouseClicked = false;
                                    mAbsoluteMouseButton = (mAbsoluteMouseButton == 1) ? 0 : 1;
                                }
                            } else {
                                if (mAbsoluteMouseClicked) {
                                    mAbsoluteMouseButton = (mAbsoluteMouseButton == 1) ? 0 : 1;
                                }
                                mAbsoluteMouseClicked = false;
                            }
                            return true;
                        case INPUT_MODE_JOYSTICK:
                            DosBoxControl.nativeJoystick(0, 0, (event.getAction() == KeyEvent.ACTION_DOWN) ? 0 : 1, 1);
                            return true;
                    }
                }
                break;
            case KeyEvent.KEYCODE_MENU:
            case KeyEvent.KEYCODE_HOME:
            case KeyEvent.KEYCODE_SEARCH:
            case KeyEvent.KEYCODE_UNKNOWN:
                break;
            default:
                boolean down = (event.getAction() == KeyEvent.ACTION_DOWN);

                if (!down || (event.getRepeatCount() == 0)) {
                    int unicode = event.getUnicodeChar();

                    //locnet, 2011-08-20, filter system generated modifier key, but not hardware key
                    if ((event.isAltPressed() || event.isShiftPressed()) && (unicode == 0) && ((event.getFlags() & KeyEvent.FLAG_FROM_SYSTEM) == 0)) {
                        break;
                    }

                    if (handleJoyKey(keyCode, event, down)) {
                        return true;
                    }

                    //handle game button mapping
                    switch (keyCode) {
                        case KeyEvent.KEYCODE_DPAD_CENTER:
                            unicode = '\n';
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_A:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 0);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_B:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 1);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_C:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 2);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_X:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 3);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_Y:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 4);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_Z:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 5);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_L1:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 6);
                            break;
                        case DosBoxMenuUtility.KEYCODE_BUTTON_R1:
                            unicode = DosBoxMenuUtility.doGetMapperChar(mParent, 7);
                            break;
                    }

                    //fixed alt key problem for physical keyboard with only left alt
                    if ((!mParent.mPrefUseLeftAltOn) && (keyCode == KeyEvent.KEYCODE_ALT_LEFT)) {
                        break;
                    }

                    if ((keyCode > 255) || (unicode > 255)) {
                        //unknown keys
                        break;
                    }

                    keyCode = keyCode | (unicode << 8);

                    long diff = event.getEventTime() - event.getDownTime();

                    if (mParent.mPrefKeyUpDelay && (!down) && (diff < 50)) {
                        //simulate as long press
                        mKeyHandler.removeMessages(keyCode);
                        mKeyHandler.sendEmptyMessageDelayed(keyCode, BUTTON_REPEAT_DELAY - diff);
                    } else if (down && mKeyHandler.hasMessages(keyCode)) {
                        //there is an key up in queue, should be repeated event
                    } else if (DosBoxControl.sendNativeKey(keyCode, down, mModifierCtrl, mModifierAlt, mModifierShift)) {
                        mModifierCtrl = false;
                        mModifierAlt = false;
                        mModifierShift = false;
                    }
                }
                break;
        }
        return false;
    }

    boolean handleJoyKey(int keyCode, KeyEvent event, boolean down) {
        boolean isJoyButtonEvent = false;

        if (mParent.mPrefJoyKeySet == DosBoxMenuUtility.JOYKEY_SET_NONE) {
            return false;
        }

        switch (keyCode) {
            case DosBoxMenuUtility.KEYCODE_BUTTON_A:
            case DosBoxMenuUtility.KEYCODE_BUTTON_B:
            case DosBoxMenuUtility.KEYCODE_BUTTON_C:
            case DosBoxMenuUtility.KEYCODE_BUTTON_X:
            case DosBoxMenuUtility.KEYCODE_BUTTON_Y:
            case DosBoxMenuUtility.KEYCODE_BUTTON_Z:
            case DosBoxMenuUtility.KEYCODE_BUTTON_L1:
            case DosBoxMenuUtility.KEYCODE_BUTTON_R1:
            case KeyEvent.KEYCODE_DPAD_CENTER:
                keyCode = KeyEvent.KEYCODE_DPAD_CENTER;
                break;
            default:
                switch (mParent.mPrefJoyKeySet) {
                    case DosBoxMenuUtility.JOYKEY_SET_DPAD:
                        switch (keyCode) {
                            case KeyEvent.KEYCODE_DPAD_LEFT:
                            case KeyEvent.KEYCODE_DPAD_RIGHT:
                            case KeyEvent.KEYCODE_DPAD_UP:
                            case KeyEvent.KEYCODE_DPAD_DOWN:
                            case KeyEvent.KEYCODE_DPAD_CENTER:
                                break;
                            default:
                                return false;
                        }
                        break;

                    case DosBoxMenuUtility.JOYKEY_SET_WAXD:
                        switch (keyCode) {
                            case KeyEvent.KEYCODE_A:
                                keyCode = KeyEvent.KEYCODE_DPAD_LEFT;
                                break;
                            case KeyEvent.KEYCODE_D:
                                keyCode = KeyEvent.KEYCODE_DPAD_RIGHT;
                                break;
                            case KeyEvent.KEYCODE_W:
                                keyCode = KeyEvent.KEYCODE_DPAD_UP;
                                break;
                            case KeyEvent.KEYCODE_S:
                                keyCode = KeyEvent.KEYCODE_DPAD_DOWN;
                                break;
                            case KeyEvent.KEYCODE_SPACE:
                                keyCode = KeyEvent.KEYCODE_DPAD_CENTER;
                                break;
                            default:
                                return false;
                        }
                        break;

                    case DosBoxMenuUtility.JOYKEY_SET_NUMPAD:
                        switch (keyCode) {
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_4:
                                keyCode = KeyEvent.KEYCODE_DPAD_LEFT;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_6:
                                keyCode = KeyEvent.KEYCODE_DPAD_RIGHT;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_8:
                                keyCode = KeyEvent.KEYCODE_DPAD_UP;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_2:
                                keyCode = KeyEvent.KEYCODE_DPAD_DOWN;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_7:
                                keyCode = DosBoxMenuUtility.KEYCODE_DPAD_UP_LEFT;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_9:
                                keyCode = DosBoxMenuUtility.KEYCODE_DPAD_UP_RIGHT;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_1:
                                keyCode = DosBoxMenuUtility.KEYCODE_DPAD_DOWN_LEFT;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_3:
                                keyCode = DosBoxMenuUtility.KEYCODE_DPAD_DOWN_RIGHT;
                                break;
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_0:
                            case DosBoxMenuUtility.KEYCODE_NUMPAD_5:
                                keyCode = KeyEvent.KEYCODE_DPAD_CENTER;
                                break;
                            default:
                                return false;
                        }
                        break;
                }
                break;
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_LEFT:
                curJoyX = (down) ? -255 : 0;
                break;
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                curJoyX = (down) ? 255 : 0;
                break;
            case KeyEvent.KEYCODE_DPAD_UP:
                curJoyY = (down) ? -255 : 0;
                break;
            case KeyEvent.KEYCODE_DPAD_DOWN:
                curJoyY = (down) ? 255 : 0;
                break;
            case DosBoxMenuUtility.KEYCODE_DPAD_UP_LEFT:
                curJoyX = (down) ? -255 : 0;
                curJoyY = (down) ? -255 : 0;
                break;
            case DosBoxMenuUtility.KEYCODE_DPAD_UP_RIGHT:
                curJoyX = (down) ? 255 : 0;
                curJoyY = (down) ? -255 : 0;
                break;
            case DosBoxMenuUtility.KEYCODE_DPAD_DOWN_LEFT:
                curJoyX = (down) ? -255 : 0;
                curJoyY = (down) ? 255 : 0;
                break;
            case DosBoxMenuUtility.KEYCODE_DPAD_DOWN_RIGHT:
                curJoyX = (down) ? 255 : 0;
                curJoyY = (down) ? 255 : 0;
                break;
            case KeyEvent.KEYCODE_DPAD_CENTER:
                isJoyButtonEvent = true;
                break;
        }

        {
            long diff = event.getEventTime() - event.getDownTime();
            int joyMessage = (isJoyButtonEvent) ? DosBoxLauncher.RESET_JOY_BUTTON_MESSAGE : DosBoxLauncher.RESET_JOY_DIRECTION_MESSAGE;

            if (mParent.mPrefKeyUpDelay && (!down) && (diff < 50)) {
                //simulate as long press
                mKeyHandler.removeMessages(joyMessage);
                mKeyHandler.sendEmptyMessageDelayed(joyMessage, BUTTON_REPEAT_DELAY - diff);
            } else if (down && mKeyHandler.hasMessages(joyMessage)) {
                //there is an key up in queue, should be repeated event
            } else {
                if (isJoyButtonEvent) {
                    DosBoxControl.nativeJoystick(0, 0, (down) ? 0 : 1, 0);
                } else {
                    DosBoxControl.nativeJoystick(curJoyX, curJoyY, 2, -1);
                }
            }
        }
        return true;
    }

    public void setDirty() {
        mDirtyCount = 0;		
    }

    public void resetScreen(boolean redraw) {
        setDirty();
        mScroll_x = 0;
        mScroll_y = 0;

        if (redraw) {
            forceRedraw();
        }
    }

    public void forceRedraw() {
        setDirty();
        VideoRedraw(mBitmap, mSrc_width, mSrc_height, 0, mSrc_height);
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setJoystickRect();
        resetScreen(true);
    }

    public void surfaceCreated(SurfaceHolder holder) {
        mSurfaceViewRunning = true;
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        mSurfaceViewRunning = false;
    }
}

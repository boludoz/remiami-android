package com.revc;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.Window;
import android.view.WindowManager;
import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

public class GameActivity extends SDLActivity {
    private static final String TAG = "reVC";
    private static final float TARGET_FRAME_RATE = 120.0f;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
        super.onCreate(savedInstanceState);
        requestStableFrameRate();
        getWindow().getDecorView().post(this::requestStableFrameRate);
    }

    @Override
    public void setOrientationBis(int w, int h, boolean resizable, String hint) {
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
    }

    @Override
    protected void onResume() {
        super.onResume();
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
        requestStableFrameRate();
    }

    @Override
    protected SDLSurface createSDLSurface(Context context) {
        return new GameSurface(context);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "openal",
            "revc"
        };
    }

    @Override
    protected String getMainFunction() {
        return "LaunchAndroid";
    }

    private void requestStableFrameRate() {
        Window window = getWindow();
        if (window == null) {
            return;
        }

        WindowManager.LayoutParams attrs = window.getAttributes();
        attrs.preferredRefreshRate = TARGET_FRAME_RATE;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Display display = window.getDecorView().getDisplay();
            if (display == null) {
                display = getWindowManager().getDefaultDisplay();
            }

            Display.Mode bestMode = selectBestDisplayMode(display);
            if (bestMode != null) {
                attrs.preferredDisplayModeId = bestMode.getModeId();
                Log.i(TAG, "Requested display mode " + bestMode.getModeId() +
                    " " + bestMode.getPhysicalWidth() + "x" +
                    bestMode.getPhysicalHeight() + " @ " +
                    bestMode.getRefreshRate() + " Hz");
            }
        }

        window.setAttributes(attrs);
    }

    private static Display.Mode selectBestDisplayMode(Display display) {
        Display.Mode currentMode = display.getMode();
        Display.Mode bestMode = null;
        double bestScore = Double.MAX_VALUE;
        double currentArea = Math.max(1,
            currentMode.getPhysicalWidth() * currentMode.getPhysicalHeight());

        for (Display.Mode mode : display.getSupportedModes()) {
            double refreshDelta = Math.abs(mode.getRefreshRate() - TARGET_FRAME_RATE);
            double areaDelta = Math.abs(
                (mode.getPhysicalWidth() * mode.getPhysicalHeight()) - currentArea) /
                currentArea;
            double aspectDelta = Math.abs(
                ((double)mode.getPhysicalWidth() / (double)mode.getPhysicalHeight()) -
                ((double)currentMode.getPhysicalWidth() /
                 (double)currentMode.getPhysicalHeight()));
            double score = refreshDelta * 1000.0 + areaDelta + aspectDelta;
            if (score < bestScore) {
                bestScore = score;
                bestMode = mode;
            }
        }
        return bestMode;
    }

    private static class GameSurface extends SDLSurface {
        private final GameActivity activity;

        GameSurface(Context context) {
            super(context);
            activity = context instanceof GameActivity ? (GameActivity)context : null;
        }

        @Override
        public void surfaceCreated(SurfaceHolder holder) {
            super.surfaceCreated(holder);
            requestSurfaceFrameRate(holder);
            if (activity != null) {
                activity.requestStableFrameRate();
            }
        }

        @Override
        public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
            super.surfaceChanged(holder, format, width, height);
            requestSurfaceFrameRate(holder);
            if (activity != null) {
                activity.requestStableFrameRate();
            }
        }

        private void requestSurfaceFrameRate(SurfaceHolder holder) {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R || holder == null) {
                return;
            }

            Surface surface = holder.getSurface();
            if (surface == null || !surface.isValid()) {
                return;
            }

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                surface.setFrameRate(TARGET_FRAME_RATE,
                    Surface.FRAME_RATE_COMPATIBILITY_FIXED_SOURCE,
                    Surface.CHANGE_FRAME_RATE_ALWAYS);
            } else {
                surface.setFrameRate(TARGET_FRAME_RATE,
                    Surface.FRAME_RATE_COMPATIBILITY_FIXED_SOURCE);
            }
            Log.i(TAG, "Requested surface frame rate " + TARGET_FRAME_RATE + " Hz");
        }
    }
}

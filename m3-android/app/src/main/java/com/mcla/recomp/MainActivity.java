package com.mcla.recomp;

import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

import org.libsdl.app.SDL;
import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

/**
 * MainActivity for MCLA Recomp.
 *
 * Extends SDLActivity so the SDL Android backend (inside librexruntime.so) can
 * route its lifecycle messages (COMMAND_CHANGE_TITLE, COMMAND_CHANGE_WINDOW_STYLE,
 * touch/key, surface) through SDLActivity's own mSingleton/sendCommand/onResume
 * plumbing. This is the same pattern used by the working reference apps
 * (NFSMW / NFS Carbon): SDLSurface (SurfaceView) + Vulkan, zero OpenGL.
 *
 * SDL's native code is compiled INTO librexruntime.so (there is no separate
 * libSDL3.so), so getLibraries() returns only mcla_main, which transitively
 * links librexruntime.so + librexgpu-xenos.so.
 *
 * The game/render thread is NOT started by SDL's SDL_main entry point. Instead,
 * we override main() to a no-op (so SDL never tries to dlsym "SDL_main", which
 * does not exist) and drive everything from our own nativeInit(), which spawns
 * the game thread. The game thread waits for the ANativeWindow (surfaceCreated)
 * before wiring the Vulkan presenter.
 */
public class MainActivity extends SDLActivity
        implements MCLASurface.OnSurfaceLifecycleListener {
    private static final String TAG = "MCLA_Android";

    static {
        System.loadLibrary("mcla_main");
    }

    // Native methods
    public native void nativeInit(String assetsDir, String filesDir, String cacheDir);
    public native void nativeRunGame();
    public native void nativeShutdown();
    public native void nativeOnPause();
    public native void nativeOnResume();
    public native void nativeOnSurfaceCreated(int width, int height);
    public native void nativeOnSurfaceChanged(int width, int height);
    public native void nativeOnTouchEvent(int action, float x, float y, int pointerId);
    public native void nativeOnKeyEvent(int keyCode, int action);
    public native void nativeSetAssetManager(android.content.res.AssetManager assetManager);
    public native void nativeSetNativeLibraryDir(String nativeLibDir);

    /**
     * The SDL Android backend loads our native library here. SDL3 is STATICALLY
     * linked into librexruntime.so (there is no standalone libSDL3.so), and
     * mcla_main links librexruntime + librexgpu-xenos transitively, so we only
     * need to load mcla_main.
     */
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "mcla_main"
        };
    }

    /**
     * SDL's own native entry point is unused. Our game thread is driven from
     * nativeInit(); keep this a no-op so SDL's handleNativeState() doesn't try to
     * dlsym "SDL_main" (which does not exist in libmcla_main.so).
     */
    @Override
    protected void main() {
        Log.v(TAG, "main() overridden: SDL_main entry point disabled (game thread via nativeInit)");
    }

    @Override
    protected SDLSurface createSDLSurface(Context context) {
        return new MCLASurface(context, this);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // super.onCreate() sets up SDL: loadLibraries(), createSDLSurface(),
        // mSingleton = this, commandHandler, surface/layout, fullscreen flags.
        super.onCreate(savedInstanceState);

        Log.i(TAG, "onCreate");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            final android.view.WindowInsetsController controller = getWindow().getInsetsController();
            if (controller != null) {
                controller.setSystemBarsBehavior(
                    android.view.WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                controller.hide(android.view.WindowInsets.Type.statusBars()
                    | android.view.WindowInsets.Type.navigationBars());
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }

        // Extract assets (default.xex) to internal storage
        extractAssets();

        // Initialize native code (spawns the game thread)
        String assetsDir = getFilesDir().getAbsolutePath() + "/assets";
        String filesDir = getFilesDir().getAbsolutePath();
        String cacheDir = getCacheDir().getAbsolutePath();
        nativeInit(assetsDir, filesDir, cacheDir);

        // Pass native library directory for GPU plugin loading
        String nativeLibDir = getApplicationInfo().nativeLibraryDir;
        Log.i(TAG, "Native library dir: " + nativeLibDir);
        nativeSetNativeLibraryDir(nativeLibDir);

        // Pass AssetManager to native for loading menu textures
        nativeSetAssetManager(getAssets());
    }

    // ---- OnSurfaceLifecycleListener -----------------------------------------

    @Override
    public void onSurfaceCreated(int width, int height) {
        Log.i(TAG, "surfaceCreated: " + width + "x" + height);
        nativeOnSurfaceCreated(width, height);
    }

    @Override
    public void onSurfaceChanged(int width, int height) {
        Log.i(TAG, "surfaceChanged: " + width + "x" + height);
        nativeOnSurfaceChanged(width, height);
    }

    // ---- Input forwarding ---------------------------------------------------

    @Override
    public boolean dispatchKeyEvent(android.view.KeyEvent event) {
        int action = event.getAction();
        int keyCode = event.getKeyCode();
        nativeOnKeyEvent(keyCode, action == android.view.KeyEvent.ACTION_DOWN ? 0 : 1);
        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event) {
        int action = event.getActionMasked();
        int pointerCount = event.getPointerCount();
        for (int i = 0; i < pointerCount; i++) {
            int pointerId = event.getPointerId(i);
            float x = event.getX(i);
            float y = event.getY(i);
            nativeOnTouchEvent(action, x, y, pointerId);
        }
        return super.dispatchTouchEvent(event);
    }

    // ---- Lifecycle ----------------------------------------------------------

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            final android.view.WindowInsetsController controller = getWindow().getInsetsController();
            if (controller != null) {
                controller.setSystemBarsBehavior(
                    android.view.WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                controller.hide(android.view.WindowInsets.Type.statusBars()
                    | android.view.WindowInsets.Type.navigationBars());
            }
        } else {
            getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        }
        nativeOnResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "onPause");
        nativeOnPause();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                final android.view.WindowInsetsController controller = getWindow().getInsetsController();
                if (controller != null) {
                    controller.setSystemBarsBehavior(
                        android.view.WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                    controller.hide(android.view.WindowInsets.Type.statusBars()
                        | android.view.WindowInsets.Type.navigationBars());
                }
            } else {
                getWindow().getDecorView().setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy");
        nativeShutdown();
    }

    // ---- Asset extraction ---------------------------------------------------

    private void extractAssets() {
        try {
            String assetsDir = getFilesDir().getAbsolutePath() + "/assets";
            File dir = new File(assetsDir);
            if (!dir.exists()) {
                dir.mkdirs();
            }
            File xexFile = new File(assetsDir, "default.xex");
            if (!xexFile.exists()) {
                Log.i(TAG, "Extracting default.xex from assets...");
                try (InputStream is = getAssets().open("default.xex");
                     OutputStream os = new FileOutputStream(xexFile)) {
                    byte[] buffer = new byte[8192];
                    int len;
                    while ((len = is.read(buffer)) > 0) {
                        os.write(buffer, 0, len);
                    }
                    Log.i(TAG, "Extracted default.xex (" + xexFile.length() + " bytes)");
                }
            } else {
                Log.i(TAG, "default.xex already exists (" + xexFile.length() + " bytes)");
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to extract assets", e);
        }
    }
}
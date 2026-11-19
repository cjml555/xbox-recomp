package com.mcla.recomp;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends Activity {
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
    public native void nativeOnDrawFrame();
    public native void nativeOnTouchEvent(int action, float x, float y, int pointerId);
    public native void nativeOnKeyEvent(int keyCode, int action);

    private GameView mGameView;
    private boolean mGameStarted = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate");

        // Extract assets to internal storage
        extractAssets();

        // Initialize native code
        String assetsDir = getFilesDir().getAbsolutePath() + "/assets";
        String filesDir = getFilesDir().getAbsolutePath();
        String cacheDir = getCacheDir().getAbsolutePath();
        nativeInit(assetsDir, filesDir, cacheDir);

        // Create game view
        mGameView = new GameView(this);
        setContentView(mGameView);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume");
        if (mGameStarted) {
            nativeOnResume();
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.i(TAG, "onPause");
        if (mGameStarted) {
            nativeOnPause();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "onDestroy");
        nativeShutdown();
    }

    private void extractAssets() {
        try {
            String assetsDir = getFilesDir().getAbsolutePath() + "/assets";
            File dir = new File(assetsDir);
            if (!dir.exists()) {
                dir.mkdirs();
            }

            // Extract default.xex from assets
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
            Toast.makeText(this, "Failed to extract game assets: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    // Game view that handles OpenGL rendering
    public class GameView extends android.opengl.GLSurfaceView {
        private GameRenderer mRenderer;

        public GameView(Context context) {
            super(context);
            setEGLContextClientVersion(3);
            mRenderer = new GameRenderer();
            setRenderer(mRenderer);
            setRenderMode(RENDERMODE_CONTINUOUSLY);
        }

        @Override
        public boolean onTouchEvent(android.view.MotionEvent event) {
            int action = event.getActionMasked();
            int pointerCount = event.getPointerCount();
            for (int i = 0; i < pointerCount; i++) {
                int pointerId = event.getPointerId(i);
                float x = event.getX(i);
                float y = event.getY(i);
                nativeOnTouchEvent(action, x, y, pointerId);
            }
            return true;
        }

        @Override
        public boolean onKeyDown(int keyCode, android.view.KeyEvent event) {
            nativeOnKeyEvent(keyCode, 0);
            return super.onKeyDown(keyCode, event);
        }

        @Override
        public boolean onKeyUp(int keyCode, android.view.KeyEvent event) {
            nativeOnKeyEvent(keyCode, 1);
            return super.onKeyUp(keyCode, event);
        }

        private class GameRenderer implements android.opengl.GLSurfaceView.Renderer {
            @Override
            public void onSurfaceCreated(javax.microedition.khronos.opengles.GL10 gl, javax.microedition.khronos.egl.EGLConfig config) {
                Log.i(TAG, "onSurfaceCreated");
                int width = getWidth();
                int height = getHeight();
                nativeOnSurfaceCreated(width, height);
            }

            @Override
            public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl, int width, int height) {
                Log.i(TAG, "onSurfaceChanged: " + width + "x" + height);
                nativeOnSurfaceChanged(width, height);
            }

            @Override
            public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
                if (!mGameStarted) {
                    mGameStarted = true;
                    // Start the game in a separate thread
                    new Thread(() -> nativeRunGame()).start();
                }
                nativeOnDrawFrame();
            }
        }
    }
}
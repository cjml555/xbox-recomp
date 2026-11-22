package com.mcla.recomp;

import android.content.Context;
import android.view.SurfaceHolder;

import org.libsdl.app.SDLActivity;
import org.libsdl.app.SDLSurface;

/**
 * Public subclass of SDLSurface so MainActivity (which is NOT a subclass of
 * SDLActivity) can instantiate a SurfaceView for Vulkan presentation.
 *
 * We deliberately do NOT call super.surfaceCreated/surfaceChanged because those
 * invoke SDLActivity.onNativeSurfaceCreated/Changed() which expect mSingleton to
 * be non-null. Instead, lifecycle events are forwarded here and relayed to the
 * game thread via nativeOnSurfaceCreated/nativeOnSurfaceChanged JNI calls.
 *
 * SDL's Android backend fetches the ANativeWindow via
 * SDLActivity.mSurface.getNativeSurface(), so this instance is published into
 * that protected static field via reflection from MainActivity.
 */
public class MCLASurface extends SDLSurface {
    private final OnSurfaceLifecycleListener mListener;

    // Set by MainActivity right after nativeInit(): the SDL statics must not be
    // called before SDL is initialized by the native thread.
    private static volatile boolean sNativeReady = false;

    public static void setNativeReady(boolean ready) { sNativeReady = ready; }

    public interface OnSurfaceLifecycleListener {
        void onSurfaceCreated(int width, int height);
        void onSurfaceChanged(int width, int height);
    }

    public MCLASurface(Context context, OnSurfaceLifecycleListener listener) {
        // Use the package-private constructor (SDLSurface has protected ctor).
        super(context);
        mListener = listener;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // Do NOT call super.surfaceCreated/surfaceChanged: those touch SDLActivity
        // state that requires mSingleton. The static native is safe and is the
        // only way SDL learns about a NEW ANativeWindow after a destroy+create;
        // the Vulkan presenter rebuilds its swapchain from it on the next paint.
        if (sNativeReady) {
            SDLActivity.onNativeSurfaceCreated();
        }
        if (mListener != null) {
            mListener.onSurfaceCreated(getWidth(), getHeight());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        // Drop SDL's stale native window so the presenter disconnects instead of
        // presenting to a dead surface. super.surfaceDestroyed() would also drive
        // SDLActivity.handleNativeState() and can pause our custom game thread.
        if (sNativeReady) {
            SDLActivity.onNativeSurfaceDestroyed();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // Do NOT call super — it would invoke SDLActivity.onNativeResize() and
        // SDLActivity.nativeSetScreenResolution() which require mSingleton != null.
        if (mListener != null) {
            mListener.onSurfaceChanged(width, height);
        }
    }
}

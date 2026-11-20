package com.mcla.recomp;

import android.content.Context;
import android.view.SurfaceHolder;

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
        // Do NOT call super — it would invoke SDLActivity.onNativeSurfaceCreated()
        // which requires mSingleton != null (and crashes otherwise).
        if (mListener != null) {
            mListener.onSurfaceCreated(getWidth(), getHeight());
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

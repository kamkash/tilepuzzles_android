package net.kamkash.tilepuzzles

import android.annotation.SuppressLint
import android.graphics.PixelFormat
import android.graphics.SurfaceTexture
import android.os.Build
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.TextureView
import android.view.TextureView.SurfaceTextureListener
import androidx.annotation.Nullable
import com.google.android.filament.SwapChain
import com.google.android.filament.android.UiHelper

/**
 * UiHelper is a simple class that can manage either a SurfaceView, TextureView, or a SurfaceHolder
 * so it can be used to render into with Filament.
 *
 * Here is a simple example with a SurfaceView. The code would be exactly the same with a
 * TextureView:
 *
 * <pre>
 * public class FilamentActivity extends Activity {
 *     private UiHelper mUiHelper;
 *     private SurfaceView mSurfaceView;
 *
 *     // Filament specific APIs
 *     private Engine mEngine;
 *     private Renderer mRenderer;
 *     private View mView; // com.google.android.filament.View, not android.view.View
 *     private SwapChain mSwapChain;
 *
 *     public void onCreate(Bundle savedInstanceState) {
 *         super.onCreate(savedInstanceState);
 *
 *         // Create a SurfaceView and add it to the activity
 *         mSurfaceView = new SurfaceView(this);
 *         setContentView(mSurfaceView);
 *
 *         // Create the Filament UI helper
 *         mUiHelper = new UiHelper(UiHelper.ContextErrorPolicy.DONT_CHECK);
 *
 *         // Attach the SurfaceView to the helper, you could do the same with a TextureView
 *         mUiHelper.attachTo(mSurfaceView);
 *
 *         // Set a rendering callback that we will use to invoke Filament
 *         mUiHelper.setRenderCallback(new UiHelper.RendererCallback() {
 *             public void onNativeWindowChanged(Surface surface) {
 *                 if (mSwapChain != null) mEngine.destroySwapChain(mSwapChain);
 *                 mSwapChain = mEngine.createSwapChain(surface, mUiHelper.getSwapChainFlags());
 *             }
 *
 *             // The native surface went away, we must stop rendering.
 *             public void onDetachedFromSurface() {
 *                 if (mSwapChain != null) {
 *                     mEngine.destroySwapChain(mSwapChain);
 *
 *                     // Required to ensure we don't return before Filament is done executing the
 *                     // destroySwapChain command, otherwise Android might destroy the Surface
 *                     // too early
 *                     mEngine.flushAndWait();
 *
 *                     mSwapChain = null;
 *                 }
 *             }
 *
 *             // The native surface has changed size. This is always called at least once
 *             // after the surface is created (after onNativeWindowChanged() is invoked).
 *             public void onResized(int width, int height) {
 *                 // Compute camera projection and set the viewport on the view
 *             }
 *         });
 *
 *         mEngine = Engine.create();
 *         mRenderer = mEngine.createRenderer();
 *         mView = mEngine.createView();
 *         // Create scene, camera, etc.
 *     }
 *
 *     public void onDestroy() {
 *         super.onDestroy();
 *         // Always detach the surface before destroying the engine
 *         mUiHelper.detach();
 *
 *         mEngine.destroy();
 *     }
 *
 *     // This is an example of a render function. You will most likely invoke this from
 *     // a Choreographer callback to trigger rendering at vsync.
 *     public void render() {
 *         if (mUiHelper.isReadyToRender) {
 *             // If beginFrame() returns false you should skip the frame
 *             // This means you are sending frames too quickly to the GPU
 *             if (mRenderer.beginFrame(swapChain)) {
 *                 mRenderer.render(mView);
 *                 mRenderer.endFrame();
 *             }
 *         }
 *     }
 * }
 * </pre>
 */
class TUiHelper(policy: ContextErrorPolicy? = ContextErrorPolicy.CHECK) {
    private val LOG_TAG = "UiHelper"
    private val LOGGING = true

    private var mDesiredWidth = 0
    private var mDesiredHeight = 0
    private var mNativeWindow: Any? = null

    private var mRenderCallback: RendererCallback? = null
    private var mHasSwapChain = false

    private var mRenderSurface: RenderSurface? = null

    private var mOpaque = true
    private var mOverlay = false

    /**
     * Enum used to decide whether UiHelper should perform extra error checking.
     *
     * @see UiHelper.UiHelper
     */
    enum class ContextErrorPolicy {
        /** Check for extra errors.  */
        CHECK,

        /** Do not check for extra errors.  */
        DONT_CHECK
    }

    /**
     * Interface used to know when the native surface is created, destroyed or resized.
     *
     * @see .setRenderCallback
     */
    interface RendererCallback {
        /**
         * Called when the underlying native window has changed.
         */
        fun onNativeWindowChanged(surface: Surface?)

        /**
         * Called when the surface is going away. After this call `isReadyToRender()`
         * returns false. You MUST have stopped drawing when returning.
         * This is called from detach() or if the surface disappears on its own.
         */
        fun onDetachedFromSurface()

        /**
         * Called when the underlying native window has been resized.
         */
        fun onResized(width: Int, height: Int)
    }

    private interface RenderSurface {
        fun resize(width: Int, height: Int)
        fun detach()
    }

    private inner class  SurfaceViewHandler internal constructor(private val mSurfaceView: SurfaceView) :
        RenderSurface {
        override fun resize(width: Int, height: Int) {
            mSurfaceView.holder.setFixedSize(width, height)
        }

        override fun detach() {}
    }

    private inner class  SurfaceHolderHandler internal constructor(private val mSurfaceHolder: SurfaceHolder) :
        RenderSurface {
        override fun resize(width: Int, height: Int) {
            mSurfaceHolder.setFixedSize(width, height)
        }

        override fun detach() {}
    }

    private inner class TextureViewHandler internal constructor(private val mTextureView: TextureView) :
        RenderSurface {
        private var mSurface: Surface? = null
        override fun resize(width: Int, height: Int) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
                mTextureView.surfaceTexture!!.setDefaultBufferSize(width, height)
            }
            // the call above won't cause TextureView.onSurfaceTextureSizeChanged()
            mRenderCallback!!.onResized(width, height)
        }

        override fun detach() {
            setSurface(null)
        }

        fun setSurface(surface: Surface?) {
            if (surface == null) {
                mSurface?.release()
            }
            mSurface = surface
        }
    }

    /**
     * Creates a UiHelper which will help manage the native surface provided by a
     * SurfaceView or a TextureView.
     *
     * @param policy The error checking policy to use.
     */
//    fun TUiHelper(policy: ContextErrorPolicy? = ContextErrorPolicy.CHECK) {
        // TODO: do something with policy
//    }

    /**
     * Sets the renderer callback that will be notified when the native surface is
     * created, destroyed or resized.
     *
     * @param renderCallback The callback to register.
     */
    fun setRenderCallback(@Nullable renderCallback: RendererCallback?) {
        mRenderCallback = renderCallback
    }

    /**
     * Returns the current render callback associated with this UiHelper.
     */
    @Nullable
    fun getRenderCallback(): RendererCallback? {
        return mRenderCallback
    }

    /**
     * Free resources associated to the native window specified in [.attachTo],
     * [.attachTo], or [.attachTo].
     */
    fun detach() {
        destroySwapChain()
        mNativeWindow = null
        mRenderSurface = null
    }

    /**
     * Checks whether we are ready to render into the attached surface.
     *
     * Using OpenGL ES when this returns true, will result in drawing commands being lost,
     * HOWEVER, GLES state will be preserved. This is useful to initialize the engine.
     *
     * @return true: rendering is possible, false: rendering is not possible.
     */
    fun isReadyToRender(): Boolean {
        return mHasSwapChain
    }

    /**
     * Set the size of the render target buffers of the native surface.
     */
    fun setDesiredSize(width: Int, height: Int) {
        mDesiredWidth = width
        mDesiredHeight = height
        if (mRenderSurface != null) {
            mRenderSurface!!.resize(width, height)
        }
    }

    /**
     * Returns the requested width for the native surface.
     */
    fun getDesiredWidth(): Int {
        return mDesiredWidth
    }

    /**
     * Returns the requested height for the native surface.
     */
    fun getDesiredHeight(): Int {
        return mDesiredHeight
    }

    /**
     * Returns true if the render target is opaque.
     */
    fun isOpaque(): Boolean {
        return mOpaque
    }

    /**
     * Controls whether the render target (SurfaceView or TextureView) is opaque or not.
     * The render target is considered opaque by default.
     *
     * Must be called before calling [.attachTo], [.attachTo],
     * or [.attachTo].
     *
     * @param opaque Indicates whether the render target should be opaque. True by default.
     */
    fun setOpaque(opaque: Boolean) {
        mOpaque = opaque
    }

    /**
     * Returns true if the SurfaceView used as a render target should be positioned above
     * other surfaces but below the activity's surface. False by default.
     */
    fun isMediaOverlay(): Boolean {
        return mOverlay
    }

    /**
     * Controls whether the surface of the SurfaceView used as a render target should be
     * positioned above other surfaces but below the activity's surface. This property
     * only has an effect when used in combination with [setOpaque(false)][.setOpaque]
     * and does not affect TextureView targets.
     *
     * Must be called before calling [.attachTo]
     * or [.attachTo].
     *
     * Has no effect when using [.attachTo].
     *
     * @param overlay Indicates whether the render target should be rendered below the activity's
     * surface when transparent.
     */
    fun setMediaOverlay(overlay: Boolean) {
        mOverlay = overlay
    }

    /**
     * Returns the flags to pass to
     * [com.google.android.filament.Engine.createSwapChain] to honor all
     * the options set on this UiHelper.
     */
    fun getSwapChainFlags(): Long {
        return if (isOpaque()) SwapChain.CONFIG_DEFAULT else SwapChain.CONFIG_TRANSPARENT
    }

    /**
     * Associate UiHelper with a SurfaceView.
     *
     * As soon as SurfaceView is ready (i.e. has a Surface), we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    @SuppressLint("WrongConstant")
    fun attachTo(view: SurfaceView) {
        if (attach(view)) {
            val translucent = !isOpaque()
            // setZOrderOnTop() and setZOrderMediaOverlay() override each other,
            // we must only call one of them
            if (isMediaOverlay()) {
                view.setZOrderMediaOverlay(translucent)
            } else {
                view.setZOrderOnTop(translucent)
            }
            val format = if (isOpaque()) PixelFormat.OPAQUE else PixelFormat.TRANSLUCENT
            view.holder.setFormat(format)
            mRenderSurface = SurfaceViewHandler(view)
            val callback: SurfaceHolder.Callback = object : SurfaceHolder.Callback {
                override fun surfaceCreated(holder: SurfaceHolder) {
                    if (LOGGING) Log.d(LOG_TAG, "surfaceCreated()")
                    createSwapChain(holder.surface)
                }

                override fun surfaceChanged(
                    holder: SurfaceHolder, format: Int, width: Int, height: Int
                ) {
                    // Note: this is always called at least once after surfaceCreated()
                    if (LOGGING) Log.d(LOG_TAG, "surfaceChanged($width, $height)")
                    mRenderCallback!!.onResized(width, height)
                }

                override fun surfaceDestroyed(holder: SurfaceHolder) {
                    if (LOGGING) Log.d(LOG_TAG, "surfaceDestroyed()")
                    destroySwapChain()
                }
            }
            val holder = view.holder
            holder.addCallback(callback)
            if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                holder.setFixedSize(mDesiredWidth, mDesiredHeight)
            }

            // in case the SurfaceView's surface already existed
            val surface: Surface? = holder.surface
            if (surface != null && surface.isValid()) {
                callback.surfaceCreated(holder)
                callback.surfaceChanged(
                    holder, format,
                    holder.surfaceFrame.width(), holder.surfaceFrame.height()
                )
            }
        }
    }

    /**
     * Associate UiHelper with a TextureView.
     *
     * As soon as TextureView is ready (i.e. has a buffer), we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    fun attachTo(view: TextureView) {
        if (attach(view)) {
            view.isOpaque = isOpaque()
            mRenderSurface = TextureViewHandler(view)
            val listener: SurfaceTextureListener = object : SurfaceTextureListener {
                override fun onSurfaceTextureAvailable(
                    surfaceTexture: SurfaceTexture, width: Int, height: Int
                ) {
                    if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureAvailable()")
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1) {
                        if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                            surfaceTexture.setDefaultBufferSize(mDesiredWidth, mDesiredHeight)
                        }
                    }
                    val surface = Surface(surfaceTexture)
                    val textureViewHandler = mRenderSurface as TextureViewHandler
                    textureViewHandler.setSurface(surface)
                    createSwapChain(surface)

                    // Call this the first time because onSurfaceTextureSizeChanged()
                    // isn't called at initialization time
                    mRenderCallback!!.onResized(width, height)
                }

                override fun onSurfaceTextureSizeChanged(
                    surfaceTexture: SurfaceTexture, width: Int, height: Int
                ) {
                    if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureSizeChanged()")
                    if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                        surfaceTexture.setDefaultBufferSize(mDesiredWidth, mDesiredHeight)
                        mRenderCallback!!.onResized(mDesiredWidth, mDesiredHeight)
                    } else {
                        mRenderCallback!!.onResized(width, height)
                    }
                }

                override fun onSurfaceTextureDestroyed(surfaceTexture: SurfaceTexture): Boolean {
                    if (LOGGING) Log.d(LOG_TAG, "onSurfaceTextureDestroyed()")
                    destroySwapChain()
                    return true
                }

                override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {}
            }
            view.surfaceTextureListener = listener

            // in case the View's SurfaceTexture already existed
            if (view.isAvailable) {
                val surfaceTexture = view.surfaceTexture
                listener.onSurfaceTextureAvailable(surfaceTexture!!, mDesiredWidth, mDesiredHeight)
            }
        }
    }

    /**
     * Associate UiHelper with a SurfaceHolder.
     *
     * As soon as a Surface is created, we'll create the
     * EGL resources needed, and call user callbacks if needed.
     */
    @SuppressLint("WrongConstant")
    fun attachTo(holder: SurfaceHolder) {
        if (attach(holder)) {
            val format = if (isOpaque()) PixelFormat.OPAQUE else PixelFormat.TRANSLUCENT
            holder.setFormat(format)
            mRenderSurface = SurfaceHolderHandler(holder)
            val callback: SurfaceHolder.Callback = object : SurfaceHolder.Callback {
                override fun surfaceCreated(surfaceHolder: SurfaceHolder) {
                    if (LOGGING) Log.d(LOG_TAG, "surfaceCreated()")
                    createSwapChain(holder.surface)
                }

                override fun surfaceChanged(
                    holder: SurfaceHolder,
                    format: Int,
                    width: Int,
                    height: Int
                ) {
                    // Note: this is always called at least once after surfaceCreated()
                    if (LOGGING) Log.d(LOG_TAG, "surfaceChanged($width, $height)")
                    mRenderCallback!!.onResized(width, height)
                }

                override fun surfaceDestroyed(surfaceHolder: SurfaceHolder) {
                    if (LOGGING) Log.d(LOG_TAG, "surfaceDestroyed()")
                    destroySwapChain()
                }
            }
            holder.addCallback(callback)
            if (mDesiredWidth > 0 && mDesiredHeight > 0) {
                holder.setFixedSize(mDesiredWidth, mDesiredHeight)
            }

            // in case the SurfaceHolder's surface already existed
            val surface: Surface? = holder.surface
            if (surface != null && surface.isValid()) {
                callback.surfaceCreated(holder)
                callback.surfaceChanged(
                    holder, format,
                    holder.surfaceFrame.width(), holder.surfaceFrame.height()
                )
            }
        }
    }

    private fun attach(nativeWindow: Any): Boolean {
        if (mNativeWindow != null) {
            // we are already attached to a native window
            if (mNativeWindow === nativeWindow) {
                // nothing to do
                return false
            }
            destroySwapChain()
        }
        mNativeWindow = nativeWindow
        return true
    }

    private fun createSwapChain(surface: Surface) {
        mRenderCallback!!.onNativeWindowChanged(surface)
        mHasSwapChain = true
    }

    private fun destroySwapChain() {
        if (mRenderSurface != null) {
            mRenderSurface!!.detach()
        }
        mRenderCallback!!.onDetachedFromSurface()
        mHasSwapChain = false
    }
}
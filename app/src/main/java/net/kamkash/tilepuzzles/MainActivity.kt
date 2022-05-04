package net.kamkash.tilepuzzles

import android.content.res.AssetManager
import android.graphics.Rect
import android.os.Bundle
import android.util.Log
import android.view.*
import android.widget.ImageButton
import androidx.appcompat.app.AppCompatActivity
import net.kamkash.tilepuzzles.databinding.ActivityMainBinding


class MainActivity : AppCompatActivity(), Choreographer.FrameCallback {
    private val LOG_TAG = "MainActivity"
    private lateinit var mSurfaceView: SurfaceView
    private lateinit var imageButton1: ImageButton
    private lateinit var imageButton2: ImageButton
    private lateinit var mUiHelper: TUiHelper
    private var mVisisbleFrame: Rect = Rect()
    private lateinit var binding: ActivityMainBinding


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Log.d(LOG_TAG, "MainActivity::onCreate()")
        Log.d(LOG_TAG, "MainActivity::onCreate() JNI call ${stringFromJNI()}")

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        mSurfaceView = findViewById(R.id.surfaceView)
        imageButton1 = findViewById(R.id.imageButton1)
        imageButton2 = findViewById(R.id.imageButton2)


        imageButton1.setOnClickListener {
            Log.d(LOG_TAG, "........ Shuffle")
            shuffle()
        }

        val listener: View.OnTouchListener = View.OnTouchListener { v, event ->
            when (event!!.action) {
                MotionEvent.ACTION_DOWN -> {
                    Log.d(LOG_TAG, "Action was DOWN: ${event.x}, ${event.y}")
                    touchAction(event.action, event.x, event.y)
                    true
                }
                MotionEvent.ACTION_MOVE -> {
                    Log.d(LOG_TAG, "Action was MOVE: ${event.x}, ${event.y}")
                    touchAction(event.action, event.x, event.y)
                    true
                }
                MotionEvent.ACTION_UP -> {
                    Log.d(LOG_TAG, "Action was UP: ${event.x}, ${event.y}")
                    touchAction(event.action, event.x, event.y)
                    v.performClick()
                    true
                }
                MotionEvent.ACTION_CANCEL -> {
                    Log.d(LOG_TAG, "Action was CANCEL")
                    true
                }
                MotionEvent.ACTION_OUTSIDE -> {
                    Log.d(LOG_TAG, "Movement occurred outside bounds of current screen element")
                    true
                }
                else -> true
            }
        }
        mSurfaceView.setOnTouchListener(listener)
        initUiHelper()
        windowVisibleDisplayFrame()
    }

    private fun windowVisibleDisplayFrame() {
        val window = window
        window.decorView.getWindowVisibleDisplayFrame(mVisisbleFrame)
        Log.d(LOG_TAG, "mVisisbleFrame: $mVisisbleFrame")
    }

    private fun initUiHelper() {
        mUiHelper = TUiHelper(TUiHelper.ContextErrorPolicy.DONT_CHECK)
        mUiHelper.setRenderCallback(object : TUiHelper.RendererCallback {
            override fun onNativeWindowChanged(surface: Surface?) {
                // start the draw events
                Choreographer.getInstance().postFrameCallback(this@MainActivity)
                destroySwapChain()
                createSwapChain(surface)
            }

            override fun onDetachedFromSurface() {
                destroySwapChain()
                Choreographer.getInstance().removeFrameCallback(this@MainActivity)
            }

            override fun onResized(width: Int, height: Int) {
                Log.d(LOG_TAG, "resizeWindow: $width, $height")
                resizeWindow(width, height)
            }
        })
        mUiHelper.attachTo(mSurfaceView)
        init(assets)
    }

    override fun doFrame(frameTimeNanos: Long) {
//        Log.d(LOG_TAG, "MainActivity::doFrame(${frameTimeNanos})")
        Choreographer.getInstance().postFrameCallback(this)
        gameLoop(frameTimeNanos)
    }

    override fun onDestroy(): Unit {
        Log.d(LOG_TAG, "MainActivity::onDestroy()")
        super.onDestroy()
        // Always detach the surface before destroying the engine
        mUiHelper.detach()
        destroy()
    }

    override fun onPause() {
        super.onPause()
        Log.d(LOG_TAG, "onPause unhooking choreographer")
        Choreographer.getInstance().removeFrameCallback(this)
    }

    override fun onResume() {
        super.onResume()
        Log.d(LOG_TAG, "onResume re-hooking choreographer")
        Choreographer.getInstance().postFrameCallback(this)
    }

    /**
     * Native methods that are implemented by the 'tilepuzzles' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String
    external fun createSwapChain(surface: Surface?): Unit
    external fun destroySwapChain(): Unit
    external fun resizeWindow(width: Int, height: Int): Unit
    external fun init(assetManager: AssetManager): Unit
    external fun destroy(): Unit
    external fun shuffle(): Unit
    external fun gameLoop(frameTimeNanos: Long): Unit
    external fun touchAction(action: Int, rawX: Float, rawY: Float): Unit

    companion object {
        // Used to load the 'tilepuzzles' library on application startup.
        init {
            System.loadLibrary("tilepuzzles")
        }
    }
}
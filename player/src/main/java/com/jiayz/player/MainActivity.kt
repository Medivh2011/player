package com.jiayz.player

import android.Manifest
import android.app.Activity
import android.content.pm.ActivityInfo
import android.content.res.Configuration
import android.os.Bundle
import android.util.DisplayMetrics
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.view.WindowManager
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import com.jiayz.ffmpeg.opengles.SoakGlSurfaceView
import com.jiayz.ffmpeg.soakplayer.SoakPlayer
import com.jiayz.ffmpeg.util.PermissionUtils
import com.jiayz.ffmpeg.util.SoakTimeUtil
import kotlinx.coroutines.*


class MainActivity : AppCompatActivity(),CoroutineScope by MainScope() {

    private val glSurface by lazy<SoakGlSurfaceView> { findViewById(R.id.gl_surface) }

    private val playBtn by lazy<Button> { findViewById(R.id.play) }

    private val pauseBtn by lazy<Button> { findViewById(R.id.pause) }

    private val fullScreenBtn by lazy<Button> { findViewById(R.id.full_screen) }

    private val currentTime by lazy<TextView> { findViewById(R.id.tv_current_time) }

    private val totalTime by lazy<TextView> { findViewById(R.id.tv_total_time) }

    private val seekBar by lazy<SeekBar> { findViewById(R.id.sb_seek) }

    private val llControl by lazy<LinearLayout> { findViewById(R.id.ll_control) }

    private var widthPixels: Int = 0
    private var heightPixels: Int = 0

    private var height: Int = 0
    private var width: Int = 0

    private val soakPlayer: SoakPlayer = SoakPlayer()

    private var position = 0

    private var isSeek = false

    private var isPlayStop = false

    ///storage/emulated/0/
    private val pathUrl = "/storage/emulated/0/Movies/2022-02-10-16-32-31.mp4"
    private val pathUrl1 = "/storage/emulated/0/Music/虎二-即使知道.flac"
    private val pathUrl2 = "/storage/emulated/0/SmartRecorder(TEST)/RecordFiles/audio/2022-02-11-13-47-10.wav"
    private val pathUrl4 = "/storage/emulated/0/Music/萧敬腾&张杰&袁娅维TIARAY&欧阳娜娜&陈立农-我们同唱一首歌.mp3"
    private val onlineVideoPath = "http://mirror.aarnet.edu.au/pub/TED-talks/911Mothers_2010W-480p.mp4"
    private val pathUrl5 = "https://stream7.iqilu.com/10339/upload_transcode/202002/18/20200218114723HDu3hhxqIT.mp4"

   //凡人修仙传-21.mp4
    private val onlineAudioPath = "/storage/emulated/0/Movies/凡人修仙传-21.mp4"

    companion object{
        private const val TAG = "MainActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN);
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.activity_main)
        soakPlayer.setDataSource(onlineAudioPath)
        soakPlayer.isOnlySoft = true
        soakPlayer.setGlSurfaceView(glSurface)
        playBtn.setOnClickListener {
            if (isPlayStop){
//                soakPlayer.setDataSource(pathUrl)
//                soakPlayer.isOnlySoft = true
                soakPlayer.prepared()
            }else{
                soakPlayer.resume()
            }
        }

        fullScreenBtn.setOnClickListener {
            setOrientation()
        }

        pauseBtn.setOnClickListener { soakPlayer.pause() }

        seekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                position = soakPlayer.duration * progress / 100
            }

            override fun onStartTrackingTouch(seekBar: SeekBar) {}
            override fun onStopTrackingTouch(seekBar: SeekBar) {
                isSeek = true
                soakPlayer.seek(position)
            }
        })
        /**
         * 准备完成时
         */
        soakPlayer.setOnPreparedListener {
            isPlayStop = false
            soakPlayer.start()
        }
        /**
         * 加载中时
         */
        soakPlayer.setOnLoadListener {
            if (it) {
                launch(Dispatchers.Main) {
                   // Toast.makeText(applicationContext,"Loading...",Toast.LENGTH_SHORT).show()
                }
            }
        }
        /**
         * 更新时间戳
         */
        soakPlayer.setOnInfoListener {
          //  Log.e(TAG, "soakPlayer: -----------------setOnInfoListener-----totalSeconds: ${it.totalSeconds}-- currentSeconds: ${it.currentSeconds}-----" )
            launch(Dispatchers.Main) {
                totalTime.text = SoakTimeUtil.secdsToDateFormat(it.totalSeconds)
                currentTime.text = SoakTimeUtil.secdsToDateFormat(it.currentSeconds)
                if (isSeek) {
                    seekBar.progress = position * 100 / it.totalSeconds
                    isSeek = false
                } else {
                    seekBar.progress =
                        it.currentSeconds * 100 / it.totalSeconds
                }
            }

        }

        /**
         * 播放完成回调
         */
        soakPlayer.setOnCompleteListener {
            soakPlayer.stop(false)
        }
        soakPlayer.setVideoSizeChangedListener { w, h, dar ->
            launch(Dispatchers.Main) {
                val videoWidth: Int = w
                val videoHeight: Int = h
                width = w
                height = h
                Log.e(TAG, "setVideoSizeChangedListener: width: $videoWidth height: $videoHeight, dar: $dar")
                val dar: Float = dar
                val viewWidth: Int = resources.displayMetrics.widthPixels
                var viewHeight = 0
                viewHeight =
                    if (dar.compareTo(Float.NaN) == 0 || dar.compareTo(0.0f) == 0) {
                        (viewWidth * 1.0f / videoWidth).toInt()
                    } else {
                       if(videoWidth > videoHeight) (viewWidth * 1.0f / dar).toInt() else (viewWidth * 1.0f*dar).toInt()
                    }
                val params = RelativeLayout.LayoutParams(viewWidth, viewHeight)
                params.addRule(RelativeLayout.CENTER_IN_PARENT)
                Log.e(TAG,"viewWith=$viewWidth, viewHeight=$viewHeight")
                glSurface.layoutParams = params

            }
        }

        /**
         * 播放停止回调
         */
        soakPlayer.setOnStopListener {
            Log.e(TAG, "soakPlayer: ---setOnStopListener------play is stop" )
            launch (Dispatchers.Main){
               // isPlayStop = true
                position = 0
                seekBar.progress = 0
                soakPlayer.seek(0)
                delay(20)
                soakPlayer.prepared()
            }

        }

        /**
         * 播放出现错误
         */
        soakPlayer.setOnErrorListener { code, msg ->
            launch (Dispatchers.Main){
                Toast.makeText(applicationContext,"play has some error happened,code:$code  msg:$msg",Toast.LENGTH_SHORT).show()
            }
        }

        /**
         * 截取播放界面
         */
        soakPlayer.setOnCutVideoImgListener {
            it?.let {
                Log.e(TAG, "soakPlayer: ---setOnCutVideoImgListener------bitmap is cut" )
            }
        }
        val storagePermissions = arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE)
        PermissionUtils.checkAndRequestMorePermissions(this,storagePermissions,0x12) {
            soakPlayer.prepared()
            controlVideo(false)
        }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {

        when(event.action){
            MotionEvent.ACTION_DOWN ->{
                llControl.visibility = View.VISIBLE
                controlVideo(false)
            }
        }
        return super.onTouchEvent(event)
    }

    fun controlVideo(isShow:Boolean){
        launch {
            delay(5000)
            if (isShow){
                llControl.visibility = View.VISIBLE
            }else{
                llControl.visibility = View.GONE
            }
        }

    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        //setOrientation()
    }
    private var isFullScreen = false
    private fun setOrientation() {
        Log.d(TAG, "setOrientation")
        if (isFullScreen){
            requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_PORTRAIT
            isFullScreen = false
        }else{
            requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE
            isFullScreen = true
        }
        val outMetrics = DisplayMetrics()
        windowManager.defaultDisplay.getRealMetrics(outMetrics)
        widthPixels =  outMetrics.widthPixels
        heightPixels =   outMetrics.heightPixels
        GlobalScope.launch(Dispatchers.IO) {
            withContext(Dispatchers.Main) {
                glSurface.layoutParams?.let {
                    if (widthPixels.toFloat() / width <= heightPixels.toFloat() / height) {
                        //占比多的填满全部
                        it.width = widthPixels
                        it.height = widthPixels * height / width
                    } else {
                        it.width = heightPixels * width / height
                        it.height = heightPixels
                    }
                    glSurface.layoutParams = it
                }
            }
        }
    }

}
package pkocia.gallery

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Context
import android.content.Intent
import android.net.Uri
import androidx.core.app.ActivityCompat.startActivityForResult

class Application : android.app.Application() {
    companion object {
        @SuppressLint("StaticFieldLeak")
        lateinit var context: Context
            private set

        var activity: Activity? = null

        fun startImageDetailsActivity(activity: Activity, image: Image) {
            val intent = Intent(context, ImageDetails::class.java)
            intent.putExtra("uri", image.uri)
            activity.startActivityForResult(intent, REQUEST_IMAGE_DETAILS)
        }

        fun startCameraActivity(activity: Activity) {
            val intent = Intent(context, CameraActivity::class.java)
            activity.startActivityForResult(intent, REQUEST_IMAGE_CAPTURE)
        }

        fun startAddImageActivity(activity: Activity, uri: Uri) {
            val intent = Intent(context, AddImage::class.java)
            intent.putExtra("uri", uri)
            activity.startActivityForResult(intent, REQUEST_IMAGE_ADD)
        }
    }

    override fun onCreate() {
        super.onCreate()
        context = getApplicationContext()
    }
}

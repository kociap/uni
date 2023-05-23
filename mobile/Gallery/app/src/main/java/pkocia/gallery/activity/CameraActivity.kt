package pkocia.gallery

import android.app.Activity
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.MediaStore
import androidx.core.content.FileProvider
import java.io.File

class CameraActivity : Activity() {
    private lateinit var uri: Uri

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (savedInstanceState != null) {
            // We are returning from the camera. Do not launch it again.
            return
        }

        val storage = Application.context.filesDir
        if (!storage.exists()) {
            storage.mkdirs()
        }
        val tmp = File(storage.absolutePath + "/camera.tmp")
        // We need to grant the camera permissions to write to the temporary file.
        uri = FileProvider.getUriForFile(this, "pkocia.gallery.fileprovider", tmp)
        // Start camera activity.
        val intent = Intent(MediaStore.ACTION_IMAGE_CAPTURE)
        intent.putExtra(MediaStore.EXTRA_OUTPUT, uri)
        startActivityForResult(intent, REQUEST_IMAGE_CAPTURE)
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        super.onRestoreInstanceState(savedInstanceState)
        uri = savedInstanceState.getParcelable("uri", Uri::class.java)!!
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        outState.putParcelable("uri", uri)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode != REQUEST_IMAGE_CAPTURE) {
            super.onActivityResult(requestCode, resultCode, data)
            return
        }

        if (resultCode == RESULT_OK) {
            val intent = Intent()
            intent.putExtra("uri", uri)
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
            setResult(RESULT_OK, intent)
        } else {
            setResult(RESULT_CANCELED, null)
        }
        finish()
    }
}

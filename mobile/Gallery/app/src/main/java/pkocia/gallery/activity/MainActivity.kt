package pkocia.gallery

import android.Manifest
import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.widget.Button
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView

class MainActivity : Activity() {
    private lateinit var cameraButton: Button
    private lateinit var recyclerView: RecyclerView
    private lateinit var recyclerAdapter: GalleryViewAdapter

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        Application.activity = this

        val images = ArrayList<Image>()
        val files = Application.context.filesDir.listFiles()
        for (file in files) {
            if (file.name == "camera.tmp") {
                continue
            }

            images.add(Image(Uri.fromFile(file)))
        }

        cameraButton = findViewById(R.id.camera_button)
        recyclerView = findViewById(R.id.recycler_view)

        recyclerAdapter = GalleryViewAdapter(images)

        recyclerView.layoutManager = GridLayoutManager(applicationContext, 3)
        recyclerView.adapter = recyclerAdapter

        cameraButton.setOnClickListener { Application.startCameraActivity(this) }

        // Verify we have the required permissions
        if (checkSelfPermission(Manifest.permission.CAMERA) == PackageManager.PERMISSION_DENIED ||
            checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) ==
                PackageManager.PERMISSION_DENIED ||
            checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) ==
                PackageManager.PERMISSION_DENIED) {
            Log.e(
                "PERMISSIONS",
                "permissions to access camera or read storage or write storage are missing")
        }
    }

    override fun onRestoreInstanceState(savedInstanceState: Bundle) {
        super.onRestoreInstanceState(savedInstanceState)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        if (requestCode == REQUEST_IMAGE_CAPTURE) {
            if (resultCode == Activity.RESULT_OK) {
                val uri = data?.getParcelableExtra("uri", Uri::class.java)
                if (uri != null) {
                    Application.startAddImageActivity(this, uri)
                }
            }
        } else if (requestCode == REQUEST_IMAGE_ADD) {
            if (resultCode == Activity.RESULT_OK) {
                val uri = data!!.getParcelableExtra("uri", Uri::class.java)!!
                val image = Image(uri)
                recyclerAdapter.append(image)
            }
        } else {
            super.onActivityResult(requestCode, resultCode, data)
        }
    }
}

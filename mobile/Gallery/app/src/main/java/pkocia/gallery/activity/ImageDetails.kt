package pkocia.gallery

import android.app.Activity
import android.net.Uri
import android.os.Bundle
import android.widget.ImageView

class ImageDetails : Activity() {
    private var imageObjects = ArrayList<Image>()
    private var index = 0

    private lateinit var imageView: ImageView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.image_details)

        Application.activity = this

        imageView = findViewById(R.id.image)!!

        val uri = intent.extras!!.getParcelable("uri", Uri::class.java)!!
        imageView.setImageURI(uri)
    }
}

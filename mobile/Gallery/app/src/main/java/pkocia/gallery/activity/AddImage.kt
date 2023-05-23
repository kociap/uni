package pkocia.gallery

import android.app.Activity
import android.content.Intent
import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.ImageView
import java.io.File
import java.io.FileOutputStream

class AddImage : Activity() {
    private lateinit var uri: Uri

    private lateinit var imageView: ImageView

    private lateinit var buttonSave: Button
    private lateinit var buttonDiscard: Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.add_photo)

        Application.activity = this

        imageView = findViewById(R.id.image)!!
        buttonSave = findViewById(R.id.button_save)!!
        buttonDiscard = findViewById(R.id.button_discard)!!

        uri = intent.extras!!.getParcelable("uri", Uri::class.java)!!
        imageView.setImageURI(uri)

        buttonSave.setOnClickListener {
            val bitmap =
                ImageDecoder.decodeBitmap(
                    ImageDecoder.createSource(Application.context.contentResolver, uri))
            val name = String.format("%d.png", System.currentTimeMillis())
            val path = File(Application.context.filesDir, name)
            val output = FileOutputStream(path)
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, output)
            output.flush()
            output.close()

            val intent = Intent()
            intent.putExtra("uri", uri)
            setResult(RESULT_OK, intent)
            finish()
        }

        buttonDiscard.setOnClickListener { finish() }
    }
}

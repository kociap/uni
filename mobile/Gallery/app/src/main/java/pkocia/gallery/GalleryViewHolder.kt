package pkocia.gallery

import android.view.View
import android.widget.ImageView
import androidx.recyclerview.widget.RecyclerView

class GalleryViewHolder(view: View) : RecyclerView.ViewHolder(view) {
    private val imageView: ImageView
    private lateinit var image: Image

    init {
        imageView = view.findViewById(R.id.image)

        imageView.setOnClickListener {
            Application.startImageDetailsActivity(Application.activity!!, image)
        }
    }

    fun bind(image: Image) {
        this.image = image
        imageView.setImageURI(image.uri)
    }
}

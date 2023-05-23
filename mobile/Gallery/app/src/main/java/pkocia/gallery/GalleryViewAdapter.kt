package pkocia.gallery

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import kotlin.collections.MutableList

class GalleryViewAdapter(private val images: MutableList<Image>) :
    RecyclerView.Adapter<GalleryViewHolder>() {

    override fun getItemCount(): Int {
        return images.size
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): GalleryViewHolder {
        val view =
            LayoutInflater.from(parent.context).inflate(R.layout.gallery_photo, parent, false)
        return GalleryViewHolder(view)
    }

    override fun onBindViewHolder(holder: GalleryViewHolder, position: Int) {
        val image = images[position]
        holder.bind(image)
    }

    fun append(image: Image) {
        images.add(image)
        notifyItemInserted(images.size - 1)
    }
}

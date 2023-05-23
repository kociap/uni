package pkocia.gallery

import android.net.Uri
import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
class Image(
    var uri: Uri,
) : Parcelable

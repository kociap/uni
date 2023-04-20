package pkocia.minesweeper

import android.graphics.drawable.Drawable

import android.content.Context

object ApplicationContext {
    var context: Context? = null;

    fun getDrawable(id: Int): Drawable? {
        return context?.getDrawable(id)
    }
}

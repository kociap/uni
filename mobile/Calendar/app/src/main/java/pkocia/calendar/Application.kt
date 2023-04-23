package pkocia.calendar

import android.annotation.SuppressLint
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import androidx.core.app.NotificationCompat

class Application: android.app.Application() {
    companion object {
        @SuppressLint("StaticFieldLeak")
        private var _context: Context? = null
        val context: Context
            get() = _context!!
    }

    private val mEventNotificationManager = EventNotificationManager()

    override fun onCreate() {
        super.onCreate()
        _context = getApplicationContext()

        EventCreatedListener.register(mEventNotificationManager)
        EventService.initialise()
    }
}

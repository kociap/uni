package pkocia.calendar

import android.app.*
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.icu.util.Calendar
import androidx.core.app.NotificationCompat
import java.time.ZoneOffset

class EventNotificationManager: EventCreatedListener, BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        val event = intent.getLongExtra("event", 0)
        NotificationService.DisplayUpcomingEventNotification(event)
    }

    override fun OnEventCreated(event: Event) {
        val time = event.date.atTime(event.time)
        val evttime = time.atOffset(ZoneOffset.UTC).toInstant().toEpochMilli()
        time.minusMinutes(30)
        val notiftime = time.atOffset(ZoneOffset.UTC).toInstant().toEpochMilli()
        val systime = System.currentTimeMillis()
        if(notiftime > systime) {
            CreatePendingNotification(notiftime - systime, evttime)
        }
        // For testing.
        // CreatePendingNotification(3000, evttime)
    }

    private fun CreatePendingNotification(milliseconds: Long, event: Long) {
        val manager = Application.context.getSystemService(Context.ALARM_SERVICE) as AlarmManager
        val intent = Intent(Application.context, EventNotificationManager::class.java)
            .putExtra("event", event)
        val pending = PendingIntent.getBroadcast(Application.context, 0, intent, PendingIntent.FLAG_ONE_SHOT or PendingIntent.FLAG_IMMUTABLE)
        val time = System.currentTimeMillis() + milliseconds
        manager.setExact(AlarmManager.RTC_WAKEUP, time, pending)
    }
}

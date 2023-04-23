package pkocia.calendar

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import java.time.Instant
import java.time.LocalDateTime
import java.time.ZoneId
import java.time.ZoneOffset

object NotificationService {
    fun DisplayUpcomingEventNotification(event: Long) {
        // Create required notification channel. This is only required in API level 26 and greater.
        val notificationManager = Application.context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        val channelID = "pkocia_calendar_notifications"
        val channel = NotificationChannel(
            channelID,
            "Calendar Notifications",
            NotificationManager.IMPORTANCE_HIGH
        )
        notificationManager.createNotificationChannel(channel)

        val date: LocalDateTime =
            LocalDateTime.ofInstant(Instant.ofEpochMilli(event), ZoneOffset.UTC)
        val builder = Notification.Builder(Application.context, channelID)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentTitle("Upcoming event!")
            .setContentText("You have an upcoming event at " + CalendarUtils.formatTimeLong(date.toLocalTime()))
            .setAutoCancel(true)
        val id: Int = System.currentTimeMillis().toInt()
        notificationManager.notify(id, builder.build())
    }
}
package pkocia.calendar

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.ListView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.time.LocalTime
import java.time.format.TextStyle
import java.util.*

class DayViewActivity : AppCompatActivity() {
    private var monthDayText: TextView? = null
    private var dayOfWeekTV: TextView? = null
    private var hourListView: ListView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_day_view)

        monthDayText = findViewById(R.id.monthDayText)
        dayOfWeekTV = findViewById(R.id.dayOfWeekTV)
        hourListView = findViewById(R.id.hourListView)
    }

    private fun initWidgets() {}

    override fun onResume() {
        super.onResume()
        setDayView()
    }

    private fun setDayView() {
        val selected = Calendar.selectedDate
        monthDayText?.text = CalendarUtils.monthDayFromDate(selected)
        dayOfWeekTV?.text = selected.dayOfWeek.getDisplayName(TextStyle.FULL, Locale.getDefault())
        setHourAdapter()
    }

    private fun setHourAdapter() {
        hourListView?.adapter = HourAdapter(applicationContext, hourEventList())
    }

    private fun hourEventList(): ArrayList<HourEvent> {
        val selected = Calendar.selectedDate
        val list = ArrayList<HourEvent>()
        for (hour in 0..23) {
            val time = LocalTime.of(hour, 0)
            val events: ArrayList<Event> = EventService.getEvents(selected, time)
            val hourEvent = HourEvent(time, events)
            list.add(hourEvent)
        }
        return list
    }

    fun previousDayAction(view: View?) {
        Calendar.selectedDate = Calendar.selectedDate.minusDays(1)
        setDayView()
    }

    fun nextDayAction(view: View?) {
        Calendar.selectedDate = Calendar.selectedDate.plusDays(1)
        setDayView()
    }

    fun newEventAction(view: View?) {
        startActivity(Intent(this, EventEditActivity::class.java))
    }
}

package pkocia.calendar

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.ListView
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import java.time.LocalDate
import pkocia.calendar.CalendarAdapter.OnItemListener

class WeekViewActivity : AppCompatActivity(), OnItemListener {
    private var monthYearText: TextView? = null
    private var calendarRecyclerView: RecyclerView? = null
    private var eventListView: ListView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_week_view)

        calendarRecyclerView = findViewById(R.id.calendarRecyclerView)
        monthYearText = findViewById(R.id.monthYearTV)
        eventListView = findViewById(R.id.eventListView)

        setWeekView()
    }

    private fun setWeekView() {
        val selected = Calendar.selectedDate
        monthYearText!!.text = CalendarUtils.monthYearFromDate(selected)
        val days = CalendarUtils.getDaysOfWeek(selected)
        val calendarAdapter = CalendarAdapter(days, this)
        val layoutManager: RecyclerView.LayoutManager = GridLayoutManager(applicationContext, 7)
        calendarRecyclerView!!.layoutManager = layoutManager
        calendarRecyclerView!!.adapter = calendarAdapter
        setEventAdapater()
    }

    fun previousWeekAction(view: View?) {
        Calendar.selectedDate = Calendar.selectedDate.minusWeeks(1)
        setWeekView()
    }

    fun nextWeekAction(view: View?) {
        Calendar.selectedDate = Calendar.selectedDate.plusWeeks(1)
        setWeekView()
    }

    override fun onItemClick(position: Int, date: LocalDate) {
        Calendar.selectedDate = date
        setWeekView()
    }

    override fun onResume() {
        super.onResume()
        setEventAdapater()
    }

    private fun setEventAdapater() {
        val selected = Calendar.selectedDate
        val dailyEvents: ArrayList<Event> = EventService.getEvents(selected)
        val eventAdapter = EventAdapter(applicationContext, dailyEvents)
        eventListView!!.adapter = eventAdapter
    }

    fun newEventAction(view: View?) {
        startActivity(Intent(this, EventEditActivity::class.java))
    }

    fun dailyAction(view: View?) {
        startActivity(Intent(this, DayViewActivity::class.java))
    }
}

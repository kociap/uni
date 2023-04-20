package pkocia.calendar

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView
import java.time.LocalDate
import pkocia.calendar.CalendarAdapter.OnItemListener

class MonthViewActivity : AppCompatActivity(), OnItemListener {
    private var monthYearTV: TextView? = null
    private var calendarRecyclerView: RecyclerView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_month_view)

        calendarRecyclerView = findViewById(R.id.calendarRecyclerView)
        monthYearTV = findViewById(R.id.monthYearTV)

        Calendar.selectedDate = LocalDate.now()

        setMonthView()
    }

    private fun setMonthView() {
        val selected = Calendar.selectedDate
        monthYearTV!!.text = CalendarUtils.monthYearFromDate(selected)
        val daysInMonth = CalendarUtils.getDaysOfMonth(selected)
        val calendarAdapter = CalendarAdapter(daysInMonth, this)
        val layoutManager: RecyclerView.LayoutManager = GridLayoutManager(applicationContext, 7)
        calendarRecyclerView!!.layoutManager = layoutManager
        calendarRecyclerView!!.adapter = calendarAdapter
    }

    fun previousMonthAction(view: View?) {
        Calendar.selectedDate = Calendar.selectedDate.minusMonths(1)
        setMonthView()
    }

    fun nextMonthAction(view: View?) {
        Calendar.selectedDate = Calendar.selectedDate.plusMonths(1)
        setMonthView()
    }

    override fun onItemClick(position: Int, date: LocalDate) {
        Calendar.selectedDate = date
        setMonthView()
    }

    fun weeklyAction(view: View?) {
        startActivity(Intent(this, WeekViewActivity::class.java))
    }
}

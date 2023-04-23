package pkocia.calendar

import android.view.View
import android.widget.TextView
import android.widget.ImageView
import androidx.recyclerview.widget.RecyclerView
import java.time.LocalDate
import android.graphics.Color
import pkocia.calendar.CalendarAdapter.OnItemListener

class CalendarViewHolder(
    itemView: View,
    private val onItemListener: OnItemListener,
) : RecyclerView.ViewHolder(itemView), View.OnClickListener, EventCreatedListener {
    private val parentView: View
    private val dayText: TextView
    private val eventDot: ImageView
    private var date: LocalDate = LocalDate.now()

    init {
        parentView = itemView.findViewById(R.id.parent_view)
        dayText = itemView.findViewById(R.id.day_text)
        eventDot = itemView.findViewById(R.id.event_dot)

        itemView.setOnClickListener(this)

        EventCreatedListener.register(this)
    }

    override fun onClick(view: View) {
        onItemListener.onItemClick(adapterPosition, date)
    }

    override fun OnEventCreated(event: Event) {
        val hasEvents = EventService.hasAnyEventsOnDay(date)
        toggleEventDot(hasEvents)
    }

    fun update(date: LocalDate) {
        this.date = date
        val selected = Calendar.selectedDate
        dayText.text = date.dayOfMonth.toString()
        if (date == selected) {
            parentView.setBackgroundColor(0xffd6fcb0.toInt())
        }

        if (date.month == selected.month) {
            dayText.setTextColor(Color.BLACK)
        } else {
            dayText.setTextColor(Color.LTGRAY)
        }

        val hasEvents = EventService.hasAnyEventsOnDay(date)
        toggleEventDot(hasEvents)
    }

    private fun toggleEventDot(display: Boolean) {
        if(display) {
            eventDot.setVisibility(View.VISIBLE)
        } else {
            eventDot.setVisibility(View.GONE)
        }
    }
}

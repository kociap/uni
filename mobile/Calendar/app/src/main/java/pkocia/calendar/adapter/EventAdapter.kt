package pkocia.calendar

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.TextView

class EventAdapter(context: Context, events: List<Event>) :
    ArrayAdapter<Event>(context, 0, events) {
    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        var view = convertView
        if (view == null) {
            view = LayoutInflater.from(context).inflate(R.layout.event_cell, parent, false)
        }

        val event = getItem(position)
        if (event != null) {
            val eventTitle = event.name + " " + CalendarUtils.formatTimeLong(event.time)
            val eventCellTV = view!!.findViewById<TextView>(R.id.eventCellTV)
            eventCellTV.text = eventTitle
        }
        return view!!
    }
}

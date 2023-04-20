package pkocia.calendar

import android.annotation.SuppressLint
import android.os.Bundle
import android.view.View
import android.widget.EditText
import android.widget.TextView
import android.widget.TimePicker
import androidx.appcompat.app.AppCompatActivity
import java.time.LocalTime

class EventEditActivity : AppCompatActivity() {
    private var eventNameWidget: EditText? = null
    private var eventDateWidget: TextView? = null
    private var eventTimePicker: TimePicker? = null
    private var event: Event? = null

    @SuppressLint("SetTextI18n")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_event_edit)

        eventNameWidget = findViewById(R.id.event_name)
        eventDateWidget = findViewById(R.id.event_date)
        eventTimePicker = findViewById(R.id.event_time_picker)

        eventTimePicker?.setIs24HourView(true)

        val date = Calendar.selectedDate
        event = Event("<unnamed event>", date, LocalTime.now())
        eventDateWidget!!.text = "Date: " + CalendarUtils.formatDate(date)
    }

    fun save(view: View?) {
        val name = eventNameWidget!!.text.toString()
        if(name.length == 0) {
            eventNameWidget!!.setError("Missing event name!")
            return
        }

        event!!.name = name
        val hour = eventTimePicker!!.getHour()
        val minute = eventTimePicker!!.getMinute()
        event!!.time = event!!.time.withHour(hour).withMinute(minute)
        EventList.add(event!!)

        EventCreatedListener.invoke(event!!)

        finish()
    }

    fun cancel(view: View?) {
        finish()
    }
}

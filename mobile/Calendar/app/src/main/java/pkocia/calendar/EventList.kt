package pkocia.calendar

import java.time.LocalDate
import java.time.LocalTime
import java.util.ArrayList

object EventList {
    private var list = ArrayList<Event>()

    public fun add(event: Event) {
        list.add(event)
    }

    public fun hasAnyEventsOnDay(date: LocalDate): Boolean {
        for (event in list) {
            if (event.date == date) {
                return true
            }
        }
        return false;
    }

    public fun getEvents(date: LocalDate): ArrayList<Event> {
        val events = ArrayList<Event>()
        for (event in list) {
            if (event.date == date) {
                events.add(event)
            }
        }
        return events
    }

    public fun getEvents(date: LocalDate, time: LocalTime): ArrayList<Event> {
        val events = ArrayList<Event>()
        for (event in list) {
            val eventHour = event.time.hour
            val cellHour = time.hour
            if (event.date == date && eventHour == cellHour) {
                events.add(event)
            }
        }
        return events
    }
}

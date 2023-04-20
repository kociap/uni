package pkocia.calendar

import java.time.DayOfWeek
import java.time.LocalDate
import java.time.LocalTime
import java.time.YearMonth
import java.time.format.DateTimeFormatter

object CalendarUtils {
    fun formatDate(date: LocalDate): String {
        val formatter = DateTimeFormatter.ofPattern("dd MMMM yyyy")
        return date.format(formatter)
    }

    fun formatTimeLong(time: LocalTime): String {
        val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")
        return time.format(formatter)
    }

    fun formatTimeShort(time: LocalTime): String {
        val formatter = DateTimeFormatter.ofPattern("HH:mm")
        return time.format(formatter)
    }

    fun monthYearFromDate(date: LocalDate): String {
        val formatter = DateTimeFormatter.ofPattern("MMMM yyyy")
        return date.format(formatter)
    }

    fun monthDayFromDate(date: LocalDate): String {
        val formatter = DateTimeFormatter.ofPattern("MMMM d")
        return date.format(formatter)
    }

    fun getDaysOfMonth(date: LocalDate): ArrayList<LocalDate> {
        val yearMonth = YearMonth.from(date)
        val daysInMonth = yearMonth.lengthOfMonth()
        val prevMonth = date.minusMonths(1)
        val nextMonth = date.plusMonths(1)
        val prevYearMonth = YearMonth.from(prevMonth)
        val prevDaysInMonth = prevYearMonth.lengthOfMonth()
        val firstOfMonth = date.withDayOfMonth(1)
        val dayOfWeek = firstOfMonth.dayOfWeek.value
        val days = ArrayList<LocalDate>()
        for (i in 1..42) {
            if (i <= dayOfWeek)
                days.add(
                    LocalDate.of(prevMonth.year, prevMonth.month, prevDaysInMonth + i - dayOfWeek))
            else if (i > daysInMonth + dayOfWeek)
                days.add(LocalDate.of(nextMonth.year, nextMonth.month, i - dayOfWeek - daysInMonth))
            else days.add(LocalDate.of(date.year, date.month, i - dayOfWeek))
        }
        return days
    }

    fun getDaysOfWeek(date: LocalDate): ArrayList<LocalDate> {
        var current = sundayForDate(date)
        val endDate = current.plusWeeks(1)
        val days = ArrayList<LocalDate>()
        while (current.isBefore(endDate)) {
            days.add(current)
            current = current.plusDays(1)
        }
        return days
    }

    private fun sundayForDate(current: LocalDate): LocalDate {
        var current = current
        while (current.dayOfWeek != DayOfWeek.SUNDAY) {
            current = current.minusDays(1)
        }
        return current
    }
}

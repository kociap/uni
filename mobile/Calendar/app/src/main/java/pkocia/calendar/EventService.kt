package pkocia.calendar

import androidx.lifecycle.LiveData
import androidx.room.*
import java.time.LocalDate
import java.time.LocalTime
import java.time.format.DateTimeFormatter
import java.util.ArrayList

@Entity(tableName = "events")
private class DBEvent (
    @PrimaryKey(autoGenerate = true)
    val id: Int,
    val name: String,
    val date: LocalDate,
    val time: LocalTime
) {
    companion object {
        fun fromEvent(event: Event): DBEvent {
            return DBEvent(0, event.name, event.date, event.time)
        }

        fun toEvent(event: DBEvent): Event {
            return Event(event.name, event.date, event.time)
        }
    }
}

private class DBEventConverter {
    @TypeConverter
    fun fromLocalDate(value: LocalDate): String {
        val formatter = DateTimeFormatter.ofPattern("dd-MMMM-yyyy")
        return value.format(formatter)
    }

    @TypeConverter
    fun toLocalDate(value: String): LocalDate {
        val formatter = DateTimeFormatter.ofPattern("dd-MMMM-yyyy")
        return LocalDate.parse(value, formatter)
    }

    @TypeConverter
    fun fromLocalTime(value: LocalTime): String {
        val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")
        return value.format(formatter)
    }

    @TypeConverter
    fun toLocalTime(value: String): LocalTime {
        val formatter = DateTimeFormatter.ofPattern("HH:mm:ss")
        return LocalTime.parse(value, formatter)
    }
}

@Dao
private interface DBEventDao {
    @Query("select * from events")
    fun getAll(): List<DBEvent>

    @Insert
    fun insert(event: DBEvent)

    @Delete
    fun delete(event: DBEvent)
}

@Database(entities = [DBEvent::class], version = 1)
@TypeConverters(DBEventConverter::class)
private abstract class DBEventDatabase: RoomDatabase() {
    abstract fun getDBEventDao(): DBEventDao
}

object EventService {
    private val list = ArrayList<Event>()
    private var database: DBEventDatabase? = null

    fun initialise() {
        database = Room.databaseBuilder(
            Application.context,
            DBEventDatabase::class.java,
            "eventdb"
        ).allowMainThreadQueries().build()

        val dao = database!!.getDBEventDao()
        for(event in dao.getAll()) {
            list.add(DBEvent.toEvent(event))
        }
    }

    fun add(event: Event) {
        list.add(event)
        val dao = database!!.getDBEventDao()
        dao.insert(DBEvent.fromEvent(event))
    }

    fun hasAnyEventsOnDay(date: LocalDate): Boolean {
        for (event in list) {
            if (event.date == date) {
                return true
            }
        }
        return false;
    }

    fun getEvents(date: LocalDate): ArrayList<Event> {
        val events = ArrayList<Event>()
        for (event in list) {
            if (event.date == date) {
                events.add(event)
            }
        }
        return events
    }

    fun getEvents(date: LocalDate, time: LocalTime): ArrayList<Event> {
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

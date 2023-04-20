package pkocia.calendar

interface EventCreatedListener {
    fun OnEventCreated(event: Event)

    companion object {
        private var listeners = ArrayList<EventCreatedListener>()

        fun register(listener: EventCreatedListener) {
            listeners.add(listener)
        }

        fun invoke(event: Event) {
            for (listener in listeners) {
                listener.OnEventCreated(event)
            }
        }
    }
}

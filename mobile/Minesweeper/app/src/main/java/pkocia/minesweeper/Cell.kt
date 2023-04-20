package pkocia.minesweeper

class Cell {
    enum class State {
        HIDDEN,
        REVEALED,
        FLAGGED,
    }

    var state: State = State.HIDDEN
    var bomb: Boolean = false
    var adjacent: Int = 0

    val hidden: Boolean
        get() { return state == State.HIDDEN }
    val revealed: Boolean
        get() { return state == State.REVEALED }
    val flagged: Boolean
        get() { return state == State.FLAGGED }

    fun reveal() {
        if(state == State.HIDDEN) {
            state = State.REVEALED
        }
    }

    fun toggleFlag() {
        when(state) {
            State.HIDDEN -> state = State.FLAGGED
            State.FLAGGED -> state = State.HIDDEN
            else -> {}
        }
    }
}

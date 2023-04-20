package pkocia.minesweeper

import java.util.*

class BombGrid(size: Int) {
    private val cellsMutable = ArrayList<Cell>()
    val cells: List<Cell>
        get() = cellsMutable
    private val size: Int

    init {
        this.size = size;
        for(i in 0 until size * size) {
            cellsMutable.add(Cell())
        }
    }

    fun at(x: Int, y: Int): Cell? {
        if(x < 0 || x >= size || y < 0 || y >= size) {
            return null
        } else {
            return cellsMutable[x + y * size]
        }
    }

    fun clear() {
        for(cell in cellsMutable) {
            cell.state = Cell.State.HIDDEN
            cell.bomb = false
            cell.adjacent = 0;
        }
    }

    // Grid must be cleared before.
    fun generate(bombs: Int) {
        var placed = 0
        val engine = Random()
        while(placed < bombs) {
            val index = engine.nextInt(size * size)
            val cell = cellsMutable[index]
            if(!cell.bomb) {
                cell.bomb = true
                placed += 1
            }
        }

        for(x in 0 until size) {
            for(y in 0 until size) {
                val cell: Cell? = at(x, y)
                val adjacent = getAdjacentCells(x, y)
                for(adjcell in adjacent) {
                    if (adjcell.bomb) {
                        cell!!.adjacent += 1;
                    }
                }
            }
        }
    }

    fun getAdjacentCells(index: Int): List<Cell> {
        val x = index % size
        val y = index / size
        return getAdjacentCells(x, y)
    }

    fun getAdjacentCells(x: Int, y: Int): List<Cell> {
        val candidates: MutableList<Cell?> = ArrayList()
        candidates.add(at(x, y + 1))
        candidates.add(at(x, y - 1))
        candidates.add(at(x + 1, y))
        candidates.add(at(x + 1, y + 1))
        candidates.add(at(x + 1, y - 1))
        candidates.add(at(x - 1, y))
        candidates.add(at(x - 1, y + 1))
        candidates.add(at(x - 1, y - 1))
        val adjacent: MutableList<Cell> = ArrayList()
        for(cell in candidates) {
            if(cell != null) {
                adjacent.add(cell)
            }
        }
        return adjacent
    }
}

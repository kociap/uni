package pkocia.minesweeper

import kotlin.collections.ArrayDeque

class Game {
    enum class Status {
        IN_PROGRESS,
        ENDED_WIN,
        ENDED_LOSS,
    }

    var grid: BombGrid = BombGrid(0)
        private set
    var status: Status = Status.ENDED_LOSS
        private set
    var flagMode = false
        private set
    var bombs: Int = 0
        private set
    val flags: Int
        get() {
            var count = 0
            for(cell in grid.cells) {
                if(cell.state == Cell.State.FLAGGED) {
                    count += 1
                }
            }
            return count
        }

    fun startNewGame(size: Int, bombs: Int) {
        status = Status.IN_PROGRESS
        grid = BombGrid(size)
        grid.generate(bombs)
        this.bombs = bombs
    }

    fun endWithWin() {
        status = Status.ENDED_WIN
    }

    fun endWithLoss() {
        status = Status.ENDED_LOSS
    }

    fun hasEnded(): Boolean {
        return status == Status.ENDED_WIN || status == Status.ENDED_LOSS
    }

    fun toggleFlagMode() {
        flagMode = !flagMode
    }

    fun onCellClick(cell: Cell) {
        if (hasEnded()) {
            return
        }

        if(flagMode) {
            cell.toggleFlag()
        } else {
            cell.reveal()
            if(cell.revealed && cell.bomb) {
                endWithLoss()
            } else {
                revealTilesAround(cell)

                var unrevealed = 0
                for(c in grid.cells) {
                    if(c.hidden || c.flagged) {
                        unrevealed += 1
                    }
                }

                if(unrevealed == bombs) {
                    endWithWin()
                }
            }
        }
    }

    fun revealTilesAround(startCell: Cell) {
        if(startCell.flagged || startCell.bomb) {
            return
        }

        val index = grid.cells.indexOf(startCell)
        val frontier = ArrayDeque<Cell>()
        val startAdjacent: List<Cell> = grid.getAdjacentCells(index);
        var bombsMarked = 0
        for(cell in startAdjacent) {
            if(cell.state == Cell.State.FLAGGED) {
                bombsMarked += 1
            }
            frontier.add(cell)
        }

        // Do not run autoreveal if not enough bombs have been marked.
        if(bombsMarked < startCell.adjacent) {
            return
        }

        while (frontier.size > 0) {
            val cell: Cell = frontier.last()
            frontier.removeLast()
            if(cell.state != Cell.State.HIDDEN) {
                continue;
            }

            cell.reveal()
            if(cell.revealed && cell.bomb) {
                endWithLoss()
                return
            }
            // The frontier of our expansion must end at cells that have
            // adjacent bombs.
            if(cell.adjacent > 0) {
                continue;
            }

            val cellIndex = grid.cells.indexOf(cell)
            val adjacent: List<Cell> = grid.getAdjacentCells(cellIndex)
            for (adj in adjacent) {
                frontier.add(adj)
            }
        }
    }

    fun revealBombs() {
        for (cell in grid.cells) {
            if (cell.bomb) {
                cell.state = Cell.State.REVEALED
            }
        }
    }
}

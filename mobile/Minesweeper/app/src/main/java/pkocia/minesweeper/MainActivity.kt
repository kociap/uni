package pkocia.minesweeper

import android.content.Context
import android.graphics.drawable.GradientDrawable
import android.graphics.drawable.Drawable
import android.os.Bundle
import android.os.CountDownTimer
import android.view.View
import android.widget.TextView
import android.widget.ImageView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.GridLayoutManager
import androidx.recyclerview.widget.RecyclerView

import pkocia.minesweeper.ApplicationContext

private const val BOMB_COUNT = 20
private const val GRID_SIZE = 10

class MainActivity : AppCompatActivity(), OnCellClickListener {
    private val game: Game = Game()
    private val gridAdapter = GridRecyclerAdapter(game.grid.cells, this)
    private var flagImage: ImageView? = null
    private var flags: TextView? = null
    private var smiley: ImageView? = null
    private var time: TextView? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)

        ApplicationContext.context = getApplicationContext()

        smiley = findViewById(R.id.smiley) as ImageView?
        time = findViewById(R.id.time)
        flags = findViewById(R.id.flags)
        flagImage = findViewById(R.id.flag_image)

        val gridView: RecyclerView? = findViewById(R.id.grid)
        gridView?.setLayoutManager(GridLayoutManager(this, 10))
        gridView?.setAdapter(gridAdapter)

        smiley?.setOnClickListener(View.OnClickListener {
            createNewGame()
        })

        flagImage?.setOnClickListener(View.OnClickListener {
            game.toggleFlagMode()
            if (game.flagMode) {
                flagImage?.setBackgroundResource(R.drawable.flag_highlight)
            } else {
                flagImage?.setBackgroundResource(0)
            }
        })

        createNewGame()
    }

    override fun onCellClick(cell: Cell?) {
        if(cell == null) {
            return
        }

        if(game.status != Game.Status.IN_PROGRESS) {
            return
        }

        game.onCellClick(cell)
        if(game.hasEnded()) {
            if(game.status == Game.Status.ENDED_WIN) {
                Toast.makeText(ApplicationContext.context, "Game Won", Toast.LENGTH_SHORT).show()
            } else {
                Toast.makeText(ApplicationContext.context, "Game Over", Toast.LENGTH_SHORT).show()
            }
            game.revealBombs()
        }
        updateFlags()
        updateSmiley()
        updateTime()
        gridAdapter.setCells(game.grid.cells)
    }

    private fun createNewGame() {
        game.startNewGame(GRID_SIZE, BOMB_COUNT)
        gridAdapter.setCells(game.grid.cells)
        updateFlags()
        updateSmiley()
        updateTime()
    }

    private fun updateSmiley() {
        when(game.status) {
            Game.Status.IN_PROGRESS -> {
                val drawable: Drawable? = ApplicationContext.getDrawable(R.drawable.smiley_default)
                smiley?.setImageDrawable(drawable)
            }

            Game.Status.ENDED_WIN -> {
                val drawable: Drawable? = ApplicationContext.getDrawable(R.drawable.smiley_chad)
                smiley?.setImageDrawable(drawable)
            }

            Game.Status.ENDED_LOSS -> {
                val drawable: Drawable? = ApplicationContext.getDrawable(R.drawable.smiley_dead)
                smiley?.setImageDrawable(drawable)
            }
        }
    }

    private fun updateFlags() {
        flags?.setText(String.format("%03d", game.bombs - game.flags))
    }

    private fun updateTime() {
        time?.setText(String.format("%03d", 0))
    }
}

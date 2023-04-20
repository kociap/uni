package pkocia.minesweeper

import android.content.Context
import android.graphics.Color
import android.graphics.drawable.Drawable
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import android.widget.ImageView
import androidx.recyclerview.widget.RecyclerView

import pkocia.minesweeper.GridRecyclerAdapter.CellViewHolder

class GridRecyclerAdapter(
    private var cells: List<Cell>,
    private val listener: OnCellClickListener
) : RecyclerView.Adapter<CellViewHolder>() {
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): CellViewHolder {
        val itemView =
            LayoutInflater.from(parent.context).inflate(R.layout.cell, parent, false)
        return CellViewHolder(itemView)
    }

    override fun onBindViewHolder(holder: CellViewHolder, position: Int) {
        holder.bind(cells[position])
        holder.setIsRecyclable(false)
    }

    override fun getItemCount(): Int {
        return cells.size
    }

    fun setCells(cells: List<Cell>) {
        this.cells = cells
        notifyDataSetChanged()
    }

    inner class CellViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        val text: TextView
        val image: ImageView

        init {
            text = itemView.findViewById(R.id.cell_text)
            image = itemView.findViewById(R.id.cell_image)
        }

        fun bind(cell: Cell?) {
            if(cell == null) {
                return
            }

            itemView.setOnClickListener { listener.onCellClick(cell) }
            when(cell.state) {
                Cell.State.HIDDEN -> {
                    itemView.setBackgroundResource(R.drawable.cell_hidden)
                }

                Cell.State.REVEALED -> {
                    itemView.setBackgroundResource(R.drawable.cell_revealed)
                    if(cell.bomb) {
                        val drawable: Drawable? = ApplicationContext.getDrawable(R.drawable.bomb)
                        image.setImageDrawable(drawable)
                    } else {
                        if (cell.adjacent == 1) {
                            text.setTextColor(Color.BLUE)
                        } else if (cell.adjacent == 2) {
                            text.setTextColor(Color.GREEN)
                        } else if (cell.adjacent == 3) {
                            text.setTextColor(Color.RED)
                        }

                        if(cell.adjacent > 0) {
                            text.text = cell.adjacent.toString()
                        }
                    }
                }

                Cell.State.FLAGGED -> {
                    itemView.setBackgroundResource(R.drawable.cell_hidden)
                    val drawable: Drawable? = ApplicationContext.getDrawable(R.drawable.flag)
                    image.setImageDrawable(drawable)
                }
            }
        }
    }
}
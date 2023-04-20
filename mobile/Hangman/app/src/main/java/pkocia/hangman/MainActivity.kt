package pkocia.hangman

import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.TextView
import android.widget.EditText
import android.widget.ImageView
import android.widget.Button
import android.widget.Toast
import kotlin.random.Random
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    private var word: String = ""
    private var imageIndex = 0

    private val images = ArrayList<Int>()
    private var words: Array<String>? = null
    private val guessedLetters = ArrayList<String>()

    val win: Boolean
        get() {
            for(c in word) {
                if(!guessedLetters.contains("$c")) {
                    return false
                }
            }
            return true
        }

    val loss: Boolean
        get() {
            return imageIndex == 6
        }

    private var wordDisplay: TextView? = null
    private var userInputField: EditText? = null
    private var hangmanImage: ImageView? = null
    private var checkButton: Button? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        userInputField = findViewById(R.id.user_input_field)
        wordDisplay = findViewById(R.id.word_display)
        hangmanImage = findViewById(R.id.hangman_image)
        checkButton = findViewById(R.id.check_button)

        loadPictures()

        words = resources.getStringArray(R.array.words)

        reset()
    }

    private fun loadPictures() {
        images.add(R.drawable.one)
        images.add(R.drawable.two)
        images.add(R.drawable.three)
        images.add(R.drawable.four)
        images.add(R.drawable.five)
        images.add(R.drawable.six)
        images.add(R.drawable.seven)
    }

    private fun reset() {
        imageIndex = 0
        // Reset the image.
        hangmanImage?.setImageResource(images[0])
        guessedLetters.clear()

        word = selectRandomWord().uppercase()
        Log.d(" ", word)

        wordDisplay?.text = " _ ".repeat(word.length)
    }

    private fun processUserInput(): Boolean {
        val input: String? = userInputField?.text.toString().singleOrNull()?.uppercase()
        if (input == null) {
            Toast.makeText(this, "Choose a letter", Toast.LENGTH_SHORT).show()
            return false
        }

        if(guessedLetters.contains(input)) {
            Toast.makeText(this, "This letter has already been guessed", Toast.LENGTH_SHORT).show()
            return false
        }

        if (word.contains(input)) {
            guessedLetters.add(input)
            wordDisplay?.text = getDisplayWordString()
        } else {
            nextPicture()
        }
        return true
    }

    fun onCheckButtonClick(view: View) {
        if (win || loss) {
            reset()
            changeButton()
            return
        }

        if (!processUserInput()) {
            return
        }

        clearInputField()
        changeButton()
        if(win || loss) {
            displayGameEndImage()
        }
    }

    private fun changeButton() {
        if (win || loss) {
            checkButton?.setText("RESTART")
        } else {
            checkButton?.setText("CHECK")
        }
    }

    private fun displayGameEndImage() {
        if(win) {
            hangmanImage?.setImageResource(R.drawable.win)
        } else {
            hangmanImage?.setImageResource(R.drawable.loss)
        }
    }

    private fun clearInputField() {
        userInputField?.setText("")
    }

    private fun nextPicture() {
        imageIndex = (imageIndex + 1) % images.size
        if (imageIndex == 7) {
            imageIndex = 0
        }
        hangmanImage?.setImageResource(images[imageIndex])
    }

    private fun selectRandomWord(): String {
        val string = words?.random()
        if(string != null) {
            return string
        } else {
            return ""
        }
    }

    private fun getDisplayWordString(): String {
        var string = ""
        for (c in word) {
            if (guessedLetters.contains("$c")) {
                string += " " + c + " "
            } else {
                string += " _ "
            }
        }
        return string
    }
}

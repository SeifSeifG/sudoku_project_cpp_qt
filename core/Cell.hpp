#ifndef SUDOKU_CORE_CELL_HPP
#define SUDOKU_CORE_CELL_HPP

#include "Position.hpp"

/**
 * @file Cell.hpp
 * @brief One square of the Sudoku board.
 */

namespace sudoku {

/**
 * @brief Tests whether a value is a placeable Sudoku digit.
 *
 * Used later by SudokuBoard::setValue() to reject illegal input.
 *
 * @param value The value to test.
 * @return True for 1..9. Anything else, including Cell::EmptyValue, is not a
 *         placeable digit.
 */

/**
 * @brief One square of the board: a digit and whether it came with the puzzle.
 *
 * Encapsulation note: the digit and the locked-flag are private so that no caller
 * can reach straight into the grid and rewrite them. The accessors below are the
 * only way in.
 *
 * What this class does NOT do is enforce game rules. A Cell cannot see its
 * neighbours, so it cannot know whether a 5 conflicts with the row, or whether the
 * player is allowed to overwrite a given cell. That policy lives in SudokuBoard, which
 * can see the whole grid. Splitting it this way means there is exactly one place to
 * look when a rule misbehaves.
 *
 * The locked flag is why this is a class and not a plain int. It marks the clues
 * that came with the puzzle: the player must not edit them, and the Qt UI renders
 * them differently from digits the player entered.
 */
class Cell {
  private:
    int value_;   ///< The digit held here, 1..9, or EmptyValue when blank.
    bool locked_; ///< True when this digit came from the original puzzle.

  public:
    /// The value stored in an empty cell.
    static constexpr int EmptyValue = 0;
    static constexpr int MinValue = 1;
    static constexpr int MaxValue = Position::BoardSize;
    static constexpr int TotalCells = Position::BoardSize * Position::BoardSize; // 81

    /**
     * @brief Checks if a raw integer lies within the valid Sudoku value range (0..9).
     */
    static bool isValidValue(int val) noexcept { return val >= MinValue && val <= MaxValue; }

    /**
     * @brief Validates overall cell attribute rules.
     * Empty cells (val == 0) cannot be marked as original/given clues.
     */
    static bool isValidState(int val, bool isLocked) noexcept {
        if (!isValidValue(val)) {                // not in range of 1 to 9
            if (val == EmptyValue && isLocked) { // AND not (0 and locked)
                return false;
            }
        }
        return true;
    }

    /// @brief Constructs an empty, editable cell.
    Cell() : value_(EmptyValue), locked_(false) {}

    // Explicitly restore default Copy operations so std::vector can copy Cells
    Cell(const Cell &) noexcept = default;
    Cell &operator=(const Cell &) noexcept = default;

    // Explicitly restore Move operations as well
    Cell(Cell &&) noexcept = default;
    Cell &operator=(Cell &&) noexcept = default;

    /**
     * @brief Constructs a cell holding a digit.
     * @param value The digit, 1..9. Anything else becomes EmptyValue.
     * @param locked True to mark this cell as an original clue.
     */
    explicit Cell(int value, bool locked = false)
        : value_(isValidValue(value) ? value : EmptyValue), locked_(locked) {}

    /// @return The digit held here, or EmptyValue when the cell is blank.
    int value() const { return value_; }

    /// @return True when this digit came from the original puzzle.
    bool isLocked() const { return locked_; }

    /**
     * @brief Whether the player is allowed to change this cell.
     *
     * Givens stay locked for the whole game; empty cells and player-entered digits
     * are editable.
     *
     * @return True when the cell is not a given.
     */
    bool isEditable() const { return !locked_; }

    /// @return True when the cell holds EmptyValue.
    bool isEmpty() const { return value_ == EmptyValue; }

    /// @return True when the cell holds a digit, 1..9.
    bool hasValue() const { return static_cast<bool>(value_); }

    /**
     * @brief Stores a value in this cell.
     * @param value The digit to store, 1..9, or EmptyValue to blank the cell.
     * @warning This performs no validation and writes whatever it is locked.
     *          SudokuBoard::setValue() is the guard: it checks the digit, the
     *          given-lock, and the Sudoku rules before calling this. Do not call
     *          this from outside SudokuBoard.
     */
    void setValue(int value) { value_ = value; }

    /**
     * @brief Marks this cell as part of the original puzzle.
     *
     * Called by SudokuBoard when a puzzle is loaded or generated, never during play.
     *
     * @param lock True to lock the cell as a clue.
     */
    void setLocked(bool lock) { locked_ = lock; }
};

} // namespace sudoku

#endif // SUDOKU_CORE_CELL_HPP
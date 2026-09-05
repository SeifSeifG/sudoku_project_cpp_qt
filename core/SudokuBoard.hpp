#ifndef SUDOKU_CORE_SUDOKUBOARD_HPP
#define SUDOKU_CORE_SUDOKUBOARD_HPP

#include <array>
#include <random>

#include "core/Cell.hpp"
#include "core/Load.hpp"
#include "core/Position.hpp"
#include "core/Save.hpp"

/**
 * @file SudokuBoard.hpp
 * @brief The 9x9 grid, its contents, and the rules that govern them.
 */

namespace sudoku {

/**
 * @brief Why a move was refused.
 *
 * The board returns a reason code, never a message: TerminalUI turns these into
 * English sentences, the Qt UI turns them into status-bar text or a red highlight.
 * Putting strings in the core would hard-code a console assumption into logic the
 * GUI also has to use.
 */
enum class MoveStatus {
    Accepted,     ///< The digit was placed (or the cell was cleared).
    InvalidDigit, ///< The value is neither 1..9 nor Cell::EmptyValue.
    CellIsGiven,  ///< The cell is part of the original puzzle and is locked.
    Conflict,     ///< The digit already appears in the row, column, or box.
};

/**
 * @brief The 9x9 Sudoku grid and the rules that govern it.
 *
 * This class is the single door into the grid: every mutation in the project goes
 * through setValue() or clearCell(), so the given-lock and the conflict check
 * cannot be bypassed by the solver, the loader, or either UI.
 *
 * It knows nothing about presentation. There is no print() here on purpose --
 * TerminalUI draws the ASCII grid with its box separators, the Qt UI draws the
 * same board as widgets, and neither layout belongs in the rules.
 *
 * @note Rule of Zero: the grid is a std::vector, which acquires its memory in its
 *       own constructor and releases it in its own destructor. That makes this
 *       class RAII-correct without declaring a destructor, copy constructor, or
 *       assignment operator, the compiler generates all of them correctly.
 */
class SudokuBoard {
  private:
    std::array<std::array<Cell, Position::BoardSize>, Position::BoardSize>
        grid_; ///< Position::BoardSize rows of Position::BoardSize cells.

    /**
     * @brief Tests whether @p value already appears in @p p's row.
     * @param p     The cell being considered. The cell itself is skipped.
     * @param value The digit to look for.
     * @return True on the first match found.
     */
    bool rowContains(const Position &p, int value) const;

    /**
     * @brief Tests whether @p value already appears in @p p's column.
     * @param p     The cell being considered. The cell itself is skipped.
     * @param value The digit to look for.
     * @return True on the first match found.
     */
    bool colContains(const Position &p, int value) const;

    /**
     * @brief Tests whether @p value already appears in @p p's 3x3 box.
     * @param p     The cell being considered. The cell itself is skipped.
     * @param value The digit to look for.
     * @return True on the first match found.
     */
    bool boxContains(const Position &p, int value) const;

    /**
     * @brief Verifies that no two clues in the grid contradict each other.
     *
     * Used when loading a new Puzzle. it inspects a raw grid before any
     * board holds it, and so cannot use the member scans above.
     *
     * @param grid A 9x9 grid of digits, 0 for empty.
     * @return True when the clues are mutually consistent.
     */
    static bool gridIsSelfConsistent(
        const std::array<std::array<Cell, Position::BoardSize>, Position::BoardSize> &grid);

  public:
    /**
     * @brief Creates an empty board.
     *
     * 81 cells, all Cell::EmptyValue, none marked given.
     */
    SudokuBoard();
    SudokuBoard &operator=(const SudokuBoard &other) = default;

    /// @name Reading
    /// @{

    /**
     * @brief The cell at @p p.
     * @param p A coordinate inside the grid.
     * @return A reference to the cell, valid until the board is modified.
     * @pre p.isValid()
     * @warning This does not bounds-check. Callers holding an unchecked
     *          coordinate must test Position::isValid() first.
     */
    const Cell &at(const Position &p) const;

    /**
     * @brief Convenience for at(p).value().
     * @param p A coordinate inside the grid.
     * @return The digit held there, or Cell::EmptyValue.
     * @pre p.isValid()
     */
    int valueAt(const Position &p) const;

    /**
     * @brief Tests whether every cell holds a digit.
     *
     * @note This does not re-verify the rules: a board filled only through
     *       setValue() is correct by construction, since illegal placements were
     *       refused at the door.
     *
     * @return True when no cell is empty.
     */
    bool isComplete() const;

    /**
     * @brief Counts the cells still holding Cell::EmptyValue.
     *
     * Useful for progress display and for the generator's difficulty targets later.
     *
     * @return The number of empty cells, 0..81.
     */
    int emptyCount() const;

    /**
     * @brief Retrieves the entire row of cells for a given board position.
     *
     * Because the internal grid is structured as a vector of rows (row-major order),
     * the row exists contiguously in memory. This allows us to return a reference
     * directly to the underlying data, making this operation O(1) and highly efficient.
     *
     * @param pos The position whose row should be retrieved.
     * @return const std::vector<Cell>& A constant reference to the vector containing the row's
     * cells.
     */
    const std::array<Cell, Position::BoardSize> &getRow(const Position &pos) const;

    /**
     * @brief Retrieves the entire column of cells for a given board position.
     *
     * Since the internal grid stores data row-by-row, columns are not contiguous
     * in memory. This function iterates through each row to extract the cell at the
     * specified column, constructing a new vector.
     *
     * @param pos The position whose column should be retrieved.
     * @return std::vector<Cell> A newly constructed vector containing copies of the column's cells.
     */
    const std::array<Cell, Position::BoardSize> getColumn(const Position &pos) const;

    const std::array<Cell, Position::BoardSize> getBox(const Position &p) const;

    /// @}

    /// @name Rule checking
    /// @{

    /**
     * @brief Tests whether @p value may legally go at @p p.
     *
     * Legal means nothing in @p p's row, column, or box already holds that digit.
     * The cell at @p p is skipped during the scan; without that, re-entering a
     * digit a cell already holds would be reported as conflicting with itself.
     *
     * @param p     The target cell.
     * @param value The digit to test, 1..9.
     * @return True when the placement breaks no rule. False when @p p is out of
     *         range or @p value is not 1..9.
     * @see setValue() for callers that need to tell "illegal" apart from
     *      "malformed".
     */
    bool canPlace(const Position &p, int value) const;

    /// @}

    /// @name Writing
    /// @{

    /**
     * @brief Places a digit, or clears the cell when @p value is Cell::EmptyValue.
     *
     * Checks run in order: digit validity, given-lock, then conflicts. The first
     * failure is reported and the board is left untouched.
     *
     * @param p     The target cell.
     * @param value The digit to place, 1..9, or Cell::EmptyValue to blank the cell.
     * @return MoveStatus::Accepted on success, otherwise the reason for refusal.
     */
    MoveStatus setValue(const Position &p, int value);

    void setGiven(const Position &p);

    /**
     * @brief Blanks a cell.
     *
     * Equivalent to setValue(p, Cell::EmptyValue); provided because the intent
     * reads better at call sites, especially in the solver's backtracking step.
     *
     * @param p The cell to clear.
     * @return MoveStatus::Accepted on success, otherwise the reason for refusal.
     */
    MoveStatus clearCell(const Position &p);

    /// @}

    /// @name Puzzle setup
    /// @{

    /**
     * @brief Replaces the whole board from a 9x9 grid of digits.
     *
     * Zero means empty; every non-zero cell becomes a given.
     *
     * This is the seam between file handling and game rules. Load opens the file,
     * parses the text, and produces this grid; the board decides whether it is a
     * legal puzzle. A hard-coded array in main.cpp exercises the same entry point
     * before Load exists.
     *
     * @param grid A 9x9 grid of values in 0..9.
     * @return PuzzleLoadStatus::Accepted on success, otherwise the reason the
     *         puzzle was rejected: wrong dimensions, a value outside 0..9, or
     *         clues that already contradict each other.
     * @post On any status other than Accepted the board is left unchanged, since
     *       every check runs before the first mutation.
     */
    PuzzleLoadStatus loadValidGrid(std::string_view puzzleFileName);

    /**
     * @brief Saves this board's grid to a file.
     *
     * Delegates to Save::saveGrid(), passing the board's internal grid_.
     *
     * @param puzzleFileName Name (or path) of the file to save to.
     * @return true on success, false on failure.
     */
    bool saveGrid(const std::string &puzzleFileName) const;

    /**
     * @brief Empties every cell and clears every given flag.
     */
    void reset();

    /**
     * @brief Removes the player's digits and keeps the original clues.
     *
     * The "restart this puzzle" action both UIs will offer.
     */
    void clearPlayerEntries();

    /// @}
};

} // namespace sudoku

#endif // SUDOKU_CORE_SUDOKUBOARD_HPP
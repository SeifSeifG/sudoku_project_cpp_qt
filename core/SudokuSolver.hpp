#ifndef SUDOKU_CORE_SUDOKUSOLVER_HPP
#define SUDOKU_CORE_SUDOKUSOLVER_HPP

#include <array>
#include <optional>

#include "core/SudokuBoard.hpp"

/**
 * @file SudokuSolver.hpp
 * @brief Backtracking search over a SudokuBoard with Minimum Remaining Values (MRV) enhancement.
 */

namespace sudoku {

/**
 * @struct CandidateSet
 * @brief Holds the set of valid digits that can be placed in a specific cell.
 */
struct CandidateSet {
    /** @brief Array storing the valid digits (Cell::MinValue to Cell::MaxValue). */
    std::array<int, Cell::MaxValue - Cell::MinValue + 1> digits{};

    /** @brief The number of valid digits currently available in the set. */
    int count = 0;
};

/**
 * @struct ConstrainedCell
 * @brief Associates a board position with its computed candidate digits.
 *
 * Used primarily to find the most constrained cell (the one with the fewest
 * valid choices) to optimize the backtracking search tree.
 */
struct ConstrainedCell {
    /** @brief The coordinates of the cell on the board. */
    Position position;

    /** @brief The set of valid digits for this position. */
    CandidateSet candidates;

    /**
     * @brief Constructs a ConstrainedCell with an empty candidate set.
     *
     * Since Position has no default constructor, ConstrainedCell's default
     * constructor is implicitly deleted. Providing an explicit one fixes this.
     *
     * @param p The position of the cell on the board.
     */
    ConstrainedCell(Position p) : position(p), candidates{} {}

    /**
     * @brief Constructs a ConstrainedCell with a specific candidate set.
     *
     * @param p The position of the cell on the board.
     * @param c The pre-calculated candidate set for this cell.
     */
    ConstrainedCell(Position p, CandidateSet c) : position(p), candidates(c) {}
};

/**
 * @class SudokuSolver
 * @brief Solves Sudoku puzzles by backtracking, and counts how many solutions exist.
 *
 * The solver holds no state between calls: everything it needs is the board handed
 * to each method. It is a class rather than a pair of free functions so that a step
 * counter or a node limit has somewhere to live if the project grows.
 *
 * Every placement goes through SudokuBoard::setValue(), so the solver cannot write
 * a digit the rules forbid even by mistake. The board stays the single door into
 * the grid.
 *
 * @note The search calls SudokuBoard::canPlace() to choose a digit and setValue()
 *       then checks the rules again. That redundant scan is deliberate: bypassing
 *       the board's guard would mean a friend declaration or a raw setter, which
 *       is a poor trade for a saved scan on an 81-cell grid.
 */
class SudokuSolver {
  private:
    /**
     * @brief Evaluates which digits can be legally placed at a given position.
     *
     * @param board The current state of the Sudoku board.
     * @param p The position to check for candidate digits.
     * @return A CandidateSet containing all valid digits and their total count.
     */
    static CandidateSet countCandidates(SudokuBoard &board, const Position &p);

    /**
     * @brief Finds the empty cell with the fewest valid candidate digits.
     *
     * This implements the Minimum Remaining Values (MRV) heuristic. If a cell
     * has no valid candidates, it is returned immediately to prune the search tree early.
     *
     * @param board The current state of the Sudoku board.
     * @return An std::optional containing the most constrained cell, or
     *         std::nullopt if the board is completely filled.
     */
    static std::optional<ConstrainedCell> findMostConstrainedCell(SudokuBoard &board);

    /**
     * @brief The recursive backtracking step behind solve().
     *
     * Picks an empty cell using MRV, tries each digit that fits, and recurses.
     * On failure, it clears the cell it wrote before returning, so the board
     * unwinds to the state it had on entry.
     *
     * @param board The board to fill, modified in place.
     * @return True when the board is successfully completed, false if a dead end is reached.
     */
    static bool fillFrom(SudokuBoard &board);

    /**
     * @brief The recursive step behind countSolutions().
     *
     * Explores every branch instead of stopping at the first success, but abandons
     * the search as soon as @p limit solutions have been found.
     *
     * @param board The working board, modified in place and unwound as it returns.
     * @param limit Stop counting once this many solutions are found.
     * @return The number of solutions found in this subtree, never exceeding @p limit.
     */
    static int countFrom(SudokuBoard &board, int limit = 1);

  public:
    /**
     * @brief Fills every empty cell with a digit that satisfies the Sudoku rules.
     *
     * Backs the "Solve automatically" menu option.
     *
     * @param board The puzzle to solve. On success it is replaced by the completed
     *              board; on failure it is left exactly as it was.
     * @return True when a solution was found, false when the puzzle is unsolvable.
     *
     * @note The search runs on an internal copy and is assigned back only when it
     *       succeeds. Without that, a failed solve would leave the player's board
     *       full of the partial guesses the search happened to write last.
     */
    bool solve(SudokuBoard &board) const;

    /**
     * @brief Counts how many distinct solutions a puzzle has.
     *
     * The caller's board is never modified; the search runs on a const lvalue ref.
     *
     * @param board The puzzle to examine.
     * @param limit Stop once this many solutions have been found. The default of 2
     *              answers the only question the generator asks -- "is this puzzle
     *              unique?" -- without enumerating a search space that can be
     *              enormous for a sparse grid.
     * @return The number of solutions found, capped at @p limit. 0 means
     *         unsolvable, 1 means a unique solution, and a result equal to
     *         @p limit means "at least this many".
     */
    int countSolutions(const SudokuBoard &board, int limit = 1) const;
};

} // namespace sudoku

#endif // SUDOKU_CORE_SUDOKUSOLVER_HPP
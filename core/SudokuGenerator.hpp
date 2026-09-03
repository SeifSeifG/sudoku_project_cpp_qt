// SudokuGenerator.hpp
#ifndef SUDOKU_CORE_SUDOKU_GENERATOR_HPP
#define SUDOKU_CORE_SUDOKU_GENERATOR_HPP

#include <optional>
#include <random>

#include "core/SudokuBoard.hpp"
#include "core/SudokuSolver.hpp"

namespace sudoku {

/**
 * @brief Puzzle difficulty tiers, each mapped internally to a target clue (given) count.
 */
enum class Difficulty { Easy = 40, Medium = 32, Hard = 27, Expert = 22 };

/**
 * @brief Builds randomized Sudoku puzzles with a unique solution at a requested difficulty.
 */
class SudokuGenerator {
  public:
    /**
     * @brief Generates a new puzzle at the given difficulty.
     * @param difficulty Target difficulty; determines how many clues the returned board keeps.
     * @return A SudokuBoard populated with clues (locked as givens), otherwise empty cells,
     * guaranteed to have exactly one solution.
     * @throws std::runtime_error if no puzzle converges within maxGenerationAttempts random
     * restarts.
     */
    std::optional<SudokuBoard> generate(Difficulty difficulty);

  private:
    /**
     * @brief Seeds an empty board with 9 cells that are mutually exclusive by row and column.
     * @param board Board to populate; assumed empty on entry.
     * @param rng Random engine used to shuffle both the column pairing and the candidate values.
     * @note Cells are not required to hold distinct digits -- only row, column, and box rules
     * apply.
     */
    void placeInitialTransversal(SudokuBoard &board, std::mt19937 &rng);

    /**
     * @brief Adds clues one at a time until the target count is reached with a uniquely solvable
     * puzzle.
     * @param board Board to populate; expected to already hold the initial transversal.
     * @param rng Random engine used for cell and value selection.
     * @param targetGivens Number of filled cells the board should hold once a unique solution is
     * confirmed.
     * @return true if a puzzle at targetGivens with exactly one solution was reached; false if the
     * search stalled first.
     * @note Below the target, only checks that a solution exists (cheap); uniqueness is only
     * checked once the target count is hit (expensive), since sparser boards almost always have
     * many solutions and would reject nearly everything if checked early.
     */
    bool growPuzzle(SudokuBoard &board, std::mt19937 &rng, int targetGivens);

    /**
     * @brief Picks a uniformly random empty cell on the board.
     * @param board Board to search.
     * @param rng Random engine used for sampling.
     * @return Position of an empty cell.
     * @warning Loops indefinitely if the board has no empty cells -- caller must not invoke this on
     * a full board.
     */
    Position randomEmptyPosition(const SudokuBoard &board, std::mt19937 &rng) const;

    SudokuSolver solver_; ///< Used to check solution existence during growth and uniqueness once
                          ///< the target is reached.
    static constexpr int maxGenerationAttempts =
        10; ///< Number of full random restarts generate() tries before giving up.
};

} // namespace sudoku

#endif // SUDOKU_CORE_SUDOKU_GENERATOR_HPP
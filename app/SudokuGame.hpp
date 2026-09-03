#ifndef SUDOKU_APP_SUDOKUGAME_HPP
#define SUDOKU_APP_SUDOKUGAME_HPP

#include <string_view>
#include <vector>

#include "app/GameObserver.hpp"
#include "core/SudokuBoard.hpp"
#include "core/SudokuGenerator.hpp"
#include "core/SudokuSolver.hpp"

/**
 * @file SudokuGame.hpp
 * @brief The UI-agnostic coordinator between the board, the solver, and a frontend.
 */

namespace sudoku {

/**
 * @brief Runs a game of Sudoku without knowing how it is displayed.
 *
 * SudokuGame owns the board and the solver and exposes one method per player
 * action. It is the only class that decides what a command means: which core call
 * to make, and which observer callbacks that outcome deserves.
 *
 * @par Deviation from the brief
 * The brief assigns this class the console menu and the reading and writing of user
 * input. That is precisely what would make a Qt frontend impossible, so the menu
 * moves to TerminalUI and this class keeps only the coordination. Document this in
 * the report.
 *
 * @par No input loop
 * There is deliberately no run() here. TerminalUI blocks on std::getline and drives
 * its own loop; Qt's event loop owns the thread and calls in from slot handlers.
 * A loop inside the controller would suit the first and be unusable to the second,
 * so every method below is a synchronous command that returns immediately.
 *
 * @par Return values and callbacks
 * Commands both return a status and notify the observer. The return value is for
 * the caller's own control flow -- the code that issued the command and needs to
 * know what happened next. The callbacks are for anything watching the game, which
 * in Qt is not necessarily the widget that issued the command.
 *
 * @note Rule of Zero. The board and solver are members by value, and observer_ is a
 *       non-owning pointer to an object that outlives this one. Nothing here is
 *       owned by hand, so the compiler-generated destructor, copy constructor, and
 *       assignment operator are all correct.
 */
class SudokuGame {
  private:
    SudokuBoard board_;   ///< The grid being played.
    SudokuSolver solver_; ///< Stateless; a member so solve() reads as a game action.s
    SudokuGenerator generator_;
    GameObserver *observer_; ///< Non-owning, may be null. Never a concrete UI type.

  public:
    /**
     * @brief Constructs a game over an empty board.
     *
     * @param observer The frontend to notify, or nullptr to run unobserved. Not
     *                 owned: it must outlive this game.
     */
    explicit SudokuGame(GameObserver *observer = nullptr);

    /**
     * @brief Attaches or replaces the observer.
     *
     * Needed because a frontend and its game are usually mutually referential: the
     * UI cannot be passed to this constructor if the UI itself needs the game's
     * address to be built. Construct both, then connect them here.
     *
     * @param observer The frontend to notify, or nullptr to detach. Not owned.
     */
    void setObserver(GameObserver *observer);

    /**
     * @brief provides a window to the board for the derived UI classes
     *
     * @return a constant reference of the board
     */
    const SudokuBoard &board() const;

    /// @name Commands
    /// @{

    /**
     * @brief Starts a new puzzle from a 9x9 grid of digits.
     *
     * Zero means empty; every non-zero cell becomes a locked given. Until Load
     * exists this is the only way a puzzle enters the game, so main.cpp hands it the
     * brief's sample array.
     *
     * @param grid A 9x9 grid of values in 0..9.
     * @return PuzzleLoadStatus::Accepted, or why the puzzle was rejected.
     * @post On success: onBoardChanged() then onPuzzleLoaded(Accepted). On failure:
     *       onPuzzleLoaded(reason) alone, and any game already in progress is left
     *       untouched, because the board validates the whole grid before it mutates.
     */
    PuzzleLoadStatus loadPuzzle(std::string_view puzzleFileName);

    /**
     * @brief Saves the current puzzle state to a file.
     *
     * @param puzzleFileName Name (or path) of the file to save to. A ".txt"
     *                        extension is appended automatically if not present.
     * @return true if the puzzle was written successfully, false otherwise
     *         (e.g., file could not be opened or a write error occurred).
     */
    bool savePuzzle(const std::string &puzzleFileName) const;

    /**
     * @brief Places the player's digit, or clears the cell.
     *
     * @param p     The target cell. Being a Position, it is already inside the grid;
     *              a UI must screen raw input with Position::isValidPosition()
     *              before constructing one.
     * @param value The digit to place, 1..9, or Cell::EmptyValue to erase.
     * @return MoveStatus::Accepted, or why the move was refused.
     * @post On success: onBoardChanged(), then onPuzzleCompleted() if that move
     *       filled the last empty cell. On failure: onMoveRejected(reason) alone.
     */
    MoveStatus makeMove(const Position &p, int value);

    /**
     * @brief Fills every empty cell automatically.
     *
     * Backs the brief's "Solve automatically" menu option.
     *
     * @return True when a solution was found.
     * @post On success: onBoardChanged() then onSolveSucceeded(). On failure:
     *       onSolveFailed() alone, with the board untouched -- the search runs on a
     *       copy, so a failed solve cannot litter the player's grid with guesses.
     * @note onPuzzleCompleted() is not raised. A puzzle the machine finished is not
     *       a win, and keeping the two apart is what lets a UI congratulate the
     *       player only when the player earned it.
     */
    bool solve();

    bool generatePuzzle(Difficulty difficulty);

    /// @}

    /**
     * @brief Asks the observer to redraw the current board.
     *
     * Exists because the board is pushed to the observer rather than exposed by an
     * accessor: without this, a UI could only draw the grid at the instant something
     * changed. A menu loop needs to reprint it at the top of every iteration, and
     * Qt needs it after a window is shown or resized.
     *
     * Changes nothing. Does nothing when no observer is attached.
     *
     * @post onBoardChanged() is raised.
     */
    void refresh() const;
};

} // namespace sudoku

#endif // SUDOKU_APP_SUDOKUGAME_HPP

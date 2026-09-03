#ifndef SUDOKU_APP_GAMEOBSERVER_HPP
#define SUDOKU_APP_GAMEOBSERVER_HPP

#include "core/Load.hpp"
#include "core/Save.hpp"
#include "core/SudokuBoard.hpp"

/**
 * @file GameObserver.hpp
 * @brief The interface through which SudokuGame reports what happened.
 */

namespace sudoku {

/**
 * @brief What a user interface must implement to hear from SudokuGame.
 *
 * This interface is the inversion that makes the frontends swappable. Commands run
 * downward -- a UI calls SudokuGame directly -- while notifications run back upward
 * through this abstract type. SudokuGame holds an GameObserver*, never a
 * TerminalUI* or a QtMainWindow*, so the controller can be linked against either
 * frontend without a line of it changing.
 *
 * It lives in app/ rather than ui/ on purpose. Putting it in ui/ would make app/
 * depend on ui/ and invert the dependency rule; the interface belongs to the layer
 * that calls it, not the layer that implements it.
 *
 * Every method is pure virtual. Splitting the outcomes into separate callbacks is
 * only worth doing if each frontend is made to answer for all of them, and a missing
 * override then fails to compile rather than silently swallowing an event.
 *
 * @note No English text crosses this interface. Failures arrive as the same status
 *       enums the core uses, and each UI supplies its own wording -- which is what
 *       lets the terminal print a sentence and Qt colour a cell from one value.
 */
class GameObserver {
  public:
    /**
     * @brief Virtual destructor.
     *
     * The one destructor this project declares. UIs are held and destroyed through
     * IGameObserver*, and deleting a derived object through a base pointer whose
     * destructor is not virtual is undefined behaviour.
     * Defaulted, because this interface owns nothing.
     */
    virtual ~GameObserver() = default;

    /**
     * @brief The grid changed and should be redrawn.
     *
     * Fires only when a command actually modified the board, and always before the
     * outcome callback that accompanies it, so the message a UI prints describes a
     * board the user has already been shown.
     *
     * @param board The board in its new state.
     * @warning The reference is valid only for the duration of this call. It refers
     *          to the board owned by SudokuGame, so a UI must read what it needs and
     *          not store the reference.
     */
    virtual void onBoardChanged(const SudokuBoard &board) = 0;

    /**
     * @brief A move was refused and the board was left untouched.
     * @param reason Why the digit was not placed.
     */
    virtual void onMoveRejected(MoveStatus reason) = 0;

    /**
     * @brief A puzzle load finished, successfully or not.
     * @param status PuzzleLoadStatus::Accepted, or why the puzzle was rejected.
     */
    virtual void onPuzzleLoaded(PuzzleLoadStatus status) = 0;

    /**
     * @brief A puzzle is saved successfully.
     */
    virtual void onPuzzleSave() = 0;

    /**
     * @brief The automatic solver filled the board.
     *
     * Distinct from onPuzzleCompleted(): the puzzle was finished by the machine, so
     * a UI should report a solution rather than congratulate the player.
     */
    virtual void onSolveSucceeded() = 0;

    /**
     * @brief The automatic solver found no solution.
     *
     * The board is left exactly as it was; the solver searches on a copy.
     */
    virtual void onSolveFailed() = 0;

    /**
     * @brief The player's own move filled the last empty cell.
     *
     * The win condition, and the reason this is separate from onSolveSucceeded().
     * A board completed by the solver never raises it.
     */
    virtual void onPuzzleCompleted() = 0;

    /**
     * @brief A new puzzle was generated and is ready to play.
     *
     * Unlike the other outcome callbacks, this one carries the board itself rather
     * than firing alongside a separate onBoardChanged(): generation replaces the
     * entire board in one step, so there is no earlier "changed" moment to report
     * before this fires. A UI that wants a redraw through onBoardChanged() as well
     * (as TerminalUI does) is responsible for calling it itself from here.
     *
     * @warning The reference is valid only for the duration of this call, exactly
     *          as in onBoardChanged().
     */
    virtual void onPuzzleGenerationSucceeded() = 0;

    /**
     * @brief Generation could not converge on a puzzle within its attempt budget.
     *
     * The board is left exactly as it was before generatePuzzle() was called.
     */
    virtual void onPuzzleGenerationFailed() = 0;
};

} // namespace sudoku

#endif // SUDOKU_APP_GAMEOBSERVER_HPP

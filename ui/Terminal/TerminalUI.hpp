#ifndef SUDOKU_UI_TERMINALUI_HPP
#define SUDOKU_UI_TERMINALUI_HPP

#include "app/GameObserver.hpp"
#include "app/SudokuGame.hpp"
#include <string>

/**
 * @file TerminalUI.hpp
 * @brief The console frontend: the menu loop, the ASCII grid, and all English text.
 */

namespace sudoku {

/**
 * @brief Plays the game on a text console.
 *
 * Implements GameObserver, so the game pushes board changes and outcomes here rather
 * than this class polling for them. It also owns the menu loop: run() blocks on
 * std::cin and drives the whole session, which is why SudokuGame has no loop of its
 * own -- Qt's event loop will call in from the other direction.
 *
 * @par The only place English lives
 * The core and app layers report enum codes. Every sentence the player reads is
 * produced here, by describe(). The Qt frontend maps the same enums to its own
 * wording, which is what lets one status value serve both.
 *
 * @par The only place 1-based numbers live
 * The player types rows and columns as 1..9; every Position in the project is 0..8.
 * The conversion happens in readMove() and nowhere else.
 *
 * @warning This class is the boundary where untrusted input becomes a Position, and
 *          Position's constructor throws on an out-of-range coordinate. readMove()
 *          screens with Position::isValidCoordinate() before constructing, so the
 *          ordinary case of a mistyped row never reaches a throw. An uncaught
 *          InvalidPosition would call std::terminate and lose the player's game.
 */
class TerminalUI : public GameObserver {
  private:
    SudokuGame &game_; ///< non-owning reference; must outlive this UI.
    bool running_;     ///< Cleared by the Exit option to end run()'s loop.
    bool needsRedraw_; ///< Set by onBoardChanged(), consumed at the top of run()'s loop.
    bool gameFinished_;

    /// @name Rendering
    /// @{

    /**
     * @brief Draws the 9x9 grid in the format the brief specifies.
     *
     * Empty cells print as a dot, and rules of dashes separate the bands of boxes.
     *
     * @param board The board to draw.
     */
    void printBoard(const SudokuBoard &board) const;

    /// @brief Prints the numbered menu of actions.
    void printMenu() const;

    /// @}

    /// @name Input
    /// @{

    /**
     * @brief Clears a failed std::cin and discards the rest of the line.
     *
     * Without this, a non-numeric entry leaves the stream in a failed state, every
     * later read returns immediately, and the menu spins forever.
     * common bug in a console project of this shape.
     */
    void recoverFromBadInput() const;

    /**
     * @brief Reads one integer from the console.
     *
     * @param prompt Printed before reading.
     * @param out    Receives the value on success; untouched on failure.
     * @return True on a well-formed integer. On failure the stream has already been
     *         recovered, so the caller may prompt again immediately.
     */
    bool readInt(const char *prompt, int &out) const;

    bool readString(const char *prompt, std::string &out) const;

    /**
     * @brief Reads a row, a column, and a value from one line of input.
     *
     * Converts the player's 1-based row and column to the 0-based coordinate the
     * rest of the project uses, and screens them with Position::isValidCoordinate()
     * before constructing the Position.
     *
     * @param p     Receives the target cell on success.
     * @param value Receives the digit on success.
     * @return True when all three numbers were read and the coordinate is inside the
     *         grid. On failure an explanatory line has already been printed.
     */
    bool readMove(Position &p, int &value) const;

    /// @}

    /// @name Menu actions
    /// @{

    /// @brief Prompts for a move and hands it to the game.
    void handleEnterMove();

    /// @brief Runs the automatic solver.
    void handleSolve();

    /// @brief loads a puzzle after prompting the user for its name
    void handleLoad();

    /// @brief saves a puzzle after prompting the user for its name
    void handleSave();

    void handleGenerate();

    void handleNotImplemented() const;
    /// @}

    /// @name Enum to English
    /// @{

    /**
     * @brief The message for a refused move.
     * @param status The reason the board gave.
     * @return A sentence to print. Never null.
     */
    static const std::string describe(MoveStatus status);

    /**
     * @brief The message for a rejected puzzle.
     * @param status The reason the board gave.
     * @return A sentence to print. Never null.
     */
    static const std::string describe(PuzzleLoadStatus status);

    /// @}

  public:
    /**
     * @brief Binds the UI to a game.
     *
     * Does not register itself as the observer: the caller does that with
     * SudokuGame::setObserver(), because both objects must exist before they can
     * point at each other.
     *
     * @param game The game to drive. Not owned; it must outlive this UI.
     */
    explicit TerminalUI(SudokuGame &game);

    /**
     * @brief Runs the menu loop until the player exits.
     *
     * Draws the board, prints the menu, reads a choice, and dispatches. Returns when
     * the Exit option is chosen or std::cin reaches end of file, so that closing the
     * input stream ends the program cleanly instead of looping on a dead stream.
     */
    void run();

    /// @name GameObserver
    /// @{

    /**
     * @copydoc GameObserver::onBoardChanged
     *
     * Redraws the grid immediately.
     */
    void onBoardChanged(const SudokuBoard &) override;

    /// @copydoc GameObserver::onMoveRejected
    void onMoveRejected(MoveStatus reason) override;

    /// @copydoc GameObserver::onPuzzleLoaded
    void onPuzzleLoaded(PuzzleLoadStatus status) override;

    /// @copydoc GameObserver::onPuzzleSave
    void onPuzzleSave() override;

    /// @copydoc GameObserver::onSolveSucceeded
    void onSolveSucceeded() override;

    /// @copydoc GameObserver::onSolveFailed
    void onSolveFailed() override;

    /// @copydoc GameObserver::onPuzzleCompleted
    void onPuzzleCompleted() override;

    /// @copydoc GameObserver::onPuzzleGenerationSucceeded
    void onPuzzleGenerationSucceeded() override;

    /// @copydoc GameObserver::onPuzzleGenerationFailed
    void onPuzzleGenerationFailed() override;

    /// @}
};

} // namespace sudoku

#endif // SUDOKU_UI_TERMINALUI_HPP
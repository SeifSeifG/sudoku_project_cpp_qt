#ifndef SUDOKU_UI_MAINWINDOW_HPP
#define SUDOKU_UI_MAINWINDOW_HPP

#include <QMainWindow>
#include <array>

#include "app/GameObserver.hpp"
#include "app/SudokuGame.hpp"
#include "core/Position.hpp"

class QLabel;
class QAction;
class QKeyEvent;

namespace sudoku {

class QCell;

/**
 * @file MainWindow.hpp
 * @brief The Qt frontend: the main window, the board of SudokuCell widgets, and
 *        all English text this UI shows the player.
 */

/**
 * @brief Plays the game in a Qt Widgets window.
 *
 * Implements GameObserver exactly as TerminalUI does, so SudokuGame pushes board
 * changes and outcomes here without knowing this is a GUI. Unlike TerminalUI there
 * is no run() -- Qt's own event loop drives the application, and every Qt slot
 * below that changes game state calls straight into SudokuGame rather than
 * touching SudokuBoard/SudokuSolver/SudokuGenerator directly, mirroring the rule
 * that only SudokuGame decides what a command means.
 *
 * @par The only place English lives (Qt side)
 * Exactly as documented on TerminalUI: the core and app layers report enum codes,
 * and describe() is where those become sentences for this window's status bar.
 *
 * @par The only place 1-based numbers live (Qt side)
 * The Selection panel shows the player 1-based row/column numbers for the same
 * readability reason TerminalUI converts on input/output; internally every
 * Position constructed here is still 0-based, exactly as the rest of the project
 * requires.
 */
class MainWindow : public QMainWindow, public GameObserver {
    Q_OBJECT

  private:
    SudokuGame &game_; ///< non-owning reference; must outlive this window.

    /// The 9x9 grid of cell widgets, indexed [row][col].
    std::array<std::array<QCell *, Position::BoardSize>, Position::BoardSize> cells_;

    int selectedRow_;   ///< 0..8, meaningful only when hasSelection_.
    int selectedCol_;   ///< 0..8, meaningful only when hasSelection_.
    bool hasSelection_; ///< False until the player clicks a cell.

    // Selection panel value labels.
    QLabel *rowValueLabel_;
    QLabel *colValueLabel_;
    QLabel *valueValueLabel_;
    QLabel *typeValueLabel_;

    // Status bar's single permanent label.
    QLabel *statusLabel_;

    QAction *loadAction_;
    QAction *saveAction_;
    QAction *exitAction_;
    QAction *enterMoveAction_;
    QAction *solveAction_;
    QAction *generateAction_;
    QAction *aboutAction_;

    /// @name Window construction
    /// @{
    void setupMenuBar();
    void setupCentralWidget();
    void setupStatusBar();
    /// @}

    /// @name Selection bookkeeping
    /// @{

    /// @brief Makes (row, col) the current selection and refreshes dependent UI.
    void selectCell(int row, int col);

    /// @brief Clear all selection and refreshes dependent UI.
    void clearCellSelection();

    /// @brief Recomputes each cell's selected/highlighted flags from the current
    ///        selection state and repaints them.
    void updateHighlighting();

    /// @brief Rewrites the four Selection panel labels from the current selection
    ///        (and, when there is one, the board's own state at that cell).
    void updateSelectionPanel();

    /// @}

    /**
     * @brief Writes a message to the status bar.
     * @param message The text to show.
     * @param isError True to render it in red (#cc0000); false for black.
     */
    void showStatusMessage(const QString &message, bool isError);

    /// @name Enum to English
    /// @{

    /// @brief The status bar message for a refused move. Never empty.
    static QString describe(MoveStatus status);

    /// @brief The status bar message for a rejected puzzle. Never empty.
    static QString describe(PuzzleLoadStatus status);

    /// @}

  protected:
    /**
     * @brief Routes 1-9, Delete/Backspace, and the arrow keys to the selected cell.
     *
     * Digits and Delete/Backspace are handed straight to SudokuGame::makeMove();
     * whether the target cell is a given or the digit conflicts is not
     * pre-checked here, since SudokuBoard already enforces both and reports the
     * refusal back through onMoveRejected(). The arrow keys move the selection by
     * one cell without touching the board at all.
     */
    void keyPressEvent(QKeyEvent *event) override;

  private slots:
    void onCellClicked(int row, int col);
    void onLoadPuzzle();
    void onSavePuzzle();
    void onSolveAutomatically();
    void onGenerateNewPuzzle();
    void onAbout();

  public:
    /**
     * @brief Builds the window. Does not attach itself as the game's observer.
     *
     * Exactly as with TerminalUI: both objects must exist before they can point
     * at each other, so the caller (main.cpp) constructs this window and then
     * calls SudokuGame::setObserver(this).
     *
     * @param game   The game to drive. Not owned; it must outlive this window.
     * @param parent Owning widget, passed straight to QMainWindow.
     */
    explicit MainWindow(SudokuGame &game, QWidget *parent = nullptr);

    /// @name GameObserver
    /// @{

    /**
     * @copydoc GameObserver::onBoardChanged
     *
     * Repaints every SudokuCell from the given board, then reapplies
     * highlighting and refreshes the Selection panel (the selected cell's value
     * or type may have just changed).
     */
    void onBoardChanged(const SudokuBoard &board) override;

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

    /**
     * @copydoc GameObserver::onPuzzleGenerationSucceeded
     *
     * Unlike onBoardChanged(), generatePuzzle() does not carry the new board to
     * this callback, so this method fetches it itself via SudokuGame::board()
     * and calls onBoardChanged() directly -- the same responsibility TerminalUI
     * takes on for the same reason.
     */
    void onPuzzleGenerationSucceeded() override;

    /// @copydoc GameObserver::onPuzzleGenerationFailed
    void onPuzzleGenerationFailed() override;

    /// @}
};

} // namespace sudoku

#endif // SUDOKU_UI_MAINWINDOW_HPP
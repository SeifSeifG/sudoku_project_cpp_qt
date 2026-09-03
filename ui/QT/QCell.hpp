#ifndef SUDOKU_UI_SUDOKUCELL_HPP
#define SUDOKU_UI_SUDOKUCELL_HPP

#include <QWidget>

#include "core/Cell.hpp"
#include "core/Position.hpp"

class QMouseEvent;
class QPaintEvent;

/**
 * @file SudokuCell.hpp
 * @brief One square of the Qt board: a plain QWidget that paints itself and
 *        reports clicks.
 */

namespace sudoku {

/**
 * @brief A single interactive Sudoku cell in the Qt grid.
 *
 * SudokuCell owns no game logic whatsoever -- it is purely a presentation widget.
 * It knows its own (row, col), the digit and given-flag it was last told to show,
 * and whether MainWindow currently considers it selected or highlighted. All of
 * that state is pushed in from outside via setValue()/setSelected()/setHighlighted();
 * MainWindow is the only class that ever calls them, driven by SudokuGame's
 * GameObserver callbacks and by its own selection bookkeeping.
 *
 * A click is reported upward as clicked(row, col) rather than handled here, for the
 * same reason GameObserver exists: this widget should not need to know what
 * "selecting a cell" means to the rest of the application.
 */
class QCell : public QWidget {
    Q_OBJECT

  private:
    int row_;          ///< 0..8, fixed for the lifetime of this widget.
    int col_;          ///< 0..8, fixed for the lifetime of this widget.
    int value_;        ///< The digit to paint, or Cell::EmptyValue for a blank cell.
    bool given_;       ///< True to paint bold black text (a locked clue).
    bool selected_;    ///< True to paint the #316ac5 selected background.
    bool highlighted_; ///< True to paint the #dce9fc highlighted background.

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  public:
    /**
     * @brief Constructs a blank, unselected cell at a fixed board position.
     * @param row    0..8, this cell's row. Never changes afterward.
     * @param col    0..8, this cell's column. Never changes afterward.
     * @param parent Owning widget, passed straight to QWidget.
     */
    explicit QCell(int row, int col, QWidget *parent = nullptr);

    /// @return This cell's row, 0..8.
    int row() const;

    /// @return This cell's column, 0..8.
    int col() const;

    /**
     * @brief Updates what this cell displays and repaints it.
     * @param value The digit to show, or Cell::EmptyValue for blank.
     * @param given True to render as a locked clue (bold black); false for a
     *              player-entered digit (blue).
     */
    void setValue(int value, bool given);

    /// @brief Marks/unmarks this cell as the current selection and repaints it.
    void setSelected(bool selected);

    /// @brief Marks/unmarks this cell as sharing the selection's row/col/box.
    void setHighlighted(bool highlighted);

  signals:
    /**
     * @brief Raised on a left-click anywhere in this cell.
     * @param row This cell's row, 0..8.
     * @param col This cell's column, 0..8.
     */
    void clicked(int row, int col);
};

} // namespace sudoku

#endif // SUDOKU_UI_SUDOKUCELL_HPP
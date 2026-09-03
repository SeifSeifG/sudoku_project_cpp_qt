#ifndef SUDOKU_CORE_POSITION_HPP
#define SUDOKU_CORE_POSITION_HPP

#ifndef SUDOKU_BOARD_SIZE
#define SUDOKU_BOARD_SIZE 9
#endif

#include <exception>
#include <utility>

/**
 * @file Position.hpp
 * @brief Board geometry constants and the coordinate type used across the project.
 */

namespace sudoku {
/**
 * @name Coordinate component types
 *
 * @warning These are aliases, not distinct types -- row_t and col_t are both int,
 *          so the compiler will not catch a swapped pair. They document intent for
 *          a reader; they do not enforce it. That is why the accessors in Position
 *          are the only code that touches .first / .second: every other method uses
 *          row() and col(), where a mix-up is visible on sight.
 * @{
 */
using row_t = int; ///< A row index, 0..8.
using col_t = int; ///< A column index, 0..8.
/// @}

/**
 * @brief Thrown when a coordinate outside the 9x9 grid is constructed.
 *
 * Carries the offending row and column and no message: what() returns only the
 * type name, because any English wording belongs to the UI that will show it.
 *
 * @see Position::isValidPosition() to test a coordinate without a try/catch.
 */
class InvalidPosition : public std::exception {
  private:
    int row_;
    int col_;

  public:
    /**
     * @param row The rejected row index.
     * @param col The rejected column index.
     */
    InvalidPosition(int row, int col) noexcept : row_(row), col_(col) {}

    /// @return The rejected row index.
    int row() const noexcept { return row_; }

    /// @return The rejected column index.
    int col() const noexcept { return col_; }

    /// @return The type name only -- for a debugger or a log, not for the player.
    const char *what() const noexcept override { return "sudoku::InvalidPosition"; }
};

constexpr int computeBoxSize(int size) {
    int root = 1;
    while (root * root < size) {
        ++root;
    }
    return root;
}

/**
 * @brief A coordinate on the board, guaranteed to be inside the grid.
 *
 * Always valid by construction: the constructors reject anything outside 0..8, so
 * every Position that exists can be used to index the board without a further
 * check. Code that receives a Position never has to ask whether it is in range --
 * that question was settled when the object was built.
 *
 * Immutable by design: a Position is set at construction and never changes. To
 * refer to a different cell, make a different Position. This is why there are no
 * setters and every accessor is const.
 *
 * @warning Row and column are ALWAYS 0-based (0..8) everywhere in the project. The
 *          user-facing 1..9 numbering exists only inside TerminalUI, which converts
 *          on input and output. Never store or pass a 1-based value in a Position.
 */
class Position {
  private:
    /// first is the row, second is the column. Only row() and col() may read
    /// these members directly.
    std::pair<row_t, col_t> coords_;

  public:
    /// @name Board geometry
    /// Defined once here so no other file hard-codes 9 or 3.
    /// @{

    static constexpr int BoardSize = SUDOKU_BOARD_SIZE;        ///< Cells per row / column.
    static constexpr int BoxSize = computeBoxSize(BoardSize);  ///< Cells per side of a 3x3 box.
    static constexpr int BoxCount = computeBoxSize(BoardSize); ///< Boxes per row / column.

    /// @}

    /**
     * @brief Tests whether a row and column pair would make a valid Position.
     *
     * Lets a caller check user input before constructing, so the ordinary path of
     * a mistyped coordinate does not have to go through a try/catch.
     *
     * @param row The row index to test.
     * @param col The column index to test.
     * @return True when both lie in 0..8.
     */
    static bool isValidPosition(row_t row, col_t col) noexcept {
        return row >= 0 && row < BoardSize && col >= 0 && col < BoardSize;
    }

    /**
     * @brief Constructs a coordinate from a row and a column.
     * @param row The row index, 0..8.
     * @param col The column index, 0..8.
     * @throws InvalidPosition when either index lies outside the grid.
     */
    Position(row_t row, col_t col) : coords_(row, col) {
        if (!isValidPosition(row, col)) {
            throw InvalidPosition(row, col);
        }
    }

    /**
     * @brief Constructs a coordinate from an existing pair.
     *
     * Not explicit: converting a pair<row_t, col_t> to a Position loses nothing
     * and reads naturally at call sites.
     *
     * @param coords A (row, column) pair.
     * @throws InvalidPosition when either index lies outside the grid.
     */
    Position(const std::pair<row_t, col_t> &coords) : Position(coords.first, coords.second) {
        if (!isValidPosition(coords.first, coords.second)) {
            throw InvalidPosition(coords.first, coords.second);
        }
    }

    /// @return The row index, always 0..8.
    row_t row() const { return coords_.first; }

    /// @return The column index, always 0..8.
    col_t col() const { return coords_.second; }

    /**
     * @brief The raw pair.
     *
     * For callers that want std::pair's own comparisons or to use a Position as a
     * map key.
     *
     * @return A reference to the underlying (row, column) pair.
     */
    const std::pair<row_t, col_t> &coords() const { return coords_; }

    /// @return The row index of the top-left cell of this position's box.
    row_t boxStartRow() const { return (row() / BoxSize) * BoxSize; }

    /// @return The column index of the top-left cell of this position's box.
    col_t boxStartCol() const { return (col() / BoxSize) * BoxSize; }

    /**
     * @brief The index of the 3x3 box containing this cell (from 0 to 80).
     *
     * This is the only place the box formula is written.
     *
     * @return The box index, 0..8, numbered left-to-right then top-to-bottom.
     */
    int box() const { return (row() / BoxSize) * BoxCount + (col() / BoxSize); }

    /**
     * @brief The top-left corner of this position's box.
     *
     * The same box as box(), expressed as a coordinate pair instead of a single
     * index. Useful for iterating over the box's cells.
     *
     * @return A (row, column) pair for the box's top-left cell.
     */
    std::pair<row_t, col_t> boxStart() const {
        return std::make_pair(boxStartRow(), boxStartCol());
    }
};

/**
 * @brief Compares two coordinates for equality.
 *
 * Forwards to std::pair's own operator, so the comparison logic is written once by
 * the standard library rather than by hand here.
 *
 * @return True when both the row and the column match.
 */
inline bool operator==(const Position &a, const Position &b) { return a.coords() == b.coords(); }

/// @brief The negation of operator==.
inline bool operator!=(const Position &a, const Position &b) { return !(a == b); }

} // namespace sudoku

#endif // SUDOKU_CORE_POSITION_HPP
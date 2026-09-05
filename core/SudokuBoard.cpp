#include "core/SudokuBoard.hpp"
#include <algorithm>

namespace sudoku {

// ---- Construction ------------------------------------------------------

SudokuBoard::SudokuBoard() {
    // grid_ default-constructs: every Cell() is already EmptyValue/unlocked,
    // and an array of arrays owns no heap memory to set up or free.
}

// ---- Reading -----------------------------------------------------------

const Cell &SudokuBoard::at(const Position &p) const { return grid_[p.row()][p.col()]; }

int SudokuBoard::valueAt(const Position &p) const { return at(p).value(); }

bool SudokuBoard::isComplete() const {
    for (const auto &row : grid_) {
        for (const Cell &cell : row) {
            if (cell.isEmpty()) {
                return false;
            }
        }
    }
    return true;
}

int SudokuBoard::emptyCount() const {
    int count = 0;
    for (const auto &row : grid_) {
        for (const Cell &cell : row) {
            if (cell.isEmpty()) {
                ++count;
            }
        }
    }
    return count;
}

const std::array<Cell, Position::BoardSize> &SudokuBoard::getRow(const Position &pos) const {
    return grid_[pos.row()];
}

const std::array<Cell, Position::BoardSize> SudokuBoard::getColumn(const Position &pos) const {
    std::array<Cell, Position::BoardSize> column{};
    const int c = pos.col();
    for (int r = 0; r < Position::BoardSize; ++r) {
        column[r] = grid_[r][c];
    }
    return column;
}

const std::array<Cell, Position::BoardSize> SudokuBoard::getBox(const Position &p) const {
    std::array<Cell, Position::BoardSize> box{};
    const auto &[firstRow, firstCol] = p.boxStart();

    int i = 0;
    for (row_t row = firstRow; row < firstRow + Position::BoxSize; ++row) {
        for (col_t col = firstCol; col < firstCol + Position::BoxSize; ++col) {
            box[i++] = grid_[row][col];
        }
    }
    return box;
}

// ---- Rule checking -----------------------------------------------------
//
// Each of the three scans skips the cell at p itself. Without that, asking
// whether a cell may hold the digit it already holds would find that digit
// and report a conflict with itself.

bool SudokuBoard::rowContains(const Position &p, int value) const {
    const auto &[p_row, p_col] = p.coords();

    for (col_t col = 0; col < Position::BoardSize; ++col) {
        if (col == p_col) {
            continue;
        }
        if (grid_[p_row][col].value() == value) {
            return true;
        }
    }
    return false;
}

bool SudokuBoard::colContains(const Position &p, int value) const {
    const auto &[p_row, p_col] = p.coords();

    for (row_t row = 0; row < Position::BoardSize; ++row) {
        if (row == p_row) {
            continue;
        }
        if (grid_[row][p_col].value() == value) {
            return true;
        }
    }
    return false;
}

bool SudokuBoard::boxContains(const Position &p, int value) const {
    const auto &[firstRow, firstCol] = p.boxStart();

    for (row_t row = firstRow; row < firstRow + Position::BoxSize; ++row) {
        for (col_t col = firstCol; col < firstCol + Position::BoxSize; ++col) {
            if (row == p.row() && col == p.col()) {
                continue;
            }
            if (grid_[row][col].value() == value) {
                return true;
            }
        }
    }
    return false;
}

// ---- Puzzle setup ------------------------------------------------------

bool SudokuBoard::gridIsSelfConsistent(
    const std::array<std::array<Cell, Position::BoardSize>, Position::BoardSize> &grid) {
    // Static, so it cannot use the member scans above: this runs before any
    // board holds the data. It walks each clue and looks for the same digit
    // elsewhere in the clue's row, column, and box.
    for (row_t row = 0; row < Position::BoardSize; ++row) {
        for (col_t col = 0; col < Position::BoardSize; ++col) {
            const int value = grid[row][col].value();
            if (value == Cell::EmptyValue) {
                continue;
            }

            for (col_t other = 0; other < Position::BoardSize; ++other) {
                if (other != col && grid[row][other].value() == value) {
                    return false;
                }
            }
            for (row_t other = 0; other < Position::BoardSize; ++other) {
                if (other != row && grid[other][col].value() == value) {
                    return false;
                }
            }

            const row_t firstRow = (row / Position::BoxSize) * Position::BoxSize;
            const col_t firstCol = (col / Position::BoxSize) * Position::BoxSize;
            for (row_t r = firstRow; r < firstRow + Position::BoxSize; ++r) {
                for (col_t c = firstCol; c < firstCol + Position::BoxSize; ++c) {
                    if ((r != row || c != col) && grid[r][c].value() == value) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool SudokuBoard::canPlace(const Position &p, int value) const {
    if (!Cell::isValidValue(value)) {
        return false;
    }
    return !rowContains(p, value) && !colContains(p, value) && !boxContains(p, value);
}

// ---- Writing -----------------------------------------------------------

MoveStatus SudokuBoard::setValue(const Position &p, int value) {
    // Range first: every check below indexes into grid_, which is only safe
    // once the coordinate is known to be inside the board.
    if (value != Cell::EmptyValue && !Cell::isValidValue(value)) {
        return MoveStatus::InvalidDigit;
    }

    Cell &cell = grid_[p.row()][p.col()];
    if (cell.isLocked()) {
        return MoveStatus::CellIsGiven;
    }

    // Clearing a cell can never conflict, so the scan runs only for digits.
    if (value != Cell::EmptyValue && !canPlace(p, value)) {
        return MoveStatus::Conflict;
    }

    cell.setValue(value);
    return MoveStatus::Accepted;
}

void SudokuBoard::setGiven(const Position &p) {
    const auto &[row, col] = p.coords();
    Cell &cell = grid_[row][col];
    if (cell.value() != Cell::EmptyValue) {
        cell.setLocked(true);
    }
}

MoveStatus SudokuBoard::clearCell(const Position &p) {
    // validations are done in setValue(), so this is just a convenience wrapper.
    return setValue(p, Cell::EmptyValue);
}

PuzzleLoadStatus SudokuBoard::loadValidGrid(std::string_view puzzleFileName) {
    // Validate everything before touching grid_, so a rejected puzzle leaves
    // the current game untouched.
    // Build the replacement separately, then hand it over in one step.
    std::array<std::array<Cell, Position::BoardSize>, Position::BoardSize> loaded{};
    PuzzleLoadStatus loadStatus = Load::loadGrid(puzzleFileName, loaded);

    if (loadStatus != PuzzleLoadStatus::Accepted) {
        return loadStatus;
    }

    if (!gridIsSelfConsistent(loaded)) {
        return PuzzleLoadStatus::ContradictoryClues;
    }

    // move() is safe here because loaded is a local variable that will be
    // destroyed after the assignment. The vector's memory is transferred to
    // grid_ without copying the individual Cell objects.
    grid_ = std::move(loaded);
    return PuzzleLoadStatus::Accepted;
}

bool SudokuBoard::saveGrid(const std::string &puzzleFileName) const {
    return Save::saveGrid(puzzleFileName, grid_);
}

void SudokuBoard::reset() {
    for (auto &row : grid_) {
        for (Cell &cell : row) {
            cell = Cell();
        }
    }
}

void SudokuBoard::clearPlayerEntries() {
    for (auto &row : grid_) {
        for (Cell &cell : row) {
            if (cell.isEditable()) {
                cell.setValue(Cell::EmptyValue);
            }
        }
    }
}

// ---- Puzzle Generation -----------------------------------------------------------

} // namespace sudoku
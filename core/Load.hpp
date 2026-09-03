#ifndef SUDOKU_CORE_LOAD_HPP
#define SUDOKU_CORE_LOAD_HPP

#include "core/Cell.hpp"
#include "core/Position.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace sudoku {

/**
 * @brief The outcome of a puzzle load.
 */
enum class PuzzleLoadStatus {
    Accepted,          ///< The puzzle was loaded and is now on the board.
    FileNotFound,      ///< The file could not be opened.
    InvalidLineCount,  ///< The file does not have exactly 81 lines.
    WrongFileFormat,   ///< The file has wrong format (e.g. invalid syntax/trailing chars).
    InvalidCellState,  ///< A cell holds (something outside 1..9) OR (0 and not given)
    ContradictoryClues ///< Two clues already break the Sudoku rules.
};

class Load {
  public:
    // Pure function: Takes a filename, loads into grid, handles its own stream internally
    static PuzzleLoadStatus loadGrid(const std::string &puzzleFileName,
                                     std::vector<std::vector<Cell>> &grid) {
        std::ifstream file(puzzleFileName.data());
        if (!file.is_open()) {
            return PuzzleLoadStatus::FileNotFound;
        }
        std::string line;
        int cellCount = 0;

        while (std::getline(file, line)) {
            // Trim trailing carriage returns (Windows CRLF on Linux)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // Skip blank lines
            if (line.empty()) {
                continue;
            }

            // Check for too many lines (more than 81)
            if (cellCount >= Cell::TotalCells) {
                return PuzzleLoadStatus::InvalidLineCount;
            }

            std::istringstream lineStream(line);
            int val = 0;     // cell value (0 for empty, 1..9 for digits)
            int isGiven = 0; // Flag indicating if the cell is a given (1) or not (0)

            // Format check: must contain two integers (value, given_flag)
            if (!(lineStream >> val >> isGiven)) {
                return PuzzleLoadStatus::WrongFileFormat;
            }

            // Domain check: range validation & structural validity (e.g., empty cell cannot be
            // given)
            if (!Cell::isValidState(val, isGiven)) {
                return PuzzleLoadStatus::InvalidCellState;
            }

            // Utilize Position class to derive grid indices safely
            try {
                const row_t row = cellCount / Position::BoardSize;
                const col_t col = cellCount % Position::BoardSize;
                const Position pos(row, col); // just a check, will throw if out of bounds

                grid[row][col] = Cell(val, isGiven);
            } catch (const InvalidPosition &) {
                return PuzzleLoadStatus::InvalidLineCount;
            }

            cellCount++;
        }

        if (cellCount != Cell::TotalCells) {
            return PuzzleLoadStatus::InvalidLineCount;
        }

        return PuzzleLoadStatus::Accepted;
    }
};

} // namespace sudoku

#endif // SUDOKU_CORE_LOAD_HPP
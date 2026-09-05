#ifndef SUDOKU_CORE_SAVE_HPP
#define SUDOKU_CORE_SAVE_HPP

#include <array>
#include <fstream>
#include <string>

#include "core/Cell.hpp"
#include "core/Position.hpp"

namespace sudoku {

class Save {
  public:
    /**
     * @brief Writes a Sudoku grid to a text file.
     *
     * Each cell is written on its own line as "value isGiven", where isGiven
     * is 1 if the cell was part of the original puzzle and 0 otherwise. This
     * format matches what Load expects when reading a puzzle back in.
     *
     * If @p puzzleFileName does not already end in ".txt", the extension is
     * appended before opening the file. Any existing file at that path is
     * truncated and overwritten.
     *
     * @param puzzleFileName Name (or path) of the file to save to.
     * @param grid The grid of cells to write, indexed as grid[row][col].
     * @return true if the file was opened and the entire grid was written
     *         without error; false if the file could not be opened or a
     *         write failure occurred partway through (e.g., disk full).
     */
    static bool
    saveGrid(const std::string &puzzleFileName,
             const std::array<std::array<Cell, Position::BoardSize>, Position::BoardSize> &grid) {

        std::string formattedName = puzzleFileName;

        // Append .txt if the user didn't explicitly provide it
        if (formattedName.compare(formattedName.length() - 4, 4, ".txt") != 0) {
            formattedName += ".txt";
        }

        std::ofstream file(formattedName, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            return false;
        }
        for (const auto &row : grid) {
            for (const auto &cell : row) {
                // Output in the same "value isGiven" format read by Load
                file << cell.value() << ' ' << (cell.isLocked() ? 1 : 0) << '\n';

                // Catch mid-write failures (e.g., out of disk space)
                if (file.fail()) {
                    return false;
                }
            }
        }
        return true;
    }
};

} // namespace sudoku

#endif // #ifndef SUDOKU_CORE_SAVE_HPP
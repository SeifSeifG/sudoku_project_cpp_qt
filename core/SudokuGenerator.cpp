// SudokuGenerator.cpp
#include "core/SudokuGenerator.hpp"

#include <algorithm>
#include <array>
#include <numeric>
#include <stdexcept>

namespace sudoku {

std::optional<SudokuBoard> SudokuGenerator::generate(Difficulty difficulty) {
    // Generation is retried from scratch on failure rather than resumed,
    // since a stalled random start has no salvageable state to continue from.
    for (int attempt = 0; attempt < maxGenerationAttempts; ++attempt) {
        SudokuBoard board;
        std::mt19937 rng(std::random_device{}());

        placeInitialTransversal(board, rng);
        if (!growPuzzle(board, rng, static_cast<int>(difficulty))) {
            continue; // this random start didn't converge -- try a fresh one
        }

        // growPuzzle() only guarantees the *values* are correct; the clues
        // still need to be locked so the player can't edit them later.
        for (row_t row = 0; row < Position::BoardSize; ++row) {
            for (col_t col = 0; col < Position::BoardSize; ++col) {
                Position p(row, col);
                if (!board.at(p).isEmpty()) {
                    board.setGiven(p);
                }
            }
        }
        return board;
    }

    // Every attempt stalled -- report failure rather than handing back an
    // under-filled or ambiguous board.
    return std::nullopt;
}

void SudokuGenerator::placeInitialTransversal(SudokuBoard &board, std::mt19937 &rng) {
    std::array<col_t, Position::BoardSize> cols;
    std::iota(cols.begin(), cols.end(), 0);
    std::shuffle(cols.begin(), cols.end(), rng);

    std::array<int, Cell::MaxValue - Cell::MinValue + 1> values;
    std::iota(values.begin(), values.end(), Cell::MinValue);

    for (row_t row = 0; row < Position::BoardSize; ++row) {
        Position p(row, cols[row]);
        std::shuffle(values.begin(), values.end(), rng);
        for (int value : values) {
            if (board.setValue(p, value) == MoveStatus::Accepted) {
                break;
            }
        }
    }
}

bool SudokuGenerator::growPuzzle(SudokuBoard &board, std::mt19937 &rng, int targetGivens) {
    int placed = Position::BoardSize; // the initial transversal already filled 9 cells
    int stalled = 0;
    constexpr int maxStall = 20; // bail-out guard against an unlucky run that never converges

    while (stalled < maxStall) {
        // Only check uniqueness once the puzzle is otherwise done, see the
        // note on the loop body below for why it isn't checked every cell.
        if (placed >= targetGivens && solver_.countSolutions(board, 2) == 1) {
            return true; // target reached, and the puzzle has exactly one solution
        }

        Position p = randomEmptyPosition(board, rng);
        std::array<int, Cell::MaxValue - Cell::MinValue + 1> values;
        std::iota(values.begin(), values.end(), Cell::MinValue);
        std::shuffle(values.begin(), values.end(), rng);

        bool cellPlaced = false;
        for (int value : values) {
            if (board.setValue(p, value) != MoveStatus::Accepted) {
                continue;
                if (board.isComplete()) {
                    return true;
                }
            }
            // Existence, not uniqueness: this early, almost every legal
            // board still has many completions, so demanding a single
            // solution here would reject nearly every placement. Uniqueness
            // is only checked once targetGivens is reached, above.
            if (solver_.countSolutions(board, 1) > 0) {
                cellPlaced = true;
                break;
            }
            board.clearCell(p); // legal placement, but a dead end -- undo it
        }

        if (cellPlaced) {
            ++placed;
            stalled = 0; // progress was made, so reset the stall counter
        } else {
            ++stalled; // no value worked for this cell -- count it as a failed attempt
        }
    }
    return false; // hit maxStall without reaching a unique puzzle at the target count
}

Position SudokuGenerator::randomEmptyPosition(const SudokuBoard &board, std::mt19937 &rng) const {
    std::uniform_int_distribution<int> dist(0, Position::BoardSize - 1);
    Position p(dist(rng), dist(rng));
    while (!board.at(p).isEmpty()) {
        p = Position(dist(rng), dist(rng));
    }
    return p;
}

} // namespace sudoku
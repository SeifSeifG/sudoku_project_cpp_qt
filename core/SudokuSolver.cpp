#include "core/SudokuSolver.hpp"

namespace sudoku {

// ---- Private helpers ---------------------------------------------------

CandidateSet SudokuSolver::countCandidates(SudokuBoard &board, const Position &p) {
    CandidateSet candidates;
    for (int digit = Cell::MinValue; digit <= Cell::MaxValue; ++digit) {
        if (board.setValue(p, digit) == MoveStatus::Accepted) {
            candidates.digits[candidates.count++] = digit;
            board.clearCell(p);
        }
    }
    return candidates;
}

std::optional<ConstrainedCell> SudokuSolver::findMostConstrainedCell(SudokuBoard &board) {
    std::optional<ConstrainedCell> best;

    for (row_t row = 0; row < Position::BoardSize; ++row) {
        for (col_t col = 0; col < Position::BoardSize; ++col) {
            const Position p(row, col);
            if (!board.at(p).isEmpty()) {
                continue;
            }

            CandidateSet candidates = countCandidates(board, p);
            if (candidates.count == 0) {
                return ConstrainedCell{p, candidates}; // dead cell -- report immediately
            }
            if (!best || candidates.count < best->candidates.count) {
                best = ConstrainedCell{p, candidates};
            }
        }
    }
    return best;
}

bool SudokuSolver::fillFrom(SudokuBoard &board) {
    const std::optional<ConstrainedCell> next = findMostConstrainedCell(board);
    if (!next) {
        return true;
    }

    // Iterating next->candidates.digits instead of Cell::MinValue..MaxValue
    // avoids re-probing digits findMostConstrainedCell already ruled out for
    // this exact cell, on this exact board state, one call ago.
    for (int i = 0; i < next->candidates.count; ++i) {
        const int digit = next->candidates.digits[i];
        // Expected to always succeed -- kept as a guard against the board
        // having changed underneath us, not as a normal filtering step.
        if (board.setValue(next->position, digit) != MoveStatus::Accepted) {
            continue;
        }
        if (fillFrom(board)) {
            return true;
        }
        board.clearCell(next->position);
    }
    return false;
}

int SudokuSolver::countFrom(SudokuBoard &board, int limit) {
    const std::optional<ConstrainedCell> next = findMostConstrainedCell(board);
    if (!next) {
        return 1; // a complete board is one solution
    }

    int found = 0;
    for (int i = 0; i < next->candidates.count; ++i) {
        const int digit = next->candidates.digits[i];
        if (board.setValue(next->position, digit) != MoveStatus::Accepted) {
            continue;
        }

        found += countFrom(board, limit - found);
        board.clearCell(next->position);

        // Unlike fillFrom(), this does not stop at the first success -- it
        // keeps exploring until the caller has seen enough. The limit is what
        // keeps that affordable: asking "is this puzzle unique?" costs a search
        // for two solutions, not a walk of the entire space.
        if (found >= limit) {
            break;
        }
    }
    return found;
}

// ---- Public interface --------------------------------------------------

bool SudokuSolver::solve(SudokuBoard &board) const {
    // The search runs on a copy. A failed solve would otherwise leave the
    // player's board holding whichever partial guesses the search wrote last.
    SudokuBoard working = board;
    if (!fillFrom(working)) {
        return false;
    }
    board = std::move(working);
    return true;
}

int SudokuSolver::countSolutions(const SudokuBoard &board, int limit) const {
    if (limit <= 0) {
        return 0;
    }
    SudokuBoard working = board;
    return countFrom(working, limit);
}

} // namespace sudoku
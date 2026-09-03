#include "app/SudokuGame.hpp"

namespace sudoku {

// board_ and solver_ default-construct on their own; only the observer needs naming.
SudokuGame::SudokuGame(GameObserver *observer) : observer_(observer) {}

void SudokuGame::setObserver(GameObserver *observer) { observer_ = observer; }

const SudokuBoard &SudokuGame::board() const { return board_; }

PuzzleLoadStatus SudokuGame::loadPuzzle(std::string_view puzzleFileName) {
    const PuzzleLoadStatus status = board_.loadValidGrid(puzzleFileName);

    // The board only mutates on Accepted, so a rejected puzzle gets the outcome
    // callback and nothing else.
    if (status == PuzzleLoadStatus::Accepted && observer_ != nullptr) {
        observer_->onBoardChanged(board_);
    }
    // The outcome callback is always sent, even when the board was not modified.
    if (observer_ != nullptr) {
        observer_->onPuzzleLoaded(status);
    }

    return status;
}

bool SudokuGame::savePuzzle(const std::string &puzzleFileName) const {
    return board_.saveGrid(puzzleFileName);
}

MoveStatus SudokuGame::makeMove(const Position &p, int value) {
    const MoveStatus status = board_.setValue(p, value);
    if (observer_ != nullptr) {
        if (status != MoveStatus::Accepted) {
            observer_->onMoveRejected(status);

            // called here to re draw the puzzle only (not actual change)
            observer_->onBoardChanged(board_);
        } else {
            observer_->onBoardChanged(board_);

            // Checked after the redraw so the win message follows the finished board.
            // Erasing a digit also lands here, which is why the completion test is on
            // the board's state and not on the fact that a move succeeded.
            if (board_.isComplete()) {
                observer_->onPuzzleCompleted();
            }
        }
    }
    return status;
}

bool SudokuGame::solve() {
    const bool solved = solver_.solve(board_);

    if (observer_ != nullptr) {
        if (solved) {
            observer_->onBoardChanged(board_);
            observer_->onSolveSucceeded();
        } else {
            // The solver searched a copy, so board_ still holds the player's work.
            observer_->onSolveFailed();
        }
    }

    return solved;
}

bool SudokuGame::generatePuzzle(Difficulty difficulty) {
    std::optional<SudokuBoard> generated = generator_.generate(difficulty);
    if (!generated) {
        observer_->onPuzzleGenerationFailed();
        return false;
    }

    board_ = std::move(generated.value());

    if (observer_ != nullptr) {
        observer_->onPuzzleGenerationSucceeded();
        observer_->onBoardChanged(board_);
    }

    return true;
}

void SudokuGame::refresh() const {
    if (observer_ != nullptr) {
        observer_->onBoardChanged(board_);
    }
}

} // namespace sudoku

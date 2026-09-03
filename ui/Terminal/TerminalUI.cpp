#include "TerminalUI.hpp"

#include <iostream>
#include <limits>

namespace sudoku {

/// The rule the brief draws above, between, and below the bands of boxes.
const char *const BoardRule = "-------------------------------------";

TerminalUI::TerminalUI(SudokuGame &game)
    : game_(game), running_(true), needsRedraw_(true), gameFinished_(false) {}

// ---- Rendering ---------------------------------------------------------

void TerminalUI::printBoard(const SudokuBoard &board) const {
    std::cout << BoardRule << "\n";
    for (row_t row = 0; row < Position::BoardSize; ++row) {
        for (col_t col = 0; col < Position::BoardSize; ++col) {
            const int value = board.valueAt(Position(row, col));
            if (value == Cell::EmptyValue) {
                std::cout << '.';
            } else {
                std::cout << value;
            }

            // A space between cells, a bar between boxes, nothing after the last.
            if (col == Position::BoardSize - 1) {
                std::cout << '\n';
            } else if (col % Position::BoxSize == Position::BoxSize - 1) {
                std::cout << " | ";
            } else {
                std::cout << ' ';
            }
        }
        if (row % Position::BoxSize == Position::BoxSize - 1) {
            std::cout << BoardRule << "\n";
        }
    }
}

void TerminalUI::printMenu() const {
    std::cout << "\n1) Enter a move\n"
              << "2) Solve automatically\n"
              << "3) Load puzzle from file\n"
              << "4) Save current puzzle to file\n"
              << "5) Generate new puzzle\n"
              << "6) Exit\n"
              << "Choice: ";
}

// ---- Input -------------------------------------------------------------

void TerminalUI::recoverFromBadInput() const {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool TerminalUI::readInt(const char *prompt, int &out) const {
    std::cout << prompt;
    if (std::cin >> out) {
        return true;
    }
    // EOF is the player closing the stream, not a typo: leave the failed state
    // alone so run() can see it and exit.
    if (!std::cin.eof()) {
        recoverFromBadInput();
    }
    return false;
}

bool TerminalUI::readString(const char *prompt, std::string &out) const {
    std::cout << prompt;
    if (std::cin >> out) {
        return true;
    }
    if (!std::cin.eof()) {
        recoverFromBadInput();
    }
    return false;
}

bool TerminalUI::readMove(Position &p, int &value) const {
    int row = 0;
    int col = 0;
    std::cout << "Enter row (1-9), column (1-9), and value (1-9): ";
    if (!(std::cin >> row >> col >> value)) {
        if (!std::cin.eof()) {
            recoverFromBadInput();
            std::cout << "Please enter three numbers.\n";
        }
        return false;
    }

    // The one place 1-based input becomes a 0-based coordinate.
    const row_t zeroRow = row - 1;
    const col_t zeroCol = col - 1;

    // Screened here because Position's constructor throws, and a mistyped row is
    // ordinary input rather than an exceptional condition.
    if (zeroRow < 0 || zeroRow >= Position::BoardSize) {
        std::cout << "Row must be between 1 and 9.\n";
        return false;
    }
    if (zeroCol < 0 || zeroCol >= Position::BoardSize) {
        std::cout << "Column must be between 1 and 9.\n";
        return false;
    }

    p = Position(zeroRow, zeroCol);
    return true;
}

// ---- Menu actions ------------------------------------------------------

void TerminalUI::handleEnterMove() {
    Position p(0, 0);
    int value = 0;
    if (!readMove(p, value)) {
        return;
    }

    // The rejection message arrives through onMoveRejected(); only the success
    // line comes from the return value, because there is no onMoveAccepted().
    if (game_.makeMove(p, value) == MoveStatus::Accepted) {
        if (gameFinished_ != true) {
            std::cout << describe(MoveStatus::Accepted) << "\n";
        }
    }
}

void TerminalUI::handleSolve() { game_.solve(); }

void TerminalUI::handleLoad() {
    std::string fileName;
    if (!readString("Enter puzzle file name to load: ", fileName)) {
        return;
    }

    const PuzzleLoadStatus status = game_.loadPuzzle(fileName);
    if (status == PuzzleLoadStatus::Accepted) {
        std::cout << "Puzzle loaded successfully.\n";
    } else {
        std::cout << "Failed to load puzzle: " << describe(status) << "\n";
    }
}

void TerminalUI::handleSave() {
    std::string fileName;
    if (!readString("Enter file name to save puzzle: ", fileName)) {
        return;
    }

    if (game_.savePuzzle(fileName)) {
        std::cout << "Puzzle saved successfully to " << fileName << ".\n";
    } else {
        std::cout << "Error: Failed to save puzzle to " << fileName << ".\n";
    }
}

void TerminalUI::handleGenerate() {
    std::cout << "1) Easy\n"
              << "2) Medium\n"
              << "3) Hard\n"
              << "4) Expert\n";

    int choice = 0;
    if (!readInt("Choose a difficulty (1-4): ", choice)) {
        return;
    }

    Difficulty difficulty = Difficulty::Easy;
    switch (choice) {
    case 1:
        difficulty = Difficulty::Easy;
        break;
    case 2:
        difficulty = Difficulty::Medium;
        break;
    case 3:
        difficulty = Difficulty::Hard;
        break;
    case 4:
        difficulty = Difficulty::Expert;
        break;
    default:
        std::cout << "Please enter a number between 1 and 4.\n";
        return;
    }

    // Feedback and the redraw both arrive through onPuzzleGenerationSucceeded()/
    // onPuzzleGenerationFailed() -- nothing further to do with the return value
    // here, same pattern as handleSolve().
    game_.generatePuzzle(difficulty);
}

void TerminalUI::handleNotImplemented() const {
    std::cout << "That option is not available yet.\n";
}

// ---- Enum to English ---------------------------------------------------

const std::string TerminalUI::describe(MoveStatus status) {
    switch (status) {
    case MoveStatus::Accepted:
        return "Move accepted!";
    case MoveStatus::InvalidDigit:
        return "Value must be between 1 and 9, or 0 to erase.";
    case MoveStatus::CellIsGiven:
        return "That cell is part of the original puzzle and cannot be changed.";
    case MoveStatus::Conflict:
        return "That digit already appears in the same row, column, or box.";
    }
    return "The move could not be made.";
}

const std::string TerminalUI::describe(PuzzleLoadStatus status) {
    switch (status) {
    case PuzzleLoadStatus::Accepted:
        return "Puzzle loaded.";
    case PuzzleLoadStatus::FileNotFound:
        return "That puzzle file was not found.";
    case PuzzleLoadStatus::InvalidLineCount:
        return "That puzzle file has an invalid number of lines.";
    case PuzzleLoadStatus::WrongFileFormat:
        return "That puzzle contains a wrong format (e.g. invalid syntax).";
    case PuzzleLoadStatus::InvalidCellState:
        return "That puzzle has invalid cell values";
    case PuzzleLoadStatus::ContradictoryClues:
        return "That puzzle's starting clues already break the Sudoku rules.";
    }
    return "The puzzle could not be loaded.";
}

// ---- The loop ----------------------------------------------------------

void TerminalUI::run() {
    game_.refresh(); // ask for the opening draw rather than reaching for the board

    while (running_) {
        if (needsRedraw_) {
            printBoard(game_.board());
            needsRedraw_ = false;
        }

        printMenu();
        int choice = 0;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                std::cout << "\n";
                return; // stream closed: leave rather than spin on a dead cin
            }
            recoverFromBadInput();
            std::cout << "Please enter a number between 1 and 5.\n";
            continue;
        }

        // to handle entering multiple choices in the same line
        recoverFromBadInput();

        switch (choice) {
        case 1:
            handleEnterMove();
            break;
        case 2:
            handleSolve();
            break;
        case 3:
            handleLoad();
            break;
        case 4:
            handleSave();
            break;
        case 5:
            handleGenerate();
            break;
        case 6:
            running_ = false;
            break;
        default:
            std::cout << "Please enter a number between 1 and 5.\n";
            break;
        }
    }
}

// ---- GameObserver ------------------------------------------------------

void TerminalUI::onBoardChanged(const SudokuBoard &) {
    // Marked, not drawn. The brief prints "Move accepted!" before the new board,
    // and this callback fires inside makeMove(), before that line exists. Deferring
    // the paint is also how a GUI behaves: invalidate now, repaint later.
    needsRedraw_ = true;
}

void TerminalUI::onMoveRejected(MoveStatus reason) { std::cout << describe(reason) << "\n"; }

void TerminalUI::onPuzzleLoaded(PuzzleLoadStatus status) {
    if (status != PuzzleLoadStatus::Accepted) {
        std::cout << describe(status) << "\n";
        gameFinished_ = false;
    }
}

void TerminalUI::onPuzzleSave() { std::cout << "Puzzle saved.\n"; }

void TerminalUI::onSolveSucceeded() {
    std::cout << "Puzzle solved!\n";
    gameFinished_ = true;
}

void TerminalUI::onSolveFailed() { std::cout << "This puzzle has no solution.\n"; }

void TerminalUI::onPuzzleCompleted() {
    std::cout << "Congratulations, you solved it!\n";
    gameFinished_ = true;
}

void TerminalUI::onPuzzleGenerationSucceeded() {
    std::cout << "Puzzle Generated Successfully!\n";
    gameFinished_ = false;
};

void TerminalUI::onPuzzleGenerationFailed() {
    std::cout << "Puzzle Generation Failed! Try again later.!\n";
};

} // namespace sudoku
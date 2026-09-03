#include "MainWindow.hpp"

#include <algorithm>

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>

#include "QCell.hpp"
#include "core/Cell.hpp"
#include "core/SudokuBoard.hpp"
#include "core/SudokuGenerator.hpp"

namespace sudoku {

namespace {
const char *kDefaultStatusMessage =
    "Click a cell, then type a number (1-9). Press Delete to clear.";
const char *kEnterMoveHint =
    "Click a cell, then press a digit 1-9 to place it, or Delete/Backspace to clear it.";
} // namespace

// -----------------------------------------------------------------------------
// Constructor: Initializes the main window, sets up game references, and nulls out pointers
// -----------------------------------------------------------------------------
MainWindow::MainWindow(SudokuGame &game, QWidget *parent)
    : QMainWindow(parent), game_(game), selectedRow_(-1), selectedCol_(-1), hasSelection_(false),
      rowValueLabel_(nullptr), colValueLabel_(nullptr), valueValueLabel_(nullptr),
      typeValueLabel_(nullptr), statusLabel_(nullptr), loadAction_(nullptr), saveAction_(nullptr),
      exitAction_(nullptr), enterMoveAction_(nullptr), solveAction_(nullptr),
      generateAction_(nullptr), aboutAction_(nullptr) {

    setWindowTitle("Sudoku");

    // Call helper methods to build the UI components
    setupMenuBar();
    setupCentralWidget();
    setupStatusBar();

    // Prevent the window from being resized by the user; keep it at its ideal size
    // setFixedSize(sizeHint());
    resize(sizeHint());

    // Initialize the side panel to show no selection by default
    updateSelectionPanel();
}

// ─────────────────────────────────────────────────────────────────────────
// Window construction
// ─────────────────────────────────────────────────────────────────────────

void MainWindow::setupMenuBar() { // ---- File Menu ----
    // The '&' creates a keyboard shortcut (Alt+F opens this menu)
    QMenu *fileMenu = menuBar()->addMenu("&File");

    // 1) Action: Load Puzzle
    loadAction_ = fileMenu->addAction("Load Puzzle from File...");
    // Connects the action's 'triggered' signal to the 'onLoadPuzzle' method (Slot)
    connect(loadAction_, &QAction::triggered, this, &MainWindow::onLoadPuzzle);

    // 2) Action: Save Puzzle
    saveAction_ = fileMenu->addAction("Save Current Puzzle...");
    connect(saveAction_, &QAction::triggered, this, &MainWindow::onSavePuzzle);

    // Add a thin horizontal separator line between the previous options and Exit
    fileMenu->addSeparator();

    // 3) Action: Exit
    exitAction_ = fileMenu->addAction("Exit");
    // Connects directly to the application's quit method
    connect(exitAction_, &QAction::triggered, qApp, &QApplication::quit);

    // ---- Game Menu ----
    QMenu *gameMenu = menuBar()->addMenu("&Game");

    // 1) Action: Enter a Move
    enterMoveAction_ = gameMenu->addAction("Enter a Move");
    // Uses a lambda function (inline function) to show a hint message on the status bar
    connect(enterMoveAction_, &QAction::triggered, this,
            [this]() { showStatusMessage(kEnterMoveHint, false); });

    // 2) Action: Solve Automatically
    solveAction_ = gameMenu->addAction("Solve Automatically");
    connect(solveAction_, &QAction::triggered, this, &MainWindow::onSolveAutomatically);

    // 3) Action: Generate New Puzzle
    generateAction_ = gameMenu->addAction("Generate New Puzzle");
    connect(generateAction_, &QAction::triggered, this, &MainWindow::onGenerateNewPuzzle);

    // ---- Help Menu ----
    QMenu *helpMenu = menuBar()->addMenu("&Help");

    // Action: About
    aboutAction_ = helpMenu->addAction("About...");
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupCentralWidget() {
    auto *central = new QWidget(this);
    // Use a Horizontal layout to place the Board on the Left and Panels on the Right
    auto *mainLayout = new QHBoxLayout(central);

    // ==========================================
    // LEFT SIDE: The 9x9 Sudoku Board
    // ==========================================
    auto *boardGroup = new QGroupBox("Puzzle Board", central);
    // Use a Grid layout to arrange cells in rows and columns
    auto *boardLayout = new QGridLayout(boardGroup);
    boardLayout->setSpacing(0); // Remove space between cells so it looks like a solid grid
    boardLayout->setContentsMargins(4, 4, 4, 4);

    // Loop to create the 81 (9x9) individual cells
    for (int r = 0; r < Position::BoardSize; ++r) {
        for (int c = 0; c < Position::BoardSize; ++c) {
            // Instantiate a custom QCell widget
            auto *cell = new QCell(r, c, boardGroup);
            // Listen for when the user clicks this cell
            connect(cell, &QCell::clicked, this, &MainWindow::onCellClicked);
            // Add it to the grid at the specific row and column
            boardLayout->addWidget(cell, r, c);
            // Store a pointer to the cell in our 2D array for later updates
            cells_[r][c] = cell;
        }
    }
    mainLayout->addWidget(boardGroup);

    // ==========================================
    // RIGHT SIDE: Selection Info, Legend, Actions
    // ==========================================
    // Vertical layout to stack these panels on top of each other
    auto *rightLayout = new QVBoxLayout();

    // 1) Selection Info Group (Shows details of the currently clicked cell)
    auto *selectionGroup = new QGroupBox("Selection", central);
    // QFormLayout is perfect for "Label: Value" pairs
    auto *selectionForm = new QFormLayout(selectionGroup);
    rowValueLabel_ = new QLabel("-", selectionGroup);
    colValueLabel_ = new QLabel("-", selectionGroup);
    valueValueLabel_ = new QLabel("-", selectionGroup);
    typeValueLabel_ = new QLabel("-", selectionGroup);

    selectionForm->addRow("Row", rowValueLabel_);
    selectionForm->addRow("Column", colValueLabel_);
    selectionForm->addRow("Value", valueValueLabel_);
    selectionForm->addRow("Type", typeValueLabel_);
    rightLayout->addWidget(selectionGroup);

    // 2) Legend Group (Explains what colors mean)
    auto *legendGroup = new QGroupBox("Legend", central);
    auto *legendLayout = new QVBoxLayout(legendGroup);

    // Define a local helper (Lambda) to easily create rows in the legend without repeating code
    auto addLegendRow = [legendLayout, legendGroup](const QString &bgColor,
                                                    const QString &textColor, bool bold,
                                                    const QString &description) {
        auto *rowWidget = new QWidget(legendGroup);
        auto *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        // Create a small colored square (swatch) using a QLabel with CSS styling
        auto *swatch = new QLabel("5", rowWidget);
        swatch->setFixedSize(24, 24);
        swatch->setAlignment(Qt::AlignCenter);
        QString style = QString("background-color: %1; color: %2; border: 1px solid #888888;")
                            .arg(bgColor, textColor);
        if (bold) {
            style += " font-weight: bold;";
        }
        swatch->setStyleSheet(style);

        // Label explaining what the swatch means
        auto *description_label = new QLabel(description, rowWidget);

        rowLayout->addWidget(swatch);
        rowLayout->addWidget(description_label);
        rowLayout->addStretch(); // Pushes everything to the left

        legendLayout->addWidget(rowWidget);
    };

    // Use the lambda to create the four legend entries
    addLegendRow(kDefaultBg.name(), kGivenText.name(), true, "Given (read-only)");
    addLegendRow(kDefaultBg.name(), kPlayerText.name(), false, "Player entry");
    addLegendRow(kHighlightedBg.name(), kGivenText.name(), false,
                 "Highlighted (same row / col / box)");
    addLegendRow(kSelectedBg.name(), kDefaultBg.name(), false, "Selected cell");

    rightLayout->addWidget(legendGroup);

    // 3) Quick Actions Group (Buttons for auto-solve and generation)
    auto *actionsGroup = new QGroupBox("Quick Actions", central);
    auto *actionsLayout = new QVBoxLayout(actionsGroup);

    // Solve Button
    auto *solveButton = new QPushButton("Solve Automatically", actionsGroup);
    solveButton->setFocusPolicy(Qt::NoFocus); // for arrow input and backspace
    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveAutomatically);
    actionsLayout->addWidget(solveButton);

    // Generate Button
    auto *generateButton = new QPushButton("Generate New Puzzle", actionsGroup);
    generateButton->setFocusPolicy(Qt::NoFocus); // for arrow input and backspace
    connect(generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateNewPuzzle);
    actionsLayout->addWidget(generateButton);

    rightLayout->addWidget(actionsGroup);

    // 4) Key Shortcuts Group
    auto *shortcutsGroup = new QGroupBox("Key Shortcuts", central);

    auto *shortcutsLayout = new QVBoxLayout(shortcutsGroup);
    shortcutsLayout->setSpacing(4);

    // Local helper — creates one key-badge + description row
    auto addShortcutRow = [&](const QStringList &keys, const QString &description) {
        auto *row = new QWidget(shortcutsGroup);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 2, 0, 2);
        rowLayout->setSpacing(6);

        // Key badges (styled like physical keyboard keys)
        for (const QString &key : keys) {
            auto *badge = new QLabel(key, row);
            badge->setAlignment(Qt::AlignCenter);
            badge->setFixedHeight(22);
            badge->setMinimumWidth(24);
            badge->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
            badge->setStyleSheet("QLabel {"
                                 "  font-family: 'Courier New', monospace;"
                                 "  font-size: 11px;"
                                 "  color: #222222;"
                                 "  background-color: #ffffff;"
                                 "  border: 1px solid #999999;"
                                 "  border-bottom: 2px solid #777777;"
                                 "  border-radius: 3px;"
                                 "  padding: 0px 5px;"
                                 "}");
            rowLayout->addWidget(badge);
        }

        // Thin vertical separator between badges and text
        auto *sep = new QFrame(row);
        sep->setFrameShape(QFrame::VLine);
        sep->setFrameShadow(QFrame::Sunken);
        rowLayout->addWidget(sep);

        auto *desc = new QLabel(description, row);
        rowLayout->addWidget(desc);

        // Horizontal rule below each row
        auto *hr = new QFrame(shortcutsGroup);
        hr->setFrameShape(QFrame::HLine);
        hr->setFrameShadow(QFrame::Plain);
        hr->setStyleSheet("color: #e0e0e0;");

        shortcutsLayout->addWidget(row);
        shortcutsLayout->addWidget(hr);
    };

    addShortcutRow({"Del", "⌫"}, "Clear selected cell");
    addShortcutRow({"Esc"}, "Clear all highlights");
    addShortcutRow({"↑", "↓", "←", "→"}, "Navigate board");

    rightLayout->addWidget(shortcutsGroup);

    // Add stretch at the bottom so the right panels align at the top instead of spreading out
    rightLayout->addStretch();

    // Add the whole right column to the main layout
    mainLayout->addLayout(rightLayout);

    // Tell the main window to use this assembled widget as its core content
    setCentralWidget(central);
}

// Sets up the status bar to display helpful hints and game updates
void MainWindow::setupStatusBar() {
    // Create a new QLabel (text widget) initialized with the default hint message.
    // 'this' makes the MainWindow the parent, ensuring proper memory cleanup.
    statusLabel_ = new QLabel(kDefaultStatusMessage, this);

    // statusBar() gets the QMainWindow's built-in status bar.
    // addWidget() inserts our label into it.
    // The '1' is the stretch factor, which tells the label to expand and fill
    // all available horizontal space in the bar.
    statusBar()->addWidget(statusLabel_, 1);
}

// ─────────────────────────────────────────────────────────────────────────
// Selection bookkeeping
// ─────────────────────────────────────────────────────────────────────────

void MainWindow::selectCell(int row, int col) {
    selectedRow_ = row;
    selectedCol_ = col;
    hasSelection_ = true;
    updateHighlighting();
    updateSelectionPanel();
}

void MainWindow::clearCellSelection() {
    if (!hasSelection_) {
        return; // Nothing to clear
    }

    hasSelection_ = false;
    selectedRow_ = -1;
    selectedCol_ = -1;

    // Remove the visual highlights
    updateHighlighting();
    // Reset the selection info panel on the right
    updateSelectionPanel();
}

void MainWindow::updateHighlighting() {
    Position selected(hasSelection_ ? selectedRow_ : 0, hasSelection_ ? selectedCol_ : 0);

    for (int r = 0; r < Position::BoardSize; ++r) {
        for (int c = 0; c < Position::BoardSize; ++c) {
            const bool isSelected = (hasSelection_) && (r == selectedRow_) && (c == selectedCol_);
            bool isHighlighted = false;
            if (hasSelection_ && !isSelected) {
                Position here(r, c);
                isHighlighted =
                    (r == selectedRow_) || (c == selectedCol_) || (here.box() == selected.box());
            }
            cells_[r][c]->setSelected(isSelected);
            cells_[r][c]->setHighlighted(isHighlighted);
        }
    }
}

void MainWindow::updateSelectionPanel() {
    if (!hasSelection_) {
        rowValueLabel_->setText("-");
        colValueLabel_->setText("-");
        valueValueLabel_->setText("-");
        typeValueLabel_->setText("-");
        return;
    }

    // Shown 1-based for readability, exactly as TerminalUI converts for the
    // player; the Position built below stays 0-based like everywhere else.
    rowValueLabel_->setText(QString::number(selectedRow_ + 1));
    colValueLabel_->setText(QString::number(selectedCol_ + 1));

    const Cell &cell = game_.board().at(Position(selectedRow_, selectedCol_));

    valueValueLabel_->setText(cell.isEmpty() ? QString::fromUtf8("--")
                                             : QString::number(cell.value()));
    typeValueLabel_->setText(cell.isLocked() ? "Given" : "Editable");
}

void MainWindow::showStatusMessage(const QString &message, bool isError) {
    statusLabel_->setText(message);
    statusLabel_->setStyleSheet(isError ? "color: #cc0000;" : "color: #ffffff;");
}

// ─────────────────────────────────────────────────────────────────────────
// Enum to English
// ─────────────────────────────────────────────────────────────────────────

QString MainWindow::describe(MoveStatus status) {
    switch (status) {
    case MoveStatus::Accepted:
        return "Move accepted.";
    case MoveStatus::InvalidDigit:
        return "Invalid digit: enter a value from 1 to 9.";
    case MoveStatus::CellIsGiven:
        return "That cell is part of the original puzzle and cannot be changed.";
    case MoveStatus::Conflict:
        return "That value already appears in this row, column, or box.";
    }
    return "Move rejected.";
}

QString MainWindow::describe(PuzzleLoadStatus status) {
    const int num = Position::BoardSize * Position::BoardSize;
    switch (status) {
    case PuzzleLoadStatus::Accepted:
        return "Puzzle loaded.";
    case PuzzleLoadStatus::FileNotFound:
        return "That puzzle file could not be found.";
    case PuzzleLoadStatus::InvalidLineCount:
        return "That puzzle file does not contain exactly " + QString(std::to_string(num).c_str()) +
               " lines";
    case PuzzleLoadStatus::WrongFileFormat:
        return "That puzzle file is not in a recognized format.";
    case PuzzleLoadStatus::InvalidCellState:
        return "That puzzle file contains an invalid cell value.";
    case PuzzleLoadStatus::ContradictoryClues:
        return "That puzzle's clues contradict each other.";
    default:
        return "Puzzle rejected.";
    }
}

// ─────────────────────────────────────────────────────────────────────────
// Keyboard interaction
// ─────────────────────────────────────────────────────────────────────────

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (!hasSelection_) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    const int key = event->key();

    if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        const int value = key - Qt::Key_0;
        game_.makeMove(Position(selectedRow_, selectedCol_), value);
        return;
    }

    if (key == Qt::Key_Delete || key == Qt::Key_Backspace) {
        game_.makeMove(Position(selectedRow_, selectedCol_), Cell::EmptyValue);
        return;
    }

    if (key == Qt::Key_Escape) {
        clearCellSelection(); // Call your clear slot here (or onClear() if you named it that)
        return;
    }

    int newRow = selectedRow_;
    int newCol = selectedCol_;
    switch (key) {
    case Qt::Key_Up:
        newRow = std::max(0, selectedRow_ - 1);
        break;
    case Qt::Key_Down:
        newRow = std::min(Position::BoardSize - 1, selectedRow_ + 1);
        break;
    case Qt::Key_Left:
        newCol = std::max(0, selectedCol_ - 1);
        break;
    case Qt::Key_Right:
        newCol = std::min(Position::BoardSize - 1, selectedCol_ + 1);
        break;
    default:
        QMainWindow::keyPressEvent(event);
        return;
    }

    selectCell(newRow, newCol);
}

// ─────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────

void MainWindow::onCellClicked(int row, int col) { selectCell(row, col); }

void MainWindow::onLoadPuzzle() {
    const QString fileName = QFileDialog::getOpenFileName(this, "Load Puzzle from File", QString(),
                                                          "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    game_.loadPuzzle(fileName.toStdString());
}

void MainWindow::onSavePuzzle() {
    const QString fileName = QFileDialog::getSaveFileName(this, "Save Current Puzzle", QString(),
                                                          "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }

    // SudokuGame::savePuzzle() reports success only through its return value --
    // unlike loadPuzzle()/makeMove()/solve(), its doc comment promises no
    // observer callback -- so this slot decides what to show itself instead of
    // waiting on one. onPuzzleSave() is still invoked on success so GameObserver
    // is honored uniformly, exactly as it would be if the game had called it.
    if (game_.savePuzzle(fileName.toStdString())) {
        onPuzzleSave();
    } else {
        showStatusMessage("The puzzle could not be saved to that file.", true);
    }
}

void MainWindow::onSolveAutomatically() { game_.solve(); }

void MainWindow::onGenerateNewPuzzle() {
    const QStringList difficulties = {"Easy", "Medium", "Hard", "Expert"};
    bool ok = false;
    const QString choice = QInputDialog::getItem(this, "Generate New Puzzle",
                                                 "Difficulty:", difficulties, 0, false, &ok);
    if (!ok) {
        return;
    }

    Difficulty difficulty = Difficulty::Medium;
    if (choice == "Easy") {
        difficulty = Difficulty::Easy;
    } else if (choice == "Medium") {
        difficulty = Difficulty::Medium;
    } else if (choice == "Hard") {
        difficulty = Difficulty::Hard;
    } else if (choice == "Expert") {
        difficulty = Difficulty::Expert;
    }

    game_.generatePuzzle(difficulty);
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About Sudoku",
                       "A Qt Widgets Sudoku frontend.\n\n"
                       "Board logic, solving, and generation are provided by the "
                       "existing SudokuBoard, SudokuSolver, and SudokuGenerator core classes.");
}

// ─────────────────────────────────────────────────────────────────────────
// GameObserver
// ─────────────────────────────────────────────────────────────────────────

void MainWindow::onBoardChanged(const SudokuBoard &board) {
    for (int r = 0; r < Position::BoardSize; ++r) {
        for (int c = 0; c < Position::BoardSize; ++c) {
            const Cell &cell = board.at(Position(r, c));
            cells_[r][c]->setValue(cell.value(), cell.isLocked());
        }
    }
    updateHighlighting();
    updateSelectionPanel();
}

void MainWindow::onMoveRejected(MoveStatus reason) { showStatusMessage(describe(reason), true); }

void MainWindow::onPuzzleLoaded(PuzzleLoadStatus status) {
    if (status == PuzzleLoadStatus::Accepted) {
        hasSelection_ = false;
        updateHighlighting();
        updateSelectionPanel();
        showStatusMessage("Puzzle loaded successfully.", false);
    } else {
        showStatusMessage(describe(status), true);
    }
}

void MainWindow::onPuzzleSave() { showStatusMessage("Puzzle saved successfully.", false); }

void MainWindow::onSolveSucceeded() { showStatusMessage("Puzzle solved automatically.", false); }

void MainWindow::onSolveFailed() {
    showStatusMessage("No solution exists for the current puzzle.", true);
}

void MainWindow::onPuzzleCompleted() {
    showStatusMessage("Congratulations -- you completed the puzzle!", false);
}

void MainWindow::onPuzzleGenerationSucceeded() {
    // generatePuzzle() does not carry the new board to this callback (unlike
    // onBoardChanged()), so this window fetches it itself and redraws -- the
    // same responsibility TerminalUI takes on for the same reason.
    onBoardChanged(game_.board());
    hasSelection_ = false;
    updateHighlighting();
    updateSelectionPanel();
    showStatusMessage("New puzzle generated.", false);
}

void MainWindow::onPuzzleGenerationFailed() {
    showStatusMessage("Could not generate a puzzle at that difficulty. Try again.", true);
}

} // namespace sudoku
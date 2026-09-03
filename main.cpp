#include <string>

// #define QT_UI
#ifdef QT_UI
#include <QApplication>
#include <QStyleFactory>

#include "MainWindow.hpp"
#else
#include "TerminalUI.hpp"
#endif

#include "app/SudokuGame.hpp"
#include "core/SudokuGenerator.hpp"
using namespace sudoku;
/**
 * @file main.cpp
 * @brief Entry point, shared between the Qt and terminal frontends.
 *
 * Which frontend gets built is selected by the QT_UI preprocessor define, passed
 * on the compiler command line (see .vscode/tasks.json's two build tasks, and
 * later the CMake build). Both branches follow the same shape: construct
 * SudokuGame, construct the frontend, connect them with setObserver(), load a
 * puzzle, then hand control to the frontend -- a blocking run() for the console,
 * Qt's own event loop for the GUI.
 */
int main(int argc, char *argv[]) {
#ifdef QT_UI
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    const std::string puzzleFileName = "test.txt";

    sudoku::SudokuGame game;
    sudoku::MainWindow window(game);
    game.setObserver(&window);

    game.loadPuzzle(puzzleFileName);

    window.show();
    return app.exec();
#else
    (void)argc;
    (void)argv;

    const std::string puzzleFileName = "test.txt";

    sudoku::SudokuGame game;
    sudoku::TerminalUI ui(game);
    game.setObserver(&ui);

    game.loadPuzzle(puzzleFileName);
    ui.run();

    return 0;
#endif
}
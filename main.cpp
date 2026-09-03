#include <string>

#ifdef QT_UI
#include <QApplication>
#include <QStyleFactory>

#include "MainWindow.hpp"
#else // TERMINAL_UI
#include "TerminalUI.hpp"
#endif

#include "app/SudokuGame.hpp"
#include "core/SudokuGenerator.hpp"
using namespace sudoku;

constexpr std::string_view puzzleFileName = "InitialPuzzle.txt";

int main(int argc, char *argv[]) {
#ifdef QT_UI
    // this needs to be called before creating the QApplication object,
    // otherwise it will be ignored
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    sudoku::SudokuGame game;
    sudoku::MainWindow ui(game);
    game.setObserver(&ui);

    game.loadPuzzle(puzzleFileName);

    ui.show();
    return app.exec();
#else
    (void)argc;
    (void)argv;

    sudoku::SudokuGame game;
    sudoku::TerminalUI ui(game);
    game.setObserver(&ui);

    game.loadPuzzle(puzzleFileName);
    ui.run();

    return 0;
#endif
}
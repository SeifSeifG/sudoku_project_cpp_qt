# Sudoku

A Sudoku game engine with two interchangeable frontends — a terminal UI and a Qt6 GUI — sharing a single game core through an observer interface. Built in modern C++ with CMake.

## Features

- **Puzzle generation** across multiple difficulty levels (Easy → Expert), with a backtracking solver used to verify each generated puzzle stays uniquely solvable
- **Two interfaces, one core** — `TerminalUI` and `MainWindow` (Qt) both implement a shared `GameObserver` interface, so `SudokuGame` never depends on which frontend is attached
- **Save / load** puzzles to and from file
- **Statically-linked terminal build** — `sudoku-term` runs standalone with no external runtime dependencies; the Qt build is skipped automatically if Qt6 isn't found, so the terminal target always builds cleanly

## Screenshots

<!-- Add screenshots or a short GIF of the Qt UI and terminal UI here -->

## Getting started

### Prerequisites

- CMake ≥ 3.16
- A C++20-capable compiler (project is developed against g++-15)
- Qt6 (`Widgets` component) — optional, only required to build the GUI target

### Build

```bash
git clone https://github.com/SeifSeifG/sudoku_project_cpp_qt.git
cd sudoku_project_cpp_qt
cmake -B build
cmake --build build
```

This produces two executables in `build/`:

| Target | Description |
|---|---|
| `sudoku-term` | Terminal interface, statically linked |
| `sudoku-qt` | Qt6 GUI interface (built only if Qt6 is found) |

### Run

```bash
./build/sudoku-term
./build/sudoku-qt
```

## Project structure

```
core/       Sudoku board, generator, and solver — UI-agnostic game logic
app/        SudokuGame + GameObserver: the shared interface both UIs implement
ui/Terminal/  Terminal frontend
ui/QT/        Qt6 frontend
main.cpp    Entry point; selects the active UI at compile time
```

## License

Distributed under the GPL-3.0 License. See [LICENSE](LICENSE) for details.

/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.hpp'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../ui/QT/MainWindow.hpp"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_sudoku__MainWindow_t {
    uint offsetsAndSizes[20];
    char stringdata0[19];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[4];
    char stringdata4[4];
    char stringdata5[13];
    char stringdata6[13];
    char stringdata7[21];
    char stringdata8[20];
    char stringdata9[8];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_sudoku__MainWindow_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_sudoku__MainWindow_t qt_meta_stringdata_sudoku__MainWindow = {
    {
        QT_MOC_LITERAL(0, 18),  // "sudoku::MainWindow"
        QT_MOC_LITERAL(19, 13),  // "onCellClicked"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 3),  // "row"
        QT_MOC_LITERAL(38, 3),  // "col"
        QT_MOC_LITERAL(42, 12),  // "onLoadPuzzle"
        QT_MOC_LITERAL(55, 12),  // "onSavePuzzle"
        QT_MOC_LITERAL(68, 20),  // "onSolveAutomatically"
        QT_MOC_LITERAL(89, 19),  // "onGenerateNewPuzzle"
        QT_MOC_LITERAL(109, 7)   // "onAbout"
    },
    "sudoku::MainWindow",
    "onCellClicked",
    "",
    "row",
    "col",
    "onLoadPuzzle",
    "onSavePuzzle",
    "onSolveAutomatically",
    "onGenerateNewPuzzle",
    "onAbout"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_sudoku__MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   50,    2, 0x08,    1 /* Private */,
       5,    0,   55,    2, 0x08,    4 /* Private */,
       6,    0,   56,    2, 0x08,    5 /* Private */,
       7,    0,   57,    2, 0x08,    6 /* Private */,
       8,    0,   58,    2, 0x08,    7 /* Private */,
       9,    0,   59,    2, 0x08,    8 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject sudoku::MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_sudoku__MainWindow.offsetsAndSizes,
    qt_meta_data_sudoku__MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_sudoku__MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'onCellClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onLoadPuzzle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSavePuzzle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSolveAutomatically'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onGenerateNewPuzzle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAbout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void sudoku::MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onCellClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->onLoadPuzzle(); break;
        case 2: _t->onSavePuzzle(); break;
        case 3: _t->onSolveAutomatically(); break;
        case 4: _t->onGenerateNewPuzzle(); break;
        case 5: _t->onAbout(); break;
        default: ;
        }
    }
}

const QMetaObject *sudoku::MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *sudoku::MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_sudoku__MainWindow.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "GameObserver"))
        return static_cast< GameObserver*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int sudoku::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

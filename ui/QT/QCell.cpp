#include "QCell.hpp"

#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QSize>

namespace sudoku {

namespace {
constexpr int CellPixelSize = 48;

// Border colors/widths per the spec: hairline 1px #bbb everywhere, except every
// 3rd row/column boundary and the outer edges, which are 2px #444. Because col 0
// and col Position::BoardSize-1 both satisfy the "% BoxSize == 0" test below, the
// outer-edge case falls out of the same formula as the box-boundary case -- no
// separate special-casing is needed.
const QColor kThinBorderColor("#bbbbbb");
const QColor kThickBorderColor("#444444");
const QColor kSelectedBg("#316ac5");
const QColor kHighlightedBg("#dce9fc");
const QColor kDefaultBg("#ffffff");
const QColor kGivenText("#000000");
const QColor kPlayerText("#1aa839");
} // namespace

QCell::QCell(int row, int col, QWidget *parent)
    : QWidget(parent), row_(row), col_(col), value_(Cell::EmptyValue), given_(false),
      selected_(false), highlighted_(false) {
    // setFixedSize(CellPixelSize, CellPixelSize);

    // 1. Ensure the cells never shrink smaller than 48x48
    setMinimumSize(CellPixelSize, CellPixelSize);

    // 2. Tell the cell it is allowed to grow in both directions
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

int QCell::row() const { return row_; }

int QCell::col() const { return col_; }

void QCell::setValue(int value, bool given) {
    value_ = value;
    given_ = given;
    update();
}

void QCell::setSelected(bool selected) {
    if (selected_ == selected) {
        return;
    }
    selected_ = selected;
    update();
}

void QCell::setHighlighted(bool highlighted) {
    if (highlighted_ == highlighted) {
        return;
    }
    highlighted_ = highlighted;
    update();
}

void QCell::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(row_, col_);
    }
    QWidget::mousePressEvent(event);
}

void QCell::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    const QRect area = rect();

    // Background.
    QColor bg = kDefaultBg;
    if (selected_) {
        bg = kSelectedBg;
    } else if (highlighted_) {
        bg = kHighlightedBg;
    }
    painter.fillRect(area, bg);

    // Digit.
    if (value_ != Cell::EmptyValue) {
        QFont font = painter.font();
        font.setPointSize(16);
        font.setBold(given_);
        painter.setFont(font);
        painter.setPen(given_ ? kGivenText : kPlayerText);
        painter.drawText(area, Qt::AlignCenter, QString::number(value_));
    }

    // Borders: every 3rd boundary (which includes both outer edges) is thick.
    const bool leftThick = (col_ % Position::BoxSize) == 0;
    const bool rightThick = ((col_ + 1) % Position::BoxSize) == 0;
    const bool topThick = (row_ % Position::BoxSize) == 0;
    const bool bottomThick = ((row_ + 1) % Position::BoxSize) == 0;

    auto drawEdge = [&painter](int x1, int y1, int x2, int y2, bool thick) {
        QPen pen(thick ? kThickBorderColor : kThinBorderColor);
        pen.setWidth(thick ? 2 : 1);
        painter.setPen(pen);
        painter.drawLine(x1, y1, x2, y2);
    };

    drawEdge(0, 0, 0, area.height() - 1, leftThick);
    drawEdge(area.width() - 1, 0, area.width() - 1, area.height() - 1, rightThick);
    drawEdge(0, 0, area.width() - 1, 0, topThick);
    drawEdge(0, area.height() - 1, area.width() - 1, area.height() - 1, bottomThick);
}

} // namespace sudoku
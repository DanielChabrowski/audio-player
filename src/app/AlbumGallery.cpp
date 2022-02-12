#include "AlbumGallery.hpp"

#include <QCursor>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QScrollBar>

AlbumGallery::AlbumGallery(QWidget *parent)
: QAbstractItemView{ parent }
{
    setAcceptDrops(false);
    setViewportMargins(0, 0, 0, 0);
    setFrameShape(QFrame::Shape::NoFrame);
    setFrameStyle(0);
}

int AlbumGallery::verticalOffset() const
{
    return verticalScrollBar()->value();
}

void AlbumGallery::doLayout()
{
    const auto widgetSize = viewport()->size();
    const int itemWidth = qBound<int>(192, widgetSize.width() * 0.25, 256);
    const int itemHeight = itemWidth + 48;
    const auto rows = model()->rowCount();

    items.reserve(rows);

    constexpr auto defaultSpacing = 5;

    const auto itemsPerRow = widgetSize.width() / (itemWidth + defaultSpacing);
    const auto remainder = widgetSize.width() - (itemWidth * itemsPerRow);
    const auto spacing = remainder / (itemsPerRow * 2);

    for(int row = 0; row < rows; ++row)
    {
        const int itemHorizontalIndex = row % itemsPerRow;
        int x = (itemWidth * itemHorizontalIndex) + spacing * (itemHorizontalIndex * 2 + 1);
        int y = (itemHeight + spacing) * (row / itemsPerRow);

        items.append(QRect{ x, y, itemWidth, itemHeight });
    }
}

void AlbumGallery::clear()
{
    items.clear();
}

void AlbumGallery::resizeEvent(QResizeEvent *event)
{
    clear();
    doLayout();

    auto contentsRect = QRect{};
    for(const auto &item : items)
    {
        contentsRect |= item;
    }

    verticalScrollBar()->setRange(0, contentsRect.height() - viewport()->height());
    verticalScrollBar()->setPageStep(100);
    verticalScrollBar()->setSingleStep(100);

    QAbstractItemView::resizeEvent(event);
}

void AlbumGallery::paintEvent(QPaintEvent *)
{
    constexpr int column{ 0 };
    const auto rowCount = model()->rowCount(rootIndex());

    QPainter painter{ viewport() };
    painter.setRenderHints(
        QPainter::RenderHint::Antialiasing | QPainter::RenderHint::SmoothPixmapTransform, true);

    const auto viewport = this->viewport()->rect().translated(horizontalOffset(), verticalOffset());

    for(auto row = 0; row < rowCount; ++row)
    {
        const QRect itemRect = items.at(row);
        if(!viewport.intersects(itemRect))
        {
            continue;
        }

        const auto rect{ itemRect.adjusted(0, -verticalOffset(), 0, -verticalOffset()) };

        const auto modelIndex = model()->index(row, column, rootIndex());
        const auto displayData = model()->data(modelIndex).value<QString>();
        const auto decoration = model()->data(modelIndex, Qt::DecorationRole).value<QImage>();

        const QRect targetRect{ rect.x(), rect.y(), rect.width(), rect.height() - 48 };
        if(decoration.width() > rect.width() || decoration.height() > rect.height())
        {
            auto minSize =
                decoration.width() > decoration.height() ? decoration.height() : decoration.width();

            const QSize sourceSize{ minSize, minSize };

            const QPoint offset(decoration.width() * 0.5 - sourceSize.width() * 0.5,
                decoration.height() * 0.5 - sourceSize.height() * 0.5);
            painter.drawImage(targetRect, decoration, QRect{ offset, sourceSize });
        }
        else
        {
            painter.drawImage(targetRect, decoration);
        }

        painter.drawRect(rect);
        painter.drawText(rect.adjusted(3, rect.height() - 42, 0, 0), 0, displayData);
    }
}

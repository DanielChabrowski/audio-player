#include "PlaylistWidget.hpp"

#include <QDebug>
#include <QKeyEvent>

PlaylistWidget::PlaylistWidget(std::function<void(int)> itemSelectedCallback, QWidget *parent)
: QTreeView{ parent }
, itemSelectedCallback_{ itemSelectedCallback }
{
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setFrameShape(QFrame::NoFrame);
    setSelectionMode(ExtendedSelection);
    setDragDropMode(DragDrop);
}

void PlaylistWidget::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);
    qDebug() << "Key pressed: " << event->key();
}

void PlaylistWidget::mouseMoveEvent(QMouseEvent *event)
{
    QTreeView::mouseMoveEvent(event);
}

void PlaylistWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(Qt::LeftButton != event->button())
    {
        return QTreeView::mouseDoubleClickEvent(event);
    }

    QModelIndex index = indexAt(event->pos());
    if(index.isValid())
    {
        itemSelectedCallback_(index.row());
        update();
    }
}

void PlaylistWidget::leaveEvent(QEvent *event)
{
    QTreeView::leaveEvent(event);
}

void PlaylistWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QTreeView::dragMoveEvent(event);
    qDebug() << "DropMoveEvent";
}

void PlaylistWidget::dropEvent(QDropEvent *event)
{
    QTreeView::dropEvent(event);
    qDebug() << "DropEvent";
}

void PlaylistWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
}

void PlaylistWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
}

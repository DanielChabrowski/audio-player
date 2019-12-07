#include "PlaylistWidget.hpp"

#include "Playlist.hpp"

#include <QDebug>
#include <QKeyEvent>
#include <QShortcut>

PlaylistWidget::PlaylistWidget(Playlist &playlist, std::function<void(int)> itemSelectedCallback, QWidget *parent)
: QTreeView{ parent }
, playlist_{ playlist }
, itemSelectedCallback_{ itemSelectedCallback }
{
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setFrameShape(QFrame::NoFrame);
    setSelectionMode(ExtendedSelection);
    setDropIndicatorShown(true);
    setDragDropMode(DragDrop);
    setDragEnabled(true);

    enablePlayTrackShortcut();
    enableDeleteTrackShortcut();
}

const Playlist &PlaylistWidget::getPlaylist() const
{
    return playlist_;
}

void PlaylistWidget::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);
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

    const auto index = indexAt(event->pos());
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

void PlaylistWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
}

void PlaylistWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
}

void PlaylistWidget::enablePlayTrackShortcut()
{
    const auto playShortcut = new QShortcut(Qt::Key_Return, this);

    // Probably has to be in Widget context or controlled externally
    playShortcut->setContext(Qt::ShortcutContext::WindowShortcut);

    connect(playShortcut, &QShortcut::activated, [this]() {
        const auto currentIndex = this->currentIndex();
        if(currentIndex.isValid())
        {
            itemSelectedCallback_(currentIndex.row());
            update();
        }
    });
}

void PlaylistWidget::enableDeleteTrackShortcut()
{
    const auto shortcut = new QShortcut(Qt::Key_Delete, this);
    shortcut->setContext(Qt::ShortcutContext::WidgetShortcut);

    connect(shortcut, &QShortcut::activated, [this]() {
        const auto indexes = selectionModel()->selectedRows();
        if(indexes.isEmpty())
        {
            return;
        }

        std::vector<std::size_t> indexesToRemove;
        std::for_each(indexes.begin(), indexes.end(), [&indexesToRemove](const auto &modelIndex) {
            indexesToRemove.push_back(modelIndex.row());
        });

        playlist_.removeTracks(indexesToRemove);
        clearSelection();
        update();
    });
}

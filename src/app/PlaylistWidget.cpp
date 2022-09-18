#include "PlaylistWidget.hpp"

#include "Playlist.hpp"

#include <QMouseEvent>
#include <QShortcut>

PlaylistWidget::PlaylistWidget(Playlist &playlist, std::function<void(int)> itemSelectedCallback, QWidget *parent)
: QTreeView{ parent }
, playlist_{ playlist }
, itemSelectedCallback_{ std::move(itemSelectedCallback) }
{
    setAllColumnsShowFocus(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setItemsExpandable(false);
    setExpandsOnDoubleClick(false);
    setRootIsDecorated(false);
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

void PlaylistWidget::enablePlayTrackShortcut()
{
    const auto playShortcut = new QShortcut(Qt::Key_Return, this);
    playShortcut->setContext(Qt::ShortcutContext::WidgetShortcut);

    connect(playShortcut, &QShortcut::activated,
        [this]()
        {
            const auto currentIndex = this->currentIndex();
            if(currentIndex.isValid())
            {
                itemSelectedCallback_(currentIndex.row());
                update();
            }
        });
}

using ModelIndexListView = std::pair<QModelIndexList::const_iterator, QModelIndexList::const_iterator>;

std::vector<ModelIndexListView> consecutive_values(const QModelIndexList::const_iterator begin,
    const QModelIndexList::const_iterator end)
{
    if(begin == end) return {};

    std::vector<ModelIndexListView> views;
    auto start = begin;
    for(auto it = begin + 1; it != end; ++it)
    {
        if((it - 1)->row() + 1 != it->row())
        {
            views.emplace_back(ModelIndexListView{ start, it });
            start = it;
        }
    }
    views.emplace_back(ModelIndexListView{ start, end });

    return views;
}

void PlaylistWidget::enableDeleteTrackShortcut()
{
    const auto shortcut = new QShortcut(Qt::Key_Delete, this);
    shortcut->setContext(Qt::ShortcutContext::WidgetShortcut);

    connect(shortcut, &QShortcut::activated,
        [this]()
        {
            auto *model = this->model();
            auto selectedRows = selectionModel()->selectedRows();

            if(selectedRows.empty()) return;

            // User could select rows in random order
            std::sort(selectedRows.begin(), selectedRows.end());

            // Batch removal is faster therefore compute views of consecutive values
            const auto views = consecutive_values(selectedRows.cbegin(), selectedRows.cend());

            // Remove from end to the beginning to not invalidate indexes
            std::for_each(views.crbegin(), views.crend(),
                [model](const auto view)
                {
                    const auto count = std::distance(view.first, view.second);
                    model->removeRows(view.first->row(), count);
                });
        });
}

#pragma once

#include <QAbstractItemView>
#include <QList>
#include <QRect>

class AlbumGallery : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit AlbumGallery(QWidget *parent = nullptr);

    QRect visualRect(const QModelIndex &index) const override
    {
        (void)index;
        return QRect{};
    }

    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override
    {
        (void)index;
        (void)hint;
    }

    QModelIndex indexAt(const QPoint &point) const override
    {
        (void)point;
        return QModelIndex();
    }

    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) override
    {
        return QModelIndex();
    }

    int horizontalOffset() const override
    {
        return 0;
    }

    int verticalOffset() const override;

    bool isIndexHidden(const QModelIndex &) const override
    {
        return false;
    }

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override
    {
        (void)rect;
        (void)command;
    }

    QRegion visualRegionForSelection(const QItemSelection &selection) const override
    {
        (void)selection;
        return QRegion();
    }

private:
    void doLayout();
    void clear();

    QList<QRect> items;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

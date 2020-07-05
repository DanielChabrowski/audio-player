#pragma once

#include <QMouseEvent>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

struct Tab
{
    QString text;
    QRect rect;
};

class MultilineTabBar : public QWidget
{
    Q_OBJECT

public:
    MultilineTabBar(QWidget *parent);

    virtual ~MultilineTabBar() = default;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void showEvent(QShowEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

    int count() const
    {
        return tabs_.count();
    }

    QSize tabSizeHint(int index) const;

    int tabAt(QPoint p);

    QRect tabRect(int index)
    {
        return tabs_.at(index).rect;
    }

    const QString &tabText(int index) const
    {
        return tabs_.at(index).text;
    }

    void setTabText(int, QString)
    {
        recalculateTabsLayout();
    }

    int addTab(Tab tab);
    void removeTab(int tabIndex);

    int currentIndex()
    {
        return currentIndex_;
    }

    void setCurrentIndex(int tabIndex)
    {
        currentIndex_ = tabIndex;
        emit currentChanged(tabIndex);
    }

private:
    inline bool validIndex(int index) const
    {
        return index >= 0 && index < tabs_.count();
    }

    void recalculateTabsLayout();

Q_SIGNALS:
    void tabDoubleClicked(int tabIndex);
    void currentChanged(int tabIndex);

private:
    QList<Tab> tabs_;
    int currentIndex_{ 0 };
};

class MultilineTabWidget : public QWidget
{
    Q_OBJECT

public:
    MultilineTabWidget(QWidget *parent = nullptr);

    virtual ~MultilineTabWidget() = default;

    void resizeEvent(QResizeEvent *) override;
    void paintEvent(QPaintEvent *) override;

    QWidget *widget(int index)
    {
        return stack_->widget(index);
    }

    int count()
    {
        return tabBar_->count();
    }

    int addTab(QWidget *widget, QString tabText);
    void removeTab(int tabIndex);

    int currentIndex()
    {
        return tabBar_->currentIndex();
    }

    void setCurrentIndex(int tabIndex)
    {
        tabBar_->setCurrentIndex(tabIndex);
    }

    MultilineTabBar *tabBar() const
    {
        return tabBar_;
    }

private:
    void swichWidget(int index)
    {
        stack_->setCurrentIndex(index);
    }

    void privRemoveTab(int tabIndex)
    {
        tabBar_->removeTab(tabIndex);
        update();
        updateGeometry();
    }

private:
    MultilineTabBar *tabBar_;
    QStackedWidget *stack_;

    QRect panelRect;
};

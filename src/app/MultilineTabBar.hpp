#pragma once

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStackedWidget>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QTimer>
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
    MultilineTabBar(QWidget *parent)
    : QWidget{ parent }
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    virtual ~MultilineTabBar() = default;

    QSize sizeHint() const override
    {
        const_cast<MultilineTabBar *>(this)->recalculateTabsLayout();

        QRect r;
        for(int i = 0; i < tabs_.count(); ++i)
        {
            r = r.united(tabs_[i].rect);
        }
        return r.size();
    }

    QSize minimumSizeHint() const override
    {
        const QSize sh = tabSizeHint(0);
        return { sh.width(), sh.height() };
    }

    void showEvent(QShowEvent *) override
    {
        recalculateTabsLayout();

        if(isVisible())
        {
            update();
            updateGeometry();
        }
    }

    void resizeEvent(QResizeEvent *) override
    {
        recalculateTabsLayout();
        updateGeometry();
    }

    void paintEvent(QPaintEvent *) override
    {
        QStylePainter p{ this };

        for(int i = 0; i < tabs_.count(); ++i)
        {
            const auto tabRect = this->tabRect(i);
            QStyleOptionTab tab;
            tab.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
            tab.state |= QStyle::State_Enabled | QStyle::State_Active;

            if(i == currentIndex_)
            {
                tab.state |= QStyle::State_HasFocus | QStyle::State_Selected;
            }

            tab.text = tabText(i);
            tab.rect = tabRect;
            p.drawControl(QStyle::CE_TabBarTab, tab);
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        event->accept();
        for(int i = 0; i < tabs_.count(); ++i)
        {
            const auto rect = tabRect(i);
            if(rect.contains(event->pos()))
            {
                if(i != currentIndex_)
                {
                    currentIndex_ = i;
                    update();

                    QTimer::singleShot(0, this, [this]() { emit currentChanged(currentIndex_); });
                }

                break;
            }
        }
    }

    int count() const
    {
        return tabs_.count();
    }

    QSize tabSizeHint(int index) const
    {
        // constexpr qreal fontSize = 10.f;
        constexpr int spacing = 12;

        // QFont boldFont(font());
        // boldFont.setPointSizeF(fontSize);
        // boldFont.setBold(true);
        // const QFontMetrics fm(boldFont);
        const auto fm = fontMetrics();
        const int textWidth = fm.horizontalAdvance(tabText(index));

        auto hSpace = style()->proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace);
        return { textWidth + hSpace, spacing + fm.height() };
    }

    int tabAt(QPoint p)
    {
        for(int i = 0; i < tabs_.count(); ++i)
        {
            if(tabs_[i].rect.contains(p))
            {
                return i;
            }
        }

        return -1;
    }

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

    int addTab(Tab tab)
    {
        tabs_.append(std::move(tab));
        recalculateTabsLayout();
        updateGeometry();
        return tabs_.size() - 1;
    }

    void removeTab(int tabIndex)
    {
        tabs_.removeAt(tabIndex);
        recalculateTabsLayout();
        updateGeometry();
    }

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
    void recalculateTabsLayout()
    {
        int hOffset = 0;
        int vOffset = 0;

        int rowUsedSpace = 0;
        int rowFirstTab = 0;

        for(int i = 0; i < tabs_.count(); ++i)
        {
            auto &tab = tabs_[i];
            auto sizeHint = tabSizeHint(i);

            if(hOffset + sizeHint.width() > width())
            {
                hOffset = 0;
                vOffset += sizeHint.height();

                const int leftoverSpace = width() - rowUsedSpace - 1;
                const int tabsInRow = i - rowFirstTab;
                const int spacePerTab = leftoverSpace / tabsInRow;
                const int spaceRemainder = leftoverSpace % tabsInRow;

                // Give leftover space back to tabs in a row
                for(int rowTabIndex = rowFirstTab; rowTabIndex < i; ++rowTabIndex)
                {
                    auto &rowTab = tabs_[rowTabIndex];
                    const int moveBy = spacePerTab * (rowTabIndex - rowFirstTab);
                    rowTab.rect.moveLeft(rowTab.rect.x() + moveBy);
                    rowTab.rect.setWidth(rowTab.rect.width() + spacePerTab);
                }

                // Give leftover space to the last tab
                if(spaceRemainder > 0)
                {
                    auto &lastRowTab = tabs_[i - 1];
                    lastRowTab.rect.setWidth(lastRowTab.rect.width() + spaceRemainder);
                }

                rowFirstTab = i;
                rowUsedSpace = sizeHint.width();
            }
            else
            {
                rowUsedSpace += sizeHint.width();
            }

            tab.rect = QRect{ QPoint{ hOffset, vOffset }, sizeHint };

            hOffset += sizeHint.width();
        }
    }

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
    MultilineTabWidget(QWidget *parent = nullptr)
    : QWidget{ parent }
    , tabBar_{ new MultilineTabBar(this) }
    , stack_{ new QStackedWidget(this) }
    {
        auto mainLayout2 = new QHBoxLayout;

        auto mainLayout = new QVBoxLayout;
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        mainLayout->addWidget(tabBar_);
        mainLayout->addWidget(stack_);

        mainLayout2->addLayout(mainLayout);
        mainLayout2->setContentsMargins(0, 0, 0, 0);
        mainLayout2->setSpacing(0);

        setLayout(mainLayout2);

        connect(tabBar_, &MultilineTabBar::currentChanged, this, &MultilineTabWidget::swichWidget);
        connect(stack_, &QStackedWidget::widgetRemoved, this, &MultilineTabWidget::privRemoveTab);
    }

    virtual ~MultilineTabWidget() = default;

    QWidget *widget(int index)
    {
        return stack_->widget(index);
    }

    int count()
    {
        return tabBar_->count();
    }

    int addTab(QWidget *widget, QString tabText)
    {
        stack_->addWidget(widget);
        const int tabIndex = tabBar_->addTab(Tab{ std::move(tabText), QRect{} });
        update();
        return tabIndex;
    }

    void removeTab(int tabIndex)
    {
        auto *widget = stack_->widget(tabIndex);
        if(widget)
        {
            stack_->removeWidget(widget);
        }
    }

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
};

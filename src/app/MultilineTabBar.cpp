#include "MultilineTabBar.hpp"
#include <QApplication>
#include <QPainter>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QTimer>

MultilineTabBar::MultilineTabBar(QWidget *parent)
: QWidget{ parent }
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QSize MultilineTabBar::sizeHint() const
{
    const_cast<MultilineTabBar *>(this)->recalculateTabsLayout();

    QRect r;
    for(int i = 0; i < tabs_.count(); ++i)
    {
        r = r.united(tabs_[i].rect);
    }

    QSize sz = QApplication::globalStrut();
    return r.size().expandedTo(sz);
}

QSize MultilineTabBar::minimumSizeHint() const
{
    if(tabs_.count() > 0)
    {
        const QSize sh = tabSizeHint(0);
        return { sh.width(), sh.height() };
    }

    return { 0, 0 };
}

void MultilineTabBar::showEvent(QShowEvent *)
{
    recalculateTabsLayout();

    if(isVisible())
    {
        update();
        updateGeometry();
    }
}

void MultilineTabBar::resizeEvent(QResizeEvent *)
{
    recalculateTabsLayout();
    updateGeometry();
}

void MultilineTabBar::paintEvent(QPaintEvent *)
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

void MultilineTabBar::mousePressEvent(QMouseEvent *event)
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

void MultilineTabBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->accept();

    const auto tabIndex = tabAt(event->pos());
    if(tabIndex != -1)
    {
        emit tabDoubleClicked(tabIndex);
    }
}

QSize MultilineTabBar::tabSizeHint(int index) const
{
    constexpr int spacing = 12;

    const auto fm = fontMetrics();
    const int textWidth = fm.horizontalAdvance(tabText(index));

    auto hSpace = style()->proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace);
    return { textWidth + hSpace, spacing + fm.height() };
}

int MultilineTabBar::tabAt(QPoint p)
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

void MultilineTabBar::setTabText(int index, QString text)
{
    if(validIndex(index))
    {
        tabs_[index].text = text;
        recalculateTabsLayout();
        update();
        updateGeometry();
    }
}

int MultilineTabBar::addTab(Tab tab)
{
    tabs_.append(std::move(tab));
    recalculateTabsLayout();
    updateGeometry();
    return tabs_.size() - 1;
}

void MultilineTabBar::removeTab(int tabIndex)
{
    if(validIndex(tabIndex))
    {
        tabs_.removeAt(tabIndex);
        setCurrentIndex(std::clamp(currentIndex_ - 1, 0, tabs_.size()));
        recalculateTabsLayout();
        updateGeometry();
    }
}

void MultilineTabBar::recalculateTabsLayout()
{
    int hOffset = 0;
    int vOffset = 0;

    int rowUsedSpace = 0;
    int rowFirstTab = 0;

    for(int i = 0; i < tabs_.count(); ++i)
    {
        auto &tab = tabs_[i];
        auto sizeHint = tabSizeHint(i);

        const int tabsInRow = i - rowFirstTab;
        if(tabsInRow != 0 and hOffset + sizeHint.width() > width())
        {
            hOffset = 0;
            vOffset += sizeHint.height() - 4;
            // -4 to keep the rows right below each other

            const int leftoverSpace = width() - rowUsedSpace - 1;
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

MultilineTabWidget::MultilineTabWidget(QWidget *parent)
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

    QStyleOptionTabWidgetFrame option;
    panelRect = style()->subElementRect(QStyle::SE_TabWidgetTabPane, &option, this);
}

void MultilineTabWidget::resizeEvent(QResizeEvent *)
{
    QStyleOptionTabWidgetFrame option;
    panelRect = style()->subElementRect(QStyle::SE_TabWidgetTabPane, &option, this);
}

void MultilineTabWidget::paintEvent(QPaintEvent *)
{
    QStylePainter p{ this };

    QStyleOptionTabWidgetFrame opt;
    opt.rect = stack_->geometry();
    opt.shape = QTabBar::Shape::RoundedNorth;
    opt.tabBarRect = tabBar_->geometry();
    opt.tabBarSize = tabBar_->size();
    opt.lineWidth = 40;
    opt.midLineWidth = 50;
    opt.selectedTabRect = tabBar_->tabRect(tabBar_->currentIndex());

    p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
}

int MultilineTabWidget::addTab(QWidget *widget, QString tabText)
{
    stack_->addWidget(widget);
    const int tabIndex = tabBar_->addTab(Tab{ std::move(tabText), QRect{} });
    update();
    return tabIndex;
}

void MultilineTabWidget::removeTab(int tabIndex)
{
    auto *widget = stack_->widget(tabIndex);
    if(widget)
    {
        stack_->removeWidget(widget);
    }
}

#include "MultilineTabBar.hpp"
#include <QApplication>
#include <QBoxLayout>
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
    QRect r;
    for(const auto &tab : tabs_)
    {
        r = r.united(tab.rect);
    }

    QSize sz = QApplication::globalStrut();
    return r.size().expandedTo(sz);
}

QSize MultilineTabBar::minimumSizeHint() const
{
    const auto sh = sizeHint();

    if(tabs_.count() > 0)
    {
        const QSize tabSh = tabSizeHint(0);
        return { tabSh.width(), sh.height() };
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
        QStyleOptionTab styleOption{};
        styleOption.text = tabText(i);

        const auto &tab = tabs_[i];
        const bool isBegginingTab = i == 0 or tab.isFirstOnRow;
        const bool isEndTab = i == tabs_.count() - 1 or tab.isLastOnRow;

        if(tabs_.count() == 1 or (isBegginingTab && isEndTab))
        {
            styleOption.position = QStyleOptionTab::TabPosition::OnlyOneTab;
        }
        else if(isBegginingTab)
        {
            styleOption.position = QStyleOptionTab::TabPosition::Beginning;
        }
        else if(isEndTab)
        {
            styleOption.position = QStyleOptionTab::TabPosition::End;
        }
        else
        {
            styleOption.position = QStyleOptionTab::TabPosition::Middle;
        }

        styleOption.state &= ~(QStyle::State_HasFocus | QStyle::State_MouseOver);
        styleOption.state |= QStyle::State_Enabled | QStyle::State_Active;

        if(i == currentIndex_)
        {
            styleOption.state |= QStyle::State_Selected;
        }

        styleOption.rect = tab.rect;
        p.drawControl(QStyle::CE_TabBarTab, styleOption);
    }
}

void MultilineTabBar::mousePressEvent(QMouseEvent *event)
{
    event->accept();

    const auto tabIndex = tabAt(event->pos());
    if(tabIndex != -1 && tabIndex != currentIndex_)
    {
        currentIndex_ = tabIndex;
        update();

        QTimer::singleShot(0, this, [this]() { emit currentChanged(currentIndex_); });
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
    const auto text = tabText(index);
    const auto fm = fontMetrics();
    const auto textSize = fm.size(Qt::TextShowMnemonic, text);

    QStyleOptionTab opt;
    opt.text = text;
    opt.rect = tabRect(index);

    QSize contentSize{ textSize.width(), fm.height() };
    return style()->sizeFromContents(QStyle::CT_TabBarTab, &opt, contentSize, this);
}

int MultilineTabBar::tabAt(QPoint p) const
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

QRect MultilineTabBar::tabRect(int index) const
{
    return validIndex(index) ? tabs_.at(index).rect : QRect();
}

QString MultilineTabBar::tabText(int index) const
{
    return validIndex(index) ? tabs_.at(index).text : QString();
}

void MultilineTabBar::setTabText(int index, QString text)
{
    if(validIndex(index))
    {
        tabs_[index].text = std::move(text);

        recalculateTabsLayout();
        update();
        updateGeometry();
    }
}

int MultilineTabBar::addTab(const Tab &tab)
{
    tabs_.append(tab);

    if(isVisible())
    {
        recalculateTabsLayout();
    }

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

void MultilineTabBar::setCurrentIndex(int tabIndex)
{
    if(currentIndex_ != tabIndex)
    {
        currentIndex_ = tabIndex;
        update();
        emit currentChanged(tabIndex);
    }
}

void MultilineTabBar::recalculateTabsLayout()
{
    int row = 0;
    int hOffset = 0;
    int vOffset = 0;

    int rowUsedSpace = 0;
    int rowFirstTab = 0;

    for(int i = 0; i < tabs_.count(); ++i)
    {
        auto &tab = tabs_[i];
        auto sizeHint = tabSizeHint(i);

        tab.isFirstOnRow = false;
        tab.isLastOnRow = false;

        const int tabsInRow = i - rowFirstTab;
        if(tabsInRow != 0 and hOffset + sizeHint.width() > width())
        {
            row += 1;
            hOffset = 0;
            vOffset += sizeHint.height();

            const int leftoverSpace = width() - rowUsedSpace;
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

            tabs_[i].isFirstOnRow = true;
            tabs_[i - 1].isLastOnRow = true;

            rowFirstTab = i;
            rowUsedSpace = sizeHint.width();
        }
        else
        {
            rowUsedSpace += sizeHint.width();
        }

        tab.rect = QRect{ QPoint{ hOffset, vOffset }, sizeHint };
        tab.isOnAFirstRow = (row == 0);

        hOffset += sizeHint.width();
    }
}

MultilineTabWidget::MultilineTabWidget(QWidget *parent)
: QWidget{ parent }
, tabBar_{ new MultilineTabBar() }
, stack_{ new QStackedWidget() }
{
    setLayout(new QBoxLayout(QBoxLayout::Direction::TopToBottom));
    layout()->setContentsMargins(0, 1, 0, 0); // TODO: Figure out the offset of `pane`
    layout()->setSpacing(0);
    layout()->addWidget(tabBar_);
    layout()->addWidget(stack_);

    connect(tabBar_, &MultilineTabBar::currentChanged, this, &MultilineTabWidget::switchWidget);
    connect(stack_, &QStackedWidget::widgetRemoved, this, &MultilineTabWidget::privRemoveTab);
}

void MultilineTabWidget::paintEvent(QPaintEvent *)
{
    QStylePainter p{ this };

    QStyleOptionTabWidgetFrame opt;
    opt.rect = QRect(0, 0, tabBar_->sizeHint().width(), tabBar_->height());
    opt.tabBarRect = tabBar_->geometry();
    opt.tabBarSize = tabBar_->size();
    opt.lineWidth = 1;
    opt.midLineWidth = 0;
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

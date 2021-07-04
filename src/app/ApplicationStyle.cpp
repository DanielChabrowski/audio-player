#include "ApplicationStyle.hpp"

int ApplicationStyle::styleHint(QStyle::StyleHint hint,
    const QStyleOption *option,
    const QWidget *widget,
    QStyleHintReturn *returnData) const
{
    if(QStyle::SH_Slider_AbsoluteSetButtons == hint)
    {
        // Allows moving slider's tick by clicking anywhere on a slider
        return Qt::LeftButton;
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);
}

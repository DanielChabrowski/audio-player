#pragma once

#include <QProxyStyle>

class ApplicationStyle final : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint,
        const QStyleOption * = nullptr,
        const QWidget * = nullptr,
        QStyleHintReturn * = nullptr) const override;
};

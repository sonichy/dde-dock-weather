#include "dde-dock/constants.h"
#include "weatherwidget.h"
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

WeatherWidget::WeatherWidget(QWidget *parent)
    : QWidget(parent),
    m_settings("deepin", "dde-dock-HTYWeather")
{
    sw = "Weather";
    temp = "Temp";
    pixmap = QPixmap(":icon/Default/na.png");
}

bool WeatherWidget::enabled()
{
    return m_settings.value(PLUGIN_STATE_KEY, true).toBool();
}

void WeatherWidget::setEnabled(const bool b)
{
    m_settings.setValue(PLUGIN_STATE_KEY, b);
}

QSize WeatherWidget::sizeHint() const
{
    QFontMetrics FM(qApp->font());
    QSize size;
    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();
    if (displayMode == Dock::Efficient) {
        if(FM.boundingRect(sw).width() >= FM.boundingRect(temp).width()){
            size = FM.boundingRect(sw).size() + QSize(10,FM.boundingRect(sw).height());
        }else{
            size = FM.boundingRect(temp).size() + QSize(10,FM.boundingRect(temp).height());
        }
    }else{
        const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
        if (position == Dock::Top || position == Dock::Bottom)
            size = QSize(height(), height());
        else
            size = QSize(width(), width());
    }
    return size;
}

void WeatherWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void WeatherWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (displayMode == Dock::Efficient) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, sw + "\n" + temp);
    } else {
        QPixmap pixmap1 = pixmap.scaled(width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(rect().center() - pixmap1.rect().center(), pixmap1);
    }
}
#include "constants.h"
#include "weatherwidget.h"
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

WeatherWidget::WeatherWidget(QWidget *parent)
    : QWidget(parent),
    m_settings("deepin", "dde-dock-weather")
{
    sw1 = "天气";
    temp = "温度";
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
        if(FM.boundingRect(sw1).width() >= FM.boundingRect(temp).width()){
            size = FM.boundingRect(sw1).size();
        }else{
            size = FM.boundingRect(temp).size();
        }
    }else{
        FM.boundingRect("天气").size();
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
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    if (displayMode == Dock::Efficient) {
        painter.drawText(rect(), Qt::AlignCenter, sw1 + "\n" + temp);
    }else{
        painter.drawImage(rect(), image);
    }
}

void WeatherWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::RightButton)
        return QWidget::mousePressEvent(e);

    const QPoint p(e->pos() - rect().center());
    if (p.manhattanLength() < std::min(width(), height()) * 0.8 * 0.5)
    {
        emit requestContextMenu();
        return;
    }

    QWidget::mousePressEvent(e);
}

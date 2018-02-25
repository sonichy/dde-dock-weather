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
    text = "天气\n温度";
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
    QFontMetrics fm(qApp->font());
    return fm.boundingRect("25°C").size() + QSize(0, 0);
}

void WeatherWidget::resizeEvent(QResizeEvent *e)
{    
    QWidget::resizeEvent(e);
}

void WeatherWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter, text);
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

#ifndef LINESERIES_H
#define LINESERIES_H
#include <QObject>
#include <QLineSeries>
#include <QWidget>
#include <Qt>

QT_CHARTS_USE_NAMESPACE

class LineSeries : public QLineSeries
{
    Q_OBJECT
public:
    explicit LineSeries(QWidget* parent = Q_NULLPTR);
    ~LineSeries();
Q_SIGNALS:
        void pressed(const QPointF &point,const LineSeries* name= 0);
};

#endif // LINESERIES_H

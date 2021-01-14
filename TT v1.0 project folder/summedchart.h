//handles chart model/view
#ifndef ORHCHART_H
#define ORHCHART_H

#include "summedtablemodel.h"
#include "barproxymodel.h"

//debug
#include <QDebug>
//date
#include <QDateTime>
//data view & widgets
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

class SummedChart : public QObject
{
private:
    QChart *chart;
    //models
    SummedTableModel *model = NULL;
    BarProxyModel *barModel = NULL;
    //select graph
    //0-demand   1-avg travel time   2-avg rev   3-total rev
    //4-peak hours   5-avg rev bar chart   6-avg travel time bar chart
    int graphSelected = 0;
    //for line graphs
    QSplineSeries *series;
    QVXYModelMapper *mapper;
    QChartView *chartView;
    QDateTimeAxis *xAxis;
    QValueAxis *yAxis;
    //for bar charts
    QBarSeries *barSeries;
    QVBarModelMapper *barMapper;
    QBarCategoryAxis *barAxis;
    QValueAxis *barYAxis;

public:
    SummedChart();
    ~SummedChart();
    void setModel(SummedTableModel *model);
    void setXMin(const QDateTime &datetime);
    void setXMax(const QDateTime &datetime);
    void updateXTickCount();
    void updateYAxis();
    void swapToGraph(int selected);
    int getSelected();
    int getYMax();
    int getYAxisMax();
    QChartView* getChartView();
};

#endif // ORHCHART_H

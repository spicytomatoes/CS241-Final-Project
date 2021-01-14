#include "summedchart.h"

SummedChart::SummedChart()
{
    chart = new QChart;
    chartView = new QChartView(chart);
    series = new QSplineSeries;
    mapper = new QVXYModelMapper(this);
    xAxis = new QDateTimeAxis();
    xAxis->setFormat("MM-dd hh:mm");
    xAxis->setTitleText("time");
    yAxis = new QValueAxis();
    yAxis->setTitleText("Orders");
    chart->setAnimationOptions(QChart::AllAnimations);
    chartView->setRenderHint(QPainter::Antialiasing);
    mapper->setXColumn(0);
    mapper->setYColumn(1);
    swapToGraph(0);

    mapper->setSeries(series);
    chart->addSeries(series);
    chart->addAxis(xAxis,Qt::AlignBottom);
    chart->addAxis(yAxis,Qt::AlignLeft);
    chart->legend()->hide();
    series->attachAxis(xAxis);
    series->attachAxis(yAxis);

    //bar chart setup
    barSeries = new QBarSeries();
    barAxis = new QBarCategoryAxis();
    barAxis->setTitleText("time (hour)");
    barModel = new BarProxyModel();
    barMapper = new QVBarModelMapper(this);
    QStringList categories;
    for(int i = 0; i < 24; i++)
    {
        QString t = QString::number(i);
        categories.append(t);
    }
    barAxis->setCategories(categories);
    barMapper->setSeries(barSeries);
    barMapper->setFirstBarSetColumn(1);
    barMapper->setLastBarSetColumn(1);
    barMapper->setFirstRow(0);
    barMapper->setRowCount(24);

}

SummedChart::~SummedChart()
{
    delete chart;
    delete chartView;
    delete series;
    delete mapper;
    delete xAxis;
    delete yAxis;
}

void SummedChart::setModel(SummedTableModel *m)
{
    model = m;
    mapper->setModel(model);
    barModel->setSourceModel(model);
    barMapper->setModel(barModel);
}

void SummedChart::setXMin(const QDateTime &datetime)
{
    xAxis->setMin(datetime);
    //updateXTickCount();
}

void SummedChart::setXMax(const QDateTime &datetime)
{
    xAxis->setMax(datetime);
    //updateXTickCount();
}
//not used
void SummedChart::updateXTickCount()
{
    int hours = xAxis->min().secsTo(xAxis->max()) / 3600;
    xAxis->setTickCount(hours/24+1);
}

QChartView *SummedChart::getChartView()
{
    return chartView;
}

//reset y-axis to fit data
void SummedChart::updateYAxis()
{
    yAxis->setMax(getYMax());
    yAxis->applyNiceNumbers();
}

//0-demand   1-avg travel time   2-avg rev   3-total rev
//4-peak hours   5-avg travel time bar chart    6-avg rev bar chart
void SummedChart::swapToGraph(int selected)
{
    if (model == NULL) return;
    if (selected == graphSelected) return;
    if (selected < 0 || selected > 6) return;

    if (selected == 0) {
        chart->setTitle("Demand over time");
        yAxis->setTitleText("Orders");
    } else if (selected == 1) {
        chart->setTitle("Average travel time");
        yAxis->setTitleText("Travel Time (seconds)");
    } else if (selected == 2) {
        chart->setTitle("Average Revenue per hour");
        yAxis->setTitleText("Revenue (RMB)");
    } else if (selected == 3) {
        chart->setTitle("Total Revenue");
        yAxis->setTitleText("Revenue (RMB)");
    } else if (selected == 4) {
        chart->setTitle("Peak Hours");
        yAxis->setTitleText("Average orders");
    } else if (selected == 5) {
        chart->setTitle("Average Travel Time by Time of Day");
        yAxis->setTitleText("Travel Time (seconds)");
    } else if (selected == 6) {
        chart->setTitle("Avarage Revenue by Time of Day");
        yAxis->setTitleText("Revenue (RMB)");
    }

    if (selected < 4)   //spline chart
    {
        mapper->setYColumn(selected+1);
        if (graphSelected >= 4)
        {
            chart->setAnimationOptions(QChart::AllAnimations);
            chart->removeSeries(barSeries);
            chart->removeAxis(barAxis);
            chart->addAxis(xAxis,Qt::AlignBottom);
            chart->addSeries(series);
            series->attachAxis(xAxis);
            series->attachAxis(yAxis);
        }
    }
    else
    {
        barMapper->setFirstBarSetColumn(selected-3);
        barMapper->setLastBarSetColumn(selected-3);
        if (graphSelected < 4)
        {
            chart->setAnimationOptions(QChart::SeriesAnimations);
            chart->removeSeries(series);
            chart->removeAxis(xAxis);
            chart->addAxis(barAxis,Qt::AlignBottom);
            chart->addSeries(barSeries);
            barSeries->attachAxis(barAxis);
            barSeries->attachAxis(yAxis);
        }
    }
    graphSelected = selected;
    updateYAxis();
}

//return selected graph
int SummedChart::getSelected()
{
    return graphSelected;
}

//return max value of data shown
int SummedChart::getYMax()
{
    int max = 0;
    if (graphSelected < 4)
    {
        for (int i = 0; i < series->count(); i++)
        {
            if (max < series->at(i).y()) max = series->at(i).y();
        }
    } else {
        if (barSeries->barSets().size() == 0) {
            qDebug() << "empty bars!";
            return 0;
        }
        for (int i = 0; i < barSeries->barSets()[0]->count(); i++)
        {
            if (max <  barSeries->barSets()[0]->at(i)) max = barSeries->barSets()[0]->at(i);
        }
    }

    return max;
}

//return max value of y-axis
int SummedChart::getYAxisMax()
{
    return yAxis->max();
}

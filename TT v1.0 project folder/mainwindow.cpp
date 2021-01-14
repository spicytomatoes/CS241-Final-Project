#include "mainwindow.h"
#include "ui_mainwindow.h"

//stores info about grids, where x, y is the most NW vertex,
//lngGap and latGap is the size of one grid
//number of grids is hardcoded to 10x10
struct grid
{
    double lng,lat,lngGap,latGap;
    //the full region
    QGeoRectangle area;
    //the input la, ln here is the most SW or bottom left of grids
    grid(double ln , double la, double lnG, double laG):lngGap(lnG),latGap(laG) {
        QGeoCoordinate botLeft(la,ln);
        QGeoCoordinate topRight(la+10*laG,ln+10*lnG);
        area.setBottomLeft(botLeft);
        area.setTopRight(topRight);
        QGeoCoordinate topLeft = area.topLeft();
        lng = topLeft.longitude();
        lat = topLeft.latitude();
    };
};
grid *theGrid = NULL;
//map grid selection
int selectedID = -1;
bool routeMode = false;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("TT v1.0 - powered by Qt");

    //setup ui
    ui->GFTop->setAlignment(ui->startEndLabel,Qt::AlignRight);
    ui->GFTop->setAlignment(ui->startEndSelector,Qt::AlignRight);
    ui->selectGraphLayout->setAlignment(ui->selectGraphLabel,Qt::AlignRight);
    ui->hMNoneButton->toggle();

    QWidget *spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    ui->toolBar->addWidget(spacerWidget);

    //time filters
    startDTFilter = new QDateTimeEdit(this);
    endDTFilter = new QDateTimeEdit(this);
    QLabel *startLabel = new QLabel(this);
    QLabel *endLabel = new QLabel(this);
    startLabel->setText("Filter order time from  ");
    endLabel->setText("   to   ");
    ui->toolBar->addWidget(startLabel);
    ui->toolBar->addWidget(startDTFilter);
    ui->toolBar->addWidget(endLabel);
    ui->toolBar->addWidget(endDTFilter);

    QWidget *rightSpacer = new QWidget(this);
    rightSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    rightSpacer->setMinimumWidth(10);
    rightSpacer->setVisible(true);
    ui->toolBar->addWidget(rightSpacer);

    //model
    summedTableModel = new SummedTableModel(this);
    //charts
    summedChart = new SummedChart();
    summedChart->setModel(summedTableModel);
    ui->GraphLayout->addWidget(summedChart->getChartView());

    //QGeocoding
    QMap<QString,QVariant> params;
    params["osm.geocoding.host"] = "https://nominatim.openstreetmap.org";
    geoSrv = new QGeoServiceProvider( "osm" , params);
    geoCoder = geoSrv->geocodingManager();
    if (!geoCoder) {
        QMessageBox::warning(this,"Warning","Openstreetmap service not available!");
    }
    geoCoder->setLocale(QLocale(QLocale::English));
    QObject::connect(geoCoder, &QGeoCodingManager::error,this, &MainWindow::updateLocationInfoToUi);
    QObject::connect(geoCoder, &QGeoCodingManager::finished,this, &MainWindow::updateLocationInfoToUi);

    //QML stuff
    engine = new QQmlApplicationEngine(this);
    engine->load(QUrl(QStringLiteral("qrc:/main/GridFilter.qml")));
    engine->load(QUrl(QStringLiteral("qrc:/main/MapViewer.qml")));

    gridFilterItem = engine->rootObjects().at(0);
    mapViewerItem = engine->rootObjects().at(1);
    QQuickWindow *win = qobject_cast<QQuickWindow *>(gridFilterItem);
    QQuickWindow *mapWindow = qobject_cast<QQuickWindow *>(mapViewerItem);
    QWidget *container =  QWidget::createWindowContainer(win, this);
    QWidget *mapContainer =  QWidget::createWindowContainer(mapWindow, this);
    container->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    container->setMinimumSize(400,400);
    mapContainer->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    ui->gridFilterLayout->addWidget(container);
    ui->mapLayout->addWidget(mapContainer);

    //connect stuff
    //filters
    QObject::connect(gridFilterItem,SIGNAL(clickIndexSignal(int)),this,SLOT(updateGridFilter(int)));
    connect(startDTFilter,&QDateTimeEdit::dateTimeChanged,this,&MainWindow::updateStartFiler);
    connect(endDTFilter,&QDateTimeEdit::dateTimeChanged,this,&MainWindow::updateEndFilter);
    //variable infos
    connect(summedTableModel,&SummedTableModel::modelReset,this,&MainWindow::updateVariableInfo);
    //map selections
    QObject::connect(mapViewerItem,SIGNAL(selectStartSignal(int)),this,SLOT(updateGridInfo(int)));
    QObject::connect(mapViewerItem,SIGNAL(selectRouteSignal(int)),this,SLOT(updateRouteInfo(int)));
    //disable buttons until data is loaded
    enableGraphButtons(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//enable/disable all buttons (at least in tab1)
void MainWindow::enableGraphButtons(bool flag)
{
    startDTFilter->setEnabled(flag);
    endDTFilter->setEnabled(flag);
    gridFilterItem->setProperty("buttonsEnabled",flag);
    ui->startEndSelector->setEnabled(flag);
    ui->SelectAllButton->setEnabled(flag);
    ui->clearButton->setEnabled(flag);
    ui->selectGraphComboBox->setEnabled(flag);
    ui->tabWidget->setEnabled(flag);
}
//filter slots
//naming should be startFilterUpdated but it's too late now
void MainWindow::updateStartFiler(const QDateTime &datetime)
{
    endDTFilter->setMinimumDateTime(datetime);
    summedTableModel->updateStartFilter(datetime);
    summedChart->setXMin(datetime);

    timeFilterUpdated();
}

void MainWindow::updateEndFilter(const QDateTime &datetime)
{
    startDTFilter->setMaximumDateTime(datetime);
    summedTableModel->updateEndFilter(datetime.addSecs(3600));
    summedChart->setXMax(datetime);

    timeFilterUpdated();
}

//stuffs that need to be updated when either time filter is updated
void MainWindow::timeFilterUpdated()
{
    updateGridInfoToUi();

    //if data max is less than half of axis max, update it
    if (summedChart->getYMax()*2 < summedChart->getYAxisMax()) {
        summedChart->updateYAxis();
    }
    if (summedChart->getYMax() >= summedChart->getYAxisMax()) {
        summedChart->updateYAxis();
    }
    //map tab is selected, condition because the operation is more expensive
    if (ui->tabWidget->currentIndex() == 1) {
        if (ui->hMstartButton->isChecked()) {
            QMetaObject::invokeMethod(mapViewerItem, "loadHeatMap",
                                      Q_ARG(QVariant, QVariant::fromValue(summedTableModel->getHeatMap(true))));
        }
        if (ui->hMEndButton->isChecked()) {
            QMetaObject::invokeMethod(mapViewerItem, "loadHeatMap",
                                      Q_ARG(QVariant, QVariant::fromValue(summedTableModel->getHeatMap(false))));
        }
    }
}

void MainWindow::updateGridFilter(const int &gridID)
{
    enableGraphButtons(false);
    summedTableModel->updateLocationFilter(gridID);

    if (summedChart->getYMax()*2 < summedChart->getYAxisMax()) {
        summedChart->updateYAxis();
    }
    if (summedChart->getYMax() >= summedChart->getYAxisMax()) {
        summedChart->updateYAxis();
    }

    enableGraphButtons(true);
}

//called once when data is loaded
void MainWindow::updateConstantInfo()
{
    if (summedTableModel->rowCount() == 0) return;  //just in case
    if (theGrid == NULL) return;

    QString regionSizeText;
    QString totalTimeRangeText, daysCountText;
    QString totalRevenueText, perHourRevenueText;

    //call geoCoder, then handle the signal in updateLocationInfoToUi (connected in constructor)
    ui->locationLabel->setText("loading...");
    ui->mapLocLabel->setText("loading...");
    QGeoCoordinate center = theGrid->area.center();
    geoCoder->reverseGeocode(center);

    //calculate region area (approximate)
    double width = theGrid->area.topLeft().distanceTo(theGrid->area.topRight());
    double height = theGrid->area.topLeft().distanceTo(theGrid->area.bottomLeft());
    double width2 = theGrid->area.bottomLeft().distanceTo(theGrid->area.bottomRight());

    double area = 0.5 * ((width+width2)/1000) * (height/1000);
    regionSizeText = QString::number(area,'f',2) + "km" + QChar(178);

    //get total time range
    QDateTime startDate, endDate;
    startDate.setSecsSinceEpoch(summedTableModel->getStartTime());
    endDate.setSecsSinceEpoch(summedTableModel->getEndTime());
    int days = startDate.daysTo(endDate);

    totalTimeRangeText = startDate.toString("yyyy-MM-dd") + "   to   " +endDate.toString("yyyy-MM-dd");
    daysCountText = "(" + QString::number(days) + "days)  ";

    //calculate total and average revenue per hour
    //should be the last index of total revenue column as this function is only called
    //after file is initially loaded
    int row = summedTableModel->rowCount()-1;
    QModelIndex index = summedTableModel->index(row,4);
    double revenue = summedTableModel->data(index).toDouble();
    int hours = days*24;
    double avgRev = revenue / (double)hours;

    totalRevenueText = QChar(165) + QLocale(QLocale::English).toString(revenue,'f',2);
    perHourRevenueText = QChar(165) + QLocale(QLocale::English).toString(avgRev,'f',2);

    ui->sizeLabel->setText(regionSizeText);
    ui->totalTimeRangeLabel->setText(totalTimeRangeText);
    ui->daysCountLabel->setText(daysCountText);
    ui->daysCountLabel->setAlignment(Qt::AlignRight);
    ui->daysCountLabel->setAlignment(Qt::AlignVCenter);
    ui->TRLabel->setText(totalRevenueText);
    ui->PHRLabel->setText(perHourRevenueText);

    ui->mapSizeLabel->setText(regionSizeText);
    ui->mapTolTimeRangeLabel->setText(totalTimeRangeText);
    ui->mapDaysCountLabel->setText("    "+daysCountText);
    ui->mapDaysCountLabel->setAlignment(Qt::AlignRight);
    ui->mapDaysCountLabel->setAlignment(Qt::AlignVCenter);
}

//update location info after recieving reply
void MainWindow::updateLocationInfoToUi(QGeoCodeReply *reply)
{
    QString location = "";
    if (reply->error()) {
        qDebug() << reply->errorString();
        location = "error";
    } else if (reply->locations().size() == 0) {
        qDebug() << "empty reply";
        location = "???";
    } else {
        QString country = reply->locations()[0].address().country();
        QString city = reply->locations()[0].address().city();
        location.append(city);
        location.append(", ");
        location.append(country);
    }
    ui->locationLabel->setText(location);
    ui->mapLocLabel->setText(location);
    QString windowTitle = "Online Ride-Hailing Order Data Analyser @" + location;
    setWindowTitle(windowTitle);

    reply->deleteLater();
}

//info that needs to be updated acooridng to filter
void MainWindow::updateVariableInfo()
{
    QString sTimeRangeText;
    QString sTotalRevenueText, sPerHourRevenueText;

    QDateTime startTime = startDTFilter->dateTime();
    QDateTime endTime = endDTFilter->dateTime();

    sTimeRangeText = startTime.toString("yyyy-MM-dd hh:mm") + "   to   " +endTime.toString("yyyy-MM-dd hh:mm");

    //get total revenue and calculate average revenue per hour
    int row = summedTableModel->rowCount()-1;
    QModelIndex index = summedTableModel->index(row,4);
    double revenue = summedTableModel->data(index).toDouble();
    int hours = startTime.secsTo(endTime)/3600;
    double avgRev = revenue / (double)hours;

    sTotalRevenueText = QChar(165) + QLocale(QLocale::English).toString(revenue,'f',2);
    sPerHourRevenueText = QChar(165) + QLocale(QLocale::English).toString(avgRev,'f',2);

    ui->selectedTimeRangeLabel->setText(sTimeRangeText);
    ui->mapSelTimeLabel->setText(sTimeRangeText);
    ui->TRSelectedLabel->setText(sTotalRevenueText);
    ui->PHRSelectedLabel->setText(sPerHourRevenueText);
}

//below is for tab2 (map), called from QML whenever a grid is selected
void MainWindow::updateGridInfo(const int &gridID)
{
    selectedID = gridID;
    routeMode = false;
    updateGridInfoToUi();
    if (gridID != -1)
    {
        QMetaObject::invokeMethod(mapViewerItem, "loadHeatMap",
                                  Q_ARG(QVariant, QVariant::fromValue(summedTableModel->getDestinationsHeatMap(gridID))),
                                  Q_ARG(QVariant, "green"));
    }
}

void MainWindow::updateGridInfoToUi()
{
    if (selectedID == -1) {
        ui->gridLabel1->setText("Orders from grid: ");
        ui->gridLabel2->setText("Orders to grid: ");
        ui->gridLabel3->setText("");
        ui->routePriceLabel->setText("");
        ui->sGridIDLabel->setText("");
        ui->sGridStartOrdersLabel->setText("");
        ui->sGridEndOrdersLabel->setText("");
    } else if (!routeMode) {
        int startDemand = summedTableModel->getDemand(selectedID);
        int endDemand = summedTableModel->getDemand(selectedID+100);
        ui->gridLabel1->setText("Orders from grid: ");
        ui->gridLabel2->setText("Orders to grid: ");
        ui->gridLabel3->setText("");
        ui->routePriceLabel->setText("");
        ui->sGridIDLabel->setText(QString::number(selectedID));
        ui->sGridStartOrdersLabel->setText(QString::number(startDemand));
        ui->sGridEndOrdersLabel->setText(QString::number(endDemand));
        ui->hMNoneButton->toggle();
        on_hMNoneButton_clicked();
    }
}

void MainWindow::updateRouteInfo(const int &gridID)
{
    if (selectedID == -1) {
        qDebug() << "Error: no previous selected id.";
    }
    routeMode = true;

    int demand = summedTableModel->getRouteRemand(selectedID,gridID);
    QTime time = summedTableModel->getRouteAverageTime(selectedID,gridID);
    double price = summedTableModel->getRouteAveragePrice(selectedID,gridID);

    ui->gridLabel1->setText("Orders: ");
    ui->gridLabel2->setText("Average travel time: ");
    ui->gridLabel3->setText("Average fees: ");
    ui->sGridIDLabel->setText(QString::number(selectedID)+" to "+QString::number(gridID));
    ui->sGridStartOrdersLabel->setText(QString::number(demand));
    ui->sGridEndOrdersLabel->setText(time.toString("hh:mm:ss"));
    ui->routePriceLabel->setText(QChar(165)+QLocale(QLocale::English).toString(price,'f',2));
}

//MappedReduce functions for getSummedData
QHash<QString, QList<int> > mapTable(const QString &fileName)
{
    //initialize QHash
    QHash<QString, QList<int> > data;
    //Should have less than 15days*24hours*200grid ids
    data.reserve(80000);

    //open file
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);

    //read file
    QTextStream in(&file);             
    QString line = in.readLine();    //skip firstline
    while (!line.isNull())
    {
        line = in.readLine();
        QList<QString> str = line.split(",");
        if (str.size() < 8) break;

        //generate key
        QString keyStart, keyEnd;       //2 entries for start grid and end grid

        //grid id part
        //to mark start grid: 0 - 99    end grid: 100-199
        int xStart = (str[3].toDouble() - theGrid->lng) / theGrid->lngGap;
        int yStart = (theGrid->lat - str[4].toDouble()) / theGrid->latGap;
        int xEnd = (str[5].toDouble() - theGrid->lng) / theGrid->lngGap;
        int yEnd = (theGrid->lat - str[6].toDouble()) / theGrid->latGap;
        int gridStart = 10*yStart + xStart;
        int gridEnd = 100 + 10*yEnd + xEnd;

        if (gridStart < 0 || gridStart >= 100 || gridEnd < 100 || gridEnd >= 200){
            continue;
        }

        keyStart = QString("%1").arg(gridStart,3,10,QChar('0'));
        keyEnd = QString("%1").arg(gridEnd,3,10,QChar('0'));

        //time stamp part
        int startTime = str[1].toInt();
        startTime = (startTime/3600)*3600;       //round down to nearest hour
        keyStart.append(QString("%1").arg(startTime));
        keyEnd.append(QString("%1").arg(startTime));

        //get summed data
        QList<int> row;
        //demand (count)
        row.append(1);
        //total time taken - to be converted to average time taken later
        int timeTaken = str[2].toInt() - str[1].toInt();
        row.append(timeTaken);
        //total fee - stored in cents
        int fee = str[7].toDouble()*100;
        row.append(fee);

        //if key already exist, sum it. Otherwise insert new
        if (data.contains(keyStart))
        {
            for (int i = 0; i < 3; i++)
            {
                data[keyStart][i] += row[i];
            }
        } else {
            data.insert(keyStart,row);
        }
        if (data.contains(keyEnd))
        {
            for (int i = 0; i < 3; i++)
            {
                data[keyEnd][i] += row[i];
            }
        } else data.insert(keyEnd,row);

        //gridID destination part
        //this store gridID key - 100 of ( destination orders - total time taken - total revenue )
        QString gridIDKey = QString("%1").arg(gridStart,3,10,QChar('0'));
        //if key already exist, sum it. Otherwise insert new
        if (data.contains(gridIDKey))
        {
            data[gridIDKey][gridEnd-100]++;
            data[gridIDKey][gridEnd] += timeTaken;
            data[gridIDKey][gridEnd+100] += fee;
        } else {
            QList<int> row2;
            for (int i = 0; i < 300; i++) row2.append(0);
            row2[gridEnd-100]++;
            row2[gridEnd] += timeTaken;
            row2[gridEnd+100] += fee;
            data.insert(gridIDKey,row2);
        }
    }

    data.squeeze();

    return data;
}

void reduceTable(QHash<QString, QList<int> > &result, const QHash<QString, QList<int> > data)
{
    QHash<QString, QList<int> >::const_iterator i = data.constBegin();
    while (i != data.constEnd())
    {
        if (result.contains(i.key()))
        {
            for (int j = 0; j < i.value().size(); j++)
            {
                result[i.key()][j] += i.value()[j];
            }
        } else {
            result.insert(i.key(),i.value());
        }
        ++i;
    }
}

void MainWindow::on_actionOpen_triggered()
{  
    //Open directory
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), "");
    if (dir.isEmpty() || dir.isNull()) return;

    //iterate all csv files and generate list of filenames
    QDirIterator it(dir, QStringList() << "*.csv", QDir::NoFilter, QDirIterator::Subdirectories);
    QList <QString> fileNames;
    while (it.hasNext())
    {
        QFile file(it.next());
        if (file.fileName().contains("order"))
            fileNames.append(file.fileName());
        //read grid data
        if (file.fileName().contains("grid"))
        {
            //open file
            file.open(QIODevice::ReadOnly);

            //read file
            QTextStream in(&file);                   
            QString line = in.readLine();   //skip firstline
            line = in.readLine();
            QList<QString> str = line.split(",");
            if (str.size() == 9)
            {
                double lng = str[7].toDouble();
                double lat = str[8].toDouble();
                double lngGap = str[3].toDouble()-lng;
                double latGap = str[4].toDouble()-lat;
                theGrid = new grid(lng,lat,lngGap,latGap);
            }
        }
    }

    if (fileNames.isEmpty())
    {
        QMessageBox::warning(this,"Warning","No valid data in selected folder");
        return;
    }
    if (theGrid == NULL)
    {
        QMessageBox::warning(this,"Warning","No valid grid data in selected folder");
        return;
    }

    QElapsedTimer timer;
    timer.start();

    //load coordinates to mapFilter and map
    gridFilterItem->setProperty("region",QVariant::fromValue(theGrid->area));
    QMetaObject::invokeMethod(gridFilterItem, "loadMap");

    mapViewerItem->setProperty("region",QVariant::fromValue(theGrid->area));
    mapViewerItem->setProperty("lngGap",QVariant::fromValue(theGrid->lngGap));
    mapViewerItem->setProperty("latGap",QVariant::fromValue(theGrid->latGap));

    //disable filters
    enableGraphButtons(false);

    //assuming it isn't sorted
    std::sort(fileNames.begin(),fileNames.end());

    //load the data and sum it
    QHash<QString, QList<int> > table;

    //get first and last date for filter
    QString fF(QFileInfo(fileNames.first()).fileName());
    QString lF(QFileInfo(fileNames.last()).fileName());
    QDate firstDate(fF.mid(6,4).toInt(),fF.mid(10,2).toInt(),fF.mid(12,2).toInt());
    QDate lastDate(lF.mid(6,4).toInt(),lF.mid(10,2).toInt(),lF.mid(12,2).toInt());

    int startTime = firstDate.startOfDay().toSecsSinceEpoch();
    int endTime = lastDate.addDays(1).startOfDay().toSecsSinceEpoch();

    //Progress dialog
    QProgressDialog pDialog;
    pDialog.setLabelText("Loading files");
    pDialog.setWindowTitle("TT");

    //Load data onto QHash using mappedReduced
    QFutureWatcher<QHash<QString, QList<int> >> watcher;
    QFuture<QHash<QString, QList<int> > > future = QtConcurrent::mappedReduced(fileNames,mapTable,reduceTable);
    watcher.setFuture(future);

    connect(&pDialog, &QProgressDialog::canceled, &watcher, &QFutureWatcher<QHash<QString, QList<int> >>::cancel);
    connect(&watcher, &QFutureWatcher<QHash<QString, QList<int> >>::finished, &pDialog, &QProgressDialog::reset);
    connect(&watcher, &QFutureWatcher<QHash<QString, QList<int> >>::progressRangeChanged, &pDialog, &QProgressDialog::setRange);
    connect(&watcher, &QFutureWatcher<QHash<QString, QList<int> >>::progressValueChanged, &pDialog, &QProgressDialog::setValue);

    pDialog.exec();
    watcher.waitForFinished();

    if (watcher.isCanceled())
    {
        qDebug() << "Canceled!";
        return;
    }
    table = future.result();

    //add date range info into table for summedTableModel
    QList<int> dateRange = {startTime, endTime};
    table.insert("dateRange",dateRange);
    qDebug() << "generating summed data took " << timer.elapsed() << "ms";

    summedTableModel->setTable(table);

    //update info
    updateConstantInfo();

    //set filters
    startDTFilter->setDateTimeRange(firstDate.startOfDay(),lastDate.endOfDay().addSecs(-3599));
    endDTFilter->setDateTimeRange(firstDate.startOfDay(),lastDate.endOfDay().addSecs(-3599));
    startDTFilter->setDateTime(firstDate.startOfDay());
    endDTFilter->setDateTime(lastDate.addDays(1).startOfDay());

    enableGraphButtons(true);

    //charts
    summedChart->setXMax(lastDate.endOfDay().addSecs(-3599));
    summedChart->setXMin(firstDate.startOfDay());
    summedChart->swapToGraph(0);

    //map
    QMetaObject::invokeMethod(mapViewerItem, "loadMap");
}

void MainWindow::on_actionview_table_triggered()
{
//    nw = new TableDialog(this,summedTableModel);
//    nw->show();
}

void MainWindow::on_startEndSelector_currentIndexChanged(int index)
{
    enableGraphButtons(false);

    summedTableModel->flipLocationFilter(!index);

    enableGraphButtons(true);
}

void MainWindow::on_SelectAllButton_clicked()
{
    enableGraphButtons(false);

    QMetaObject::invokeMethod(gridFilterItem,"checkAll");
    summedTableModel->fillLocationFilter();
    summedChart->updateYAxis();

    enableGraphButtons(true);
}

void MainWindow::on_clearButton_clicked()
{
    enableGraphButtons(false);

    QMetaObject::invokeMethod(gridFilterItem,"unCheckAll");
    summedTableModel->emptyLocationFilter();
    summedChart->updateYAxis();

    enableGraphButtons(true);
}

void MainWindow::on_selectGraphComboBox_currentIndexChanged(int index)
{
    if (!summedChart) return;
    summedChart->swapToGraph(index);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1)
    {
//        QMetaObject::invokeMethod(mapViewerItem, "loadMap");
//        QMetaObject::invokeMethod(mapViewerItem, "loadHeatMap",
//                                  Q_ARG(QVariant, QVariant::fromValue(summedTableModel->getHeatMap(true))));
    }
}

void MainWindow::on_hMstartButton_clicked()
{
    QMetaObject::invokeMethod(mapViewerItem, "loadHeatMap",
                              Q_ARG(QVariant, QVariant::fromValue(summedTableModel->getHeatMap(true))),
                              Q_ARG(QVariant, "red"));
}

void MainWindow::on_hMEndButton_clicked()
{
    QMetaObject::invokeMethod(mapViewerItem, "loadHeatMap",
                              Q_ARG(QVariant, QVariant::fromValue(summedTableModel->getHeatMap(false))),
                              Q_ARG(QVariant, "red"));
}

void MainWindow::on_hMNoneButton_clicked()
{
    QMetaObject::invokeMethod(mapViewerItem, "unLoadHeatMap");
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chartview.h"
#include "lineseries.h"
#include <QDebug>

#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QFileDialog>
#include <QMessageBox>

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    dev_label(new ClickableLabel),
    serial_port_label(new QLabel),
    etat_label(new QLabel),
    deltax_label(new QLabel),
    serial(new QSerialPort),
    ui(new Ui::MainWindow)
{
    timer = new QTimer();
    nbr=0;
    marche_bool=0;
    chart_update = new QTimer();
    connect(timer, SIGNAL(timeout()), this,SLOT(serial_timeout()));
    timer->stop();
    connect(chart_update,SIGNAL(timeout()),this,SLOT(append_point()));
    chart_update->stop();

    rcv_serie.nb_char=0;
    rcv_serie.rcv_ok=false;
    curseur_bt_d=false;
    curseur_bt_g=false;
    ui->setupUi(this);

    dev_label->setAlignment(Qt::AlignLeft);
    dev_label->setFixedWidth(150);
    connect(dev_label,SIGNAL(clicked()),this,SLOT(on_dev_label_clicked()));

    serial_port_label->setAlignment(Qt::AlignCenter);
    serial_port_label->setIndent(25);

    etat_label->setAlignment(Qt::AlignCenter);
    etat_label->setIndent(25);

    deltax_label->setAlignment(Qt::AlignRight);
    deltax_label->setIndent(25);
    deltax_label->setFixedWidth(150);

    ui->statusBar->addWidget(dev_label);
    ui->statusBar->addWidget(serial_port_label, 1);
    ui->statusBar->addWidget(etat_label,2);
    ui->statusBar->addWidget(deltax_label,3);
    etat_label->setText("Système Arrêté");
    dev_label->setText("Réalisé par Enise - XG");
    serial_port_label->setText("Port série: "+serial->portName());
    deltax_label->setText("Delta X:");

    ui->cu_lb_y1->show();
    ui->cu_lb_y2->hide();
    ui->cu_lb_y3->hide();
    ui->cu_lb_y4->hide();
    ui->cu_lb_y5->hide();
    ui->cu_lb_y6->hide();
    ui->cu1_lb_y1->show();
    ui->cu1_lb_y2->hide();
    ui->cu1_lb_y3->hide();
    ui->cu1_lb_y4->hide();
    ui->cu1_lb_y5->hide();
    ui->cu1_lb_y6->hide();
    // initialisation composant pour le graphique et le graphique
    serie_consigne = new LineSeries();
    serie_erreur = new LineSeries();
    serie_spid1 = new LineSeries();
    serie_spid2 = new LineSeries();
    serie_val1 = new LineSeries();
    serie_val2 = new LineSeries();
    serie_consigne->setUseOpenGL(true);
    serie_erreur->setUseOpenGL(true);
    serie_spid1->setUseOpenGL(true);
    serie_spid2->setUseOpenGL(true);
    serie_val1->setUseOpenGL(true);
    serie_val2->setUseOpenGL(true);

  /*  serie_consigne->connect(serie_consigne, SIGNAL(pointAdded(int)),this,SLOT(pt_consigne_added(int)));
    serie_erreur->connect(serie_erreur, SIGNAL(pointAdded(int)),this,SLOT(pt_erreur_added(int)));
    serie_spid1->connect(serie_spid1, SIGNAL(pointAdded(int)),this,SLOT(pt_spid1_added(int)));
    serie_spid2->connect(serie_spid2, SIGNAL(pointAdded(int)),this,SLOT(pt_spid2_added(int)));
    serie_val1->connect(serie_val1, SIGNAL(pointAdded(int)),this,SLOT(pt_val1_added(int))); */
    serie_val2->connect(serie_val2, SIGNAL(pointAdded(int)),this,SLOT(pt_added(int)));

    serie_consigne->setPen(QPen(COLOR_CONSIGNE)); //,2, Qt::DotLine,Qt::SquareCap,Qt::BevelJoin));
    serie_erreur->setPen(QPen(COLOR_ERREUR));
    serie_spid1->setPen(QPen(COLOR_SPID1));
    serie_spid2->setPen(QPen(COLOR_SPID2));
    serie_val1->setPen(QPen(COLOR_VAL1));
    serie_val2->setPen(QPen(COLOR_VAL2));
    axis_consigne = new QValueAxis;
    axis_consigne->setLinePenColor(COLOR_CONSIGNE);
    axis_erreur = new QValueAxis;
    axis_erreur->setLinePenColor(COLOR_ERREUR);
    axis_spid1 = new QValueAxis;
    axis_spid1->setLinePenColor(COLOR_SPID1);
    axis_spid2 = new QValueAxis;
    axis_spid2->setLinePenColor(COLOR_SPID2);
    axis_val1 = new QValueAxis;
    axis_val1->setLinePenColor(COLOR_VAL1);
    axis_val2 = new QValueAxis;
    axis_val2->setLinePenColor(COLOR_VAL2);
    axe_temps = new QValueAxis;
    axe_temps->setRange(0,3);
    axe_temps->setLabelFormat("%.3G");

    chart = new QChart();
    chart->legend()->hide();
    chart->createDefaultAxes();

    chart->addSeries(serie_consigne);
    chart->addSeries(serie_erreur);
    chart->addSeries(serie_spid1);
    chart->addSeries(serie_spid2);
    chart->addSeries(serie_val1);
    chart->addSeries(serie_val2);

    chart->addAxis(axis_consigne, Qt::AlignLeft);
    chart->addAxis(axis_erreur, Qt::AlignLeft);
    chart->addAxis(axis_spid1, Qt::AlignLeft);
    chart->addAxis(axis_spid2, Qt::AlignLeft);
    chart->addAxis(axis_val1, Qt::AlignLeft);
    chart->addAxis(axis_val2, Qt::AlignLeft);

    serie_consigne->attachAxis(axis_consigne);
    serie_erreur->attachAxis(axis_erreur);
    serie_spid1->attachAxis(axis_spid1);
    serie_spid2->attachAxis(axis_spid2);
    serie_val1->attachAxis(axis_val1);
    serie_val2->attachAxis(axis_val2);

    serie_consigne->show();
    axis_consigne->show();
    serie_erreur->hide();
    axis_erreur->hide();
    serie_spid1->hide();
    axis_spid1->hide();
    serie_spid2->hide();
    axis_spid2->hide();
    serie_val1->hide();
    axis_val1->hide();
    serie_val2->hide();
    axis_val2->hide();
    chart->setAxisX(axe_temps,serie_consigne);
    chart->setAxisX(axe_temps,serie_erreur);
    chart->setAxisX(axe_temps,serie_spid1);
    chart->setAxisX(axe_temps,serie_spid2);
    chart->setAxisX(axe_temps,serie_val1);
    chart->setAxisX(axe_temps,serie_val2);

    axis_consigne->setMinorTickCount(2);
    axis_erreur->setMinorTickCount(2);
    axis_spid1->setMinorTickCount(2);
    axis_spid2->setMinorTickCount(2);
    axis_val1->setMinorTickCount(2);
    axis_val2->setMinorTickCount(2);

    axe_temps->setTickCount(10);
    axe_temps->setMinorTickCount(2);

    chart->setObjectName("Chart_xg");
    chart->setMargins(QMargins(0,0,0,0));
    chartview = new ChartView(chart,this);

    chartview->connect(chartview,SIGNAL(zoom_change()),this,SLOT(replot_zoom()));
    chartview->connect(chartview,SIGNAL(scroll_sig()),this,SLOT(replot_scroll()));
    chartview->connect(chartview,SIGNAL(rubberBandChanged(QRect,QPointF,QPointF)),this,SLOT(replot_rubber(QRect,QPointF,QPointF)));
    chartview->connect(chart,SIGNAL(plotAreaChanged(QRectF)),this,SLOT(replot_area(QRectF)));
    chartview->connect(chartview,SIGNAL(zoom_reset()),this,SLOT(replot_zoomreset()));
    chartview->connect(chartview,SIGNAL(efface()),this,SLOT(clear_series()));
    chartview->connect(chartview,SIGNAL(souris_click_gauche(int)),this,SLOT(cliick(int)));

    ui->Chart_layout->addWidget(chartview);

    curseur_g = new QLineSeries();
    curseur_g->setPen(QPen(QColor(173, 127, 168), 1, Qt::DashLine));
    curseur_g->setUseOpenGL(true);
    chart->addSeries(curseur_g);
    curseur_g->attachAxis(axis_consigne);
    curseur_g->setObjectName("curseur_g");
    chart->setAxisX(axe_temps,curseur_g);

    //click courbe peu precis par rapport curseur j'enleve
    /*   serie_consigne->connect(serie_consigne,SIGNAL(pressed(QPointF)),this,SLOT(test(QPointF)));
    serie_erreur->connect(serie_erreur,SIGNAL(pressed(QPointF)),this,SLOT(test(QPointF)));
    serie_spid1->connect(serie_spid1,SIGNAL(pressed(QPointF)),this,SLOT(test(QPointF)));
    serie_spid2->connect(serie_spid2,SIGNAL(pressed(QPointF)),this,SLOT(test(QPointF)));
    serie_val1->connect(serie_val1,SIGNAL(pressed(QPointF)),this,SLOT(test(QPointF)));
    serie_val2->connect(serie_val2,SIGNAL(pressed(QPointF)),this,SLOT(test(QPointF)));
*/
    curseur_d = new QLineSeries();
    curseur_d->setPen(QPen(QColor(0, 0, 0), 1, Qt::DashLine));
    curseur_d->setUseOpenGL(true);
    chart->addSeries(curseur_d);
    curseur_d->attachAxis(axis_consigne);
    chart->setAxisX(axe_temps,curseur_d);

    on_combobox_mode_fct_currentIndexChanged(0);
    ui->aff_cb_consigne->setChecked(true);
}

void MainWindow::test(QPointF pt)
{
    pt_clicked=pt;
    QMessageBox::about(this,"Point Clické:",
                       "X: " + QString().number(pt.x()) + " <br>"
                                                          "Y: " + QString().number(pt.y()) + " <br>");
}

QPointF MainWindow::extrapol(qreal x, QLineSeries *serie)
{
    qreal tmpx,tmpx1,tmpy,tmpy1,y;
    int i=0;

    for(tmpx=0;(tmpx<x)&&(i<serie->count());)
    {
        tmpx=serie->at(i).x();
        i++;
    }
    tmpx=serie->at(i-2).x();
    tmpy=serie->at(i-2).y();
    tmpx1=serie->at(i-1).x();
    tmpy1=serie->at(i-1).y();
    y=((tmpy1-tmpy)/(tmpx1-tmpx))*(x-tmpx)+tmpy;
    return QPointF(x,y);
}
void MainWindow::clear_series()
{
    ymax_consigne=0;
    ymin_consigne=100000;
    ymax_erreur=0;
    ymin_erreur=100000;
    ymax_spid1=0;
    ymin_spid1=100000;
    ymax_spid2=0;
    ymin_spid2=100000;
    ymax_val1=0;
    ymin_val1=100000;
    ymax_val2=0;
    ymin_val2=100000;
    compteur=0;
    nbr=0;
    serie_consigne->clear();
    serie_erreur->clear();
    serie_spid1->clear();
    serie_spid2->clear();
    serie_val1->clear();
    serie_val2->clear();
    axe_temps->setRange(0,3);
    axis_consigne->setRange(0,1);

    curseur_d->clear();
    curseur_g->clear();
    txt_curs1.hide();
    txt_curs2.hide();
}

void MainWindow::replot_zoom()
{
    QPointF zm,z;
    QRectF zoom_area,plot_area;

    chart_update->stop();
    plot_area=chart->plotArea();
    z=chartview->mouse_point;
    zm=chartview->mouse_reel;
    if(chartview->ymouse==1)
    {
        zoom_area.setX(zm.x()-plot_area.width()/4);
        zoom_area.setY(zm.y()-plot_area.height()/4);
        zoom_area.setWidth(plot_area.width()/2);
        zoom_area.setHeight(plot_area.height()/2);
        if(zoom_area.x()<0)
        {
            zoom_area.setX(chart->mapToPosition(QPointF(0,0)).x());
        }
        if(z.x()<(3/1000))
        {
            zoom_area.setX(chart->mapToPosition(QPointF(0,0)).x());
        }
        if((chart->mapToValue(zoom_area.topRight()).x()-chart->mapToValue(zoom_area.topLeft()).x())>(3/1000))
        {
            chart->zoomIn(zoom_area);
            chartview->nb_zoom++;
            //chart->zoomIn();
        }
    }
    else if(chartview->ymouse==0)
    {
        zoom_area.setX(zm.x()-1*plot_area.width());
        zoom_area.setY(zm.y()-1*plot_area.height());
        zoom_area.setWidth(plot_area.width()*2);
        zoom_area.setHeight(plot_area.height()*2);

        if((chart->mapToValue(zoom_area.topRight()).x()-chart->mapToValue(zoom_area.topLeft()).x())>(3/1000))
        {
            if(chartview->nb_zoom>0)
            {
                chart->zoomIn(zoom_area);
                chartview->nb_zoom--;
            }
            else
            {
                chart->zoomReset();
                chart_update->start();
            }
        }
    }
    else
    {}
    if(axe_temps->min()<0)
    {
        zoom_area.setX(chart->mapToPosition(QPointF(0,0)).x());
        chart->zoomIn(zoom_area);
    }
    replot_cursor();
}

void MainWindow::replot_scroll()
{
    if(chartview->scroll=='U')
    {
        chart->scroll(0,(ymax_consigne-ymin_consigne)/100);
    }
    if(chartview->scroll=='D')
    {
        chart->scroll(0,-(ymax_consigne-ymin_consigne)/100);
    }
    if(chartview->scroll=='L')
    {
        qreal axe_max;
        axe_max=axe_temps->max();
        chart->scroll(-(axe_temps->max()-axe_temps->min())/10,0);
        if(axe_temps->min()<0)
        {
            axe_temps->setMin(0);
            axe_temps->setMax(axe_max);
        }
    }
    if(chartview->scroll=='R')
    {
        chart->scroll((axe_temps->max()-axe_temps->min())/10,0);
    }
    replot_cursor();
}

void MainWindow::replot_cursor()
{
    if ((curseur_g->count()<1)&&(curseur_d->count()<1))
    {}
    else
    {
        pos_curs1=chart->mapToPosition(QPointF(curseur_g->at(1).x(),axis_consigne->max()),curseur_g);
        txt_curs1.setPos(pos_curs1);

        curseur_g->replace(curseur_g->at(1).x(),curseur_g->at(1).y(),curseur_g->at(1).x(),axis_consigne->min());
        curseur_g->replace(curseur_g->at(0).x(),curseur_g->at(0).y(),curseur_g->at(0).x(),axis_consigne->max());

        pos_curs2=chart->mapToPosition(QPointF(curseur_d->at(1).x(),axis_consigne->max()),curseur_d);
        txt_curs2.setPos(pos_curs2);

        curseur_d->replace(curseur_d->at(1).x(),curseur_d->at(1).y(),curseur_d->at(1).x(),axis_consigne->min());
        curseur_d->replace(curseur_d->at(0).x(),curseur_d->at(0).y(),curseur_d->at(0).x(),axis_consigne->max());
    }
}

void MainWindow::replot_zoomreset()
{
    chart_update->start();
    replot_cursor();
    chartview->nb_zoom=0;
}

void MainWindow::replot_area(QRectF rect)
{
    replot_cursor();
    rect.isValid();
}

void MainWindow::replot_rubber(QRect rect, QPointF pos1, QPointF pos2)
{
    QPointF posc1,posc2;
    if(rect.isNull())
    {
        posc1=chart->mapToValue(chartview->pos_sel1);
        posc2=chart->mapToValue(chartview->pos_sel2);
        if((abs(posc1.x()-posc2.x())>0.003) && (posc1.x()>0) && (posc2.x()>0))
        {
            chart->zoomIn(chartview->selection);
        }
        replot_cursor();
    }
    chartview->selection=rect;
    chartview->pos_sel1=pos1;
    chartview->pos_sel2=pos2;
    chart_update->stop();
}

void MainWindow::cliick(int clk)
{
    qreal x;
    QPointF pt;
    QString a;
    x=chartview->mouse_point.x();

    if(clk==0)
    {
        if(curseur_bt_g==0)
        {
            ui->cu_lb_x->setText("X1: "+a.number(x,'G'));
            if(serie_consigne->count()==0)
            {}
            else
            {
                curseur_bt_g=1;
                curseur_g->append(x,ymin_consigne);
                curseur_g->append(x,ymax_consigne);
                pos_curs1=chart->mapToPosition(QPointF(curseur_g->at(1).x(),axis_consigne->max()),curseur_g);
                txt_curs1.setPos(pos_curs1);
                txt_curs1.setPen(QPen(QColor(173, 127, 168)));
                txt_curs1.setText("1");
                txt_curs1.show();
                chartview->scene()->addItem(&txt_curs1);
            }
            if(serie_consigne->count()>2)
            {
                pt=this->extrapol(x,serie_consigne);
                ui->cu_lb_y1->setText(a.number(pt.y(),'G',4));
                if(ui->aff_cb_consigne->isChecked())
                    ui->cu_lb_y1->show();
            }
            else
            {
                ui->cu_lb_y1->hide();
            }
            if((serie_erreur->count()>2)&&(ui->aff_cb_erreur->isChecked()))
            {
                pt=this->extrapol(x,serie_erreur);
                ui->cu_lb_y2->setText(a.number(pt.y(),'G',4));
                ui->cu_lb_y2->show();
            }
            else
            {
                ui->cu_lb_y2->hide();
            }
            if((serie_spid1->count()>2)&&(ui->aff_cb_pid1->isChecked()))
            {
                pt=this->extrapol(x,serie_spid1);
                ui->cu_lb_y3->setText(a.number(pt.y(),'G',4));
                ui->cu_lb_y3->show();
            }
            else
            {
                ui->cu_lb_y3->hide();
            }
            if((serie_spid2->count()>2)&&(ui->aff_cb_pid2->isChecked()))
            {
                pt=this->extrapol(x,serie_spid2);
                ui->cu_lb_y4->setText(a.number(pt.y(),'G',4));
                ui->cu_lb_y4->show();
            }
            else
            {
                ui->cu_lb_y4->hide();
            }
            if((serie_val1->count()>2)&&(ui->aff_cb_val1->isChecked()))
            {
                pt=this->extrapol(x,serie_val1);
                ui->cu_lb_y5->setText(a.number(pt.y(),'G',4));
                ui->cu_lb_y5->show();
            }
            else
            {
                ui->cu_lb_y5->hide();
            }
            if((serie_val2->count()>2)&&(ui->aff_cb_val2->isChecked()))
            {
                pt=this->extrapol(x,serie_val2);
                ui->cu_lb_y6->setText(a.number(pt.y(),'G',4));
                ui->cu_lb_y6->show();
            }
            else
            {
                ui->cu_lb_y6->hide();
            }
        }
        else
        {
            chartview->scene()->removeItem(&txt_curs1);
            ui->cu_lb_x->setText("X1: ");
            curseur_bt_g=0;
            curseur_g->clear();
            ui->cu_lb_y1->hide();
            ui->cu_lb_y2->hide();
            ui->cu_lb_y3->hide();
            ui->cu_lb_y4->hide();
            ui->cu_lb_y5->hide();
            ui->cu_lb_y6->hide();
        }
    }
    else if (clk==1)
    {

        if(curseur_bt_d==0)
        {
            ui->cu1_lb_x->setText("X2 : "+a.number(x,'G'));
            if(serie_consigne->count()==0)
            {}
            else
            {
                curseur_bt_d=1;
                curseur_d->append(x,ymin_consigne);
                curseur_d->append(x,ymax_consigne);
                pos_curs2=chart->mapToPosition(curseur_d->at(1),curseur_d);
                txt_curs2.setPos(pos_curs2);
                txt_curs2.setPen(QPen(QColor(173, 127, 168)));
                txt_curs2.setText("2");
                txt_curs2.show();
                chartview->scene()->addItem(&txt_curs2);
            }
            if(serie_consigne->count()>2)
            {
                pt=this->extrapol(x,serie_consigne);
                ui->cu1_lb_y1->setText(a.number(pt.y(),'G',4));
                if(ui->aff_cb_consigne->isChecked())
                    ui->cu1_lb_y1->show();
            }
            else
            {
                ui->cu1_lb_y1->hide();
            }
            if((serie_erreur->count()>2)&&(ui->aff_cb_erreur->isChecked()))
            {
                pt=this->extrapol(x,serie_erreur);
                ui->cu1_lb_y2->setText(a.number(pt.y(),'G',4));
                ui->cu1_lb_y2->show();
            }
            else
            {
                ui->cu1_lb_y2->hide();
            }
            if((serie_spid1->count()>2)&&(ui->aff_cb_pid1->isChecked()))
            {
                pt=this->extrapol(x,serie_spid1);
                ui->cu1_lb_y3->setText(a.number(pt.y(),'G',4));
                ui->cu1_lb_y3->show();
            }
            else
            {
                ui->cu1_lb_y3->hide();
            }
            if((serie_spid2->count()>2)&&(ui->aff_cb_pid2->isChecked()))
            {
                pt=this->extrapol(x,serie_spid2);
                ui->cu1_lb_y4->setText(a.number(pt.y(),'G',4));
                ui->cu1_lb_y4->show();
            }
            else
            {
                ui->cu1_lb_y4->hide();
            }
            if((serie_val1->count()>2)&&(ui->aff_cb_val1->isChecked()))
            {
                pt=this->extrapol(x,serie_val1);
                ui->cu1_lb_y5->setText(a.number(pt.y(),'G',4));
                ui->cu1_lb_y5->show();
            }
            else
            {
                ui->cu1_lb_y5->hide();
            }
            if((serie_val2->count()>2)&&(ui->aff_cb_val2->isChecked()))
            {
                pt=this->extrapol(x,serie_val2);
                ui->cu1_lb_y6->setText(a.number(pt.y(),'G',4));
                ui->cu1_lb_y6->show();
            }
            else
            {
                ui->cu1_lb_y6->hide();
            }
        }
        else
        {
            chartview->scene()->removeItem(&txt_curs2);
            ui->cu1_lb_x->setText("X2: ");
            curseur_bt_d=0;
            curseur_d->clear();
            ui->cu1_lb_y1->hide();
            ui->cu1_lb_y2->hide();
            ui->cu1_lb_y3->hide();
            ui->cu1_lb_y4->hide();
            ui->cu1_lb_y5->hide();
            ui->cu1_lb_y6->hide();
        }
    }
    replot_cursor();
    if((curseur_d->count()>1)&&(curseur_g->count()>1))
    {
        deltax_label->setText("Delta X:"+QString().number(abs(curseur_d->at(0).x()-curseur_g->at(0).x()),'G'));
    }
    else
    {
        deltax_label->setText("Delta X:");
    }
}

void MainWindow::pt_added(int index)
{
    qreal y = serie_consigne->at(index).y();
    if(y< ymin_consigne || y > ymax_consigne)
    {
        if(y < ymin_consigne)
            ymin_consigne = y;
        if(y> ymax_consigne)
            ymax_consigne = y;
    }
    if(ymax_consigne-ymin_consigne<1)
    {
        ymax_consigne=ymin_consigne+1;
    }

    y = serie_erreur->at(index).y();
    if(y< ymin_erreur || y > ymax_erreur)
    {
        if(y < ymin_erreur)
            ymin_erreur = y;
        if(y> ymax_erreur)
            ymax_erreur = y;
    }
    if(ymax_erreur-ymin_erreur<1)
    {
        ymax_erreur=ymin_erreur+1;
    }

    y = serie_spid1->at(index).y();
    if(y< ymin_spid1 || y > ymax_spid1)
    {
        if(y < ymin_spid1)
            ymin_spid1 = y;
        if(y> ymax_spid1)
            ymax_spid1 = y;
    }
    if(ymax_spid1-ymin_spid1<1)
    {
        ymax_spid1=ymin_spid1+1;
    }

    y = serie_spid2->at(index).y();
    if(y< ymin_spid2 || y > ymax_spid2)
    {
        if(y < ymin_spid2)
            ymin_spid2 = y;
        if(y> ymax_spid2)
            ymax_spid2 = y;
    }
    if(ymax_spid2-ymin_spid2<1)
    {
        ymax_spid2=ymin_spid2+1;
    }

    y = serie_val1->at(index).y();
    if(y< ymin_val1 || y > ymax_val1)
    {
        if(y < ymin_val1)
            ymin_val1 = y;
        if(y> ymax_val1)
            ymax_val1 = y;
    }
    if(ymax_val1-ymin_val1<1)
    {
        ymax_val1=ymin_val1+1;
    }

    y = serie_val2->at(index).y();
    if(y< ymin_val2 || y > ymax_val2)
    {
        if(y < ymin_val2)
            ymin_val2 = y;
        if(y> ymax_val2)
            ymax_val2 = y;
    }
    if(ymax_val2-ymin_val2<1)
    {
        ymax_val2=ymin_val2+1;
    }
}

void MainWindow::pt_consigne_added(int index)
{
    qreal y = serie_consigne->at(index).y();

    if(y< ymin_consigne || y > ymax_consigne)
    {
        if(y < ymin_consigne)
            ymin_consigne = y;
        if(y> ymax_consigne)
            ymax_consigne = y;
    }
    if(ymax_consigne-ymin_consigne<1)
    {
        ymax_consigne=ymin_consigne+1;
    }

    //    axis_consigne->setRange(ymin_consigne,ymax_consigne);
}

void MainWindow::pt_erreur_added(int index)
{
    qreal y = serie_erreur->at(index).y();

    if(y< ymin_erreur || y > ymax_erreur)
    {
        if(y < ymin_erreur)
            ymin_erreur = y;
        if(y> ymax_erreur)
            ymax_erreur = y;
    }
    if(ymax_erreur-ymin_erreur<1)
    {
        ymax_erreur=ymin_erreur+1;
    }
    //  axis_erreur->setRange(ymin_erreur,ymax_erreur);
}

void MainWindow::pt_spid1_added(int index)
{
    qreal y = serie_spid1->at(index).y();

    if(y< ymin_spid1 || y > ymax_spid1)
    {
        if(y < ymin_spid1)
            ymin_spid1 = y;
        if(y> ymax_spid1)
            ymax_spid1 = y;
    }
    if(ymax_spid1-ymin_spid1<1)
    {
        ymax_spid1=ymin_spid1+1;
    }
    //    axis_spid1->setRange(ymin_spid1,ymax_spid1);
}

void MainWindow::pt_spid2_added(int index)
{
    qreal y = serie_spid2->at(index).y();

    if(y< ymin_spid2 || y > ymax_spid2)
    {
        if(y < ymin_spid2)
            ymin_spid2 = y;
        if(y> ymax_spid2)
            ymax_spid2 = y;
    }
    if(ymax_spid2-ymin_spid2<1)
    {
        ymax_spid2=ymin_spid2+1;
    }
    //  axis_spid2->setRange(ymin_spid2,ymax_spid2);
}

void MainWindow::pt_val1_added(int index)
{
    qreal y = serie_val1->at(index).y();

    if(y< ymin_val1 || y > ymax_val1)
    {
        if(y < ymin_val1)
            ymin_val1 = y;
        if(y> ymax_val1)
            ymax_val1 = y;
    }
    if(ymax_val1-ymin_val1<1)
    {
        ymax_val1=ymin_val1+1;
    }
    //  axis_val1->setRange(ymin_val1,ymax_val1);
}

void MainWindow::pt_val2_added(int index)
{
    qreal y = serie_val2->at(index).y();

    if(y< ymin_val2 || y > ymax_val2)
    {
        if(y < ymin_val2)
            ymin_val2 = y;
        if(y> ymax_val2)
            ymax_val2 = y;
    }
    if(ymax_val2-ymin_val2<1)
    {
        ymax_val2=ymin_val2+1;
    }
    //  axis_val2->setRange(ymin_val2,ymax_val2);
}
MainWindow::~MainWindow()
{
    delete ui;
    serial->close();
}

void MainWindow::serial_timeout()
{
    rcv_serie.rcv_ok=false;
    timer->stop();
}

void MainWindow::serialreceived()
{
    long i;
    timer->start(2000);
    serbuf=serial->readAll();

    for(i=0;i<serbuf.size();i++)
    {
        if ((serbuf.at(i)=='P')&&(rcv_serie.nb_char==0))
        {
            rcv_serie.nb_char=1;
        }
        else if((serbuf.at(i)=='P')&&(rcv_serie.nb_char==1))
        {
            rcv_serie.nb_char=2;
        }
        else if((rcv_serie.nb_char>=2)&&(rcv_serie.nb_char<18))
        {
            rcv_serie.nb_char++;
            if(rcv_serie.nb_char==3)
            {
                rcv_serie.control=(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==5)
            {
                rcv_serie.Tech=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==7)
            {
                rcv_serie.consigne=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==9)
            {
                rcv_serie.epsilon=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==11)
            {
                rcv_serie.spid1=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==13)
            {
                rcv_serie.spid2=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==15)
            {
                rcv_serie.val1=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
            }
            else if(rcv_serie.nb_char==17)
            {
                timer->stop();
                rcv_serie.val2=(rcv_serie.temp_val<<8)+(unsigned char)serbuf.at(i);
                rcv_serie.nb_char=0;
                rcv_serie.rcv_ok=true;
            }
            rcv_serie.temp_val=serbuf.at(i);
        }
        else
        {
            rcv_serie.nb_char=0;
            rcv_serie.temp_val=0;
            rcv_serie.rcv_ok=false;
        }
        if(rcv_serie.rcv_ok==true)
        {
            rcv_serie.rcv_ok=false;
            tab_data.append(rcv_serie);
            qDebug() << rcv_serie.spid1;
        }
    }
}

// modif d'une valeur => envoie trame complète :
void MainWindow::calc_num_den_float(double value,unsigned short &value_num,unsigned short &value_den)
{
    if (value >=1000)
    {
        value_num=(unsigned short)(value);
        value_den=1;
    }
    else if (value >= 100)
    {
        value_num=(unsigned short)(value*10.0);
        value_den=10;
    }
    else if (value >=10)
    {
        value_num=(unsigned short)(value*100.0);
        value_den=100;
    }
    else if (value >=1)
    {
        value_num=(unsigned short)(value*1000.0);
        value_den=1000;
    }
    else
    {
        value_num=(unsigned short)(value*10000.0);
        value_den=10000;
    }
}

// modif d'une valeur => envoie trame complète :
void MainWindow::calc_num_den_float_per(double value,unsigned short &value_num,unsigned short &value_den)
{
    if (value >=1000)
    {
        value_num=(unsigned short)(value);
        value_den=1;
    }
    else if (value >= 100)
    {
        value_num=(unsigned short)(value*10.0);
        value_den=10;
    }
    else if (value >=10)
    {
        value_num=(unsigned short)(value*100.0);
        value_den=100;
    }
    else
    {
        value_num=(unsigned short)(value*1000.0);
        value_den=1000;
    }
}

void MainWindow::on_spinBox_fech_editingFinished()
{
    //some data
   /*     short i;
    for(i=0;i<200;i++)
    {
    serie_spid2->insert(compteur, QPointF(compteur,20*sin((double)compteur/50)));//  append( compteur,20*sin((double)compteur));
      chart->axisX()->setRange(0,compteur+10);
   compteur=compteur+1;
    } */
    //    this->envoie_trame_config();
}

void MainWindow::on_combobox_mode_fct_currentIndexChanged(int index)
{
    clear_series();
    switch (index) {
    case 0:
        ui->label_11->setText("Vitesse Initiale (tr/min)");
        ui->cons_sb_vinit->setRange(-5000,5000);
        ui->label_12->setText("Vitesse Finale (tr/min)");
        ui->cons_sd_vfin->setRange(-5000,5000);
        ui->groupbox_pid1->setTitle("PID Boucle Vitesse");
        ui->label_6->setText("Kp coeff. Proportionnel");
        ui->label_4->show();
        ui->pid1_sb_kp->show();
        ui->aff_cb_erreur->show();
        ui->label_4->setText("Kp coeff. Proportionnel");
        ui->aff_cb_consigne->setText("Consigne (tr/min)");
        ui->aff_cb_pid1->setText("Sortie PID Vitesse");
        ui->label_5->setText("Ki: coeff. Intégral (1/s)");
        ui->label_3->setText("Kd: coeff. Dérivé (s)");
        ui->label_7->setText("Ki: coeff. Intégral (1/s)");
        ui->label_8->setText("Kd: coeff. Dérivé (s)");
        ui->aff_cb_erreur->setText("Erreur Vitesse");
        ui->groupbox_pid1->show();
        ui->groupbox_pid2->hide();
        ui->aff_cb_pid1->show();
        ui->aff_cb_pid2->setChecked(false);
        ui->aff_cb_pid2->hide();
        break;
    case 1:
        ui->label_11->setText("Position Initiale (°)");
        ui->cons_sb_vinit->setRange(-16000,16000);
        ui->label_12->setText("Position Finale (°)");
        ui->cons_sd_vfin->setRange(-16000,16000);
        ui->label_4->show();
        ui->label_4->setText("Kp coeff. Proportionnel");
        ui->label_6->setText("Kp coeff. Proportionnel");
        ui->pid1_sb_kp->show();
        ui->groupbox_pid2->setTitle("PID Boucle Position");
        ui->groupbox_pid1->setTitle("PID Boucle Vitesse (interne)");
        ui->aff_cb_erreur->show();
        ui->aff_cb_consigne->setText("Consigne (°)");
        ui->aff_cb_pid2->setText("Sortie PID Position");
        ui->label_5->setText("Ki: coeff. Intégral (1/s)");
        ui->label_3->setText("Kd: coeff. Dérivé (s)");
        ui->label_7->setText("Ki: coeff. Intégral (1/s)");
        ui->label_8->setText("Kd: coeff. Dérivé (s)");
        ui->groupbox_pid1->hide();
        ui->aff_cb_erreur->setText("Erreur Position");
        ui->groupbox_pid2->show();
        ui->aff_cb_pid1->setChecked(false);
        ui->aff_cb_pid1->hide();
        ui->aff_cb_pid2->show();
        break;
    case 2:
        ui->label_11->setText("Position Initiale (°)");
        ui->cons_sb_vinit->setRange(-16000,16000);
        ui->label_12->setText("Position Finale (°)");
        ui->cons_sd_vfin->setRange(-16000,16000);
        ui->label_4->show();
        ui->pid1_sb_kp->show();
        ui->label_4->setText("Kp coeff. Proportionnel");
        ui->label_6->setText("Kp coeff. Proportionnel");
        ui->groupbox_pid2->setTitle("PID Boucle Position (externe)");
        ui->groupbox_pid1->setTitle("PID Boucle Vitesse (interne)");
        ui->aff_cb_consigne->setText("Consigne (°)");
        ui->aff_cb_erreur->show();
        ui->aff_cb_pid1->setText("Sortie PID Vitesse");
        ui->aff_cb_pid2->setText("Sortie PID Position");
        ui->label_5->setText("Ki: coeff. Intégral (1/s)");
        ui->label_3->setText("Kd: coeff. Dérivé (s)");
        ui->label_7->setText("Ki: coeff. Intégral (1/s)");
        ui->label_8->setText("Kd: coeff. Dérivé (s)");
        ui->aff_cb_erreur->setText("Erreur Position");
        ui->groupbox_pid1->show();
        ui->groupbox_pid2->show();
        ui->aff_cb_pid1->show();
        ui->aff_cb_pid2->show();
        break;
    default:
        ui->label_11->setText("PWM Initiale(%)");
        ui->cons_sb_vinit->setRange(-100,100);
        ui->label_12->setText("PWM Finale(%)");
        ui->cons_sd_vfin->setRange(-100,100);
        ui->groupbox_pid1->setTitle("Coefficients intégral et dérivé");
        ui->aff_cb_consigne->setText("Consigne PWM (%)");
        ui->aff_cb_pid1->setText("Sortie intégration");
        ui->aff_cb_pid2->setText("Sortie dérivation");
        ui->pid1_sb_kp->hide();
        ui->label_4->hide();
        ui->label_3->setText("Coeff. Dérivé");
        ui->pid1_sb_ki->setValue(1);
        ui->label_5->setText("Coeff. Intégral");
        ui->pid1_sb_kd->setValue(1);
        //ui->aff_cb_erreur->hide();
        ui->aff_cb_erreur->setText("Valeur Registre PWM");
        ui->groupbox_pid1->show();
        ui->groupbox_pid2->hide();
        ui->aff_cb_pid1->show();
        ui->aff_cb_pid2->show();
        break;
    }
    //    this->envoie_trame_config();
}

void MainWindow::on_cons_cb_type_currentIndexChanged(int index)
{
    //    this->envoie_trame_config();
}

void MainWindow::on_cons_sb_per_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_cons_sb_vinit_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_cons_sd_vfin_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_pid1_sb_kp_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_pid2_sb_kp_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_pid1_sb_ki_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_pid2_sb_ki_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_pid1_sb_kd_editingFinished()
{
    //    this->envoie_trame_config();
}

void MainWindow::on_pid2_sb_kd_editingFinished()
{
    //    this->envoie_trame_config();
}

// Gestion Zoom et afficahge

void MainWindow::on_aff_cb_consigne_stateChanged(int arg1)
{
    QPointF pt;
    QString a;
    if(arg1)
    {
        serie_consigne->show();
        axis_consigne->show();
        if(curseur_g->count()>1)
            ui->cu_lb_y1->show();
        if(curseur_d->count()>1)
            ui->cu1_lb_y1->show();
        if(serie_consigne->count()>2)
        {
            pt=this->extrapol(curseur_g->at(1).x(),serie_consigne);
            ui->cu_lb_y1->setText(a.number(pt.y(),'G',4));
            pt=this->extrapol(curseur_d->at(1).x(),serie_consigne);
            ui->cu1_lb_y1->setText(a.number(pt.y(),'G',4));
        }
    }
    else
    {
        serie_consigne->hide();
        axis_consigne->hide();
        ui->cu_lb_y1->hide();
        ui->cu1_lb_y1->hide();
    }
}

void MainWindow::on_aff_cb_erreur_stateChanged(int arg1)
{
    QPointF pt;
    QString a;
    if(arg1)
    {
        serie_erreur->show();
        axis_erreur->show();
        if(curseur_g->count()>1)
            ui->cu_lb_y2->show();
        if(curseur_d->count()>1)
            ui->cu1_lb_y2->show();
        if(serie_erreur->count()>2)
        {
            pt=this->extrapol(curseur_g->at(1).x(),serie_erreur);
            ui->cu_lb_y2->setText(a.number(pt.y(),'G',4));
            pt=this->extrapol(curseur_d->at(1).x(),serie_erreur);
            ui->cu1_lb_y2->setText(a.number(pt.y(),'G',4));
        }
    }
    else
    {
        serie_erreur->hide();
        axis_erreur->hide();
        ui->cu_lb_y2->hide();
        ui->cu1_lb_y2->hide();
    }
}

void MainWindow::on_aff_cb_pid1_stateChanged(int arg1)
{
    QPointF pt;
    QString a;
    if(arg1)
    {
        serie_spid1->show();
        axis_spid1->show();
        if(curseur_g->count()>1)
            ui->cu_lb_y3->show();
        if(curseur_d->count()>1)
            ui->cu1_lb_y3->show();
        if(serie_spid1->count()>2)
        {
            pt=this->extrapol(curseur_g->at(1).x(),serie_spid1);
            ui->cu_lb_y3->setText(a.number(pt.y(),'G',4));
            pt=this->extrapol(curseur_d->at(1).x(),serie_spid1);
            ui->cu1_lb_y3->setText(a.number(pt.y(),'G',4));
        }
    }
    else
    {
        serie_spid1->hide();
        axis_spid1->hide();
        ui->cu_lb_y3->hide();
        ui->cu1_lb_y3->hide();
    }
}

void MainWindow::on_aff_cb_pid2_stateChanged(int arg1)
{
    QPointF pt;
    QString a;
    if(arg1)
    {
        serie_spid2->show();
        axis_spid2->show();
        if(curseur_g->count()>1)
            ui->cu_lb_y4->show();
        if(curseur_d->count()>1)
            ui->cu1_lb_y4->show();
        if(serie_spid2->count()>2)
        {
            pt=this->extrapol(curseur_g->at(1).x(),serie_spid2);
            ui->cu_lb_y4->setText(a.number(pt.y(),'G',4));
            pt=this->extrapol(curseur_d->at(1).x(),serie_spid2);
            ui->cu1_lb_y4->setText(a.number(pt.y(),'G',4));
        }
    }
    else
    {
        serie_spid2->hide();
        axis_spid2->hide();
        ui->cu_lb_y4->hide();
        ui->cu1_lb_y4->hide();
    }
}

void MainWindow::on_aff_cb_val1_stateChanged(int arg1)
{
    QPointF pt;
    QString a;
    if(arg1)
    {
        serie_val1->show();
        axis_val1->show();
        if(curseur_g->count()>1)
            ui->cu_lb_y5->show();
        if(curseur_d->count()>1)
            ui->cu1_lb_y5->show();
        if(serie_val1->count()>2)
        {
            pt=this->extrapol(curseur_g->at(1).x(),serie_val1);
            ui->cu_lb_y5->setText(a.number(pt.y(),'G',4));
            pt=this->extrapol(curseur_d->at(1).x(),serie_val1);
            ui->cu1_lb_y5->setText(a.number(pt.y(),'G',4));
        }
    }
    else
    {
        serie_val1->hide();
        axis_val1->hide();
        ui->cu_lb_y5->hide();
        ui->cu1_lb_y5->hide();
    }
}

void MainWindow::on_aff_cb_val2_stateChanged(int arg1)
{
    QPointF pt;
    QString a;
    if(arg1)
    {
        serie_val2->show();
        axis_val2->show();
        if(curseur_g->count()>1)
            ui->cu_lb_y6->show();
        if(curseur_d->count()>1)
            ui->cu1_lb_y6->show();
        if(serie_val2->count()>2)
        {
            pt=this->extrapol(curseur_g->at(1).x(),serie_val2);
            ui->cu_lb_y6->setText(a.number(pt.y(),'G',4));
            pt=this->extrapol(curseur_d->at(1).x(),serie_val2);
            ui->cu1_lb_y6->setText(a.number(pt.y(),'G',4));
        }
    }
    else
    {
        serie_val2->hide();
        axis_val2->hide();
        ui->cu_lb_y6->hide();
        ui->cu1_lb_y6->hide();
    }
}

void MainWindow::envoie_trame_config()
{
    QByteArray data_to_send;
    unsigned short CRC;
    config_ui.Tech=ui->spinBox_fech->value();
    config_ui.nb_boucles=ui->combobox_mode_fct->currentIndex()+0x31+128*marche_bool;
    calc_num_den_float(ui->pid1_sb_kp->value(),config_ui.Kp1_num,config_ui.Kp1_den);
    calc_num_den_float(ui->pid2_sb_kp->value(),config_ui.Kp2_num,config_ui.Kp2_den);
    calc_num_den_float(ui->pid1_sb_kd->value(),config_ui.Kd1_num,config_ui.Kd1_den);
    calc_num_den_float(ui->pid2_sb_kd->value(),config_ui.Kd2_num,config_ui.Kd2_den);
    calc_num_den_float(ui->pid1_sb_ki->value(),config_ui.Ki1_num,config_ui.Ki1_den);
    calc_num_den_float(ui->pid2_sb_ki->value(),config_ui.Ki2_num,config_ui.Ki2_den);
    config_ui.type_consigne=(char)(ui->cons_cb_type->currentIndex()+0x41);
    calc_num_den_float_per(ui->cons_sb_per->value(),config_ui.cons_per_num,config_ui.cons_per_den);
    if(ui->combobox_mode_fct->currentIndex()==0)
    {
        config_ui.cons_initiale=ui->cons_sb_vinit->value()*1;
        config_ui.cons_finale=ui->cons_sd_vfin->value()*1;
    }
    else if((ui->combobox_mode_fct->currentIndex()==1)||(ui->combobox_mode_fct->currentIndex()==2))
    {
        config_ui.cons_initiale=ui->cons_sb_vinit->value()*1;
        config_ui.cons_finale=ui->cons_sd_vfin->value()*1;
    }
    else
    {
        config_ui.cons_initiale=ui->cons_sb_vinit->value()*100;
        config_ui.cons_finale=ui->cons_sd_vfin->value()*100;
    }

    data_to_send.append((char)(config_ui.Tech>>8));
    data_to_send.append((char)(config_ui.Tech));
    data_to_send.append((char)config_ui.nb_boucles);
    data_to_send.append((char)(config_ui.Kp1_num>>8));
    data_to_send.append((char)(config_ui.Kp1_num));
    data_to_send.append((char)(config_ui.Kp1_den>>8));
    data_to_send.append((char)(config_ui.Kp1_den));
    data_to_send.append((char)(config_ui.Kp2_num>>8));
    data_to_send.append((char)(config_ui.Kp2_num));
    data_to_send.append((char)(config_ui.Kp2_den>>8));
    data_to_send.append((char)(config_ui.Kp2_den));
    data_to_send.append((char)(config_ui.Kd1_num>>8));
    data_to_send.append((char)(config_ui.Kd1_num));
    data_to_send.append((char)(config_ui.Kd1_den>>8));
    data_to_send.append((char)(config_ui.Kd1_den));
    data_to_send.append((char)(config_ui.Kd2_num>>8));
    data_to_send.append((char)(config_ui.Kd2_num));
    data_to_send.append((char)(config_ui.Kd2_den>>8));
    data_to_send.append((char)(config_ui.Kd2_den));
    data_to_send.append((char)(config_ui.Ki1_num>>8));
    data_to_send.append((char)(config_ui.Ki1_num));
    data_to_send.append((char)(config_ui.Ki1_den>>8));
    data_to_send.append((char)(config_ui.Ki1_den));
    data_to_send.append((char)(config_ui.Ki2_num>>8));
    data_to_send.append((char)(config_ui.Ki2_num));
    data_to_send.append((char)(config_ui.Ki2_den>>8));
    data_to_send.append((char)(config_ui.Ki2_den));
    data_to_send.append(config_ui.type_consigne);
    data_to_send.append((char)(config_ui.cons_initiale>>8));
    data_to_send.append((char)(config_ui.cons_initiale));
    data_to_send.append((char)(config_ui.cons_finale>>8));
    data_to_send.append((char)(config_ui.cons_finale));
    data_to_send.append((char)((config_ui.cons_per_num)>>8));
    data_to_send.append((char)(config_ui.cons_per_num));
    data_to_send.append((char)((config_ui.cons_per_den)>>8));
    data_to_send.append((char)(config_ui.cons_per_den));
    CRC=config_ui.Tech^config_ui.Kp1_num^config_ui.Kp1_den^config_ui.Kp2_num^config_ui.Kp2_den^ \
            config_ui.Ki1_num^config_ui.Ki1_den^config_ui.Ki2_num^config_ui.Ki2_den^config_ui.Kd1_num^config_ui.Kd1_den^ \
            config_ui.Kd1_num^config_ui.Kd1_den^config_ui.Kp2_num^config_ui.cons_initiale^   \
            config_ui.cons_finale^(config_ui.cons_per_den)^(config_ui.cons_per_num)^((config_ui.nb_boucles<<8)+config_ui.type_consigne);
    data_to_send.append((char)(CRC>>8));
    data_to_send.append((char)(CRC));
    serial->write("PAR.");
    serial->write(data_to_send);
    data_to_send.clear();
}

void MainWindow::on_pushbutton_matlab_clicked()
{
    int i;
    QFile fichiercsv;
    QString tmp,filename, stemps, sconsigne, serreur,sspid1,sspid2,sval1,sval2;
    filename=QFileDialog::getSaveFileName();
    fichiercsv.setFileName(filename);

    if (fichiercsv.open(QFile::WriteOnly|QFile::Truncate))
    {
        QTextStream stream(&fichiercsv);
        stream.setLocale(QLocale("fr"));
        stemps.append("temps =[");
        sconsigne.append("consigne = [");
        serreur.append("erreur =[");
        sspid1.append("spid1 =[");
        sspid2.append("spid2 =[");
        sval1.append("val1 =[");
        sval2.append("val2 =[");
        for(i=0;i<serie_consigne->count();i++)
        {
            stemps.append(tmp.number(serie_consigne->at(i).x())+";");
            sconsigne.append(tmp.number(serie_consigne->at(i).y())+";");
            serreur.append(tmp.number(serie_erreur->at(i).y())+";");
            sspid1.append(tmp.number(serie_spid1->at(i).y())+";");
            sspid2.append(tmp.number(serie_spid2->at(i).y())+";");
            sval1.append(tmp.number(serie_val1->at(i).y())+";");
            sval2.append(tmp.number(serie_val2->at(i).y())+";");
        }
        stemps.append("]; \r\n");
        sconsigne.append("]; \r\n");
        serreur.append("]; \r\n");
        sspid1.append("]; \r\n");
        sspid2.append("]; \r\n");
        sval1.append("]; \r\n");
        sval2.append("]; \r\n");
        stream << stemps;
        stream << sconsigne;
        stream << serreur;
        stream << sspid1;
        stream << sspid2;
        stream << sval1;
        stream << sval2;
        fichiercsv.close();
    }

}

void MainWindow::on_dev_label_clicked()
{
    QMessageBox::about(this,"IHM TP Automatique",
                       "<h4>TP Automatique 4° Année</h4>\n\n"
                       "TP Automatique <br>"
                       "Copyright 2018 Enise.<br>"
                       "Support: Galtier Granjon - ENISE <br>"
                       "<a href=\"http://www.enise.fr\">http://www.enise.fr</a>" );

}
void MainWindow::on_pushbutton_csv_clicked()
{
    int i;
    QFile fichiercsv;
    QString filename;
    filename=QFileDialog::getSaveFileName();// fd->directory().path()+"/fichiercsv.csv");
    fichiercsv.setFileName(filename);

    if (fichiercsv.open(QFile::WriteOnly|QFile::Truncate))
    {
        QTextStream stream(&fichiercsv);
        stream.setLocale(QLocale("fr"));
        for(i=0;i<serie_consigne->count();i++)
        {
            stream << serie_consigne->at(i).x()<<";"<< serie_consigne->at(i).y()<<";"<< serie_erreur->at(i).y()<<";" \
                   << serie_spid1->at(i).y()<<";"<< serie_spid2->at(i).y()<<";"<< serie_val1->at(i).y()<<";" \
                   << serie_val2->at(i).y()<<"\r\n" ;
        }
        fichiercsv.close();
    }

}
void MainWindow::grise_commande(bool on_off)
{
    ui->groupBox_ech->setEnabled(on_off);
    ui->groupbox_pid1->setEnabled(on_off);
    ui->groupbox_pid2->setEnabled(on_off);
    ui->groupBox_mode_fct->setEnabled(on_off);
    ui->groupBox_consigne->setEnabled(on_off);
}

void MainWindow::on_pushButton_toggled(bool checked)
{
    if(checked)
    {
        QString port_name;
        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            /*  qDebug() << "Name : " << info.portName();
                qDebug() << "Description : " << info.description();
                qDebug() << "Manufacturer: " << info.manufacturer();
                qDebug() << "VID " << info.vendorIdentifier();
                qDebug() << "PID" << info.productIdentifier();
                qDebug() << index << "\r\n"; */
            if((info.vendorIdentifier()==1204)&&(info.productIdentifier()==2))
            {
                port_name=info.portName();
            }
        }
        if(serial->isOpen())
        {
            serial->disconnect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()));
            serial->flush();
            serbuf.clear();
            serial->close();
        }
        rcv_serie.nb_char=0;
        rcv_serie.rcv_ok=false;
#ifdef OS_LINUX
        serial->setPortName("/dev/"+port_name);
        // serial->setPortName("/dev/pts/1"); // only for debug
#elif
        serial->setPortName(portname);
#endif
        if(port_name=="")
        {
            ui->pushButton->setChecked(false);
            serial_port_label->setText("Port Série: Clos");
        }
        else
        {
            tab_data.clear();
            serbuf.clear();
            this->grise_commande(false);
            serial->setBaudRate(921600);
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);
            serial->setReadBufferSize(0);
            serial->open(QIODevice::ReadWrite);
            serial->connect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()),Qt::UniqueConnection);
            serial_port_label->setText("Port série: "+serial->portName());
            chart_update->start(100);
            marche_bool=1;
            envoie_trame_config();
            ui->pushButton->setText("Stop Acquisition");
        }
    }
    else
    {
        serial->disconnect(serial,SIGNAL(readyRead()),this,SLOT(serialreceived()));
        chart_update->stop();
        marche_bool=0;
        this->grise_commande(true);
        serial_port_label->setText("Port Série: Clos");
        if(serial->isOpen())
        {
            serial->putChar('S');
            while(!serial->flush());
            serial->clear(QSerialPort::AllDirections);
            serbuf.clear();
            serial->close();
            ui->pushButton->setText("Start Acquisition");
        }
    }
}
void MainWindow::append_point()
{
    long cpt;
    double compteur_sec;
    QVector<rcv_message> tmp;
    tmp=tab_data;
    tab_data.clear();
    if(tmp.size()>0) tech=tmp.at(0).Tech;
    if(tmp.size()==0)
    {return;}
    serial_port_label->setText("Port Série: "+serial->portName()+ "   Tech = "+QString().number(tech)+ " ms");
    // qDebug()<< "Tech: "<<tmp.at(0).Tech <<"\r\n";
    for(cpt=0;cpt<(tmp.size());cpt++)
    {
        compteur=compteur+(tmp.at(cpt).Tech);
        compteur_sec=compteur/1000;
        if(ui->combobox_mode_fct->currentIndex()==3)
        {
            serie_consigne->append(compteur_sec,tmp.at(cpt).consigne/10);
        }
        else
        {
            serie_consigne->append(compteur_sec,tmp.at(cpt).consigne);
        }
        serie_erreur->append(compteur_sec,tmp.at(cpt).epsilon);
        serie_spid1->append(compteur_sec,tmp.at(cpt).spid1);
        serie_spid2->append(compteur_sec,tmp.at(cpt).spid2);
        serie_val1->append(compteur_sec,tmp.at(cpt).val1);
        serie_val2->append(compteur_sec,tmp.at(cpt).val2);
        // Pour le mot de controle sur 8 bits:
        // 7    6   5   4   3   2   1   0
        // ??   ??  ??  ??  ??  Pb  ATU AT
        etat_label->setText("Système :");
        if(tmp.at(cpt).control&0x02)
        {
            etat_label->setText("Système: ATU");
        }
        else
        {
            etat_label->setText("Système: Marche");
        }
        if(tmp.at(cpt).control&0x01)
        {
            etat_label->setText("Système: Arrêt");
        }
        else
        {
            etat_label->setText("Système: Marche");
        }
        if(tmp.at(cpt).control&0x04)
        {
            etat_label->setText(etat_label->text()+" Défaut");
        }
        if(tech<20)
        {
            if (nbr>NB_PT_SCROLLING)
            {
              /*  if(((nbr)%NB_PT_SCROLLING)==0)
                {
                    ymax_consigne=-100000;
                    ymin_consigne=100000;
                    ymax_erreur=-100000;
                    ymin_erreur=100000;
                    ymax_spid1=-100000;
                    ymin_spid1=100000;
                    ymax_spid2=-100000;
                    ymin_spid2=100000;
                    ymax_val1=-100000;
                    ymin_val1=100000;
                    ymax_val2=-100000;
                    ymin_val2=100000;
                }*/
                serie_consigne->removePoints(0,1);
                serie_erreur->removePoints(0,1);
                serie_spid1->removePoints(0,1);
                serie_spid2->removePoints(0,1);
                serie_val1->removePoints(0,1);
                serie_val2->removePoints(0,1);
            }
            nbr++;
        }
    }
    //  scrolling horizontal
if(tmp.size()>100)
{}
else
{
    if((compteur-NB_PT_SCROLLING*tech)>0)
    {
        chart->axisX()->setRange((compteur-NB_PT_SCROLLING*tech)/1000,compteur_sec+0.1);
        if(ui->combobox_mode_fct->currentIndex()==0)
        {
            if(ymin_consigne<ymin_val1)
            {
                ymin_val1=ymin_consigne;
            }
            else if(ymin_consigne>ymin_val1)
            {
                ymin_consigne=ymin_val1;
            }
            if(ymax_consigne>ymax_val1)
            {
                ymax_val1=ymax_consigne;
            }
            else if(ymax_consigne<ymax_val1)
            {
                ymax_consigne=ymax_val1;
            }
            axis_consigne->setRange(ymin_val1,ymax_val1);
        }
        else if((ui->combobox_mode_fct->currentIndex()==1)||(ui->combobox_mode_fct->currentIndex()==2))
        {
            if(ymin_consigne<ymin_val2)
            {
                ymin_val2=ymin_consigne;
            }
            else if(ymin_consigne>ymin_val2)
            {
                ymin_consigne=ymin_val2;
            }
            if(ymax_consigne>ymax_val2)
            {
                ymax_val2=ymax_consigne;
            }
            else if(ymax_consigne<ymax_val2)
            {
                ymax_consigne=ymax_val2;
            }
            axis_consigne->setRange(ymin_val2,ymax_val2);
        }
        else
        {
            axis_consigne->setRange(ymin_consigne,ymax_consigne);
        }
        axis_erreur->setRange(ymin_erreur,ymax_erreur);
        axis_spid1->setRange(ymin_spid1,ymax_spid1);
        axis_spid2->setRange(ymin_spid2,ymax_spid2);
        axis_val1->setRange(ymin_val1,ymax_val1);
        axis_val2->setRange(ymin_val2,ymax_val2);
    }
    else
    {
        chart->axisX()->setRange(0,compteur_sec+0.1);
       if(ui->combobox_mode_fct->currentIndex()==0)
        {
            if(ymin_consigne<ymin_val1)
            {
                ymin_val1=ymin_consigne;
            }
            else if(ymin_consigne>ymin_val1)
            {
                ymin_consigne=ymin_val1;
            }
            if(ymax_consigne>ymax_val1)
            {
                ymax_val1=ymax_consigne;
            }
            else if(ymax_consigne<ymax_val1)
            {
                ymax_consigne=ymax_val1;
            }
            axis_consigne->setRange(ymin_val1,ymax_val1);
        }
        else if((ui->combobox_mode_fct->currentIndex()==1)||(ui->combobox_mode_fct->currentIndex()==2))
        {
            if(ymin_consigne<ymin_val2)
            {
                ymin_val2=ymin_consigne;
            }
            else if(ymin_consigne>ymin_val2)
            {
                ymin_consigne=ymin_val2;
            }
            if(ymax_consigne>ymax_val2)
            {
                ymax_val2=ymax_consigne;
            }
            else if(ymax_consigne<ymax_val2)
            {
                ymax_consigne=ymax_val2;
            }
            axis_consigne->setRange(ymin_val2,ymax_val2);
        }
        else
        {
            axis_consigne->setRange(ymin_consigne,ymax_consigne);
        }
        axis_erreur->setRange(ymin_erreur,ymax_erreur);
        axis_spid1->setRange(ymin_spid1,ymax_spid1);
        axis_spid2->setRange(ymin_spid2,ymax_spid2);
        axis_val1->setRange(ymin_val1,ymax_val1);
        axis_val2->setRange(ymin_val2,ymax_val2);
    }
    replot_cursor();
}
}

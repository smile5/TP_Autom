#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define OS_LINUX
#define NB_PT_SCROLLING 20000
#include "chartview.h"
#include "clickableqlabel.h"
#include "lineseries.h"
#include <QMainWindow>
#include <QLabel>
#include <QSerialPortInfo>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

QT_CHARTS_USE_NAMESPACE


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

#define COLOR_CONSIGNE QColor(193,125,17)
#define COLOR_ERREUR QColor(255,0,0)
#define COLOR_SPID1 QColor(245,166,0)
#define COLOR_SPID2 QColor(255, 255, 0)
#define COLOR_VAL1 QColor(115,210,22)
#define COLOR_VAL2 QColor(114,159,207)

 typedef struct{
        unsigned short Tech; // Temps echantillonnage en multiple de 1ms
        char nb_boucles; // nb boucles '0' boucle ouverte, '1' une boucle, '2' 2 boucles imbriquÃ©es , '3' boucle ouvert avec signal du PC
        unsigned short Kp1_num; // numerateur KP 1 - Proportionnel  - si 1 boucle alors seule la boucle 1 est active
        unsigned short Kp1_den; // Denominateur KP 1
        unsigned short Kp2_num; // numerateur KP 2
        unsigned short Kp2_den; // Denominateur KP 2
        unsigned short Kd1_num; // numerateur Kd 1 - DÃ©rivÃ©e
        unsigned short Kd1_den; // Denominateur Kd 1
        unsigned short Kd2_num; // numerateur Kd 2
        unsigned short Kd2_den; // Denominateur Kd 2
        unsigned short Ki1_num; // numerateur Ki 1 - IntÃ©gral
        unsigned short Ki1_den; // Denominateur Ki 1
        unsigned short Ki2_num; // numerateur Ki 2
        unsigned short Ki2_den; // Denominateur Ki 2
        char type_consigne; // A revoir pas correct 'E' Echellon, 'R' Rampe, 'S' Sinus, 'T' triangle, C'carrÃ©e, 'W' dent de scie
        short cons_initiale; //100 * pourcentage initial
        short cons_finale; //100 * pourcentage final
        unsigned short cons_per_num; // numerateur Kd 1 - DÃ©rivÃ©e
        unsigned short cons_per_den; // Denominateur Kd 1
        unsigned short CRC;
    } config;

    typedef struct{
        unsigned char control;
        short Tech;
        short consigne;
        short epsilon;
        short spid1;
        short spid2;
        short val1;
        short val2;
        bool rcv_ok;
        unsigned char nb_char;
        unsigned short temp_val;
    } rcv_message;
public:
    explicit MainWindow(QWidget *parent = 0);
     QSerialPortInfo serialPortInfo;
     ClickableLabel *dev_label;
     QLabel *serial_port_label;
     QLabel *etat_label,*deltax_label;
     LineSeries *serie_consigne,*serie_erreur,*serie_spid1,*serie_spid2,*serie_val1,*serie_val2;
     QLineSeries *curseur_g,*curseur_d;
     QValueAxis *axis_consigne,*axis_erreur,*axis_spid1,*axis_spid2,*axis_val1,*axis_val2;
     QValueAxis *axe_temps;
     QChart *chart ;
     ChartView *chartview;
     double compteur=0;
     ~MainWindow();
     config config_ui;
     rcv_message rcv_serie;
     QGraphicsSimpleTextItem txt_curs1,txt_curs2;
     QGraphicsLineItem line_curs1;
     QPointF pos_curs1,pos_curs2;
     QTimer *timer;

private slots:
    void replot_rubber(QRect rect, QPointF pos1, QPointF pos2);
    void replot_area(QRectF Frect);
    void replot_zoom();
    void replot_zoomreset();
    void replot_scroll();
    void serial_timeout();
    void pt_added(int index);
    void pt_consigne_added(int index);
    void pt_erreur_added(int index);
    void pt_spid1_added(int index);
    void pt_spid2_added(int index);
    void pt_val1_added(int index);
    void pt_val2_added(int index);
    void clear_series();
    void cliick(int);
    void test(QPointF);
    void serialreceived();
//    void on_serial_cb_currentIndexChanged(const QString &arg1);
    void on_spinBox_fech_editingFinished();
    void on_combobox_mode_fct_currentIndexChanged(int index);
    void on_cons_cb_type_currentIndexChanged(int index);
    void on_cons_sb_per_editingFinished();
    void on_cons_sb_vinit_editingFinished();
    void on_cons_sd_vfin_editingFinished();
    void on_pid1_sb_kp_editingFinished();
    void on_pid2_sb_kp_editingFinished();
    void on_pid1_sb_ki_editingFinished();
    void on_pid2_sb_ki_editingFinished();
    void on_pid1_sb_kd_editingFinished();
    void on_pid2_sb_kd_editingFinished();
    void on_aff_cb_consigne_stateChanged(int arg1);
    void on_aff_cb_erreur_stateChanged(int arg1);
    void on_aff_cb_pid1_stateChanged(int arg1);
    void on_aff_cb_pid2_stateChanged(int arg1);
    void on_aff_cb_val1_stateChanged(int arg1);
    void on_aff_cb_val2_stateChanged(int arg1);
    void on_pushbutton_matlab_clicked();
    void on_pushbutton_csv_clicked();
    void on_dev_label_clicked();
    void on_pushButton_toggled(bool checked);
    void append_point();
private:
    QSerialPort *serial;
    Ui::MainWindow *ui;
    QPointF extrapol(qreal x, QLineSeries *serie);
    QPointF pt_clicked;
    qreal ymin_consigne,ymin_erreur,ymin_spid1,ymin_spid2,ymin_val1,ymin_val2;
    qreal ymax_consigne,ymax_erreur,ymax_spid1,ymax_spid2,ymax_val1,ymax_val2;
    void envoie_trame_config(void);
    void replot_cursor(void);
    void calc_num_den_float(double value,unsigned short &value_num,unsigned short &value_den);
    void calc_num_den_float_per(double value,unsigned short &value_num,unsigned short &value_den);
    bool curseur_bt_g,curseur_bt_d;
    QByteArray serbuf;
    void grise_commande(bool on_off);
    QVector<rcv_message> tab_data;
    QTimer *chart_update;
    long nbr;
    int tech;
    unsigned char marche_bool;
};

#endif // MAINWINDOW_H

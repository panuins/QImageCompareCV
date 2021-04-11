#ifndef WIDGET_H
#define WIDGET_H

#include "StaticResultModel.h"
#include <QWidget>
#include <QImage>
#include <QPair>
#include<opencv2\imgproc\imgproc.hpp>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_SelectTemplate_clicked();
    void on_pushButton_Add_clicked();
    void on_pushButton_Remove_clicked();
    void on_pushButton_Static_clicked();
    void on_pushButton_export_clicked();

    void on_spinBox_itemViewSize_valueChanged(int arg1);

private:
    void compare(const cv::Mat &temp, const QString &fileName, const QImage &image, const cv::Mat &src, int method);

    QImage m_template;
    QList<QPair<QString, QImage> > m_images;
    Ui::Widget *ui;
    StaticResultModel *m_result;
};
#endif // WIDGET_H

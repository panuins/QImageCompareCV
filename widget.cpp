#include "widget.h"
#include "ui_widget.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QPlainTextEdit>
#include <QPixmap>
#include <QSettings>

#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include <QDebug>

static QDir lastDir;

static cv::Mat QImageToCvMat( const QImage &inImage, bool inCloneImageData = true )
{
    QImage image = inImage.convertToFormat(QImage::Format_ARGB32);
    switch ( image.format() )
    {
    // 8-bit, 4 channel
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    {
        cv::Mat  mat( image.height(), image.width(),
                      CV_8UC4,
                      const_cast<uchar*>(image.bits()),
                      static_cast<size_t>(image.bytesPerLine())
                      );

        return (inCloneImageData ? mat.clone() : mat);
    }

    // 8-bit, 3 channel
    case QImage::Format_RGB32:
    case QImage::Format_RGB888:
    {
        if ( !inCloneImageData )
        {
            qWarning() << "CVS::QImageToCvMat() - Conversion requires cloning because we use a temporary QImage";
        }

        QImage   swapped = image;

        if ( image.format() == QImage::Format_RGB32 )
        {
            swapped = swapped.convertToFormat( QImage::Format_RGB888 );
        }

        swapped = swapped.rgbSwapped();

        return cv::Mat( swapped.height(), swapped.width(),
                        CV_8UC3,
                        const_cast<uchar*>(swapped.bits()),
                        static_cast<size_t>(swapped.bytesPerLine())
                        ).clone();
    }

    // 8-bit, 1 channel
    case QImage::Format_Indexed8:
    {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC1,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );

        return (inCloneImageData ? mat.clone() : mat);
    }

    default:
        qWarning() << "CVS::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
        break;
    }

    return cv::Mat();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    m_result = new StaticResultModel(this);
    ui->tableView_Result->setModel(m_result);
    ui->tableView_Result->verticalHeader()->setVisible(false);
    QSettings settings(
                QDir(qApp->applicationDirPath())
                .absoluteFilePath("settings.ini"), QSettings::IniFormat);
    lastDir = QDir(settings.value("lastDir", ".").toString());
//    qDebug() << lastDir;
}

Widget::~Widget()
{
    QSettings settings(QDir(qApp->applicationDirPath()).absoluteFilePath("settings.ini"), QSettings::IniFormat);
    settings.setValue("lastDir", lastDir.absolutePath());
    delete ui;
}


void Widget::on_pushButton_SelectTemplate_clicked()
{
    QString fileName =
            QFileDialog::getOpenFileName(this, tr("Select template image"), lastDir.absolutePath(),
                                         "Images (*.png *.xpm *.jpg *.jpeg *.bmp);;All files (*.*)");
    if (!fileName.isEmpty())
    {
        m_template = QImage(fileName);
        ui->label_templateImage->setPixmap(QPixmap::fromImage(m_template));
        QFileInfo info(fileName);
        lastDir = info.dir();
    }
}

void Widget::on_pushButton_Add_clicked()
{
    QStringList files =
            QFileDialog::getOpenFileNames(this, tr("Select template image"), lastDir.absolutePath(),
                                          "Images (*.png *.xpm *.jpg *.jpeg *.bmp);;All files (*.*)");
    for (QString fileName: files)
    {
        if (!fileName.isEmpty())
        {
            QImage image = QImage(fileName);
            m_images.append(QPair<QString, QImage>(fileName, image));
            QFileInfo fileInfo(fileName);
            QPixmap pixmap = QPixmap::fromImage(image);
            ui->listWidget_Images->addItem(
                        new QListWidgetItem(QIcon(pixmap), fileInfo.fileName(), ui->listWidget_Images));
            QFileInfo info(fileName);
            lastDir = info.dir();
        }
    }
}

void Widget::on_pushButton_Remove_clicked()
{
    QList<QListWidgetItem*> list = ui->listWidget_Images->selectedItems();
    for (QListWidgetItem *item: list)
    {
        ui->listWidget_Images->removeItemWidget(item);
        delete item;
    }
}

void Widget::on_pushButton_Static_clicked()
{
    m_result->clear();
    cv::Mat temp = QImageToCvMat(m_template);
    for (QPair<QString, QImage> image: m_images)
    {
        cv::Mat src = QImageToCvMat(image.second);
        compare(temp, image.first, image.second,
                src, cv::TM_SQDIFF);
        compare(temp, image.first, image.second,
                src, cv::TM_SQDIFF_NORMED);
        compare(temp, image.first, image.second,
                src, cv::TM_CCORR);
        compare(temp, image.first, image.second,
                src, cv::TM_CCORR_NORMED);
        compare(temp, image.first, image.second,
                src, cv::TM_CCOEFF);
        compare(temp, image.first, image.second,
                src, cv::TM_CCOEFF_NORMED);
    }
}

void Widget::compare(const cv::Mat &temp,
                     const QString &fileName,
                     const QImage &image,
                     const cv::Mat &src,
                     int method)
{
    try
    {
        cv::Mat result;
        cv::matchTemplate(src, temp, result, method);
        double minVal = 0.0;
        double maxVal = 0.0;
        cv::Point minLoc;
        cv::Point maxLoc;
        cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
        m_result->addResultData(fileName, image, method,
                                minVal, maxVal,
                                QPoint(minLoc.x, minLoc.y),
                                QPoint(maxLoc.x, maxLoc.y));
    }
    catch (cv::Exception cvExceptions)
    {
        std::string exceptionStr = cvExceptions.msg;
        qDebug() << "Widget::compare: exception: fileName=" << fileName
                 << "method=" << method << exceptionStr.c_str();
    }
    catch (std::exception exceptions)
    {
        qDebug() << "Widget::compare: exception: fileName=" << fileName
                 << "method=" << method << exceptions.what();
    }
    catch (...)
    {
        qDebug() << "Widget::compare: exception: fileName=" << fileName
                 << "method=" << method << "unkown exception";
    }

}

void Widget::on_pushButton_export_clicked()
{
    QString exported = m_result->exportResults();
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setAttribute(Qt::WA_DeleteOnClose, true);
    edit->setPlainText(exported);
    edit->show();
}

void Widget::on_spinBox_itemViewSize_valueChanged(int arg1)
{
    ui->listWidget_Images->setIconSize(QSize(arg1, arg1));
    int i = 0;
    for (; i < ui->listWidget_Images->count(); i++)
    {
        QListWidgetItem *item = ui->listWidget_Images->item(i);
        QSize size = m_images.at(i).second.size();
        size.scale(arg1, arg1, Qt::KeepAspectRatio);
        item->setData(Qt::SizeHintRole, size);
    }
}

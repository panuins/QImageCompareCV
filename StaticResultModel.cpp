#include "StaticResultModel.h"
#include <QFileInfo>
#include <include/opencv2/imgproc.hpp>

StaticResultModel::StaticResultModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant StaticResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
//    (void)orientation;
    if (role != Qt::DisplayRole)
    {
        return QAbstractItemModel::headerData(section, orientation, role);
    }
    switch (section)
    {
    case 0:
        return tr("Image");
    case 1:
        return tr("Param");
    case 2:
        return tr("MinVal");
    case 3:
        return tr("MaxVal");
    case 4:
        return tr("MinLoc");
    case 5:
        return tr("MaxLoc");
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

int StaticResultModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_results.size();
}

int StaticResultModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return 6;
}

QVariant StaticResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (index.row() < 0)
    {
        return QVariant();
    }
    if (index.row() >= m_results.size())
    {
        return QVariant();
    }

    switch (index.column())
    {
    case 0:
    {
        QFileInfo fileInfo(m_results.at(index.row()).fileName);
        return QVariant(fileInfo.fileName());
    }
    case 1:
    {
        int method = m_results.at(index.row()).method;
        switch (method)
        {
        case cv::TM_SQDIFF:
            return tr("TM_SQDIFF");
        case cv::TM_SQDIFF_NORMED:
            return tr("TM_SQDIFF_NORMED");
        case cv::TM_CCORR:
            return tr("TM_CCORR");
        case cv::TM_CCORR_NORMED:
            return tr("TM_CCORR_NORMED");
        case cv::TM_CCOEFF:
            return tr("TM_CCOEFF");
        case cv::TM_CCOEFF_NORMED:
            return tr("TM_CCOEFF_NORMED");
        default:
            return method;
        }
    }
    case 2:
        return m_results.at(index.row()).minVal;
    case 3:
        return m_results.at(index.row()).maxVal;
    case 4:
    {
        QPoint p = m_results.at(index.row()).minLoc;
        return QString("(%1,%2)").arg(p.x()).arg(p.y());
    }
    case 5:
    {
        QPoint p = m_results.at(index.row()).maxLoc;
        return QString("(%1,%2)").arg(p.x()).arg(p.y());
    }
    }

    return QVariant();
}

void StaticResultModel::clear()
{
    beginResetModel();
    m_results.clear();
    endResetModel();
}

void StaticResultModel::addResultData(QString fileName,
                                      QImage image,
                                      int method,
                                      double minVal,
                                      double maxVal,
                                      QPoint minLoc,
                                      QPoint maxLoc)
{
    int insertPos = m_results.size();
    for (; insertPos > 0; insertPos--)
    {
        if (m_results.at(insertPos-1).method == method)
        {
            break;
        }
    }
    beginInsertRows(QModelIndex(), insertPos, insertPos);
    struct ResultData data;
    data.fileName = fileName;
    data.image = image;
    data.method = method;
    data.minVal = minVal;
    data.maxVal = maxVal;
    data.minLoc = minLoc;
    data.maxLoc = maxLoc;
    m_results.insert(insertPos, data);
    endInsertRows();
}

QString StaticResultModel::exportResults() const
{
    QString ret;
    {
        QString line =
                headerData(0, Qt::Horizontal).toString() + "\t" +
                headerData(1, Qt::Horizontal).toString() + "\t" +
                headerData(2, Qt::Horizontal).toString() + "\t" +
                headerData(3, Qt::Horizontal).toString() + "\t" +
                headerData(4, Qt::Horizontal).toString() + "\t" +
                headerData(5, Qt::Horizontal).toString() + "\n";
        ret.append(line);
    }
    int i = 0;
    for (; i < rowCount(); i++)
    {
        QString line =
                m_results.at(i).fileName + "\t" +
                data(createIndex(i, 1)).toString() + "\t" +
                data(createIndex(i, 2)).toString() + "\t" +
                data(createIndex(i, 3)).toString() + "\t" +
                data(createIndex(i, 4)).toString() + "\t" +
                data(createIndex(i, 5)).toString() + "\n";
        ret.append(line);
    }
    return ret;
}

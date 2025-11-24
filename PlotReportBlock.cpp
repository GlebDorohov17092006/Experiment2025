#include "PlotReportBlock.h"
#include <QTextDocument>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QUrl>

PlotReportBlock::PlotReportBlock(const QPixmap& pixmap, QWidget* parent)
    : ReportBlock(parent)
    , m_plotLabel(nullptr)
    , m_pixmap(pixmap)
{
}

QWidget* PlotReportBlock::createContent(QWidget* parent)
{
    QWidget* container = new QWidget(parent);
    QVBoxLayout* containerLayout = new QVBoxLayout(container);

    m_plotLabel = new QLabel(container);
    m_plotLabel->setPixmap(m_pixmap);
    m_plotLabel->setMinimumHeight(400);
    m_plotLabel->setMaximumHeight(400);
    m_plotLabel->setAlignment(Qt::AlignCenter);
    containerLayout->addWidget(m_plotLabel);

    return container;
}

void PlotReportBlock::exportToPDF(QTextDocument* document, QTextCursor& cursor)
{
    static int imageCounter = 0;
    QString imageName = QString("plot_image_%1").arg(++imageCounter);
    QUrl imageUrl = QUrl(QString("internal://%1").arg(imageName));
    
    document->addResource(QTextDocument::ImageResource, imageUrl, QVariant(m_pixmap));

    // Вставляем изображение в документ
    QTextImageFormat imageFormat;
    imageFormat.setName(imageUrl.toString());
    imageFormat.setWidth(m_pixmap.width());
    imageFormat.setHeight(m_pixmap.height());
    cursor.insertImage(imageFormat);
    cursor.insertBlock();
}


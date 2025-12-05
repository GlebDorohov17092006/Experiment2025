#include "ReportBlock.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QTextDocument>
#include <QTextCursor>

ReportBlock::ReportBlock(QWidget* parent)
    : m_frame(nullptr)
    , m_parentLayout(nullptr)
{
    Q_UNUSED(parent);
}

QFrame* ReportBlock::createWidget(QWidget* parent, QVBoxLayout* parentLayout)
{
    m_parentLayout = parentLayout;

    m_frame = new QFrame(parent);
    m_frame->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_frame->setLineWidth(1);

    QVBoxLayout* frameLayout = new QVBoxLayout(m_frame);

    QWidget* content = createContent(m_frame);
    if (content) {
        frameLayout->addWidget(content);
    }

    createControlButtons(m_frame, frameLayout, parentLayout);

    return m_frame;
}

void ReportBlock::createControlButtons(QFrame* frame, QVBoxLayout* frameLayout, QVBoxLayout* parentLayout)
{
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton* upButton = new QPushButton("↑", frame);
    upButton->setMaximumWidth(40);
    QPushButton* downButton = new QPushButton("↓", frame);
    downButton->setMaximumWidth(40);
    QPushButton* removeButton = new QPushButton("Удалить", frame);

    buttonsLayout->addWidget(upButton);
    buttonsLayout->addWidget(downButton);
    buttonsLayout->addWidget(removeButton);
    buttonsLayout->addStretch();

    frameLayout->addLayout(buttonsLayout);

    connect(upButton, &QPushButton::clicked, this, [frame, parentLayout]() {
        int index = parentLayout->indexOf(frame);
        if (index > 0) {
            parentLayout->removeWidget(frame);
            parentLayout->insertWidget(index - 1, frame);
        }
    });

    connect(downButton, &QPushButton::clicked, this, [frame, parentLayout]() {
        int index = parentLayout->indexOf(frame);
        if (index >= 0 && index < parentLayout->count() - 1) {
            parentLayout->removeWidget(frame);
            parentLayout->insertWidget(index + 1, frame);
        }
    });

    connect(removeButton, &QPushButton::clicked, this, [frame, parentLayout, this]() {
        parentLayout->removeWidget(frame);
        frame->deleteLater();
    });
}



#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class ReportDialog;
}
QT_END_NAMESPACE

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = nullptr);
    ~ReportDialog();

private slots:
    void onGenerateReport();
    void onCancel();

private:
    Ui::ReportDialog *ui;
};

#endif // REPORTDIALOG_H

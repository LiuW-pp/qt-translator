#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSortFilterProxyModel>

class QModelIndex;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setupDatabase();
    void setupHistoryModel();
    void translateCurrentText();
    void saveHistory(const QString &source, const QString &target,
                     const QString &fromLang, const QString &toLang,
                     const QString &example);
    void deleteHistoryByIds(const QList<int> &ids);

private slots:
    void on_btnTranslate_clicked();
    void on_btnClear_clicked();
    void on_lineEditInput_returnPressed();
    void on_tableHistory_doubleClicked(const QModelIndex &index);
    void handleTranslationFinished(QNetworkReply *reply);
    void on_btnDeleteSelected_clicked();
    void on_btnClearHistory_clicked();
    void on_actionAbout_triggered();
    void on_lineEditFilter_textChanged(const QString &text);

private:
    Ui::MainWindow *ui;
    QSqlDatabase m_db;
    QSqlTableModel *m_historyModel;
    QSortFilterProxyModel *m_proxyModel;
    QNetworkAccessManager *m_networkManager;
};
#endif // MAINWINDOW_H

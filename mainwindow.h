#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddBookClicked();
    void onEditBookClicked();
    void onDeleteBookClicked();
    void onAddShelfClicked();
    void onDeleteShelfClicked();
    void onShelfSelected();
    void refreshBookList();
    void refreshShelfList();
    void onBookSelected(const QModelIndex &index);

private:
    Ui::MainWindow *ui;
    DatabaseManager *dbManager;
    QSqlTableModel *bookModel;
    int currentBookId;
};
#endif // MAINWINDOW_H

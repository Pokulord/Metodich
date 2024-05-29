#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


// Класс для работы с базой данных в отдельном потоке
class DatabaseWorker : public QObject
{
    Q_OBJECT

public:
    DatabaseWorker(); // Конструктор класса
    ~DatabaseWorker();
    void setDatabasePath(const QString& dbPath); // задаём путь к базе данных

signals:
    void dataReady(QSqlQueryModel* model); // условный сигнал , оповещающий программу о готовности отображения данных
    void databaseOpened(bool success); // сигнал, оповещающий о том, что БД открыта

public slots:
    void openDatabase(); // слот для открытия БД
    void loadData(); // слот для загрузки данных из БД
    void deleteRecord(int id); // слот для удаления данных из БД

private:
    QSqlDatabase db; // непосредственно объект БД
    QString databasePath; // путь к БД
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
// Слоты
private slots:
    void onDataReady(QSqlQueryModel* model); // Слот для сигнала о говности данных к выводу
    void onDeleteButtonClicked(); // Прототип функции для удаления записей из БД
    void onOpenButtonClicked();  // Прототип функции для открытия БД
    void onDatabaseOpened(bool success); // Слот об открытии БД

    void on_treeView_clicked(const QModelIndex &index);

private:
    Ui::MainWindow *ui; // Указатель на все элементы интерфейса
    QThread dbThread; // Поток для работы с БД
    DatabaseWorker *dbWorker; // Объект для работы с БД
    QSqlQueryModel *model; // Модель, которую мы используем для отображения в TreeView
};
#endif // MAINWINDOW_H

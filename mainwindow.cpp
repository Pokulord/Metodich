#include "mainwindow.h"     // Импорт заголовочного файла окна
#include "ui_mainwindow.h"
#include <QSqlQuery> // Модуль для работы с запросами
#include <QSqlError> // Модуль , содержащий в себе ошибки ,которые могут возникнуть в процессе выполнения запроса
#include <QDebug> // Модуль для отображения значений переменных в консоли
#include <QFileDialog> // Модуль для открытия файлового диалога с целью дальнейшего выбора открываемого файла
#include <QMessageBox> // Модуль для отображения сообщений


// Конструктор класса для работы с БД. По умолчанию путь к ней пустой
DatabaseWorker::DatabaseWorker() : databasePath("")
{
    db = QSqlDatabase::addDatabase("QSQLITE"); // Подключаем драйвер для работы с Sqlite3
}
// Закрытие базы данных
DatabaseWorker::~DatabaseWorker()
{
    db.close(); // закрытие БД
}


// Функция для задания пути к БД. На вход поступает строка
void DatabaseWorker::setDatabasePath(const QString& dbPath)
{
    databasePath = dbPath;
}


// Функция открытия БД
void DatabaseWorker::openDatabase()
{
    db.setDatabaseName(databasePath); // "Прикрепляем" базу данных к объекту
    bool success = db.open(); // True - если выполнена успешно. Иначе - false
    emit databaseOpened(success); // Подача сигнала об успешном открытии БД
}


// Функция загрузки данных
void DatabaseWorker::loadData()
{
    if (!db.isOpen()) {
        return;
    }

    // Инициализаия модели
    QSqlQueryModel *model = new QSqlQueryModel();
    QSqlQuery query(db);
    // Выполнение запроса на выборку
    query.exec("SELECT * FROM urls");
    model->setQuery(query);

    emit dataReady(model); // Подача сигнала о готовности данных к выводу
}


// Функция удаления записи по ID
void DatabaseWorker::deleteRecord(int id)
{
    if (!db.isOpen()) {
        return;
    }
    QSqlQuery query(db);

    // Удаление записи с подстановкой id в запрос
    query.prepare("DELETE FROM urls WHERE id = :id");
    query.bindValue(":id", id);

    // В случае успеха - загрузка данных , иначе - сообщение об ошибке
    if (!query.exec()) {
        QMessageBox::critical(nullptr, "Error", "Ошибка при выполнении запроса на выборку!!!");
        return ;
    } else {
        loadData(); // Вызов функции загрузки данных
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    dbWorker(new DatabaseWorker()),
    model(new QSqlQueryModel())

{
    ui->setupUi(this);
    ui->treeView->setModel(model); // Задание модели для отображения


    // Перемещаем объект работы с БД в отдельный поток
    dbWorker->moveToThread(&dbThread);
    // Подключение сигналов и слотов
    connect(&dbThread, &QThread::started, dbWorker, &DatabaseWorker::openDatabase);
    connect(dbWorker, &DatabaseWorker::dataReady, this, &MainWindow::onDataReady);
    connect(dbWorker, &DatabaseWorker::databaseOpened, this, &MainWindow::onDatabaseOpened);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteButtonClicked);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::onOpenButtonClicked);
    // Запуск потока
    dbThread.start();
}

MainWindow::~MainWindow()
{
    dbThread.quit();
    dbThread.wait();
    delete dbWorker;
    delete ui;
}


// Функция , которая обрабатывает готовые данные
void MainWindow::onDataReady(QSqlQueryModel* newModel)
{
    model->setQuery(newModel->query());
    // Освобождаем память, удаляя модель после заполнения
    delete newModel;
    ui->treeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    // Скрываем столбцы
    ui->treeView->setColumnHidden(0, true);
    ui->treeView->setColumnHidden(3, true);
    ui->treeView->setColumnHidden(4, true);
    ui->treeView->setColumnHidden(5, true);
    ui->treeView->setColumnHidden(6, true);
}


// Обработчик нажатия на кнопку удаления записи
void MainWindow::onDeleteButtonClicked()
{
    // Получаем индекс  ThreeView выделенной записи
    QModelIndex index = ui->treeView->currentIndex();
    // Проверяем его на корректность
    if (index.isValid()) {
        // Получаем индекс, который соответствует id записи в базе данных
        int id = model->data(model->index(index.row(), 0)).toInt();
        // Вызываем метод удаления из базы данных
        QMetaObject::invokeMethod(dbWorker, "deleteRecord", Q_ARG(int, id));
    }
}
// Обработчик нажатия на кнопку открытия БД
void MainWindow::onOpenButtonClicked()
{
    // Вызываем открытие файлового диалога.
    //В случае, если пользователь выбрал файл, выполняется открытие БД
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть БД", "", "");
    if (!filePath.isEmpty()) {
        dbWorker->setDatabasePath(filePath);
        QMetaObject::invokeMethod(dbWorker, "openDatabase");
    }
}


// Функция для проверки открытия базы данных
void MainWindow::onDatabaseOpened(bool success)
{
    // Если БД успешно открылась,
    //то начинается процедура загрузки данных
    if (success) {
        QMetaObject::invokeMethod(dbWorker, "loadData");
    } else {
        QMessageBox::critical(this, "Error", "Ошибка при открытии БД");
    }
}


// Функция, которая выводит дополнительную информацию о записи по нажатию на неё

void MainWindow::on_treeView_clicked(const QModelIndex &index)
{
    // Обращаемся к текущей используемой модели ThreeView
    QAbstractItemModel *model = ui->treeView->model();
    // Получаем текущий индекс записи ThreeView
    QModelIndex current_index = ui->treeView->selectionModel()->currentIndex();
    // Обращаемся к атрибуту модели, который отвечает за количество посещений
    QModelIndex visit_count = model->index(current_index.row(),3) ;
    // Обращаемся к атрибуту модели, который отвечает за id url
    QModelIndex url_id = model->index(current_index.row(), 0) ;
    // Выводим информацию в Label
    ui->info_label->setText("Additional information:  \nЧисло посещений: " + visit_count.data().toString() + "\nID URL: " + url_id.data().toString() );
}


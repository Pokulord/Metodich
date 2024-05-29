#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowTitle("DB_output_app"); // Задаём текст заголовка окна
    w.setWindowIcon(QIcon("../../main_win_icon.png")); // Задаём иконку приложения
    return a.exec();
}

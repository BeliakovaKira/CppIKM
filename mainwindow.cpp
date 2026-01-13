#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentBookId(-1)
{
    ui->setupUi(this);

    dbManager = new DatabaseManager(this);

    if (!dbManager->initializeDatabase()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать базу данных");
    }

    bookModel = new QSqlTableModel(this);
    bookModel->setTable("books");
    bookModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    bookModel->setHeaderData(1, Qt::Horizontal, "Название");
    bookModel->setHeaderData(2, Qt::Horizontal, "Автор");
    bookModel->setHeaderData(3, Qt::Horizontal, "Страниц");
    bookModel->setHeaderData(4, Qt::Horizontal, "Год");
    bookModel->setHeaderData(5, Qt::Horizontal, "Статус");
    bookModel->setHeaderData(6, Qt::Horizontal, "Оценка");
    bookModel->setHeaderData(7, Qt::Horizontal, "Полка");

    bookModel->select();

    ui->tableViewBooks->setModel(bookModel);
    ui->tableViewBooks->hideColumn(0); // Скрыть ID
    ui->tableViewBooks->resizeColumnsToContents();

    ui->comboBoxStatus->addItems({"Хочу прочитать", "Читаю", "Прочитана"});
    ui->comboBoxFilterStatus->addItems({"Все", "Хочу прочитать", "Читаю", "Прочитана"});

    for (int i = 1; i <= 10; i++) {
        ui->comboBoxRating->addItem(QString::number(i));
    }

    connect(ui->pushButtonAddBook, &QPushButton::clicked, this, &MainWindow::onAddBookClicked);
    connect(ui->pushButtonEditBook, &QPushButton::clicked, this, &MainWindow::onEditBookClicked);
    connect(ui->pushButtonDeleteBook, &QPushButton::clicked, this, &MainWindow::onDeleteBookClicked);
    connect(ui->pushButtonAddShelf, &QPushButton::clicked, this, &MainWindow::onAddShelfClicked);
    connect(ui->pushButtonDeleteShelf, &QPushButton::clicked, this, &MainWindow::onDeleteShelfClicked);
    connect(ui->comboBoxShelves, &QComboBox::currentTextChanged, this, &MainWindow::onShelfSelected);
    connect(ui->tableViewBooks->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &MainWindow::onBookSelected);
    connect(ui->comboBoxFilterStatus, &QComboBox::currentTextChanged,
            this, &MainWindow::refreshBookList);

    refreshShelfList();
    refreshBookList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshBookList()
{
    QString shelf = ui->comboBoxShelves->currentText();
    QString status = ui->comboBoxFilterStatus->currentText();

    QString filter;
    if (shelf != "Все полки" && !shelf.isEmpty()) {
        filter = QString("shelf = '%1'").arg(shelf);
    }

    if (status != "Все" && !status.isEmpty()) {
        if (!filter.isEmpty()) filter += " AND ";
        filter += QString("status = '%1'").arg(status);
    }

    bookModel->setFilter(filter);
    bookModel->select();
    ui->tableViewBooks->resizeColumnsToContents();
}

void MainWindow::refreshShelfList()
{
    ui->comboBoxShelves->clear();
    ui->comboBoxBookShelf->clear();
    ui->comboBoxShelves->addItem("Все полки");
    ui->comboBoxBookShelf->addItem("");

    QSqlQuery shelves = dbManager->getAllShelves();
    while (shelves.next()) {
        QString shelfName = shelves.value(0).toString();
        ui->comboBoxShelves->addItem(shelfName);
        ui->comboBoxBookShelf->addItem(shelfName);
    }
}

void MainWindow::onAddBookClicked()
{
    QString title = ui->lineEditTitle->text().trimmed();
    QString author = ui->lineEditAuthor->text().trimmed();
    int pages = ui->spinBoxPages->value();
    int year = ui->spinBoxYear->value();
    QString status = ui->comboBoxStatus->currentText();
    int rating = ui->comboBoxRating->currentText().toInt();
    QString shelf = ui->comboBoxBookShelf->currentText();

    if (title.isEmpty() || author.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Заполните название и автора");
        return;
    }

    if (dbManager->addBook(title, author, pages, year, status, rating, shelf)) {
        QMessageBox::information(this, "Успех", "Книга добавлена");
        refreshBookList();

        // Очистка полей
        ui->lineEditTitle->clear();
        ui->lineEditAuthor->clear();
        ui->spinBoxPages->setValue(0);
        ui->spinBoxYear->setValue(1900);
        ui->comboBoxStatus->setCurrentIndex(0);
        ui->comboBoxRating->setCurrentIndex(0);
        ui->comboBoxBookShelf->setCurrentIndex(0);

        currentBookId = -1;
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить книгу");
    }
}

void MainWindow::onEditBookClicked()
{
    if (currentBookId == -1) {
        QMessageBox::warning(this, "Внимание", "Выберите книгу для редактирования");
        return;
    }

    QString title = ui->lineEditTitle->text().trimmed();
    QString author = ui->lineEditAuthor->text().trimmed();
    int pages = ui->spinBoxPages->value();
    int year = ui->spinBoxYear->value();
    QString status = ui->comboBoxStatus->currentText();
    int rating = ui->comboBoxRating->currentText().toInt();
    QString shelf = ui->comboBoxBookShelf->currentText();

    if (title.isEmpty() || author.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Заполните название и автора");
        return;
    }

    if (dbManager->updateBook(currentBookId, title, author, pages, year, status, rating, shelf)) {
        QMessageBox::information(this, "Успех", "Книга обновлена");
        refreshBookList();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось обновить книгу");
    }
}

void MainWindow::onDeleteBookClicked()
{
    if (currentBookId == -1) {
        QMessageBox::warning(this, "Внимание", "Выберите книгу для удаления");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение",
                                  "Удалить выбранную книгу?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteBook(currentBookId)) {
            QMessageBox::information(this, "Успех", "Книга удалена");
            currentBookId = -1;

            ui->lineEditTitle->clear();
            ui->lineEditAuthor->clear();
            ui->spinBoxPages->setValue(0);
            ui->spinBoxYear->setValue(1900);
            ui->comboBoxStatus->setCurrentIndex(0);
            ui->comboBoxRating->setCurrentIndex(0);
            ui->comboBoxBookShelf->setCurrentIndex(0);

            refreshBookList();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить книгу");
        }
    }
}

void MainWindow::onAddShelfClicked()
{
    QString shelfName = ui->lineEditShelf->text().trimmed();

    if (shelfName.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Введите название полки");
        return;
    }

    if (dbManager->addShelf(shelfName)) {
        QMessageBox::information(this, "Успех", "Полка добавлена");
        ui->lineEditShelf->clear();
        refreshShelfList();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить полку");
    }
}

void MainWindow::onDeleteShelfClicked()
{
    QString shelfName = ui->comboBoxShelves->currentText();

    if (shelfName == "Все полки" || shelfName.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Выберите полку для удаления");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение",
                                  "Удалить полку \"" + shelfName + "\"?\n"
                                  "Книги на этой полке не будут удалены.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (dbManager->deleteShelf(shelfName)) {
            QMessageBox::information(this, "Успех", "Полка удалена");
            refreshShelfList();
            refreshBookList();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось удалить полку");
        }
    }
}

void MainWindow::onShelfSelected()
{
    refreshBookList();
}

void MainWindow::onBookSelected(const QModelIndex &index)
{
    if (!index.isValid()) {
        currentBookId = -1;
        return;
    }

    int row = index.row();
    currentBookId = bookModel->data(bookModel->index(row, 0)).toInt();

    ui->lineEditTitle->setText(bookModel->data(bookModel->index(row, 1)).toString());
    ui->lineEditAuthor->setText(bookModel->data(bookModel->index(row, 2)).toString());
    ui->spinBoxPages->setValue(bookModel->data(bookModel->index(row, 3)).toInt());
    ui->spinBoxYear->setValue(bookModel->data(bookModel->index(row, 4)).toInt());

    QString status = bookModel->data(bookModel->index(row, 5)).toString();
    ui->comboBoxStatus->setCurrentText(status);

    int rating = bookModel->data(bookModel->index(row, 6)).toInt();
    if (rating > 0 && rating <= 10) {
        ui->comboBoxRating->setCurrentText(QString::number(rating));
    } else {
        ui->comboBoxRating->setCurrentIndex(0);
    }

    QString shelf = bookModel->data(bookModel->index(row, 7)).toString();
    ui->comboBoxBookShelf->setCurrentText(shelf);
}

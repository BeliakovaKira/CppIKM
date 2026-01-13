#include "databasemanager.h"
#include <QDir>
#include <QDebug>
#include <QApplication>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    // Инициализация базы данных
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // Указываем путь к базе данных в директории приложения
    QString appDir = QApplication::applicationDirPath();
    QString dbPath = appDir + "/reading_diary.db";
    m_db.setDatabaseName(dbPath);

    qDebug() << "Database path:" << dbPath;
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    if (!m_db.open()) {
        qDebug() << "Error opening database:" << m_db.lastError();
        return false;
    }

    QSqlQuery query;

    // Создание таблицы книг
    QString createBooksTable =
        "CREATE TABLE IF NOT EXISTS books ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "title TEXT NOT NULL, "
        "author TEXT NOT NULL, "
        "pages INTEGER, "
        "year INTEGER, "
        "status TEXT, "
        "rating INTEGER DEFAULT 0, "
        "shelf TEXT)";

    if (!query.exec(createBooksTable)) {
        qDebug() << "Error creating books table:" << query.lastError();
        return false;
    }

    // Создание таблицы полок
    QString createShelvesTable =
        "CREATE TABLE IF NOT EXISTS shelves ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT UNIQUE NOT NULL)";

    if (!query.exec(createShelvesTable)) {
        qDebug() << "Error creating shelves table:" << query.lastError();
        return false;
    }

    // Добавление стандартных полок, если их нет
    QStringList defaultShelves = {};
    for (const QString &shelf : defaultShelves) {
        query.prepare("INSERT OR IGNORE INTO shelves (name) VALUES (:name)");
        query.bindValue(":name", shelf);
        if (!query.exec()) {
            qDebug() << "Error inserting default shelf" << shelf << ":" << query.lastError();
        }
    }

    return true;
}

bool DatabaseManager::addBook(const QString &title, const QString &author,
                             int pages, int year, const QString &status,
                             int rating, const QString &shelf)
{
    QSqlQuery query;
    query.prepare(
        "INSERT INTO books (title, author, pages, year, status, rating, shelf) "
        "VALUES (:title, :author, :pages, :year, :status, :rating, :shelf)"
    );

    query.bindValue(":title", title);
    query.bindValue(":author", author);
    query.bindValue(":pages", pages);
    query.bindValue(":year", year);
    query.bindValue(":status", status);
    query.bindValue(":rating", rating);
    query.bindValue(":shelf", shelf);

    return query.exec();
}

bool DatabaseManager::updateBook(int id, const QString &title, const QString &author,
                               int pages, int year, const QString &status,
                               int rating, const QString &shelf)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE books SET title = :title, author = :author, pages = :pages, "
        "year = :year, status = :status, rating = :rating, shelf = :shelf "
        "WHERE id = :id"
    );

    query.bindValue(":id", id);
    query.bindValue(":title", title);
    query.bindValue(":author", author);
    query.bindValue(":pages", pages);
    query.bindValue(":year", year);
    query.bindValue(":status", status);
    query.bindValue(":rating", rating);
    query.bindValue(":shelf", shelf);

    return query.exec();
}

bool DatabaseManager::deleteBook(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM books WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

QSqlQuery DatabaseManager::getAllBooks()
{
    QSqlQuery query("SELECT * FROM books ORDER BY title");
    return query;
}

QSqlQuery DatabaseManager::getBooksByShelf(const QString &shelf)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM books WHERE shelf = :shelf ORDER BY title");
    query.bindValue(":shelf", shelf);
    query.exec();
    return query;
}

bool DatabaseManager::addShelf(const QString &name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO shelves (name) VALUES (:name)");
    query.bindValue(":name", name);
    return query.exec();
}

bool DatabaseManager::deleteShelf(const QString &name)
{
    QSqlQuery query;
    query.prepare("DELETE FROM shelves WHERE name = :name");
    query.bindValue(":name", name);
    return query.exec();
}

QSqlQuery DatabaseManager::getAllShelves()
{
    QSqlQuery query("SELECT name FROM shelves ORDER BY name");
    query.exec();
    return query;
}

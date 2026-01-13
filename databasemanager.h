#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initializeDatabase();

    bool addBook(const QString &title, const QString &author,
                 int pages, int year, const QString &status,
                 int rating, const QString &shelf);
    bool updateBook(int id, const QString &title, const QString &author,
                   int pages, int year, const QString &status,
                   int rating, const QString &shelf);
    bool deleteBook(int id);
    QSqlQuery getAllBooks();
    QSqlQuery getBooksByShelf(const QString &shelf);

    bool addShelf(const QString &name);
    bool deleteShelf(const QString &name);
    QSqlQuery getAllShelves();

private:
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H

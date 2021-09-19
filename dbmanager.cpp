#include "dbmanager.h"
#include <QDebug>

DbManager::DbManager()
{

}

DbManager::~DbManager()
{
    closeDataBase();
}
void DbManager::closeDataBase()
{
    if (m_db.open()) {
        m_db.close();
    }


    m_db.removeDatabase( QSqlDatabase::defaultConnection);
}

QStringList DbManager::getTableItems(QString table)
{
    QSqlQuery query(m_db);

    query.exec("PRAGMA table_info(" + table +")");

    QStringList ret = {};

    while (query.next()) {
        ret.append(query.value(1).toString());
    }

    return ret;
}

void DbManager::addColumnToTable(QString table, QString column, QString data_type)
{
    QSqlQuery query(m_db);
    query.exec(QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(table).arg(column).arg(data_type));
}

void DbManager::setupDatabase(QString path, QString name)
{
    if (m_db.isOpen()) {
        m_db.close();
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", name);
    m_db.setDatabaseName(path);

    if (!m_db.open())
    {
        qDebug() << "Error: connection with database fail";
        return;
    } else
    {
        qDebug() << "Database: connection ok";
    }
}

QStringList DbManager::getAllTableNames()
{
    QString query_string = "SELECT name from sqlite_master";

    QSqlQuery query(m_db);
    query.exec(query_string);

    QStringList ret;

    while (query.next())
    {
        ret.append(query.value(0).toString());
    }

    return ret;
}

void DbManager::createTable(QString table_name, QList<SQLiteColumnInfo> column_info)
{
    QString query_string = "CREATE TABLE %1 %2";

    QString item_data_string = "(";
    int counter = 0;
    for (auto item : column_info){

        if (counter == 0) {
            item_data_string = item_data_string.append(item.name + " " + item.data_type + " PRIMARY KEY,");
        } else {
            item_data_string = item_data_string.append(item.name + " " + item.data_type + " DEFAULT " + item.default_val + ",");
        }

       counter++;
    }

    item_data_string = item_data_string.remove(item_data_string.size()-1, 1);
    item_data_string = item_data_string.append(")");

    query_string = query_string.arg(table_name).arg(item_data_string);


     QSqlQuery query(m_db);
     query.exec(query_string);
}

QList<QVariantList> DbManager::getDataFromTable(QString table_name, QList<SQLiteColumnInfo> items, QMap<QString, QVariant> map, QString sort_name, QString sqlite_filter)
{

    QString query_string = "SELECT %1 FROM %2 %3 %4";

    QString item_data_string;

    for (auto item : items){
        item_data_string = item_data_string.append(item.name + ",");
    }

    item_data_string = item_data_string.remove(item_data_string.size()-1, 1);


    QString where_string = sqlite_filter;
    if (map.size() > 0 && where_string.isEmpty()) {
        where_string = "WHERE (";

        for (auto item : map.keys()) {
            where_string = where_string.append(item + "=? AND ");
        }

        int index = where_string.lastIndexOf("AND");

        where_string.remove(index, where_string.length() - index);

        where_string = where_string.append(")");
    }

    QString sorting = "";
    if (!sort_name.isEmpty()) {
        sorting = "ORDER BY " + sort_name;
    }

    query_string = query_string.arg(item_data_string).arg(table_name).arg(where_string).arg(sorting);
    //qInfo() << query_string;
    QSqlQuery query(m_db);

    query.prepare(query_string);

   // qInfo() << query_string;

    if (sqlite_filter.isEmpty()) {
        for (auto &item : map.keys()) {
            query.addBindValue(map[item]);
        }
    }

    query.exec();

    QList<QVariantList> ret;

    while (query.next()) {
        QVariantList tmp = {};
        int counter = 0;
        for (auto item : items) {
            tmp.append(query.value(counter));
            counter++;
        }

        ret.append(tmp);
    }

    return ret;

}

QList<QVariantList> DbManager::getDataFromTable(QString table_name, QStringList items, QMap<QString, QVariant> map)
{

    QString query_string = "SELECT %1 FROM %2 %3";

    QString item_data_string;

    for (auto item : items){
        item_data_string = item_data_string.append(item + ",");
    }

    item_data_string = item_data_string.remove(item_data_string.size()-1, 1);


    QString where_string;
    if (map.size() > 0) {
        where_string = "WHERE (";

        for (auto item : map.keys()) {
            where_string = where_string.append(item + "=? AND ");
        }

        int index = where_string.lastIndexOf("AND");

        where_string.remove(index, where_string.length() - index);

        where_string = where_string.append(")");
    }

    query_string = query_string.arg(item_data_string).arg(table_name).arg(where_string);

    QSqlQuery query(m_db);

    query.prepare(query_string);

    for (auto item : map.keys()) {
        query.addBindValue(map[item]);
    }

    query.exec();

    QList<QVariantList> ret;

    while (query.next()) {
        QVariantList tmp = {};
        int counter = 0;
        for (auto item : items) {
            tmp.append(query.value(counter));
            counter++;
        }

        ret.append(tmp);
    }

    return ret;

}

bool DbManager::updateDataInTable(QString table_name, FilterMap data, FilterMap map )
{
    QString query_string = "UPDATE %1 SET %2 %3";

    QString item_data_string;

    for (auto &item : data.keys()){
        item_data_string = item_data_string.append(item + " =?,");
    }

    item_data_string = item_data_string.remove(item_data_string.size()-1, 1);


    QString where_string;
    if (map.size() > 0) {
        where_string = "WHERE (";

        for (auto item : map.keys()) {
            where_string = where_string.append(item + "=? AND ");
        }

        int index = where_string.lastIndexOf("AND");

        where_string.remove(index, where_string.length() - index);

        where_string = where_string.append(")");
    }

    query_string = query_string.arg(table_name).arg(item_data_string).arg(where_string);

    QSqlQuery query(m_db);

    query.prepare(query_string);

    for (auto &item : data.keys()) {
        query.addBindValue(data[item]);
    }

    for (auto &item : map.keys()) {
        query.addBindValue(map[item]);
    }

    query.exec();

    return true;
}

bool DbManager::deleteDataInTable(QString table_name, QMap<QString, QVariant> filter)
{
    QString query_string = "DELETE FROM %1 %2";

    QString item_data_string;


    QString where_string;
    if (filter.size() > 0) {
        where_string = "WHERE (";

        for (auto item : filter.keys()) {
            where_string = where_string.append(item + "=? AND ");
        }

        int index = where_string.lastIndexOf("AND");

        where_string.remove(index, where_string.length() - index);

        where_string = where_string.append(")");
    }

    query_string = query_string.arg(table_name).arg(where_string);

    QSqlQuery query(m_db);

    query.prepare(query_string);

    for (auto &item : filter.keys()) {
        query.addBindValue(filter[item]);
    }

    query.exec();

    return true;
}

int DbManager::addDataToTable(QString table_name, FilterMap data)
{
    QString query_string = "INSERT INTO %1 (%2) VALUES %3";

    QString item_data_string;

    for (auto &item : data.keys()){
        item_data_string = item_data_string.append(item + ",");
    }

    item_data_string = item_data_string.remove(item_data_string.size()-1, 1);


    QString values_string;
    if (data.size() > 0) {
        values_string = "(";

        for (auto item : data.keys()) {
            values_string = values_string.append(":" + item + ",");
        }

        values_string = values_string.remove(values_string.size()-1, 1);
        values_string = values_string.append(")");
    }

    query_string = query_string.arg(table_name).arg(item_data_string).arg(values_string);

    //qInfo() << query_string;

    QSqlQuery query(m_db);

    query.prepare(query_string);

    for (auto &item : data.keys()) {
        query.addBindValue(data[item]);
    }

    query.exec();

    return query.lastInsertId().toInt();
}

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
bool DbManager::updateDatabaseVersion()
{
    bool success = true;
    if (m_db.isOpen()) {
        return false;
    }

    QSqlQuery query(m_db);

    success &= query.prepare("PRAGMA user_version");
    success &= query.exec();
    success &= query.first();

    int version = query.value(0).toInt();
    int new_version = version;

    success &= query.exec("PRAGMA foreign_keys = ON");

    if (!success) {
        return false;
    }

    if(!m_db.transaction()) {
        return false;
    }

    bool updated_version = true;

    switch (version) {
    case 0: { // update version from 0 to 1

        // get all departments ids from employee table
        QList<int> employee_dep_id;

        QMap<int, QString> dep_id_to_name_map;
        dep_id_to_name_map[1] = "Director";
        dep_id_to_name_map[2] = "Sugar Cane";
        dep_id_to_name_map[3] = "Ranch";
        dep_id_to_name_map[4] = "House";
        dep_id_to_name_map[5] = "Workshop";
        dep_id_to_name_map[6] = "Ranch Patrol";
        dep_id_to_name_map[7] = "Hydropower";
        dep_id_to_name_map[8] = "Watchman Sugar Cane";
        dep_id_to_name_map[9] = "Watchman";

        // add all known departments to the table if they don't exist yet
        for (auto &dep_id : dep_id_to_name_map.keys()) {
            success &= query.prepare("SELECT id FROM department WHERE id=?");
            query.addBindValue(dep_id);
            success &=query.exec();

            if (!query.next()) {
                success &= query.prepare("INSERT INTO department (id,description,has_sdl) VALUES(?,?,?)");
                query.addBindValue(dep_id);
                query.addBindValue(dep_id_to_name_map[dep_id]);
                query.addBindValue(0);
                success &= query.exec();
            }

        }

        success &= query.exec("SELECT department FROM employees");
        while(query.next()) {
            int id = query.value(0).toInt();
            if (!employee_dep_id.contains(id)) {
                employee_dep_id.append(id);
            }
        }

        for (auto& dep_id : employee_dep_id) {
            success &= query.exec("SELECT id FROM department WHERE id=" + QString::number(dep_id));

            if (!query.next()) {
                // unknown department ID, we need to get rid of it
                success &= query.prepare("UPDATE employees SET department=? WHERE department=?");
                query.addBindValue(QVariant(QVariant::Int));
                query.addBindValue(dep_id);
                success &= query.exec();
            }
        }


        success &= query.prepare("CREATE TABLE employees2 (id INTEGER PRIMARY KEY, surname TEXT, name TEXT, nssf INTEGER DEFAULT 0,tipau INTEGER DEFAULT 0, sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0, department INTEGER DEFAULT 0, nssf_number TEXT, inactive INTEGER DEFAULT 0, is_active INTEGER DEFAULT 0, tin_number TEXT, nida_number TEXT, mobile_number TEXT, bank_account TEXT, bank_name TEXT, uuid TEXT, FOREIGN KEY(department) REFERENCES department(id))");

        success &= query.exec();
        success &= query.prepare("INSERT INTO employees2 SELECT * FROM employees");

        success &= query.exec();


        success &= query.exec("DROP TABLE employees");
        success &= query.exec("ALTER TABLE employees2 RENAME TO employees");

        new_version++;

        break;
    }

    default:
        // if we get in here then it means that the database was already at the newest version
        updated_version = false;

    }

    if (success) {
        success &= query.prepare(QString("PRAGMA user_version =") + QString::number(new_version));
        success &= query.exec();
    }

    if (success) {
        m_db.commit();
    } else {
        m_db.rollback();
    }

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

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

int DbManager::updateDatabaseVersion()
{
    bool success = true;
    if (!m_db.isOpen()) {
        return -1;
    }

    QSqlQuery query(m_db);

    success &= query.prepare("PRAGMA user_version");
    success &= query.exec();
    success &= query.first();

    int version = query.value(0).toInt();
    int new_version = version;

    success &= query.exec("PRAGMA foreign_keys = ON");

    if (!success) {
        return new_version;
    }

    if(!m_db.transaction()) {
        return new_version;
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

        success &= cleanupTableDependencyPriorForeignKey(QString("employees"), QString("department"), QString("department"), QString("id"), QVariant(QVariant::Int));

        QString new_table_sql_statement("CREATE TABLE employees2 \
                 (id INTEGER PRIMARY KEY NOT NULL, surname TEXT NOT NULL, name TEXT NOT NULL, \
                 nssf INTEGER DEFAULT 0,tipau INTEGER DEFAULT 0, sdl INTEGER DEFAULT 0,\
                 paye INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, \
                salary_fixed INTEGER DEFAULT 0, department INTEGER DEFAULT 0, nssf_number TEXT, \
                inactive INTEGER DEFAULT 0, is_active INTEGER DEFAULT 0, tin_number TEXT, \
                nida_number TEXT, mobile_number TEXT, bank_account TEXT, bank_name TEXT, \
                FOREIGN KEY(department) REFERENCES department(id))");
        success&= replaceTable(QString("employees"), new_table_sql_statement);

        // update daily records table
        success &= cleanupTableDependencyPriorForeignKey(QString("daily_record"), QString("location"), QString("location"), QString("id"), QVariant(QVariant::Int));

        new_table_sql_statement = QString("CREATE TABLE daily_record2 \
                 (id INTEGER PRIMARY KEY,date DATE DEFAULT NULL,employee_id INTEGER DEFAULT NULL,\
                work_type INTEGER DEFAULT NULL,pay INTEGER DEFAULT NULL,location INTEGER DEFAULT NULL,\
                description TEXT DEFAULT NULL,location_string TEXT DEFAULT NULL,\
                work_quantity_description TEXT DEFAULT NULL,work_quantity REAL DEFAULT NULL, \
                FOREIGN KEY(employee_id) REFERENCES employees(id), FOREIGN KEY(work_type) REFERENCES work_type(id), \
                FOREIGN KEY(location) REFERENCES location(id))");

        success &= replaceTable(QString("daily_record"), new_table_sql_statement);

        success &= cleanupTableDependencyPriorForeignKey(QString("payroll_entry"), QString("payroll_id"), QString("payroll_list"), QString("id"), QVariant(QVariant::Int));

        new_table_sql_statement = QString("CREATE TABLE payroll_entry2 \
                (payroll_id INTEGER, employee_id INTEGER, nssf INTEGER DEFAULT 0, tipau INTEGER DEFAULT 0,\
                sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, days_normal INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, \
                days_special INTEGER DEFAULT 0, rate_special INTEGER DEFAULT 0,sugar_cane_related INTEGER DEFAULT 0, \
                auxilliary INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0, bonus INTEGER, overtime INTEGER,\
                FOREIGN KEY(employee_id) REFERENCES employees(id), FOREIGN KEY(payroll_id) REFERENCES payroll_list(id))");

        success &= replaceTable(QString("payroll_entry"), new_table_sql_statement);
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
        new_version = version;
    }

    return new_version;

}

bool DbManager::replaceTable(QString table_name, QString sql_create_statement)
{
    bool success = true;
    QSqlQuery query(m_db);

    QString existing_col_string;

    success &= query.exec(QString("PRAGMA table_info(%1)").arg(table_name));

    while(query.next()) {
        existing_col_string.append(query.value(1).toString() + QString(","));
    }

    int last_pos = existing_col_string.lastIndexOf(QChar(','));
    existing_col_string = existing_col_string.left(last_pos);

    success &= query.prepare(sql_create_statement);
    success &= query.exec();

    QString query_string = QString("INSERT INTO %1 (%2) SELECT %2 FROM %3").arg(table_name+QString("2")).arg(existing_col_string).arg(table_name);
    success &= query.prepare(query_string);
    success &= query.exec();
    success &= query.exec(QString("DROP TABLE %1").arg(table_name));
    success &= query.exec(QString("ALTER TABLE %1 RENAME TO %2").arg(table_name+QString("2")).arg(table_name));

    return success;
}

bool DbManager::cleanupTableDependencyPriorForeignKey(QString childTable, QString childCol, QString parentTable, QString parentCol, QVariant default_val)
{
    bool success = true;
    QSqlQuery query(m_db);
    success &= query.exec(QString("SELECT %1 FROM %2").arg(childCol).arg(childTable));

    QList<int> foreign_keys;

    while(query.next()) {
        int val = query.value(0).toInt();
        if (!foreign_keys.contains(val)) {
            foreign_keys.append(val);
        }
    }

    for (auto &id : foreign_keys) {
        success &= query.exec(QString("SELECT %1 FROM %2 WHERE %1=%3").arg(parentCol).arg(parentTable).arg(QString::number(id)));

        if (!query.next()) {
            success &= query.prepare(QString("UPDATE %1 SET %2=? WHERE %2=?").arg(childTable).arg(childCol));
            query.addBindValue(default_val);
            query.addBindValue(id);
            success &= query.exec();
        }
    }

    return success;
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

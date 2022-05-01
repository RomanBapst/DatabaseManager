#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include <QString>
#include <QObject>
#include <QDate>
#include <QVariant>
#include <QDebug>

typedef QMap<QString, QVariant> KeyValueMap;

struct SQLiteColumnInfo {
    QString name;
    QString data_type;
    QString default_val;
};

struct TableEntryDependency {
    QString dependent_table;
    QString dependent_field_name;
    QString table_name;
    QString field_name;
};

class DbManager: public QObject
{
    Q_OBJECT
public:
    DbManager();

    ~DbManager();

    typedef QMap<QString, QVariant> FilterMap;

    void setupDatabase(QString path, QString name="main");

    void closeDataBase();

    QStringList getTableItems(QString table);

    QStringList getAllTableNames();

    void createTable(QString table_name, QList<SQLiteColumnInfo> column_info);

    void addColumnToTable(QString table, QString column, QString data_type, QVariant default_val = QVariant());

    void triggerDataBaseChanged() { emit dataBaseChanged(); }

    QList<QVariantList> getDataFromTable(QString table_name, QList<SQLiteColumnInfo> items, QMap<QString, QVariant> map, QString sort_name="", QString sqlite_filter="");

    QList<QVariantList> getDataFromTable(QString table_name, QStringList items, QMap<QString, QVariant> map);

    bool updateDataInTable(QString table_name, FilterMap data, FilterMap map );

    int addDataToTable(QString table_name, FilterMap data);

    bool deleteDataInTable(QString table_name, QMap<QString, QVariant> filter);

    bool hasDependantEntry(QString table_name, QString field_name, int id)
    {
        for (auto &item : _table_dependency_list) {
            if (item.table_name == table_name && item.field_name == field_name) {
                QList<QVariantList> ret = getDataFromTable(item.dependent_table, QList<SQLiteColumnInfo>{SQLiteColumnInfo{.name=item.dependent_field_name,.data_type="",.default_val=""}}, KeyValueMap{{item.dependent_field_name, id}});
                if (ret.length() > 0) {
                    return true;
                }
            }
        }

        return false;
    }

signals:

    void dataBaseChanged();

private:
    QSqlDatabase m_db;

    QList<TableEntryDependency> _table_dependency_list = {
        TableEntryDependency {
            .dependent_table = "employees",
            .dependent_field_name = "department",
            .table_name = "department",
            .field_name = "id"
        },
        TableEntryDependency {
            .dependent_table = "payroll_entry",
            .dependent_field_name = "employee_id",
            .table_name = "employees",
            .field_name = "id"
        },

        TableEntryDependency {
            .dependent_table = "daily_record",
            .dependent_field_name = "employee_id",
            .table_name = "employees",
            .field_name = "id"
        }
    };
};

#endif // DBMANAGER_H

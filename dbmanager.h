#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include <QString>
#include <QObject>
#include <QDate>

struct SQLiteColumnInfo {
    QString name;
    QString data_type;
    QString default_val;
};

typedef QMap<QString, QVariant> KeyValueMap;

class DbManager: public QObject
{
    Q_OBJECT
public:
    DbManager(QObject *parent);

    ~DbManager();

    typedef QMap<QString, QVariant> FilterMap;

    void setupDatabase(QString path, QString name="main");

    void closeDataBase();

    int getDatabaseVersion() { return _database_version; }

    int updateDatabaseVersion();
    QStringList getTableItems(QString table);

    QStringList getAllTableNames();

    void createTable(QString table_name, QList<SQLiteColumnInfo> column_info);

    void addColumnToTable(QString table, QString column, QString data_type);

    void triggerDataBaseChanged() { emit dataBaseChanged(); }

    QList<QVariantList> getDataFromTable(QString table_name, QList<SQLiteColumnInfo> items, QMap<QString, QVariant> map, QString sort_name="", QString sqlite_filter="");

    QList<QVariantList> getDataFromTable(QString table_name, QStringList items, QMap<QString, QVariant> map);

    bool updateDataInTable(QString table_name, FilterMap data, FilterMap map );

    int addDataToTable(QString table_name, FilterMap data);

    bool deleteDataInTable(QString table_name, QMap<QString, QVariant> filter);

signals:

    void dataBaseChanged();

private:
    QSqlDatabase m_db;

    int _database_version = -1;

    bool replaceTable(QString table_name, QString sql_create_statement);
    bool cleanupTableDependencyPriorForeignKey(QString childTable, QString childCol, QString parentTable, QString parentCol, QVariant default_val);
    bool enforLocationIdInDailyRecordsTable();
};

#endif // DBMANAGER_H

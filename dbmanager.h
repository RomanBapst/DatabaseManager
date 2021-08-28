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
    DbManager();

    ~DbManager();

    typedef QMap<QString, QVariant> FilterMap;

    struct EmployeeInfo {
        int id;
        QString surname;
        QString name;
        bool nssf;
        bool tipau;
        bool sdl;
        bool paye;
        int rate_per_day;
        int salary_fixed;
        int department;
        QString nssf_number;
        bool is_active;
        QString tin_number;
        QString nida_number;
    };

    struct PayrollInfo {
        int id;
        QDate start_date;
        QDate end_date;
        QString name;
    };

    struct PayrollEntryInfo {
        int payroll_id;
        int employee_id;
        bool has_nssf;
        bool has_tipau;
        bool has_sdl;
        bool has_paye;
        int days_normal;
        int rate_normal;
        int days_special;
        int rate_special;
        int sugar_cane_related;
        int auxilliary_amount;
        int salary_fixed;
        int bonus;
        int overtime;
        QString department_name;
    };

    struct CompanyInfo {
        int id;
        QString name;
        QString postal_number;
        QString postal_city;
        QString location;
        QString phone_number;
        QString email;
        QString tin_number;
        QString nssf_reg_nr;
        QString nssf_ctrl_nr;
    };

    struct WorkType {
        int id;
        QString description;
        int default_pay;
        bool is_active;

         friend bool operator==(const WorkType& lhs, const WorkType& rhs) {
             return rhs.id == lhs.id;
         }
    };

    struct DailyRecord {
        int id;
        QDate date;
        int work_type_id;
        int employee_id;
        QString description;
        int pay;
        int location;

        friend bool operator==(const DailyRecord&lhs, const DailyRecord& rhs) {
            return (rhs.date == lhs.date && rhs.work_type_id == lhs.work_type_id && rhs.employee_id == lhs.employee_id);
        }
    };

    struct WorkCategory {
        QString hash;
    };

    void setupDatabase(QString path, QString name="main");

    void closeDataBase();

    QStringList getTableItems(QString table);

    void addColumnToTable(QString table, QString column, QString data_type);

    void triggerDataBaseChanged() { emit dataBaseChanged(); }

    void addEmployee(EmployeeInfo info, QString table_name="employees");

    void setCompanyInfo(CompanyInfo info);

    CompanyInfo getCompanyInfo();

    int addPayroll(PayrollInfo);

    void addWorkType(WorkType work_type);

    void addDailyRecord(DbManager::DailyRecord record);

    void updatePayroll(PayrollInfo info);

    void addPayrollEntry(int payroll_id, PayrollEntryInfo info);

    void updatePayrollEntry(int payroll_id, PayrollEntryInfo info);

    void removeByID(int id);

    void updateByID(EmployeeInfo info);

    void removePayrollByID(int payroll_id);

    QList<EmployeeInfo> getAllEmployees();

    QList<WorkType> getAllWorkItems();

    QList<DbManager::DailyRecord> getAllDailyRecords();

    EmployeeInfo getEmployeeInfo(int id, bool&success, QString table="employees");

    WorkType getWorkType(int id);

    DailyRecord getDailyRecord(int id);

    QList<EmployeeInfo> getMatchName(QString name);

    QList<PayrollInfo> getAllPayrolls();

    QList<PayrollEntryInfo> getPayrollEntryInfo(int payroll_id);

    void removePayrollEntry(int payroll_id, int employee_id);

    DbManager::PayrollInfo getPayrollInfoByID(int id);

    QList<DbManager::DailyRecord> getDailyRecordForDateAndEmployeeID(QDate date, int employee_id);

    QList<DbManager::DailyRecord> getDailyRecordForDateAndWorkType(QDate date, int work_id);

    QList<DbManager::DailyRecord> getDailyRecordForDate(QDate date);

    QList<DbManager::WorkType> getWorkTypeListForDate(QDate date);

    DbManager::EmployeeInfo getEmployeeFromName(QString surname, QString name);

    DbManager::WorkType getWorkTypeFromName(QString name);

    void removeDailyRecord(int id);

    void updateDailyRecord(DbManager::DailyRecord record);

    DbManager::EmployeeInfo getEmployeeByID(int id);

    QList<QVariantList> getDataFromTable(QString table_name, QList<SQLiteColumnInfo> items, QMap<QString, QVariant> map, QString sort_name="", QString sqlite_filter="");

    QList<QVariantList> getDataFromTable(QString table_name, QStringList items, QMap<QString, QVariant> map);

    bool updateDataInTable(QString table_name, FilterMap data, FilterMap map );

    int addDataToTable(QString table_name, FilterMap data);

    bool deleteDataInTable(QString table_name, QMap<QString, QVariant> filter);

signals:

    void dataBaseChanged();

private:
    QSqlDatabase m_db;

    QStringList company_info_header = {"id", "name", "postal_number", "postal_city", "location", "phone_number", "email", "tin_number", "nssf_nr", "nssf_ctrl_nr"};
    QStringList employee_header{"id", "surname", "name", "nssf", "tipau", "sdl", "paye", "rate_normal", "salary_fixed", "department", "nssf_number", "is_active", "tin_number", "nida_number"};
    QStringList payroll_info_header = {"id", "start_date", "end_date", "name"};
    QStringList payroll_entry_header = {"payroll_id", "employee_id", "days_normal", "rate_normal", "days_special", "rate_special", "nssf", "tipau", "sdl", "paye", "auxilliary", "sugar_cane_related", "salary_fixed", "bonus", "overtime", "department_name"};
    QStringList daily_record_header = {"id", "date", "employee_id", "work_type", "pay", "location", "description"};
    QStringList work_type_header = {"id", "default_pay", "description", "is_active"};

    QStringList department_header = {"id", "description", "has_sdl"};
    QStringList department_types = {"INTEGER", "TEXT", "INTEGER"};

    CompanyInfo getCompanyInfoFromVariantList(QVariantList list);
    EmployeeInfo getEmployeeInfoFromVariantList(QVariantList list);
    PayrollInfo getPayrollInfoFromVariantList(QVariantList list);
    PayrollEntryInfo getPayrollEntryInfoFromVariantList(QVariantList list);
    DailyRecord getDailyRecordFromVariantList(QVariantList list);
    WorkType getWorkTypeFromVariantList(QVariantList list);

    QMap<QString, QVariant> companyInfoToVariantMap(DbManager::CompanyInfo info);
    QMap<QString, QVariant> employeeInfoToVariantMap(DbManager::EmployeeInfo info);
    QMap<QString, QVariant> payrollInfoToVariantMap(DbManager::PayrollInfo info);
    QMap<QString, QVariant> payrollEntryInfoToVariantMap(DbManager::PayrollEntryInfo info);
    QMap<QString, QVariant> dailyRecordToVariantMap(DbManager::DailyRecord info);
    QMap<QString, QVariant> workTypeToVariantMap(DbManager::WorkType info);

    void createTable(QString table_name, QStringList header, QStringList data_types);

};

#endif // DBMANAGER_H

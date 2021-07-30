#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include <QString>
#include <QObject>
#include <QDate>


class DbManager: public QObject
{
    Q_OBJECT
public:
    DbManager();

    ~DbManager();

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
    };

    struct CompanyInfo {
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

    void setupDatabase(QString path, QString name="main");

    void closeDataBase();

    void addEmployee(EmployeeInfo info);

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

    EmployeeInfo getEmployeeInfo(int id);

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
signals:

    void dataBaseChanged();

private:
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H

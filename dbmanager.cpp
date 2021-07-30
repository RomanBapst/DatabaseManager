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
    } else
    {
        qDebug() << "Database: connection ok";

        QSqlQuery query(m_db);

        query.prepare("CREATE TABLE employees (id INTEGER PRIMARY KEY, surname TEXT, name TEXT, nssf INTEGER DEFAULT 0,"
                        "tipau INTEGER DEFAULT 0, sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0, department INTEGER DEFAULT 0, nssf_number TEXT)");
        query.exec();

        if(!query.isActive()) {
            qWarning() << "ERROR: " << query.lastError().text();
        }

        query.exec("PRAGMA table_info(employees)");

        bool found_nssf_nr = false;
        bool found_is_active = false;
        bool found_tin_nr = false;
        bool found_nida_nr = false;
        while (query.next()) {
            QString column_name = query.value(1).toString();

            if (column_name.compare(QString("nssf_number")) == 0) {
                found_nssf_nr = true;
            }

            if (column_name.compare(QString("is_active")) == 0) {
                found_is_active = true;
            }

            if (column_name.compare(QString("tin_number")) == 0) {
                found_tin_nr = true;
            }

            if (column_name.compare(QString("nida_number")) == 0) {
                found_nida_nr = true;
            }
        }

        if (!found_nssf_nr) {
            query.exec("ALTER TABLE employees ADD COLUMN nssf_number TEXT");
        }

        if (!found_is_active) {
            query.exec("ALTER TABLE employees ADD COLUMN is_active INTEGER DEFAULT 0");
        }

        if (!found_tin_nr) {
            query.exec("ALTER TABLE employees ADD COLUMN tin_number TEXT");
        }

        if (!found_nida_nr) {
            query.exec("ALTER TABLE employees ADD COLUMN nida_number TEXT");
        }

        if(!query.isActive()) {
            qWarning() << "ERROR: " << query.lastError().text();
        }

        query.prepare("CREATE TABLE payroll_list (id INTEGER PRIMARY KEY, start_date, end_date DATE, name TEXT)");
        query.exec();

        if(!query.isActive()) {
            qWarning() << "ERROR: " << query.lastError().text();
        }

        query.prepare("CREATE TABLE payroll_entry (payroll_id INTEGER, employee_id INTEGER, nssf INTEGER DEFAULT 0, tipau INTEGER DEFAULT 0,"
                      "sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, days_normal INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, days_special INTEGER DEFAULT 0, rate_special INTEGER DEFAULT 0,"
                      "sugar_cane_related INTEGER DEFAULT 0,  auxilliary INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0)");
        query.exec();

        if(!query.isActive()) {
            qWarning() << "ERROR: " << query.lastError().text();
        }


        query.exec("PRAGMA table_info(payroll_entry)");

        bool found_bonus = false;
         bool found_overtime = false;

        while (query.next()) {
            QString column_name = query.value(1).toString();

            if (column_name.compare(QString("bonus")) == 0) {
                found_bonus = true;
            }

            if (column_name.compare(QString("overtime")) == 0) {
                found_overtime = true;
            }
        }

        if (!found_bonus) {
            query.exec("ALTER TABLE payroll_entry ADD COLUMN bonus INTEGER");
        }

        if (!found_overtime) {
            query.exec("ALTER TABLE payroll_entry ADD COLUMN overtime INTEGER");
        }

        if(!query.isActive()) {
            qWarning() << "ERROR: " << query.lastError().text();
        }

         query.prepare("CREATE TABLE company_info (id INTEGER PRIMARY KEY, name TEXT, postal_number TEXT, postal_city TEXT, location TEXT, phone_number TEXT, email TEXT, tin_number TEXT)");

         query.exec();

         if(!query.isActive()) {
             qWarning() << "ERROR: " << query.lastError().text();
         }

         query.exec("PRAGMA table_info(payroll_entry)");

         bool found_nssf_reg_nr = false;
         bool found_nssf_ctrl_nr = false;
         while (query.next()) {
             QString column_name = query.value(1).toString();

             if (column_name.compare(QString("nssf_nr")) == 0) {
                 found_nssf_reg_nr = true;
             }

             if (column_name.compare(QString("nssf_ctrl_nr")) == 0) {
                 found_nssf_ctrl_nr = true;
             }
         }

         if (!found_nssf_reg_nr) {
             query.exec("ALTER TABLE company_info ADD COLUMN nssf_nr TEXT");
         }

         if (!found_nssf_ctrl_nr) {
             query.exec("ALTER TABLE company_info ADD COLUMN nssf_ctrl_nr TEXT");
         }

         query.prepare("CREATE TABLE daily_record (id INTEGER PRIMARY KEY, date DATE, employee_id INTEGER, work_type INTEGER, pay INTEGER, location INTEGER, description TEXT)");
         query.exec();

         if(!query.isActive()) {
             qWarning() << "ERROR: " << query.lastError().text();
         }

         query.prepare("CREATE TABLE work_type (id INTEGER PRIMARY KEY, description TEXT, default_pay INTEGER, is_active INTEGER)");
         query.exec();

         if(!query.isActive()) {
             qWarning() << "ERROR: " << query.lastError().text();
         }


    }
}

void DbManager::addEmployee(DbManager::EmployeeInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO employees (surname, name, nssf, tipau, sdl, paye, rate_normal, salary_fixed, department, nssf_number, is_active, tin_number, nida_number) "
                      "VALUES (:surname, :name, :nssf, :tipau, :sdl, :paye, :rate_normal, :salary_fixed, :department, :nssf_number, :is_active, :tin_number, :nida_number)");

   query.bindValue(":surname", info.surname);
   query.bindValue(":name", info.name);
   query.bindValue(":nssf", info.nssf ? 1 : 0);
   query.bindValue(":tipau", info.tipau ? 1 : 0);
   query.bindValue(":sdl", info.sdl ? 1 : 0);
   query.bindValue(":paye", info.paye ? 1 : 0);
   query.bindValue(":rate_normal", info.rate_per_day);
   query.bindValue(":salary_fixed", info.salary_fixed);
   query.bindValue(":department", info.department);
   query.bindValue(":nssf_number", info.nssf_number);
   query.bindValue(":is_active", info.is_active);
   query.bindValue(":tin_number", info.tin_number);
   query.bindValue(":nida_number", info.nida_number);
   query.exec();

   emit dataBaseChanged();
}

void DbManager::setCompanyInfo(DbManager::CompanyInfo info)
{
    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM company_info");

    query.first();

    if (query.value(0).toInt() > 0) {
        qInfo() << "going in here";
        query.prepare("UPDATE company_info SET name = ?, postal_number = ?, postal_city = ?, location = ?, phone_number = ?, email = ?, tin_number = ?, nssf_nr = ?, nssf_ctrl_nr = ? WHERE id = ?");
        query.addBindValue(info.name);
        query.addBindValue(info.postal_number);
        query.addBindValue(info.postal_city);
        query.addBindValue(info.location);
        query.addBindValue(info.phone_number);
        query.addBindValue(info.email);
        query.addBindValue(info.tin_number);
        query.addBindValue(info.nssf_reg_nr);
        query.addBindValue(info.nssf_ctrl_nr);
        query.addBindValue(1);  // there will only ever be one entry and the ID is 1
        query.exec();
         qDebug() << "SqLite error:" << query.lastError().text();
    } else {
        query.prepare("INSERT INTO company_info (name, postal_number, postal_city, location, phone_number, email, tin_number )"
                      "VALUES (:name, :postal_number, :postal_city, :location, :phone_number, :email, :tin_number)");

        query.bindValue(":name", info.name);
        query.bindValue(":postal_number", info.postal_number);
        query.bindValue(":postal_city", info.postal_city);
        query.bindValue(":location", info.location);
        query.bindValue(":phone_number", info.phone_number);
        query.bindValue(":email", info.email);
        query.bindValue(":tin_number", info.tin_number);

        query.exec();
    }
}

DbManager::CompanyInfo DbManager::getCompanyInfo()
{
    CompanyInfo ret = {};
    QSqlQuery query(m_db);
    query.exec("SELECT id, name, postal_number, postal_city, location, phone_number, email, tin_number, nssf_nr, nssf_ctrl_nr FROM company_info");

    while (query.next()) {

        ret.name = query.value(1).toString();
        ret.postal_number = query.value(2).toString();
        ret.postal_city = query.value(3).toString();
        ret.location = query.value(4).toString();
        ret.phone_number = query.value(5).toString();
        ret.email = query.value(6).toString();
        ret.tin_number = query.value(7).toString();
        ret.nssf_reg_nr = query.value(8).toString();
        ret.nssf_ctrl_nr = query.value(9).toString();

      }

    return ret;
}

int DbManager::addPayroll(PayrollInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO payroll_list (start_date, end_date, name) "
                      "VALUES (:start_date, :end_date, :name)");

   query.bindValue(":start_date", info.start_date.toString("yyyy-MM-dd"));
   query.bindValue(":end_date", info.end_date.toString("yyyy-MM-dd"));
   query.bindValue(":name", info.name);

   query.exec();

   int id = query.lastInsertId().toInt();


   emit dataBaseChanged();
   return id;

}

void DbManager::updatePayroll(DbManager::PayrollInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE payroll_list SET start_date = ?, end_date = ?, name = ? WHERE (id = ?)");
    query.addBindValue(info.start_date);
    query.addBindValue(info.end_date);
    query.addBindValue(info.name);
    query.addBindValue(info.id);
    query.exec();
}

void DbManager::addPayrollEntry(int payroll_id, PayrollEntryInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO payroll_entry (payroll_id, employee_id, nssf, tipau, sdl, paye, days_normal, rate_normal, days_special, rate_special, sugar_cane_related, auxilliary, salary_fixed, bonus, overtime) "
                      "VALUES (:pid, :eid, :nssf, :tipau, :sdl, :paye, :days_normal, :rate_normal, :days_special, :rate_special, :sugar_cane_related, :auxilliary, :salary_fixed, :bonus, :overtime)");

   query.bindValue(":pid", payroll_id);
   query.bindValue(":eid", info.employee_id);
   query.bindValue(":nssf", info.has_nssf ? 1 : 0);
   query.bindValue(":tipau", info.has_tipau ? 1 : 0);
   query.bindValue(":sdl", info.has_sdl ? 1 : 0);
   query.bindValue(":paye", info.has_paye ? 1 : 0);
   query.bindValue(":days_normal", info.days_normal);
   query.bindValue(":rate_normal", info.rate_normal);
   query.bindValue(":days_special", info.days_special);
   query.bindValue(":rate_special", info.rate_special);
   query.bindValue(":sugar_cane_related", info.sugar_cane_related);
   query.bindValue(":auxilliary", info.auxilliary_amount);
   query.bindValue(":salary_fixed", info.salary_fixed);
   query.bindValue(":bonus", info.bonus);
   query.bindValue(":overtime", info.overtime);
   query.exec();
}

void DbManager::addWorkType(WorkType work_type)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO work_type (id, description, default_pay, is_active) "
                  "VALUES (:id ,:description, :default_pay, :is_active)");

    query.bindValue(":id", work_type.id);
    query.bindValue(":description", work_type.description);
    query.bindValue(":default_pay", work_type.default_pay);
    query.bindValue(":is_active", work_type.is_active ? 1 : 0);

    query.exec();

    emit dataBaseChanged();
}



void DbManager::addDailyRecord(DbManager::DailyRecord record)
{

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO daily_record (date, employee_id, work_type, pay, location, description) "
                  "VALUES (:date, :employee_id, :work_type, :pay, :location, :description)");

    query.bindValue(":date", record.date);
    query.bindValue(":employee_id", record.employee_id);
    query.bindValue(":work_type", record.work_type_id);
    query.bindValue(":pay", record.pay);
    query.bindValue(":location", record.location);
    query.bindValue(":description", record.description);

    query.exec();
}

void DbManager::removeDailyRecord(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM daily_record WHERE (id = ?)");
    query.addBindValue(id);
    query.exec();
}

void DbManager::updateDailyRecord(DbManager::DailyRecord record)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE daily_record SET date = ?, employee_id = ?, work_type = ?, pay = ?, location = ?, description = ? WHERE id = ?");
    query.addBindValue(record.date);
    query.addBindValue(record.employee_id);
    query.addBindValue(record.work_type_id);
    query.addBindValue(record.pay);
    query.addBindValue(record.location);
    query.addBindValue(record.description);
    query.addBindValue(record.id);

    query.exec();
}

void DbManager::updatePayrollEntry(int payroll_id, DbManager::PayrollEntryInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE payroll_entry SET days_normal = ?, rate_normal = ?, days_special = ?, rate_special = ?, nssf = ?, tipau = ?, sdl = ?, paye = ?, auxilliary = ?, sugar_cane_related = ?, salary_fixed = ?, bonus = ?, overtime = ? WHERE (payroll_id = ? AND employee_id = ?)");
    query.addBindValue(info.days_normal);
    query.addBindValue(info.rate_normal);
    query.addBindValue(info.days_special);
    query.addBindValue(info.rate_special);
    query.addBindValue(info.has_nssf ? 1 : 0);
    query.addBindValue(info.has_tipau ? 1 : 0);
    query.addBindValue(info.has_sdl ? 1 : 0);
    query.addBindValue(info.has_paye ? 1 : 0);
    query.addBindValue(info.auxilliary_amount);
    query.addBindValue(info.sugar_cane_related);
    query.addBindValue(info.salary_fixed);
    query.addBindValue(info.bonus);
    query.addBindValue(info.overtime);
    query.addBindValue(payroll_id);
    query.addBindValue(info.employee_id);

    query.exec();

    //emit dataBaseChanged();

    qDebug() << "SqLite error:" << query.lastError().text();
}

void DbManager::removePayrollByID(int payroll_id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM payroll_list WHERE (id = ?)");
    query.addBindValue(payroll_id);
    query.exec();

    query.prepare("DELETE FROM payroll_entry WHERE (payroll_id = ?)");
    query.addBindValue(payroll_id);
    query.exec();

    emit dataBaseChanged();
}

void DbManager::removePayrollEntry(int payroll_id, int employee_id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM payroll_entry WHERE (payroll_id = ? AND employee_id = ?)");
    query.addBindValue(payroll_id);
    query.addBindValue(employee_id);
    query.exec();

    //qDebug() << "SqLite error:" << query.lastError().text();

    emit dataBaseChanged();
}

void DbManager::removeByID(int id)
{
    QSqlQuery query(m_db);
    query.exec("DELETE FROM employees WHERE (id = ?)");
    query.addBindValue(id);
    query.exec();

    //qDebug() << "SqLite error:" << query.lastError().text();

    emit dataBaseChanged();
}

void DbManager::updateByID(DbManager::EmployeeInfo info)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE employees SET surname = ?, name = ?, nssf = ?, tipau = ?, sdl = ?, paye = ?, rate_normal = ?, salary_fixed = ?, department = ?, nssf_number = ?, is_active = ?, tin_number = ?, nida_number = ? WHERE id = ?");
    query.addBindValue(info.surname);
    query.addBindValue(info.name);
    query.addBindValue(info.nssf ? 1 : 0);
    query.addBindValue(info.tipau ? 1 : 0);
    query.addBindValue(info.sdl ? 1 : 0);
    query.addBindValue(info.paye ? 1 : 0);
    query.addBindValue(info.rate_per_day);
    query.addBindValue(info.salary_fixed);
    query.addBindValue(info.department);
    query.addBindValue(info.nssf_number);
    query.addBindValue(info.is_active);
    query.addBindValue(info.tin_number);
    query.addBindValue(info.nida_number);

    query.addBindValue(info.id);
    query.exec();

    emit dataBaseChanged();

}

QList<DbManager::EmployeeInfo> DbManager::getAllEmployees()
{
    QList<EmployeeInfo> ret;

    QSqlQuery query(m_db);
    query.exec("SELECT id, surname, name, nssf, tipau, sdl, paye, rate_normal, salary_fixed, department, nssf_number, is_active, tin_number, nida_number FROM employees");

    while (query.next()) {
            int id = query.value(0).toInt();
            QString surname = query.value(1).toString();
            QString name = query.value(2).toString();
            bool nssf = query.value(3) > 0;
            bool tipau = query.value(4) > 0;
            bool sdl = query.value(5) > 0;
            bool paye = query.value(6) > 0;
            int rate_normal = query.value(7).toInt();
            int salary_fixed = query.value(8).toInt();
            int department = query.value(9).toInt();
            QString nssf_number = query.value(10).toString();
            bool is_active = query.value(11) > 0;
            QString tin_number = query.value(12).toString();
            QString nida_number = query.value(13).toString();

            EmployeeInfo tmp = {};
            tmp.id = id;
            tmp.name = name;
            tmp.surname = surname;
            tmp.nssf = nssf;
            tmp.tipau = tipau;
            tmp.sdl = sdl;
            tmp.paye = paye;
            tmp.salary_fixed = salary_fixed;
            tmp.department = department;
            tmp.rate_per_day = rate_normal;
            tmp.nssf_number = nssf_number;
            tmp.is_active = is_active;
            tmp.tin_number = tin_number;
            tmp.nida_number = nida_number;

            ret.append(tmp);
    }

    return ret;
}

QList<DbManager::WorkType> DbManager::getAllWorkItems()
{
    QList<WorkType> ret;

    QSqlQuery query(m_db);
    query.exec("SELECT id, description, default_pay, is_active FROM work_type");

    while (query.next()) {
            int id = query.value(0).toInt();
            QString description = query.value(1).toString();
            int default_pay = query.value(2).toInt();
            bool is_active = query.value(3) > 0;

            WorkType tmp = {};
            tmp.id = id;
            tmp.description = description;
            tmp.default_pay = default_pay;
            tmp.is_active = is_active;

            ret.append(tmp);
    }

    return ret;
}

QList<DbManager::DailyRecord> DbManager::getAllDailyRecords()
{
    QList<DailyRecord> ret;

    QSqlQuery query(m_db);
    query.exec("SELECT id, date, employee_id, work_type, pay, location, description FROM daily_record");

    while (query.next()) {
            int id = query.value(0).toInt();
            QDate date = query.value(1).toDate();
            int employee_id = query.value(2).toInt();
            int work_type = query.value(3).toInt();
            int pay = query.value(4).toInt();
            int location = query.value(5).toInt();
            QString description = query.value(6).toString();

            DailyRecord tmp = {};
            tmp.id = id;
            tmp.employee_id = employee_id;
            tmp.date = date;
            tmp.work_type_id = work_type;
            tmp.pay = pay;
            tmp.location = location;
            tmp.description = description;

            ret.append(tmp);
    }

    return ret;
}

DbManager::EmployeeInfo DbManager::getEmployeeFromName(QString surname, QString name)
{
    DbManager::EmployeeInfo ret = {};

    // mark as invalid
    ret.id = -1;

    QSqlQuery query(m_db);
    query.prepare("SELECT id, surname, name, nssf, tipau, sdl, paye, rate_normal, salary_fixed, department, nssf_number, is_active, tin_number, nida_number FROM employees WHERE surname = (:surname) AND name = (:name) ");
    query.bindValue(":surname", surname);
    query.bindValue(":name", name);

    query.exec();
    if (query.next()) {
        ret.id = query.value(0).toInt();
        ret.surname = query.value(1).toString();
        ret.name = query.value(2).toString();
        ret.nssf = query.value(3) > 0;
        ret.tipau = query.value(4) > 0;
        ret.sdl = query.value(5) > 0;
        ret.paye = query.value(6) > 0;
        ret.rate_per_day = query.value(7).toInt();
        ret.salary_fixed = query.value(8).toInt();
        ret.department = query.value(9).toInt();
        ret.nssf_number = query.value(10).toString();
        ret.is_active = query.value(11) > 0;
        ret.tin_number = query.value(12).toString();
        ret.nida_number = query.value(13).toString();
}

    return ret;


}

DbManager::EmployeeInfo DbManager::getEmployeeInfo(int id)
{
    EmployeeInfo ret = {};



    QSqlQuery query(m_db);
    query.prepare("SELECT id, surname, name, nssf, tipau, sdl, paye, rate_normal, salary_fixed, department, nssf_number, is_active, tin_number, nida_number FROM employees WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    query.first();
    ret.id = id;
    ret.surname = query.value(1).toString();
    ret.name = query.value(2).toString();
    ret.nssf = query.value(3) > 0;
    ret.tipau = query.value(4) > 0;
    ret.sdl = query.value(5) > 0;
    ret.paye = query.value(6) > 0;
    ret.rate_per_day = query.value(7).toInt();
    ret.salary_fixed = query.value(8).toInt();
    ret.department = query.value(9).toInt();
    ret.nssf_number = query.value(10).toString();
    ret.is_active = query.value(11) > 0;
    ret.tin_number = query.value(12).toString();
    ret.nida_number = query.value(13).toString();

    return ret;

}

DbManager::WorkType DbManager::getWorkType(int id)
{
    WorkType ret = {};
    QSqlQuery query(m_db);
    query.prepare("SELECT id, description, default_pay, is_active FROM work_type WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    query.first();
    ret.id = id;
    ret.description = query.value(1).toString();
    ret.default_pay = query.value(2).toInt();
    ret.is_active = query.value(3).toBool();

    return ret;

}

DbManager::DailyRecord DbManager::getDailyRecord(int id)
{
    DailyRecord ret = {};
    QSqlQuery query(m_db);
    query.prepare("SELECT id, date, employee_id, work_type, pay, location, description FROM daily_record WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    query.first();
    ret.id = id;
    ret.date = query.value(1).toDate();
    ret.employee_id = query.value(2).toInt();
    ret.work_type_id = query.value(3).toInt();
    ret.pay = query.value(4).toInt();
    ret.location = query.value(5).toInt();
    ret.description = query.value(6).toString();

    return ret;
}

QList<DbManager::DailyRecord> DbManager::getDailyRecordForDateAndEmployeeID(QDate date, int employee_id)
{
    DailyRecord ret = {};
    QSqlQuery query(m_db);
    query.prepare("SELECT id, date, employee_id, work_type, pay, location, description FROM daily_record WHERE employee_id = (:id) AND date = (:date)");
    query.bindValue(":id", employee_id);
    query.bindValue(":date", date);
    query.exec();

    QList<DailyRecord> ret_list = {};
    while (query.next()) {

        DailyRecord ret = {};
        ret.id = query.value(0).toInt();
        ret.date = date;
        ret.employee_id = employee_id;
        ret.work_type_id = query.value(3).toInt();
        ret.pay = query.value(4).toInt();
        ret.location = query.value(5).toInt();
        ret.description = query.value(6).toString();
        ret_list.append(ret);
    }

    return ret_list;

}

QList<DbManager::DailyRecord> DbManager::getDailyRecordForDateAndWorkType(QDate date, int work_id)
{
    DailyRecord ret = {};
    QSqlQuery query(m_db);
    query.prepare("SELECT id, date, employee_id, work_type, pay, location, description FROM daily_record WHERE work_type = (:work_type) AND date = (:date)");
    query.bindValue(":work_type", work_id);
    query.bindValue(":date", date);
    query.exec();

    QList<DailyRecord> ret_list = {};
    while (query.next()) {

        DailyRecord ret = {};
        ret.id = query.value(0).toInt();
        ret.date = date;
        ret.employee_id = query.value(2).toInt();
        ret.work_type_id = query.value(3).toInt();
        ret.pay = query.value(4).toInt();
        ret.location = query.value(5).toInt();
        ret.description = query.value(6).toString();
        ret_list.append(ret);
    }

    return ret_list;

}

QList<DbManager::DailyRecord> DbManager::getDailyRecordForDate(QDate date)
{
    DailyRecord ret = {};
    QSqlQuery query(m_db);
    query.prepare("SELECT id, date, employee_id, work_type, pay, location, description FROM daily_record WHERE date = (:date)");
    query.bindValue(":date", date);
    query.exec();

    QList<DailyRecord> ret_list = {};
    while (query.next()) {

        DailyRecord ret = {};
        ret.id = query.value(0).toInt();
        ret.date = date;
        ret.employee_id = query.value(2).toInt();
        ret.work_type_id = query.value(3).toInt();
        ret.pay = query.value(4).toInt();
        ret.location = query.value(5).toInt();
        ret.description = query.value(6).toString();
        ret_list.append(ret);
    }

    return ret_list;
}

QList<DbManager::WorkType> DbManager::getWorkTypeListForDate(QDate date)
{
    QList<DailyRecord> records = getDailyRecordForDate(date);
    QList<WorkType> work_list_ret = {};
    QList<int> work_type_id_list = {};

    for (auto record : records) {
        if (!work_type_id_list.contains(record.work_type_id)) {
            work_list_ret.append(getWorkType(record.work_type_id));
            work_type_id_list.append(record.work_type_id);
        }
    }

    return work_list_ret;

}



QList<DbManager::EmployeeInfo> DbManager::getMatchName(QString name)
{
    QList<EmployeeInfo> ret;

    QList<EmployeeInfo> all_employees = getAllEmployees();

    for (auto info : all_employees) {
        QString full_name = info.name + QString(" ") + info.surname;
        if (full_name.contains(name)) {
            ret.append(info);
        }
    }

    return ret;

}

QList<DbManager::PayrollInfo> DbManager::getAllPayrolls()
{
    QList<PayrollInfo> ret;

    QSqlQuery query(m_db);
    query.exec("SELECT id, start_date, end_date, name FROM payroll_list");

    while (query.next()) {
            int id = query.value(0).toInt();
            QDate start_date = query.value(1).toDate();
            QDate end_date = query.value(2).toDate();
            QString name = query.value(3).toString();

            PayrollInfo tmp = {id, start_date, end_date, name};

            ret.append(tmp);
    }

    return ret;
}

DbManager::PayrollInfo DbManager::getPayrollInfoByID(int id)
{
    PayrollInfo ret = {};

    QSqlQuery query(m_db);
    query.prepare("SELECT id, start_date, end_date, name FROM payroll_list WHERE id = (:id)");
    query.bindValue(":id", id);
    query.exec();
    query.first();
    ret.id = query.value(0).toInt();
    ret.start_date = query.value(1).toDate();
    ret.end_date = query.value(2).toDate();
    ret.name = query.value(3).toString();

    return ret;

}

QList<DbManager::PayrollEntryInfo> DbManager::getPayrollEntryInfo(int payroll_id)
{
    QList<PayrollEntryInfo> ret = {} ;
    QSqlQuery query(m_db);
    query.prepare("SELECT employee_id, days_normal, rate_normal, days_special, rate_special, nssf, tipau, sdl, paye, auxilliary, sugar_cane_related, salary_fixed, bonus, overtime FROM payroll_entry WHERE payroll_id = (:id)");
    query.bindValue(":id", payroll_id);
    query.exec();

    while (query.next()) {
        PayrollEntryInfo tmp = {};
        tmp.employee_id = query.value(0).toInt();
        tmp.days_normal = query.value(1).toInt();
        tmp.rate_normal = query.value(2).toInt();
        tmp.days_special = query.value(3).toInt();
        tmp.rate_special = query.value(4).toInt();
        tmp.has_nssf = query.value(5).toInt();
        tmp.has_tipau = query.value(6).toInt();
        tmp.has_sdl = query.value(7).toInt();
        tmp.has_paye = query.value(8).toInt();
        tmp.auxilliary_amount = query.value(9).toInt();
        tmp.sugar_cane_related = query.value(10).toInt();
        tmp.salary_fixed = query.value(11).toInt();
        tmp.bonus = query.value(12).toInt();
        tmp.overtime = query.value(13).toInt();
        ret.append(tmp);
    }

    return ret;
}

DbManager::WorkType DbManager::getWorkTypeFromName(QString name)
{
    WorkType ret = {};
    ret.id = -1;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, default_pay, is_active FROM work_type WHERE description = (:desc)");
    query.bindValue(":desc", name);

    query.exec();

    if (query.next()) {
        ret.description = name;
        ret.id = query.value(0).toInt();
        ret.default_pay = query.value(1).toInt();
        ret.is_active = query.value(2).toBool();
    }

    return ret;
}

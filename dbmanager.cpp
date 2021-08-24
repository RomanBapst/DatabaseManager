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

        QSqlQuery query(m_db);

        query.prepare("CREATE TABLE employees (id INTEGER PRIMARY KEY, surname TEXT, name TEXT, nssf INTEGER DEFAULT 0,"
                        "tipau INTEGER DEFAULT 0, sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0, department INTEGER DEFAULT 0, nssf_number TEXT)");
        query.exec();

        query.prepare("CREATE TABLE employees_archived (id INTEGER PRIMARY KEY, surname TEXT, name TEXT, nssf INTEGER DEFAULT 0,"
                        "tipau INTEGER DEFAULT 0, sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0, department INTEGER DEFAULT 0, nssf_number TEXT, is_active INT DEFAULT 0, tin_number TEXT, nida_number TEXT)");
        query.exec();

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

        query.prepare("CREATE TABLE payroll_list (id INTEGER PRIMARY KEY, start_date, end_date DATE, name TEXT)");
        query.exec();

        query.prepare("CREATE TABLE payroll_entry (payroll_id INTEGER, employee_id INTEGER, nssf INTEGER DEFAULT 0, tipau INTEGER DEFAULT 0,"
                      "sdl INTEGER DEFAULT 0, paye INTEGER DEFAULT 0, days_normal INTEGER DEFAULT 0, rate_normal INTEGER DEFAULT 0, days_special INTEGER DEFAULT 0, rate_special INTEGER DEFAULT 0,"
                      "sugar_cane_related INTEGER DEFAULT 0,  auxilliary INTEGER DEFAULT 0, salary_fixed INTEGER DEFAULT 0)");
        query.exec();


        query.exec("PRAGMA table_info(payroll_entry)");

        bool found_bonus = false;
        bool found_overtime = false;
        bool found_department_name = false;

        while (query.next()) {
            QString column_name = query.value(1).toString();

            if (column_name.compare(QString("bonus")) == 0) {
                found_bonus = true;
            }

            if (column_name.compare(QString("overtime")) == 0) {
                found_overtime = true;
            }

            if (column_name.compare(QString("department_name")) == 0) {
                found_department_name = true;
            }
        }

        if (!found_bonus) {
            query.exec("ALTER TABLE payroll_entry ADD COLUMN bonus INTEGER");
        }

        if (!found_overtime) {
            query.exec("ALTER TABLE payroll_entry ADD COLUMN overtime INTEGER");
        }

        if (!found_department_name) {
             query.exec("ALTER TABLE payroll_entry ADD COLUMN department_name TEXT");
        }

        query.prepare("CREATE TABLE company_info (id INTEGER PRIMARY KEY, name TEXT, postal_number TEXT, postal_city TEXT, location TEXT, phone_number TEXT, email TEXT, tin_number TEXT)");

        query.exec();

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

        query.prepare("CREATE TABLE work_type (id INTEGER PRIMARY KEY, description TEXT, default_pay INTEGER, is_active INTEGER)");
        query.exec();

        query.prepare("CREATE TABLE work_category (hash TEXT PRIMARY KEY, name TEXT, sdl INTEGER)");
        query.exec();

        createTable("department", department_header, department_types);

//        FilterMap data;
//        data["department"] = 0;
//        FilterMap map;

//        updateDataInTable("employees", data, map);
    }
}

void DbManager::createTable(QString table_name, QStringList header, QStringList data_types)
{
    QString query_string = "CREATE TABLE %1 %2";


    QString item_data_string = "(";
    int counter = 0;
    for (auto item : header){

        if (counter == 0) {
            item_data_string = item_data_string.append(item + " " + data_types[header.indexOf(item)] + " PRIMARY KEY,");
        } else {
            item_data_string = item_data_string.append(item + " " + data_types[header.indexOf(item)] + ",");
        }

       counter++;
    }

    item_data_string = item_data_string.remove(item_data_string.size()-1, 1);
    item_data_string = item_data_string.append(")");

    query_string = query_string.arg(table_name).arg(item_data_string);

     QSqlQuery query(m_db);
     query.exec(query_string);
}

QList<QVariantList> DbManager::getDataFromTable(QString table_name, QList<SQLiteColumnInfo> items, QMap<QString, QVariant> map, QString sort_name)
{

    QString query_string = "SELECT %1 FROM %2 %3 %4";

    QString item_data_string;

    for (auto item : items){
        item_data_string = item_data_string.append(item.name + ",");
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

    QString sorting = "";
    if (!sort_name.isEmpty()) {
        sorting = "ORDER BY " + sort_name;
    }

    query_string = query_string.arg(item_data_string).arg(table_name).arg(where_string).arg(sorting);

    QSqlQuery query(m_db);

    query.prepare(query_string);

    qInfo() << query_string;

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

    qInfo() << query_string;

    QSqlQuery query(m_db);

    query.prepare(query_string);

    for (auto &item : data.keys()) {
        query.addBindValue(data[item]);
    }

    query.exec();

    return query.lastInsertId().toInt();
}

void DbManager::addEmployee(DbManager::EmployeeInfo info, QString table_name)
{
    FilterMap data = employeeInfoToVariantMap(info);
    data.remove("id");
    addDataToTable("employees", data);
}

void DbManager::setCompanyInfo(DbManager::CompanyInfo info)
{
    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM company_info");

    query.first();

    if (query.value(0).toInt() > 0) {

        FilterMap map;
        map["id"] = 1;
        info.id = 1;
        updateDataInTable("company_info", companyInfoToVariantMap(info), map);

    } else {
        FilterMap data = companyInfoToVariantMap(info);
        data.remove("id");
        addDataToTable("company_info", data);
    }
}

DbManager::CompanyInfo DbManager::getCompanyInfo()
{
    CompanyInfo ret = {};

    QList<QVariantList> data = getDataFromTable("company_info", company_info_header, QMap<QString, QVariant>());

    ret = getCompanyInfoFromVariantList(data.first());
    return ret;
}

int DbManager::addPayroll(PayrollInfo info)
{

    QMap<QString, QVariant> map = payrollInfoToVariantMap(info);
    map.remove("id");
    int id = addDataToTable("payroll_list", map);
    emit dataBaseChanged();
    return id;
}

void DbManager::updatePayroll(DbManager::PayrollInfo info)
{
    FilterMap map;
    map["id"] = info.id;

    updateDataInTable("payroll_list", payrollInfoToVariantMap(info), map);
}

void DbManager::addPayrollEntry(int payroll_id, PayrollEntryInfo info)
{
    info.payroll_id = payroll_id;
    addDataToTable("payroll_entry", payrollEntryInfoToVariantMap(info));
}

void DbManager::addWorkType(WorkType work_type)
{

    addDataToTable("work_type", workTypeToVariantMap(work_type));

    emit dataBaseChanged();
}

void DbManager::addDailyRecord(DbManager::DailyRecord record)
{

    addDataToTable("daily_record", dailyRecordToVariantMap(record));
}

void DbManager::removeDailyRecord(int id)
{
    FilterMap map;
    map["id"] = id;
    deleteDataInTable("daily_record", map);
}

void DbManager::updateDailyRecord(DbManager::DailyRecord record)
{
    FilterMap map;
    map["id"] = record.id;
    updateDataInTable("daily_record", dailyRecordToVariantMap(record), map);
}

void DbManager::updatePayrollEntry(int payroll_id, DbManager::PayrollEntryInfo info)
{
    FilterMap map = payrollEntryInfoToVariantMap(info);

    FilterMap filter_map;
    filter_map["payroll_id"] = payroll_id;
    filter_map["employee_id"] = info.employee_id;

    updateDataInTable("payroll_entry", map, filter_map);
}

void DbManager::removePayrollByID(int payroll_id)
{
    FilterMap map1,map2;
    map1["id"] = payroll_id;
    deleteDataInTable("payroll_list", map1);

    map2["payroll_id"] = payroll_id;

    deleteDataInTable("payroll_entry", map2);

    emit dataBaseChanged();
}

void DbManager::removePayrollEntry(int payroll_id, int employee_id)
{

    FilterMap map;
    map["payroll_id"] = payroll_id;
    map["employee_id"] = employee_id;

    deleteDataInTable("payroll_entry", map);

    emit dataBaseChanged();
}

void DbManager::removeByID(int id)
{
    emit dataBaseChanged();
}

void DbManager::updateByID(DbManager::EmployeeInfo info)
{
    FilterMap map;
    map["id"] = info.id;
    updateDataInTable("employees", employeeInfoToVariantMap(info), map);
    emit dataBaseChanged();
}

QMap<QString, QVariant> DbManager::companyInfoToVariantMap(DbManager::CompanyInfo info)
{
    FilterMap ret;
    ret["id"] = info.id;
    ret["name"] = info.name;
    ret["postal_number"] = info.postal_number;
    ret["postal_city"] = info.postal_city;
    ret["location"] = info.location;
    ret["phone_number"] = info.phone_number;
    ret["email"] = info.email;
    ret["tin_number"] = info.tin_number;
    ret["nssf_nr"] = info.nssf_reg_nr;
    ret["nssf_ctrl_nr"] = info.nssf_ctrl_nr;

    return ret;

}

QMap<QString, QVariant> DbManager::employeeInfoToVariantMap(DbManager::EmployeeInfo info)
{
    FilterMap ret;
    ret["id"] = info.id;
    ret["surname"] = info.surname;
    ret["name"] = info.name;
    ret["nssf"] = info.nssf;
    ret["tipau"] = info.tipau;
    ret["sdl"] = info.sdl;
    ret["paye"] = info.paye;
    ret["rate_normal"] = info.rate_per_day;
    ret["salary_fixed"] = info.salary_fixed;
    ret["department"] = info.department;
    ret["nssf_number"] = info.nssf_number;
    ret["is_active"] = info.is_active;
    ret["tin_number"] = info.tin_number;
    ret["nida_number"] = info.nida_number;

    return ret;

}

QMap<QString, QVariant> DbManager::payrollInfoToVariantMap(DbManager::PayrollInfo info)
{
    FilterMap ret;
    ret["id"] = info.id;
    ret["start_date"] = info.start_date;
    ret["end_date"] = info.end_date;
    ret["name"] = info.name;

    return ret;
}

QMap<QString, QVariant> DbManager::payrollEntryInfoToVariantMap(DbManager::PayrollEntryInfo info)
{
    FilterMap ret;
    ret["payroll_id"] = info.payroll_id;
    ret["employee_id"] = info.employee_id;
    ret["days_normal"] = info.days_normal;
    ret["rate_normal"] = info.rate_normal;
    ret["days_special"] = info.days_special;
    ret["rate_special"] = info.rate_special;
    ret["nssf"] = info.has_nssf;
    ret["tipau"] = info.has_tipau;
    ret["sdl"] = info.has_sdl;
    ret["paye"] = info.has_paye;
    ret["auxilliary"] = info.auxilliary_amount;
    ret["sugar_cane_related"] = info.sugar_cane_related;
    ret["salary_fixed"] = info.salary_fixed;
    ret["bonus"] = info.bonus;
    ret["overtime"] = info.overtime;
    ret["department_name"] = info.department_name;

    return ret;
}

QMap<QString, QVariant> DbManager::dailyRecordToVariantMap(DbManager::DailyRecord info)
{
    FilterMap ret;
    ret["id"] = info.id;
    ret["date"] = info.date;
    ret["employee_id"] = info.employee_id;
    ret["work_type"] = info.work_type_id;
    ret["pay"] = info.pay;
    ret["location"] = info.location;
    ret["description"] = info.description;

    return ret;
}

QMap<QString, QVariant> DbManager::workTypeToVariantMap(DbManager::WorkType info)
{
    FilterMap ret;
    ret["id"] = info.id;
    ret["default_pay"] = info.default_pay;
    ret["description"] = info.description;
    ret["is_active"] = info.is_active;

    return ret;
}

DbManager::CompanyInfo DbManager::getCompanyInfoFromVariantList(QVariantList list)
{
    CompanyInfo info = {};

    info.email = list.value(company_info_header.indexOf("email")).toString();
    info.location = list.value(company_info_header.indexOf("location")).toString();
    info.name = list.value(company_info_header.indexOf("name")).toString();
    info.nssf_ctrl_nr = list.value(company_info_header.indexOf("nssf_ctrl_nr")).toString();
    info.nssf_reg_nr = list.value(company_info_header.indexOf("nssf_nr")).toString();
    info.phone_number = list.value(company_info_header.indexOf("phone_number")).toString();
    info.postal_city = list.value(company_info_header.indexOf("postal_city")).toString();
    info.postal_number = list.value(company_info_header.indexOf("postal_number")).toString();
    info.tin_number = list.value(company_info_header.indexOf("tin_number")).toString();

    return info;
}

DbManager::EmployeeInfo DbManager::getEmployeeInfoFromVariantList(QVariantList list)
{
    EmployeeInfo info = {};
    info.id = list.value(employee_header.indexOf("id")).toInt();
    info.surname = list.value(employee_header.indexOf("surname")).toString();
    info.name = list.value(employee_header.indexOf("name")).toString();
    info.nssf = list.value(employee_header.indexOf("nssf")).toBool();
    info.tipau = list.value(employee_header.indexOf("tipau")).toBool();
    info.sdl = list.value(employee_header.indexOf("sdl")).toBool();
    info.paye = list.value(employee_header.indexOf("paye")).toBool();
    info.rate_per_day = list.value(employee_header.indexOf("rate_per_day")).toInt();
    info.salary_fixed = list.value(employee_header.indexOf("salary_fixed")).toInt();
    info.department = list.value(employee_header.indexOf("department")).toInt();
    info.nssf_number = list.value(employee_header.indexOf("nssf_number")).toString();
    info.is_active = list.value(employee_header.indexOf("is_active")).toBool();
    info.tin_number = list.value(employee_header.indexOf("tin_number")).toString();
    info.nida_number = list.value(employee_header.indexOf("nida_number")).toString();

    return info;
}

DbManager::PayrollInfo DbManager::getPayrollInfoFromVariantList(QVariantList list)
{
    PayrollInfo info = {};
    info.id = list.value(payroll_info_header.indexOf("id")).toInt();
    info.start_date = list.value(payroll_info_header.indexOf("start_date")).toDate();
    info.end_date = list.value(payroll_info_header.indexOf("end_date")).toDate();
    info.name = list.value(payroll_info_header.indexOf("name")).toString();
    return info;
}

DbManager::PayrollEntryInfo DbManager::getPayrollEntryInfoFromVariantList(QVariantList list)
{
    PayrollEntryInfo info = {};
    info.payroll_id = list.value(payroll_entry_header.indexOf("payroll_id")).toInt();
    info.employee_id = list.value(payroll_entry_header.indexOf("employee_id")).toInt();
    info.days_normal = list.value(payroll_entry_header.indexOf("days_normal")).toInt();
    info.rate_normal = list.value(payroll_entry_header.indexOf("rate_normal")).toInt();
    info.days_special = list.value(payroll_entry_header.indexOf("days_special")).toInt();
    info.rate_special = list.value(payroll_entry_header.indexOf("rate_special")).toInt();
    info.has_nssf = list.value(payroll_entry_header.indexOf("nssf")).toBool();
    info.has_tipau = list.value(payroll_entry_header.indexOf("tipau")).toBool();
    info.has_sdl = list.value(payroll_entry_header.indexOf("sdl")).toBool();
    info.has_paye = list.value(payroll_entry_header.indexOf("paye")).toBool();
    info.auxilliary_amount = list.value(payroll_entry_header.indexOf("auxilliary")).toInt();
    info.sugar_cane_related = list.value(payroll_entry_header.indexOf("sugar_cane_related")).toInt();
    info.salary_fixed = list.value(payroll_entry_header.indexOf("salary_fixed")).toInt();
    info.bonus = list.value(payroll_entry_header.indexOf("bonus")).toInt();
    info.overtime = list.value(payroll_entry_header.indexOf("overtime")).toInt();
    info.department_name = list.value(payroll_entry_header.indexOf("department_name")).toInt();

    return info;
}

DbManager::DailyRecord DbManager::getDailyRecordFromVariantList(QVariantList list)
{
    DailyRecord info = {};
    info.id = list.value(daily_record_header.indexOf("id")).toInt();
    info.date = list.value(daily_record_header.indexOf("date")).toDate();
    info.employee_id = list.value(daily_record_header.indexOf("employee_id")).toInt();
    info.work_type_id = list.value(daily_record_header.indexOf("work_type")).toInt();
    info.pay = list.value(daily_record_header.indexOf("pay")).toInt();
    info.location = list.value(daily_record_header.indexOf("location")).toInt();
    info.description = list.value(daily_record_header.indexOf("description")).toString();

    return info;
}

DbManager::WorkType DbManager::getWorkTypeFromVariantList(QVariantList list)
{
    WorkType info = {};
    info.id = list.value(work_type_header.indexOf("id")).toInt();
    info.default_pay = list.value(work_type_header.indexOf("default_pay")).toInt();
    info.description = list.value(work_type_header.indexOf("description")).toString();
    info.is_active = list.value(work_type_header.indexOf("is_active")).toBool();

    return info;
}

QList<DbManager::EmployeeInfo> DbManager::getAllEmployees()
{
    QList<EmployeeInfo> ret;
    QList<QVariantList> data = getDataFromTable("employees", employee_header, QMap<QString, QVariant>());

    for (auto list : data) {
        EmployeeInfo info = getEmployeeInfoFromVariantList(list);
        ret.append(info);
    }

    return ret;
}

QList<DbManager::WorkType> DbManager::getAllWorkItems()
{
    QList<QVariantList> data = getDataFromTable("work_type", work_type_header, QMap<QString, QVariant>());
    QList<DbManager::WorkType> ret;

    for (auto &item : data) {
        ret.append(getWorkTypeFromVariantList(item));
    }

    return ret;
}

QList<DbManager::DailyRecord> DbManager::getAllDailyRecords()
{
    QList<DailyRecord> ret;
    QList<QVariantList> data = getDataFromTable("daily_record", daily_record_header, QMap<QString, QVariant>());

    for (auto &item : data) {
        ret.append(getDailyRecordFromVariantList(item));
    }

    return ret;
}

DbManager::EmployeeInfo DbManager::getEmployeeFromName(QString surname, QString name)
{
    QMap<QString, QVariant> map;
    map["name"] = name;
    map["surname"] = surname;

    return getEmployeeInfoFromVariantList(getDataFromTable("employees", employee_header, map).first());
}

DbManager::EmployeeInfo DbManager::getEmployeeByID(int id)
{
    QMap<QString, QVariant> map;
    map["id"] = id;
    QList<QVariantList> data = getDataFromTable("employees", employee_header, map);

    return getEmployeeInfoFromVariantList(data.first());
}

DbManager::EmployeeInfo DbManager::getEmployeeInfo(int id, bool& success, QString table)
{
    success = true;
    return getEmployeeByID(id);
}

DbManager::WorkType DbManager::getWorkType(int id)
{
    FilterMap map;
    map["id"] = id;
    return getWorkTypeFromVariantList(getDataFromTable("work_type", work_type_header, map).first());
}

DbManager::DailyRecord DbManager::getDailyRecord(int id)
{
    FilterMap map;
    map["id"] = id;

    return getDailyRecordFromVariantList(getDataFromTable("daily_record", daily_record_header, map).first());
}

QList<DbManager::DailyRecord> DbManager::getDailyRecordForDateAndEmployeeID(QDate date, int employee_id)
{
    FilterMap map;
    map["date"] = date;
    map["employee_id"] = employee_id;

    QList<QVariantList> data = getDataFromTable("daily_record", daily_record_header, map);

    QList<DailyRecord> ret;

    for (auto &item : data) {
        ret.append(getDailyRecordFromVariantList(item));
    }

    return ret;
}

QList<DbManager::DailyRecord> DbManager::getDailyRecordForDateAndWorkType(QDate date, int work_id)
{
    QList<DbManager::DailyRecord> ret;

    FilterMap map;
    map["work_type"] = work_id;
    map["date"] = date;

    QList<QVariantList> data = getDataFromTable("daily_record", daily_record_header, map);

    for (auto &item : data) {
        ret.append(getDailyRecordFromVariantList(item));
    }

    return ret;
}

QList<DbManager::DailyRecord> DbManager::getDailyRecordForDate(QDate date)
{
    QList<DbManager::DailyRecord> ret;

    FilterMap map;
    map["date"] = date;

    QList<QVariantList> data = getDataFromTable("daily_record", daily_record_header, map);

    for (auto &item : data) {
        ret.append(getDailyRecordFromVariantList(item));
    }

    return ret;
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

    QList<QVariantList> data = getDataFromTable("payroll_list", payroll_info_header, QMap<QString, QVariant>());

    for (auto &item : data) {
        ret.append(getPayrollInfoFromVariantList(item));
    }

    return ret;
}

DbManager::PayrollInfo DbManager::getPayrollInfoByID(int id)
{
    FilterMap map;
    map["id"] = id;
    return getPayrollInfoFromVariantList(getDataFromTable("payroll_list", payroll_info_header, map).first());
}

QList<DbManager::PayrollEntryInfo> DbManager::getPayrollEntryInfo(int payroll_id)
{
    FilterMap map;
    map["payroll_id"] = payroll_id;

    QList<QVariantList> data = getDataFromTable("payroll_entry", payroll_entry_header, map);

    QList<PayrollEntryInfo> ret;

    for (auto &item : data) {
        ret.append(getPayrollEntryInfoFromVariantList(item));
    }

    return ret;
}

DbManager::WorkType DbManager::getWorkTypeFromName(QString name)
{
    FilterMap map;
    map["description"] = name;

    return getWorkTypeFromVariantList(getDataFromTable("work_type", work_type_header, map).first());
}

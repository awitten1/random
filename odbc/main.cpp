#include <sqltypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>

void check_error(SQLRETURN ret, SQLSMALLINT handle_type, SQLHANDLE handle, const char* msg) {
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLCHAR sqlstate[6];
        SQLCHAR message[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER native_error;
        SQLSMALLINT message_len;

        SQLGetDiagRec(handle_type, handle, 1, sqlstate, &native_error,
                     message, sizeof(message), &message_len);

        printf("Error %s: [%s] %s\n", msg, sqlstate, message);
    }
}

void ddl(SQLHDBC conn) {
    SQLHSTMT stmt;
    SQLRETURN ret;

    // Allocate statement handle
    ret = SQLAllocHandle(SQL_HANDLE_STMT, conn, &stmt);
    if (ret != SQL_SUCCESS) {
        printf("Failed to allocate statement handle\n");
        return;
    }

    // Create table SQL
    const char* create_table_sql =
        "CREATE TABLE IF NOT EXISTS employees ("
        "    id SERIAL PRIMARY KEY,"
        "    name VARCHAR(100) NOT NULL,"
        "    email VARCHAR(255) UNIQUE NOT NULL,"
        "    salary DECIMAL(10,2),"
        "    hire_date DATE DEFAULT CURRENT_DATE,"
        "    department VARCHAR(50)"
        ")";

    // Execute the CREATE TABLE statement
    ret = SQLExecDirect(stmt, (SQLCHAR*)create_table_sql, SQL_NTS);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("Table 'employees' created successfully!\n");

        // Insert some sample data
        const char* insert_sql =
            "INSERT INTO employees (name, email, salary, department) VALUES "
            "('John Doe', 'john.doe@company.com', 75000.00, 'Engineering'),"
            "('Jane Smith', 'jane.smith@company.com', 82000.00, 'Marketing'),"
            "('Bob Johnson', 'bob.johnson@company.com', 68000.00, 'Sales') "
            "ON CONFLICT (email) DO NOTHING";

        ret = SQLExecDirect(stmt, (SQLCHAR*)insert_sql, SQL_NTS);

        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
            printf("Sample data inserted successfully!\n");

            // Query the data back
            const char* select_sql = "SELECT id, name, email, salary, department FROM employees ORDER BY id";
            ret = SQLExecDirect(stmt, (SQLCHAR*)select_sql, SQL_NTS);

            if (ret == SQL_SUCCESS) {
                printf("\nEmployees table contents:\n");
                printf("ID | Name         | Email                    | Salary   | Department\n");
                printf("---|--------------|--------------------------|----------|------------\n");

                // Bind columns for fetching
                SQLINTEGER id;
                SQLCHAR name[101], email[256], department[51];
                SQLDOUBLE salary;
                SQLLEN id_len, name_len, email_len, salary_len, dept_len;

                SQLBindCol(stmt, 1, SQL_C_SLONG, &id, 0, &id_len);
                SQLBindCol(stmt, 2, SQL_C_CHAR, name, sizeof(name), &name_len);
                SQLBindCol(stmt, 3, SQL_C_CHAR, email, sizeof(email), &email_len);
                SQLBindCol(stmt, 4, SQL_C_DOUBLE, &salary, 0, &salary_len);
                SQLBindCol(stmt, 5, SQL_C_CHAR, department, sizeof(department), &dept_len);

                // Fetch and display results
                while ((ret = SQLFetch(stmt)) == SQL_SUCCESS) {
                    printf("%-2d | %-12s | %-24s | $%-7.2f | %s\n",
                           (int)id, name, email, salary, department);
                }
            }
        } else {
            check_error(ret, SQL_HANDLE_STMT, stmt, "inserting sample data");
        }
    } else {
        check_error(ret, SQL_HANDLE_STMT, stmt, "creating table");
    }

    // Clean up statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
}


int main() {
    SQLRETURN ret;
    SQLHENV env;
    SQLHDBC conn;

    // Allocate environment handle
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if (ret != SQL_SUCCESS) {
        printf("Failed to allocate environment handle\n");
        return 1;
    }

    // Set ODBC version
    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    check_error(ret, SQL_HANDLE_ENV, env, "setting ODBC version");

    // Allocate connection handle
    ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
    check_error(ret, SQL_HANDLE_DBC, conn, "allocating connection handle");

    // Method 1: Using connection string (recommended for PostgreSQL)
    SQLCHAR conn_str[] = "driver={PostgreSQL Unicode};server=localhost;port=5432;uid=postgres;password=mysecretpassword;";
    SQLCHAR out_conn_str[1024];
    SQLSMALLINT out_conn_str_len;

    ret = SQLDriverConnect(conn, NULL, conn_str, SQL_NTS,
                          out_conn_str, sizeof(out_conn_str),
                          &out_conn_str_len, SQL_DRIVER_COMPLETE);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        printf("Connected to PostgreSQL successfully!\n");

        // Call your DDL function
        ddl(conn);

    } else {
        check_error(ret, SQL_HANDLE_DBC, conn, "connecting to database");
    }

    // Alternative Method 2: Using DSN (if you have one configured)
    /*
    ret = SQLConnect(conn, (SQLCHAR*)"PostgreSQL-Test", SQL_NTS,
                     (SQLCHAR*)"your_username", SQL_NTS,
                     (SQLCHAR*)"your_password", SQL_NTS);
    */

    // Cleanup
    SQLDisconnect(conn);
    SQLFreeHandle(SQL_HANDLE_DBC, conn);
    SQLFreeHandle(SQL_HANDLE_ENV, env);

    return 0;
}
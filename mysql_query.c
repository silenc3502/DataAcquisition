#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>

void finish_with_error(MYSQL *conn)
{
	fprintf(stderr, "%s\n", mysql_error(conn));
	mysql_close(conn);
	exit(1);
}
 
int main(int argc, char **argv)
{
        MYSQL *conn;
        MYSQL_RES *result;
        MYSQL_ROW row;
 
        char query_buffer[2048];
 
        conn = mysql_init(NULL);
 
        if(!mysql_real_connect(
			conn, "localhost", "bitai", "456123", NULL, 0, NULL, 0))
		{
                printf("cannot connect");
                exit(1);
        }
        else
		{
                if (mysql_select_db(conn, "cdb"))
				{
                        printf("cannot use databases");
                        exit(1);
                }
        }

		if(mysql_query(conn, "drop table if exists sensor"))
		{
			finish_with_error(conn);
		}

		if(mysql_query(conn, "create table sensor(" \
			"no integer unsigned not null auto_increment primary key," \
			"acc_x float not null," \
			"acc_y float not null," \
			"acc_z float not null," \
			"ang_x float not null," \
			"ang_y float not null," \
			"ang_z float not null," \
			"reg_date timestamp not null default now()"))
		{
			finish_with_error(conn);
		}
 
        sprintf(query_buffer, "%s", "show tables");
        if (mysql_query(conn, query_buffer))
		{
                printf("query faild : %s\n", query_buffer);
                exit(1);
        }
 
        result = mysql_use_result(conn);
        while ((row = mysql_fetch_row(result)) != NULL)
                printf("%s \n", row[0]);

		if(mysql_query(conn, "insert into sensor values(0.3, 0.2, 0.1, 0.1, 0.2, 0.3)"))
		{
			finish_with_error(conn);
		}
 
        mysql_free_result(result);
        mysql_close(conn);
}

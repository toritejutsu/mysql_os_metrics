#include <sql_class.h>
#include <table.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <mysql_version.h>
#include <mysql/plugin.h>
#include <my_global.h>
#include <dirent.h>

char* strip_newlines(char* szX) {
    int i = strlen(szX);
    while('\n'==szX[--i]) szX[i] = 0;
    while('\r'==szX[--i]) szX[i] = 0;
    return szX;
}

static struct st_mysql_information_schema osmetrics_ioscheduler_table_info = { MYSQL_INFORMATION_SCHEMA_INTERFACE_VERSION };

static ST_FIELD_INFO osmetrics_ioscheduler_table_fields[]=
{
  {"device", 25, MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {"scheduler", 25, MYSQL_TYPE_STRING, 0, 0, 0, 0},
  {0, 0, MYSQL_TYPE_NULL, 0, 0, 0, 0}
};

static int osmetrics_ioscheduler_fill_table(THD *thd, TABLE_LIST *tables, Item *cond)
{
  TABLE *table= tables->table;
  char dirpath[256] = "/sys/block/";
  char *path = dirpath;
  struct dirent *entry;
  DIR *dp;
  char buffer[256] = "";

  dp = opendir(path);
  if (dp != NULL) {
    while ((entry = readdir(dp))) {
      if (((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))) {
        char filename[256];
        strcpy(filename, dirpath);
        strcat(filename, entry->d_name);
        strcat(filename, "/queue/scheduler");

        FILE *fp = fopen(filename, "r");
        fgets(buffer, 256, fp);
        strcpy(buffer, strip_newlines(buffer));

        table->field[0]->store(entry->d_name, strlen(entry->d_name), system_charset_info);
        table->field[1]->store(buffer, strlen(buffer), system_charset_info);
        if (schema_table_store_record(thd, table)) return 1;
      }
    }
    closedir(dp);
  }
  return 0;
}

static int osmetrics_ioscheduler_table_init(void *ptr)
{
  ST_SCHEMA_TABLE *schema_table= (ST_SCHEMA_TABLE*)ptr;
  schema_table->fields_info= osmetrics_ioscheduler_table_fields;
  schema_table->fill_table= osmetrics_ioscheduler_fill_table;
  return 0;
}

mysql_declare_plugin(os_metrics_ioscheduler)
{
  MYSQL_INFORMATION_SCHEMA_PLUGIN,
  &osmetrics_ioscheduler_table_info,  /* type-specific descriptor */
  "OS_IOSCHEDULER",                   /* table name */
  "Michael Patrick",                  /* author */
  "OS Metrics: Disk Scheduler",       /* description */
  PLUGIN_LICENSE_GPL,                 /* license type */
  osmetrics_ioscheduler_table_init,   /* init function */
  NULL,
  0x0001,                             /* version = 0.1 */
  NULL,                               /* no status variables */
  NULL,                               /* no system variables */
  NULL,                               /* no reserved information */
  0                                   /* no flags */
}
mysql_declare_plugin_end;

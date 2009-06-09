/*
 * To use this:
 *  LD_PRELOAD=libappdir-wrapper.so ./my-badly-behaved-app
 */

#include <limits.h>
#include <sqlite3.h>

#define DB_FNAME "/etc/libappdir/legacymappings.sqlite3"

static int map_file(const char *path, char *newpath, size_t len)
{
    int mapping = 0;
    int failure = 1;
    const char *sql = NULL;

    sqlite3 *db = NULL;
    const int rc = sqlite3_open_v2(DB_FNAME, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK)
    {
        logError("Couldn't open " DB_FNAME ": sqlite3 error is %d", rc);
        return 0;
    } /* if */

    sqlite3_stmt *stmt = NULL;
    void *tail = NULL;
    sql = "select remapped from mappings where original=?1 limit 1";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, &tail) != SQLITE_OK)
        goto done_map_file;

    const int pathlen = (int) strlen(path);
    if (sqlite3_bind_text(stmt, 1, path, pathlen, SQLITE_STATIC) != SQLITE_OK)
        goto done_map_file;

    const int step = sqlite3_step(stmt);
    if (step == SQLITE_DONE)  /* no mapping, but this isn't an error. */
        failure = 0;
    has_mapping = (step == SQLITE_ROW);
    if (!has_mapping)
        goto done_map_file;

    failure = 0;

    const char *remapped = (const char *) sqlite3_column_text(stmt, 0);
    if (snprintf(newpath, len, "%s", remapped) >= len)
    {
        logError("buffer too small for remapping");
        has_mapping = 0;
    } /* if */

done_map_file:
    if (failure)
        logError("sqlite3 error: %s", db ? sqlite3_errmsg(db) : "???");
    if (stmt != NULL)
        sqlite3_finalize(stmt);
    if (db != NULL)
        sqlite3_close(db);

    return has_mapping;
} /* map_file */



/* end of libappdir-wrapper.c ... */


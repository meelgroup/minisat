/******************************************
Copyright (c) 2016, Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substanti)(=al portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#include "minisat/utils/sqlstats.h"
#include "minisat/utils/sqlitestats.h"
#include "minisat/utils/sql_tablestructure.h"
#include "minisat/core/Solver.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <time.h>
#include "System.h"
#include "System.h"


#define bind_null_or_double(stmt,bindat,stucture,func) \
{ \
    if (stucture.num_data_elements() == 0) {\
        sqlite3_bind_null(stmt, bindat); \
    } else { \
        sqlite3_bind_double(stmt, bindat, stucture.func()); \
    }\
    bindat++; \
}

#define bind_null_or_int(stmt,bindat,stucture,func) \
{ \
    if (stucture.num_data_elements() == 0) {\
        sqlite3_bind_null(stmt, bindat); \
    } else { \
        sqlite3_bind_int(stmt, bindat, stucture.func()); \
    }\
    bindat++; \
}

#define bind_null_or_int64(stmt,bindat,stucture,func) \
{ \
    if (stucture.num_data_elements() == 0) {\
        sqlite3_bind_null(stmt, bindat); \
    } else { \
        sqlite3_bind_int64(stmt, bindat, stucture.func()); \
    }\
    bindat++; \
}

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using namespace Minisat;

SQLiteStats::SQLiteStats(std::string _filename) :
        filename(_filename)
{
}

vector<string> SQLiteStats::get_columns(const char* tablename)
{
    vector<string> ret;

    std::stringstream q;
    q << "pragma table_info(" << tablename << ");";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, q.str().c_str(), -1, &stmt, NULL)) {
        cerr << "ERROR: Couln't create table structure for SQLite: "
        << sqlite3_errmsg(db)
        << endl;
        std::exit(-1);
    }

    sqlite3_bind_int(stmt, 1, 16);
    int rc;
    while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        ret.push_back(string((const char*)sqlite3_column_text(stmt, 1)));
    }
    sqlite3_finalize(stmt);

    return ret;
}

void SQLiteStats::del_prepared_stmt(sqlite3_stmt* stmt)
{
    if (stmt == NULL) {
        return;
    }

    int ret = sqlite3_finalize(stmt);
    if (ret != SQLITE_OK) {
        cout << "Error closing prepared statement" << endl;
        std::exit(-1);
    }
}


SQLiteStats::~SQLiteStats()
{
    if (!setup_ok)
        return;

    //Free all the prepared statements
    del_prepared_stmt(stmtRst);
    del_prepared_stmt(stmtVarRst);
    del_prepared_stmt(stmtClRst);
    del_prepared_stmt(stmtFeat);
    del_prepared_stmt(stmtReduceDB);
    del_prepared_stmt(stmtTimePassed);
    del_prepared_stmt(stmtMemUsed);
    del_prepared_stmt(stmt_clause_stats);
    del_prepared_stmt(stmt_delete_cl);
    del_prepared_stmt(stmt_var_data_picktime);
    del_prepared_stmt(stmt_var_data_fintime);
    del_prepared_stmt(stmt_dec_var_clid);
    del_prepared_stmt(stmt_var_dist);

    //Close clonnection
    sqlite3_close(db);
}

bool SQLiteStats::setup(const Solver* solver)
{
    setup_ok = connectServer(solver);
    if (!setup_ok) {
        return false;
    }

    //TODO check if data is in any table
    if (sqlite3_exec(db, cmsat_tablestructure_sql, NULL, NULL, NULL)) {
        cerr << "ERROR: Couln't create table structure for SQLite: "
        << sqlite3_errmsg(db)
        << endl;
        std::exit(-1);
    }

    add_solverrun(solver);
    addStartupData();
    init("timepassed", &stmtTimePassed);
    init("memused", &stmtMemUsed);
    init("satzilla_features", &stmtFeat);
    init("clause_stats", &stmt_clause_stats);
    init("restart", &stmtRst);
    init("restart_dat_for_var", &stmtVarRst);
    init("restart_dat_for_cl", &stmtClRst);
    init("reduceDB", &stmtReduceDB);
    init("var_data_fintime", &stmt_var_data_fintime);
    init("var_data_picktime", &stmt_var_data_picktime);
    init("dec_var_clid", &stmt_dec_var_clid);
    init("cl_last_in_solver", &stmt_delete_cl);
    init("var_dist", &stmt_var_dist);

    return true;
}


bool file_exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}


bool SQLiteStats::connectServer(const Solver* solver)
{
    bool sql_overwrite_file = true;
    if (file_exists(filename) && !sql_overwrite_file) {
        cout << "ERROR -- the database already exists: " << filename << endl;
        cout << "ERROR -- We cannot store more than one run in the same database"
        << endl
        << "Exiting." << endl;
        exit(-1);
    }

    int rc = sqlite3_open(filename.c_str(), &db);
    if(rc) {
        cout << "c Cannot open sqlite database: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, NULL)) {
        cerr << "ERROR: Problem setting pragma synchronous = OFF to SQLite DB" << endl;
        cerr << "c " << sqlite3_errmsg(db) << endl;
        std::exit(-1);
    }

    if (sqlite3_exec(db, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL)) {
        cerr << "ERROR: Problem setting pragma journal_mode = MEMORY to SQLite DB" << endl;
        cerr << "c " << sqlite3_errmsg(db) << endl;
        std::exit(-1);
    }


    if (solver->verbosity) {
        cout << "c writing to SQLite file: " << filename << endl;
    }

    return true;
}

void SQLiteStats::begin_transaction()
{
    if (sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL)) {
        cerr << "ERROR: Beginning SQLITE transaction" << endl;
        cerr << "c " << sqlite3_errmsg(db) << endl;
        std::exit(-1);
    }
}

void SQLiteStats::end_transaction()
{
    if (sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL)) {
        cerr << "ERROR: Beginning SQLITE transaction" << endl;
        cerr << "c " << sqlite3_errmsg(db) << endl;
        std::exit(-1);
    }
}

bool SQLiteStats::add_solverrun(const Solver* solver)
{
    std::stringstream ss;
//     ss
//     << "INSERT INTO solverRun (`runtime`, `gitrev`) values ("
//     << time(NULL)
//     << ", '" << solver->get_version_sha1() << "'"
//     << ");";

    //Inserting element into solverruns to get unique ID
    const int rc = sqlite3_exec(db, ss.str().c_str(), NULL, NULL, NULL);
    if (rc) {
        if (solver->verbosity >= 6) {
            cerr << "c ERROR Couldn't insert into table 'solverruns'" << endl;
            cerr << "c " << sqlite3_errmsg(db) << endl;
        }

        return false;
    }

    return true;
}

void SQLiteStats::add_tag(const std::pair<string, string>& tag)
{
    std::stringstream ss;
    ss
    << "INSERT INTO `tags` (`name`, `val`) VALUES("
    << "'" << tag.first << "'"
    << ", '" << tag.second << "'"
    << ");";

    //Inserting element into solverruns to get unique ID
    if (sqlite3_exec(db, ss.str().c_str(), NULL, NULL, NULL)) {
        cerr << "SQLite: ERROR Couldn't insert into table 'tags'" << endl;
        assert(false);
        std::exit(-1);
    }
}

void SQLiteStats::addStartupData()
{
    std::stringstream ss;
    ss
    << "INSERT INTO `startup` (`startTime`) VALUES ("
    << "datetime('now')"
    << ");";

    if (sqlite3_exec(db, ss.str().c_str(), NULL, NULL, NULL)) {
        cerr << "ERROR Couldn't insert into table 'startup' : "
        << sqlite3_errmsg(db) << endl;

        std::exit(-1);
    }
}


void SQLiteStats::finishup(const lbool status)
{
    std::stringstream ss;
    // TODO : this data should be added
//     ss
//     << "INSERT INTO `finishup` (`endTime`, `status`) VALUES ("
//     << "datetime('now') , "
//     << "'" << status << "'"
//     << ");";

    if (sqlite3_exec(db, ss.str().c_str(), NULL, NULL, NULL)) {
        cerr << "ERROR Couldn't insert into table 'finishup'" << endl;
        std::exit(-1);
    }
}

void SQLiteStats::writeQuestionMarks(
    size_t num
    , std::stringstream& ss
) {
    ss << "(";
    for(size_t i = 0
        ; i < num
        ; i++
    ) {
        if (i < num-1)
            ss << "?,";
        else
            ss << "?";
    }
    ss << ")";
}

void SQLiteStats::run_sqlite_step(sqlite3_stmt* stmt, const char* name)
{
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cout
        << "ERROR: while executing '" << name << "' SQLite prepared statement"
        << endl;

        cout << "Error from sqlite: "
        << sqlite3_errmsg(db)
        << endl;
        cout << "Error code from sqlite: " << rc << endl;
        std::exit(-1);
    }

    if (sqlite3_reset(stmt)) {
        cerr << "Error calling sqlite3_reset on cl_last_in_solver" << endl;
        std::exit(-1);
    }

    if (sqlite3_clear_bindings(stmt)) {
        cerr << "Error calling sqlite3_clear_bindings on '"
        << name << "'" << endl;
        std::exit(-1);
    }
}


void SQLiteStats::init(const char* name, sqlite3_stmt** stmt)
{
    vector<string> cols = get_columns(name);
    const size_t numElems = cols.size();

    std::stringstream ss;
    ss << "insert into `" << name << "` (";
    for(uint32_t i = 0; i < cols.size(); i++) {
        if (i > 0) {
            ss << ", ";
        }
        ss << "`" << cols[i] << "`";
    }
    ss << ") values ";
    writeQuestionMarks(
        numElems
        , ss
    );
    ss << ";";

    //Prepare the statement
    if (sqlite3_prepare(db, ss.str().c_str(), -1, stmt, NULL)) {
        cerr << "ERROR in sqlite_stmt_prepare(), INSERT failed"
        << endl
        << sqlite3_errmsg(db)
        << endl
        << "Query was: " << ss.str()
        << endl;
        std::exit(-1);
    }
}

void SQLiteStats::reduceDB(
    const Solver* solver
    , const bool locked
    , const Clause* cl
    , const string& cur_restart_type
    , const uint32_t act_ranking_top_10
    , const uint32_t act_ranking
    , const uint32_t tot_cls_in_db
) {
    assert(cl->stats.dump_no != std::numeric_limits<uint16_t>::max());

    int bindAt = 1;
    sqlite3_bind_int64(stmtReduceDB, bindAt++, solver->conflicts);
    sqlite3_bind_text(stmtReduceDB, bindAt++, cur_restart_type.c_str(), -1, NULL);
    sqlite3_bind_double(stmtReduceDB, bindAt++, cpuTime());

    //data
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.ID);
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.dump_no);
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.conflicts_made);
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.propagations_made);
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.sum_propagations_made);
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.clause_looked_at);
    sqlite3_bind_int64(stmtReduceDB, bindAt++, cl->stats.used_for_uip_creation);

    int64_t last_touched_diff = solver->conflicts - cl->stats.last_touched;
    sqlite3_bind_int64(stmtReduceDB, bindAt++, last_touched_diff);

    sqlite3_bind_int(stmtReduceDB, bindAt++, locked);
    sqlite3_bind_int(stmtReduceDB, bindAt++, cl->stats.glue);
    sqlite3_bind_int(stmtReduceDB, bindAt++, cl->size());
    sqlite3_bind_int(stmtReduceDB, bindAt++, act_ranking_top_10);
    sqlite3_bind_int(stmtReduceDB, bindAt++, act_ranking);
    sqlite3_bind_int(stmtReduceDB, bindAt++, tot_cls_in_db);
    sqlite3_bind_int(stmtReduceDB, bindAt++, cl->stats.sum_uip1_used);

    run_sqlite_step(stmtReduceDB, "reduceDB");
}

void SQLiteStats::dump_clause_stats(
    const Solver* solver
    , uint64_t clid
    , uint32_t orig_glue
    , uint32_t glue_before_minim
    , uint32_t backtrack_level
    , uint32_t size
    , size_t decision_level
    , size_t trail_depth
    , uint64_t conflicts_this_restart
    , const bool is_decision
) {
    int bindAt = 1;
    sqlite3_bind_int64 (stmt_clause_stats, bindAt++, solver->conflicts);
    sqlite3_bind_int64 (stmt_clause_stats, bindAt++, clid);

    sqlite3_bind_int   (stmt_clause_stats, bindAt++, orig_glue);
    sqlite3_bind_int   (stmt_clause_stats, bindAt++, glue_before_minim);
    sqlite3_bind_int   (stmt_clause_stats, bindAt++, size);
    sqlite3_bind_int64 (stmt_clause_stats, bindAt++, conflicts_this_restart);
    sqlite3_bind_int   (stmt_clause_stats, bindAt++, is_decision);

    sqlite3_bind_int   (stmt_clause_stats, bindAt++, backtrack_level);
    sqlite3_bind_int64 (stmt_clause_stats, bindAt++, decision_level);
    sqlite3_bind_int64 (stmt_clause_stats, bindAt++, trail_depth);

    run_sqlite_step(stmt_clause_stats, "dump_clause_stats");
}


void SQLiteStats::cl_last_in_solver(
    const Solver* solver
    , const uint64_t clid)
{
    assert(clid != 0);

    int bindAt = 1;
    sqlite3_bind_int64(stmt_delete_cl, bindAt++, solver->conflicts);
    sqlite3_bind_int64(stmt_delete_cl, bindAt++, clid);

    run_sqlite_step(stmt_delete_cl, "cl_last_in_solver");
}

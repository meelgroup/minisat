/******************************************
Copyright (c) 2016, Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************/

#ifndef __SQLSTATS_H__
#define __SQLSTATS_H__

#include <limits>
#include <cstdint>
#include <string>
#include <cmath>
#include <cstdint>

#include "minisat/core/SolverTypes.h"

using std::string;

namespace Minisat {

class Solver;
class Searcher;
class Clause;

class SQLStats
{
public:

    virtual ~SQLStats() {}

    virtual void end_transaction() = 0;
    virtual void begin_transaction() = 0;

    virtual void reduceDB(
        const Solver* solver
        , const bool locked
        , const Clause* cl
        , const uint32_t act_ranking_top_10
        , const uint32_t act_ranking
        , const uint32_t tot_cls_in_db
    ) = 0;


    virtual void cl_last_in_solver(
        const Solver* solver
        , const uint64_t clid
    ) = 0;

    virtual void dump_clause_stats(
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
    ) = 0;

    virtual bool setup(const Solver* solver) = 0;
    virtual void add_tag(const std::pair<std::string, std::string>& tag) = 0;
};

} //end namespace

#endif //__SQLSTATS_H__

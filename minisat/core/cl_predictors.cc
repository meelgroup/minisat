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

#include "cl_predictors.h"
#include <cmath>
#include "Solver.h"
#include "SolverTypes.h"

#define MISSING_VAL -1334556787.0

using namespace Minisat;

enum predict_type { short_pred = 0, long_pred = 1 };

ClPredictors::ClPredictors(Solver* _solver) : solver(_solver)
{
    BoosterHandle handle;
    int ret;

    handles.push_back(handle);
    ret = XGBoosterCreate(0, 0, &(handles[0]));
    assert(ret == 0);
    ret = XGBoosterSetParam(handles[0], "nthread", "1");
    assert(ret == 0);

    handles.push_back(handle);
    ret = XGBoosterCreate(0, 0, &(handles[1]));
    assert(ret == 0);
    ret = XGBoosterSetParam(handles[1], "nthread", "1");
    assert(ret == 0);
}

ClPredictors::~ClPredictors()
{
    for (auto& h: handles) {
        XGBoosterFree(h);
    }
}

void ClPredictors::load_models(std::string short_fname, std::string long_fname)
{
    int ret;
    //     const char* filen;// = short_fname.c_str();
    //     std::ifstream infile(short_fname.c_str());
    //     printf("c boost file read %d", infile.good());
    ret = XGBoosterLoadModel(handles[short_pred], short_fname.c_str());
    assert(ret == 0);

    //     ret = XGBoosterLoadModel(handles[long_pred], long_fname.c_str());
    //     assert(ret == 0);
}

void ClPredictors::set_up_input(const Clause* cl,
                                const uint64_t sumConflicts,
                                const int64_t last_touched_diff,
                                const double act_ranking_rel,
                                const uint32_t act_ranking_top_10,
                                const uint32_t cols)
{
    float* at = train;
    uint32_t x = 0;
    uint64_t time_inside_solver = solver->conflicts - cl->stats.introduced_at_conflict;


    //prevent divide by zero
    if (cl->stats.orig_glue == 0 ||
        cl->stats.orig_glue == 1)
    {
        at[x++] = MISSING_VAL;
    } else {
        at[x++] = ((double)(cl->stats.propagations_made)) /
              ::log2(cl->stats.orig_glue);
    }
    //((rdb0.propagations_made)/log2(cl.orig_glue))


    //prevent divide by zero
    if (cl->stats.sum_uip1_used == 0)
    {
        at[x++] = MISSING_VAL;
    } else {
        at[x++] =
            ::log2(cl->stats.glue_before_minim) / ((double)cl->stats.sum_uip1_used / (double)time_inside_solver);
    }
    //(log2(cl.glue_before_minim)/(rdb0.sum_uip1_used/cl.time_inside_solver))

    //prevent divide by zero
    //updated glue can actually be 1. Original glue cannot.
    if (cl->stats.glue == 1) {
        at[x++] = MISSING_VAL;
    } else {
        at[x++] = (double)cl->stats.sum_uip1_used / ::log2(cl->stats.glue);
    }
    //(rdb0.sum_uip1_used/log2(rdb0.glue))

    at[x++] = ::log2(act_ranking_rel) / (double)cl->stats.orig_glue;
    //(log2(rdb0_act_ranking_rel)/cl.orig_glue)

    if (time_inside_solver == 0) {
        at[x++] = MISSING_VAL;
    } else {
        at[x++] = (double)cl->stats.propagations_made / (double)time_inside_solver;
    }
    //(rdb0.propagations_made/cl.time_inside_solver)

    at[x++] = (double)cl->stats.propagations_made / (double)cl->stats.orig_glue;
    //(rdb0.propagations_made/cl.orig_glue)

    //NOTE: this is actually really low ranked. Very interesting.
    at[x++] = (double)cl->stats.used_for_uip_creation/
        (double)cl->stats.glue_before_minim;;
    //(rdb0.used_for_uip_creation/cl.glue_before_minim)

    assert(x == cols);
}

float ClPredictors::predict_one(int num, DMatrixHandle dmat)
{
    bst_ulong out_len;
    const float *out_result;
    int ret = XGBoosterPredict(
        handles[num],
        dmat,
        0,  //0: normal prediction
        0,  //use all trees
        0,  //do not use for training
        &out_len,
        &out_result
    );
    assert(ret == 0);
    assert(out_len == 1);

    float retval = out_result[0];
    return retval;
}

float ClPredictors::predict_short(
    const Clause* cl,
    const uint64_t sumConflicts,
    const int64_t last_touched_diff,
    const double   act_ranking_rel,
    const uint32_t act_ranking_top_10)
{
    // convert to DMatrix
    set_up_input(cl, sumConflicts, last_touched_diff,
                 act_ranking_rel, act_ranking_top_10,
                 PRED_COLS);
    int rows = 1;
    int ret = XGDMatrixCreateFromMat((float*)train, rows, PRED_COLS, MISSING_VAL, &dmat);
    assert(ret == 0);

    float val = predict_one(short_pred, dmat);
    XGDMatrixFree(dmat);

    return val;
}

float ClPredictors::predict_long(const Clause* cl, const uint64_t sumConflicts,
                                 const int64_t last_touched_diff, const double act_ranking_rel,
                                 const uint32_t act_ranking_top_10)
{
    // convert to DMatrix
    set_up_input(cl, sumConflicts, last_touched_diff, act_ranking_rel, act_ranking_top_10,
                 PRED_COLS);
    int rows = 1;
    int ret = XGDMatrixCreateFromMat((float*)train, rows, PRED_COLS, MISSING_VAL, &dmat);
    assert(ret == 0);

    float val = predict_one(long_pred, dmat);
    XGDMatrixFree(dmat);

    return val;
}

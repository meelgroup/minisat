#!/bin/bash

FNAMEOUT="mydata"
FIXED="6000"
RATIO="0.2"
CONF=1
SHORTPERC=50
LONGPERC=40
FNAME="php_h10_p9.cnf"

NEXT_OP="$1"

if [ "$NEXT_OP" == "-h" ] || [ "$NEXT_OP" == "--help" ]; then
    echo "You must give a CNF file as input"
    exit
fi
initial="$(echo ${NEXT_OP} | head -c 1)"

FNAME="${NEXT_OP}"

set -e

echo "--> Running on file                   $FNAME"
echo "--> Outputting to data                $FNAMEOUT"
echo "--> Using clause gather ratio of      $RATIO"
echo "--> with fixed number of data points  $FIXED"



if [ "$FNAMEOUT" == "" ]; then
    echo "Error: FNAMEOUT is not set, it's empty. Exiting."
    exit -1
fi

EXTRA_GEN_PANDAS_OPTS=""
if [ "$1" == "--csv" ]; then
    EXTRA_GEN_PANDAS_OPTS="--csv"
    echo "CSV will be generated (may take some disk space)"
    NEXT_OP="$2"
else
    NEXT_OP="$1"
fi

echo "Cleaning up"
(
rm -rf "$FNAME-dir"
mkdir "$FNAME-dir"
cd "$FNAME-dir"
rm -f $FNAMEOUT.d*
rm -f $FNAMEOUT.lemma*
)

set -x

# =============================================================================
#  Build statistics-gathering MiniSat and DRAT
# =============================================================================

./build_stats.sh

( cd ../drat-trim
cmake .
make -j8 )

(
# =============================================================================
#  Obtain dynamic data in SQLite and DRAT info
# =============================================================================

cd "$FNAME-dir"
../minisat -cldatadumpratio="$RATIO" -sqlitedb="$FNAMEOUT.db-raw" -drup-file="$FNAMEOUT.drat" "../$FNAME" #| tee minisat-stat-run.out

# =============================================================================
#  Run our own DRAT-Trim
# =============================================================================

../../drat-trim/drat-trim "../$FNAME" "$FNAMEOUT.drat" -o "$FNAMEOUT.usedCls" -i

# =============================================================================
#  Augment, fix up and sample the SQLite data
# =============================================================================

../fill_used_clauses.py "$FNAMEOUT.db-raw" "$FNAMEOUT.usedCls"
cp "$FNAMEOUT.db-raw" "$FNAMEOUT.db"
../clean_update_data.py "$FNAMEOUT.db"
../check_data_quality.py "$FNAMEOUT.db"
cp "$FNAMEOUT.db" "$FNAMEOUT-min.db"
../sample_data.py "$FNAMEOUT-min.db"

# =============================================================================
#  Denormalize the data into a Pandas Table, label it and sample it
# =============================================================================

../cldata_gen_pandas.py "${FNAMEOUT}-min.db" --limit "$FIXED"

mkdir -p ../../minisat/predict
rm -f ../../minisat/predict/*


# =============================================================================
#  Create the classifiers
# =============================================================================

../cldata_predict.py "${FNAMEOUT}-min.db-cldata-short.dat" --name short --final --xgboost --basedir ../../minisat/predict --tier short
../cldata_predict.py "${FNAMEOUT}-min.db-cldata-long.dat" --name long --final --xgboost --basedir ../../minisat/predict --tier long

)

# =============================================================================
#  Build final MiniSat with the classifier
# =============================================================================

./build_final_predictor.sh

# =============================================================================
#  Test the newly built MiniSat
# =============================================================================

(
cd "$FNAME-dir"
../minisat "../$FNAME" | tee minisat-final-run.out
)
exit

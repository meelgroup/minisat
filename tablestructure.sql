DROP TABLE IF EXISTS `reduceDB`;
CREATE TABLE `reduceDB` (
  `simplifications` int(20) NOT NULL,
  `restarts` int(20) NOT NULL,
  `conflicts` bigint(20) NOT NULL,
  `latest_satzilla_feature_calc` int(20) NOT NULL,
  `cur_restart_type` varchar(6) NOT NULL,
  `runtime` float NOT NULL,
  `clauseID` int(20) NOT NULL,
  `dump_no` int(20) NOT NULL,
  `conflicts_made` bigint(20) NOT NULL,
  `propagations_made` bigint(20) NOT NULL,
  `sum_propagations_made` bigint(20) NOT NULL,
  `clause_looked_at` bigint(20) NOT NULL,
  `used_for_uip_creation` bigint(20) NOT NULL,
  `last_touched_diff` bigint(20) NOT NULL,
  `activity_rel` float(20) NOT NULL,
  `locked` int(20) NOT NULL,
  `in_xor` int(20) NOT NULL,
  `glue` int(20) NOT NULL,
  `size` int(20) NOT NULL,
  `ttl` int(20) NOT NULL,
  `act_ranking_top_10` int(20) NOT NULL,
  `act_ranking` int(20) NOT NULL,
  `tot_cls_in_db` int(20) NOT NULL,
  `sum_uip1_used`  int(20) NOT NULL
);

DROP TABLE IF EXISTS `clause_stats`;
CREATE TABLE `clause_stats` (
  `conflicts` bigint(20) NOT NULL,
  `clauseID` bigint(20) NOT NULL,

  `orig_glue` int(20) NOT NULL,
  `glue_before_minim` int(20) NOT NULL,
  `orig_size` int(20) NOT NULL,
  `conflicts_this_restart` bigint(20) NOT NULL,
  `is_decision` int(20) NOT NULL,

  `backtrack_level` int(20) NOT NULL,
  `decision_level` int(20) NOT NULL,
  `trail_depth_level` int(20) NOT NULL
);

DROP TABLE IF EXISTS `used_clauses`;
create table `used_clauses` (
    `clauseID` bigint(20) NOT NULL
    , `used_at` bigint(20) NOT NULL
);

DROP TABLE IF EXISTS `cl_last_in_solver`;
create table `cl_last_in_solver` (
  `conflicts` bigint(20) NOT NULL
  , `clauseID` bigint(20) NOT NULL
);

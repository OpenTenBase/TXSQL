#!/bin bash

RUNMTR="./mysql-test-run.pl  --timer --max-test-fail=1000 --force --parallel=8 --comment=ps_row --vardir=var-ps_row --ps-protocol --skip-ndb --max-connections=2048  --clean-vardir --report-unstable-tests  --retry-failure=4 --retry=5 --suite-timeout=600"

$RUNMTR &>all.mtrresult

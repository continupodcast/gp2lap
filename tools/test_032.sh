#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p validation release
python3 tools/build_editor.py
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
for test_name in test_f1_pit032 test_f1_pit028 test_f1_race027 test_f1_loops test_f1_progress test_pit_hybrid test_gap_diagnostic; do
 cc -std=c89 -Wall -Wextra -Isrc "tools/$test_name.c" -o "$test_tmp/$test_name"
 "$test_tmp/$test_name"
done
python3 tools/test_f1_theme90.py
python3 tools/test_driver027.py
python3 tools/test_f1_compact.py
python3 tools/test_f1_config.py
node tools/test_editor_14teams.cjs

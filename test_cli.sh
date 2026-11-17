#!/bin/sh
# End-to-end tests for ./cachetree: feeds commands on stdin and compares the output
# with what is expected. Not part of the submission. Run: make test (or ./test_cli.sh)

cd "$(dirname "$0")" || exit 1
fails=0

# check NAME INPUT EXPECTED
check() {
    actual=$(printf '%s\n' "$2" | ./cachetree)
    code=$?
    if [ "$actual" = "$3" ] && [ $code -eq 0 ]; then
        echo "  ok   $1"
    else
        echo "  FAIL $1 (exit code $code)"
        echo "--- expected"; printf '%s\n' "$3"
        echo "--- actual";   printf '%s\n' "$actual"
        fails=$((fails + 1))
    fi
}

echo "cli"

check "put / get / overwrite" \
"PUT a 1
PUT b 2
GET a
PUT a 10
GET a
GET zzz
EXIT" \
"OK
OK
1
OK
10
KEY_NOT_FOUND"

check "del" \
"PUT a 1
DEL a
DEL a
GET a
EXIT" \
"OK
DELETED
KEY_NOT_FOUND
KEY_NOT_FOUND"

check "empty tree" \
"GET x
DEL x
RANGE a z
TOP 3
EXIT" \
"KEY_NOT_FOUND
KEY_NOT_FOUND"

check "range is inclusive and ascending" \
"LOAD sample_data.txt
RANGE banana date
RANGE b c
RANGE z a
EXIT" \
"LOADED 7
banana: 20
cherry: 75
date: 75
banana: 20"

check "top k: highest first, ties alphabetical" \
"LOAD sample_data.txt
TOP 3
TOP 0
EXIT" \
"LOADED 7
fig: 90
cherry: 75
date: 75"

check "top k larger than the tree" \
"PUT x 1
PUT y 2
TOP 10
EXIT" \
"OK
OK
y: 2
x: 1"

check "negative values" \
"PUT a -5
PUT b -1
TOP 1
GET a
EXIT" \
"OK
OK
b: -1
-5"

check "missing file" \
"LOAD no_such_file.txt
EXIT" \
"ERROR: cannot open file no_such_file.txt"

check "bad input is rejected, not crashed on" \
"PUT a
PUT a abc
PUT a 12abc
PUT a 99999999999
PUT a 1 2
GET
TOP -1
TOP x
FLY a
EXIT" \
"ERROR: usage PUT <key> <int>
ERROR: usage PUT <key> <int>
ERROR: usage PUT <key> <int>
ERROR: usage PUT <key> <int>
ERROR: usage PUT <key> <int>
ERROR: usage GET <key>
ERROR: usage TOP <k>, k >= 0
ERROR: usage TOP <k>, k >= 0
ERROR: unknown command FLY"

check "blank lines, extra spaces, lowercase commands" \
"
   PUT    a    5
put b 6
get b
EXIT" \
"OK
OK
6"

check "end of input without EXIT still exits cleanly" \
"PUT a 1" \
"OK"

check "commands after EXIT are ignored" \
"PUT a 1
EXIT
GET a" \
"OK"

if [ $fails -eq 0 ]; then
    echo "all cli tests passed"
else
    echo "$fails cli test(s) failed"
    exit 1
fi

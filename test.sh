#!/usr/bin/env bash

set -e

IME="${1:-./build/ime}"
PASSED=0
FAILED=0

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

pass() {
  echo -e "${GREEN}PASS${NC}: $1"
  PASSED=$((PASSED + 1))
}

fail() {
  echo -e "${RED}FAIL${NC}: $1"
  echo "  Expected: $2"
  echo "  Got:      $3"
  FAILED=$((FAILED + 1))
}

test_success() {
  local desc="$1"
  shift
  if "$IME" "$@" > /dev/null 2>&1; then
    pass "$desc"
  else
    fail "$desc" "exit 0" "exit $?"
  fi
}

test_failure() {
  local desc="$1"
  shift
  if "$IME" "$@" > /dev/null 2>&1; then
    fail "$desc" "exit non-zero" "exit 0"
  else
    pass "$desc"
  fi
}

test_stdout_contains() {
  local desc="$1"
  local expected="$2"
  shift 2
  local output
  output=$("$IME" "$@" 2>/dev/null) || true
  if echo "$output" | grep -q "$expected"; then
    pass "$desc"
  else
    fail "$desc" "stdout contains '$expected'" "stdout: $output"
  fi
}

test_stderr_contains() {
  local desc="$1"
  local expected="$2"
  shift 2
  local stderr_output
  stderr_output=$("$IME" "$@" 2>&1 >/dev/null) || true
  if echo "$stderr_output" | grep -q "$expected"; then
    pass "$desc"
  else
    fail "$desc" "stderr contains '$expected'" "stderr: $stderr_output"
  fi
}

test_stdout_matches_regex() {
  local desc="$1"
  local regex="$2"
  shift 2
  local output
  output=$("$IME" "$@" 2>/dev/null) || true
  if echo "$output" | grep -Eq "$regex"; then
    pass "$desc"
  else
    fail "$desc" "stdout matches: $regex" "stdout: $output"
  fi
}

group() {
  echo ""
  echo "--- $1 ---"
}

# Reverse-DNS-like input source ID (e.g. com.apple.keylayout.ABC).
ID_REGEX='^[A-Za-z][A-Za-z0-9._-]*$'

echo "Testing: $IME"

group "Help and usage"
  test_failure "no args exits non-zero"
  test_stderr_contains "no args mentions missing command" "missing command"
  test_stderr_contains "no args prints Usage" "Usage:"

  test_success "help exits 0" help
  test_stdout_contains "help prints Usage to stdout" "Usage:" help
  test_stdout_contains "help lists 'get'" "get" help
  test_stdout_contains "help lists 'set'" "set" help
  test_stdout_contains "help lists 'list'" "list" help

  test_success "-h exits 0" -h
  test_stdout_contains "-h prints Usage to stdout" "Usage:" -h
  test_success "--help exits 0" --help
  test_stdout_contains "--help prints Usage to stdout" "Usage:" --help

group "Unknown command"
  test_failure "unknown command exits non-zero" foo
  test_stderr_contains "unknown command mentions 'unknown'" "unknown command" foo

group "Argument count validation"
  test_failure "get rejects extra arg" get extra
  test_stderr_contains "get extra-arg error message" "no arguments" get extra
  test_failure "list rejects extra arg" list extra
  test_stderr_contains "list extra-arg error message" "no arguments" list extra
  test_failure "set without ID" set
  test_stderr_contains "set missing-arg error message" "exactly one" set
  test_failure "set with too many args" set a b
  test_stderr_contains "set extra-arg error message" "exactly one" set a b

group "get (read-only)"
  test_success "get exits 0" get
  test_stdout_matches_regex "get returns reverse-DNS ID" "$ID_REGEX" get

group "list (read-only)"
  test_success "list exits 0" list
  list_output=$("$IME" list 2>/dev/null) || true
  case_name="list returns at least one entry"
  if [ -n "$list_output" ]; then
    pass "$case_name"
  else
    fail "$case_name" "non-empty output" "empty"
  fi

  case_name="every list entry matches reverse-DNS ID"
  bad_lines=$(echo "$list_output" | grep -Ev "$ID_REGEX" || true)
  if [ -z "$bad_lines" ]; then
    pass "$case_name"
  else
    fail "$case_name" "all lines match $ID_REGEX" "bad: $bad_lines"
  fi

  case_name="get output appears in list output"
  current=$("$IME" get 2>/dev/null) || true
  if echo "$list_output" | grep -Fxq "$current"; then
    pass "$case_name"
  else
    fail "$case_name" "list contains '$current'" "list: $list_output"
  fi

group "set error handling"
  test_failure "set with bogus ID exits non-zero" set this.id.does.not.exist
  test_stderr_contains "set bogus mentions 'not found'" "not found" set this.id.does.not.exist

group "set idempotent (current -> current)"
  current=$("$IME" get 2>/dev/null) || true
  case_name="set to current input source succeeds"
  if "$IME" set "$current" > /dev/null 2>&1; then
    pass "$case_name"
  else
    fail "$case_name" "exit 0" "exit $?"
  fi

  case_name="get returns same ID after self-set"
  after=$("$IME" get 2>/dev/null) || true
  if [ "$current" = "$after" ]; then
    pass "$case_name"
  else
    fail "$case_name" "$current" "$after"
  fi

echo -e "\nResults: ${GREEN}${PASSED} passed${NC}, ${RED}${FAILED} failed${NC}"

if [ $FAILED -gt 0 ]; then
  exit 1
fi
exit 0

#!/usr/bin/bash
# Usage: run_tests.sh <path-to-kriol-binary> <project-root>

set -e

KRIOL="$1"
ROOT="$2"

if [ -z "$KRIOL" ] || [ -z "$ROOT" ]; then
    echo "Usage: run_tests.sh <kriol-binary> <project-root>"
    exit 1
fi

echo -e "\n~~ Running tests ~~\n"
pass=0; fail=0

# ---- examples/*.kr --------------------------------------------------------
for f in "$ROOT"/examples/*.kriol; do
    printf "  %-44s" "$f"
    tmpbin=$(mktemp /tmp/kriol_XXXX)
    if "$KRIOL" "$f" -o "$tmpbin" 2>/dev/null && \
       timeout 5 "$tmpbin" > /dev/null 2>&1; then
        echo " PASS"; pass=$((pass+1))
    else
        echo " FAIL"; fail=$((fail+1))
    fi
    rm -f "$tmpbin"
done

# ---- tests/pass/*.kr -------------------------------------------------------
if [ -d "$ROOT/tests/pass" ]; then
    for f in "$ROOT"/tests/pass/*.kr; do
        [ -f "$f" ] || continue
        printf "  %-44s" "$f"
        tmpbin=$(mktemp /tmp/kriol_XXXX)
        if "$KRIOL" "$f" -o "$tmpbin" 2>/dev/null && \
           timeout 5 "$tmpbin" > /dev/null 2>&1; then
            echo " PASS"; pass=$((pass+1))
        else
            echo " FAIL"; fail=$((fail+1))
        fi
        rm -f "$tmpbin"
    done
fi

# ---- tests/fail/*.kr -------------------------------------------------------
if [ -d "$ROOT/tests/fail" ]; then
    for f in "$ROOT"/tests/fail/*.kr; do
        [ -f "$f" ] || continue
        printf "  %-44s" "$f"
        tmpbin=$(mktemp /tmp/kriol_fail_bin_XXXX)
        tmperr=$(mktemp /tmp/kriol_fail_err_XXXX)
        expect="$f.err"

        if "$KRIOL" "$f" -o "$tmpbin" > /dev/null 2>"$tmperr"; then
            echo " FAIL (should have been rejected)"; fail=$((fail+1))
        else
            if [ -f "$expect" ]; then
                missing=0
                while IFS= read -r needle || [ -n "$needle" ]; do
                    case "$needle" in ''|'#'*) continue ;; esac
                    if ! grep -Fq "$needle" "$tmperr"; then
                        missing=1
                        echo " FAIL (missing diagnostic fragment: $needle)"
                        break
                    fi
                done < "$expect"
                if [ $missing -eq 0 ]; then
                    echo " PASS (rejected, diagnostics match)"; pass=$((pass+1))
                else
                    echo "      stderr:"; sed 's/^/      /' "$tmperr"; fail=$((fail+1))
                fi
            else
                echo " PASS (rejected)"; pass=$((pass+1))
            fi
        fi
        rm -f "$tmpbin" "$tmperr"
    done
fi

echo -e "\n  $pass/$((pass+fail)) passed\n"
[ $fail -eq 0 ]

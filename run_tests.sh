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
failed_tests=""

record_failure() {
    fail=$((fail+1))
    failed_tests="${failed_tests}\n  - $1"
}

# ---- examples/*.kr --------------------------------------------------------
for f in "$ROOT"/examples/*.kriol; do
    printf "  %-44s" "$f"
    tmpbin=$(mktemp /tmp/kriol_XXXX)
    if "$KRIOL" "$f" -o "$tmpbin" 2>/dev/null && \
       timeout 5 "$tmpbin" > /dev/null 2>&1; then
        echo " PASS"; pass=$((pass+1))
    else
        echo " FAIL"; record_failure "$f"
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
            echo " FAIL"; record_failure "$f"
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

        if "$KRIOL" "$f" -o "$tmpbin" > /dev/null 2>&1; then
            echo " FAIL (should have been rejected)"
            record_failure "$f"
        else
            echo " PASS (rejected)"; pass=$((pass+1))
        fi
        rm -f "$tmpbin"
    done
fi

# ---- wasm32-wasi compile checks --------------------------------------------
if "$KRIOL" --help 2>&1 | grep -Fq "wasm32-wasi"; then
    printf "  %-44s" "wasm32-wasi hello_world"
    tmpwasm=$(mktemp /tmp/kriol_wasm_XXXX.wasm)
    if "$KRIOL" "$ROOT/examples/hello_world.kriol" --target wasm32-wasi -o "$tmpwasm" 2>/dev/null; then
        if command -v file >/dev/null 2>&1; then
            if file "$tmpwasm" | grep -Fq "WebAssembly"; then
                echo " PASS"; pass=$((pass+1))
            else
                echo " FAIL (not a WebAssembly module)"
                record_failure "wasm32-wasi hello_world"
            fi
        else
            echo " PASS"; pass=$((pass+1))
        fi
    else
        echo " FAIL"; record_failure "wasm32-wasi hello_world"
    fi
    rm -f "$tmpwasm"

    printf "  %-44s" "wasm32-wasi f-string runtime"
    tmpwasm=$(mktemp /tmp/kriol_wasm_fstr_gc_XXXX.wasm)
    if "$KRIOL" "$ROOT/tests/pass/mostra_interpolation.kr" --target wasm32-wasi -o "$tmpwasm" 2>/dev/null; then
        echo " PASS"; pass=$((pass+1))
    else
        echo " FAIL"; record_failure "wasm32-wasi f-string runtime"
    fi
    rm -f "$tmpwasm"
fi

echo -e "\n  $pass/$((pass+fail)) passed\n"

if [ $fail -ne 0 ]; then
    echo -e "Failed tests:$failed_tests\n"
fi

[ $fail -eq 0 ]

# Source this file from the gp2lap project root before invoking wmake.
# The compiler backup extracts to ../toolchains/watcom.
export WATCOM="$(cd ../toolchains/watcom && pwd)"
export PATH="$WATCOM/binl64:$PATH"
export INCLUDE="$WATCOM/h"

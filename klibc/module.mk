# --- Sub directories to be includes ---

INCLUDES += -Ikunistd

include klibc/debug/module.mk
include klibc/math/module.mk
include klibc/memory/module.mk
include klibc/string/module.mk
include klibc/utility/module.mk

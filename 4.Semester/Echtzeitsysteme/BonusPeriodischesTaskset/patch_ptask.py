#!/usr/bin/env python3
"""
Patch ptask/src/libdl.{h,c} fuer neuere glibc (>= 2.41, z.B. Ubuntu 26.04).

Ab glibc 2.41 liefert <sched.h> selbst 'struct sched_attr' sowie
'sched_setattr'/'sched_getattr'. ptask definiert diese ebenfalls -> harter
Compilerfehler (redefinition / conflicting types). Dieses Script kapselt
ptasks eigene Definitionen in '#ifndef SCHED_ATTR_SIZE_VER0', das genau dann
gesetzt ist, wenn glibc die Definitionen bereits bereitstellt. Auf aelteren
glibc bleibt alles wie bisher.

Idempotent: ein bereits gepatchtes ptask wird nicht erneut veraendert.
"""
import sys, os

GUARD_OPEN = "#ifndef SCHED_ATTR_SIZE_VER0  /* glibc >= 2.41 stellt diese selbst bereit */\n"
GUARD_CLOSE = "#endif /* SCHED_ATTR_SIZE_VER0 */\n\n"

def patch_file(path, open_before, close_before):
    with open(path, "r") as f:
        text = f.read()
    if "SCHED_ATTR_SIZE_VER0" in text:
        print(f"  {os.path.basename(path)}: bereits gepatcht, ueberspringe")
        return
    if open_before not in text or close_before not in text:
        print(f"  {os.path.basename(path)}: erwartete Stellen nicht gefunden "
              f"(andere ptask-Version?) - uebersprungen")
        return
    text = text.replace(open_before, GUARD_OPEN + open_before, 1)
    text = text.replace(close_before, GUARD_CLOSE + close_before, 1)
    with open(path, "w") as f:
        f.write(text)
    print(f"  {os.path.basename(path)}: gepatcht")

def main():
    src = sys.argv[1] if len(sys.argv) > 1 else "ptask/src"
    libdl_h = os.path.join(src, "libdl.h")
    libdl_c = os.path.join(src, "libdl.c")
    if not os.path.exists(libdl_h):
        print(f"patch_ptask: {libdl_h} nicht gefunden - nichts zu tun")
        return 0
    print("patch_ptask: pruefe ptask/src/libdl.{h,c} ...")
    # libdl.h: struct + Deklarationen kapseln, gettid bleibt aussen vor
    patch_file(libdl_h,
               open_before="struct sched_attr {",
               close_before="pid_t gettid(void);")
    # libdl.c: die beiden Funktionsdefinitionen kapseln, gettid bleibt aussen
    patch_file(libdl_c,
               open_before="int sched_setattr(pid_t pid, const struct sched_attr *attr,",
               close_before="pid_t gettid(void) { return syscall(__NR_gettid); }")
    return 0

if __name__ == "__main__":
    sys.exit(main())

cmake_minimum_required(VERSION 3.10)
project(PeriodicTaskset LANGUAGES C)

set(CMAKE_C_STANDARD 11)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -O0 -g -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

# --------------------------------------------------------------------------
# ptask static library (Buttazzo/Lipari).
# Repo per "git clone https://github.com/glipari/ptask" in dieses Verzeichnis
# klonen -> Quellen liegen dann unter ./ptask/src
# --------------------------------------------------------------------------
set(PTASK_DIR ${CMAKE_SOURCE_DIR}/ptask/src)

if(NOT EXISTS ${PTASK_DIR}/ptask.c)
  message(FATAL_ERROR
    "ptask-Quellen nicht gefunden unter ${PTASK_DIR}.\n"
    "Bitte zuerst:  git clone https://github.com/glipari/ptask")
endif()

# ptask fuer neuere glibc (>= 2.41, Ubuntu 26.04) patchen: glibc liefert dort
# 'struct sched_attr' / 'sched_setattr' selbst -> sonst Redefinition-Fehler.
# Das Script ist idempotent und aendert aelteres glibc nicht.
execute_process(
  COMMAND python3 ${CMAKE_SOURCE_DIR}/patch_ptask.py ${PTASK_DIR}
  RESULT_VARIABLE PTASK_PATCH_RES
  OUTPUT_VARIABLE PTASK_PATCH_OUT)
message(STATUS "patch_ptask: ${PTASK_PATCH_OUT}")

file(GLOB PTASK_SRC ${PTASK_DIR}/*.c)
# Tracing-Provider nur mit lttng noetig -> hier ausschliessen
list(FILTER PTASK_SRC EXCLUDE REGEX "tpt_provider\\.c$")

add_library(ptask STATIC ${PTASK_SRC})
target_include_directories(ptask PUBLIC ${PTASK_DIR})

# ptask ist aelterer C-Code. Neuere GCC-Versionen (>=14, Ubuntu 26.04) behandeln
# implizite Funktionsdeklarationen u. ae. standardmaessig als FEHLER. Fuer die
# Library wieder auf Warnung herabstufen, damit sie baut (betrifft nur ptask).
target_compile_options(ptask PRIVATE
  -Wno-error=implicit-function-declaration
  -Wno-implicit-function-declaration
  -Wno-error=implicit-int
  -Wno-error=int-conversion
  -Wno-error=incompatible-pointer-types)

# --------------------------------------------------------------------------
# Anwendung
# --------------------------------------------------------------------------
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod>=2.0)

add_executable(taskset src/main.c)
target_link_libraries(taskset PRIVATE ptask Threads::Threads PkgConfig::GPIOD m rt)

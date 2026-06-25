# Aufgabe 2 – Periodisches Task-System mit ptask

Drei periodische Tasks (Perioden 10/20/50 ms, fallende Priorität) werden mit
der `ptask`-Library (Buttazzo/Lipari) erzeugt. Jeder Task misst pro Aktivierung
seinen **Release-Jitter** (Abweichung der tatsächlichen von der nominalen
Periode) und schreibt die Werte als CSV. Der schnellste Task toggelt zusätzlich
**GPIO26**, damit das periodische Verhalten am **Oszilloskop** sichtbar ist.

## 1. Abhängigkeiten (Raspberry Pi 5, Ubuntu)

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config git libgpiod-dev
```

## 2. ptask-Library holen

Im Projektordner (dort, wo diese `README.md` und `CMakeLists.txt` liegen):

```bash
git clone https://github.com/glipari/ptask
```

Es werden nur die Quellen unter `ptask/src/` benötigt (reine pthread-Library,
**kein** Allegro). Die `CMakeLists.txt` baut daraus automatisch `libptask`.

## 3. Bauen

```bash
mkdir -p build && cd build
cmake ..
make
```

Ergebnis: `build/taskset`.

## 4. Ausführen

Echtzeit-Scheduling (SCHED_FIFO) und GPIO-Zugriff erfordern root:

```bash
sudo ./taskset            # läuft 15 s (Default)
sudo ./taskset 30         # optional: Laufzeit in Sekunden
```

Während des Laufs liegt am **GPIO26 (physischer Pin 37)** ein Rechtecksignal an
(Oszilloskop-Probe an Pin 37, Masse an einen GND-Pin). Beim Beenden werden je
Task die Messwerte geschrieben:

```
task0_10ms.csv
task1_20ms.csv
task2_50ms.csv
```

Jede Zeile = gemessene Zeit zwischen zwei Aktivierungen in Nanosekunden.
Der Jitter ist die Abweichung vom Sollwert (Periode in ns), z. B. 10 ms →
10 000 000 ns.

## 5. Konfiguration

In `src/main.c` oben anpassbar:

- `task_period_ms[]`, `task_priority[]`, `task_toggle[]` – das Taskset
- `GPIO_CHIP`, `GPIO_PIN` – Pin/Chip (mit `gpioinfo` prüfen; Pi 5 = `gpiochip0`)
- `TASK_PROCESSOR` – CPU-Kern für alle Tasks
- Scheduling-Policy in `ptask_init(...)`: `SCHED_FIFO`, `SCHED_RR` oder
  `SCHED_OTHER` – für den Vergleich verschiedener Verfahren

## 6. Auswertung

Die CSVs lassen sich wie in Aufgabe 1 auswerten (Mittelwert, Std/Jitter,
Worst-Case, Histogramm/Box-Plot).

## Hinweis

Falls sich die Original-`ptask` auf dem Pi nicht bauen lässt (z. B.
SCHED_DEADLINE-/Architektur-Themen), kann alternativ eine schlanke, eigenständige
ptask-Variante (nur pthreads + `clock_nanosleep`) verwendet werden – einfach
Bescheid geben.

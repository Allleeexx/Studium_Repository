# FreeRTOS Bonusaufgabe – Kurzbericht

Datum: 28.10.2025
Projekt: STM32F407 (CubeIDE) – `demo3`
Autor: <Name eintragen>

---

## 1. Installierte Tasks und ihre Nachrichten

- defaultTask (`StartDefaultTask`)
  - Startet USB Host, startet TIM7-Interrupts (LED‑Blinken). Keine Nachrichten.

- producerTask (`StartTask02`)
  - Legt 10 Zufallszahlen (0–99) in `myQueue01`, danach eine `0` als Batch‑Ende.
  - Wartet anschließend auf das Binärsemaphor `myBinarySem01`.

- consumerTask (`StartTask03`)
  - Liest fortlaufend aus `myQueue01`.
  - Bei `0`: bildet Mittelwert über die vorangegangenen Werte und gibt `myBinarySem01` frei (Handshake zum Producer).

- Button‑Task (`StartUserTasteTask`)
  - Liest User‑Button (PA0) und sendet alle 100 ms an `displayQueue`:
  - `DisplayMessage { type: MSG_BUTTON, value: 0|1 }`

- Random‑Generator (`StartBonusTaskRandomGen`)
  - Generiert jede Sekunde eine Zufallszahl (0–99) und sendet sie an `evaluateQueue` (uint32_t).

- Auswertung (`StartBonusTaskAuswertung`)
  - Bildet aus jeweils 5 Werten den Mittelwert und sendet an `displayQueue`:
  - `DisplayMessage { type: MSG_AVERAGE, value: mittelwert }`

- (Simulierter) ADC‑Task (`StartBonusTaskAdc`)
  - Simuliert 12‑Bit‑ADC‑Werte (0–4095) per RNG im 200‑ms‑Takt und sendet an `displayQueue`:
  - `DisplayMessage { type: MSG_ADC, value: 0..4095 }`

- Display‑Task (`StartBoDisplay`)
  - Holt `DisplayMessage` aus `displayQueue` und schreibt Zeilen auf das LCD:
    - y=20:  Buttonzustand
    - y=40:  Mittelwert (aus 5 Werten)
    - y=60:  „ADC“‑Wert (simuliert)
  - Zugriff wird durch `BonusSemaphore` (als einfacher Schutz) umschlossen.

Zusätzlich:
- TIM7‑IRQ toggelt LED (PD13) ca. 1 Hz.

---

## 2. Queue‑Konfiguration

Nachrichtenstruktur (siehe `Core/Src/main.c`):

```c
typedef enum {
  MSG_BUTTON,
  MSG_AVERAGE,
  MSG_ADC
} MessageType;

typedef struct {
  MessageType type;
  uint32_t value;
} DisplayMessage;
```

Queues:

- `myQueue01`
  - Anzahl: 16 Einträge
  - Elementgröße: `sizeof(uint32_t)`
  - Inhalt: Producer‑Werte (0–99) + Batch‑Ende‑Marker (`0`).

- `evaluateQueue`
  - Anzahl: 16 Einträge
  - Elementgröße: `sizeof(uint32_t)`
  - Inhalt: Zufallswerte (0–99) für 5er‑Mittelwert.

- `displayQueue`
  - Anzahl: 16 Einträge
  - Elementgröße: `sizeof(DisplayMessage)`
  - Inhalt: `DisplayMessage` aus Button/ADC/Auswertung.

Semaphore/Timer:
- `myBinarySem01`: Binärsemaphor Producer↔Consumer (Handshake).
- `BonusSemaphore`: Schutz um die LCD‑Ausgabe (optional, aber implementiert).
- Software‑Timer (`myTimer01`) ist angelegt; TIM7 läuft als HW‑Timer für LED‑Blinken.

---

## 3. Aufbau der Display‑Task (+ Screenshot)

- Blockiert auf `displayQueue` und verarbeitet danach je nach `msg.type`:
  - `MSG_BUTTON`  → Zeile y=20: "Button: Pressed" / "Button: Not Pressed"
  - `MSG_AVERAGE` → Zeile y=40: "Mittelwert: <wert>"
  - `MSG_ADC`     → Zeile y=60: "ADC: <wert>"
- Jede Ausgabe innerhalb `BonusSemaphore` gekapselt, um den LCD‑Zugriff zu serialisieren.

Platz für Screenshot (bitte hier einfügen):

![Screenshot LCD‑Ausgabe](./screenshot_lcd_placeholder.png)

Hinweis: Wenn kein Bild vorliegt, einfach ein Foto vom Board mit sichtbarem Display hier einfügen (Datei ins `docs/`-Verzeichnis legen und oben den Dateinamen anpassen).

---

## 4. Größte Probleme und Lösungen

1) Falsche Queue‑Elementgröße für `displayQueue`
- Problem: Queue war als `uint16_t` konfiguriert, tatsächlich wurden `DisplayMessage`‑Strukturen gesendet → Datenkorruption.
- Lösung: Elementgröße auf `sizeof(DisplayMessage)` korrigiert; Daten‑Queues konsequent auf `sizeof(uint32_t)` gestellt.

2) LED‑Blinken über falschen Timer
- Problem: LED‑Toggling war mit TIM6 (HAL Timebase) vermischt.
- Lösung: LED‑Blinken auf TIM7 gelegt, eigener Zähler `ledBlinkCounter` → saubere Trennung.

3) Anzeige‑Überlappung / Button‑Logik
- Problem: Texte überlagerten sich; Button‑Status war invertiert.
- Lösung: Feste Zeilen (y=20/40/60) + korrekte Interpretation von PA0; Strings leicht gepolstert, damit alte Zeichen überschrieben werden.

---

## 5. Wie testen

- Flashen, Board starten.
- Beobachten:
  - LED (PD13) blinkt ca. 1 Hz.
  - LCD zeigt drei Zeilen: Button, Mittelwert (aktualisiert ~alle 5 s), ADC (simuliert, ~5 Hz).
  - Button (PA0) drücken und Statuswechsel prüfen.

---

## 6. PDF erzeugen (Windows)

Option A – Drucken als PDF:
1. Öffne diese Datei in der Markdown‑Vorschau (VS Code: `Strg+Shift+V`).
2. Rechtsklick → "Open in Browser" (falls verfügbar) oder Inhalt in Browser kopieren.
3. Im Browser: Drucken → "Microsoft Print to PDF" → Speichern als PDF.

Option B – Word/LibreOffice:
1. Datei `Bonus_Report.md` in Word/LibreOffice öffnen.
2. "Speichern unter" → PDF.

Option C – VS Code Extension (Markdown PDF):
1. Erweiterung "Markdown PDF" installieren.
2. In dieser Datei Rechtsklick → "Markdown PDF: Export (pdf)".

---

## 7. Version / Referenzen
- Geänderte Datei: `Core/Src/main.c`
- Display‑API: `Core/Inc/display.h`
- Datum dieser Fassung: 28.10.2025

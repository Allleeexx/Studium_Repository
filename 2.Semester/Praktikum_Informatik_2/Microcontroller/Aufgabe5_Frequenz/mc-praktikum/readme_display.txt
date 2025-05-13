Falls Sie die Display Bonuspunkte-Aufgabe nicht angehen möchten, können Sie alternativ display.h und display.o in Ihr Project einbinden. Gehen Sie dabei wie folgt vor:

    Kopieren Sie die beiden Dateien display.h und display.o in das Entwicklungsverzeichnis
    Bitte nutzen Sie den ARM Compiler Version 6.
    Inkludieren Sie display.h in ihr main Programm mittels #include "display.h"
    Fügen Sie in ihrem Project Explorer mittels C Source -> Add existing file ... -> Filter: *.o die Datei display.o zu ihrem Projekt hinzu
    Bitte beachten: die Funktion LCD_Output16BitWord() muss in Ihrem Projekt vorhanden sein! Falls nicht, bekommen Sie eine Linker-Warnung ".\Objects\MCPraktikum.axf: Error: L6218E: Undefined symbol LCD_Output16BitWord (referred from display.o)."
    Rufen Sie in der Funktion main() zunächst die Funktion LCD_Init() auf. Dann können Sie mit LCD_ClearDisplay() das Display löschen und mittels LCD_WriteString() Text auf den Bildschirm schreiben. Als Farbcode können Sie z.B: 0x0000 (BLACK), 0xFFFF (WHITE) oder 0xFE00 (RED) verwenden:

LCD_Init()
LCD_ClearDisplay( 0xFE00 );
LCD_WriteString( 10, 10, 0xFFFF, 0x0000, "Hallo Welt!");

    Beachten Sie, dass das Hintergrundlicht des Displays angeschaltet sein muss (Pin PD13 muss 1 sein), sonst bleibt der Bildschirm schwarz und Sie sehen nichts!
    Am Ende sollte das Display und das Lauflicht gleichzeitig funktionieren! Bei Problemen können Sie das im Praktikum debuggen (Achten Sie auf Pin PD11!).

Getestet am 5.4.2024 mit Keil uVision 5.39 und Compiler V6.21 "Default compiler version 6".
# EDF-Kurzbeschreibung

Für den Bonus wurde ein einfacher EDF-Scheduler über FreeRTOS-Prioritäten umgesetzt. Jeder Task besitzt eine Periode, eine relative Deadline und eine kleine Queue für verzögerte Aktivierungen. Bei jeder Aktivierung wird aus der relativen Deadline eine absolute Deadline berechnet. Danach werden die bereiten Tasks nach der frühesten Deadline sortiert und ihre FreeRTOS-Prioritäten entsprechend angepasst.

Die Taskdaten stehen in der Struktur `EdfTask`. Die wichtigsten Debug-Variablen sind `edfCurrentOrder`, `edfReadyMask`, `deadlineMissA-D` und `responseTaskAMaxUs-DMaxUs`.
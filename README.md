# Warpgate - Sistem distribuit pentru execuția paralelă de sarcini în rețele peer-to-peer

https://github.com/tudor-c/warpgate

Proiectul facilitează proiectarea și implementarea unei rețele distribuite de calcul care îmbină avantajele arhitecturii centralizate cu cele ale unei rețele peer-to-peer. Rețeaua formată este compusă din server-ul central (Tracker) și restul rețelei, formată din Clienți, în care orice client poate solicita execuția de task-uri și poate, la rândul lui, prelua sarcini de la alți membri pentru a le procesa.

## Instalarea
1. Clonează repository-ul: `git clone https://github.com/tudor-c/warpgate`
2. Din directorul proiectului, configurarea și build-ul se face printr-o singură comandă: `./build-scripts/build.py --clean --configure`. Există și un symlink `./build` către script chiar în rădăcina proiectului pentru accesibilitate.
3. Executabilul compilat se găsește la calea `output/warpgate`, iar biblioteca dinamică este la `output/libtasklib.so`
4. Mută executabilul `warpgate` obtinut pe toate mașinile care vor face parte din sistem. (Opțional) Plasează `warpgate` într-un director accesibil din PATH pentru invocare mai ușoară (de exemplu, pe sisteme Unix `/usr/local/bin` sau `~/.local/bin`)

## Utilizarea
Pentru a alege configurația în care e lansată aplicația, pasează ca prim flag `tracker` sau `client`:
``` bash
warpgate tracker # pe nodul central
warpgate client  # pe workeri
warpfate client --task example_task.json # pe acquireri
```

Tracker nu are nevoie de opțiuni suplimentare, iar pornirea unui worker se face doar prin pasarea parametrului `client`. Pentru lansarea unui Acquirer și trimiterea unui nou task în rețea, mai este nevoie de specificarea căii catre fișierul JSON de configurare a task-ului, precedată de flag-ul `--task`. Programul se va lansa și rularea subtask-urilor va putea fi urmărita prin mesajele de logging.

Scrierea de task-uri noi se face compilând o biblioteca dinamica ce contine funcțiile dorite. Procesul poate fi simplificat, folosind build system-ul pus deja la dispozitie de proiect. Astfel, se pot aduce modificări locale fișierului `src/tasklib/tasklib.cpp`. În el se va pune codul dorit și se va compila în același mod ca mai înainte. Astfel, se vor putea folosi și bibliotecile prezente deja în proiect, cum ar fi msgpack, pentru serializarea și deserializarea parametrilor și valorilor de return ale funcțiilor. Aceste valori trebuie să respecte același tip comun, fie `std::vector<unsigned char>`, fie `std::string`, alegere care se face la compilarea proiectului prin schimbarea tipului `ResultType` din types.h, astfel ca se poate alege intre flexibilitatea manipulării de orice dat e binare sau ușurința manipulării de date text. In cazul alegerii `std::vector<unsigned char>`, recomand folosirea funcțiilor din biblioteca msgpack pentru serializare și deserializare.

În plus, se va scrie fișierul JSON de configurare, care să exprime structura arborescentă a task-ului dorit. Un exemplu poate fi găsit la calea `resources\test_task_1.json`. Tot acolo se găsesc și exemple de date de intrare care pot fi folosite impreuna cu codul dat exemplu din `tasklib.cpp`.

Aceasta este funcționalitatea de bază a sistemului. Pentru detalii despre opțiuni extinse, se poate pasa flag-ul `--help`.
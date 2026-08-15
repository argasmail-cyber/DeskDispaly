# CHANGELOG v8.4

## v8.4 (2026-08-15) - Ritocchi estetici e UX

### Modifiche
- **Stile orologio di default = LARGE** invece di Standard. Cambia solo il
  valore iniziale in `SystemState`; tutto il resto (ciclo stili col bottone,
  auto-cycle) resta invariato.
- **Nuova melodia di avvio**: arpeggio maggiore (C5-E5-G5-C6-E6-C6-E6-G6) più
  musicale rispetto alla scaletta precedente, con nota finale più lunga per
  dare un senso di chiusura.
- **Schermata orologio Standard**: ora mostra sia Godo che Milano in una riga
  compatta ("Godo 22° Mi 18°"), invece della sola Godo con icona — non c'era
  spazio per due icone leggibili affiancate su 128px, quindi ho tolto le
  icone e tenuto solo il testo, centrato.
- **Rimossi i pallini di navigazione pagina** (in basso a destra sulle
  schermate meteo/status): non erano chiari come indicatore, tolti del tutto
  insieme alla funzione `drawScreenDots` (non più referenziata da nessuna
  parte).

### File modificati
- `config.h` — stile orologio di default
- `buzzer.cpp` — nuova melodia di avvio
- `display_modes.cpp` / `display_modes.h` — Milano in schermata Standard,
  rimossi i pallini pagina
- `DeskDisplay_v8_4.ino` — solo stringhe di versione

### Nota
Se preferisci lo spazio vuoto invece di Godo+Milano nella schermata Standard,
è una modifica di un paio di righe in `renderClockStandard()` — dimmelo pure.

### Proposte di nuovi stili orologio (da confermare prima di implementarli)
1. **WAVE** — un'onda sinusoidale animata che scorre dietro l'ora, ampiezza e
   velocità leggermente variabili nel tempo per un effetto rilassante.
2. **STARFIELD** — piccoli punti che si muovono lentamente come un campo
   stellare sullo sfondo, con l'ora ben leggibile sopra.
3. **ORBIT** — un piccolo punto che orbita attorno all'ora come un pianeta,
   con periodo di rotazione legato ai secondi (una figura elegante e minimal,
   diversa da Mario/Invaders che sono più "giocosi").
4. **BINARY** — sotto l'ora in cifre normali, una riga di puntini che
   rappresentano l'ora in binario: un tocco più "da smanettone", coerente col
   tuo profilo da home-lab enthusiast.

Fammi sapere quali ti piacciono (anche più di uno) e li implemento nella
prossima versione.

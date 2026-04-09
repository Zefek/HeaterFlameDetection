# HeaterFlameDetection

Jednoduchý hlídač plamene pro kotle na tuhá paliva postavený na Arduinu Nano.

Krabička sleduje teplotu ve spalinovém kanále, pozná zátop podle nárůstu teploty, přemostí spalinový termostat a pak hlídá, jestli kotel stále hoří. Žádný displej, žádná tlačítka — jen dvě LED a buzzer.

## Proč by to někdo chtěl?

Kdo topí dřevem, zná tenhle scénář: zatopíš, otočíš spalinový termostat na zátop, odejdeš — a buď zapomeneš termostat přetočit na provozní teplotu, nebo zátop nechytne a ty to zjistíš až za hodinu. Případně kotel vyhasne a ventilátor s laddomatem běží dál zbytečně.

Tahle krabička:

- Pozná, že kotel startuje (nárůst teploty spalin)
- Přemostí spalinový termostat — už ho nemusíš přetáčet
- Hlídá, že kotel doopravdy chytl (do hodiny se musí dostat nad provozní teplotu)
- Pokud zátop selže, rozepne relé a spustí alarm
- Když kotel dohoří, zase se sama vypne a ventilátor s laddomatem se zastaví

Komerční ekvivalent neexistuje. Spalinové termostaty jsou hloupé bimetalové kontakty.

## Jak to funguje

1. **IDLE** — krabička čeká, měří teplotu spalin, nic nesvítí.
2. **Detekce zátopu** — jakmile teplota spalin začne stoupat rychlostí 3–15 °C/min, krabička to vyhodnotí jako zátop. Každý validní vzorek blikne zelenou. Po 7 po sobě jdoucích vzorcích (cca 7 sekund) přejde dál.
3. **BURNING** — krabička sepne relé (přemostí spalinový termostat), zelená svítí trvale. Od této chvíle má hodinu na to, aby se teplota spalin dostala nad 85 °C (= kotel reálně hoří).
4. **Timeout / selhání** — pokud zátop do hodiny nechytl, krabička rozepne relé, rozsvítí červenou LED a na 2 minuty spustí buzzer. Po dalších 5 minutách chladnutí se automaticky vrátí do IDLE.
5. **Dohašení** — když kotel dohoří a teplota spalin klesne pod 75 °C (a uběhlo aspoň 15 minut od startu), krabička rozepne relé a vrátí se do IDLE.

### Stavy LED

| Stav | Zelená | Červená | Buzzer |
|---|---|---|---|
| IDLE | — | — | — |
| Zátop (detekce gradientu) | bliká | — | — |
| Kotel hoří | svítí | — | — |
| Selhání zátopu | — | svítí | pípá (2 min) |

## Hardware

### Součástky

| Díl | Poznámka |
|---|---|
| Arduino Nano | Stačí i klon |
| PT1000 čidlo | Do pouzdra odolného vůči teplotě spalin (keramika / ocel) |
| Rezistor 10 kΩ | Pro dělič napětí k PT1000 |
| Relé modul 5 V (active LOW) | 1 kanál stačí, s optočlenem |
| LED zelená + rezistor ~330 Ω | |
| LED červená + rezistor ~330 Ω | |
| Aktivní buzzer 5 V | |
| Napájení 5 V | Stačí běžný USB zdroj |

### Zapojení

```
PT1000 dělič:
   5V ──[PT1000]──┬──[10k]── GND
                  │
                  └──> A0

Arduino Nano piny:
   A0 ── PT1000 (mid-point děliče)
   D2 ── vstup relé (IN)
   D3 ── zelená LED (+ 330Ω na GND)
   D4 ── červená LED (+ 330Ω na GND)
   D5 ── aktivní buzzer (+ na GND)

Relé se zapojí paralelně ke kontaktům spalinového termostatu —
když krabička sepne relé, termostat je přemostěn a kotel běží
nezávisle na jeho poloze.
```

**Důležité:** PT1000 musí být v kovovém / keramickém pouzdře odolném vůči teplotám až ~400 °C a umístěn ve spalinovém kanále blízko za výstupem z kotle. Čím blíže ke kotli, tím rychlejší reakce na zátop i vyhasnutí.

Podrobný popis zapojení všech bloků, bezpečnostní poznámky a blokové schéma najdeš v [docs/wiring.md](docs/wiring.md).

## Instalace firmware

1. Nainstaluj [Arduino IDE](https://www.arduino.cc/en/software) nebo [arduino-cli](https://arduino.github.io/arduino-cli/).
2. Naklonuj repo.
3. Otevři [HeaterFlameDetection/HeaterFlameDetection.ino](HeaterFlameDetection/HeaterFlameDetection.ino).
4. Vyber desku **Arduino Nano** (případně ATmega328P Old Bootloader, pokud máš starší klon).
5. Nahraj.

Nebo z příkazové řádky:

```bash
arduino-cli compile --fqbn arduino:avr:nano ./HeaterFlameDetection
arduino-cli upload --fqbn arduino:avr:nano --port COMx ./HeaterFlameDetection
```

## Konfigurace

Všechno laditelné je v [HeaterFlameDetection/config.h](HeaterFlameDetection/config.h):

| Parametr | Výchozí | Význam |
|---|---|---|
| `WGT_ON` | 85 °C | Nad touto teplotou je kotel "opravdu hoří" |
| `WGT_OFF` | 75 °C | Pod touto teplotou je kotel "studený" |
| `GRADIENT_MIN` | 3 °C/min | Minimální nárůst pro detekci zátopu |
| `GRADIENT_MAX` | 15 °C/min | Maximální nárůst (filtr proti glitchům) |
| `GRADIENT_TRY_COUNT` | 7 | Počet platných vzorků pro sepnutí relé |
| `IGNITION_TIMEOUT` | 3 600 000 ms (1 h) | Čas na to, aby zátop přešel do plného hoření |
| `MINIMUM_BURN_TIME` | 900 000 ms (15 min) | Minimální doba běhu před návratem do IDLE |
| `BUZZER_DURATION` | 120 000 ms (2 min) | Délka alarmu při selhání |

Prahy jsou nastavené pro běžný kotel na dřevo. Pokud máš jiný kotel nebo jiné umístění čidla, možná bude potřeba je upravit. Doporučuji:

1. Nahrát firmware s defaultními hodnotami.
2. Zatopit a sledovat chování přes sériový monitor (`Serial.begin(9600)`).
3. Podle reálných teplot doladit `WGT_ON` / `WGT_OFF`.

## Status projektu

Projekt je ve fázi **funkční základ**. Logika detekce je převzatá z většího projektu [HeatingTemperatureRegulator](https://github.com/Zefek/HeatingTemperatureRegulator), kde běží v produkci od topné sezóny 2025/2026.

### Co chybí / co by se hodilo

- Ověření na reálném HW jako samostatné zařízení (zatím jen v rámci většího projektu)
- KiCad / Fritzing schéma
- Fotky instalace PT1000 do spalinového kanálu
- Varianta pro ESP32 / ESP8266 s MQTT notifikacemi
- Test mode (zkrácené timeouty pro ladění)

PRs vítány.

## Licence

MIT — viz [LICENSE](LICENSE).

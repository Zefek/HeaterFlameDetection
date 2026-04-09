# Contributing to HeaterFlameDetection

Díky za zájem o projekt! Tenhle dokument popisuje, jak přispívat kódem, nápady nebo zpětnou vazbou.

## Než začneš

- Přečti si [README.md](README.md), abys rozuměl/a tomu, co projekt dělá a proč.
- Přečti si [CLA.md](CLA.md) — každý příspěvek implicitně spadá pod MIT licenci.
- Pokud plánuješ větší změnu (nový stav automatu, podpora jiné platformy, zásadní refactoring), otevři nejdřív **issue** s návrhem, ať se o tom můžeme pobavit, než na tom strávíš hodiny.

## Jak nahlásit chybu

Otevři issue a uveď:

1. **Co se stalo** — reálné chování.
2. **Co jsi čekal/a** — očekávané chování.
3. **Jak to reprodukovat** — jaký kotel, jaké teploty, v jakém stavu to selhalo.
4. **Hardware** — deska (Nano, klon, ...), PT1000 (typ, umístění), relé modul.
5. **Verze firmware** — commit hash nebo tag.
6. **Log** — výstup ze sériového monitoru (9600 baud), pokud máš.

## Jak navrhnout funkcionalitu

Otevři issue označené jako **enhancement** a popiš:

- Jaký problém to řeší.
- Jak by se to chovalo z pohledu uživatele (LED, buzzer, relé).
- Jestli to vyžaduje nový hardware nebo změnu zapojení.

Filozofie projektu je **jednoduchost**. Krabička má být tak jednoduchá, aby si ji bastlíř postavil za víkend a pochopil kód za odpoledne. Návrhy jako "přidat WiFi", "přidat displej", "přidat menu" budou pravděpodobně odmítnuty do hlavní větve — ale jsou vítány jako samostatné varianty (např. `HeaterFlameDetection-ESP32/`).

## Jak poslat pull request

1. **Fork + branch** — vytvoř si vlastní fork a pracuj na samostatné větvi (`fix/neco`, `feat/neco`).
2. **Malé commity** — jeden logický celek = jeden commit. Zpráva v angličtině nebo češtině, hlavní věc je jasnost.
3. **Otestuj** — aspoň zkompiluj přes `arduino-cli`, ideálně otestuj na reálném HW.
4. **Vyplň PR šablonu** — je tam checklist pro hardware, testování a CLA.
5. **Čekej na review** — projekt udržuje jeden člověk ve volném čase, review může chvíli trvat.

### Co pravděpodobně bude přijato

- Oprava chyby v detekci (false positive / false negative)
- Zpřesnění převodu PT1000
- Lepší default hodnoty prahů po ověření na reálném kotli
- Schéma zapojení (KiCad, Fritzing)
- Fotky instalace
- Překlady README
- Oprava typos a dokumentace

### Co pravděpodobně nebude přijato do mainu

- Přidání WiFi, MQTT, displeje, tlačítek, menu — to je jiný projekt
- Generická abstrakce "pro případ, že by někdo chtěl jiný senzor"
- Refactoring bez jasného přínosu
- Změna stylu kódu bez funkční změny

## Styl kódu

- Odsazení 2 mezery.
- Prahy a časy do `config.h`, ne do `.ino`.
- Komentáře jen tam, kde logika není samozřejmá (např. proč je ten převod PT1000 takhle napsaný).
- `snake_case` pro konstanty maker, `camelCase` pro proměnné a funkce.
- Žádné dynamické alokace, žádné `String` objekty.

## CI kontroly

Každý PR projde přes GitHub Actions:

- **cppcheck** s minimálním počtem suppressů.
- **arduino-cli compile** pro `arduino:avr:nano`.

Pokud něco selže, oprav to a pushni znova — není potřeba otevírat nový PR.

## Otázky

Pokud si nejsi jist/á čímkoli — otevři issue a zeptej se. Raději hloupá otázka před prací než zbytečně strávený víkend.

Díky za příspěvek!

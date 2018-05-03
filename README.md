Sources et fichiers pour le magazine Hackable #24
=================================================

Ce dépôt contient les croquis, sources et fichiers utilisés dans le 24ème numéro de [Hackable](http://www.hackable.fr/) :

* `esp_si4021web2` : croquis Arduino pour ESP8266 permettant de piloter l'installation luminaire télécommandée avec une puce si4021.
* `esp_NOsi4021web2` : croquis similaire mais utilisant directement une télécommande modifiée comme émetteur (sans si4021 donc).
* `Z80ctl_mem_74165RAM_serial` : croquis Arduino du "moniteur" Z80 avec un support spécial pour l'inspection du dialogue avec l'UART.
* `serial` : code en C et assembleur pour Z80 permettant l'émission de messages via un UART 16550.
* `SIDplayer/hardSIDuino` : croquis Arduino pour le pilotage de la puce sonore CSG 8580 (alias SID) du Commodore 64.
* `SIDplayer/hardSIDuino_notimer` : idem mais sans génération du signal d'horloge de 1 Mhz (oscillateur à quartz directement dans le montage).
* `SIDplayer/hardSIDuino_notimer_noclock` : idem mais avec une cadence de lecture gérée par le lecteur Python et non le croquis lui-même.
* `SIDplayer/Python/RealSID.py` : script Python de lecture de fichiers musicaux SID (version cadencée par le croquis Arduino).
* `SIDplayer/Python/RealSIDclock.py` : idem, mais version "final" avec une lecture cadencée directement par le script Python.


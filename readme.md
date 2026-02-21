# Canon X-07 RPG (Z80)

## 🇫🇷 Français

Petit RPG programmé en assembleur Z80 à destination de l’ordinateur de poche **Canon X-07**.

Le projet vise à exploiter les capacités graphiques, le clavier et le contrôleur LCD (T6834) du X-07 pour proposer un mini-RPG complet en environnement 8-bit.

---

### Compilation

Il est recommandé d’assembler le projet avec **sjasmplus** :

```bash
sjasmplus --raw=rpg.bin rpg.z80
```

Cela génère un binaire brut `rpg.bin`.

---

### Chargement en mémoire

Le binaire doit être injecté à l’adresse :

```
0x2000
```

C’est le début de la mémoire RAM additionnelle de 8 ko.

⚠️ Si vous souhaitez utiliser une autre adresse, il faudra adapter le code source en conséquence.

---

### Fichier `canon_x07.inc`

Le fichier `canon_x07.inc` contient les définitions des constantes, appels ROM et structures système.

Il peut être complété si nécessaire.
Il est principalement basé sur la documentation technique disponible pour le Canon X-07.

---

### Déjà implémenté

* Routines graphiques (buffer écran + blit vers LCD)
* Gestion map / tiles
* Tilemapper par "chunks" (pas de scrolling continu, affichage trop lent)
* Système de menus
* Déclenchement aléatoire des combats
* Base du système de combat

---

### TODO

* Collisions du tilemapper
* Gestion des événements / dialogues
* Gestion complète des stats joueur / ennemis
* Finaliser le système de combats
* Ajouter du son / de la musique
* Gestion des sauvegardes
* Si possible, résoudre le problème d’octets non copiés à l’écran par le T6834

---

## 🇬🇧 English

A small RPG written in Z80 assembly for the **Canon X-07** pocket computer.

This project aims to leverage the graphical capabilities, keyboard, and LCD controller (T6834) of the X-07 to create a complete mini-RPG in an 8-bit environment.

---

### Compilation

It is recommended to assemble the project using **sjasmplus**:

```bash
sjasmplus --raw=rpg.bin rpg.z80
```

This generates a raw binary file named `rpg.bin`.

---

### Memory Loading

The binary must be injected at address:

```
0x2000
```

This corresponds to the beginning of the 8 KB additional RAM.

⚠️ If you wish to use a different address, the source code must be modified accordingly.

---

### `canon_x07.inc` File

The `canon_x07.inc` file contains constant definitions, ROM calls, and system structures.

It can be extended if necessary.
It is primarily based on the available technical documentation for the Canon X-07.

---

### Already Implemented

* Graphics routines (screen buffer + LCD blitting)
* Map / tile handling
* Chunk-based tilemapper (no smooth scrolling due to display speed limitations)
* Menu system
* Random battle triggering
* Base battle system

---

### TODO

* Tilemapper collision handling
* Event / dialogue system
* Complete player and enemy stat management
* Finalize the battle system
* Add sound / music
* Save system
* If possible, fix the issue of bytes not being properly copied to the screen by the T6834
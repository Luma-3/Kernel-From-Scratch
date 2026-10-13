# 🧠 Rapport Technique : Mapping de la Mémoire & Gestion Physique avec Support PAE (Physical Address Extension)

> **Projet :** Kernel From Scratch (KFS) — Architecture x86 (IA-32 / i386) avec PAE  
> **Auteur :** Antigravity AI Assistant  
> **Contexte :** Évolution du sous-système mémoire vers l'adressage physique étendu (36 bits / 64 Go), pagination 3 niveaux et protection matérielle NX/XD.

---

## Table des Matières
1. [Introduction & Pourquoi le PAE ?](#1-introduction--pourquoi-le-pae-)
2. [Architecture Matérielle de la Pagination avec PAE](#2-architecture-matérielle-de-la-pagination-avec-pae)
   - [2.1 Hiérarchie à 3 Niveaux (PDPT, Page Directory, Page Table)](#21-hiérarchie-à-3-niveaux-pdpt-page-directory-page-table)
   - [2.2 Décomposition d'une Adresse Virtuelle sous PAE](#22-décomposition-dune-adresse-virtuelle-sous-pae)
   - [2.3 Format des Entrées 64 bits (PDPE, PDE, PTE)](#23-format-des-entrées-64-bits-pdpe-pde-pte)
   - [2.4 Grandes Pages (Large Pages de 2 Mo)](#24-grandes-pages-large-pages-de-2-mo)
   - [2.5 Le Bit NX / XD (Execute-Disable / No-Execute)](#25-le-bit-nx--xd-execute-disable--no-execute)
3. [Impact sur le Gestionnaire de Mémoire Physique (PMM)](#3-impact-sur-le-gestionnaire-de-mémoire-physique-pmm)
   - [3.1 Types de Données : Passage au 64 bits (`uint64_t`)](#31-types-de-données--passage-au-64-bits-uint64_t)
   - [3.2 Dimensionnement du Bitmap Allocator pour 64 Go](#32-dimensionnement-du-bitmap-allocator-pour-64-go)
   - [3.3 L'Explosion de la Zone HIGHMEM et la Problématique du Noyau 32 bits](#33-lexplosion-de-la-zone-highmem-et-la-problématique-du-noyau-32-bits)
   - [3.4 Le Buddy Allocator sous PAE](#34-le-buddy-allocator-sous-pae)
4. [Techniques de Mapping Virtuel Avancées sous PAE](#4-techniques-de-mapping-virtuel-avancées-sous-pae)
   - [4.1 Le Paging Récursif (Fractal Mapping) adapté au PAE](#41-le-paging-récursif-fractal-mapping-adapté-au-pae)
   - [4.2 Mécanisme de Fenêtrage Temporaire (`kmap` / `kmap_atomic`)](#42-mécanisme-de-fenêtrage-temporaire-kmap--kmap_atomic)
5. [Procédure d'Activation Pas à Pas du PAE (Code & Algorithme)](#5-procédure-dactivation-pas-à-pas-du-pae-code--algorithme)
6. [Tableau Comparatif : x86 Legacy (32 bits) vs x86 PAE (36/64 bits)](#6-tableau-comparatif--x86-legacy-32-bits-vs-x86-pae-3664-bits)

---

## 1. Introduction & Pourquoi le PAE ?

Sur l'architecture x86 32 bits standard (i386 / IA-32), les bus d'adresses physique et virtuel font tous deux 32 bits, limitant strictement la mémoire vive installable à :
$$2^{32}\text{ octets} = 4\text{ Go de RAM physique}$$

Introduit par Intel avec le **Pentium Pro (1995)**, le mécanisme **PAE (Physical Address Extension)** permet à un processeur 32 bits d'accéder à **64 Go de mémoire RAM physique** (bus d'adresses physiques étendu à 36 bits : $2^{36}$ octets), voire jusqu'à 4 To (52 bits) sur les processeurs modernes.

### Règle Fondamentale du PAE :
> **Les adresses physiques passent à 36/64 bits, mais les adresses virtuelles restent strictement sur 32 bits !**
> Chaque processus utilisateur ne peut toujours voir qu'un espace virtuel de 4 Go au maximum à un instant donné. En revanche, le système d'exploitation peut exécuter simultanément plusieurs dizaines de processus occupant chacun 2 ou 3 Go dans un pool physique global de 16, 32 ou 64 Go.

De plus, le PAE est le prérequis matériel indispensable pour activer le bit de sécurité **NX / XD (No-Execute / Execute-Disable)** qui protège contre l'exécution de code malveillant sur la pile ou le tas (Data Execution Prevention - DEP).

---

## 2. Architecture Matérielle de la Pagination avec PAE

### 2.1 Hiérarchie à 3 Niveaux (PDPT, Page Directory, Page Table)

En mode legacy (sans PAE), les entrées font 32 bits (4 octets) et se répartissent sur 2 niveaux ($1024 \times 1024$).  
Avec le PAE, pour pouvoir stocker des adresses physiques supérieures à 4 Go (jusqu'à 36 bits ou 52 bits), **chaque entrée doit désormais faire 64 bits (8 octets)**.

Une page de 4096 octets ne peut donc plus contenir 1024 entrées, mais seulement :
$$\frac{4096\text{ octets}}{8\text{ octets par entrée}} = 512\text{ entrées}$$

Pour continuer à mapper l'intégralité de l'espace virtuel de 4 Go ($2^{32}$ octets), le processeur ajoute un 3ᵉ niveau au sommet : le **PDPT (Page Directory Pointer Table)**.

```
                           +----------------------------------------+
                           |  Registre CR3 (Adresse physique PDPT)  |
                           +----------------------------------------+
                                                │
                                                ▼
                               +---------------------------------+
                               |  PDPT (4 entrées de 64 bits)    | (Niveau 3 - 32 octets)
                               +---------------------------------+
                                 │         │         │         │
                 ┌───────────────┘         │         │         └───────────────┐
                 ▼                         ▼         ▼                         ▼
   +---------------------------+     +-----------+ +-----------+     +---------------------------+
   | Page Directory 0 (512 e.) |     |  PD 1     | |  PD 2     |     | Page Directory 3 (512 e.) | (Niveau 2 - 4 Ko chacun)
   +---------------------------+     +-----------+ +-----------+     +---------------------------+
     │                                                                 │
     ▼                                                                 ▼
   +---------------------------+                                     +---------------------------+
   | Page Table 0 (512 entrées)|                                     | Page Table (512 entrées)  | (Niveau 1 - 4 Ko chacun)
   +---------------------------+                                     +---------------------------+
     │                                                                 │
     ▼                                                                 ▼
   [ Cadre Physique 4 Ko       ]                                     [ Cadre Physique 4 Ko       ]
   (Adresse physique 36-bit)                                         (Adresse physique 36-bit)
```

1. **PDPT (Page Directory Pointer Table) :**
   - Composé de seulement **4 entrées** de 8 octets (total : 32 octets).
   - Chaque entrée couvre $1\text{ Go}$ d'espace virtuel ($4 \times 1\text{ Go} = 4\text{ Go}$).
   - **Contrainte critique :** Le PDPT doit être aligné sur une frontière de 32 octets (généralement aligné sur 4 Ko pour simplifier) et **doit impérativement résider sous la barre des 4 Go physiques** car le registre `CR3` en mode protégé 32 bits ne stocke que 32 bits de base physique.
2. **Page Directory (PD) :**
   - 4 répertoires distincts (un par entrée du PDPT).
   - Chaque répertoire contient **512 entrées** de 64 bits (taille exacte : 4096 octets).
   - Chaque entrée de PD couvre $512 \times 4\text{ Ko} = 2\text{ Mo}$ d'espace virtuel.
3. **Page Table (PT) :**
   - Contient **512 entrées** de 64 bits (taille exacte : 4096 octets).
   - Chaque entrée couvre un cadre physique standard de 4 Ko.

---

### 2.2 Décomposition d'une Adresse Virtuelle sous PAE

L'adresse linéaire virtuelle de 32 bits est découpée selon le schéma $2 + 9 + 9 + 12$ :

```
31  30 29                 21 20                 12 11                      0
+-----+---------------------+---------------------+-------------------------+
|PDPT |   Directory Index   |     Table Index     |      Page Offset        |
|Index|      (9 bits)       |      (9 bits)       |       (12 bits)         |
|(2 b)|    (512 entrées)    |    (512 entrées)    |      (4096 octets)      |
+-----+---------------------+---------------------+-------------------------+
```

- **Bits 31-30 (2 bits) :** Index PDPT ($2^2 = 4$ entrées). Sélectionne le quart de 1 Go de la mémoire virtuelle.
- **Bits 29-21 (9 bits) :** Index dans le Page Directory sélectionné ($2^9 = 512$ entrées).
- **Bits 20-12 (9 bits) :** Index dans la Page Table sélectionnée ($2^9 = 512$ entrées).
- **Bits 11-0  (12 bits) :** Décalage (*Offset*) au sein de la page physique de 4 Ko.

---

### 2.3 Format des Entrées 64 bits (PDPE, PDE, PTE)

Toutes les entrées font désormais **8 octets (`uint64_t`)**.

```
63  62                   52 51                        36 35                 12 11   9 8 7 6 5 4 3 2 1 0
+-+------------------------+----------------------------+---------------------+------+-+-+-+-+-+-+-+-+-+
|X|      Réservé /         |  Adresse Physique Étendue  |   Adresse Physique  | AVL  |G|S|D|A|C|W|U|R|P|
|D|   Disponible pour OS   |        (Bits 51-36)        |     (Bits 35-12)    |      | | | | |D|T|/|/| |
| |        (11 bits)       |        (Doit être 0)       |      (24 bits)      |      | | | | | | |S|W| |
+-+------------------------+----------------------------+---------------------+------+-+-+-+-+-+-+-+-+-+
```

#### Détail des Drapeaux :
* **Bit 0 (`P`) : Present.** 1 = Présent en RAM.
* **Bit 1 (`R/W`) : Read/Write.** 0 = Lecture seule, 1 = Lecture/Écriture. *(Dans le PDPE, ce bit est réservé et doit être 1 si supporté ou ignoré selon l'implémentation Intel)*.
* **Bit 2 (`U/S`) : User/Supervisor.** 0 = Ring 0 uniquement, 1 = Ring 3 accessible.
* **Bit 3 (`PWT`) : Page-level Write-Through.**
* **Bit 4 (`PCD`) : Page-level Cache Disable.** Crucial pour le MMIO matériel.
* **Bit 5 (`A`) : Accessed.** Mis à 1 par le CPU lors d'un accès.
* **Bit 6 (`D`) : Dirty.** *(PTE uniquement)* Mis à 1 par le CPU lors d'une écriture.
* **Bit 7 (`PS`) : Page Size.** *(PDE uniquement)* 0 = Page de 4 Ko (pointe vers une PT), 1 = **Grande Page de 2 Mo**.
* **Bit 8 (`G`) : Global.** *(PTE uniquement)* Empêche l'invalidation dans le TLB lors du rechargement de `CR3`.
* **Bits 9-11 (`AVL`) : Available.** Disponibles pour l'OS.
* **Bits 12-35 : Physical Address Base.** 24 bits d'adresse physique de trame (combinés avec les 12 bits d'offset, cela donne des adresses physiques sur **36 bits = 64 Go**).
* **Bits 36-51 : Extensions futures (jusqu'à 52 bits).** Sur les CPU limités à 36 bits physiques, ces bits doivent obligatoirement être à **0** sous peine de faute `#PF`.
* **Bit 63 (`XD / NX`) : Execute-Disable / No-Execute.** Si activé dans le MSR `EFER`, empêche le fetch d'instructions machine depuis cette page physique !

---

### 2.4 Grandes Pages (Large Pages de 2 Mo)

En mode legacy 32 bits, les grandes pages font 4 Mo.  
**Sous PAE, les grandes pages font 2 Mo ($2^{21}$ octets).**

Lorsque le bit `PS` (bit 7) d'une entrée de Page Directory est mis à 1 :
- Le Page Directory ne pointe plus vers une Page Table, mais directement vers un cadre physique de 2 Mo aligné sur 2 Mo.
- La décomposition de l'adresse virtuelle devient :
  - Bits 31-30 : PDPT Index (2 bits)
  - Bits 29-21 : Page Directory Index (9 bits)
  - Bits 20-0  : Offset dans la page de 2 Mo (21 bits)
- **Avantage majeur :** Économise une page entière de table de pages et réduit drastiquement la pression sur le cache TLB (1 seule entrée TLB couvre 2 Mo entiers au lieu de 512 entrées TLB pour des pages de 4 Ko). Idéal pour mapper le code noyau ou les gros buffers vidéo (VBE Framebuffer).

---

### 2.5 Le Bit NX / XD (Execute-Disable / No-Execute)

Le bit 63 (`XD` chez Intel, `NX` chez AMD) permet d'éradiquer les failles d'exploitation par dépassement de tampon (*buffer overflow*) en interdisant au CPU d'exécuter du code stocké sur la pile (*stack*) ou sur le tas (*heap*).

#### Procédure d'activation du bit NX :
1. **Détection par CPUID :**
   Appeler `CPUID` avec `EAX = 0x80000001`. Le bit 20 du registre `EDX` indique la présence du support NX.
2. **Activation du flag dans le MSR `IA32_EFER` (Extended Feature Enable Register) :**
   Le registre MSR `0xC0000080` possède un bit `NXE` (bit 11).

```c
#define IA32_EFER_MSR   0xC0000080
#define EFER_NXE_BIT    (1 << 11)

void enable_nx_bit(void) {
    uint32_t edx;
    // Vérification CPUID (Extended Function 0x80000001)
    asm volatile ("cpuid" : "=d"(edx) : "a"(0x80000001) : "ebx", "ecx");
    if (edx & (1 << 20)) {
        // Lire le MSR IA32_EFER
        uint32_t low, high;
        asm volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(IA32_EFER_MSR));
        // Activer le bit 11 (NXE)
        low |= EFER_NXE_BIT;
        // Réécrire le MSR
        asm volatile ("wrmsr" :: "a"(low), "d"(high), "c"(IA32_EFER_MSR));
    }
}
```

Dès cet instant, tout bit 63 mis à 1 dans un PDE ou un PTE déclenche une faute de page `#PF` avec le bit d'erreur 4 (`Instruction Fetch Violation`) si le CPU tente d'y sauter.

---

## 3. Impact sur le Gestionnaire de Mémoire Physique (PMM)

L'introduction du PAE bouleverse la conception du PMM (Physical Memory Manager).

### 3.1 Types de Données : Passage au 64 bits (`uint64_t`)

Dans un noyau 32 bits sans PAE, une adresse physique tient dans un `uint32_t` ou `uintptr_t`.  
**Sous PAE, une adresse physique peut valoir `0x100000000` (4 Go) jusqu'à `0xFFFFFFFFF` (64 Go).**  
Le noyau doit impérativement redéfinir ses types de mémoire physique :

```c
typedef uint64_t phys_addr_t;  // 64 bits pour la mémoire physique
typedef uint32_t virt_addr_t;  // 32 bits pour la mémoire virtuelle (toujours 4 Go !)
```

Toutes les signatures de fonctions du PMM doivent être mises à jour :
```c
phys_addr_t pmm_alloc_frame(void);
void        pmm_free_frame(phys_addr_t frame);
```

---

### 3.2 Dimensionnement du Bitmap Allocator pour 64 Go

Voyons l'empreinte mémoire d'une Bitmap pour gérer l'intégralité de 64 Go de RAM physique découpée en cadres de 4 Ko :

$$\text{Nombre total de cadres} = \frac{64 \times 1024 \times 1024 \times 1024}{4096} = 16\,777\,216\text{ cadres}$$
$$\text{Taille de la Bitmap} = \frac{16\,777\,216\text{ bits}}{8\text{ bits/octet}} = 2\,097\,152\text{ octets} = \mathbf{2\text{ Mo}}$$

> **Constat remarquable :** Gérer **64 Go** de mémoire physique ne consomme que **2 Mo de RAM** pour la Bitmap !  
> 2 Mo correspond exactement à la taille d'une seule grande page PAE. Il est donc très aisé de mapper la bitmap d'allocation d'un seul bloc.

---

### 3.3 L'Explosion de la Zone HIGHMEM et la Problématique du Noyau 32 bits

Sur x86 32 bits, l'espace d'adressage virtuel du noyau est limité à $1\text{ Go}$ (de `0xC0000000` à `0xFFFFFFFF`).  
Il est **mathématiquement impossible** pour le noyau de mapper simultanément 64 Go de RAM physique dans un espace virtuel qui n'en fait qu'un seul !

C'est ici que la notion de zones mémoire introduite dans notre fichier [`arch/i386/mm/mmzone.h`](file:///home/gcaptari/Documents/Kernel-From-Scratch/arch/i386/mm/mmzone.h) prend toute son ampleur :

```
0 Go           16 Mo                  896 Mo                             64 Go
+----------------+----------------------+----------------------------------+
|    ZONE_DMA    |     ZONE_NORMAL      |           ZONE_HIGHMEM           |
| (ISA 24 bits)  | (Direct Mapped 1:1)  |   (Inaccessible en mapping direct) |
+----------------+----------------------+----------------------------------+
```

1. **`ZONE_DMA` (0 à 16 Mo) :** Pour les vieux contrôleurs DMA.
2. **`ZONE_NORMAL` (16 Mo à 896 Mo) :** Mappée en permanence dans l'espace noyau (`0xC0000000` à `0xF8000000`). Toute structure noyau critique (code, piles des threads, tables de pages du noyau) doit être allouée dans cette zone.
3. **`ZONE_HIGHMEM` (896 Mo à 64 Go) :**  
   Occupe plus de **98% de la mémoire disponible** !  
   Elle est destinée :
   - Aux données de l'espace utilisateur (mémoire des processus Ring 3).
   - Au cache de pages de disques (*Page Cache*).
   - Le noyau ne peut y accéder directement : il doit utiliser des **fenêtres de mapping temporaires** (voir section 4.2).

---

### 3.4 Le Buddy Allocator sous PAE

Le Buddy Allocator s'adapte remarquablement bien au PAE :
* Les ordres s'étendent de l'ordre 0 (4 Ko) jusqu'à l'ordre 10 (4 Mo) ou l'ordre 12 (16 Mo).
* La recherche de compagnons par `XOR` s'applique sur des variables 64 bits :
  $$\text{Buddy\_Phys} = \text{Addr\_Phys} \oplus (2^{\text{ordre}} \times 4096\text{ULL})$$
* On instancie généralement **une instance du Buddy Allocator par zone** (`ZONE_DMA`, `ZONE_NORMAL`, `ZONE_HIGHMEM`) pour éviter d'épuiser prématurément la précieuse mémoire basse (`ZONE_NORMAL`).

---

## 4. Techniques de Mapping Virtuel Avancées sous PAE

### 4.1 Le Paging Récursif (Fractal Mapping) adapté au PAE

En mode 32 bits legacy (1 niveau de PD), faire pointer l'entrée 1023 vers le PD résolvait tout.  
**Sous PAE, nous avons un PDPT à 4 entrées et 4 Page Directories distincts !**

Pour appliquer le principe récursif sous PAE :
1. Dans le **4ᵉ Page Directory** (PD 3, qui gère le 4ᵉ Go de mémoire virtuelle de `0xC0000000` à `0xFFFFFFFF`), on réserve les **4 dernières entrées** (entrées 508, 509, 510, 511).
2. On configure ces 4 entrées pour qu'elles pointent vers les adresses physiques des **4 Page Directories (PD 0, PD 1, PD 2, PD 3)** avec les droits `Present` et `Writable`.

```
Dernier 2 Mo de l'espace virtuel (0xFFE00000 - 0xFFFFFFFF) :
PD 3, Entrée 508 -> Pointeur physique vers PD 0
PD 3, Entrée 509 -> Pointeur physique vers PD 1
PD 3, Entrée 510 -> Pointeur physique vers PD 2
PD 3, Entrée 511 -> Pointeur physique vers PD 3 (Lui-même)
```

Grâce à cette astuce, l'intégralité des $4 \times 512 = 2048$ Tables de Pages et les 4 Page Directories deviennent manipulables dans les derniers 2 Mo de l'espace virtuel du noyau sans mapping dynamique permanent.

---

### 4.2 Mécanisme de Fenêtrage Temporaire (`kmap` / `kmap_atomic`)

Lorsqu'un noyau 32 bits sous PAE a besoin de lire ou d'écrire dans une page physique située dans la zone `HIGHMEM` (par exemple à l'adresse physique `0x2_0000_0000` = 8 Go) :
1. Le noyau réserve une plage d'adresses virtuelles dédiées (ex: `0xFE000000` à `0xFF000000`, souvent appelée zone `PKMAP` ou `FIXMAP`).
2. Pour manipuler la page physique `P`, il prend une entrée libre de table de page dans cette zone, y inscrit `P | PRESENT | WRITABLE`, et exécute `invlpg`.
3. Il lit/écrit les données via l'adresse virtuelle temporaire retournée.
4. Une fois l'opération terminée, il démonte la page (`kunmap`), remet l'entrée à zéro et invalide le TLB.

Ce mécanisme est au cœur de la conception de Linux 32-bit PAE (`mm/highmem.c`).

---

## 5. Procédure d'Activation Pas à Pas du PAE (Code & Algorithme)

Pour activer le PAE sur x86, l'ordre matériel des opérations est très strict. Toute violation entraîne un Triple Fault immédiat.

### Séquence Chronologique d'Activation :

```
[ Étape 1 : Vérification CPUID ]
  Vérifier EDX bit 6 avec EAX=1 (PAE supporté)
                │
                ▼
[ Étape 2 : Activation optionnelle du bit NX ]
  Vérifier EDX bit 20 avec EAX=0x80000001
  Positionner le bit 11 (NXE) dans le MSR IA32_EFER (0xC0000080)
                │
                ▼
[ Étape 3 : Allocation des Structures Paginées 64 bits ]
  - Allouer 1 PDPT (4 entrées uint64_t, aligné sur 32 octets sous les 4 Go)
  - Allouer 4 Page Directories (512 entrées uint64_t, alignés sur 4 Ko)
  - Allouer les Page Tables nécessaires (512 entrées uint64_t, alignées sur 4 Ko)
  - Lier PDPT[0..3] vers les adresses physiques des 4 PD respectifs (avec bit Present=1)
                │
                ▼
[ Étape 4 : Chargement de CR3 ]
  Charger l'adresse physique du PDPT dans le registre CR3
  (mov %eax, %cr3)
                │
                ▼
[ Étape 5 : Activation du PAE dans CR4 ]
  Mettre le bit 5 (PAE) du registre CR4 à 1
  (bts $5, %cr4)
  ATTENTION : La pagination n'est pas encore active !
                │
                ▼
[ Étape 6 : Activation de la Pagination dans CR0 ]
  Mettre le bit 31 (PG) du registre CR0 à 1
  (or $0x80000000, %cr0)
                │
                ▼
  ==> Le processeur bascule en mode paginé PAE 3 niveaux !
```

### Exemple d'Implémentation en Langage C et Assembleur inline :

```c
#include <stdint.h>
#include <stdbool.h>

// Structures alignées selon les contraintes matérielles du processeur
static uint64_t pdpt[4] __attribute__((aligned(32)));
static uint64_t page_directory_0[512] __attribute__((aligned(4096)));
static uint64_t page_table_0[512] __attribute__((aligned(4096)));

bool check_pae_support(void) {
    uint32_t edx;
    asm volatile ("cpuid" : "=d"(edx) : "a"(1) : "ebx", "ecx");
    return (edx & (1 << 6)) != 0; // Bit 6 = PAE
}

void init_pae_paging(void) {
    if (!check_pae_support()) {
        // CPU non compatible PAE -> Panic
        return;
    }

    // 1. Initialiser le PDPT pour pointer vers le premier Page Directory
    pdpt[0] = (uint64_t)(uintptr_t)page_directory_0 | 0x01; // Present
    pdpt[1] = 0x00;
    pdpt[2] = 0x00;
    pdpt[3] = 0x00;

    // 2. Initialiser le premier Page Directory pour pointer vers la Page Table
    page_directory_0[0] = (uint64_t)(uintptr_t)page_table_0 | 0x03; // Present | R/W

    // 3. Identity mapping des premiers 2 Mo (pages 4 Ko)
    for (uint64_t i = 0; i < 512; i++) {
        // Adresse physique = i * 4096
        page_table_0[i] = (i * 0x1000) | 0x03; // Present | R/W
    }

    // 4. Charger CR3 avec l'adresse physique du PDPT
    asm volatile ("movl %0, %%cr3" :: "r" (&pdpt));

    // 5. Activer le bit PAE (bit 5) dans CR4
    uint32_t cr4;
    asm volatile ("movl %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 5);
    asm volatile ("movl %0, %%cr4" :: "r"(cr4));

    // 6. Activer la pagination (bit 31 PG) dans CR0
    uint32_t cr0;
    asm volatile ("movl %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("movl %0, %%cr0" :: "r"(cr0));
}
```

---

## 6. Tableau Comparatif : x86 Legacy (32 bits) vs x86 PAE (36/64 bits)

| Caractéristique | x86 Legacy (Non-PAE) | x86 PAE (Physical Address Extension) |
|---|---|---|
| **Espace Virtuel par Processus** | 4 Go ($2^{32}$) | 4 Go ($2^{32}$) *(Inchangé)* |
| **Mémoire Physique Maximale** | **4 Go** ($2^{32}$) | **64 Go** ($2^{36}$) à 4 To ($2^{52}$) |
| **Largeur du Bus Physique** | 32 bits | 36 bits (ou plus) |
| **Niveaux de Pagination** | **2 niveaux** (PD -> PT) | **3 niveaux** (PDPT -> PD -> PT) |
| **Registre `CR3` pointe vers** | Page Directory (aligné 4 Ko) | PDPT (4 entrées, aligné 32 octets) |
| **Taille des Entrées (PTE/PDE)** | 32 bits (4 octets) | **64 bits (8 octets)** |
| **Entrées par Table (PD/PT)** | 1024 entrées | 512 entrées |
| **Taille des Grandes Pages** | 4 Mo | **2 Mo** |
| **Sécurité Matérielle (NX/XD)** |  Non disponible |  **Disponible (Bit 63 de chaque entrée)** |
| **Type C pour Adresses Physiques** | `uint32_t` | `uint64_t` (`phys_addr_t`) |
| **Taille Bitmap PMM pour RAM Max**| 128 Ko (pour 4 Go) | 2 Mo (pour 64 Go) |
| **Gestion de la RAM > 896 Mo** | `ZONE_HIGHMEM` limitée (< 3 Go) | `ZONE_HIGHMEM` massive (jusqu'à 63 Go) |

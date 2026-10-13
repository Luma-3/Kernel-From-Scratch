# 🌐 Répertoire des Sources & Ressources Internet : Support PAE (Physical Address Extension) & Pagination 3 Niveaux

> **Projet :** Kernel From Scratch (KFS)  
> **Objet :** Recensement exhaustif et commenté des sources, documentations officielles matérielles, articles de l'OSDev Wiki, implémentations du noyau Linux et cours universitaires dédiés au support du **PAE (Physical Address Extension)** et à la pagination 64 bits sur architecture x86.

---

## Sommaire des Ressources
1. [Spécifications Matérielles Officielles (Intel & AMD)](#1-spécifications-matérielles-officielles-intel--amd)
2. [La Référence OSDev Wiki Spéciale PAE](#2-la-référence-osdev-wiki-spéciale-pae)
3. [Noyau Linux : Implémentation du PAE, HIGHMEM & Code Source](#3-noyau-linux--implémentation-du-pae-highmem--code-source)
4. [Sécurité & Bit NX/XD (Data Execution Prevention)](#4-sécurité--bit-nxxd-data-execution-prevention)
5. [Cours Académiques & Analyses Techniques](#5-cours-académiques--analyses-techniques)
6. [Dépôts Open Source & Projets Démonstrateurs](#6-dépôts-open-source--projets-démonstrateurs)

---

## 1. Spécifications Matérielles Officielles (Intel & AMD)

Les manuels des fabricants décrivent le fonctionnement électronique exact de l'extension d'adresses physiques et du contrôleur MMU.

* **[Intel® 64 and IA-32 Architectures Software Developer’s Manual — Volume 3A: System Programming Guide](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)**
  - **Section 4.1.3 :** *PAE Paging Mode* (Description des modes de pagination IA-32 : 32-bit Paging, PAE Paging, 4-Level Paging, 5-Level Paging).
  - **Section 4.4 :** *PAE Paging* (Le chapitre fondamental sur le PAE) :
    - Format du registre `CR3` sous PAE (bits 31:5 pointant vers le PDPT de 32 octets).
    - Format des 4 entrées du PDPT (Page Directory Pointer Table).
    - Format des entrées PDE (Grandes pages 2 Mo vs tables 4 Ko).
    - Format des entrées PTE (64 bits, adressage physique 36 à 52 bits, bit 63 XD/NX).
    - Règles d'invalidation TLB spécifiques au PAE.
  - *Lien direct d'accès :* [Intel SDM Complete Volumes](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

* **[AMD64 Architecture Programmer’s Manual — Volume 2: System Programming](https://www.amd.com/en/resources/support-modes/developer/amd64-architecture-programmer-manual.html)**
  - **Chapitre 5 :** *Page Translation and Protection* (Sous-section dédiée au mode Legacy PAE).
  - Description de l'activation du bit NX via le registre MSR `EFER` (Extended Feature Enable Register, MSR `0xC0000080`, bit 11 `NXE`).

---

## 2. La Référence OSDev Wiki Spéciale PAE

Guides pratiques et pièges d'implémentation documentés par la communauté des développeurs de noyaux.

* **[OSDev Wiki — Setting Up Paging With PAE](https://wiki.osdev.org/Setting_Up_Paging_With_PAE)**
  - *Le tutoriel pratique de référence pour basculer un OS x86 en PAE.*
  - Déclaration des structures alignées en C (`uint64_t pdpt[4] __attribute__((aligned(32)))`, `page_dir[512] __attribute__((aligned(4096)))`).
  - Séquence d'activation en assembleur : modification de `CR4.PAE` (bit 5), chargement de `CR3`, puis `CR0.PG` (bit 31).
  - Gestion du mapping initial 2 Mo via le bit `PS` (Page Size).

* **[OSDev Wiki — X86 Paging](https://wiki.osdev.org/X86_Paging)**
  - Section comparative entre la pagination 32 bits standard (2 niveaux) et la pagination PAE (3 niveaux).
  - Schéma explicatif de la décomposition de l'adresse virtuelle $2 + 9 + 9 + 12$.

* **[OSDev Wiki — Fractal Page Mapping (Recursive Paging & Multi-Level)](https://wiki.osdev.org/Fractal_Page_Mapping)**
  - Discussion détaillée sur l'adaptation du paging récursif aux architectures avec plus de 2 niveaux (PAE et x86_64 Long Mode).
  - Techniques pour réserver des entrées dans le dernier Page Directory afin d'accéder aux 4 répertoires et à toutes les tables de pages.

* **[OSDev Wiki — NX Bit (No-Execute)](https://wiki.osdev.org/NX_Bit)**
  - Comment vérifier la présence du bit NX via `CPUID.80000001H:EDX[bit 20]`.
  - Comment activer le bit dans le registre `IA32_EFER` et l'utiliser dans les entrées PTE (bit 63).

* **[OSDev Wiki — Page Fault](https://wiki.osdev.org/Page_Fault)**
  - Décodage des erreurs `#PF` spécifiques au PAE :
    - Bit d'erreur 3 (RSVD) : se déclenche si un bit réservé (comme les bits 36 à 51 sur un CPU 36 bits) est mis à 1.
    - Bit d'erreur 4 (I/D) : tentative d'exécution d'une instruction sur une page dont le bit 63 (XD/NX) est actif.

---

## 3. Noyau Linux : Implémentation du PAE, HIGHMEM & Code Source

Linux a soutenu l'effort industriel massif de passage au PAE dans les serveurs d'entreprise (option `CONFIG_X86_PAE`).

* **[Understanding the Linux Virtual Memory Manager — Mel Gorman (Livre Libre Complet)](https://www.kernel.org/doc/gorman/html/understand/)**
  - **Chapitre 2 : Describing Physical Memory :**
    - [Zone HighMem & PAE Architecture](https://www.kernel.org/doc/gorman/html/understand/understand005.html) : Explication magistrale de la gestion de 64 Go de RAM dans un noyau 32 bits dont l'espace virtuel est restreint à 1 Go.
  - **Chapitre 3 : Page Table Management :**
    - [Three-Level Page Tables for PAE](https://www.kernel.org/doc/gorman/html/understand/understand006.html) : Comment Linux abstrait les 3 niveaux (PGD/PDPT, PMD, PTE) avec des entrées de type `pteval_t` en 64 bits.

* **[Documentation Officielle du Noyau Linux — High Memory Handling](https://docs.kernel.org/mm/highmem.html)**
  - Explication des mécanismes de fenêtres temporaires :
    - `kmap(struct page *page)` : mapping persistant dans l'espace virtuel du noyau.
    - `kmap_atomic(struct page *page)` : mapping ultra-rapide par CPU pour les sections critiques et interruptions.

* **Code Source Linux Officiel (GitHub torvalds/linux) :**
  - **[arch/x86/include/asm/pgtable-3level.h](https://github.com/torvalds/linux/blob/master/arch/x86/include/asm/pgtable-3level.h)** : Définition des macros et opérations 64 bits sur les tables de pages PAE 3 niveaux.
  - **[arch/x86/include/asm/pgtable-3level_types.h](https://github.com/torvalds/linux/blob/master/arch/x86/include/asm/pgtable-3level_types.h)** : Types `typedef uint64_t pteval_t;`, `typedef uint64_t pmdval_t;`, `typedef uint64_t pgdval_t;`.
  - **[mm/highmem.c](https://github.com/torvalds/linux/blob/master/mm/highmem.c)** : Algorithme de gestion du pool de pages physiques situées au-delà de 896 Mo.

---

## 4. Sécurité & Bit NX/XD (Data Execution Prevention)

Le PAE est intimement lié à la sécurité logicielle moderne et à la mitigation des exploits.

* **[Microsoft Technical Documentation — Data Execution Prevention (DEP) & PAE](https://learn.microsoft.com/en-us/windows/win32/memory/data-execution-prevention)**
  - Présentation de la façon dont Windows utilise le mode PAE pour forcer l'isolation matériel W^X (*Write XOR Execute*) et empêcher l'exécution de shellcodes sur la pile.
* **[Phrack Magazine — Advances in Memory Corruption Mitigation & Hardware NX](http://phrack.org/)**
  - Articles d'analyse sur l'impact de l'activation du bit NX matériel via PAE et les techniques de contournement historiques (Return-to-libc, ROP).

---

## 5. Cours Académiques & Analyses Techniques

* **[MIT 6.828 / 6.S081: Operating System Engineering](https://pdos.csail.mit.edu/6.828/)**
  - Conférences et supports de cours sur la transition de la mémoire x86 vers les tables de pages multi-niveaux (passage de 2 à 3 puis 4 niveaux).
* **[Operating Systems: Three Easy Pieces (OSTEP) — Remzi H. Arpaci-Dusseau](https://pages.cs.wisc.edu/~remzi/OSTEP/)**
  - **[Chapitre 20 : Paging: Smaller Tables (PDF)](https://pages.cs.wisc.edu/~remzi/OSTEP/vm-smalltables.pdf)** : Analyse théorique des arbres de pagination multi-niveaux, coûts de traversée (*Page Walk*) et compromis d'empreinte mémoire.
* **[Harvard CS161: Operating Systems — Lecture Notes on Memory Management](https://www.eecs.harvard.edu/~margo/cs161/)**
  - Discussion sur le surcoût de performance lié à la pagination à 3 niveaux (hausse potentielle des latences TLB miss de 50%).

---

## 6. Dépôts Open Source & Projets Démonstrateurs

Projets réels implémentant le mode PAE pour étudier leur code source :

* **[ToaruOS (GitHub)](https://github.com/klange/toaruos)**
  - Système d'exploitation éducatif complet. Examiner la gestion des tables de pages et du PMM compatible avec de larges espaces mémoire.
* **[Limine Bootloader (GitHub)](https://github.com/limine-bootloader/limine)**
  - Le bootloader moderne Limine configure nativement des tables de pages x86 avec support PAE et Long Mode avant de passer la main au noyau.
* **[Bona Fide OS Development Tutorials — Paging and PAE](http://www.osdever.net/)**
  - Archives d'articles et d'exemples de code en assembleur NASM/GAS pour initialiser le mode PAE dès le secteur de boot.

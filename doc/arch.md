 <!-- * @page architecture Architecture du système -->
 <!-- * -->
 <!-- * Description globale de l'architecture clavier / terminal / rendu. -->
 <!-- * -->
 <!-- * @section input Pipeline d'entrée clavier et terminal -->
 <!-- * -->
 <!-- * @plantumlfile archi_keyboard_term.puml -->
 <!-- * -->
 <!-- * Le clavier est géré par un circular buffer via push et pop. Le terminal lit -->
 <!-- * ce buffer pour afficher les caractères. -->

# Architecture du système

Description globale de l'architecture clavier / terminal / rendu.

## Pipeline d'entrée clavier et terminal

@startuml
!include archi_keyboard_term.puml
@enduml


Le clavier est géré par un circular buffer via push et pop. Le terminal lit ce buffer pour afficher les caractères.

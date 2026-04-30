# Résumé : Synchronisation immédiate du RA Hash dans la Game List

## Objectif
Corriger le décalage entre la valeur du RA hash modifiée dans les propriétés et l'affichage de la colonne Achievements dans la game list.

Le but est que la game list soit mise à jour immédiatement après :
- édition du RA hash,
- restauration/suppression du RA hash custom,
- saisie d'un hash identique au hash original (ne doit pas être considéré comme custom).

## Modifications implémentées

### 1) Recalcul immédiat des données achievements lors des rescans ciblés
Dans [GameList::RescanCustomAttributesForPath](../../src/core/game_list.cpp#L727):
- ajout du chargement de [Achievements::ProgressDatabase](../../src/core/game_list.cpp#L758) via [GameList::LoadAchievementsProgressDatabase](../../src/core/game_list.cpp#L209), avec la même logique que [GameList::Refresh](../../src/core/game_list.cpp#L1150),
- appel de [GameList::PopulateEntryAchievements](../../src/core/game_list.cpp#L904) pour les entrées [EntryType::Disc](../../src/core/game_list.h#L24).

Effet:
- après suppression/restauration d'un custom hash, les compteurs ([achievements_game_id](../../src/core/game_list.h#L57), [num_achievements](../../src/core/game_list.h#L58), [unlocked_achievements](../../src/core/game_list.h#L59), [unlocked_achievements_hc](../../src/core/game_list.h#L60)) ne restent plus à zéro par défaut.
- la game list n'a plus besoin d'un rescan global ou d'un redémarrage pour revenir à un état cohérent.

### 2) Recalcul immédiat lors de la sauvegarde d'un custom hash
Dans [GameList::SaveCustomAchievementsHashForPath](../../src/core/game_list.cpp#L2156) (branche hash non vide) :
- validation/parsing du hash,
- reset explicite des champs achievements dérivés sur l'entrée ([achievements_game_id](../../src/core/game_list.cpp#L2234), [num_achievements](../../src/core/game_list.cpp#L2235), [unlocked_achievements](../../src/core/game_list.cpp#L2236), [unlocked_achievements_hc](../../src/core/game_list.cpp#L2237)),
- recalcul immédiat via [GameList::PopulateEntryAchievements](../../src/core/game_list.cpp#L2239),
- notification UI ensuite via [GameList::NotifyHostOfEntryChange](../../src/core/game_list.cpp#L2240).

Effet:
- la colonne Achievements est synchronisée tout de suite après édition du hash.
- les anciennes valeurs ne sont plus conservées quand le nouveau hash ne matche rien.

### 3) "Same as original" => pas de custom hash
Dans [GameList::SaveCustomAchievementsHashForPath](../../src/core/game_list.cpp#L2156) :
- si le hash saisi est identique au hash original, l'override est retiré (suppression de [AchievementsHash](../../src/core/game_list.cpp#L2214)),
- ce comportement fonctionne aussi quand un custom hash existait déjà (comparaison avec le hash média d'origine via [GameList::PopulateEntryFromPath](../../src/core/game_list.cpp#L2205)),
- un rescan ciblé est déclenché si nécessaire via [GameList::RescanCustomAttributesForPath](../../src/core/game_list.cpp#L2218) pour restaurer immédiatement l'état non custom.

Effet:
- remplacer un hash par sa valeur d'origine ne laisse plus [has_custom_achievements_hash](../../src/core/game_list.h#L43) à `true`,
- le bouton Restore ne reste pas activé à tort.

## Portee et compatibilite
- Aucun changement de pipeline réseau/runtime RetroAchievements.
- Aucun changement d'API publique.
- Aucun changement de format cache/fichier.
- Changements limités à [game_list.cpp](../../src/core/game_list.cpp).

## Validation
Les cas validés manuellement :
1. Hash custom valide vers jeu reconnu : compteur mis à jour immédiatement.
2. Hash custom valide sans match : compteur remis à un état cohérent immédiatement.
3. Restore/suppression du hash custom : retour immédiat à l'état d'origine.
4. Saisie du hash original : non considéré comme custom.
5. Cas "custom -> hash original" : non considéré comme custom après mise à jour.

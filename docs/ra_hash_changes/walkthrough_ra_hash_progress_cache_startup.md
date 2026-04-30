# Résumé : chargement fiable du cache de progression RA au démarrage

## Contexte
Après le merge avec `upstream/master`, les compteurs d'achievements de la game list pouvaient afficher `0` pour un jeu utilisant un RA hash custom juste après le démarrage de DuckStation.

Le hash custom était bien conservé et bien appliqué, mais les compteurs d'achievements débloqués ne revenaient qu'après une action manuelle qui forçait un recalcul local, par exemple restaurer puis remettre le même RA hash custom.

## Cause
La game list utilise le cache local de progression RetroAchievements, stocké côté cache sous le nom `achievement_progress.cache`, pour remplir les compteurs `unlocked` et `unlocked_hc`.

Pendant l'adaptation du merge upstream, le chargement de ce cache avait été conditionné à [Achievements::HasSavedCredentials](../../src/core/achievements.cpp#L704). Cette information dépend de l'état runtime du module RetroAchievements.

Au démarrage, la game list peut être rafraîchie avant que [Achievements::Initialize](../../src/core/achievements.cpp#L594) ait fini de remplir cet état runtime. Dans ce cas, le cache de progression n'était pas chargé pendant le scan initial, même si les identifiants RA étaient déjà présents dans les settings.

## Correction
Dans [game_list.cpp](../../src/core/game_list.cpp), ajout d'un helper local [GameList::ShouldLoadAchievementsProgressDatabase](../../src/core/game_list.cpp#L196) qui autorise le chargement du cache de progression si :
- [Achievements::HasSavedCredentials](../../src/core/achievements.cpp#L704) indique déjà que les credentials sont connus côté runtime,
- ou si les settings contiennent déjà `Cheevos/Username` et `Cheevos/Token`, même si le module RA n'a pas encore fini son initialisation.

Ajout aussi de [GameList::LoadAchievementsProgressDatabase](../../src/core/game_list.cpp#L208) pour centraliser le chargement de [Achievements::ProgressDatabase](../../src/core/achievements.h#L43) et le warning en cas d'échec.

Ce helper est utilisé dans les chemins qui recalculent les compteurs de la game list :
- [GameList::Refresh](../../src/core/game_list.cpp#L1116), pour le scan initial/global,
- [GameList::RescanCustomAttributesForPath](../../src/core/game_list.cpp#L717), pour les rescans ciblés,
- [GameList::UpdateAllAchievementData](../../src/core/game_list.cpp#L948), pour les updates déclenchées par RA,
- [GameList::SaveCustomAchievementsHashForPath](../../src/core/game_list.cpp#L2110), pour l'édition immédiate du RA hash.

## Résultat attendu
Au redémarrage de DuckStation, un jeu avec un RA hash custom doit retrouver immédiatement ses compteurs d'achievements débloqués depuis le cache local, sans devoir restaurer/remettre le hash custom ni redébloquer un achievement.

## Validation
Le build a été relancé avec `build.bat` après la modification.

Résultat :
- build réussi,
- `0` erreur,
- `1` warning linker connu : `found both wWinMain and WinMain; using latter`.

# Résumé : RA Hash en lecture seule quand le jeu est en cours d'exécution

## Objectif
Empêcher la modification du champ RA hash dans la fenêtre des propriétés quand le jeu correspondant est actuellement en cours d'exécution, avec un comportement `same-path` et une mise à jour `live`.

Concrètement :
- si la fiche ouverte correspond au jeu en cours, le champ devient non éditable et le bouton restore est désactivé,
- si ce n'est pas le même jeu (ou aucun jeu en cours), le comportement normal est conservé.

## Solution implémentée

### 1) Suivi local du jeu en cours dans le widget
Dans [GameSummaryWidget::GameSummaryWidget](../../src/duckstation-qt/gamesummarywidget.cpp#L37):
- initialisation du chemin du jeu en cours via [Host::RunOnCoreThread](../../src/duckstation-qt/gamesummarywidget.cpp#L43) avec [System::IsValid](../../src/duckstation-qt/gamesummarywidget.cpp#L44) et [System::GetGamePath](../../src/duckstation-qt/gamesummarywidget.cpp#L44),
- stockage dans [m_current_running_game_path](../../src/duckstation-qt/gamesummarywidget.h#L63),
- abonnement au signal [CoreThread::systemGameChanged](../../src/duckstation-qt/gamesummarywidget.cpp#L118) pour mettre à jour l'état en direct.

Effet:
- le widget connaît à tout moment quel jeu tourne, sans changement de pipeline core/RA.

### 2) Centralisation de la règle read-only
Ajout de [GameSummaryWidget::updateAchievementsHashReadOnlyState](../../src/duckstation-qt/gamesummarywidget.cpp#L524), déclarée dans [gamesummarywidget.h](../../src/duckstation-qt/gamesummarywidget.h#L39).

Cette méthode utilise :
- [m_achievements_hash_is_disc_entry](../../src/duckstation-qt/gamesummarywidget.h#L64),
- [m_current_running_game_path](../../src/duckstation-qt/gamesummarywidget.h#L63),
- [m_achievements_hash_has_custom_override](../../src/duckstation-qt/gamesummarywidget.h#L65),

pour appliquer une règle unique :
- champ [achievementsHash](../../src/duckstation-qt/gamesummarywidget.cpp#L528) en read-only si l'entrée n'est pas disque/discset ou si `m_path == m_current_running_game_path`,
- bouton [restoreAchievementsHash](../../src/duckstation-qt/gamesummarywidget.cpp#L529) activé uniquement quand l'édition est permise et qu'un override custom existe.

Effet:
- plus de logique contradictoire entre différentes zones du code UI.

### 3) Points d'appel live
La méthode de centralisation est appelée :
- après le remplissage initial dans [GameSummaryWidget::populateUi](../../src/duckstation-qt/gamesummarywidget.cpp#L156) (appel à [L180](../../src/duckstation-qt/gamesummarywidget.cpp#L180)),
- après sauvegarde/restauration dans [GameSummaryWidget::setCustomAchievementsHash](../../src/duckstation-qt/gamesummarywidget.cpp#L478) (appel à [L519](../../src/duckstation-qt/gamesummarywidget.cpp#L519)),
- à chaque changement du jeu en cours via [systemGameChanged](../../src/duckstation-qt/gamesummarywidget.cpp#L118) (appel à [L121](../../src/duckstation-qt/gamesummarywidget.cpp#L121)).

Effet:
- verrouillage/déverrouillage instantané sans fermer la fenêtre.

## Portée et compatibilité
- Changements limités à [gamesummarywidget.cpp](../../src/duckstation-qt/gamesummarywidget.cpp) et [gamesummarywidget.h](../../src/duckstation-qt/gamesummarywidget.h).
- Aucun changement d'API publique côté core.
- Aucun changement du pipeline RA runtime (identify/load/callbacks).

## Validation ciblée
Les comportements vérifiés pour ce scope :
1. Propriétés du jeu en cours : RA hash read-only + restore désactivé.
2. Propriétés d'un autre jeu : champ éditable selon la logique existante.
3. Démarrage/arrêt/changement de jeu pendant la fenêtre ouverte : bascule read-only live.
4. Non-disc/non-discset : reste read-only comme attendu.

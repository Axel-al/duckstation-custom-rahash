// SPDX-FileCopyrightText: 2019-2025 Connor McLaughlin <stenzek@gmail.com>
// SPDX-License-Identifier: CC-BY-NC-ND-4.0

#pragma once

#include "util/cd_image_hasher.h"

#include "common/types.h"

#include <QtWidgets/QWidget>

#include "ui_gamesummarywidget.h"

enum class DiscRegion : u8;

namespace GameList {
struct Entry;
}

class ProgressCallback;
class SettingsWindow;

class GameSummaryWidget : public QWidget
{
  Q_OBJECT

public:
  GameSummaryWidget(const GameList::Entry* entry, SettingsWindow* dialog, QWidget* parent);
  ~GameSummaryWidget();

  void reloadGameSettings();

private:
  void populateUi(const GameList::Entry* entry);
  void setCustomTitle(const std::string& text);
  void setCustomDiscSetTitle(const std::string& text);
  void setCustomRegion(int region);
  void setCustomAchievementsHash(const std::string& text);
  void updateAchievementsHashReadOnlyState();

  void populateTracksInfo();
  void disableWidgetsForRuntimeScannedEntry();

  void onSeparateDiscSettingsChanged(Qt::CheckState state);
  void onChangeSerialClicked();
  void onCustomLanguageChanged(int language);
  void onCompatibilityCommentsClicked();
  void onInputProfileChanged(int index);
  void onEditInputProfileClicked();
  void onComputeHashClicked();

  bool computeImageHash(const std::string& path, CDImageHasher::TrackHashes& track_hashes,
                        ProgressCallback* const progress, Error* const error) const;
  void processHashResults(const CDImageHasher::TrackHashes& track_hashes, bool result, bool cancelled,
                          const Error& error);

  Ui::GameSummaryWidget m_ui;
  SettingsWindow* m_dialog;

  std::string m_path;
  std::string m_redump_search_keyword;
  std::string m_original_achievements_hash;
  std::string m_current_running_game_path;
  bool m_achievements_hash_is_disc_entry = false;
  bool m_achievements_hash_has_custom_override = false;
  QString m_compatibility_comments;
};

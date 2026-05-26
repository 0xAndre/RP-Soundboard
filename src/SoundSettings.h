// src/SoundSettings.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once

#include <QDialog>
#include <QCloseEvent>
#include <QIcon>
#include <QTimer>
#include <QProcess>
#include "SoundInfo.h"

class ConfigModel;
class SoundView;

namespace Ui
{
class SoundSettingsQt;
}


class SoundSettingsQt : public QDialog
{
	Q_OBJECT

  public:
	explicit SoundSettingsQt(const SoundInfo& soundInfo, size_t buttonId, QWidget* parent = 0);
	~SoundSettingsQt();
	const SoundInfo& getSoundInfo() const
	{
		return m_soundInfo;
	}
	void focusYoutubeUrl();

  protected:
	void done(int r);

  private slots:
	void onVolumeChanged(int value);
	void onBrowsePressed();
	void onPreviewPressed();
	void onTimer();
	void onHotkeyChangePressed();
	void updateHotkeyText();
	void onColorEnabledPressed();
	void onChooseColorPressed();
	void updateSoundView();
	void onYoutubeUrlEditingFinished();
	void onYoutubeTitleFetched(int exitCode, QProcess::ExitStatus status);

  private:
	void initGui(const SoundInfo& sound);
	void fillFromGui(SoundInfo& sound);

  private:
	Ui::SoundSettingsQt* ui;
	SoundInfo m_soundInfo;
	size_t m_buttonId;
	QIcon m_iconPlay;
	QIcon m_iconStop;
	QTimer* m_timer;
	SoundView* m_soundview;
	QColor customColor;
	QProcess* m_ytdlpTitleProcess = nullptr;
};

/*
 *  Copyright (C) 2011-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2011 Pulse-Eight (http://www.pulse-eight.com/)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <vector>
#include "p8-platform/os.h"
#include "client.h"

class TiXmlNode;

struct PVRDemoEpgEntry
{
  int         iBroadcastId;
  std::string strTitle;
  int         iChannelId;
  time_t      startTime;
  time_t      endTime;
  std::string strPlotOutline;
  std::string strPlot;
  std::string strIconPath;
  int         iGenreType;
  int         iGenreSubType;
//  time_t      firstAired;
//  int         iParentalRating;
//  int         iStarRating;
  int         iSeriesNumber;
  int         iEpisodeNumber;
//  int         iEpisodePartNumber;
  std::string strEpisodeName;
};

struct PVRDemoChannel
{
  bool                    bRadio;
  int                     iUniqueId;
  int                     iChannelNumber;
  int                     iSubChannelNumber;
  int                     iEncryptionSystem;
  std::string             strChannelName;
  std::string             strIconPath;
  std::string             strStreamURL;
  std::vector<PVRDemoEpgEntry> epg;
};

struct PVRDemoRecording
{
  bool        bRadio;
  int         iDuration;
  int         iGenreType;
  int         iGenreSubType;
  int         iSeriesNumber;
  int         iEpisodeNumber;
  std::string strChannelName;
  std::string strPlotOutline;
  std::string strPlot;
  std::string strRecordingId;
  std::string strStreamURL;
  std::string strTitle;
  std::string strEpisodeName;
  std::string strDirectory;
  time_t      recordingTime;
};

struct PVRDemoTimer
{
  int             iChannelId;
  time_t          startTime;
  time_t          endTime;
  PVR_TIMER_STATE state;
  std::string     strTitle;
  std::string     strSummary;
};

struct PVRDemoChannelGroup
{
  bool             bRadio;
  int              iGroupId;
  std::string      strGroupName;
  int              iPosition;
  std::vector<int> members;
};

class PVRDemoData
{
public:
  PVRDemoData(void);
  virtual ~PVRDemoData(void);

  int GetChannelsAmount(void);
  PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio);
  bool GetChannel(const PVR_CHANNEL &channel, PVRDemoChannel &myChannel);

  int GetChannelGroupsAmount(void);
  PVR_ERROR GetChannelGroups(ADDON_HANDLE handle, bool bRadio);
  PVR_ERROR GetChannelGroupMembers(ADDON_HANDLE handle, const PVR_CHANNEL_GROUP &group);

  PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, int iChannelUid, time_t iStart, time_t iEnd);

  int GetRecordingsAmount(bool bDeleted);
  PVR_ERROR GetRecordings(ADDON_HANDLE handle, bool bDeleted);
  std::string GetRecordingURL(const PVR_RECORDING &recording);

  int GetTimersAmount(void);
  PVR_ERROR GetTimers(ADDON_HANDLE handle);

  std::string GetSettingsFile() const;
protected:
  bool LoadDemoData(void);
private:
  bool ScanXMLChannelData(const TiXmlNode* pChannelNode, int iUniqueChannelId, PVRDemoChannel& channel);
  bool ScanXMLChannelGroupData(const TiXmlNode* pGroupNode, int iUniqueGroupId, PVRDemoChannelGroup& group);
  bool ScanXMLEpgData(const TiXmlNode* pEpgNode);
  bool ScanXMLRecordingData(const TiXmlNode* pRecordingNode, int iUniqueGroupId, PVRDemoRecording& recording);
  bool ScanXMLTimerData(const TiXmlNode* pTimerNode, PVRDemoTimer& timer);

  std::vector<PVRDemoChannelGroup> m_groups;
  std::vector<PVRDemoChannel>      m_channels;
  std::vector<PVRDemoRecording>    m_recordings;
  std::vector<PVRDemoRecording>    m_recordingsDeleted;
  std::vector<PVRDemoTimer>        m_timers;
  time_t                           m_iEpgStart;
  std::string                      m_strDefaultIcon;
  std::string                      m_strDefaultMovie;
};

/*
 *  Copyright (C) 2011-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2011 Pulse-Eight (http://www.pulse-eight.com/)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "PVRDemo.h"

#include <kodi/General.h>
#include "util/XMLUtils.h"

#ifdef TARGET_WINDOWS
#define snprintf _snprintf
#endif

#include "p8-platform/os.h"
#include "p8-platform/util/StringUtils.h"


/***********************************************************
 * PVR Client AddOn specific public library functions
 ***********************************************************/

CPVRDemo::CPVRDemo()
{
  m_iEpgStart = -1;
  m_strDefaultIcon = "http://www.royalty-free.tv/news/wp-content/uploads/2011/06/cc-logo1.jpg";
  m_strDefaultMovie = "";

  LoadDemoData();

  AddMenuHook(kodi::addon::PVRMenuhook(1, 30000, PVR_MENUHOOK_SETTING));
  AddMenuHook(kodi::addon::PVRMenuhook(2, 30001, PVR_MENUHOOK_ALL));
  AddMenuHook(kodi::addon::PVRMenuhook(3, 30002, PVR_MENUHOOK_CHANNEL));
}

CPVRDemo::~CPVRDemo()
{
  m_channels.clear();
  m_groups.clear();
}

PVR_ERROR CPVRDemo::GetCapabilities(kodi::addon::PVRCapabilities& capabilities)
{
  capabilities.SetSupportsEPG(true);
  capabilities.SetSupportsTV(true);
  capabilities.SetSupportsRadio(true);
  capabilities.SetSupportsChannelGroups(true);
  capabilities.SetSupportsRecordings(true);
  capabilities.SetSupportsRecordingsUndelete(true);
  capabilities.SetSupportsTimers(true);
  capabilities.SetSupportsRecordingsRename(false);
  capabilities.SetSupportsRecordingsLifetimeChange(false);
  capabilities.SetSupportsDescrambleInfo(false);

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetBackendName(std::string& name)
{
  name = "pulse-eight demo pvr add-on";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetBackendVersion(std::string& version)
{
  version = "0.1";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetConnectionString(std::string& connection)
{
  connection = "connected";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetBackendHostname(std::string&	hostname)
{
  hostname = "";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetDriveSpace(uint64_t& total, uint64_t &used)
{
  total = 1024 * 1024 * 1024;
  used  = 0;
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetEPGForChannel(int channelUid, time_t start, time_t end, kodi::addon::PVREPGTagsResultSet& results)
{
  if (m_iEpgStart == -1)
    m_iEpgStart = start;

  time_t iLastEndTime = m_iEpgStart + 1;
  int iAddBroadcastId = 0;

  for (auto& myChannel : m_channels)
  {
    if (myChannel.iUniqueId != channelUid)
      continue;

    while (iLastEndTime < end && myChannel.epg.size() > 0)
    {
      time_t iLastEndTimeTmp = 0;
      for (unsigned int iEntryPtr = 0; iEntryPtr < myChannel.epg.size(); iEntryPtr++)
      {
        PVRDemoEpgEntry &myTag = myChannel.epg.at(iEntryPtr);

        kodi::addon::PVREPGTag tag;
        tag.SetUniqueBroadcastId(myTag.iBroadcastId + iAddBroadcastId);
        tag.SetUniqueChannelId(channelUid);
        tag.SetTitle(myTag.strTitle);
        tag.SetStartTime(myTag.startTime + iLastEndTime);
        tag.SetEndTime(myTag.endTime + iLastEndTime);
        tag.SetPlotOutline(myTag.strPlotOutline);
        tag.SetPlot(myTag.strPlot);
        tag.SetIconPath(myTag.strIconPath);
        tag.SetGenreType(myTag.iGenreType);
        tag.SetGenreSubType(myTag.iGenreSubType);
        tag.SetFlags(EPG_TAG_FLAG_UNDEFINED);
        tag.SetSeriesNumber(myTag.iSeriesNumber);
        tag.SetEpisodeNumber(myTag.iEpisodeNumber);
        tag.SetEpisodeName(myTag.strEpisodeName);

        iLastEndTimeTmp = tag.GetEndTime();

        results.Add(tag);
      }

      iLastEndTime = iLastEndTimeTmp;
      iAddBroadcastId += myChannel.epg.size();
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::IsEPGTagPlayable(const kodi::addon::PVREPGTag&, bool& bIsPlayable)
{
  bIsPlayable = true;
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetEPGTagStreamProperties(const kodi::addon::PVREPGTag& tag, std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  properties.emplace_back(PVR_STREAM_PROPERTY_STREAMURL, "http://distribution.bbb3d.renderfarming.net/video/mp4/bbb_sunflower_1080p_30fps_normal.mp4");
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelsAmount(int& amount)
{
  amount = m_channels.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannels(bool bRadio, kodi::addon::PVRChannelsResultSet& results)
{
  for (const auto& channel : m_channels)
  {
    if (channel.bRadio == bRadio)
    {
      kodi::addon::PVRChannel kodiChannel;

      kodiChannel.SetUniqueId(channel.iUniqueId);
      kodiChannel.SetIsRadio(channel.bRadio);
      kodiChannel.SetChannelNumber(channel.iChannelNumber);
      kodiChannel.SetSubChannelNumber(channel.iSubChannelNumber);
      kodiChannel.SetChannelName(channel.strChannelName);
      kodiChannel.SetEncryptionSystem(channel.iEncryptionSystem);
      kodiChannel.SetIconPath(channel.strIconPath);
      kodiChannel.SetIsHidden(false);

      results.Add(kodiChannel);
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelStreamProperties(const kodi::addon::PVRChannel& channel, std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  PVRDemoChannel addonChannel;
  GetChannel(channel, addonChannel);

  properties.emplace_back(PVR_STREAM_PROPERTY_STREAMURL, addonChannel.strStreamURL);
  properties.emplace_back(PVR_STREAM_PROPERTY_ISREALTIMESTREAM, "true");
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelGroupsAmount(int& amount)
{
  amount = m_groups.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelGroups(bool bRadio, kodi::addon::PVRChannelGroupsResultSet& results)
{
  for (const auto& group : m_groups)
  {
    if (group.bRadio == bRadio)
    {
      kodi::addon::PVRChannelGroup kodiGroup;

      kodiGroup.SetIsRadio(bRadio);
      kodiGroup.SetPosition(group.iPosition);
      kodiGroup.SetGroupName(group.strGroupName);
      results.Add(kodiGroup);
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group, kodi::addon::PVRChannelGroupMembersResultSet& results)
{
  for (const auto& myGroup : m_groups)
  {
    if (myGroup.strGroupName == group.GetGroupName())
    {
      for (const auto& iId : myGroup.members)
      {
        if (iId < 0 || iId > (int)m_channels.size() - 1)
          continue;

        PVRDemoChannel &channel = m_channels.at(iId);
        kodi::addon::PVRChannelGroupMember kodiGroupMember;
        kodiGroupMember.SetGroupName(group.GetGroupName());
        kodiGroupMember.SetChannelUniqueId(channel.iUniqueId);
        kodiGroupMember.SetChannelNumber(channel.iChannelNumber);
        kodiGroupMember.SetSubChannelNumber(channel.iSubChannelNumber);

        results.Add(kodiGroupMember);
      }
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetSignalStatus(int channelUid, kodi::addon::PVRSignalStatus &signalStatus)
{
  signalStatus.SetAdapterName("pvr demo adapter 1");
  signalStatus.SetAdapterStatus("OK");

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetRecordingsAmount(bool deleted, int& amount)
{
  amount = deleted ? m_recordingsDeleted.size() : m_recordings.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results)
{
  for (const auto& recording : deleted ? m_recordingsDeleted : m_recordings)
  {
    kodi::addon::PVRRecording kodiRecording;

    kodiRecording.SetDuration(recording.iDuration);
    kodiRecording.SetGenreType(recording.iGenreType);
    kodiRecording.SetGenreSubType(recording.iGenreSubType);
    kodiRecording.SetRecordingTime(recording.recordingTime);
    kodiRecording.SetEpisodeNumber(recording.iEpisodeNumber);
    kodiRecording.SetSeriesNumber(recording.iSeriesNumber);
    kodiRecording.SetIsDeleted(deleted);
    kodiRecording.SetChannelType(recording.bRadio ? PVR_RECORDING_CHANNEL_TYPE_RADIO : PVR_RECORDING_CHANNEL_TYPE_TV);
    kodiRecording.SetChannelName(recording.strChannelName);
    kodiRecording.SetPlotOutline(recording.strPlotOutline);
    kodiRecording.SetPlot(recording.strPlot);
    kodiRecording.SetRecordingId(recording.strRecordingId);
    kodiRecording.SetTitle(recording.strTitle);
    kodiRecording.SetEpisodeName(recording.strEpisodeName);
    kodiRecording.SetDirectory(recording.strDirectory);

    /* TODO: PVR API 5.0.0: Implement this */
    kodiRecording.SetChannelUid(PVR_CHANNEL_INVALID_UID);

    results.Add(kodiRecording);
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetRecordingStreamProperties(const kodi::addon::PVRRecording& recording, std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  properties.emplace_back(PVR_STREAM_PROPERTY_STREAMURL, GetRecordingURL(recording));
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetTimerTypes(std::vector<kodi::addon::PVRTimerType>& types)
{
  /* TODO: Implement this to get support for the timer features introduced with PVR API 1.9.7 */
  return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR CPVRDemo::GetTimersAmount(int& amount)
{
  amount = m_timers.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetTimers(kodi::addon::PVRTimersResultSet& results)
{
  unsigned int i = PVR_TIMER_NO_CLIENT_INDEX + 1;
  for (const auto& timer : m_timers)
  {
    kodi::addon::PVRTimer kodiTimer;

    /* TODO: Implement own timer types to get support for the timer features introduced with PVR API 1.9.7 */
    kodiTimer.SetTimerType(PVR_TIMER_TYPE_NONE);
    kodiTimer.SetClientIndex(i++);
    kodiTimer.SetClientChannelUid(timer.iChannelId);
    kodiTimer.SetStartTime(timer.startTime);
    kodiTimer.SetEndTime(timer.endTime);
    kodiTimer.SetState(timer.state);
    kodiTimer.SetTitle(timer.strTitle);
    kodiTimer.SetSummary(timer.strSummary);

    results.Add(kodiTimer);
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::CallEPGMenuHook(const kodi::addon::PVRMenuhook& menuhook, const kodi::addon::PVREPGTag& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallChannelMenuHook(const kodi::addon::PVRMenuhook& menuhook, const kodi::addon::PVRChannel& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallTimerMenuHook(const kodi::addon::PVRMenuhook& menuhook, const kodi::addon::PVRTimer& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallRecordingMenuHook(const kodi::addon::PVRMenuhook& menuhook, const kodi::addon::PVRRecording& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallSettingsMenuHook(const kodi::addon::PVRMenuhook& menuhook)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallMenuHook(const kodi::addon::PVRMenuhook& menuhook)
{
  int iMsg;
  switch (menuhook.GetHookId())
  {
    case 1:
      iMsg = 30010;
      break;
    case 2:
      iMsg = 30011;
      break;
    case 3:
      iMsg = 30012;
      break;
    default:
      return PVR_ERROR_INVALID_PARAMETERS;
  }
  kodi::QueueNotification(QUEUE_INFO, "", kodi::GetLocalizedString(iMsg));

  return PVR_ERROR_NO_ERROR;
}

bool CPVRDemo::LoadDemoData(void)
{
  TiXmlDocument xmlDoc;
  std::string strSettingsFile = kodi::GetAddonPath("PVRDemoAddonSettings.xml");

  if (!xmlDoc.LoadFile(strSettingsFile))
  {
    kodi::Log(ADDON_LOG_ERROR, "invalid demo data (no/invalid data file found at '%s')", strSettingsFile.c_str());
    return false;
  }

  TiXmlElement *pRootElement = xmlDoc.RootElement();
  if (strcmp(pRootElement->Value(), "demo") != 0)
  {
    kodi::Log(ADDON_LOG_ERROR, "invalid demo data (no <demo> tag found)");
    return false;
  }

  /* load channels */
  int iUniqueChannelId = 0;
  TiXmlElement *pElement = pRootElement->FirstChildElement("channels");
  if (pElement)
  {
    TiXmlNode *pChannelNode = NULL;
    while ((pChannelNode = pElement->IterateChildren(pChannelNode)) != NULL)
    {
      PVRDemoChannel channel;
      if (ScanXMLChannelData(pChannelNode, ++iUniqueChannelId, channel))
        m_channels.push_back(channel);
    }
  }

  /* load channel groups */
  int iUniqueGroupId = 0;
  pElement = pRootElement->FirstChildElement("channelgroups");
  if (pElement)
  {
    TiXmlNode *pGroupNode = NULL;
    while ((pGroupNode = pElement->IterateChildren(pGroupNode)) != NULL)
    {
      PVRDemoChannelGroup group;
      if (ScanXMLChannelGroupData(pGroupNode, ++iUniqueGroupId, group))
        m_groups.push_back(group);
    }
  }

  /* load EPG entries */
  pElement = pRootElement->FirstChildElement("epg");
  if (pElement)
  {
    TiXmlNode *pEpgNode = NULL;
    while ((pEpgNode = pElement->IterateChildren(pEpgNode)) != NULL)
    {
      ScanXMLEpgData(pEpgNode);
    }
  }

  /* load recordings */
  iUniqueGroupId = 0; // reset unique ids
  pElement = pRootElement->FirstChildElement("recordings");
  if (pElement)
  {
    TiXmlNode *pRecordingNode = NULL;
    while ((pRecordingNode = pElement->IterateChildren(pRecordingNode)) != NULL)
    {
      PVRDemoRecording recording;
      if (ScanXMLRecordingData(pRecordingNode, ++iUniqueGroupId, recording))
        m_recordings.push_back(recording);
    }
  }

  /* load deleted recordings */
  pElement = pRootElement->FirstChildElement("recordingsdeleted");
  if (pElement)
  {
    TiXmlNode *pRecordingNode = NULL;
    while ((pRecordingNode = pElement->IterateChildren(pRecordingNode)) != NULL)
    {
      PVRDemoRecording recording;
      if (ScanXMLRecordingData(pRecordingNode, ++iUniqueGroupId, recording))
        m_recordingsDeleted.push_back(recording);
    }
  }

  /* load timers */
  pElement = pRootElement->FirstChildElement("timers");
  if (pElement)
  {
    TiXmlNode *pTimerNode = NULL;
    while ((pTimerNode = pElement->IterateChildren(pTimerNode)) != NULL)
    {
      PVRDemoTimer timer;
      if (ScanXMLTimerData(pTimerNode, timer))
        m_timers.push_back(timer);
    }
  }

  return true;
}

bool CPVRDemo::GetChannel(const kodi::addon::PVRChannel &channel, PVRDemoChannel &myChannel)
{
  for (const auto& thisChannel : m_channels)
  {
    if (thisChannel.iUniqueId == (int) channel.GetUniqueId())
    {
      myChannel.iUniqueId         = thisChannel.iUniqueId;
      myChannel.bRadio            = thisChannel.bRadio;
      myChannel.iChannelNumber    = thisChannel.iChannelNumber;
      myChannel.iSubChannelNumber = thisChannel.iSubChannelNumber;
      myChannel.iEncryptionSystem = thisChannel.iEncryptionSystem;
      myChannel.strChannelName    = thisChannel.strChannelName;
      myChannel.strIconPath       = thisChannel.strIconPath;
      myChannel.strStreamURL      = thisChannel.strStreamURL;

      return true;
    }
  }

  return false;
}

std::string CPVRDemo::GetRecordingURL(const kodi::addon::PVRRecording &recording)
{
  for (const auto& thisRecording : m_recordings)
  {
    if (thisRecording.strRecordingId == recording.GetRecordingId())
    {
      return thisRecording.strStreamURL;
    }
  }

  return "";
}

bool CPVRDemo::ScanXMLChannelData(const TiXmlNode* pChannelNode, int iUniqueChannelId, PVRDemoChannel& channel)
{
  std::string strTmp;
  channel.iUniqueId = iUniqueChannelId;

  /* channel name */
  if (!XMLUtils::GetString(pChannelNode, "name", strTmp))
    return false;
  channel.strChannelName = strTmp;

  /* radio/TV */
  XMLUtils::GetBoolean(pChannelNode, "radio", channel.bRadio);

  /* channel number */
  if (!XMLUtils::GetInt(pChannelNode, "number", channel.iChannelNumber))
    channel.iChannelNumber = iUniqueChannelId;

  /* sub channel number */
  if (!XMLUtils::GetInt(pChannelNode, "subnumber", channel.iSubChannelNumber))
    channel.iSubChannelNumber = 0;

  /* CAID */
  if (!XMLUtils::GetInt(pChannelNode, "encryption", channel.iEncryptionSystem))
    channel.iEncryptionSystem = 0;

  /* icon path */
  if (!XMLUtils::GetString(pChannelNode, "icon", strTmp))
    channel.strIconPath = m_strDefaultIcon;
  else
    channel.strIconPath = ClientPath() + strTmp;

  /* stream url */
  if (!XMLUtils::GetString(pChannelNode, "stream", strTmp))
    channel.strStreamURL = m_strDefaultMovie;
  else
    channel.strStreamURL = strTmp;

  return true;
}

bool CPVRDemo::ScanXMLChannelGroupData(const TiXmlNode* pGroupNode, int iUniqueGroupId, PVRDemoChannelGroup& group)
{
  std::string strTmp;
  group.iGroupId = iUniqueGroupId;

  /* group name */
  if (!XMLUtils::GetString(pGroupNode, "name", strTmp))
    return false;
  group.strGroupName = strTmp;

  /* radio/TV */
  XMLUtils::GetBoolean(pGroupNode, "radio", group.bRadio);

  /* sort position */
  XMLUtils::GetInt(pGroupNode, "position", group.iPosition);

  /* members */
  const TiXmlNode* pMembers = pGroupNode->FirstChild("members");
  const TiXmlNode* pMemberNode = nullptr;
  while (pMembers != nullptr && (pMemberNode = pMembers->IterateChildren(pMemberNode)) != nullptr)
  {
    int iChannelId = atoi(pMemberNode->FirstChild()->Value());
    if (iChannelId > -1)
      group.members.push_back(iChannelId);
  }

  return true;
}

bool CPVRDemo::ScanXMLEpgData(const TiXmlNode* pEpgNode)
{
  std::string strTmp;
  int iTmp;
  PVRDemoEpgEntry entry;

  /* broadcast id */
  if (!XMLUtils::GetInt(pEpgNode, "broadcastid", entry.iBroadcastId))
    return false;

  /* channel id */
  if (!XMLUtils::GetInt(pEpgNode, "channelid", iTmp))
    return false;
  PVRDemoChannel& channel = m_channels.at(iTmp - 1);
  entry.iChannelId = channel.iUniqueId;

  /* title */
  if (!XMLUtils::GetString(pEpgNode, "title", strTmp))
    return false;
  entry.strTitle = strTmp;

  /* start */
  if (!XMLUtils::GetInt(pEpgNode, "start", iTmp))
    return false;
  entry.startTime = iTmp;

  /* end */
  if (!XMLUtils::GetInt(pEpgNode, "end", iTmp))
    return false;
  entry.endTime = iTmp;

  /* plot */
  if (XMLUtils::GetString(pEpgNode, "plot", strTmp))
    entry.strPlot = strTmp;

  /* plot outline */
  if (XMLUtils::GetString(pEpgNode, "plotoutline", strTmp))
    entry.strPlotOutline = strTmp;

  if (!XMLUtils::GetInt(pEpgNode, "series", entry.iSeriesNumber))
    entry.iSeriesNumber = EPG_TAG_INVALID_SERIES_EPISODE;

  if (!XMLUtils::GetInt(pEpgNode, "episode", entry.iEpisodeNumber))
    entry.iEpisodeNumber = EPG_TAG_INVALID_SERIES_EPISODE;

  if (XMLUtils::GetString(pEpgNode, "episodetitle", strTmp))
    entry.strEpisodeName = strTmp;

  /* icon path */
  if (XMLUtils::GetString(pEpgNode, "icon", strTmp))
    entry.strIconPath = strTmp;

  /* genre type */
  XMLUtils::GetInt(pEpgNode, "genretype", entry.iGenreType);

  /* genre subtype */
  XMLUtils::GetInt(pEpgNode, "genresubtype", entry.iGenreSubType);

  kodi::Log(ADDON_LOG_DEBUG, "loaded EPG entry '%s' channel '%d' start '%d' end '%d'", entry.strTitle.c_str(), entry.iChannelId, entry.startTime, entry.endTime);

  channel.epg.push_back(entry);

  return true;
}

bool CPVRDemo::ScanXMLRecordingData(const TiXmlNode* pRecordingNode, int iUniqueGroupId, PVRDemoRecording& recording)
{
  std::string strTmp;

  /* radio/TV */
  XMLUtils::GetBoolean(pRecordingNode, "radio", recording.bRadio);

  /* recording title */
  if (!XMLUtils::GetString(pRecordingNode, "title", strTmp))
    return false;
  recording.strTitle = strTmp;

  /* recording url */
  if (!XMLUtils::GetString(pRecordingNode, "url", strTmp))
    recording.strStreamURL = m_strDefaultMovie;
  else
    recording.strStreamURL = strTmp;

  /* recording path */
  if (XMLUtils::GetString(pRecordingNode, "directory", strTmp))
    recording.strDirectory = strTmp;

  strTmp = StringUtils::Format("%d", iUniqueGroupId);
  recording.strRecordingId = strTmp;

  /* channel name */
  if (XMLUtils::GetString(pRecordingNode, "channelname", strTmp))
    recording.strChannelName = strTmp;

  /* plot */
  if (XMLUtils::GetString(pRecordingNode, "plot", strTmp))
    recording.strPlot = strTmp;

  /* plot outline */
  if (XMLUtils::GetString(pRecordingNode, "plotoutline", strTmp))
    recording.strPlotOutline = strTmp;

  /* Episode Name */
  if (XMLUtils::GetString(pRecordingNode, "episodetitle", strTmp))
    recording.strEpisodeName = strTmp;

  /* Series Number */
  if (!XMLUtils::GetInt(pRecordingNode, "series", recording.iSeriesNumber))
    recording.iSeriesNumber = PVR_RECORDING_INVALID_SERIES_EPISODE;

  /* Episode Number */
  if (!XMLUtils::GetInt(pRecordingNode, "episode", recording.iEpisodeNumber))
    recording.iEpisodeNumber = PVR_RECORDING_INVALID_SERIES_EPISODE;

  /* genre type */
  XMLUtils::GetInt(pRecordingNode, "genretype", recording.iGenreType);

  /* genre subtype */
  XMLUtils::GetInt(pRecordingNode, "genresubtype", recording.iGenreSubType);

  /* duration */
  XMLUtils::GetInt(pRecordingNode, "duration", recording.iDuration);

  /* recording time */
  if (XMLUtils::GetString(pRecordingNode, "time", strTmp))
  {
    time_t timeNow = time(nullptr);
    struct tm* now = localtime(&timeNow);

    auto delim = strTmp.find(':');
    if (delim != std::string::npos)
    {
      now->tm_hour = std::stoi(StringUtils::Left(strTmp, delim));
      now->tm_min  = std::stoi(StringUtils::Mid(strTmp, (delim + 1)));
      now->tm_mday--; // yesterday

      recording.recordingTime = mktime(now);
    }
  }

  return true;
}

bool CPVRDemo::ScanXMLTimerData(const TiXmlNode* pTimerNode, PVRDemoTimer& timer)
{
  std::string strTmp;
  int iTmp;

  time_t timeNow = time(nullptr);
  struct tm* now = localtime(&timeNow);

  /* channel id */
  if (!XMLUtils::GetInt(pTimerNode, "channelid", iTmp))
    return false;
  PVRDemoChannel &channel = m_channels.at(iTmp - 1);
  timer.iChannelId = channel.iUniqueId;

  /* state */
  if (XMLUtils::GetInt(pTimerNode, "state", iTmp))
    timer.state = (PVR_TIMER_STATE) iTmp;

  /* title */
  if (!XMLUtils::GetString(pTimerNode, "title", strTmp))
    return false;
  timer.strTitle = strTmp;

  /* summary */
  if (!XMLUtils::GetString(pTimerNode, "summary", strTmp))
    return false;
  timer.strSummary = strTmp;

  /* start time */
  if (XMLUtils::GetString(pTimerNode, "starttime", strTmp))
  {
    auto delim = strTmp.find(':');
    if (delim != std::string::npos)
    {
      now->tm_hour = std::stoi(StringUtils::Left(strTmp, delim));
      now->tm_min  = std::stoi(StringUtils::Mid(strTmp, delim + 1));

      timer.startTime = mktime(now);
    }
  }

  /* end time */
  if (XMLUtils::GetString(pTimerNode, "endtime", strTmp))
  {
    auto delim = strTmp.find(':');
    if (delim != std::string::npos)
    {
      now->tm_hour = std::stoi(StringUtils::Left(strTmp, delim));
      now->tm_min  = std::stoi(StringUtils::Mid(strTmp, delim + 1));

      timer.endTime = mktime(now);
    }
  }

  kodi::Log(ADDON_LOG_DEBUG, "loaded timer '%s' channel '%d' start '%d' end '%d'", timer.strTitle.c_str(), timer.iChannelId, timer.startTime, timer.endTime);
  return true;
}

ADDONCREATOR(CPVRDemo)
/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/AddonBase.h>

#include <mutex>

class ATTR_DLL_LOCAL CMyAddon : public kodi::addon::CAddonBase
{
public:
  CMyAddon() = default;

  ADDON_STATUS CreateInstance(const kodi::addon::IInstanceInfo& instance,
                                      KODI_ADDON_INSTANCE_HDL& hdl) override;
};

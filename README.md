[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)
[![Build and run 
tests](https://github.com/kodi-pvr/pvr.demo/actions/workflows/build.yml/badge.svg?branch=Omega)](https://github.com/kodi-pvr/pvr.demo/actions/workflows/build.yml)
[![Build Status](https://dev.azure.com/teamkodi/kodi-pvr/_apis/build/status/kodi-pvr.pvr.demo?branchName=Omega)](https://dev.azure.com/teamkodi/kodi-pvr/_build/latest?definitionId=56&branchName=Omega)
[![Build Status](https://jenkins.kodi.tv/view/Addons/job/kodi-pvr/job/pvr.demo/job/Omega/badge/icon)](https://jenkins.kodi.tv/blue/organizations/jenkins/kodi-pvr%2Fpvr.demo/branches/)

# Demo PVR
Demo PVR client addon for [Kodi](https://kodi.tv). The add-on is aimed at both PVR add-on devs and at XBMC skin developers.

For more information on the relationship of the data and the UI see [here](pvr.demo/README.md)

## Build instructions

### Linux

1. `git clone --branch master https://github.com/xbmc/xbmc.git`
2. `git clone https://github.com/kodi-pvr/pvr.demo.git`
3. `cd pvr.demo && mkdir build && cd build`
4. `cmake -DADDONS_TO_BUILD=pvr.demo -DADDON_SRC_PREFIX=../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../xbmc/addons -DPACKAGE_ZIP=1 ../../xbmc/cmake/addons`
5. `make`

##### Useful links

* [Kodi's PVR user support](https://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support](https://forum.kodi.tv/forumdisplay.php?fid=136)

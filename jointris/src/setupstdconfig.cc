/*
 * Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   setupstdconfig.cc
 */

#include "setupstdconfig.h"

#include <stmm-jointris/blockevent.h>
#include <stmm-jointris/feederevent.h>

#include <stmm-games/stdconfig.h>
#include <stmm-games/options/booloption.h>

#include <stmm-input-au/playbackcapability.h>

#include <stmm-input-ev/stmm-input-ev.h>

#include <stmm-input/devicemanager.h>

#include <iostream>
#include <cassert>

namespace stmg
{

void jointrisSetupStdConfig(shared_ptr<StdConfig>& refStdConfig, const shared_ptr<stmi::DeviceManager>& refDeviceManager
								, const std::string& sJointris, const std::string& sAppVersion, bool bNoSound, bool bTestMode) noexcept
{
	assert(refStdConfig.get() == nullptr);
	StdConfig::Init oStdConfigInit;
	oStdConfigInit.m_sAppName = sJointris;
	oStdConfigInit.m_sAppVersion = sAppVersion;
	oStdConfigInit.m_bTestMode = bTestMode;

	oStdConfigInit.m_bSoundEnabled = ! bNoSound;
	oStdConfigInit.m_bSoundPerPlayerAllowed = ! bNoSound; // Per player sound is useful for team vs team games

	oStdConfigInit.m_refDeviceManager = refDeviceManager;

	AppConstraints& oAppConstraints = oStdConfigInit.m_oAppConstraints;
	oAppConstraints.m_nAIMatesPerTeamMax = 0;

	StdConfig::CapabilityAssignment& oCapabilityAssignment = oStdConfigInit.m_oCapabilityAssignment;
	oCapabilityAssignment.m_bCapabilitiesAutoAssignedToActivePlayer = true;

	std::vector<StdConfig::KeyAction>& aKeyActions = oStdConfigInit.m_aKeyActions;
	std::vector< std::pair<stmi::Capability::Class, std::vector<stmi::HARDWARE_KEY> > > aDefaultClassKeys;
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_UP, stmi::HK_W}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionRotate}, "Rotate", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_LEFT, stmi::HK_A}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionLeft}, "Left", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_RIGHT, stmi::HK_D}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionRight}, "Right", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_DOWN, stmi::HK_S}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionDown}, "Down", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_SPACE, stmi::HK_Q}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionDrop}, "Drop", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_B, stmi::HK_1}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionInvertRotation}, "Inv.Rotation", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	aDefaultClassKeys.push_back(std::make_pair(stmi::KeyCapability::getClass(), std::vector<stmi::HARDWARE_KEY>{stmi::HK_V, stmi::HK_TAB}));
	aKeyActions.emplace_back(std::vector<std::string>{BlockEvent::s_sKeyActionNext}, "Next", aDefaultClassKeys);
	aDefaultClassKeys.clear();
	//
	std::vector< shared_ptr<Option> >& aOptions = oStdConfigInit.m_aOptions;
	aOptions.push_back(std::make_shared<BoolOption>(OwnerType::GAME, BlockEvent::s_sGameOptionDelayedFreeze, false, "Delayed freeze"));
	aOptions.push_back(std::make_shared<BoolOption>(OwnerType::PLAYER, BlockEvent::s_sPlayerOptionClockwiseRotation, true, "Clockwise rotation"));
	aOptions.push_back(std::make_shared<BoolOption>(OwnerType::GAME, FeederEvent::s_sGameOptionShowPreview, true, "Show preview"));
	//
	refStdConfig = std::make_shared<StdConfig>(std::move(oStdConfigInit));
}

} //namespace stmg


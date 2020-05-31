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
 * File:   xmlblockevent.cc
 */

#include "xmlblockevent.h"

#include <stmm-games-xml-base/xmlcommonerrors.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>
#include <stmm-games-xml-base/xmlutil/xmlbasicparser.h>
#include <stmm-games-xml-game/gamectx.h>

#include <stmm-games/util/util.h>
#include <stmm-games/game.h>

#include <cassert>
#include <utility>
#include <string>
//#include <iostream>

#include <stdint.h>

namespace stmg
{

const std::string XmlBlockEventParser::s_sEventBlockNodeName = "BlockEvent";

static const std::string s_sEventBlockBlockAttr = "block";
static const std::string s_sEventBlockInitPosAttr = "initPos";
static const std::string s_sEventBlockInitPosXAttr = "initPosX";
static const std::string s_sEventBlockInitPosYAttr = "initPosY";
static const std::string s_sEventBlockPlacingMillisecAttr = "placingMillisec";
static const std::string s_sEventBlockShapeAttr = "shape";
static const std::string s_sEventBlockSkippedFallsAttr = "skippedFalls";
static const std::string s_sEventBlockCanDropAttr = "canDrop";
static const std::string s_sEventBlockControllableAttr = "controllable";
static const std::string s_sEventBlockDestroyBrickAnimationsAttr = "destroyBrickAnimations";
static const std::string s_sEventBlockEmitDestroyedBricksAttr = "emitDestroyedBricks";
static const std::string s_sEventBlockBombNodeName = "Bomb";

XmlBlockEventParser::XmlBlockEventParser()
: XmlEventParser(s_sEventBlockNodeName)
{
}

Event* XmlBlockEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventBlock(oCtx, p0Element);
}

Event* XmlBlockEventParser::parseEventBlock(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventBlock" << '\n';
	oCtx.addChecker(p0Element);
	BlockEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);

	const auto oPairBlock = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockBlockAttr);
	if (!oPairBlock.first) {
		throw XmlCommonErrors::errorAttrNotFound(oCtx, p0Element, s_sEventBlockBlockAttr);
	}
	const std::string& sBlock = oPairBlock.second;
	const bool bFound = getBlock(oCtx, sBlock, oInit.m_oBlock);
	if (!bFound) {
		throw XmlCommonErrors::error(oCtx, p0Element, s_sEventBlockBlockAttr
									, Util::stringCompose("Attribute '%1': Block '%2' not defined", s_sEventBlockBlockAttr, sBlock));
	}
	const int32_t nTotRemovedShapes = oInit.m_oBlock.shapeRemoveAllInvisible();
	if (nTotRemovedShapes > 0) {
		throw XmlCommonErrors::error(oCtx, p0Element, s_sEventBlockBlockAttr
									, Util::stringCompose("Attribute '%1': Block '%2' cannot contain empty shapes", s_sEventBlockBlockAttr, sBlock));
	}
	if (oInit.m_oBlock.isEmpty()) {
		throw XmlCommonErrors::error(oCtx, p0Element, s_sEventBlockBlockAttr
									, Util::stringCompose("Attribute '%1': Block cannot be empty", s_sEventBlockBlockAttr));
	}

	const auto oPairInitPosX = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockInitPosXAttr);
	const bool bInitPosXDefined = oPairInitPosX.first;
	if (bInitPosXDefined) {
		const std::string& sInitPosX = oPairInitPosX.second;
		oInit.m_oInitPos.m_nX = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventBlockInitPosXAttr, sInitPosX, false, false, -1, false, -1);
	}
	const auto oPairInitPosY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockInitPosYAttr);
	const bool bInitPosYDefined = oPairInitPosY.first;
	if (bInitPosYDefined) {
		const std::string& sInitPosY = oPairInitPosY.second;
		oInit.m_oInitPos.m_nY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventBlockInitPosYAttr, sInitPosY, false, false, -1, false, -1);
	}
	int32_t nInitPos = 0; // center
	const auto oPairInitPos = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockInitPosAttr);
	if (oPairInitPos.first) {
		if (bInitPosXDefined) {
			throw XmlCommonErrors::errorAttrAlreadyDefinedByAnother(oCtx, p0Element, s_sEventBlockInitPosAttr, s_sEventBlockInitPosXAttr);
		}
		if (bInitPosYDefined) {
			throw XmlCommonErrors::errorAttrAlreadyDefinedByAnother(oCtx, p0Element, s_sEventBlockInitPosAttr, s_sEventBlockInitPosYAttr);
		}
		const std::string& sInitPos = oPairInitPos.second;
		nInitPos = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventBlockInitPosAttr, sInitPos, false, false, -1, false, -1);
	}

	const auto oPairPlacing = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockPlacingMillisecAttr);
	if (oPairPlacing.first) {
		const std::string& sPlacing = oPairPlacing.second;
		oInit.m_nPlacingMillisec = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventBlockPlacingMillisecAttr, sPlacing, false, true, 0, false, -1);
	}
	const auto oPairShape = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockShapeAttr);
	if (oPairShape.first) {
		const std::string& sShape = oPairShape.second;
		oInit.m_nInitShape = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventBlockShapeAttr, sShape, false
																	, true, 0, true, oInit.m_oBlock.totShapes() - 1);
	}
	const auto oPairSkippedFalls = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockSkippedFallsAttr);
	if (oPairSkippedFalls.first) {
		const std::string& sSkippedFalls = oPairSkippedFalls.second;
		oInit.m_nSkippedFalls = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventBlockSkippedFallsAttr, sSkippedFalls, false
																, true, -1, false, -1);
	}
	const auto oPairCanDrop = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockCanDropAttr);
	if (oPairCanDrop.first) {
		const std::string& sCanDrop = oPairCanDrop.second;
		oInit.m_bCanDrop = XmlUtil::strToBool(oCtx, p0Element, s_sEventBlockCanDropAttr, sCanDrop);
	}
	const auto oPairControllable = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockControllableAttr);
	if (oPairControllable.first) {
		const std::string& sControllable = oPairControllable.second;
		oInit.m_bControllable = XmlUtil::strToBool(oCtx, p0Element, s_sEventBlockControllableAttr, sControllable);
	}
	const auto oPairDestroyAnimations = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockDestroyBrickAnimationsAttr);
	if (oPairDestroyAnimations.first) {
		const std::string& sDestroyAnimations = oPairDestroyAnimations.second;
		oInit.m_bCreateDestroyBrickAnimation = XmlUtil::strToBool(oCtx, p0Element, s_sEventBlockDestroyBrickAnimationsAttr, sDestroyAnimations);
	}
	const auto oPairEmitDestroyedBricks = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventBlockEmitDestroyedBricksAttr);
	if (oPairEmitDestroyedBricks.first) {
		const std::string& sEmitDestroyedBricks = oPairEmitDestroyedBricks.second;
		oInit.m_bEmitDestroyedBricks = XmlUtil::strToBool(oCtx, p0Element, s_sEventBlockEmitDestroyedBricksAttr, sEmitDestroyedBricks);
	}

	const xmlpp::Element* p0BombElement = getXmlConditionalParser().parseUniqueElement(oCtx, p0Element, s_sEventBlockBombNodeName, false);
	if (p0BombElement != nullptr) {
		oInit.m_refBomb = getXmlTraitsParser().parseTileSelectorAnd(oCtx, p0BombElement);
	}

	const auto oTupleTeam = getXmlConditionalParser().parseOwnerExists(oCtx, p0Element);
	const bool bExists = std::get<0>(oTupleTeam);
	// Note: ignores mate even if defined because blocks can currently only
	//       be restricted to a team not a player
	// Note 2: in OneTeamPerLevel mode it doesn't matter if a block can only
	//         be controlled by players of a certain team since there is only
	//         one team in the level anyway
	const int32_t nTeam = (bExists ? std::get<1>(oTupleTeam) : -1);
	if (oCtx.game().isAllTeamsInOneLevel()) {
		oInit.m_nLevelTeam = nTeam;
	} else {
		oInit.m_nLevelTeam = 0;
	}
	if (!(bInitPosXDefined || bInitPosYDefined)) {
		oInit.m_oInitPos = BlockEvent::calcInitialPos(&oCtx.level(), oInit.m_oBlock, nInitPos, oInit.m_nInitShape);
	}

	oCtx.removeChecker(p0Element, true);
	auto refBlockEvent = std::make_unique<BlockEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refBlockEvent), p0Element);
}
int32_t XmlBlockEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
														, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "CANNOT_PLACE") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_CANNOT_PLACE;
	} else if (sListenerGroupName == "COULD_PLACE") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_COULD_PLACE;
	} else if (sListenerGroupName == "FUSED_WITH") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_FUSED_WITH;
	} else if (sListenerGroupName == "REMOVED") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_REMOVED;
	} else if (sListenerGroupName == "DESTROYED") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_DESTROYED;
	} else if (sListenerGroupName == "FREEZED") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_FREEZED;
	} else if (sListenerGroupName == "FUSED_TO") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_FUSED_TO;
	} else if (sListenerGroupName == "BRICK_DESTROYED") {
		nListenerGroup = BlockEvent::LISTENER_GROUP_BRICK_DESTROYED;
	} else {
		return XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg

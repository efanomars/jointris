/*
 * Copyright © 2019  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   xmlfeederevent.cc
 */

#include "xmlfeederevent.h"

#include "xmlblockevent.h"

#include <stmm-games-xml-base/xmlcommonerrors.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
//#include <stmm-games-xml-base/xmltraitsparser.h>

#include <stmm-games/game.h>
#include <stmm-games/util/util.h>
#include <stmm-games/layout.h>

#include <cassert>
//#include <algorithm>
#include <utility>
#include <string>
//#include <iostream>

namespace stmg
{

static const std::string s_sEventFeederNodeName = "FeederEvent";
static const std::string s_sEventFeederWaitAttr = "wait";
static const std::string s_sEventFeederPreviewNameAttr = "preview";
static const std::string s_sEventFeederAllowBlockBunchRepetitionAttr = "allowRepetition";
static const std::string s_sEventFeederBunchNodeName = "Bunch";
static const std::string s_sEventFeederBunchJoinedAttr = "joined";

XmlFeederEventParser::XmlFeederEventParser()
: XmlEventParser(s_sEventFeederNodeName)
{
}

Event* XmlFeederEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventFeeder(oCtx, p0Element);
}

Event* XmlFeederEventParser::parseEventFeeder(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventFeeder" << '\n';
	oCtx.addChecker(p0Element);
	FeederEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);

	oInit.m_nRepeat = parseEventAttrRepeat(oCtx, p0Element);

	const auto oPairWait = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFeederWaitAttr);
	if (oPairWait.first) {
		const std::string& sWait = oPairWait.second;
		oInit.m_nWait = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventFeederWaitAttr, sWait, false, true, 0, false, -1);
	}

	const auto oOwnerPair = getXmlConditionalParser().parseOwner(oCtx, p0Element);

	const auto oPairPreviewName = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFeederPreviewNameAttr);
	if (oPairPreviewName.first) {
		const std::string& sPreviewName = oPairPreviewName.second;
		if (sPreviewName.empty()) {
			throw XmlCommonErrors::errorAttrCannotBeEmpty(oCtx, p0Element, s_sEventFeederPreviewNameAttr);
		}
		const int32_t nTeam = oOwnerPair.first;
		const int32_t nMate = oOwnerPair.second;
//std::cout << "XmlFeederEventParser::parseEventFeeder  sPreviewName=" << sPreviewName << "  nTeam=" << nTeam << "  nMate=" << nMate << '\n';
		auto refGameWidget = oCtx.game().getLayout()->getWidgetNamed(sPreviewName, nTeam, nMate);
		if (!refGameWidget) {
			const std::string sType = [&]() {
				if (nTeam < 0) {
					return "Game";
				} else if (nMate < 0) {
					return "Team";
				} else {
					return "Mate";
				}
			}();
			throw XmlCommonErrors::error(oCtx, p0Element, s_sEventFeederPreviewNameAttr, Util::stringCompose("Attribute '%1':"
										" %2 widget with name '%3' not found", s_sEventFeederPreviewNameAttr, sType, sPreviewName));
		}
		oInit.m_refPreview = std::dynamic_pointer_cast<PreviewWidget>(refGameWidget);
		if (!oInit.m_refPreview) {
			throw XmlCommonErrors::error(oCtx, p0Element, s_sEventFeederPreviewNameAttr, Util::stringCompose("Attribute '%1':"
											" the widget with name '%2' is not a preview widget!"));
		}
	}

	const auto oPairAllowRepetition = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFeederAllowBlockBunchRepetitionAttr);
	if (oPairAllowRepetition.first) {
		const std::string& sAllowRepetition = oPairAllowRepetition.second;
		oInit.m_bAllowBlockBunchRepetition = XmlUtil::strToBool(oCtx, p0Element, s_sEventFeederAllowBlockBunchRepetitionAttr, sAllowRepetition);
	}

	getXmlConditionalParser().visitNamedElementChildren(oCtx, p0Element, s_sEventFeederBunchNodeName, [&](const xmlpp::Element* p0RandomElement)
	{
		auto oPairProbBunch = parseEventFeederBunch(oCtx, p0RandomElement);
		oInit.m_aRandomBlocks.push_back(std::move(oPairProbBunch));
	});
	if (oInit.m_aRandomBlocks.empty()) {
		throw XmlCommonErrors::errorElementExpected(oCtx, p0Element, s_sEventFeederBunchNodeName);
	}
	oCtx.removeChecker(p0Element, true);
	auto refFeederEvent = std::make_unique<FeederEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refFeederEvent), p0Element);
}
std::pair<int32_t, FeederEvent::BlockBunch> XmlFeederEventParser::parseEventFeederBunch(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	oCtx.addChecker(p0Element);
	const int32_t nProb = parseEventAttrRandomProb(oCtx, p0Element);

	std::pair<int32_t, FeederEvent::BlockBunch> oPairProbBunch;
	oPairProbBunch.first = nProb;
	FeederEvent::BlockBunch& oBunch = oPairProbBunch.second;

	const auto oPairJoined = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventFeederBunchJoinedAttr);
	if (oPairJoined.first) {
		const std::string& sJoined = oPairJoined.second;
		oBunch.m_bJoined = XmlUtil::strToBool(oCtx, p0Element, s_sEventFeederBunchJoinedAttr, sJoined);
	}

	getXmlConditionalParser().visitElementChildren(oCtx, p0Element, [&](const xmlpp::Element* p0EventElement)
	{
		Event* p0Event = parseChildEvent(oCtx, p0EventElement);
		assert(p0Event != nullptr);
		auto p0BlockEvent = dynamic_cast<BlockEvent*>(p0Event);
		if (p0BlockEvent == nullptr) {
			throw XmlCommonErrors::error(oCtx, p0EventElement, Util::s_sEmptyString, "BlockEvent expected!");
		}
		oBunch.m_aBlockEvents.push_back(p0BlockEvent);
	});
	if (oBunch.m_aBlockEvents.empty()) {
		throw XmlCommonErrors::errorElementExpected(oCtx, p0Element, XmlBlockEventParser::s_sEventBlockNodeName);
	}
	oCtx.removeChecker(p0Element, false, true);
	return oPairProbBunch;
}
int32_t XmlFeederEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "PAUSE") {
		nMsg = FeederEvent::MESSAGE_PAUSE;
	} else if (sMsgName == "RESUME") {
		nMsg = FeederEvent::MESSAGE_RESUME;
	} else if (sMsgName == "RESTART") {
		nMsg = FeederEvent::MESSAGE_RESTART;
	} else if (sMsgName == "ADD_RANDOM_INT32") {
		nMsg = FeederEvent::MESSAGE_ADD_RANDOM_INT32;
	} else if (sMsgName == "ADD_RANDOM_WEIGHT") {
		nMsg = FeederEvent::MESSAGE_ADD_RANDOM_WEIGHT;
	} else if (sMsgName == "ADD_RANDOM_BUNCH") {
		nMsg = FeederEvent::MESSAGE_ADD_RANDOM_BUNCH;
	} else {
		return XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlFeederEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
															, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "BUNCH_DONE") {
		nListenerGroup = FeederEvent::LISTENER_GROUP_BUNCH_DONE;
	} else if (sListenerGroupName == "BUNCH_JOINED") {
		nListenerGroup = FeederEvent::LISTENER_GROUP_BUNCH_JOINED;
	} else if (sListenerGroupName == "BUNCH_NOT_JOINED") {
		nListenerGroup = FeederEvent::LISTENER_GROUP_BUNCH_NOT_JOINED;
	} else if (sListenerGroupName == "CANNOT_PLACE_BLOCK") {
		nListenerGroup = FeederEvent::LISTENER_GROUP_CANNOT_PLACE_BLOCK;
	} else {
		return XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg

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
 * File:   xmlrowpusherevent.cc
 */

#include "xmlrowpusherevent.h"

#include <stmm-games-xml-game/xmlutile/xmlnewrowsparser.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
#include <stmm-games-xml-base/xmltraitsparser.h>

#include <stmm-games/level.h>
#include <stmm-games/util/util.h>

#include <cassert>
#include <string>
//#include <algorithm>
//#include <iostream>

#include <stdint.h>

namespace stmg
{

static const std::string s_sEventRowPusherNodeName = "RowPusherEvent";
//static const std::string s_sEventRowPusherGapsAttr = "gaps";
static const std::string s_sEventRowPusherPushYAttr = "pushY";
static const std::string s_sEventRowPusherPushXAttr = "pushX";
static const std::string s_sEventRowPusherPushWAttr = "pushW";
static const std::string s_sEventRowPusherNewRowsNodeName = "NewRows";

XmlRowPusherEventParser::XmlRowPusherEventParser()
: XmlEventParser(s_sEventRowPusherNodeName)
{
}

Event* XmlRowPusherEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventRowPusher(oCtx, p0Element);
}

Event* XmlRowPusherEventParser::parseEventRowPusher(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventRowPusher" << '\n';
	oCtx.addChecker(p0Element);
	RowPusherEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);

	oInit.m_nPushY = oCtx.level().boardHeight() - 1;
	const auto oPairPushY = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventRowPusherPushYAttr);
	if (oPairPushY.first) {
		const std::string& sPushY = oPairPushY.second;
		oInit.m_nPushY = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventRowPusherPushYAttr, sPushY, false
									, true, 0, true, oInit.m_nPushY);
	}
	//
	oInit.m_nPushX = 0;
	const auto oPairPushX = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventRowPusherPushXAttr);
	if (oPairPushX.first) {
		const std::string& sPushX = oPairPushX.second;
		oInit.m_nPushX = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventRowPusherPushXAttr, sPushX, false
																, true, 0, true, oCtx.level().boardWidth() - 1);
	}
	//
	oInit.m_nPushW = oCtx.level().boardWidth() - oInit.m_nPushX;
	const auto oPairPushW = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventRowPusherPushWAttr);
	if (oPairPushW.first) {
		const std::string& sPushW = oPairPushW.second;
		oInit.m_nPushW = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventRowPusherPushWAttr, sPushW, false, true, 1, true, oInit.m_nPushW);
	}

	stmg::XmlNewRowsParser oXmlNewRows(getXmlConditionalParser(), getXmlTraitsParser());
	const xmlpp::Element* p0NewRowsElement = getXmlConditionalParser().parseUniqueElement(oCtx, p0Element, s_sEventRowPusherNewRowsNodeName, true);

	oInit.m_refNewRows = std::make_unique<NewRows>(oXmlNewRows.parseNewRows(oCtx, p0NewRowsElement));

	// TODO make sure random prob doesn't exceed some reasonable value (to stay within int32_t boundaries)
	oCtx.removeChecker(p0Element, true);
	auto refRowPusherEvent = std::make_unique<RowPusherEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refRowPusherEvent), p0Element);
}
int32_t XmlRowPusherEventParser::parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
												, const std::string& sMsgName)
{
	int32_t nMsg;
	if (sMsgName == "PUSH_ROW") {
		nMsg = RowPusherEvent::MESSAGE_PUSH_ROW;
	} else if (sMsgName == "PUSH_ROW_REPEAT") {
		nMsg = RowPusherEvent::MESSAGE_PUSH_ROW_REPEAT;
	} else if (sMsgName == "PUSH_ROW_BY_TWO") {
		nMsg = RowPusherEvent::MESSAGE_PUSH_ROW_BY_TWO;
	} else if (sMsgName == "PAUSE") {
		nMsg = RowPusherEvent::MESSAGE_PAUSE;
	} else if (sMsgName == "RESUME") {
		nMsg = RowPusherEvent::MESSAGE_RESUME;
	} else if (sMsgName == "SET_NEW_ROW_GEN") {
		nMsg = RowPusherEvent::MESSAGE_SET_NEW_ROW_GEN;
	} else if (sMsgName == "NEXT_NEW_ROW_GEN") {
		nMsg = RowPusherEvent::MESSAGE_NEXT_NEW_ROW_GEN;
	} else if (sMsgName == "PREV_NEW_ROW_GEN") {
		nMsg = RowPusherEvent::MESSAGE_PREV_NEW_ROW_GEN;
	} else {
		return XmlEventParser::parseEventMsgName(oCtx, p0Element, sAttr, sMsgName);
	}
	return nMsg;
}
int32_t XmlRowPusherEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
															, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "PUSHED") {
		nListenerGroup = RowPusherEvent::LISTENER_GROUP_PUSHED;
	} else {
		return XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}

} // namespace stmg

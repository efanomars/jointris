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
 * File:   xmlrowremoverevent.cc
 */

#include "xmlrowremoverevent.h"

//#include <stmm-games-xml/xmlcommonparser.h>
#include <stmm-games-xml-game/gamectx.h>
#include <stmm-games-xml-base/xmlconditionalparser.h>
//#include <stmm-games-xml-base/xmltraitsparser.h>

#include <stmm-games/util/util.h>

#include <string>
//#include <cassert>
//#include <iostream>

namespace stmg
{

static const std::string s_sEventRowRemoverNodeName = "RowRemoverEvent";
static const std::string s_sEventRowRemoverGapsAttr = "gaps";
static const std::string s_sEventRowRemoverFromGapsAttr = "fromGaps";
static const std::string s_sEventRowRemoverToGapsAttr = "toGaps";
static const std::string s_sEventRowRemoverDeleteAfterAttr = "deleteAfter";
static const std::string s_sEventRowRemoverDeleteAddAttr = "deleteAdd";

XmlRowRemoverEventParser::XmlRowRemoverEventParser()
: XmlEventParser(s_sEventRowRemoverNodeName)
{
}
Event* XmlRowRemoverEventParser::parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
	return parseEventRowRemover(oCtx, p0Element);
}
Event* XmlRowRemoverEventParser::parseEventRowRemover(GameCtx& oCtx, const xmlpp::Element* p0Element)
{
//std::cout << "parseEventRowRemover" << '\n';
	oCtx.addChecker(p0Element);
	RowRemoverEvent::Init oInit;
	parseEventBase(oCtx, p0Element, oInit);

	oInit.m_nRepeat = parseEventAttrRepeat(oCtx, p0Element);
	oInit.m_nStep = parseEventAttrStep(oCtx, p0Element);

	getXmlConditionalParser().parseAttributeFromTo<int32_t>(oCtx, p0Element, s_sEventRowRemoverGapsAttr
															, s_sEventRowRemoverFromGapsAttr, s_sEventRowRemoverToGapsAttr
															, false, true, 0, false, -1, oInit.m_nFromGaps, oInit.m_nToGaps);

	const auto oPairDeleteAfter = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventRowRemoverDeleteAfterAttr);
	if (oPairDeleteAfter.first) {
		const std::string& sDeleteAfter = oPairDeleteAfter.second;
		oInit.m_nDeleteAfter = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventRowRemoverDeleteAfterAttr, sDeleteAfter, false
															, true, 0, true, RowRemoverEvent::s_nMaxDeleteAfter);
	}

	const auto oPairDeleteAdd = getXmlConditionalParser().getAttributeValue(oCtx, p0Element, s_sEventRowRemoverDeleteAddAttr);
	if (oPairDeleteAdd.first) {
		const std::string& sDeleteAdd = oPairDeleteAdd.second;
		oInit.m_nDeleteAdd = XmlUtil::strToNumber<int32_t>(oCtx, p0Element, s_sEventRowRemoverDeleteAddAttr, sDeleteAdd, false
										, true, -RowRemoverEvent::s_nMaxDeleteAfter, true, RowRemoverEvent::s_nMaxDeleteAfter);
	}
	oCtx.removeChecker(p0Element, true);
	auto refRowRemoverEvent = std::make_unique<RowRemoverEvent>(std::move(oInit));
	return integrateAndAdd(oCtx, std::move(refRowRemoverEvent), p0Element);
}
int32_t XmlRowRemoverEventParser::parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
															, const std::string& sListenerGroupName)
{
	int32_t nListenerGroup;
	if (sListenerGroupName == "REMOVED") {
		nListenerGroup = RowRemoverEvent::LISTENER_GROUP_REMOVED;
	} else {
		return XmlEventParser::parseEventListenerGroupName(oCtx, p0Element, sAttr, sListenerGroupName);
	}
	return nListenerGroup;
}
} // namespace stmg

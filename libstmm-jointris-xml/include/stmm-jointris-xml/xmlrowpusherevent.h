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
 * File:   xmlrowpusherevent.h
 */

#ifndef STMG_XML_ROW_PUSHER_EVENT_H
#define STMG_XML_ROW_PUSHER_EVENT_H

#include <stmm-jointris/rowpusherevent.h>

#include <stmm-games-xml-game/xmleventparser.h>
#include <stmm-games-xml-game/gamectx.h>

#include <libxml++/libxml++.h>

#include <string>

namespace stmg
{

class XmlRowPusherEventParser : public XmlEventParser //, private XmlCommonParser
{
public:
	XmlRowPusherEventParser();

	Event* parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element) override;

	int32_t parseEventMsgName(ConditionalCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
							, const std::string& sMsgName) override;
	int32_t parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
										, const std::string& sListenerGroupName) override;
	using XmlEventParser::getXmlConditionalParser;
	using XmlEventParser::getXmlTraitsParser;
	using XmlEventParser::parseEventAttrRandomProb;

private:
	Event* parseEventRowPusher(GameCtx& oCtx, const xmlpp::Element* p0Element);

private:
	XmlRowPusherEventParser(const XmlRowPusherEventParser& oSource) = delete;
	XmlRowPusherEventParser& operator=(const XmlRowPusherEventParser& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_XML_ROW_PUSHER_EVENT_H */


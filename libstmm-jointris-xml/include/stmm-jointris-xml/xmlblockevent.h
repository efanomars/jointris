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
 * File:   xmlblockevent.h
 */

#ifndef STMG_XML_BLOCK_EVENT_H
#define STMG_XML_BLOCK_EVENT_H

#include <stmm-jointris/blockevent.h>

#include <stmm-games-xml-game/xmleventparser.h>

#include <libxml++/libxml++.h>

#include <vector>
#include <string>

namespace stmg
{

class GameCtx;

class XmlBlockEventParser : public XmlEventParser
{
public:
	XmlBlockEventParser();

	Event* parseEvent(GameCtx& oCtx, const xmlpp::Element* p0Element) override;

	int32_t parseEventListenerGroupName(GameCtx& oCtx, const xmlpp::Element* p0Element, const std::string& sAttr
										, const std::string& sListenerGroupName) override;

	static const std::string s_sEventBlockNodeName;

private:
	Event* parseEventBlock(GameCtx& oCtx, const xmlpp::Element* p0Element);

private:
	XmlBlockEventParser(const XmlBlockEventParser& oSource) = delete;
	XmlBlockEventParser& operator=(const XmlBlockEventParser& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_XML_BLOCK_EVENT_H */


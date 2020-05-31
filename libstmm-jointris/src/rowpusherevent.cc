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
 * File:   rowpusherevent.cc
 */

#include "rowpusherevent.h"

#include <stmm-games/level.h>
#include <stmm-games/named.h>
#include <stmm-games/util/namedindex.h>

#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>


namespace stmg
{

static std::string s_sTileAniRemoving = "TILEANI:REMOVING";

RowPusherEvent::RowPusherEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oToPushUp(level().boardHeight() * 2)
{
	commonInit(std::move(oInit));
}
void RowPusherEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	commonInit(std::move(oInit));
}
void RowPusherEvent::commonInit(LocalInit&& oInit) noexcept
{
	Level& oLevel = level();
	NamedIndex& oTileAnisIndex = oLevel.getNamed().tileAnis();
	m_nTileAniRemovingIndex = oTileAnisIndex.getIndex(s_sTileAniRemoving);
	if (m_nTileAniRemovingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! RowPusherEvent: tile animation '" << s_sTileAniRemoving << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniRemovingIndex = oTileAnisIndex.addName(s_sTileAniRemoving);
	}

	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();
	m_eState = ROW_PUSHER_STATE_ACTIVATE;

	m_nPushY = oInit.m_nPushY;
	m_nPushX = oInit.m_nPushX;
	m_nPushW = oInit.m_nPushW;

	assert((m_nPushY >= 0) && (m_nPushY <= m_nBoardH - 1));
	assert((m_nPushX >= 0) && (m_nPushW > 0) && (m_nPushX + m_nPushW <= m_nBoardW));

	assert(oInit.m_refNewRows.get() != nullptr);
	assert(oInit.m_refNewRows->getTotNewRowGens() > 0);
	m_refNewRows = std::move(oInit.m_refNewRows);

	reInitRuntime();
}
void RowPusherEvent::reInitRuntime() noexcept
{
	// run-time
	m_bPaused = false;
	m_oToPushUp.clear();
	m_nLastPushUpTime = -1;
	m_nCurRandomGen = 0;
}
void RowPusherEvent::addToPushUp(int32_t nToPushUp, int32_t nMsg) noexcept
{
//std::cout << "RowPusherEvent::addToPushUp   nToPushUp=" << nToPushUp << "  nMsg=" << nMsg << '\n';
	assert(nToPushUp >= 1);
	switch (nMsg) {
		case MESSAGE_PUSH_ROW:
		{
			if (m_oToPushUp.isEmpty()) {
				m_oToPushUp.write(nToPushUp);
//std::cout << "RowPusherEvent::addToPushUp   A"  << '\n';
				return; //------------------------------------------------------
			}
			// The last entry
			int32_t& nCurToPushUp = m_oToPushUp.peekValue(m_oToPushUp.size() - 1);
			if (nCurToPushUp < 0) {
				if (m_oToPushUp.isFull()) {
					return; //--------------------------------------------------
				}
				m_oToPushUp.write(nToPushUp);
//std::cout << "RowPusherEvent::addToPushUp   B  nCurToPushUp=" << nCurToPushUp << '\n';
				return; //------------------------------------------------------
			}
			// just increment last entry
			nCurToPushUp += nToPushUp;
//std::cout << "RowPusherEvent::addToPushUp   C  nCurToPushUp=" << nCurToPushUp << '\n';
		}
		break;
		case MESSAGE_PUSH_ROW_REPEAT:
		{
			if (! m_oToPushUp.isFull()) {
				m_oToPushUp.write(- nToPushUp);
			}
			return; //----------------------------------------------------------
		}
		break;
		case MESSAGE_PUSH_ROW_BY_TWO:
		{
			while (nToPushUp >= 2) {
				if (m_oToPushUp.isFull()) {
					return; //--------------------------------------------------
				}
				m_oToPushUp.write(-2);
				nToPushUp -= 2;
			}
			if (nToPushUp > 0) {
				if (m_oToPushUp.isFull()) {
					return; //--------------------------------------------------
				}
				m_oToPushUp.write(-1); // or 1
			}
			return; //----------------------------------------------------------
		}
		break;
	}
}
int32_t RowPusherEvent::handleOtherMsgs(int32_t nMsg, int32_t nValue, int32_t nTriggerTime, int32_t nGameTick) noexcept
{
	switch (nMsg)
	{
		case MESSAGE_PAUSE:
		{
			m_bPaused = true;
		}
		break;
		case MESSAGE_RESUME:
		{
			m_bPaused = false;
			if (! m_oToPushUp.isEmpty()) {
				return nGameTick; //--------------------------------------------
			}
		}
		break;
		case MESSAGE_SET_NEW_ROW_GEN:
		{
			m_nCurRandomGen = std::min(std::max(0, nValue), m_refNewRows->getTotNewRowGens() - 1);
		}
		break;
		case MESSAGE_NEXT_NEW_ROW_GEN:
		{
			if (m_nCurRandomGen < m_refNewRows->getTotNewRowGens() - 1) {
				++m_nCurRandomGen;
			}
		}
		break;
		case MESSAGE_PREV_NEW_ROW_GEN:
		{
			if (m_nCurRandomGen > 0) {
				--m_nCurRandomGen;
			}
		}
		break;
	}
	return nTriggerTime;
}
void RowPusherEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
	//TODO
	// ACTIVATE activate event
	// INIT init
	// RUN  insert row if possible
	// last RUN state to INIT
	//          send messages to finished-group listeners
	Level& oLevel = level();
	auto& oGame = oLevel.game();
	const int32_t nGameTick = oGame.gameElapsed();
//std::cout << "RowPusherEvent::trigger  nGameTick=" << nGameTick << " level=" << oLevel.getLevel() << '\n';

	switch (m_eState)
	{
		case ROW_PUSHER_STATE_ACTIVATE:
		{
			m_eState = ROW_PUSHER_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				if (isPushRowMsg(nMsg)) {
					const int32_t nToPushUp = std::max(1, nValue);
//std::cout << "RowPusherEvent::trigger  nToPushUp=" << nToPushUp << " nValue=" << nValue << '\n';
					addToPushUp(nToPushUp, nMsg);
					oLevel.activateEvent(this, nGameTick);
				} else {
					const int32_t nTriggerTime = getTriggerTime();
					const int32_t nNewTriggerTime = handleOtherMsgs(nMsg, nValue, nTriggerTime, nGameTick);
					if (nNewTriggerTime >= 0) {
						oLevel.activateEvent(this, nNewTriggerTime);
					}
				}
				// never push rows when triggered by another event
				return; //------------------------------------------------------
			}
//std::cout << "RowPusherEvent::trigger  A nGameTick=" << nGameTick << " level=" << oLevel.getLevel() << '\n';
			// When activated just push one row
			assert(m_oToPushUp.isEmpty());
			m_oToPushUp.write(1);
		} // fallthrough
		case ROW_PUSHER_STATE_INIT:
		{
			m_eState = ROW_PUSHER_STATE_RUN;
			m_nLastPushUpTime = -1;
		} // fallthrough
		case ROW_PUSHER_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				const int32_t nTriggerTime = getTriggerTime();
				if (isPushRowMsg(nMsg)) {
					const int32_t nToPushUp = std::max(1, nValue);
					// never push rows when triggered by another event
					addToPushUp(nToPushUp, nMsg);
					oLevel.activateEvent(this, nGameTick);
				} else {
					const int32_t nNewTriggerTime = handleOtherMsgs(nMsg, nValue, nTriggerTime, nGameTick);
					if (nNewTriggerTime >= 0) {
						oLevel.activateEvent(this, nNewTriggerTime);
					}
				}
				return; //------------------------------------------------------
			}
			if (m_bPaused) {
				return; //------------------------------------------------------
			}
			if (!checkTopCanBePushed()) {
				// retry later
//std::cout << "RowPusherEvent::trigger RETRY LATER" << '\n';
				oLevel.activateEvent(this, nGameTick + 1);
				break;
			}
			if (m_nLastPushUpTime == nGameTick) {
				// not twice on the same tick
				oLevel.activateEvent(this, nGameTick + 1);
				break;
			}
			m_nLastPushUpTime = nGameTick;
			pushRow();
			if (! m_oToPushUp.isEmpty()) {
				oLevel.activateEvent(this, nGameTick + 1);
			}
			break;
		}
		break;
	}
}

bool RowPusherEvent::checkTopCanBePushed() noexcept
{
	Level& oLevel = level();
	assert(m_nPushW > 0);
	for (int32_t nCurX = m_nPushX; nCurX < m_nPushX + m_nPushW; ++nCurX) {
		if (oLevel.boardGetTileAnimator(nCurX, 0, m_nTileAniRemovingIndex) != nullptr) {
			return false;
		}
	}
	return true;
}

void RowPusherEvent::pushRow() noexcept
{
//std::cout << "RowPusherEvent::pushRow" << '\n';
	assert(! m_oToPushUp.isEmpty());
	shared_ptr<TileBuffer> refTileBuf;
	int32_t& nCurToPush = m_oToPushUp.peekValue(0);
//std::cout << "RowPusherEvent::pushRow nCurToPush=" << nCurToPush << '\n';
	if (nCurToPush < 0) {
		if (! m_refCurrent) {
			m_refTileBufRecycler.create(m_refCurrent, NSize{m_nPushW, 1});
//			m_aRandomTilesGens[m_nCurRandomGen]->createRandomTiles(*m_refCurrent, m_nGaps);
			m_refNewRows->createNewRow(m_nCurRandomGen, *m_refCurrent, 0);
		}
		refTileBuf = m_refCurrent;
		++nCurToPush;
		if (nCurToPush >= 0) {
			m_refCurrent.reset();
			m_oToPushUp.read();
		}
	} else {
		if (! refTileBuf) {
			m_refTileBufRecycler.create(refTileBuf, NSize{m_nPushW, 1});
		}
		m_refNewRows->createNewRow(m_nCurRandomGen, *refTileBuf, 0);
//		m_aRandomTilesGens[m_nCurRandomGen]->createRandomTiles(*refTileBuf, m_nGaps);
		--nCurToPush;
		if (nCurToPush <= 0) {
			m_oToPushUp.read();
		}
	}

	NRect oArea;
	oArea.m_nX = m_nPushX;
	oArea.m_nY = 0;
	oArea.m_nW = m_nPushW;
	oArea.m_nH = m_nPushY + 1;
	level().boardInsert(Direction::UP, oArea, refTileBuf);
	informListeners(LISTENER_GROUP_PUSHED, 1);
}

} // namespace stmg

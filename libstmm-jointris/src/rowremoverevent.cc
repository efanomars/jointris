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
 * File:   rowremoverevent.cc
 */

#include "rowremoverevent.h"

#include <stmm-games/level.h>
#include <stmm-games/named.h>
#include <stmm-games/util/namedindex.h>
#include <stmm-games/util/basictypes.h>
#include <stmm-games/utile/tilerect.h>

#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>


namespace stmg
{

const std::string RowRemoverEvent::s_sRemovingTileAniName = "TILEANI:REMOVING";

RowRemoverEvent::RowRemoverEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_oData(std::move(oInit))
{
	commonInit();
}
void RowRemoverEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	commonInit();
}
void RowRemoverEvent::commonInit() noexcept
{
	Level& oLevel = level();
	NamedIndex& oTileAnisIndex = oLevel.getNamed().tileAnis();
	//
	const int32_t nTileAniRemovingIndex = oTileAnisIndex.getIndex(s_sRemovingTileAniName);
	if (nTileAniRemovingIndex < 0) {
		#ifndef NDEBUG
		std::cout << "Warning! RowRemoverEvent: tile animation '" << s_sRemovingTileAniName << "' not defined!" << '\n';
		#endif //NDEBUG
		m_nTileAniRemovingIndex = oTileAnisIndex.addName(s_sRemovingTileAniName);
	} else {
		m_nTileAniRemovingIndex = nTileAniRemovingIndex;
	}
	m_nBoardW = oLevel.boardWidth();
	m_nBoardH = oLevel.boardHeight();

	m_eState = ROW_REMOVER_STATE_ACTIVATE;

	assert((m_oData.m_nRepeat == -1) || (m_oData.m_nRepeat > 0));
	assert(m_oData.m_nStep >= 1);
	assert((m_oData.m_nFromGaps >= 0) && (m_oData.m_nFromGaps < m_nBoardW));
	assert((m_oData.m_nToGaps >= 0) && (m_oData.m_nToGaps < m_nBoardW));
	assert(m_oData.m_nFromGaps <= m_oData.m_nToGaps);
	assert((m_oData.m_nDeleteAfter >= 0) && (m_oData.m_nDeleteAfter <= s_nMaxDeleteAfter));
	assert((m_oData.m_nDeleteAdd >= - s_nMaxDeleteAfter) && (m_oData.m_nDeleteAdd <= s_nMaxDeleteAfter));

	reInitRuntime();
}

void RowRemoverEvent::reInitRuntime() noexcept
{
//std::cout << "RowRemoverEvent::reInitRuntime() m_nBoardH=" << m_nBoardH << '\n';
	// run-time
	m_nCounter = 0;
	m_aFillCount.clear();
	assert(m_nBoardH > 0);
	m_aFillCount.resize(m_nBoardH, 0);
	m_oDownCounters.clear();
}

void RowRemoverEvent::boabloPreFreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void RowRemoverEvent::boabloPostFreeze(const Coords& oCoords) noexcept
{
	boardPostDestroy(oCoords); //TODO make separate method
}
void RowRemoverEvent::boabloPreUnfreeze(const Coords& oCoords) noexcept
{
	boardPostDestroy(oCoords); //TODO make separate method
}
void RowRemoverEvent::boabloPostUnfreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void RowRemoverEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept
{
	NRect oArea;
	oArea.m_nW = m_nBoardW;
	oArea.m_nH = m_nBoardH;
	boardPreInsert(eDir, oArea, refTiles);
}
void RowRemoverEvent::boardPostScroll(Direction::VALUE eDir) noexcept
{
	NRect oArea;
	oArea.m_nW = m_nBoardW;
	oArea.m_nH = m_nBoardH;
	boardPostInsert(eDir, oArea);
}
void RowRemoverEvent::boardPreInsert(Direction::VALUE eDir, NRect /*oArea*/, const shared_ptr<TileRect>& refTiles) noexcept
{
	if (eDir == Direction::DOWN) {
		if ((refTiles) && !TileRect::isAllEmptyTiles(*refTiles)) {
			level().gameStatusTechnical(std::vector<std::string>{"RowRemoverEvent::boardPreInsert()","DOWN only supports empty tiles insertion"});
			return;
		}
	}
}
void RowRemoverEvent::boardPostInsert(Direction::VALUE eDir, NRect oArea) noexcept
{
	if (!((eDir == Direction::UP) || (eDir == Direction::DOWN))) {
		level().gameStatusTechnical(std::vector<std::string>{"RowRemoverEvent::boardPostInsert()","Only DOWN and UP supported"});
		return;
	}
	if (oArea.m_nY != 0) {
		level().gameStatusTechnical(std::vector<std::string>{"RowRemoverEvent::boardPostInsert()","Only nY=0 supported"});
		return;
	}
	if (eDir == Direction::DOWN) {
		boardPostDeleteDown(oArea.m_nY + oArea.m_nH - 1, oArea.m_nX, oArea.m_nW);
	} else{
		boardPostInsertUp(oArea.m_nY + oArea.m_nH - 1, oArea.m_nX, oArea.m_nW);
	}
}

void RowRemoverEvent::boardPostDeleteDown(int32_t nDelY, int32_t nDelX, int32_t nDelW) noexcept
{
//std::cout << "RowRemoverEvent::cellsDeleteDown() nDelY=" << nDelY << '\n';
	const int32_t nDelH = 1;
	assert((nDelX >= 0) && (nDelW > 0) && (nDelX + nDelW <= m_nBoardW));
	assert((nDelY >= 0) && (nDelH > 0) && (nDelY + nDelH <= m_nBoardH));
	if (nDelW == m_nBoardW) {
		assert(nDelX == 0);
		std::vector<int32_t>::iterator itStart = m_aFillCount.begin() + nDelY;
		std::vector<int32_t>::iterator itEnd = m_aFillCount.begin() + (nDelY + nDelH);
		m_aFillCount.erase(itStart, itEnd);
		m_aFillCount.insert(m_aFillCount.begin(), nDelH, 0);
		for (int32_t nCurY = 0; nCurY < nDelH; ++nCurY) {
			recalcRow(nCurY);
		}
	} else {
		for (int32_t nCurY = 0; nCurY < nDelY + nDelH; ++nCurY) {
			recalcRow(nCurY);
		}
	}
	for (DownCounterList::iterator itCounters = m_oDownCounters.begin()
				; itCounters != m_oDownCounters.end(); ++itCounters) {
		RowRemoverEvent::DownCounter& oDownCounter = *(*itCounters);
//std::cout << "RowRemoverEvent::cellsDeleteDown() oDownCounter.m_bLocked=" << oDownCounter.m_bLocked << '\n';
		if (!oDownCounter.m_bLocked) {
			DelStripList& oRects = oDownCounter.m_oStrips;
			for (DelStripList::iterator itRect = oRects.begin()
						; itRect != oRects.end(); ++itRect) {
				DelStrip& oRect = *(*itRect);
//std::cout << "RowRemoverEvent::cellsDeleteDown() oRect.m_nY=" << oRect.m_nY << '\n';
				if (oRect.m_nY + 1 <= nDelY) {
					adjustRectVert(oRects, oRect, nDelX, nDelW, nDelH);
				}
			}
		}
	}
}
void RowRemoverEvent::boardPostInsertUp(int32_t nInsY, int32_t nInsX, int32_t nInsW) noexcept
{
	const int32_t nInsH = 1;
	assert((nInsX >= 0) && (nInsW > 0) && (nInsX + nInsW <= m_nBoardW));
	assert((nInsY >= 0) && (nInsH > 0) && (nInsY + nInsH <= m_nBoardH));
	if (nInsW == m_nBoardW) {
		assert(nInsX == 0);
		std::vector<int32_t>::iterator itStart = m_aFillCount.begin();
		std::vector<int32_t>::iterator itEnd = m_aFillCount.begin() + nInsH;
		m_aFillCount.erase(itStart, itEnd);
		m_aFillCount.insert(m_aFillCount.begin() + nInsY, nInsH, 0);
		for (int32_t nCurY = nInsY; nCurY < nInsY + nInsH; ++nCurY) {
			recalcRow(nCurY);
		}
	} else {
		for (int32_t nCurY = 0; nCurY < nInsY + nInsH; ++nCurY) {
			recalcRow(nCurY);
		}
	}
	for (DownCounterList::iterator itCounters = m_oDownCounters.begin()
				; itCounters != m_oDownCounters.end(); ++itCounters) {
		RowRemoverEvent::DownCounter& oDownCounter = *(*itCounters);
		if (!oDownCounter.m_bLocked) {
			DelStripList& oRects = oDownCounter.m_oStrips;
			for (auto& refRect : oRects) {
				assert(refRect);
				auto& oRect = *refRect;
				if (oRect.m_nY + 1 <= nInsY + nInsH) {
					adjustRectVert(oRects, oRect, nInsX, nInsW, - nInsH);
				}
			}
			DelStripList::iterator itRect = oRects.begin();
			while (itRect != oRects.end()) {
				DelStrip& oRect = *(*itRect);
				if (oRect.m_nY < 0) { //pushed outside board
//std::cout << "RowRemoverEvent::boardPostInsertUp() pushed outside oRect.m_nY=" << oRect.m_nY << '\n';
					itRect = oRects.erase(itRect);
				} else {
					 ++itRect;
				}
			}
		}
	}
}

void RowRemoverEvent::adjustRectVert(DelStripList& oRects
		, DelStrip& oRect, int32_t nRangeX, int32_t nRangeW, int32_t nMoveH) noexcept
{
//std::cout << "RowRemoverEvent::adjustRectVert() nMoveH=" << nMoveH << '\n';
	const int32_t nX = oRect.m_nX;
	const int32_t nY = oRect.m_nY;
	const int32_t nW = oRect.m_nW;
	if (nRangeX + nRangeW <= nX) {
		return;
	}
	if (nRangeX >= nX + nW) {
		return;
	}
	int32_t nResX;
	int32_t nResX2;
	if (nRangeX < nX) {
		nResX = nX;
	} else {
		nResX = nRangeX;
	}
	if (nRangeX + nRangeW > nX + nW) {
		nResX2 = nX + nW;
	} else {
		nResX2 = nRangeX + nRangeW;
	}
	const int32_t nResW = nResX2 - nResX;
	if (nResW <= 0) {
		return;
	}

	if (nResX == nX) {
		if (nResW == nW) {
			// current rect's x-range is a subset of the deleted x-range
			//    aaaaa
			//              ->    aaaaa
			// dddDDDDDdddd    dddDDDDDdddd
			oRect.m_nY = nY + nMoveH;
		} else {
			//    aaaaa              bb
			//              ->    aaa
			// dddDDD          dddDDD
			shared_ptr<DelStrip> refRectB;
			m_oDelStripRecycler.create(refRectB);
			refRectB->m_nY = nY;
			refRectB->m_nX = nX + nResW;
			refRectB->m_nW = nW - nResW;
			oRects.insert(oRects.begin(), refRectB);
			oRect.m_nY = nY + nMoveH;
			oRect.m_nX = nResX;
			oRect.m_nW = nResW;
		}
	} else if (nResX + nResW == nX + nW) {
		// aaaaa           aa
		//              ->   bbb
		//   DDDddd          DDDddd
		shared_ptr<DelStrip> refRectB;
		m_oDelStripRecycler.create(refRectB);
		refRectB->m_nX = nResX;
		refRectB->m_nY = nY + nMoveH;
		refRectB->m_nW = nResW;
		oRects.insert(oRects.begin(), refRectB);
		oRect.m_nW = nW - nResW;
	} else {
		// aaaaa    aa  c
		//       ->   bb
		//   DD       DD
		shared_ptr<DelStrip> refRectB;
		m_oDelStripRecycler.create(refRectB);
		refRectB->m_nX = nResX;
		refRectB->m_nY = nY + nMoveH;
		refRectB->m_nW = nResW;
		oRects.insert(oRects.begin(), refRectB);
		shared_ptr<DelStrip> refRectC;
		m_oDelStripRecycler.create(refRectC);
		refRectC->m_nX = nResX + nResW;
		refRectC->m_nY = nY;
		refRectC->m_nW = nX + nW - (nResX + nResW);
		oRects.insert(oRects.begin(), refRectC);
		oRect.m_nW = nResX - nX;
	}
}
void RowRemoverEvent::boardPreDestroy(const Coords& /*oCoords*/) noexcept
{
}
void RowRemoverEvent::boardPostDestroy(const Coords& oCoords) noexcept
{
	boardPostModify(oCoords);
}
void RowRemoverEvent::boardPreModify(const TileCoords& /*oTileCoords*/) noexcept
{
}
void RowRemoverEvent::boardPostModify(const Coords& oCoords) noexcept
{
	NRect oArea = oCoords.getMinMax();
	assert((oArea.m_nX >= 0) && (oArea.m_nX < m_nBoardW));
	assert((oArea.m_nY >= 0) && (oArea.m_nY < m_nBoardH));
	assert((oArea.m_nW > 0) && (oArea.m_nX + oArea.m_nW <= m_nBoardW));
	assert((oArea.m_nH > 0) && (oArea.m_nY + oArea.m_nH <= m_nBoardH));
	for (int32_t nCurY = oArea.m_nY; nCurY < oArea.m_nY + oArea.m_nH; ++nCurY) {
		recalcRow(nCurY);
	}
}
void RowRemoverEvent::recalcRow(int32_t nY) noexcept
{
	Level& oLevel = level();
	//TODO check whether it already is in a DelStage?
	assert((nY >= 0) && (nY < m_nBoardH));
	int32_t nFill = 0;
	for (int32_t nX = 0; nX < m_nBoardW; ++nX) {
		if (!oLevel.boardGetTile(nX, nY).isEmpty()) {
			++nFill;
		}
	}
//std::cout << "RowRemoverEvent::recalcRow  m_aFillCount.size()=" << m_aFillCount.size() << "  nFill=" << nFill << '\n';
	assert(static_cast<int32_t>(m_aFillCount.size()) > nY);
	m_aFillCount[nY] = nFill;
}

void RowRemoverEvent::trigger(int32_t /*nMsg*/, int32_t /*nValue*/, Event* p0TriggeringEvent) noexcept
{
	Level& oLevel = level();
	//TODO
	// ACTIVATE activate event
	// INIT init and register
	// RUN  check for full lines
	//      send messages to listeners with the number of full lines
	//      delete full lines
	// last RUN deactivate, state to INIT
	//          send messages to finished-group listeners
	const int32_t nGameTick = oLevel.game().gameElapsed();
	switch (m_eState)
	{
		case ROW_REMOVER_STATE_ACTIVATE:
		{
			m_eState = ROW_REMOVER_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nGameTick);
				break;
			}
		} // fallthrough
		case ROW_REMOVER_STATE_INIT:
		{
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nGameTick);
				break;
			}
			m_eState = ROW_REMOVER_STATE_RUN;
			for (int32_t nY = 0; nY < m_nBoardH; ++nY) {
				recalcRow(nY);
			}
			m_nCounter = 0;

			oLevel.boardAddListener(this);
		} // fallthrough
		case ROW_REMOVER_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				break;
			}
			const bool bTerminate = (m_oData.m_nRepeat != -1) && (m_nCounter >= m_oData.m_nRepeat);
			if (bTerminate && m_oDownCounters.empty()) {
				m_eState = ROW_REMOVER_STATE_INIT; //TODO ROW_REMOVER_STATE_DEAD
				oLevel.boardRemoveListener(this);
				reInitRuntime();
				informListeners(LISTENER_GROUP_FINISHED, 0);
			} else {
				++m_nCounter;
				if (!bTerminate) {
					checkToRemove();
				}
				remove();
				oLevel.activateEvent(this, nGameTick + m_oData.m_nStep);
			}
		}
		break;
	}
}

void RowRemoverEvent::checkToRemove() noexcept
{
//std::cout << "RowRemoverEvent::checkToRemove()" << '\n';
	Level& oLevel = level();
	int32_t nToDelete = 0;
	for (int32_t nCurY = m_nBoardH - 1; nCurY >= 0; --nCurY) {
		if (checkToRemoveCondition(nCurY)) {
			int32_t nDelCountdown = m_oData.m_nDeleteAfter;
			nDelCountdown += nToDelete * m_oData.m_nDeleteAdd;
			if (nDelCountdown > s_nMaxDeleteAfter) {
				nDelCountdown = s_nMaxDeleteAfter;
			}
			if (nDelCountdown < 0) {
				nDelCountdown = 0;
			}
			shared_ptr<DownCounter> refDownCounter = recyclePopDownCounter();
			refDownCounter->m_bLocked = false;
			refDownCounter->m_nTotSteps = nDelCountdown;
			refDownCounter->m_nCountdown = nDelCountdown;
			refDownCounter->m_bInitAni = true;
			shared_ptr<DelStrip> refRect;
			m_oDelStripRecycler.create(refRect);
			refRect->m_nY = nCurY;
			refRect->m_nX = 0;
			refRect->m_nW = m_nBoardW;
			refDownCounter->m_oStrips.push_back(refRect);
			m_oDownCounters.push_back(refDownCounter);

			// mark as active for listeners
			for (int32_t nXX = 0; nXX < m_nBoardW; ++nXX) {
				oLevel.boardSetTileAnimator(nXX, nCurY, m_nTileAniRemovingIndex, this, refDownCounter->m_nAllIdx);
			}
//std::cout << "RowRemoverEvent::checkToRemove() nCurY=" << nCurY << " nDelCountdown=" << nDelCountdown << '\n';
			++nToDelete;
		}
	}
	if (nToDelete > 0) {
		informListeners(LISTENER_GROUP_REMOVED, nToDelete);
	}
}
bool RowRemoverEvent::checkToRemoveCondition(int32_t nY) noexcept
{
//std::cout << "RowRemoverEvent::checkToRemoveCondition()  nY=" << nY << "  m_aFillCount[nY]=" << m_aFillCount[nY] << '\n';
	Level& oLevel = level();
	const int32_t nFillRow = m_aFillCount[nY];
	const int32_t nGaps = m_nBoardW - nFillRow;
	if ((nGaps < m_oData.m_nFromGaps) || (nGaps > m_oData.m_nToGaps)) {
		return false;
	}
	for (int32_t nCurX = 0; nCurX < m_nBoardW; ++nCurX) {
		if (oLevel.boardGetTileAnimator(nCurX, nY, m_nTileAniRemovingIndex) != nullptr) {
			return false;
		}
	}
	return true;
}

void RowRemoverEvent::remove() noexcept
{
//std::cout << "RowRemoverEvent::remove()" << '\n';
	Level& oLevel = level();
	DownCounterList::iterator itCounters = m_oDownCounters.begin();
	while (itCounters != m_oDownCounters.end()) {
		DownCounter& oDownCounter = *(*itCounters);
		oDownCounter.m_bLocked = true;
		const int32_t nDelCountdown = oDownCounter.m_nCountdown;
		--oDownCounter.m_nCountdown;
		const bool bInitAni = oDownCounter.m_bInitAni;
		oDownCounter.m_bInitAni = false;
		const bool bDelete = (nDelCountdown == 0);
		DelStripList& oRects = oDownCounter.m_oStrips;
		for (DelStripList::iterator itRect = oRects.begin()
					; itRect != oRects.end(); ++itRect) {
			DelStrip& oRect = *(*itRect);
			const int32_t nRX = oRect.m_nX;
			const int32_t nRY = oRect.m_nY;
			const int32_t nRW = oRect.m_nW;
			const int32_t nRH = 1;
			for (int32_t nXX = nRX; nXX < nRX + nRW; ++nXX) {
				if (bDelete) {
					oLevel.boardSetTileAnimator(nXX, nRY, m_nTileAniRemovingIndex, nullptr, 0);
				} else if (bInitAni) {
				} else {
				}
			}
			if (bDelete) {
				NRect oArea;
				oArea.m_nX = nRX;
				oArea.m_nY = 0;
				oArea.m_nW = nRW;
				oArea.m_nH = nRY + 1;
				oLevel.boardInsert(Direction::DOWN, oArea, shared_ptr<TileRect>{});
			} else {
				NRect oArea;
				oArea.m_nX = nRX;
				oArea.m_nY = nRY;
				oArea.m_nW = nRW;
				oArea.m_nH = nRH;
				oLevel.boardAnimateTiles(oArea);
			}
		}
//std::cout << " dec oDelStage.m_nDelStage = " << oDelStage.m_nDelStage << '\n';
		oDownCounter.m_bLocked = false;
		if (bDelete) {
			recyclePushBackDownCounter(*itCounters);
			itCounters = m_oDownCounters.erase(itCounters);
		} else {
			 ++itCounters;
		}
	}
}

double RowRemoverEvent::getElapsed01(int32_t nHash, int32_t /*nX*/, int32_t /*nY*/, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept
{
//std::cout << "RowRemoverEvent::getElapsed01() nX=" << nX << " nY=" << nY << " nAni=" << nAni << " nViewTick=" << nViewTick << '\n';
	assert(nTotViewTicks > 0);
	assert((nViewTick >= 0) && (nViewTick < nTotViewTicks));
	if (nAni != m_nTileAniRemovingIndex) {
		assert(false);
		return 1.0;
	}

	if ((nHash < 0) || (nHash >= static_cast<int32_t>(m_oAllDownCounters.size()))) {
		assert(false);
		return 1.0;
	}
	const DownCounter& oDownCounter = *(m_oAllDownCounters[nHash]);
	assert(oDownCounter.m_nAllIdx == nHash);

	const int32_t nTotSteps = oDownCounter.m_nTotSteps;
	const int32_t nCountdown = oDownCounter.m_nCountdown + 1;
//std::cout << "RowRemoverEvent::getElapsed01() nTotSteps=" << nTotSteps << " nCountdown=" << nCountdown << '\n';
	assert(nTotSteps > 0);
	assert((nCountdown > 0) && (nCountdown <= nTotSteps));
	//TODO make an example to explain this scary formula
	const double fElapsed = 1.0 + (- nCountdown + (0.5 + nViewTick) / nTotViewTicks) / nTotSteps;
	assert((fElapsed >= 0.0) && (fElapsed <= 1.0));
	return fElapsed;
}
double RowRemoverEvent::getElapsed01(int32_t /*nHash*/, const LevelBlock& /*oLevelBlock*/, int32_t /*nBrickIdx*/, int32_t /*nAni*/
									, int32_t
									#ifndef NDEBUG
									nViewTick
									#endif //NDEBUG
									, int32_t
									#ifndef NDEBUG
									nTotViewTicks
									#endif //NDEBUG
									) const noexcept
{
	assert(nTotViewTicks > 0);
	assert((nViewTick >= 0) && (nViewTick < nTotViewTicks));

	assert(false);
	return 1.0;
}

void RowRemoverEvent::recyclePushBackDownCounter(shared_ptr<DownCounter>& refDownCounter) noexcept
{
//std::cout << "RowRemoverEvent::recyclePushBackDownCounter()"  << '\n';
	assert(refDownCounter);
	refDownCounter->m_oStrips.clear();
	m_oInactiveDownCounters.push_front(refDownCounter);
}
shared_ptr<RowRemoverEvent::DownCounter> RowRemoverEvent::recyclePopDownCounter() noexcept
{
//std::cout << "RowRemoverEvent::recyclePopDownCounter()"  << '\n';
	if (!m_oInactiveDownCounters.empty()) {
		shared_ptr<DownCounter> refTemp = *(m_oInactiveDownCounters.begin());
		m_oInactiveDownCounters.erase(m_oInactiveDownCounters.begin());
		return refTemp; //------------------------------------------------------
	}
	shared_ptr<DownCounter> refTemp = std::make_shared<DownCounter>();
	const int32_t nAllIdx = static_cast<int32_t>(m_oAllDownCounters.size());
	refTemp->m_nAllIdx = nAllIdx;
	m_oAllDownCounters.push_back(refTemp);
	return refTemp;
}

} // namespace stmg

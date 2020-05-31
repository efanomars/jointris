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
 * File:   feederevent.cc
 */

#include "feederevent.h"

#include <stmm-games/util/util.h>

#include <vector>
#include <cassert>
#include <limits>
#include <algorithm>
#include <string>
//#include <iostream>


namespace stmg
{

const std::string FeederEvent::s_sGameOptionShowPreview = "Feeder::Preview";

const std::string FeederEvent::s_sJoin = "JOIN";

FeederEvent::FeederEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, m_aRandomStack(s_nMaxRandomStackSize)
{
	commonInit(std::move(oInit));
}
void FeederEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	commonInit(std::move(oInit));
}
void FeederEvent::commonInit(LocalInit&& oInit) noexcept
{
	Level& oLevel = level();

	m_eState = FEEDER_EVENT_STATE_ACTIVATE;
	m_nRepeat = (oInit.m_nRepeat <= 0 ? std::numeric_limits<int32_t>::max() : oInit.m_nRepeat);
	m_nWait = oInit.m_nWait;
	m_bAllowBlockBunchRepetition = oInit.m_bAllowBlockBunchRepetition;
	m_refPreviewWidget = std::move(oInit.m_refPreview);

	assert((m_nRepeat > 0) || (m_nRepeat == -1));
	assert(m_nWait >= 0);

	for (auto& oRandomBlock : oInit.m_aRandomBlocks) {
		assert(oRandomBlock.first > 0);
		BlockBunch& oBlockBunch = oRandomBlock.second;
		oBlockBunch.m_aInitialBlocks.clear();
		const auto& aBlockEvents = oBlockBunch.m_aBlockEvents;
		for (auto itBEv = aBlockEvents.begin(); itBEv != aBlockEvents.end(); ++itBEv) {
			BlockEvent* p0BEv = *itBEv;
			assert(p0BEv != nullptr);
			assert(oLevel.hasEvent(p0BEv));
			oBlockBunch.m_aInitialBlocks.push_back(p0BEv->getInitialBlock());
			#ifndef NDEBUG
			// check uniqueness within bunch
			for (auto itFollowBEv = itBEv + 1; itFollowBEv != aBlockEvents.end(); ++itFollowBEv) {
				BlockEvent* p0FollowBEv = *itFollowBEv;
				assert(p0FollowBEv != p0BEv);
			}
			#endif //NDEBUG
		}
		addRandom(oRandomBlock.first, std::move(oBlockBunch));
	}
	const auto oVal = oLevel.prefs().getOptionValue(s_sGameOptionShowPreview);
	assert(oVal.getType() == Variant::TYPE_BOOL);
	m_bShowPreview = oVal.getBool();

	runtimeInit();
}

void FeederEvent::runtimeInit() noexcept
{
	// run-time
	m_nPlacedBlocks = 0;
	m_nFinishedBlocks = 0;
	m_nJoinedBlocks = 0;
	m_nPrevBunch = -1;
	m_p0NextBlocks = nullptr;
	m_p0CurBlocks = nullptr;
	m_nCounter = 0;
	m_bPaused = false;
}

void FeederEvent::addRandom(int32_t nProb, BlockBunch&& oBlockBunch) noexcept
{
//std::cout << "FeederEvent::addRandom nProb=" << nProb << '\n';
	Level& oLevel = level();
	const int32_t nIdx = m_oRandom.addRandomPart(nProb, std::move(oBlockBunch));
	auto& aBlockEvents = m_oRandom.getRandomPart(nIdx).m_aBlockEvents;

	for (BlockEvent* p0BEv : aBlockEvents) {
		assert(p0BEv != nullptr);
		// Register to all possible messages (except FINISHED that is sent along with these
		// and would make counting more difficult)
		p0BEv->addListener(BlockEvent::LISTENER_GROUP_REMOVED, this, LISTENER_MSG_MAGIC);
		p0BEv->addListener(BlockEvent::LISTENER_GROUP_DESTROYED, this, LISTENER_MSG_MAGIC);
		p0BEv->addListener(BlockEvent::LISTENER_GROUP_FREEZED, this, LISTENER_MSG_MAGIC);
		p0BEv->addListener(BlockEvent::LISTENER_GROUP_FUSED_TO, this, LISTENER_MSG_MAGIC_FUSED);
		p0BEv->addListener(BlockEvent::LISTENER_GROUP_CANNOT_PLACE, this, LISTENER_MSG_MAGIC_CANNOT_PLACE);
		p0BEv->addListener(BlockEvent::LISTENER_GROUP_COULD_PLACE, this, LISTENER_MSG_MAGIC_COULD_PLACE);
		oLevel.deactivateEvent(p0BEv);
	}
}

void FeederEvent::trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept
{
//std::cout << "FeederEvent()::trigger(" << reinterpret_cast<int64_t>(this) << ") tick=" << level().game().gameElapsed() << '\n';
//std::cout << "                        nMsg=" << nMsg << "  nValue=" << nValue << " p0TriggeringEvent=" << reinterpret_cast<int64_t>(p0TriggeringEvent) << '\n';
//std::cout << "                        m_eState=" << static_cast<int32_t>(m_eState) << "  m_bPaused=" << m_bPaused << " m_nCounter=" << m_nCounter << '\n';

	Level& oLevel = level();
	auto& oGame = oLevel.game();
	const int32_t nGameTick = oGame.gameElapsed();
	if (m_eState == FEEDER_EVENT_STATE_ZOMBIE) {
		return; //--------------------------------------------------------------
	}
	if ((nMsg == MESSAGE_ADD_RANDOM_INT32) || (nMsg == MESSAGE_ADD_RANDOM_WEIGHT) || (nMsg == MESSAGE_ADD_RANDOM_BUNCH)) {
		if (m_aRandomStack.isFull()) {
			// overflow
			oLevel.gameStatusTechnical(std::vector<std::string>{"FeederEvent::trigger()", "Random number stack overflown", "by nMsg == MESSAGE_ADD_RANDOM."
																, "Try to give new random only when needed"});
			return; //----------------------------------------------------------
		}
		RANDOM_TYPE eRT;
		if (nMsg == MESSAGE_ADD_RANDOM_INT32) {
			eRT = RANDOM_TYPE_INT32;
		} else if (nMsg == MESSAGE_ADD_RANDOM_WEIGHT) {
			eRT = RANDOM_TYPE_WEIGHT;
			const int32_t nRandomRange = m_oRandom.getRandomRange();
			nValue = nValue % nRandomRange;
			if (nValue < 0) {
				nValue += nRandomRange;
			}
		} else {
			assert(nMsg == MESSAGE_ADD_RANDOM_BUNCH);
			eRT = RANDOM_TYPE_BUNCH_IDX;
			const int32_t nTotBunches = m_oRandom.getTotRandomParts();
			nValue = nValue % nTotBunches;
			if (nValue < 0) {
				nValue += nTotBunches;
			}
		}
		m_aRandomStack.write( std::make_pair(eRT, nValue) );
		const int32_t nTriggerTime = getTriggerTime();
		if (nTriggerTime >= 0) {
			oLevel.activateEvent(this, std::max(nGameTick, nTriggerTime));
		}
		return; //--------------------------------------------------------------
	}
	switch (m_eState)
	{
		case FEEDER_EVENT_STATE_ACTIVATE:
		{
			m_eState = FEEDER_EVENT_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nGameTick);
				return; //------------------------------------------------------
			}
		} // fallthrough
		case FEEDER_EVENT_STATE_INIT:
		{
			if (p0TriggeringEvent != nullptr) {
				// If here means that the event finished and only a MESSAGE_RESTART
				// or that some other event triggered this after activateEvent in
				// FEEDER_EVENT_STATE_ACTIVATE state
				if (nMsg != MESSAGE_RESTART) {
					return; //--------------------------------------------------
				}
				oLevel.activateEvent(this, nGameTick);
				return; //------------------------------------------------------
			}
			m_eState = FEEDER_EVENT_STATE_RUN;

			// calc the first random
			calcNext();
			m_nCounter = 0;
		} // fallthrough
		case FEEDER_EVENT_STATE_RUN:
		{
			if (p0TriggeringEvent != nullptr) {
				if (m_bPaused) {
					if (nMsg == MESSAGE_RESUME) {
//std::cout << "FeederEvent()::trigger() RESUME!" << '\n';
						m_bPaused = false;
					} else {
						return; //----------------------------------------------
					}
				} else {
					if (nMsg == MESSAGE_PAUSE) {
						m_bPaused = true;
//std::cout << "FeederEvent()::trigger() PAUSED!" << '\n';
					} else  {
						return; //----------------------------------------------
					}
				}
			}
			if (m_bPaused) {
				return; //------------------------------------------------------
			}
			m_eState = FEEDER_EVENT_STATE_WAITING;
			m_p0CurBlocks = m_p0NextBlocks;
			m_nFinishedBlocks = 0;
			m_nJoinedBlocks = 0;
			m_nPlacedBlocks = 0;
			// activate all block events for the random
			const auto& aBlockEvents = m_p0CurBlocks->m_aBlockEvents;
			for (BlockEvent* p0BEv : aBlockEvents) {
				assert(p0BEv != nullptr);
				p0BEv->regenerate();
				oLevel.activateEvent(p0BEv, nGameTick);
			}
			calcNext();
		}
		break;
		case FEEDER_EVENT_STATE_WAITING:
		{
			if (p0TriggeringEvent == nullptr) {
				return; //------------------------------------------------------
			}
			switch (nMsg) {
				case LISTENER_MSG_MAGIC:
				case LISTENER_MSG_MAGIC_FUSED:
				case LISTENER_MSG_MAGIC_CANNOT_PLACE:
				case LISTENER_MSG_MAGIC_COULD_PLACE:
				break; // switch (nMsg)
				case MESSAGE_PAUSE:
				{
					m_bPaused = true;
					return; //--------------------------------------------------
				}
				case MESSAGE_RESUME:
				{
					m_bPaused = false;
					return; //--------------------------------------------------
				}
				default:
				{
//std::cout << "FeederEvent()::trigger  FEEDER_EVENT_STATE_WAITING:  unknown MSG type" << '\n';
					return; //--------------------------------------------------
				}
			}
			const bool bIsMyBlock = isMyBlock(p0TriggeringEvent);
			if (!bIsMyBlock) {
//std::cout << "FeederEvent()::trigger  FEEDER_EVENT_STATE_WAITING:  not my block" << '\n';
				return; //------------------------------------------------------
			}
			assert(m_p0CurBlocks != nullptr);
			const int32_t nTotBlocks = static_cast<int32_t>(m_p0CurBlocks->m_aBlockEvents.size());
//std::cout << "FeederEvent()::trigger  FEEDER_EVENT_STATE_WAITING:  nMsg = " << nMsg << "  nTotBlocks = " << nTotBlocks << '\n';
			// Note: while placing all blocks some might already fuse or freeze
			bool bBlockFinished = true;
			if (nMsg == LISTENER_MSG_MAGIC_FUSED) {
				const int32_t nLBId = nValue;
				const bool bIsOtherMyBlock = isMyBlock(nLBId);
				if (bIsOtherMyBlock) {
					++m_nJoinedBlocks;
				}
			} else if (nMsg == LISTENER_MSG_MAGIC_CANNOT_PLACE) {
//std::cout << "FeederEvent()::trigger  FEEDER_EVENT_STATE_WAITING:  LISTENER_MSG_MAGIC_CANNOT_PLACE" << '\n';
				m_eState = FEEDER_EVENT_STATE_ZOMBIE;
				runtimeInit();
				informListeners(LISTENER_GROUP_CANNOT_PLACE_BLOCK, 0);
				informListeners(LISTENER_GROUP_FINISHED, 0);
				return; //------------------------------------------------------
			} else if (nMsg == LISTENER_MSG_MAGIC_COULD_PLACE) {
				bBlockFinished = false;
				++m_nPlacedBlocks;
				if (nTotBlocks == m_nPlacedBlocks) {
					// only when all the blocks of the current bunch are placed on the board
					// the next bunch is shown in the preview
					showNextInPreview();
				}
			}
			if (!bBlockFinished) {
				return; //------------------------------------------------------
			}
			++m_nFinishedBlocks;
			if (nTotBlocks == m_nFinishedBlocks) {
				// all blocks froze, were destroyed or fused
				m_eState = FEEDER_EVENT_STATE_RUN;
				++m_nCounter;
				const bool bTerminate = (m_nRepeat != -1) && (m_nCounter >= m_nRepeat);
				if (m_p0CurBlocks->m_bJoined) {
					if (m_nJoinedBlocks < m_nFinishedBlocks - 1) {
						informListeners(LISTENER_GROUP_BUNCH_NOT_JOINED, 0);
					} else {
						informListeners(LISTENER_GROUP_BUNCH_JOINED, 0);
					}
				} else {
					informListeners(LISTENER_GROUP_BUNCH_DONE, 0);
				}
				if (!bTerminate) {
					// another run
					oLevel.activateEvent(this, nGameTick + m_nWait);
				} else {
//std::cout << "FeederEvent()::trigger  FEEDER_EVENT_STATE_WAITING:  FINISHED!" << '\n';
					// ready to be reactivated
					m_eState = FEEDER_EVENT_STATE_INIT;
					runtimeInit();
					informListeners(LISTENER_GROUP_FINISHED, 0);
				}
			}
		}
		break;
		case FEEDER_EVENT_STATE_ZOMBIE:
		{
		}
	}
}

bool FeederEvent::isMyBlock(Event* p0TriggeringEvent) noexcept
{
	assert(p0TriggeringEvent != nullptr);
	const auto& aBlockEvents = m_p0CurBlocks->m_aBlockEvents;
	for (BlockEvent* p0BEv : aBlockEvents) {
		assert(p0BEv != nullptr);
		if (p0TriggeringEvent == p0BEv) {
			return true; //-----------------------------------------------------
		}
	}
	return false;
}
bool FeederEvent::isMyBlock(int32_t nLevelBlockId) noexcept
{
	const auto& aBlockEvents = m_p0CurBlocks->m_aBlockEvents;
	for (BlockEvent* p0BEv : aBlockEvents) {
		assert(p0BEv != nullptr);
		if (nLevelBlockId == p0BEv->blockGetId()) {
			return true; //-----------------------------------------------------
		}
	}
	return false;
}

void FeederEvent::calcNext() noexcept
{
	int32_t nNextBunch = -1;
	int32_t nCantBeBunch = -1;
	const int32_t nTotRandomParts = m_oRandom.getTotRandomParts();
	if (nTotRandomParts > 2) {
		if ((! m_bAllowBlockBunchRepetition) && (m_nPrevBunch >= 0)) {
			nCantBeBunch = m_nPrevBunch;
		}
	} else if (nTotRandomParts == 2) {
		if ((! m_bAllowBlockBunchRepetition) && (m_nPrevBunch >= 0)) {
			nNextBunch = 1 - m_nPrevBunch;
		}
	} else {
		assert(nTotRandomParts == 1);
		nNextBunch = 0;
	}
	if (nNextBunch < 0) {
		int32_t nRandomRange = m_oRandom.getRandomRange();
		int32_t nNextRand;
		int32_t nPrecProb;
		int32_t nProb = 0;
		bool bCantBeBunch = (nCantBeBunch >= 0);
		if (bCantBeBunch) {
			m_oRandom.getRandomPart(nCantBeBunch, nProb, nPrecProb);
			nRandomRange -= nProb;
		}
		if (m_aRandomStack.isEmpty()) {
//std::cout << "FeederEvent::calcNext() m_aRandomStack.isEmpty()" << '\n';
			nNextRand = level().game().random(0, nRandomRange - 1);
		} else {
			const auto oRandPair = m_aRandomStack.read();
			const int32_t nValue = oRandPair.second;
			const RANDOM_TYPE eRT = oRandPair.first;
			if (eRT == RANDOM_TYPE_INT32) {
				const int64_t n64Value = static_cast<int64_t>(nValue) - static_cast<int64_t>(std::numeric_limits<int32_t>::min());
				const double f01Value = 1.0 * n64Value / (static_cast<int64_t>(1) << 32);
				assert((f01Value >= 0.0) && (f01Value < 1.0));
				nNextRand = static_cast<int32_t>(f01Value * nRandomRange);
			} else if (eRT == RANDOM_TYPE_WEIGHT) {
				nNextRand = nValue;
				bCantBeBunch = false;
			} else {
				assert(eRT == RANDOM_TYPE_BUNCH_IDX);
				nNextRand = - (nValue + 1);
				assert(nNextRand < 0);
				bCantBeBunch = false;
			}
//std::cout << "FeederEvent::calcNext() nValue=" << nValue << '\n';
		}
		if (bCantBeBunch) {
			const int32_t nRestNextRand = nNextRand;
			nNextRand = nRestNextRand + ((nRestNextRand >= nPrecProb) ? nProb : 0);
		}
		if (nNextRand < 0) {
			m_p0NextBlocks = &(m_oRandom.getRandomPart(- (nNextRand + 1), nNextBunch));
		} else {
			m_p0NextBlocks = &(m_oRandom.getRandomPartProb(nNextRand, nNextBunch));
		}

	} else {
		m_p0NextBlocks = &(m_oRandom.getRandomPart(nNextBunch));
	}
	m_nPrevBunch = nNextBunch;
}
void FeederEvent::showNextInPreview() noexcept
{
	if ((!m_bShowPreview) || (!m_refPreviewWidget)) {
		return;
	}
	m_refPreviewWidget->set((m_p0NextBlocks->m_bJoined ? s_sJoin : Util::s_sEmptyString), m_p0NextBlocks->m_aInitialBlocks);
}

} // namespace stmg

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
 * File:   blockevent.cc
 */

#include "blockevent.h"

#include <stmm-games/keyactionevent.h>
#include <stmm-games/util/util.h>
#include <stmm-games/appconfig.h>
#include <stmm-games/utile/tilerect.h>
#include <stmm-games/utile/tileselector.h>

#include <cassert>
#include <limits>
#include <algorithm>
#include <iostream>

namespace stmg
{

const std::string BlockEvent::s_sKeyActionRotate = "BlockEvent:Rotate";
const std::string BlockEvent::s_sKeyActionLeft = "BlockEvent:Left";
const std::string BlockEvent::s_sKeyActionRight = "BlockEvent:Right";
const std::string BlockEvent::s_sKeyActionDown = "BlockEvent:Down";
const std::string BlockEvent::s_sKeyActionDrop = "BlockEvent:Drop";
const std::string BlockEvent::s_sKeyActionInvertRotation = "BlockEvent:InvRotation";
const std::string BlockEvent::s_sKeyActionNext = "BlockEvent:Next";

const std::string BlockEvent::s_sGameOptionDelayedFreeze = "DelayedFreeze";
const std::string BlockEvent::s_sPlayerOptionClockwiseRotation = "ClockwiseRotation";

static constexpr const int32_t s_nZObjectZBlock = 2000;
static constexpr const int32_t s_nZObjectZBombExplosion = 10000;

static constexpr const int32_t s_nExplosionDurationMilllisec = 1000;

static constexpr const int32_t s_nKeysBufferSize = 10;

Recycler<BlockEvent::PrivateExplosionAnimation, ExplosionAnimation> BlockEvent::s_oBoardExplosionRecycler{};

BlockEvent::BlockEvent(Init&& oInit) noexcept
: Event(std::move(oInit))
, LevelBlock(true)
, m_oData(std::move(oInit))
, m_oKeys(s_nKeysBufferSize)
{
	initCommon();
}
void BlockEvent::reInit(Init&& oInit) noexcept
{
	Event::reInit(std::move(oInit));
	m_oData = std::move(oInit);
	initCommon();
}
void BlockEvent::initCommon() noexcept
{
	Level& oLevel = level();
	AppConfig& oAppConfig = *oLevel.prefs().getAppConfig();
	m_nKeyActionRotate = oAppConfig.getKeyActionId(s_sKeyActionRotate);
	m_nKeyActionLeft = oAppConfig.getKeyActionId(s_sKeyActionLeft);
	m_nKeyActionRight = oAppConfig.getKeyActionId(s_sKeyActionRight);
	m_nKeyActionDown = oAppConfig.getKeyActionId(s_sKeyActionDown);
	m_nKeyActionDrop = oAppConfig.getKeyActionId(s_sKeyActionDrop);
	m_nKeyActionInvertRotation = oAppConfig.getKeyActionId(s_sKeyActionInvertRotation);
	m_nKeyActionNext = oAppConfig.getKeyActionId(s_sKeyActionNext);

	assert(!m_oData.m_oBlock.isEmpty());
	assert(m_oData.m_oBlock.shapeRemoveAllInvisible() == 0);

	assert(m_oData.m_nSkippedFalls >= -1);

	if (!m_oData.m_refBomb) {
		m_oData.m_refBomb = std::make_unique<TileSelector>();
	}

	m_nBoardWidth = oLevel.boardWidth();
	m_nBoardHeight = oLevel.boardHeight();

	const auto oVal = oLevel.prefs().getOptionValue(s_sGameOptionDelayedFreeze);
	if (oVal.isNull()) {
		m_bDelayedFreeze = true;
	} else {
		assert(oVal.getType() == Variant::TYPE_BOOL);
		m_bDelayedFreeze = oVal.getBool();
	}

	initRuntime();
}
void BlockEvent::initRuntime() noexcept
{
	m_nLastLevelPlayer = -1;
	m_eState = BLOCK_EVENT_STATE_ACTIVATE;
	m_nPlaceTryMillisec = 0;
	m_oKeys.clear();
	m_nLastFallTick = std::numeric_limits<int32_t>::min();
	m_nTouchWait = 0;
	m_nSkippedFallCount = 0;
	m_bPushed = false;
	m_bCannotBeHarmed = false;
	m_bClockwiseRotation = false;
	blockInitialSet(m_oData.m_oBlock, m_oData.m_nInitShape, m_oData.m_oInitPos, m_oData.m_bControllable, m_oData.m_nLevelTeam);
}
void BlockEvent::regenerate() noexcept
{
	initRuntime();
}
	//void BlockEvent::validateBlock()
	//{
	////std::cout << "BlockEvent(" << blockGetId() << ")::validateBlock  "; m_oInitBlock.dump();
	//	if (m_oInitBlock.isEmpty()) {
	//		m_nInitShape = 0;
	//		return; //--------------------------------------------------------------
	//	}
	//	m_oInitBlock.shapeRemoveAllInvisible();
	//	if (!m_oInitBlock.isShapeId(m_nInitShape)) {
	//		// shape invalid
	//		if (m_oInitBlock.isEmpty()) {
	//			m_nInitShape = 0;
	//		} else {
	//			// take the a valid shape
	//			m_nInitShape = m_oInitBlock.shapeIds()[0];
	//		}
	//	}
	//	assert(validBlock());
	//}
bool BlockEvent::validBlock() const noexcept
{
	return !m_oData.m_oBlock.isEmpty();
}
NPoint BlockEvent::calcInitialPos(Level* p0Level, const Block& oBlock, int32_t nInitPos, int32_t nInitShapeId) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::calcInitialPos" << '\n';

	//|        ......             |
	//|---------------------------|
	//|        ....*.             |
	//|        ....*.             |
	//|        ....**             |
	//|                           |
	//|                           |
	//|                           |
	//|                           |

	//TODO make test
	// center:
	// nInitPos = 0

	// m_nBoardWidth  = 27
	// m_nBoardHeight = ???
	//
	// m_nCurShapeW = 2
	// m_nCurShapeH = 3
	//
	// m_nCurShapeMinX = 4
	// m_nCurShapeMinY = 1
	//
	// =>  nPosX = 11
	// =>  nPosY = -1

	const int32_t nShapeW = oBlock.shapeWidth(nInitShapeId);
	const int32_t nBoardWidth = p0Level->boardWidth();
	//const int32_t nBoardHeight = p0Level->boardHeight();

	int32_t nPosX = (nBoardWidth - nShapeW) / 2 + nInitPos;
	int32_t nPosY = - oBlock.shapeMinY(nInitShapeId);

	// nPosX is virtual at this moment
	// need to check it's not outside the board
	if (nPosX < 0)
	{
		nPosX = 0;
	}
	else if (nPosX + nShapeW > nBoardWidth)
	{
		nPosX = nBoardWidth - nShapeW;
	}

	// now do the correction
	nPosX -= oBlock.shapeMinX(nInitShapeId);
	return NPoint{nPosX, nPosY};
}
int32_t BlockEvent::blockPosZ() const noexcept
{
	return s_nZObjectZBlock;
}

void BlockEvent::trigger(int32_t /*nMsg*/, int32_t /*nValue*/, Event* p0TriggeringEvent) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::trigger" << '\n';
	//TODO
	// ACTIVATE activate event
	// INIT     calc initial position, reset attempts to place it
	// PLACE    try to place block on board, game over if fail
	//          if succeeded deactivate and add to level's falling blocks
	// FALL     expect and do nothing while falling
	//            (zombie state, cannot fall more than once without being reinitialized)
	Level& oLevel = level();
	auto& oGame = oLevel.game();
	const int32_t nGameTick = oGame.gameElapsed();
	switch (m_eState)
	{
		case BLOCK_EVENT_STATE_ACTIVATE:
		{
			if (!validBlock()) {
				m_eState = BLOCK_EVENT_STATE_ZOMBIE;
				break;
			}
			m_eState = BLOCK_EVENT_STATE_INIT;
			if (p0TriggeringEvent != nullptr) {
				oLevel.activateEvent(this, nGameTick);
				break;
			}
		} // fallthrough
		case BLOCK_EVENT_STATE_INIT:
		{
			if (p0TriggeringEvent != nullptr) {
				return; //------------------------------------------------------
			}
			m_eState = BLOCK_EVENT_STATE_PLACE;
			//calcInitialPos();
			m_nPlaceTryMillisec = 0;
		} // fallthrough
		case BLOCK_EVENT_STATE_PLACE:
		{
			if (p0TriggeringEvent != nullptr) {
				return; //------------------------------------------------------
			}
			if (!oLevel.blockAdd(this, LevelBlock::MGMT_TYPE_AUTO_STRICT_OWNER)) {
//std::cout << "BlockEvent(" << blockGetId() << ")::trigger   blockAdd failed" << '\n';
				const int32_t nGameInterval = oGame.gameInterval();
				m_nPlaceTryMillisec += nGameInterval;
				if (m_nPlaceTryMillisec >= m_oData.m_nPlacingMillisec) {
					m_nPlaceTryMillisec = 0;
					m_eState = BLOCK_EVENT_STATE_ZOMBIE;
					informListeners(LISTENER_GROUP_CANNOT_PLACE, 0);
					informListeners(LISTENER_GROUP_FINISHED, 0);
				} else {
					oLevel.activateEvent(this, nGameTick + 1);
				}
			} else {
				m_oKeys.clear();
				m_eState = BLOCK_EVENT_STATE_FALL;
				oLevel.boardAddListener(this);
				oLevel.deactivateEvent(this);
				informListeners(LISTENER_GROUP_COULD_PLACE, 0);
			}
		}
		break;
		case BLOCK_EVENT_STATE_FALL:
		{
			if (p0TriggeringEvent != nullptr) {
				return; //------------------------------------------------------
			}
		}
		break;
		case BLOCK_EVENT_STATE_ZOMBIE:
		{
		}
		break;
	}
}

const Block& BlockEvent::getInitialBlock() noexcept
{
	return m_oData.m_oBlock;
}
void BlockEvent::handleKeyActionInput(const shared_ptr<KeyActionEvent>& refEvent) noexcept
{
//std::cout << "BlockEvent::handleKeyActionInput" << '\n';
	KeyActionEvent* p0AEv = refEvent.get();
	const stmi::Event::AS_KEY_INPUT_TYPE eType = p0AEv->getType();
	if (eType != stmi::Event::AS_KEY_PRESS) {
//std::cout << "BlockEvent::handleKeyActionInput not press" << '\n';
		return; //--------------------------------------------------------------
	}
	const int32_t nKeyAction = p0AEv->getKeyAction();
	if (nKeyAction == m_nKeyActionNext) {
		level().blockCycleControl(this);
		return; //--------------------------------------------------------------
	}
	if (m_oKeys.isFull()) {
//std::cout << "BlockEvent::handleKeyActionInput m_oKeys.isFull()" << '\n';
		return; //--------------------------------------------------------------
	}
	m_oKeys.write(nKeyAction);
}
void BlockEvent::doKeyActionDown() noexcept
{
	Level& oLevel = level();
	const int32_t nGameTick = oLevel.game().gameElapsed();
	bool bFused = false;
	bool bMutilated = false;
	bool bDestroyed = false;
	if (move(Direction::DOWN, bFused, bMutilated, bDestroyed)) {
		m_nLastFallTick = nGameTick;
		if (bDestroyed) {
			return; //----------------------------------------------------------
		}
	} else {
		if (! bDestroyed) {
			#ifndef NDEBUG
			const bool bFroze =
			#endif //NDEBUG
			privateFreeze();
			assert(bFroze);
		}
		return; //--------------------------------------------------------------
	}
	m_nTouchWait = 0;
}
void BlockEvent::doKeyActionRotate() noexcept
{
	int32_t nDeltaX;
	int32_t nDeltaY;
	if (canPlaceOnBoardRotate(nDeltaX, nDeltaY)) {
		const int32_t nNextShapeId = incShape(blockGetShapeId());
		blockMoveRotate(nNextShapeId, nDeltaX, nDeltaY);
	}
}
void BlockEvent::doKeyActionDrop() noexcept
{
	Level& oLevel = level();
	const int32_t nGameTick = oLevel.game().gameElapsed();
	int32_t nDeltaDrop = 0;
	bool bItsPossible = true;
	bool bFused = false;
	bool bMutilated = false;
	bool bDestroyed = false;
	do {
		if (move(Direction::DOWN, bFused, bMutilated, bDestroyed)) {
			if (bFused || bMutilated || bDestroyed) {
				// it fused with another LevelBlock: stop the drop
				bItsPossible = false;
				m_nTouchWait = 0;
			} else {
				// keep free falling
				++nDeltaDrop;
			}
		} else {
			bItsPossible = false;
			if (bFused || bDestroyed) {
				// no delayed freeze if fused (or destroyed)
				m_nTouchWait = 0;
			} else if (m_bDelayedFreeze) {
				// delayed freeze
				m_nTouchWait = oLevel.getFallEachTicks();
			} else {
				m_nTouchWait = 0;
			}
		}
		if (bMutilated) {
			break; // do ----------------
		}
	} while (bItsPossible);
	if ((bFused || bDestroyed || bMutilated)) {
		if (bDestroyed) {
			return; //----------------------------------------------------------
		}
	} else {
		if ((!m_bDelayedFreeze) || (nDeltaDrop == 0)) {
			// has to go down at least once to avoid freeze
			#ifndef NDEBUG
			const bool bFroze =
			#endif //NDEBUG
			privateFreeze();
			assert(bFroze);
			m_nLastFallTick = nGameTick;
			return; //----------------------------------------------------------
		}
	}
}
void BlockEvent::handleTimer() noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::handleTimer(" << level().game().gameElapsed() << ")" << '\n';
	while (! m_oKeys.isEmpty()) {
		const int32_t nKeyAction = m_oKeys.read();
		if (nKeyAction == m_nKeyActionLeft)	{
			move(Direction::LEFT);
		} else if (nKeyAction == m_nKeyActionRight) {
			move(Direction::RIGHT);
		} else if (nKeyAction == m_nKeyActionRotate) {
			doKeyActionRotate();
		} else if (nKeyAction == m_nKeyActionDown) {
			doKeyActionDown();
		} else if (nKeyAction == m_nKeyActionDrop) {
			if (m_oData.m_bCanDrop) {
				doKeyActionDrop();
			}
		} else if (nKeyAction == m_nKeyActionInvertRotation) {
			m_bClockwiseRotation = !m_bClockwiseRotation;
		}
	}
}
bool BlockEvent::canMove(Direction::VALUE eDir) noexcept
{
	bool bFused;
	bool bMutilated;
	bool bDestroyed;
	return move(eDir, true, false, 0,0,0,0, 0,0, bFused, bMutilated, bDestroyed);
}
bool BlockEvent::canMove(Direction::VALUE eDir, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH
						, int32_t nDeltaExploX, int32_t nDeltaExploY) noexcept
{
	bool bFused;
	bool bMutilated;
	bool bDestroyed;
	return move(eDir, true, false, nClipX, nClipY, nClipW, nClipH
				, nDeltaExploX, nDeltaExploY, bFused, bMutilated, bDestroyed);
}
bool BlockEvent::move(Direction::VALUE eDir, bool& bFused, bool& bWasMutilated, bool& bDestroyed) noexcept
{
	return move(eDir, true, true, 0,0,0,0, 0,0, bFused, bWasMutilated, bDestroyed);
}
bool BlockEvent::move(Direction::VALUE eDir) noexcept
{
	bool bHasFused;
	bool bWasMutilated;
	bool bWasDestroyed;
	return move(eDir, true, true, 0,0,0,0, 0,0, bHasFused, bWasMutilated, bWasDestroyed);
}
bool BlockEvent::move(Direction::VALUE eDir, bool bHandleBomb, bool bDoMove
		, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH
		, int32_t nDeltaExploX, int32_t nDeltaExploY
		, bool& bHasFused, bool& bWasMutilated, bool& bWasDestroyed) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::move() bDoMove=" << bDoMove << "  eDir=" << (int32_t)eDir << '\n';
	assert((static_cast<int32_t>(eDir) >= 0) && (static_cast<int32_t>(eDir) < 4));
	Level& oLevel = level();

	bHasFused = false;
	bWasMutilated = false;
	bWasDestroyed = false;
	bool bPushBomb;
	bool bInteracted;
	do {
		const NPoint oLBPos = LevelBlock::blockPos();
		//const int32_t nShapeId = blockGetShapeId(); //m_nShape;
		const std::vector< Block::Contact >& aContacts = blockContacts(eDir);
		bPushBomb = false;
		bInteracted = false;
		bool bMayAttack = false;
		bool bMayFuse = false;
		for (auto& oContactBrickPos : aContacts) {
			const int32_t nRelX = oContactBrickPos.m_nRelX;
			const int32_t nRelY = oContactBrickPos.m_nRelY;
			const int32_t nContactBrickId = oContactBrickPos.m_nBrickId;
			const int32_t nBoardX = oLBPos.m_nX + nRelX;
			const int32_t nBoardY = oLBPos.m_nY + nRelY;
			if ((nClipW == 0) || (nClipH == 0)
					|| ((nBoardX >= nClipX) && (nBoardX < nClipX + nClipW)
						&& (nBoardY >= nClipY) && (nBoardY < nClipY + nClipH)))
			{
				if ((nBoardX < 0) || (nBoardX >= m_nBoardWidth)
						|| (nBoardY >= m_nBoardHeight) || (nBoardY < 0)) {
					// cannot move
					return false; //--------------------------------------------
				}
				const Tile& oBrick = blockBrickTile(nContactBrickId);
//#ifndef NDEBUG
//std::cout << "++++++++ Brick tile: ";
//oBrick.dump();
//if (oBrick.getTileChar().isCharIndex()) {
//std::cout << "  + charName: " << oLevel.getNamed().chars().getName(oBrick.getTileChar().getCharIndex());
//}
//std::cout << '\n';
//#endif //NDEBUG
				const Tile& oTile = oLevel.boardGetTile(nBoardX, nBoardY);
//#ifndef NDEBUG
//if (!oTile.isEmpty()) {
//std::cout << "         Board tile: ";
//oTile.dump();
//std::cout << '\n';
//if (oBrick.getTileChar().isCharIndex()) {
//std::cout << "";
//}
//}
//#endif //NDEBUG
				if (!oTile.isEmpty()) {
//std::cout << "BlockEvent(" << blockGetId() << ")::move()  m_refBomb->select(oTile)=" << m_refBomb->select(oTile) << " m_refBomb->select(oBrick)=" << m_refBomb->select(oBrick) << '\n';
//m_refBomb->dump(5);
					if (m_oData.m_refBomb->select(oTile) || m_oData.m_refBomb->select(oBrick)) {
						// may move after an explosion
						bPushBomb = true;
//std::cout << "BlockEvent(" << blockGetId() << ")::move() nBoardX=" << nBoardX << " nBoardY=" << nBoardY << " - may push bomb" << '\n';
					} else {
						// solid tile: cannot move
//std::cout << "BlockEvent(" << blockGetId() << ")::move() nBoardX=" << nBoardX << " nBoardY=" << nBoardY << " - solid tile, cannot move" << '\n';
//oTile.dump();
//oBrick.dump();
						return false; // ---------------------------------------
					}
				} else {
					LevelBlock* p0AttackedBlock = oLevel.boardGetOwner(nBoardX, nBoardY);
					if (p0AttackedBlock != nullptr) {
//if (p0LevelBlock == this) {
//oLevel.dump(true,true,false,false,false,false);
//}
						assert(p0AttackedBlock != this);
						LevelBlock::QUERY_ATTACK_TYPE eAttackType = p0AttackedBlock->queryAttack(*this, nBoardX, nBoardY, oBrick);
						if (eAttackType == LevelBlock::QUERY_ATTACK_TYPE_FUSE_TO_ATTACKER) {
							bMayFuse = true;
						} else {
							bMayAttack = true;
						}
					}
				}
			}
		}
		if (bMayFuse) {
			for (auto& oContactBrickPos : aContacts) {
				const int32_t nRelX = oContactBrickPos.m_nRelX;
				const int32_t nRelY = oContactBrickPos.m_nRelY;
				const int32_t nContactBrickId = oContactBrickPos.m_nBrickId;
				const NPoint oLBPos = LevelBlock::blockPos();
				const int32_t nBoardX = oLBPos.m_nX + nRelX;
				const int32_t nBoardY = oLBPos.m_nY + nRelY;
				if ((nClipW == 0) || (nClipH == 0)
						|| ((nBoardX >= nClipX) && (nBoardX < nClipX + nClipW)
							&& (nBoardY >= nClipY) && (nBoardY < nClipY + nClipH)))
				{
					assert(!( ((nBoardX < 0) || (nBoardX >= m_nBoardWidth)
							|| (nBoardY >= m_nBoardHeight) || (nBoardY < 0)) ));
					const Tile& oTile = oLevel.boardGetTile(nBoardX, nBoardY);
					if (oTile.isEmpty()) {
						LevelBlock* p0AttackedBlock = oLevel.boardGetOwner(nBoardX, nBoardY);
						if (p0AttackedBlock != nullptr) {
							assert(p0AttackedBlock != this);
							// there's another LevelBlock preventing (this block) to move:
							// fuse with it
							const Tile& oBrick = blockBrickTile(nContactBrickId);
							LevelBlock::QUERY_ATTACK_TYPE eAttackType = p0AttackedBlock->queryAttack(*this, nBoardX, nBoardY, oBrick);
							if (eAttackType == LevelBlock::QUERY_ATTACK_TYPE_FUSE_TO_ATTACKER) {
								m_bCannotBeHarmed = true;
								#ifndef NDEBUG
								const bool bPositionFreed = 
								#endif //NDEBUG
								p0AttackedBlock->attack(*this, nBoardX, nBoardY, oBrick);
								m_bCannotBeHarmed = false;
								assert(bPositionFreed);
								bInteracted = true;
								bHasFused = true;
								// and try again to move
								break; //for loop
							}
						}
					}
				}
			}
		} else if (bMayAttack) {
			for (auto& oContactBrickPos : aContacts) {
				const int32_t nRelX = oContactBrickPos.m_nRelX;
				const int32_t nRelY = oContactBrickPos.m_nRelY;
				const int32_t nContactBrickId = oContactBrickPos.m_nBrickId;
				const NPoint oLBPos = LevelBlock::blockPos();
				const int32_t nBoardX = oLBPos.m_nX + nRelX;
				const int32_t nBoardY = oLBPos.m_nY + nRelY;
				if ((nClipW == 0) || (nClipH == 0)
						|| ((nBoardX >= nClipX) && (nBoardX < nClipX + nClipW)
							&& (nBoardY >= nClipY) && (nBoardY < nClipY + nClipH)))
				{
					const Tile& oTile = oLevel.boardGetTile(nBoardX, nBoardY);
					if (!oTile.isEmpty()) {
						assert(bPushBomb);
					} else {
						LevelBlock* p0AttackedBlock = oLevel.boardGetOwner(nBoardX, nBoardY);
						if (p0AttackedBlock != nullptr) {
							assert(p0AttackedBlock != this);
							// there's another LevelBlock preventing (this block) to move:
							// attack it
							const Tile& oBrick = blockBrickTile(nContactBrickId);
							m_bCannotBeHarmed = true;
							const bool bPositionFreed = p0AttackedBlock->attack(*this, nBoardX, nBoardY, oBrick);
							m_bCannotBeHarmed = false;
							if (!bPositionFreed) {
								return false; // -------------------------------
							}
							#ifndef NDEBUG
							LevelBlock* p0RetryAttackedBlock =
							#endif //NDEBUG
							oLevel.boardGetOwner(nBoardX, nBoardY);
							assert((p0RetryAttackedBlock == nullptr) || (p0RetryAttackedBlock == this));
							bInteracted = true;
							// and try again to move
							break; //for loop
						}
					}
				}
			}
		}
	} while (bInteracted);

	if (bPushBomb) {
//std::cout << "BlockEvent(" << blockGetId() << ")::move() PushBomb" << '\n';
		if (!bHandleBomb) {
			return false; //----------------------------------------------------
		}
		// bombs are involved in block-tiles pushing board-tiles, or viceversa
		bool bMutilate;
		do {
			m_aTempContacts = blockContacts(eDir);
			bMutilate = false;
			for (auto& oContactBrickPos : m_aTempContacts) {
				const int32_t nRelX = oContactBrickPos.m_nRelX;
				const int32_t nRelY = oContactBrickPos.m_nRelY;
				const int32_t nContactBrickId = oContactBrickPos.m_nBrickId;
				const NPoint oLBPos = LevelBlock::blockPos();
				const int32_t nBoardX = oLBPos.m_nX + nRelX;
				const int32_t nBoardY = oLBPos.m_nY + nRelY;
				if ((nBoardX >= 0) && (nBoardX < m_nBoardWidth)
						&& (nBoardY < m_nBoardHeight) && (nBoardY >= 0)) {
					const Tile& oBrick = blockBrickTile(nContactBrickId);
					const Tile& oTile = oLevel.boardGetTile(nBoardX, nBoardY);
					if (!oTile.isEmpty()) {
						const bool bTileIsBomb = m_oData.m_refBomb->select(oTile);
						bool bOneIsBomb = bTileIsBomb;
						if (!bTileIsBomb) {
							bOneIsBomb = m_oData.m_refBomb->select(oBrick);
						}
						if (bOneIsBomb) {
//std::cout << "BlockEvent(" << blockGetId() << ")::move() bOneIsBomb nBoardX=" << nBoardX << "  nBoardY=" << nBoardY << '\n';
							bMutilate = true;
							bWasMutilated = true;
							#ifndef NDEBUG
							const bool bCouldDestroy =
							#endif //NDEBUG
							privateDestroyBrick(nContactBrickId, bWasDestroyed);
							assert(bCouldDestroy);
							shared_ptr<Coords> refCoords;
							getInitializedCoords(refCoords, nBoardX, nBoardY);
							//if (!bTileIsBomb) {
							oLevel.boardDestroy(*refCoords);
							//} else {
							//	oLevel.boardModify(nBoardX, nBoardY, 1, 1, RefC<TileRect>());
							//}

							if (m_oData.m_bEmitDestroyedBricks) {
								m_aTempDestroyedBricks.push_back(Util::packPointToInt32(NPoint{nBoardX + nDeltaExploX, nBoardY + nDeltaExploY}));
							}
							if (m_oData.m_bCreateDestroyBrickAnimation) {
								const int32_t nPlayer = getPlayer();
								shared_ptr<ExplosionAnimation> refBoardExplosion;
								ExplosionAnimation::Init oExplosionInit;
								oExplosionInit.m_fDuration = s_nExplosionDurationMilllisec;
								oExplosionInit.m_oPos = {0.0 + nBoardX + nDeltaExploX, 0.0 + nBoardY + nDeltaExploY};
								oExplosionInit.m_oSize = {1.0, 1.0};
								oExplosionInit.m_nZ = s_nZObjectZBombExplosion;
								oExplosionInit.m_oTile = (bTileIsBomb ? oTile : oBrick);
								oExplosionInit.m_nLevelPlayer = (bTileIsBomb ? 0 : nPlayer);
								s_oBoardExplosionRecycler.create(refBoardExplosion, std::move(oExplosionInit));
								oLevel.animationAddScrolled(refBoardExplosion, 0.0);
							}
							break; // for loop
						}
					}
				}
			}
		} while (bMutilate && !bWasDestroyed);
//std::cout << "BlockEvent(" << blockGetId() << ")::move() exit bMutilate=" << bMutilate << "  bWasDestroyed=" << bWasDestroyed << '\n';
	}
	if (bDoMove) {
		const int32_t nDx = Direction::deltaX(eDir);
		const int32_t nDy = Direction::deltaY(eDir);
//std::cout << "BlockEvent(" << blockGetId() << ")::move() bDoMove nDx=" << nDx << "  nDy=" << nDy << '\n';
		blockMove(nDx, nDy);
	}
	if (m_oData.m_bEmitDestroyedBricks && ! m_aTempDestroyedBricks.empty()) {
		for (const int32_t& nPackedXY : m_aTempDestroyedBricks) {
			informListeners(LISTENER_GROUP_BRICK_DESTROYED, nPackedXY);
		}
		m_aTempDestroyedBricks.clear();
	}
	return true;
}
void BlockEvent::getInitializedCoords(shared_ptr<Coords>& refCoords, int32_t nBoardX, int32_t nBoardY) noexcept
{
	if (!m_refCoords) {
		refCoords = std::make_shared<Coords>();
		m_refCoords = refCoords;
	} else if (m_refCoords.use_count() > 1) {
		std::cout << "Warning: BlockEvent::getInitialitedCoords() not freed by BoardListener"<< '\n';
		refCoords = std::make_shared<Coords>();
	} else {
		refCoords = m_refCoords;
		refCoords->reInit();
	}
	refCoords->add(nBoardX, nBoardY);
}
bool BlockEvent::canPlaceOnBoardRotate(int32_t& nDeltaX, int32_t& nDeltaY) const noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::canPlaceOnBoardRotate()" << '\n';
	const Level& oLevel = level();
	const Block& oBlock = blockGet();
	const int32_t nTotBricks = oBlock.totBricks();

	const int32_t nLargestShapeWidth = oBlock.maxWidth();
	const int32_t nMaxLateralMove = std::max<int32_t>(nLargestShapeWidth - 1, static_cast<int32_t>(1));

	const int32_t nHighestShapeHeight = oBlock.maxHeight();
	const int32_t nMaxDeepMove = std::max<int32_t>(nHighestShapeHeight - 1, static_cast<int32_t>(1));

	const int32_t nShape = incShape(blockGetShapeId());
	const NPoint oLBPos = LevelBlock::blockPos();
	const int32_t nPosX = oLBPos.m_nX;
	const int32_t nPosY = oLBPos.m_nY;

	const auto& aBrickId = oBlock.brickIds();
	assert(static_cast<int32_t>(aBrickId.size()) == nTotBricks);

	int32_t nDeepMove = 0;
	while (nDeepMove < nMaxDeepMove)
	{
		nDeltaY = nDeepMove;

		int32_t nMove = 0;
		while (nMove < nMaxLateralMove * 2 + 1)
		{
			const int32_t nMoveAbs = (nMove + 1) / 2;
			const int32_t nMoveSign = (nMove % 2) * 2 - 1;
			nDeltaX = nMoveAbs * nMoveSign;

			bool bOk = true;

			int32_t nBrickIdx = 0;
			while (bOk && (nBrickIdx < nTotBricks))
			{
				const int32_t nBrickId = aBrickId[nBrickIdx];
				const int32_t nBoardX = nPosX + nDeltaX + oBlock.shapeBrickPosX(nShape, nBrickId);
				const int32_t nBoardY = nPosY + nDeltaY + oBlock.shapeBrickPosY(nShape, nBrickId);
				const bool bVisible = oBlock.shapeBrickVisible(nShape, nBrickId);

				if (bVisible) {
					if ((nBoardX < 0) || (nBoardY < 0) || (nBoardX >= m_nBoardWidth)
							|| (nBoardY >= m_nBoardHeight)) {
						bOk = false;
					} else {
						if (!oLevel.boardGetTile(nBoardX, nBoardY).isEmpty()) {
							bOk = false;
						} else {
							LevelBlock* p0LevelBlock = oLevel.boardGetOwner(nBoardX, nBoardY);
							if ((p0LevelBlock != nullptr) && (p0LevelBlock != this)) {
								bOk = false;
							}
						}
					}
				}
				++nBrickIdx;
			}
			if (bOk)
			{
				// found a (deltaX,deltaY) that allows the next shape
				// to be put on board (that is to be rotated)
				return true;
			}
			++nMove;
		}
		++nDeepMove;
	}
	return false;
}

int32_t BlockEvent::incShape(int32_t nShape) const noexcept
{
	const Block& oBlock = blockGet();
	if (m_bClockwiseRotation) {
		nShape = oBlock.shapeNext(nShape);
		if (nShape < 0) {
			nShape = oBlock.shapeFirst();
		}
	} else {
		nShape = oBlock.shapePrec(nShape);
		if (nShape < 0) {
			nShape = oBlock.shapeLast();
		}
	}
	return nShape;
}

void BlockEvent::fall() noexcept
{
	const int32_t nTotSkippedFalls = m_oData.m_nSkippedFalls;
	if (nTotSkippedFalls != 0) {
		if (nTotSkippedFalls < 0) {
			return; //----------------------------------------------------------
		} else {
			if (m_nSkippedFallCount < nTotSkippedFalls) {
				++m_nSkippedFallCount;
				return; //------------------------------------------------------
			}
			m_nSkippedFallCount = 0;
		}
	}

	const int32_t nGameTick = level().game().gameElapsed();

	if (move(Direction::DOWN)) {
		m_nLastFallTick = nGameTick;
		m_nTouchWait = 0;
	} else if (m_nLastFallTick + m_nTouchWait < nGameTick) {
		#ifndef NDEBUG
		const bool bFroze =
		#endif //NDEBUG
		privateFreeze();
		assert(bFroze);
		m_nLastFallTick = nGameTick;
	}
}

void BlockEvent::onFusedWith(const LevelBlock& oLevelBlock) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::onFusedWith(" << level().game().gameElapsed() << ")"<< '\n';
	int32_t nControllerTeam = -1;
	bool bControllable = isPlayerControllable();
//std::cout << "              bControllable (before) =" << bControllable << '\n';
	if (bControllable) {
		nControllerTeam = getControllerTeam();
	} else {
		const bool bVictimControllable = oLevelBlock.isPlayerControllable();
//std::cout << "              bVictimControllable=" << bVictimControllable << '\n';
		if (bVictimControllable) {
			nControllerTeam = oLevelBlock.getControllerTeam();
			bControllable = true;
		}
	}
//std::cout << "              nControllerTeam=" << nControllerTeam << '\n';
	level().blockSetControllable(this, bControllable, nControllerTeam);
//std::cout << "              bControllable (after)=" << isPlayerControllable() << '\n';
	informListeners(LISTENER_GROUP_FUSED_WITH, oLevelBlock.blockGetId());
}
void BlockEvent::onPlayerChanged() noexcept
{
	const int32_t nLevelPlayer = getPlayer();
	if (nLevelPlayer < 0) {
		// not controlled
		return;
	}
	if (m_nLastLevelPlayer == nLevelPlayer) {
		return;
	}
	Level& oLevel = level();
	const int32_t nPlayer = oLevel.game().getPlayer(level().getLevel(), nLevelPlayer);
	const auto oVal = oLevel.prefs().getPlayer(nPlayer)->getOptionValue(s_sPlayerOptionClockwiseRotation);
	assert(oVal.getType() == Variant::TYPE_BOOL);
	m_bClockwiseRotation = oVal.getBool();
	m_nLastLevelPlayer = nLevelPlayer;
}
bool BlockEvent::remove() noexcept
{
	if (m_bCannotBeHarmed) {
		return false;
	}
	const bool bRemoved = LevelBlock::remove();
	if (bRemoved) {
		privateOnRemove([&](){
			informListeners(LISTENER_GROUP_REMOVED, 0);
		});
	}
	return bRemoved;
}
bool BlockEvent::destroy() noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::destroy()" << '\n';
	if (m_bCannotBeHarmed) {
		return false;
	}
	const bool bDestroyed = LevelBlock::destroy();
	if (bDestroyed) {
		privateOnRemove([&](){
			informListeners(LISTENER_GROUP_DESTROYED, 0);
		});
	}
	return bDestroyed;
}
bool BlockEvent::freeze() noexcept
{
	if (m_bCannotBeHarmed) {
		return false;
	}
	return privateFreeze();
}
bool BlockEvent::privateFreeze() noexcept
{
	m_oKeys.clear();
	const NPoint oLBPos = LevelBlock::blockPos();
	const int32_t nPosX = oLBPos.m_nX;
	const int32_t nPosY = oLBPos.m_nY;
	const int64_t nPosXY = Util::packPointToInt32(NPoint{nPosX, nPosY});
	m_nTouchWait = 0;
	const bool bFreezed = LevelBlock::freeze();
	if (bFreezed) {
		privateOnRemove([&](){
			informListeners(LISTENER_GROUP_FREEZED, nPosXY);
		});
	}
	return bFreezed;
}
bool BlockEvent::canFuseWith(LevelBlock& /*oLevelBlock*/) const noexcept
{
	return true;
}
bool BlockEvent::fuseTo(LevelBlock& oLevelBlock) noexcept
{
	if (m_bCannotBeHarmed) {
		return false;
	}
	const bool bFusedTo = LevelBlock::fuseTo(oLevelBlock);
	if (bFusedTo) {
		privateOnRemove([&](){
			informListeners(LISTENER_GROUP_FUSED_TO, oLevelBlock.blockGetId());
		});
	}
	return bFusedTo;
}
bool BlockEvent::removeBrick(int32_t nBrickId) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::removeBrick(" << nBrickId << ")" << '\n';
	if (m_bCannotBeHarmed) {
		return false;
	}
	if (blockBricksTotVisible() == 1) {
//std::cout << "BlockEvent(" << blockGetId() << ")::removeBrick(" << nBrickId << ") REMOVE BLOCK" << '\n';
		return remove();
	}
	const bool bBrickRemoved = LevelBlock::removeBrick(nBrickId);
	return bBrickRemoved;
}
bool BlockEvent::privateDestroyBrick(int32_t nBrickId, bool& bDestroyedLB) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::privateDestroyBrick()" << '\n';
	if (blockBricksTotVisible() == 1) {
		bDestroyedLB = destroy();
//std::cout << "BlockEvent(" << blockGetId() << ")::privateDestroyBrick(" << nBrickId << ") DESTROYED BLOCK =" << bDestroyedLB << '\n';
		return bDestroyedLB;
	}
	bDestroyedLB = false;
	const bool bBrickDestroyed = LevelBlock::destroyBrick(nBrickId);
	return bBrickDestroyed;
}
bool BlockEvent::destroyBrick(int32_t nBrickId) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::destroyBrick(" << nBrickId << ")" << '\n';
	if (m_bCannotBeHarmed) {
		return false;
	}
	if (blockBricksTotVisible() == 1) {
//std::cout << "BlockEvent(" << blockGetId() << ")::destroyBrick(" << nBrickId << ")  DESTROY BLOCK" << '\n';
		return destroy();
	}
	const bool bBrickDestroyed = LevelBlock::destroyBrick(nBrickId);
	return bBrickDestroyed;
}
LevelBlock::QUERY_ATTACK_TYPE BlockEvent::queryAttack(LevelBlock& /*oAttacker*/
													, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) const noexcept
{
	return LevelBlock::QUERY_ATTACK_TYPE_FUSE_TO_ATTACKER;
}
bool BlockEvent::attack(LevelBlock& oAttacker, int32_t /*nBoardX*/, int32_t /*nBoardY*/, const Tile& /*oTile*/) noexcept
{
//std::cout << "BlockEvent(" << blockGetId() << ")::attack(attackerId=" << oAttacker.blockGetId() << ", nBoardX=" << nBoardX << ", nBoardY=" << nBoardY << ")" << '\n';
	return fuseTo(oAttacker);
}

void BlockEvent::boabloPreFreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void BlockEvent::boabloPostFreeze(const Coords& /*oCoords*/) noexcept
{
}
void BlockEvent::boabloPreUnfreeze(const Coords& /*oCoords*/) noexcept
{
}
void BlockEvent::boabloPostUnfreeze(LevelBlock& /*oBlock*/) noexcept
{
}
void BlockEvent::boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& /*refTiles*/) noexcept
{
	// if block is moved outside board move in opposite dir or freeze!?
	assert(blockIsAutoOwner());
	const int32_t nDx = Direction::deltaX(eDir);
	const int32_t nDy = Direction::deltaY(eDir);
	if (!level().blockMoveIsWithinArea(*this, nDx, nDy, 0, 0, m_nBoardWidth, m_nBoardHeight)) {
		// block is scrolled outside board
		Direction::VALUE eOppDir = Direction::opposite(eDir);
		// try to move in opposite direction
		if (!move(eOppDir)) {
			// but there's something in the way
			#ifndef NDEBUG
			const bool bFroze =
			#endif //NDEBUG
			privateFreeze();
			assert(bFroze);
		}
	}
}
void BlockEvent::boardPostScroll(Direction::VALUE /*eDir*/) noexcept
{
	//boardPostInsert(eDir, 0, 0, m_nBoardW, m_nBoardH);
}
void BlockEvent::boardPreInsert(Direction::VALUE eDir, NRect oArea, const shared_ptr<TileRect>& refTiles) noexcept
{
	Level& oLevel = level();
	if (oArea.m_nY != 0) {
		oLevel.gameStatusTechnical(std::vector<std::string>{"BlockEvent::boardPreInsert", "Unsupported nY"});
		return; //--------------------------------------------------------------
	}
	if (eDir == Direction::DOWN) {
		assert((!refTiles) || (refTiles->getH() == 1));
		if ((refTiles) && !TileRect::isAllEmptyTiles(*refTiles)) {
			oLevel.gameStatusTechnical(std::vector<std::string>{"BlockEvent::boardPreInsert(DOWN)", "Non-empty tiles unsupported"});
			return; //----------------------------------------------------------
		}
		const int32_t nDelY = oArea.m_nY + oArea.m_nH - 1;
		m_bPushed = false;
		// Note: - if a block is destroyed it can move!
		//        - explosion animations are one below because the block doesn't
		//          really move up but the board comes down
		if ((nDelY > 0) && !canMove(Direction::UP, oArea.m_nX, 0, oArea.m_nW, nDelY, 0, +1)) {
			// Cannot move up: this means it is
			// pushed down by some board cell
			// if a bomb was involved, the board cell is already destroyed 
			// Let's see if the block can be pushed down
			if (!canMove(Direction::DOWN)) {
				// no, there's something in the way
				#ifndef NDEBUG
				const bool bFroze =
				#endif //NDEBUG
				privateFreeze();
				assert(bFroze);
				return; //------------------------------------------------------
			}
			m_bPushed = true;
		}
	} else if (eDir == Direction::UP) {
		assert((!refTiles) || (refTiles->getH() == 1));
		const int32_t nInsY = oArea.m_nY + oArea.m_nH - 1;
//std::cout << "BlockEvent(" << blockGetId() << ")::boardPreInsert(Up)" << '\n';
		// Note: - if a block is destroyed it can move!
		//        - explosion animations are one above because the block doesn't
		//          really move down but the board comes up
		if ((nInsY > 0) && !canMove(Direction::DOWN, oArea.m_nX, 1, oArea.m_nW, nInsY, 0, -1)) {
			// The block is pushed up by the board, always freeze
//std::cout << "BlockEvent(" << blockGetId() << ")::boardPreInsert(Up) blockFreeze 1" << '\n';
			#ifndef NDEBUG
			const bool bFroze =
			#endif //NDEBUG
			privateFreeze();
			assert(bFroze);
			return; //----------------------------------------------------------
		}
		// check whether one of the bricks will not be covered by one of the tiles
		// but it could be mutilated or even destroyed by bombs! 
		if (oLevel.blockIntersectsArea(*this, oArea.m_nX, nInsY, oArea.m_nW, 1)) {
			// whatever will be put into the inserted area
			// (even empty tiles) sort of kills the block
			// TODO the fact that inserted empty tiles might freeze a block is wrong!
			// TODO maybe make a version of blockIntersectsArea that takes refBuffer as a param
//std::cout << "BlockEvent(" << blockGetId() << ")::boardPreInsert(Up) blockFreeze 2" << '\n';
			#ifndef NDEBUG
			const bool bFroze =
			#endif //NDEBUG
			privateFreeze();
			assert(bFroze);
			return; //----------------------------------------------------------
		}
	} else {
		oLevel.gameStatusTechnical(std::vector<std::string>{"BlockEvent::boardPreInsert", "Unsupported dir"});
	}
}
void BlockEvent::boardPostInsert(Direction::VALUE eDir, NRect /*oArea*/) noexcept
{
	if (eDir == Direction::DOWN) {
//std::cout << "BlockEvent(" << blockGetId() << ")::boardPostDeleteDown" << '\n';
		if (m_bPushed) {
			blockMove(0, +1);
			m_bPushed = false;
		}
	} else if (eDir == Direction::UP) {
	}
}
void BlockEvent::boardPreDestroy(const Coords& /*oCoords*/) noexcept
{
}
void BlockEvent::boardPostDestroy(const Coords& /*oCoords*/) noexcept
{
}
void BlockEvent::boardPreModify(const TileCoords& /*oTileCoords*/) noexcept
{
}
void BlockEvent::boardPostModify(const Coords& /*oCoords*/) noexcept
{
}

} // namespace stmg

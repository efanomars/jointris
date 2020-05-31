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
 * File:   blockevent.h
 */

#ifndef STMG_BLOCK_EVENT_H
#define STMG_BLOCK_EVENT_H

#include <stmm-games/event.h>
#include <stmm-games/levellisteners.h>
#include <stmm-games/levelblock.h>
#include <stmm-games/level.h>
#include <stmm-games/block.h>
#include <stmm-games/animations/explosionanimation.h>

#include <stmm-games/util/direction.h>
#include <stmm-games/util/circularbuffer.h>
#include <stmm-games/util/recycler.h>

#include <vector>
#include <memory>

#include <stdint.h>

namespace stmg
{

using std::unique_ptr;
using std::shared_ptr;

class TileSelector;

class BlockEvent : public Event, public LevelBlock, public BoardListener
{
public:
	/** Calculates the initial position of the block.
	 * The position can be used by setting LocalInit::m_oInitPos.
	 * @param p0Level The level. Cannot be null.
	 * @param oBlock The block.
	 * @param nInitPos The initial x position relative to the center of the board.
	 * @param nInitShapeId The shape.
	 * @return The position in the board (with y as small as possible).
	 */
	static NPoint calcInitialPos(Level* p0Level, const Block& oBlock, int32_t nInitPos, int32_t nInitShapeId) noexcept;

	struct LocalInit
	{
		Block m_oBlock; /**< The block. */
		NPoint m_oInitPos; /**< The initial position of the block within the board. */
		int32_t m_nInitShape = 0; /**< The initial shape of the block. Default is 0. */
		int32_t m_nPlacingMillisec = 1000; /**< The time in millisec the block should try to place itself on the board. Default is 1000. */
		int32_t m_nSkippedFalls = 0; /**< The number of fall() the block should skip after falling. If -1 means the block doesn't fall at all.
										 * If 0 the block falls each time the level calls fall(). Default is 0. */
		int32_t m_nLevelTeam = -1; /**< The team that is allowed to control the block or -1 if any. Default is -1. */
		bool m_bControllable = true; /**< Whether the block can be controlled by players. Default: true. */
		bool m_bCanDrop = true; /**< Whether the block abides to the BlockEvent::s_sKeyActionDrop key action. Default is true. */
		bool m_bCreateDestroyBrickAnimation = true; /**< Whether an animation for each destroyed brick should be created. Default: true. */
		bool m_bEmitDestroyedBricks = false; /**< Whether LISTENER_GROUP_BRICK_DESTROYED listeners should receive messages. Default: false. */
		unique_ptr<TileSelector> m_refBomb; /**< Which tiles are considered bombs. Can be null. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit The initialization data.
	 */
	explicit BlockEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	void regenerate() noexcept;

	const Block& getInitialBlock() noexcept;

	// Event iface
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;

	// LevelBlock iface
	int32_t blockPosZ() const noexcept override;
	void handleKeyActionInput(const shared_ptr<KeyActionEvent>& refEvent) noexcept override;
	void handleTimer() noexcept override;
	void fall() noexcept override;

	bool remove() noexcept override;
	bool destroy() noexcept override;
	bool freeze() noexcept override;
	bool fuseTo(LevelBlock& oLevelBlock) noexcept override;
	bool canFuseWith(LevelBlock& oLevelBlock) const noexcept override;
	bool removeBrick(int32_t nBrickId) noexcept override;
	bool destroyBrick(int32_t nBrickId) noexcept override;

	LevelBlock::QUERY_ATTACK_TYPE queryAttack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY
											, const Tile& oTile) const noexcept override;
	bool attack(LevelBlock& oAttacker, int32_t nBoardX, int32_t nBoardY, const Tile& oTile) noexcept override;
	// BoardListener iface
	void boabloPreFreeze(LevelBlock& oBlock) noexcept override;
	void boabloPostFreeze(const Coords& oCoords) noexcept override;
	void boabloPreUnfreeze(const Coords& oCoords) noexcept override;
	void boabloPostUnfreeze(LevelBlock& oBlock) noexcept override;
	void boardPreScroll(Direction::VALUE eDir, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostScroll(Direction::VALUE eDir) noexcept override;
	void boardPreInsert(Direction::VALUE eDir, NRect oArea, const shared_ptr<TileRect>& refTiles) noexcept override;
	void boardPostInsert(Direction::VALUE eDir, NRect oArea) noexcept override;
	void boardPreDestroy(const Coords& oCoords) noexcept override;
	void boardPostDestroy(const Coords& oCoords) noexcept override;
	void boardPreModify(const TileCoords& oTileCoords) noexcept override;
	void boardPostModify(const Coords& oCoords) noexcept override;

	// Outputs
	enum { 
		LISTENER_GROUP_CANNOT_PLACE = 10 /**< The level block couldn't be placed on the board */
		, LISTENER_GROUP_COULD_PLACE = 11 /**< The level block could be placed on the board. */
		, LISTENER_GROUP_FUSED_WITH = 20 /**< The level block fused with another level block (actively). */
		, LISTENER_GROUP_REMOVED = 90 /**< The level block was removed. */
		, LISTENER_GROUP_DESTROYED = 91 /**< The level block was destroyed. */
		, LISTENER_GROUP_FREEZED = 92 /**< The level block freezed. Value contains board position packed with Util::packPointToInt32(). */
		, LISTENER_GROUP_FUSED_TO = 93 /**< The level block was fused to another block (passively). */
		, LISTENER_GROUP_BRICK_DESTROYED = 94 /**< A brick of the level block was destroyed.
												 * Value contains the board position of the brick packed with Util::packPointToInt32(). */
	};

	/** Get the current shape.
	 * This is exposed for testing.
	 * @return The shape id.
	 */
	inline int32_t blockGetShapeId() const noexcept { return LevelBlock::blockGetShapeId(); }

	static const std::string s_sKeyActionRotate;
	static const std::string s_sKeyActionLeft;
	static const std::string s_sKeyActionRight;
	static const std::string s_sKeyActionDown;
	static const std::string s_sKeyActionDrop;
	static const std::string s_sKeyActionInvertRotation;
	static const std::string s_sKeyActionNext;

	static const std::string s_sGameOptionDelayedFreeze;
	static const std::string s_sPlayerOptionClockwiseRotation;

protected:
	void onFusedWith(const LevelBlock& oLevelBlock) noexcept override;
	void onPlayerChanged() noexcept override;
private:
	void initCommon() noexcept;
	void initRuntime() noexcept;
	//void validateBlock();
	bool validBlock() const noexcept;

	bool privateFreeze() noexcept;
	template<class CustomRemove>
	void privateOnRemove(CustomRemove oCustomRemove) noexcept
	{
		level().boardRemoveListener(this);
		oCustomRemove();
		informListeners(LISTENER_GROUP_FINISHED, 0);
	}
	bool privateDestroyBrick(int32_t nBrickId, bool& bDestroyedLB) noexcept;

	bool canPlaceOnBoardRotate(int32_t& nDeltaX, int32_t& nDeltaY) const noexcept;
	bool canMove(Direction::VALUE eDir, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH
				, int32_t nDeltaExploX, int32_t nDeltaExploY) noexcept;
	bool canMove(Direction::VALUE eDir) noexcept;
	bool move(Direction::VALUE eDir) noexcept;
	bool move(Direction::VALUE eDir, bool& bFused, bool& bWasMutilated, bool& bWasDestroyed) noexcept;
	bool move(Direction::VALUE eDir, bool bHandleBomb, bool bDoMove
					, int32_t nClipX, int32_t nClipY, int32_t nClipW, int32_t nClipH
					, int32_t nDeltaExploX, int32_t nDeltaExploY
					, bool& bHasFused, bool& bWasMutilated, bool& bWasDestroyed) noexcept;

	int32_t incShape(int32_t nShape) const noexcept;

	void calcInitialPos(int32_t nInitPos, const Block& oBlock, int32_t nInitShapeId, int32_t& nPosX, int32_t& nPosY) noexcept;

	void getInitializedCoords(shared_ptr<Coords>& refCoords, int32_t nBoardX, int32_t nBoardY) noexcept;

	void doKeyActionDown() noexcept;
	void doKeyActionRotate() noexcept;
	void doKeyActionDrop() noexcept;

	enum BLOCK_EVENT_STATE
	{
		BLOCK_EVENT_STATE_ACTIVATE = 0,
		BLOCK_EVENT_STATE_INIT = 1,
		BLOCK_EVENT_STATE_PLACE = 2,
		BLOCK_EVENT_STATE_FALL = 3,
		BLOCK_EVENT_STATE_ZOMBIE = 4
	};
private:
	LocalInit m_oData;

	int32_t m_nBoardWidth;
	int32_t m_nBoardHeight;
	// last team that controlled the block
	int32_t m_nLastLevelPlayer;

	int32_t m_nPlaceTryMillisec;
	BLOCK_EVENT_STATE m_eState;

	CircularBuffer<int32_t> m_oKeys; // Value: nKeyActionId
	int32_t m_nLastFallTick;
	int32_t m_nTouchWait;
	bool m_bPushed;

	bool m_bCannotBeHarmed; // while attacking some other LevelBlock

	bool m_bDelayedFreeze;
	bool m_bClockwiseRotation;

	shared_ptr<Coords> m_refCoords;

	class PrivateExplosionAnimation : public ExplosionAnimation
	{
	public:
		using ExplosionAnimation::ExplosionAnimation;
		using ExplosionAnimation::reInit;
	};
	static Recycler<PrivateExplosionAnimation, ExplosionAnimation> s_oBoardExplosionRecycler;

	int32_t m_nKeyActionRotate;
	int32_t m_nKeyActionLeft;
	int32_t m_nKeyActionRight;
	int32_t m_nKeyActionDown;
	int32_t m_nKeyActionDrop;
	int32_t m_nKeyActionInvertRotation;
	int32_t m_nKeyActionNext;

	int32_t m_nSkippedFallCount;

	std::vector< Block::Contact > m_aTempContacts;
	std::vector< int32_t > m_aTempDestroyedBricks;

private:
	BlockEvent() = delete;
	BlockEvent(const BlockEvent& oSource) = delete;
	BlockEvent& operator=(const BlockEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_BLOCK_EVENT_H */


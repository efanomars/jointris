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
 * File:   rowremoverevent.h
 */

#ifndef STMG_ROW_REMOVER_EVENT_H
#define STMG_ROW_REMOVER_EVENT_H

#include <stmm-games/levellisteners.h>
#include <stmm-games/event.h>
#include <stmm-games/tileanimator.h>
#include <stmm-games/util/recycler.h>

#include <vector>
#include <list>
#include <memory>

#include <stdint.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

/** Instances of this class remove entire rows from the board.
 * Whether a row is removed depends on how many non empty tiles are in it.
 * The removal can be immediate or delayed through a tile animation.
 * If multiple rows have to be removed within a game tick, the delay can be
 * different for each row that has to be removed.
 * A row is not removed if it is being removed by another instance, that is
 * any of the tile has an active tile animator named RowRemoverEvent::s_sRemovingTileAniName.
 */
class RowRemoverEvent : public Event, public BoardListener, public TileAnimator
{
public:
	struct LocalInit
	{
		int32_t m_nRepeat = -1; /**< How many times the event checks for rows to remove.
								 * Must be &gt; 0 or -1 (means infinite). Default is -1. */
		int32_t m_nStep = 1; /**< At what cadence in game ticks the check for new rows
								 * to remove should happen (1 means "each game tick"). Default is 1.*/
		int32_t m_nFromGaps = 0; /**< Minimal number of gaps a row must have to be removed by this event. Default is 0. */
		int32_t m_nToGaps = 0; /**< Maximal number of gaps a row must have to be removed by this event. Default is 0. */
		int32_t m_nDeleteAfter = 4; /**< How long (in game ticks) the tile animation must at
									 * least last before the row is actually removed. Must be &gt;= 0. Default is 4. */
		int32_t m_nDeleteAdd = 2; /**< If more than a row has to be removed, what is added to m_nDeleteAfter for each additional row.
								 * Default is 2. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit The initialization data.
	 */
	explicit RowRemoverEvent(Init&& oInit) noexcept;
protected:
	/** Reinitialization.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:
	// Event iface
	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;
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

	// TileAnimator iface
	double getElapsed01(int32_t nHash, int32_t nX, int32_t nY, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;
	double getElapsed01(int32_t nHash, const LevelBlock& oLevelBlock, int32_t nBrickIdx, int32_t nAni, int32_t nViewTick, int32_t nTotViewTicks) const noexcept override;

	// Outputs
	enum {
		LISTENER_GROUP_REMOVED = 10 /**< One or more rows are removed. Sent on detection, not on actual removal.
									 * Param nValue of the listener's trigger function contains the number of
									 * rows (to be) removed. */
	};

	static const std::string s_sRemovingTileAniName;
	static constexpr int32_t s_nMaxDeleteAfter = 10000;
private:
	struct DelStrip {
		void reInit() {}
		int32_t m_nY;
		int32_t m_nX;
		int32_t m_nW;
	};
	typedef std::list< shared_ptr<DelStrip> > DelStripList;
	struct DownCounter {
		int32_t m_nAllIdx; // index in m_oAllDownCounters
		bool m_bLocked;
		int32_t m_nTotSteps;
		int32_t m_nCountdown;
		bool m_bInitAni;
		DelStripList m_oStrips;
	};
	using DownCounterList = std::list< shared_ptr<DownCounter> >;

	void commonInit() noexcept;
	void reInitRuntime() noexcept;

	void recalcRow(int32_t nY) noexcept;
	void adjustRectVert(DelStripList& oRects
			, DelStrip& oRect, int32_t nRangeX, int32_t nRangeW, int32_t nMoveH) noexcept;
	void checkToRemove() noexcept;
	bool checkToRemoveCondition(int32_t nY) noexcept;
	void remove() noexcept;
	void boardPostDeleteDown(int32_t nY, int32_t nX, int32_t nW) noexcept;
	void boardPostInsertUp(int32_t nY, int32_t nX, int32_t nW) noexcept;

	void recyclePushBackDownCounter(shared_ptr<DownCounter>& refDownCounter) noexcept;
	shared_ptr<DownCounter> recyclePopDownCounter() noexcept;

	enum ROW_REMOVER_STATE
	{
		ROW_REMOVER_STATE_ACTIVATE = 0
		, ROW_REMOVER_STATE_INIT = 1
		, ROW_REMOVER_STATE_RUN = 2
	};
private:
	LocalInit m_oData;

	int32_t m_nTileAniRemovingIndex;
	int32_t m_nBoardW;
	int32_t m_nBoardH;

	ROW_REMOVER_STATE m_eState;

	std::vector<int32_t> m_aFillCount; // size: m_nBoardH
	DownCounterList m_oDownCounters;
	DownCounterList m_oInactiveDownCounters;
	std::vector< shared_ptr<DownCounter> > m_oAllDownCounters;
	Recycler<DelStrip> m_oDelStripRecycler;

	int32_t m_nCounter;
private:
	RowRemoverEvent() = delete;
	RowRemoverEvent(const RowRemoverEvent& oSource) = delete;
	RowRemoverEvent& operator=(const RowRemoverEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_ROW_REMOVER_EVENT_H */


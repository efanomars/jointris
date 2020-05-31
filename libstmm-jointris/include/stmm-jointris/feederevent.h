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
 * File:   feederevent.h
 */

#ifndef STMG_FEEDER_EVENT_H
#define STMG_FEEDER_EVENT_H

#include "blockevent.h"

#include <stmm-games/event.h>
//#include <stmm-games/level.h>
//#include <stmm-games/gamewidgets.h>
#include <stmm-games/widgets/previewwidget.h>

#include <stmm-games/util/circularbuffer.h>
#include <stmm-games/util/randomparts.h>

#include <vector>
#include <string>
#include <memory>

#include <stdint.h>

namespace stmg
{

using std::unique_ptr;
using std::shared_ptr;

class FeederEvent : public Event
{
public:
	class BlockBunch
	{
	public:
		std::vector<BlockEvent*> m_aBlockEvents; /**< Non owning block events, which must already have been added to the Level. */
		bool m_bJoined = false; /**< Whether the blocks have to be joined before they freeze. Default is false. */
	private:
		friend class FeederEvent;
		std::vector<Block> m_aInitialBlocks;
	};
	struct LocalInit
	{
		int32_t m_nRepeat = -1; /**< How many block bunches have to be released. The default is -1 (means infinite). */
		int32_t m_nWait = 0; /**< How many game ticks to wait after the freeze before the next block bunch is released. Default is 0. */
		shared_ptr<PreviewWidget> m_refPreview; /**< The preview widget. Can be null. */
		std::vector< std::pair<int32_t, BlockBunch> > m_aRandomBlocks; /**< The blocks associated with their weighted probability.
																			 * Each weight must be &gt; 0. Cannot be empty. */
		bool m_bAllowBlockBunchRepetition = true; /**< Whether two consecutive block bunches are allowed. Default is true. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * This class expects the BlockEvent instances to have been already added to the (same) level.
	 * @param oInit The initialization data.
	 */
	explicit FeederEvent(Init&& oInit) noexcept;

	static const std::string s_sGameOptionShowPreview;

protected:
	/** Reinitialize.
	 * Same as constructor.
	 * @param oInit The initialization data.
	 */
	void reInit(Init&& oInit) noexcept;
public:

	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;

	// Inputs
	enum { 
		MESSAGE_PAUSE = 100 /**< Pause. Stops until MESSAGE_RESUME is received. Ignored if already pausing or finished. */
		, MESSAGE_RESUME = 101 /**< Resume. Stops until MESSAGE_RESUME is received. Ignored if not pausing or finished. */
		, MESSAGE_RESTART = 110 /**< Restart. Only works if feeder successfully finished all its blocks. */
		, MESSAGE_ADD_RANDOM_INT32 = 120 /**< Instead of using the game random generator use nValue (whole int32_t range). */
		, MESSAGE_ADD_RANDOM_WEIGHT = 121 /**< Instead of using the game random generator use nValue (as weight).
											 * If nValue is not &gt;= 0 and &lt; the sum of all the weighted probabilities the modulo is applied.
											 * Disregards m_bAllowBlockBunchRepetition passed to the constructor. */
		, MESSAGE_ADD_RANDOM_BUNCH = 122 /**< Instead of using the game random generator use nValue (as bunch index).
											 * If nValue is not &gt;= 0 and &lt; the total number of bunches the modulo is applied.
											 * Disregards m_bAllowBlockBunchRepetition passed to the constructor. */
	};
	// Outputs
	enum { 
		LISTENER_GROUP_BUNCH_DONE = 10 /**< All blocks of a bunch not supposed to join freezed. */
		, LISTENER_GROUP_BUNCH_JOINED = 15 /**< All blocks of a bunch supposed to join did. */
		, LISTENER_GROUP_BUNCH_NOT_JOINED = 16 /**< All blocks of a bunch supposed to join didn't. */
		, LISTENER_GROUP_CANNOT_PLACE_BLOCK = 20 /**< A block couldn't be placed. */
	};

private:
	void commonInit(LocalInit&& oInit) noexcept;
	void addRandom(int32_t nProb, BlockBunch&& oBlockBunch) noexcept;
	void runtimeInit() noexcept;

	bool isMyBlock(Event* p0TriggeringEvent) noexcept;
	bool isMyBlock(int32_t nLevelBlockId) noexcept;
	void calcNext() noexcept;
	void showNextInPreview() noexcept;

private:
	enum FEEDER_EVENT_STATE
	{
		FEEDER_EVENT_STATE_ACTIVATE = 0
		, FEEDER_EVENT_STATE_INIT = 1
		, FEEDER_EVENT_STATE_RUN = 2
		, FEEDER_EVENT_STATE_WAITING = 3
		, FEEDER_EVENT_STATE_ZOMBIE = 4
	};
	// Inputs from BlockEvents
	enum {
		LISTENER_MSG_MAGIC = 787876 // FREEZE, DESTROY, REMOVE
		, LISTENER_MSG_MAGIC_FUSED = 787877
		, LISTENER_MSG_MAGIC_CANNOT_PLACE = 787878
		, LISTENER_MSG_MAGIC_COULD_PLACE = 787879
	};

	int32_t m_nRepeat;
	int32_t m_nWait;
	shared_ptr<PreviewWidget> m_refPreviewWidget;
	bool m_bAllowBlockBunchRepetition;

	FEEDER_EVENT_STATE m_eState;
	// For the preview to be called m_bShowPreview must be true
	bool m_bShowPreview;

	RandomParts<BlockBunch> m_oRandom;
	int32_t m_nPlacedBlocks;
	int32_t m_nFinishedBlocks;
	int32_t m_nJoinedBlocks;
	int32_t m_nPrevBunch;
		BlockBunch* m_p0NextBlocks;
		BlockBunch* m_p0CurBlocks;

	int32_t m_nCounter;
	bool m_bPaused;

	static constexpr int32_t s_nMaxRandomStackSize = 10;
	enum RANDOM_TYPE
	{
		RANDOM_TYPE_INT32 = 0
		, RANDOM_TYPE_WEIGHT = 1
		, RANDOM_TYPE_BUNCH_IDX = 2
	};
	CircularBuffer<std::pair<RANDOM_TYPE, int32_t>> m_aRandomStack;

	static const std::string s_sJoin;

private:
	FeederEvent() = delete;
	FeederEvent(const FeederEvent& oSource) = delete;
	FeederEvent& operator=(const FeederEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_FEEDER_EVENT_H */


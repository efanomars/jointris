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
 * File:   rowpusherevent.h
 */

#ifndef STMG_ROW_PUSHER_EVENT_H
#define STMG_ROW_PUSHER_EVENT_H

#include <stmm-games/event.h>
//#include <stmm-games/level.h>
#include <stmm-games/utile/newrows.h>
#include <stmm-games/utile/tilebuffer.h>

#include <stmm-games/util/recycler.h>
#include <stmm-games/util/circularbuffer.h>
//#include <stmm-games/util/randomparts.h>

#include <vector>
#include <memory>

#include <stdint.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;

class RowPusherEvent : public Event
{
public:
	struct LocalInit
	{
		int32_t m_nPushY = 0; /**< The y coordinate of the pushed row. Default is 0. */
		int32_t m_nPushX = 0; /**< The start x coordinate of the pushed row. Default is 0. */
		int32_t m_nPushW = 1; /**< The width of the pushed row. Default is 1. */
		unique_ptr< NewRows > m_refNewRows; /**< The new rows generator. Cannot be null. */
	};
	struct Init : public Event::Init, public LocalInit
	{
	};
	/** Constructor.
	 * @param oInit Initialization data.
	 */
	explicit RowPusherEvent(Init&& oInit) noexcept;
protected:
	void reInit(Init&& oInit) noexcept;
public:

	void trigger(int32_t nMsg, int32_t nValue, Event* p0TriggeringEvent) noexcept override;

	// input
	enum {
		MESSAGE_PUSH_ROW = 100 /**< Push new row nValue times. */
		, MESSAGE_PUSH_ROW_REPEAT = 101 /**< Push the same row nValue times. */
		, MESSAGE_PUSH_ROW_BY_TWO = 102 /**< Push nValue rows by couple (a new row is followed by a clone). */
		, MESSAGE_PAUSE = 110 /**< Pause the pushes. */
		, MESSAGE_RESUME = 111 /**< Resume the pushes. */
		, MESSAGE_SET_NEW_ROW_GEN = 120 /**< nValue is index LocalInit::m_aNewRowTileGens. If negative sets to 0.
										 * If bigger than vector size sets last. */
		, MESSAGE_NEXT_NEW_ROW_GEN = 121 /**< Switches to next row generator. If last stays on last. */
		, MESSAGE_PREV_NEW_ROW_GEN = 122 /**< Switches to previous row generator. If first stays on first. */
	};
	// output
	enum {
		LISTENER_GROUP_PUSHED = 10
	};

private:
	void commonInit(LocalInit&& oInit) noexcept;
	void reInitRuntime() noexcept;

	bool isPushRowMsg(int32_t nMsg) const noexcept
	{
		return (nMsg == MESSAGE_PUSH_ROW) || (nMsg == MESSAGE_PUSH_ROW_REPEAT) || (nMsg == MESSAGE_PUSH_ROW_BY_TWO);
	}
	bool checkTopCanBePushed() noexcept;

	int32_t handleOtherMsgs(int32_t nMsg, int32_t nValue, int32_t nTriggerTime, int32_t nGameTick) noexcept;
	void addToPushUp(int32_t nToPushUp, int32_t nMsg) noexcept;
	void pushRow() noexcept;

private:
	int32_t m_nPushY;
	int32_t m_nPushX;
	int32_t m_nPushW;
	unique_ptr< NewRows > m_refNewRows;

	bool m_bPaused;

	int32_t m_nTileAniRemovingIndex;
	int32_t m_nBoardW;
	int32_t m_nBoardH;

	enum ROW_PUSHER_STATE
	{
		ROW_PUSHER_STATE_ACTIVATE = 0,
		ROW_PUSHER_STATE_INIT = 1,
		ROW_PUSHER_STATE_RUN = 2
	};
	ROW_PUSHER_STATE m_eState;

	Recycler<TileBuffer> m_refTileBufRecycler;
	shared_ptr<TileBuffer> m_refCurrent;
	CircularBuffer<int32_t> m_oToPushUp;
	int32_t m_nLastPushUpTime;
	int32_t m_nCurRandomGen; // index into m_aRandomTiles

private:
	RowPusherEvent() = delete;
	RowPusherEvent(const RowPusherEvent& oSource) = delete;
	RowPusherEvent& operator=(const RowPusherEvent& oSource) = delete;
};

} // namespace stmg

#endif	/* STMG_ROW_PUSHER_EVENT_H */


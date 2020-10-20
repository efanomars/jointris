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
 * File:   testRowPusherEvent.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <stmm-games-fake/fixtureGame.h>
#include <stmm-games-fake/fakelevelview.h>
#include <stmm-games-fake/mockevent.h>

#include "rowremoverevent.h"
#include "rowpusherevent.h"
#include "blockevent.h"

#include <stmm-games/events/sysevent.h>
#include <stmm-games/events/logevent.h>
#include <stmm-games/traitsets/tiletraitsets.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class RowPusherGameFixture : public GameFixture
							//default , public FixtureVariantDevicesKeys_Two, public FixtureVariantDevicesJoystick_Two
							, public FixtureVariantPrefsTeams<1>
							, public FixtureVariantPrefsMates<0,1>
							//default , public FixtureVariantMatesPerTeamMax_Three, public FixtureVariantAIMatesPerTeamMax_Zero
							//default , public FixtureVariantAllowMixedAIHumanTeam_False, public FixtureVariantPlayersMax_Six
							//default , public FixtureVariantTeamsMin_One, public FixtureVariantTeamsMax_Two
							, public FixtureVariantKeyActions_Custom
							, public FixtureVariantOptions_Custom
							//default , public FixtureVariantLayoutTeamDistribution_AllTeamsInOneLevel
							//default , public FixtureVariantLayoutShowMode_Show
							//default , public FixtureVariantLayoutCreateVarWidgetsFromVariables_False
							//default , public FixtureVariantLayoutCreateActionWidgetsFromKeyActions_False
							, public FixtureVariantVariablesGame_Time
							//, public FixtureVariantVariablesTeam
							, public FixtureVariantVariablesPlayer_Lives<3>
							//default , public FixtureVariantLevelInitBoardWidth<10>
							//default , public FixtureVariantLevelInitBoardHeight<6>
							//default , public FixtureVariantLevelInitShowWidth<10>
							//default , public FixtureVariantLevelInitShowHeight<6>
{
protected:
	void setup() override
	{
		GameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		RowPusherEvent::Init oInit;
		oInit.m_p0Level = p0Level;
//		oInit.m_nGaps = getPushGaps();
		oInit.m_nPushY = getPushY();
		oInit.m_nPushX = getPushX();
		oInit.m_nPushW = getPushW();
		//
		NewRows::Init oNRInit;
		NewRows::NewRowGen oNewRowGen;
		NewRows::DistrRandTiles oDistrRandTiles;
		oDistrRandTiles.m_nLeaveEmpty = getPushGaps();
		oDistrRandTiles.m_nRandomTilesIdx = 0;
		oNewRowGen.m_aDistrs.push_back(std::make_unique<NewRows::DistrRandTiles>(std::move(oDistrRandTiles)));
		oNRInit.m_aNewRowGens.push_back(std::move(oNewRowGen));

		auto refCTS = make_unique<CharTraitSet>(make_unique<CharIndexTraitSet>(40, 41));

		RandomTiles::ProbTraitSets oProbTraitSets;
		oProbTraitSets.m_nProb = 10;
		oProbTraitSets.m_aTraitSets.push_back(std::move(refCTS));
		RandomTiles::ProbTileGen oProbTileGen;
		oProbTileGen.m_aProbTraitSets.push_back(std::move(oProbTraitSets));
		oNRInit.m_aRandomTiles.push_back(std::move(oProbTileGen));
		//
		oInit.m_refNewRows = std::make_unique<NewRows>(*m_refGame, std::move(oNRInit));

		auto refRowPusherEvent = make_unique<RowPusherEvent>(std::move(oInit));
		m_p0RowPusherEvent = refRowPusherEvent.get();
		p0Level->addEvent(std::move(refRowPusherEvent));
//		p0Level->activateEvent(m_p0RowPusherEvent, 0);
	}
	void teardown() override
	{
		GameFixture::teardown();
	}

	virtual int32_t getPushGaps() { return 0; }
	virtual int32_t getPushY()
	{
		assert(m_refLevel);
		return m_refLevel->boardHeight() - 1;
	}
	virtual int32_t getPushX()
	{
		return 0;
	}
	virtual int32_t getPushW()
	{
		assert(m_refLevel);
		return m_refLevel->boardWidth();
	}

	std::vector< shared_ptr<Option> > getCustomOptions() const override
	{
		return std::vector< shared_ptr<Option> >{};
	}
	std::vector<StdConfig::KeyAction> getCustomKeyActions() const override
	{
		return std::vector<StdConfig::KeyAction>{};
	}
protected:
	shared_ptr<Level> m_refLevel;
	RowPusherEvent* m_p0RowPusherEvent;
};

class RowPusherCase1GameFixture : public RowPusherGameFixture
{
protected:
	void setup() override
	{
		RowPusherGameFixture::setup();
		Level* p0Level = m_refLevel.get();
		p0Level->activateEvent(m_p0RowPusherEvent, 0);
	}
	void teardown() override
	{
		RowPusherGameFixture::teardown();
	}
	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		{
			Tile oTile;
			oTile.getTileColor().setColorPal(4);
			const int32_t nY = nBoardH - 1;
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
		{
			Tile oTile;
			oTile.getTileColor().setColorPal(5);
			const int32_t nY = 0;
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
	}
};

class RowPusherCase2GameFixture : public RowPusherGameFixture
{
protected:
	void setup() override
	{
		RowPusherGameFixture::setup();
		Level* p0Level = m_refLevel.get();
		p0Level->activateEvent(m_p0RowPusherEvent, 0);
	}
	void teardown() override
	{
		RowPusherGameFixture::teardown();
	}
	int32_t getPushGaps() override { return 2; }
	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		{
			Tile oTile;
			oTile.getTileColor().setColorPal(4);
			const int32_t nY = nBoardH - 1;
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
		{
			Tile oTile;
			oTile.getTileColor().setColorPal(5);
			const int32_t nY = 0;
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
	}
};

class RowPusherRemoverGameFixture : public RowPusherGameFixture
{
protected:
	void setup() override
	{
		RowPusherGameFixture::setup();
		Level* p0Level = m_refLevel.get();
		p0Level->activateEvent(m_p0RowPusherEvent, 0);

		m_nTileAniRemovingIndex = m_refGame->getNamed().tileAnis().addName(RowRemoverEvent::s_sRemovingTileAniName);

		p0Level->activateEvent(m_p0RowPusherEvent, 1);

		RowRemoverEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_nRepeat = 1;
		oInit.m_nStep = 1;
		oInit.m_nFromGaps = getRemoverGaps();
		oInit.m_nToGaps = oInit.m_nFromGaps;
		oInit.m_nDeleteAfter = 2;
		oInit.m_nDeleteAdd = 0;
		auto refRowRemoverEvent = make_unique<RowRemoverEvent>(std::move(oInit));
		m_p0RowRemoverEvent = refRowRemoverEvent.get();
		p0Level->addEvent(std::move(refRowRemoverEvent));
		p0Level->activateEvent(m_p0RowRemoverEvent, 0);
	}
	void teardown() override
	{
		RowPusherGameFixture::teardown();
	}
	virtual int32_t getRemoverGaps() { return 8; }
protected:
	RowRemoverEvent* m_p0RowRemoverEvent;
	int32_t m_nTileAniRemovingIndex;
};

class RowPusherRemoverCase3GameFixture : public RowPusherRemoverGameFixture
{
protected:
	void setup() override
	{
		RowPusherRemoverGameFixture::setup();
	}
	void teardown() override
	{
		RowPusherRemoverGameFixture::teardown();
	}
	int32_t getPushGaps() override { return 8; }
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		{
			Tile oTile;
			assert(nBoardW >= 10);
			oTile.getTileColor().setColorPal(4);
			aBoard[0 + 0 * nBoardW] = oTile;
			aBoard[9 + 0 * nBoardW] = oTile;
		}
		{
			Tile oTile;
			oTile.getTileColor().setColorPal(5);
			aBoard[0 + 1 * nBoardW] = oTile;
			aBoard[9 + 1 * nBoardW] = oTile;
		}
	}
};

class RowPusherRemoverCase4GameFixture : public RowPusherRemoverGameFixture
{
protected:
	void setup() override
	{
		RowPusherRemoverGameFixture::setup();
	}
	void teardown() override
	{
		RowPusherRemoverGameFixture::teardown();
	}
	int32_t getPushGaps() override { return 8; }
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		{
			Tile oTile;
			oTile.getTileColor().setColorPal(5);
			aBoard[0 + 1 * nBoardW] = oTile;
			aBoard[9 + 1 * nBoardW] = oTile;
		}
	}
};

class RowPusherRemoverCase5GameFixture : public RowPusherRemoverGameFixture
{
protected:
	void setup() override
	{
		RowPusherRemoverGameFixture::setup();
	}
	void teardown() override
	{
		RowPusherRemoverGameFixture::teardown();
	}
	int32_t getPushGaps() override { return 0; }
	int32_t getPushY() override { return 4; }
	int32_t getPushX() override { return 3; }
	int32_t getPushW() override { return 2; }
	int32_t getRemoverGaps() override { return 2; }
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		Tile oTile1;
		oTile1.getTileColor().setColorPal(11);
		Tile oTile2;
		oTile2.getTileColor().setColorPal(14);
		Tile oTile3;
		oTile3.getTileColor().setColorPal(15);
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			if ((nX < 7) || (nX > 8)) {
				aBoard[nX + 1 * nBoardW] = oTile1;			
			}
			aBoard[nX + 4 * nBoardW] = oTile2;
			aBoard[nX + 5 * nBoardW] = oTile3;
		}
	}
};

class RowPusherEmptyBoardFixture : public RowPusherGameFixture
{
protected:
	void setup() override
	{
		RowPusherGameFixture::setup();
	}
	void teardown() override
	{
		RowPusherGameFixture::teardown();
	}
	int32_t getPushGaps() override { return 1; }
	void fillBoard(int32_t /*nBoardW*/, int32_t /*nBoardH*/, std::vector<Tile>& /*aBoard*/) override
	{
	}
};

TEST_CASE_METHOD(STFX<RowPusherEmptyBoardFixture>, "PushRowMsgOneGaps")
{
	Level* p0Level = m_refLevel.get();

	const int32_t nBoardW = m_refLevel->boardWidth();
	const int32_t nBoardH = m_refLevel->boardHeight();

	MockEvent::Init oMockInit;
	oMockInit.m_p0Level = p0Level;
	auto refMockEvent = make_unique<MockEvent>(std::move(oMockInit));
	MockEvent* p0MockEvent = refMockEvent.get();
	p0Level->addEvent(std::move(refMockEvent));

	const int32_t nMockGroup = 8889;
	p0MockEvent->addListener(nMockGroup, m_p0RowPusherEvent, RowPusherEvent::MESSAGE_PUSH_ROW);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 );

	auto oCheckTiles = [&](int32_t nExpectedTiles)
	{
		int32_t nNotEmpty = 0;
		for (int32_t nY = 0; nY < nBoardH; ++nY) {
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				const Tile& oTile = p0Level->boardGetTile(nX, nY);
				if (! oTile.isEmpty()) {
					++nNotEmpty;
				}
			}
		}
		REQUIRE(nNotEmpty == nExpectedTiles);
	};

	oCheckTiles(0);

	const int32_t nJunkRows = 5;
	p0MockEvent->setTriggerValue(nMockGroup, nJunkRows, 1); // group 8889, value, trigger after one tick

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	oCheckTiles(0);

	for (int32_t nCount = 1; nCount <= nJunkRows; ++nCount) {
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == (2 + nCount) );

		oCheckTiles(nCount * (nBoardW - 1));
	}

	oCheckTiles(nJunkRows * (nBoardW - 1));

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == (3 + nJunkRows) );

	oCheckTiles(nJunkRows * (nBoardW - 1));
}
TEST_CASE_METHOD(STFX<RowPusherEmptyBoardFixture>, "PushRowRepeatMsgOneGaps")
{
	Level* p0Level = m_refLevel.get();

	const int32_t nBoardW = m_refLevel->boardWidth();
	const int32_t nBoardH = m_refLevel->boardHeight();

	MockEvent::Init oMockInit;
	oMockInit.m_p0Level = p0Level;
	auto refMockEvent = make_unique<MockEvent>(std::move(oMockInit));
	MockEvent* p0MockEvent = refMockEvent.get();
	p0Level->addEvent(std::move(refMockEvent));

	const int32_t nMockGroup = 8889;
	p0MockEvent->addListener(nMockGroup, m_p0RowPusherEvent, RowPusherEvent::MESSAGE_PUSH_ROW_REPEAT);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 );

	auto oCheckRowsEqual = [&](int32_t nFromRow, int32_t nToRow)
	{
		assert(nFromRow < nToRow);
		for (int32_t nY = nFromRow + 1; nY <= nToRow; ++nY) {
			int32_t nTotSame = 0;
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				const Tile& oTile1 = p0Level->boardGetTile(nX, nY);
				const Tile& oTile2 = p0Level->boardGetTile(nX, nFromRow);
				if (oTile1 == oTile2) {
					++nTotSame;
				}
			}
			REQUIRE(nTotSame == nBoardW);
		}
	};

	const int32_t nJunkRows = 5;
	p0MockEvent->setTriggerValue(nMockGroup, nJunkRows, 1); // group 8889, value, trigger after one tick

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	for (int32_t nCount = 1; nCount <= nJunkRows; ++nCount) {
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == (2 + nCount) );
	}

	oCheckRowsEqual(1, nBoardH - 1);
}
TEST_CASE_METHOD(STFX<RowPusherEmptyBoardFixture>, "PushRowByTwoMsgOneGaps")
{
	Level* p0Level = m_refLevel.get();

	const int32_t nBoardW = m_refLevel->boardWidth();
	//const int32_t nBoardH = m_refLevel->boardHeight();

	MockEvent::Init oMockInit;
	oMockInit.m_p0Level = p0Level;
	auto refMockEvent = make_unique<MockEvent>(std::move(oMockInit));
	MockEvent* p0MockEvent = refMockEvent.get();
	p0Level->addEvent(std::move(refMockEvent));

	const int32_t nMockGroup = 8889;
	p0MockEvent->addListener(nMockGroup, m_p0RowPusherEvent, RowPusherEvent::MESSAGE_PUSH_ROW_BY_TWO);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 );

	auto oCheckRowsEqual = [&](int32_t nFromRow, int32_t nToRow)
	{
		assert(nFromRow < nToRow);
		for (int32_t nY = nFromRow + 1; nY <= nToRow; ++nY) {
			int32_t nTotSame = 0;
			for (int32_t nX = 0; nX < nBoardW; ++nX) {
				const Tile& oTile1 = p0Level->boardGetTile(nX, nY);
				const Tile& oTile2 = p0Level->boardGetTile(nX, nFromRow);
				if (oTile1 == oTile2) {
					++nTotSame;
				}
			}
			REQUIRE(nTotSame == nBoardW);
		}
	};

	const int32_t nJunkRows = 5;
	p0MockEvent->setTriggerValue(nMockGroup, nJunkRows, 1); // group 8889, value, trigger after one tick

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	for (int32_t nCount = 1; nCount <= nJunkRows; ++nCount) {
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == (2 + nCount) );
	}

	oCheckRowsEqual(1, 2);
	oCheckRowsEqual(3, 4);
}

TEST_CASE_METHOD(STFX<RowPusherCase1GameFixture>, "Case1_PushOneRowNoGaps")
{
	Level* p0Level = m_refLevel.get();

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	REQUIRE( ! p0Level->boardGetTile(0, 0).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 4).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 5).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 5).getTileChar().isEmpty() );
	m_refGame->handleTimer(); // start row pusher
	REQUIRE( m_refGame->gameElapsed() == 1 );
	REQUIRE( p0Level->boardGetTile(0, 0).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 4).isEmpty() );
	for (int32_t nX = 0; nX < p0Level->boardWidth(); ++nX) {
		const Tile& oTile = p0Level->boardGetTile(nX, 5);
		REQUIRE( ! oTile.isEmpty() );
		REQUIRE( ! oTile.getTileChar().isEmpty() );
		REQUIRE( oTile.getTileChar().isCharIndex() );
		const auto nCharIdx = oTile.getTileChar().getCharIndex();
		REQUIRE(( (nCharIdx == 40) || (nCharIdx == 41) ));
	}
}

TEST_CASE_METHOD(STFX<RowPusherCase2GameFixture>, "Case2_PushOneRowTwoGaps")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 19722;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0RowPusherEvent->addListener(RowPusherEvent::LISTENER_GROUP_PUSHED, p0LogEvent, 77);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	REQUIRE( ! p0Level->boardGetTile(0, 0).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 4).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 5).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 5).getTileChar().isEmpty() );
	m_refGame->handleTimer(); // start row pusher
	REQUIRE( m_refGame->gameElapsed() == 1 );
	REQUIRE( p0Level->boardGetTile(0, 0).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 4).isEmpty() );
	int32_t nTotGaps = 0;
	for (int32_t nX = 0; nX < p0Level->boardWidth(); ++nX) {
		const Tile& oTile = p0Level->boardGetTile(nX, 5);
		if (oTile.isEmpty()) {
			++nTotGaps;
		} else {
			REQUIRE( ! oTile.getTileChar().isEmpty() );
			REQUIRE( oTile.getTileChar().isCharIndex() );
			const auto nCharIdx = oTile.getTileChar().getCharIndex();
			REQUIRE(( (nCharIdx == 40) || (nCharIdx == 41) ));
		}
	}
	REQUIRE( nTotGaps == 2 );

	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 19722 );
		REQUIRE( oEntry.m_nGameTick == 0 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 77);
		//REQUIRE( oEntry.m_nValue == 1 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowPusherEvent) );
	}
}

TEST_CASE_METHOD(STFX<RowPusherRemoverCase3GameFixture>, "Case3_PushOnlyWhenTopRowRemoved")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 89722;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_REMOVED, p0LogEvent, 33);
	m_p0RowPusherEvent->addListener(RowPusherEvent::LISTENER_GROUP_PUSHED, p0LogEvent, 77);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	REQUIRE( ! p0Level->boardGetTile(0, 0).isEmpty() );
	REQUIRE( p0Level->boardGetTile(4, 0).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(9, 0).isEmpty() );
	m_refGame->handleTimer(); // start row remover
	REQUIRE( m_refGame->gameElapsed() == 1 );
	// the remover started animating top row
	REQUIRE( p0Level->boardGetTileAnimator(0, 0, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 89722 );
		REQUIRE( oEntry.m_nGameTick == 0 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 33);
		REQUIRE( oEntry.m_nValue == 2 ); // The two top rows have to be removed
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
	LogEvent::msgLog().reset();

	m_refGame->handleTimer(); // start row pusher
	REQUIRE( m_refGame->gameElapsed() == 2 );

	// the remover still animating top row
	REQUIRE( p0Level->boardGetTileAnimator(0, 0, m_nTileAniRemovingIndex) != nullptr );
	// This means the push is delayed until the top row is actually removed.
	REQUIRE( LogEvent::msgLog().totBuffered() == 0 );

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 3 );
	// the remover removed top row
	REQUIRE( p0Level->boardGetTileAnimator(0, 0, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTile(0, 0).isEmpty() );
	// Depending on the scheduler order of triggering the two events the push might
	// already have happened, but we only check in next game tick.

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 4 );

	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 89722 );
		REQUIRE(( (oEntry.m_nGameTick == 2) || (oEntry.m_nGameTick == 3) ));
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 77);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowPusherEvent) );
	}
}

TEST_CASE_METHOD(STFX<RowPusherRemoverCase4GameFixture>, "Case4_PushARowBeingRemoved")
{
	Level* p0Level = m_refLevel.get();

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	REQUIRE( ! p0Level->boardGetTile(0, 1).isEmpty() );
	REQUIRE( p0Level->boardGetTile(4, 1).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(9, 1).isEmpty() );

	m_refGame->handleTimer(); // start row remover
	REQUIRE( m_refGame->gameElapsed() == 1 );
	REQUIRE( p0Level->boardGetTileAnimator(0, 1, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(4, 1, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(9, 1, m_nTileAniRemovingIndex) != nullptr );

	m_refGame->handleTimer(); // start row pusher
	REQUIRE( m_refGame->gameElapsed() == 2 );

	REQUIRE( p0Level->boardGetTileAnimator(0, 0, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(4, 0, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(9, 0, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( ! p0Level->boardGetTile(0, 0).isEmpty() );
	REQUIRE( p0Level->boardGetTile(4, 0).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(9, 0).isEmpty() );
	REQUIRE( p0Level->boardGetTileAnimator(0, 1, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(4, 1, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(9, 1, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTile(0, 1).isEmpty() );
	REQUIRE( p0Level->boardGetTile(4, 1).isEmpty() );
	REQUIRE( p0Level->boardGetTile(9, 1).isEmpty() );
}

TEST_CASE_METHOD(STFX<RowPusherRemoverCase5GameFixture>, "Case5_PushUpSegment")
{
	Level* p0Level = m_refLevel.get();

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );

	m_refGame->handleTimer(); // start row remover
	REQUIRE( m_refGame->gameElapsed() == 1 );

	int32_t nX;
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		if ((nX < 7) || (nX > 8)) {
			Tile oTile = p0Level->boardGetTile(nX, 1 );
			REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
			REQUIRE(oTile.getTileColor().getColorPal() == 11);
		}
	}
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 4 );
		REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
		REQUIRE(oTile.getTileColor().getColorPal() == 14);
	}
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 5 );
		REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
		REQUIRE(oTile.getTileColor().getColorPal() == 15);
	}

	REQUIRE( p0Level->boardGetTileAnimator(0, 1, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(4, 1, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(9, 1, m_nTileAniRemovingIndex) != nullptr );

	m_refGame->handleTimer(); // start row pusher
	REQUIRE( m_refGame->gameElapsed() == 2 );

	REQUIRE( p0Level->boardGetTileAnimator(0, 0, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(4, 0, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(9, 0, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(0, 1, m_nTileAniRemovingIndex) != nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(4, 1, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTileAnimator(9, 1, m_nTileAniRemovingIndex) != nullptr );

	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 0 );
		if ((nX >= 3) && (nX < 3 + 2)) {
			REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
			REQUIRE(oTile.getTileColor().getColorPal() == 11);
		} else {
			REQUIRE(oTile.isEmpty());
		}
	}
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 1 );
		if ((nX >= 3) && (nX < 3 + 2)) {
			REQUIRE(oTile.isEmpty());
		} else {
			if ((nX < 7) || (nX > 8)) {
				REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
				REQUIRE(oTile.getTileColor().getColorPal() == 11);
			} else {
				REQUIRE(oTile.isEmpty());
			}
		}
	}
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 3 );
		if ((nX >= 3) && (nX < 3 + 2)) {
			REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
			REQUIRE(oTile.getTileColor().getColorPal() == 14);
		} else {
			REQUIRE(oTile.isEmpty());
		}
	}
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 4 );
		if ((nX >= 3) && (nX < 3 + 2)) {
			const auto& oTileChar = oTile.getTileChar();
			REQUIRE(oTileChar.isCharIndex());
			REQUIRE(( (oTileChar.getCharIndex() == 40) || (oTileChar.getCharIndex() == 41) ));
		} else {
			REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
			REQUIRE(oTile.getTileColor().getColorPal() == 14);
		}
	}
	for (nX = 0; nX < p0Level->boardWidth(); ++nX) {
		Tile oTile = p0Level->boardGetTile(nX, 5 );
		REQUIRE(oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
		REQUIRE(oTile.getTileColor().getColorPal() == 15);
	}
}

} // namespace testing

} // namespace stmg

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
 * File:   testRowRemoverEvent.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <stmm-games-fake/fixtureGame.h>
#include <stmm-games-fake/fakelevelview.h>

#include "rowremoverevent.h"
#include "blockevent.h"

#include <stmm-games/events/sysevent.h>
#include <stmm-games/events/logevent.h>
#include <stmm-games/traitsets/tiletraitsets.h>
#include <stmm-games/utile/tileselector.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class RowRemoverGameFixture : public GameFixture
							, public FixtureVariantDevicesKeys_One //default , public FixtureVariantDevicesJoystick_Two
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
		m_nTileAniRemovingIndex = m_refGame->getNamed().tileAnis().addName(RowRemoverEvent::s_sRemovingTileAniName);
	}
	void teardown() override
	{
		GameFixture::teardown();
	}

	std::vector< shared_ptr<Option> > getCustomOptions() const override
	{
		return std::vector< shared_ptr<Option> >{};
	}
	std::vector<StdConfig::KeyAction> getCustomKeyActions() const override
	{
		return std::vector<StdConfig::KeyAction>{};
	}
	int32_t boardGetDelStage(Level& oLevel, int32_t nX, int32_t nY, int32_t nTileAni
							, int32_t nTotSteps, int32_t nViewTick = 0, int32_t nTotViewTicks = 1)
	{
		const double fElapsed = oLevel.boardGetTileAniElapsed(nX, nY, nTileAni, nViewTick, nTotViewTicks);
		if (fElapsed == TileAnimator::s_fInactiveElapsed) {
			return -1;
		}
		REQUIRE( fElapsed >= 0.0);
		REQUIRE( fElapsed <= 1.0);
		assert(nTotSteps > 0);
		const int32_t nInterval = fElapsed * nTotSteps;
		return (nTotSteps - nInterval);
	}
protected:
	int32_t m_nTileAniRemovingIndex;
};

class RowRemoverCase1GameFixture : public RowRemoverGameFixture
{
protected:
	void setup() override
	{
		RowRemoverGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		RowRemoverEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_nRepeat = 1;
		oInit.m_nStep = 1;
		oInit.m_nFromGaps = 0;
		oInit.m_nToGaps = 0;
		oInit.m_nDeleteAfter = 0;
		oInit.m_nDeleteAdd = 0;
		auto refRowRemoverEvent = make_unique<RowRemoverEvent>(std::move(oInit));
		m_p0RowRemoverEvent = refRowRemoverEvent.get();
		p0Level->addEvent(std::move(refRowRemoverEvent));
		p0Level->activateEvent(m_p0RowRemoverEvent, 0);
	}
	void teardown() override
	{
		RowRemoverGameFixture::teardown();
	}
	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		const int32_t nY = nBoardH - 1;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			aBoard[nX + nY * nBoardW] = oTile;
		}
	}
protected:
	shared_ptr<Level> m_refLevel;
	RowRemoverEvent* m_p0RowRemoverEvent;
};

class RowRemoverCase2GameFixture : public RowRemoverGameFixture
{
protected:
	void setup() override
	{
		RowRemoverGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		RowRemoverEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_nRepeat = 1;
		oInit.m_nStep = 1;
		oInit.m_nFromGaps = 0;
		oInit.m_nToGaps = 0;
		oInit.m_nDeleteAfter = 2;
		oInit.m_nDeleteAdd = 1;
		auto refRowRemoverEvent = make_unique<RowRemoverEvent>(std::move(oInit));
		m_p0RowRemoverEvent = refRowRemoverEvent.get();
		p0Level->addEvent(std::move(refRowRemoverEvent));
		p0Level->activateEvent(m_p0RowRemoverEvent, 0);
	}
	void teardown() override
	{
		RowRemoverGameFixture::teardown();
	}
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		Tile oTileA;
		Tile oTileB;
		Tile oTileC;
		oTileA.getTileColor().setColorPal(2);
		oTileB.getTileColor().setColorPal(3);
		oTileC.getTileColor().setColorPal(4);
		const int32_t nY = 0;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			aBoard[nX + nY * nBoardW] = oTileA;
			aBoard[nX + (nY + 1) * nBoardW] = oTileB;
			aBoard[nX + (nY + 2) * nBoardW] = oTileC;
		}
	}
protected:
	shared_ptr<Level> m_refLevel;
	RowRemoverEvent* m_p0RowRemoverEvent;
};

class RowRemoverBlockGameFixture : public RowRemoverGameFixture
{
protected:
	void setup() override
	{
		RowRemoverGameFixture::setup();

		const int32_t nLevel = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		RowRemoverEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_nRepeat = 1;
		oInit.m_nStep = 1;
		oInit.m_nFromGaps = getGaps();
		oInit.m_nToGaps = oInit.m_nFromGaps;
		oInit.m_nDeleteAfter = 0;
		oInit.m_nDeleteAdd = 0;
		auto refRowRemoverEvent = make_unique<RowRemoverEvent>(std::move(oInit));
		m_p0RowRemoverEvent = refRowRemoverEvent.get();
		p0Level->addEvent(std::move(refRowRemoverEvent));
		p0Level->activateEvent(m_p0RowRemoverEvent, 3);
		//
		m_nBombTileCharIdx = m_refGame->getNamed().chars().addName("BOMB");
		//
		Block oBlock = getInitialBlock();
		NPoint m_oInitBlockPos = getInitialBlockPos();
		const int32_t nLevelTeam = 0;
		const int32_t nInitShape = 0;
		auto refCTS = make_unique<CharTraitSet>(make_unique<CharIndexTraitSet>(m_nBombTileCharIdx));
		//TileSelector oTS{make_unique<TileSelector::Trait>(false, std::move(refCTS))};
		auto refSelect = make_unique<TileSelector>(make_unique<TileSelector::Trait>(false, std::move(refCTS)));
		BlockEvent::Init oBEInit;
		oBEInit.m_p0Level = p0Level;
		oBEInit.m_oBlock = std::move(oBlock);
		oBEInit.m_nInitShape = nInitShape;
		oBEInit.m_oInitPos = m_oInitBlockPos;
		oBEInit.m_bControllable = true;
		oBEInit.m_nLevelTeam = nLevelTeam;
		oBEInit.m_refBomb = std::move(refSelect);
		auto refBlockEvent = make_unique<BlockEvent>(std::move(oBEInit));
		m_p0BlockEvent = refBlockEvent.get();
		p0Level->addEvent(std::move(refBlockEvent));
		p0Level->activateEvent(m_p0BlockEvent, 0);
	}
	void teardown() override
	{
		RowRemoverGameFixture::teardown();
	}

	virtual int32_t getGaps() { return 0; }
	virtual Block getInitialBlock() = 0;
	virtual NPoint getInitialBlockPos() = 0;

	std::vector< shared_ptr<Option> > getCustomOptions() const override
	{
		std::vector< shared_ptr<Option> > aOptions;
		aOptions.push_back(std::make_shared<BoolOption>(OwnerType::PLAYER, BlockEvent::s_sPlayerOptionClockwiseRotation, false, "Clockwise rot."));
		return aOptions;
	}
	std::vector<StdConfig::KeyAction> getCustomKeyActions() const override
	{
		std::vector<StdConfig::KeyAction> aKeyActions;
		aKeyActions.push_back({{BlockEvent::s_sKeyActionRotate}, "Rotate block", {
			{stmi::KeyCapability::getClass(), {stmi::HK_UP}}
			, {stmi::PointerCapability::getClass(), {stmi::HK_SCROLLUP}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_UP}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionDown}, "Move down", {
			{stmi::KeyCapability::getClass(), {stmi::HK_DOWN}}
			, {stmi::PointerCapability::getClass(), {stmi::HK_SCROLLDOWN}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_DOWN}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionDrop}, "Drop", {
			{stmi::KeyCapability::getClass(), {stmi::HK_SPACE}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_BTN_B}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionLeft}, "Left", {
			{stmi::KeyCapability::getClass(), {stmi::HK_LEFT}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_LEFT}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionRight}, "Right", {
			{stmi::KeyCapability::getClass(), {stmi::HK_RIGHT}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_RIGHT}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionNext}, "Next", {
			{stmi::KeyCapability::getClass(), {stmi::HK_N}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_BTN_A}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionInvertRotation}, "Invert rotation", {
			{stmi::KeyCapability::getClass(), {stmi::HK_B}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_BTN_X}}
		}});
		return aKeyActions;
	}
protected:
	shared_ptr<Level> m_refLevel;
	RowRemoverEvent* m_p0RowRemoverEvent;
	BlockEvent* m_p0BlockEvent;
	int32_t m_nBombTileCharIdx;
};

class RowRemoverBlockCase3GameFixture : public RowRemoverBlockGameFixture
{
protected:
	void setup() override
	{
		RowRemoverBlockGameFixture::setup();
	}
	void teardown() override
	{
		RowRemoverBlockGameFixture::teardown();
	}

	Block getInitialBlock() override
	{
		Block oBlock;
		Tile oTile;
		oTile.getTileChar().setChar(65);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTile, 0, 0, true);
		assert(!oBlock.isEmpty());
		return oBlock;
	}
	NPoint getInitialBlockPos() override
	{
		NPoint m_oInitBlockPos;
		m_oInitBlockPos.m_nX = 3;
		m_oInitBlockPos.m_nY = 2;
		return m_oInitBlockPos;
	}

	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		const int32_t nY = nBoardH - 1;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			aBoard[nX + nY * nBoardW] = oTile;
		}
		for (int32_t nX = 0; nX < 3; ++nX) {
			aBoard[nX + 2 * nBoardW] = oTile;
		}
	}
};

class RowRemoverBlockCase4GameFixture : public RowRemoverBlockGameFixture
{
protected:
	void setup() override
	{
		RowRemoverBlockGameFixture::setup();
	}
	void teardown() override
	{
		RowRemoverBlockGameFixture::teardown();
	}

	Block getInitialBlock() override
	{
		Block oBlock;
		Tile oTile;
		oTile.getTileChar().setChar(65);
		oBlock.brickAdd(oTile, 0, 0, true);
		oBlock.brickAdd(oTile, 0, 1, true);
		oBlock.brickAdd(oTile, 1, 0, true);
		oBlock.brickAdd(oTile, 1, 1, true);
		return oBlock;
	}
	NPoint getInitialBlockPos() override
	{
		NPoint m_oInitBlockPos;
		m_oInitBlockPos.m_nX = 3;
		m_oInitBlockPos.m_nY = 2;
		return m_oInitBlockPos;
	}

	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		const int32_t nY = nBoardH - 1;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			aBoard[nX + nY * nBoardW] = oTile;
		}
		for (int32_t nX = 0; nX < 3; ++nX) {
			aBoard[nX + 2 * nBoardW] = oTile;
		}
	}
};

class RowRemoverBlockCase5GameFixture : public RowRemoverBlockGameFixture
{
protected:
	void setup() override
	{
		RowRemoverBlockGameFixture::setup();
	}
	void teardown() override
	{
		RowRemoverBlockGameFixture::teardown();
	}

	int32_t getGaps() override { return 2; }

	Block getInitialBlock() override
	{
		Block oBlock;
		Tile oTile;
		oTile.getTileChar().setChar(65);
		oBlock.brickAdd(oTile, 0, 0, true);
		oBlock.brickAdd(oTile, 0, 1, true);
		oBlock.brickAdd(oTile, 1, 0, true);
		oBlock.brickAdd(oTile, 1, 1, true);
		return oBlock;
	}
	NPoint getInitialBlockPos() override
	{
		NPoint m_oInitBlockPos;
		m_oInitBlockPos.m_nX = 2;
		m_oInitBlockPos.m_nY = 3;
		return m_oInitBlockPos;
	}

	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		const int32_t nY = nBoardH - 1;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			if ((nX < 2) || (nX >= 4)) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
		for (int32_t nX = 0; nX < 3; ++nX) {
			aBoard[nX + 2 * nBoardW] = oTile;
		}
	}
};

class RowRemoverBlockCase6GameFixture : public RowRemoverBlockGameFixture
{
protected:
	void setup() override
	{
		RowRemoverBlockGameFixture::setup();
	}
	void teardown() override
	{
		RowRemoverBlockGameFixture::teardown();
	}

	Block getInitialBlock() override
	{
		Block oBlock;
		Tile oTile;
		oTile.getTileChar().setChar(65);
		Tile oBombTile;
		oBombTile.getTileChar().setCharIndex(m_nBombTileCharIdx);
		oBlock.brickAdd(oTile, 0, 0, true);
		oBlock.brickAdd(oTile, 1, 0, true);
		oBlock.brickAdd(oBombTile, 0, 1, true);
		oBlock.brickAdd(oBombTile, 1, 1, true);
		return oBlock;
	}
	NPoint getInitialBlockPos() override
	{
		NPoint m_oInitBlockPos;
		m_oInitBlockPos.m_nX = 2;
		m_oInitBlockPos.m_nY = 3;
		return m_oInitBlockPos;
	}

	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		const int32_t nY = nBoardH - 1;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			aBoard[nX + nY * nBoardW] = oTile;
		}
		for (int32_t nX = 0; nX < 3; ++nX) {
			aBoard[nX + 2 * nBoardW] = oTile;
		}
	}
};

class RowRemoverBlockCase7GameFixture : public RowRemoverBlockGameFixture
{
protected:
	void setup() override
	{
		RowRemoverBlockGameFixture::setup();
	}
	void teardown() override
	{
		RowRemoverBlockGameFixture::teardown();
	}

	Block getInitialBlock() override
	{
		Block oBlock;
		Tile oTile;
		oTile.getTileChar().setChar(65);
		Tile oBombTile;
		oBombTile.getTileChar().setCharIndex(m_nBombTileCharIdx);
		oBlock.brickAdd(oBombTile, 0, 0, true);
		oBlock.brickAdd(oBombTile, 1, 0, true);
		oBlock.brickAdd(oTile, 0, 1, true);
		oBlock.brickAdd(oTile, 1, 1, true);
		return oBlock;
	}
	NPoint getInitialBlockPos() override
	{
		NPoint m_oInitBlockPos;
		m_oInitBlockPos.m_nX = 2;
		m_oInitBlockPos.m_nY = 3;
		return m_oInitBlockPos;
	}

	void fillBoard(int32_t nBoardW, int32_t nBoardH, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		const int32_t nY = nBoardH - 1;
		for (int32_t nX = 0; nX < nBoardW; ++nX) {
			aBoard[nX + nY * nBoardW] = oTile;
		}
		for (int32_t nX = 0; nX < 3; ++nX) {
			aBoard[nX + 2 * nBoardW] = oTile;
		}
	}
};

TEST_CASE_METHOD(STFX<RowRemoverCase1GameFixture>, "Case1_RemoveOneRow")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(4);

	const int32_t nTotViewTicks = 4;
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	REQUIRE( ! p0Level->boardGetTile(0, 5).isEmpty() );
	REQUIRE( p0Level->boardGetTileAnimator(0, 5, m_nTileAniRemovingIndex) == nullptr );
	m_refGame->handleTimer(); // start row remover
	REQUIRE( m_refGame->gameElapsed() == 1 );
	REQUIRE( p0Level->boardGetTile(0, 5).isEmpty() );
	REQUIRE( p0Level->boardGetTileAnimator(0, 5, m_nTileAniRemovingIndex) == nullptr );
	REQUIRE( p0Level->boardGetTileAniElapsed(0, 5, m_nTileAniRemovingIndex, 0, nTotViewTicks) < 0.0 );
}

TEST_CASE_METHOD(STFX<RowRemoverCase2GameFixture>, "Case2_DeleteAfterAndAdd")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(4);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	REQUIRE( ! p0Level->boardGetTile(3, 0).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(4, 1).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(5, 2).isEmpty() );

	m_refGame->handleTimer(); // start row remover
	REQUIRE( m_refGame->gameElapsed() == 1 );
	REQUIRE( ! p0Level->boardGetTile(4, 0).isEmpty() );
	REQUIRE( p0Level->boardGetTileAnimator(3, 0, m_nTileAniRemovingIndex) != nullptr );
	const int32_t nDeleteAfter = 2;
	const int32_t nDeleteAdd = 1;
	for (int32_t nCurY = 0; nCurY < 3; ++nCurY) {
		const int32_t nDelStage = boardGetDelStage(*p0Level, 7, nCurY, m_nTileAniRemovingIndex, nDeleteAfter + nDeleteAdd * (2 - nCurY), 0, 3);
		REQUIRE( nDelStage == (nDeleteAfter + nDeleteAdd * (2 - nCurY)));
	}

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );
	for (int32_t nCurY = 0; nCurY < 3; ++nCurY) {
		const int32_t nDelStage = boardGetDelStage(*p0Level, 7, nCurY, m_nTileAniRemovingIndex, nDeleteAfter + nDeleteAdd * (2 - nCurY), 0, 3);
		REQUIRE( nDelStage == (nDeleteAfter + nDeleteAdd * (2 - nCurY) - 1 ));
		REQUIRE( ! p0Level->boardGetTile(0, nCurY).isEmpty() );
		const Tile& oTile = p0Level->boardGetTile(3, nCurY);
		REQUIRE( oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
		REQUIRE( static_cast<int32_t>(oTile.getTileColor().getColorPal()) == (2 + nCurY) );
		REQUIRE( p0Level->boardGetTileAnimator(0, nCurY, m_nTileAniRemovingIndex) != nullptr );
	}

	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 3 ); // row nY==2 is deleted and rows 0 and 1 moved down
	for (int32_t nCurY = 1; nCurY < 3; ++nCurY) {
		const int32_t nDelStage = boardGetDelStage(*p0Level, 7, nCurY, m_nTileAniRemovingIndex, nDeleteAfter + nDeleteAdd * (2 - (nCurY - 1)), 0, 3);
		REQUIRE( nDelStage == (nDeleteAfter + nDeleteAdd * (2 - (nCurY - 1)) - 2 ));
		REQUIRE( ! p0Level->boardGetTile(0, nCurY).isEmpty() );
		const Tile& oTile = p0Level->boardGetTile(0, nCurY);
		REQUIRE( oTile.getTileColor().getColorType() == TileColor::COLOR_TYPE_PAL);
		REQUIRE( static_cast<int32_t>(oTile.getTileColor().getColorPal()) == (2 + (nCurY - 1)) );
		REQUIRE( p0Level->boardGetTileAnimator(0, nCurY, m_nTileAniRemovingIndex) != nullptr );
	}
	{
		const int32_t nCurY = 0;
		const int32_t nDelStage = boardGetDelStage(*p0Level, 7, nCurY, m_nTileAniRemovingIndex, nDeleteAfter + nDeleteAdd * (2 - nCurY), 0, 3);
		REQUIRE( nDelStage < 0);
		REQUIRE( p0Level->boardGetTile(0, nCurY).isEmpty() );
		REQUIRE( p0Level->boardGetTileAnimator(0, nCurY, m_nTileAniRemovingIndex) == nullptr );
	}
}

TEST_CASE_METHOD(STFX<RowRemoverBlockCase3GameFixture>, "Case3_PushDown")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(8);

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 77161;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_REMOVED, p0LogEvent, 33);
	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 44);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place block on board
	REQUIRE( m_refGame->gameElapsed() == 1 );

	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}

	//shared_ptr<StdPreferences::Player>& refPlayer = m_refPrefs->getPlayerFull(0);
	//auto oPairCapaKey = refPlayer->getKeyValue(BlockEvent::s_sKeyActionDown);
	//assert(oPairCapaKey.second == stmi::HK_DOWN);
	//assert(m_aKeyDeviceIds.size() > 0);
	//const auto nKeyDevId = oPairCapaKey.first->getDevice()->getId();
	assert(m_aKeyDeviceIds.size() == 1);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{3, 2});

	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_DOWN);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_DOWN);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{3, 3});

	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_LEFT);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_LEFT);
	m_refGame->handleTimer(); // RowRemover started
	REQUIRE( m_refGame->gameElapsed() == 3 );

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 3});

	REQUIRE( LogEvent::msgLog().totBuffered() == 0 );

	m_refGame->handleTimer(); // remove and pushes down
	REQUIRE( m_refGame->gameElapsed() == 4 );

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 4});

	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 77161 );
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 33);
		REQUIRE( oEntry.m_nValue == 1 ); // one row was removed
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
	m_refGame->handleTimer(); // RowRemover is finished
	REQUIRE( m_refGame->gameElapsed() == 5 );
	REQUIRE( LogEvent::msgLog().totBuffered() == 2 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 77161 );
		REQUIRE( oEntry.m_nGameTick == 4 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 44);
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
}

TEST_CASE_METHOD(STFX<RowRemoverBlockCase4GameFixture>, "Case4_PushDownFreeze")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(8);

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 67753;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_REMOVED, p0LogEvent, 33);
	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FREEZED, p0LogEvent, 77);

	m_refGame->start();

	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place block on board
	REQUIRE( m_refGame->gameElapsed() == 1 );

	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}

	assert(m_aKeyDeviceIds.size() == 1);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{3, 2});

	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_DOWN);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_DOWN);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2 );

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{3, 3});

	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_LEFT);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_LEFT);
	m_refGame->handleTimer(); // RowRemover started
	REQUIRE( m_refGame->gameElapsed() == 3 );

	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 3});

	REQUIRE( LogEvent::msgLog().totBuffered() == 0 );

	m_refGame->handleTimer(); // remove and pushes down and freezes the block
	REQUIRE( m_refGame->gameElapsed() == 4 );

	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 0);
	}

	REQUIRE( LogEvent::msgLog().totBuffered() == 2 );
	int32_t nRemovedBack = 0;
	{
		const int32_t nFreezedBack = LogEvent::msgLog().findEntry([&](const LogEvent::MsgLog::Entry& oEntry)
		{
			return (oEntry.m_nMsg == 77);
		});
		REQUIRE( nFreezedBack >= 0);
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(nFreezedBack);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 67753 );
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 77);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0BlockEvent) );
		if (nFreezedBack == 0) {
			nRemovedBack = 1;
		}
	}
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(nRemovedBack);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 67753 );
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 33);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
}
TEST_CASE_METHOD(STFX<RowRemoverBlockCase5GameFixture>, "Case5_PushDownIntoGap")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(8);

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 61153;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_REMOVED, p0LogEvent, 33);
	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FREEZED, p0LogEvent, 77);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	for (int nCnt = 1; nCnt <= 3; ++nCnt) {
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == nCnt );
	}
	LogEvent::msgLog().reset();
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}
	// row remover has started
	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 3});

	REQUIRE( ! p0Level->boardGetTile(0, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(1, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(2, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(1, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 3).isEmpty() );

	m_refGame->handleTimer(); // remove and push down into gap
	REQUIRE( m_refGame->gameElapsed() == 4 );

	REQUIRE( p0Level->boardGetTile(0, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(1, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 3).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(1, 3).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(2, 3).isEmpty() );
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}
	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 4});
	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 61153 );
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 33);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
}

TEST_CASE_METHOD(STFX<RowRemoverBlockCase6GameFixture>, "Case6_PushDownMutilating")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(8);

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 61453;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_REMOVED, p0LogEvent, 33);
	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FREEZED, p0LogEvent, 77);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	for (int nCnt = 1; nCnt <= 3; ++nCnt) {
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == nCnt );
	}
	LogEvent::msgLog().reset();
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}
	// row remover has started
	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 3});
	REQUIRE( m_p0BlockEvent->blockSize() == NSize{2, 2});

	REQUIRE( ! p0Level->boardGetTile(0, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(1, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(2, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(1, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 3).isEmpty() );

	m_refGame->handleTimer(); // remove and push down to mutilate bottom row of block
	REQUIRE( m_refGame->gameElapsed() == 4 );

	REQUIRE( p0Level->boardGetTile(0, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(1, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 3).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(1, 3).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(2, 3).isEmpty() );
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}
	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 4});
	REQUIRE( m_p0BlockEvent->blockSize() == NSize{2, 1});
	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 61453 );
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 33);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
}

TEST_CASE_METHOD(STFX<RowRemoverBlockCase7GameFixture>, "Case7_PushDownOnBombsMutilating")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(8);

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 61453;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_REMOVED, p0LogEvent, 33);
	m_p0RowRemoverEvent->addListener(RowRemoverEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FREEZED, p0LogEvent, 77);

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	for (int nCnt = 1; nCnt <= 3; ++nCnt) {
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == nCnt );
	}
	LogEvent::msgLog().reset();
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}
	// row remover has started
	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 3});
	REQUIRE( m_p0BlockEvent->blockSize() == NSize{2, 2});
	REQUIRE( m_p0BlockEvent->blockBricksTotVisible() == 4);

	REQUIRE( ! p0Level->boardGetTile(0, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(1, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(2, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(0, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(1, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 3).isEmpty() );

	m_refGame->handleTimer(); // remove row and push down to mutilate top row of block
	REQUIRE( m_refGame->gameElapsed() == 4 );

	REQUIRE( p0Level->boardGetTile(0, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(1, 2).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 2).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(0, 3).isEmpty() );
	REQUIRE( ! p0Level->boardGetTile(1, 3).isEmpty() );
	REQUIRE( p0Level->boardGetTile(2, 3).isEmpty() ); // exploded

	REQUIRE( p0Level->boardGetOwner(2, 3) == nullptr ); // exploded
	REQUIRE( p0Level->boardGetOwner(3, 3) != nullptr );
	REQUIRE( p0Level->boardGetOwner(2, 4) != nullptr );
	REQUIRE( p0Level->boardGetOwner(3, 4) != nullptr );
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		REQUIRE( m_p0BlockEvent == aLBs[0] );
	}
	REQUIRE( m_p0BlockEvent->blockPos() == NPoint{2, 3});
	REQUIRE( m_p0BlockEvent->blockSize() == NSize{2, 2});
	REQUIRE( m_p0BlockEvent->blockBricksTotVisible() == 3);
	REQUIRE( LogEvent::msgLog().totBuffered() == 1 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(0);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 61453 );
		REQUIRE( oEntry.m_nGameTick == 3 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 33);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0RowRemoverEvent) );
	}
}

} // namespace testing

} // namespace stmg

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
 * File:   testBlockEvent.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "blockevent.h"

#include <stmm-games-fake/fixtureGame.h>
#include <stmm-games-fake/fakelevelview.h>

#include <stmm-games/events/sysevent.h>
#include <stmm-games/events/logevent.h>
#include <stmm-games/traitsets//tiletraitsets.h>
#include <stmm-games/utile/tileselector.h>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

class BlockEventGameFixture : public GameFixture
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
		assert(m_refPrefs->getTotPlayers() > 0);
		m_refPlayer0 = m_refPrefs->getPlayerFull(0);
		assert(m_refPlayer0);
		assert(m_aKeyDeviceIds.size() > 0);
		const int32_t nKeyDeviceId = m_aKeyDeviceIds[0];
		auto refKeyDevice = m_refDM->getDevice(nKeyDeviceId);
		auto refKeyCapa = refKeyDevice->getCapability(stmi::KeyCapability::getClass());
		{
		const int32_t nKeyActionId = m_refStdConfig->getKeyActionId(BlockEvent::s_sKeyActionDown);
		m_refPlayer0->setKeyValue(nKeyActionId, refKeyCapa.get(), stmi::HK_DOWN);
		}
		{
		const int32_t nKeyActionId = m_refStdConfig->getKeyActionId(BlockEvent::s_sKeyActionLeft);
		m_refPlayer0->setKeyValue(nKeyActionId, refKeyCapa.get(), stmi::HK_LEFT);
		}
		{
		const int32_t nKeyActionId = m_refStdConfig->getKeyActionId(BlockEvent::s_sKeyActionRight);
		m_refPlayer0->setKeyValue(nKeyActionId, refKeyCapa.get(), stmi::HK_RIGHT);
		}
		{
		const int32_t nKeyActionId = m_refStdConfig->getKeyActionId(BlockEvent::s_sKeyActionRotate);
		m_refPlayer0->setKeyValue(nKeyActionId, refKeyCapa.get(), stmi::HK_UP);
		}
		{
		const int32_t nKeyActionId = m_refStdConfig->getKeyActionId(BlockEvent::s_sKeyActionDrop);
		m_refPlayer0->setKeyValue(nKeyActionId, refKeyCapa.get(), stmi::HK_SPACE);
		}
		{
		const int32_t nKeyActionId = m_refStdConfig->getKeyActionId(BlockEvent::s_sKeyActionNext);
		m_refPlayer0->setKeyValue(nKeyActionId, refKeyCapa.get(), stmi::HK_N);
		}
	}
	void teardown() override
	{
		GameFixture::teardown();
	}

	void fillBoard(int32_t /*nBoardW*/, int32_t /*nBoardH*/, std::vector<Tile>& /*aBoard*/) override
	{
	}
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
			{stmi::KeyCapability::getClass(), {stmi::HK_UP, stmi::HK_W, stmi::HK_8}}
			, {stmi::PointerCapability::getClass(), {stmi::HK_SCROLLUP}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_UP}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionDown}, "Move down", {
			{stmi::KeyCapability::getClass(), {stmi::HK_DOWN, stmi::HK_S, stmi::HK_2}}
			, {stmi::PointerCapability::getClass(), {stmi::HK_SCROLLDOWN}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_DOWN}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionDrop}, "Drop", {
			{stmi::KeyCapability::getClass(), {stmi::HK_SPACE, stmi::HK_X, stmi::HK_0}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_BTN_B}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionLeft}, "Left", {
			{stmi::KeyCapability::getClass(), {stmi::HK_LEFT, stmi::HK_A, stmi::HK_4}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_LEFT}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionRight}, "Right", {
			{stmi::KeyCapability::getClass(), {stmi::HK_RIGHT, stmi::HK_D, stmi::HK_6}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_RIGHT}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionNext}, "Next", {
			{stmi::KeyCapability::getClass(), {stmi::HK_N, stmi::HK_E, stmi::HK_9}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_BTN_A}}
		}});
		aKeyActions.push_back({{BlockEvent::s_sKeyActionInvertRotation}, "Invert rotation", {
			{stmi::KeyCapability::getClass(), {stmi::HK_B, stmi::HK_Q, stmi::HK_7}}
			, {stmi::JoystickCapability::getClass(), {stmi::HK_BTN_X}}
		}});
		return aKeyActions;
	}
protected:
	shared_ptr<StdPreferences::Player> m_refPlayer0;
};

class BlockEventCase1GameFixture : public BlockEventGameFixture
{
protected:
	void setup() override
	{
		BlockEventGameFixture::setup();

		const int32_t nLevel = 0;
		const int32_t nLevelTeam = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		Block oBlock;
		Tile oTile;
		oTile.getTileChar().setChar(65);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTile, 0, 0, true);
		assert(!oBlock.isEmpty());
		//Level* p0Level, Block&& oBlock, int32_t nInitShape, NPoint oInitPos, int32_t nLevelTeam, bool bUser, unique_ptr<TileSelector> refBomb
		const int32_t nInitShape = 0;
		m_oInitBlockPos = BlockEvent::calcInitialPos(p0Level, oBlock, 0, nInitShape);
		BlockEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_oBlock = std::move(oBlock);
		oInit.m_nInitShape = nInitShape;
		oInit.m_oInitPos = m_oInitBlockPos;
		oInit.m_bControllable = true;
		oInit.m_nLevelTeam = nLevelTeam;
		auto refBlockEvent = make_unique<BlockEvent>(std::move(oInit));
		m_p0BlockEvent = refBlockEvent.get();
		p0Level->addEvent(std::move(refBlockEvent));
		p0Level->activateEvent(m_p0BlockEvent, 0);
	}
	void teardown() override
	{
		BlockEventGameFixture::teardown();
	}
protected:
	shared_ptr<Level> m_refLevel;
	NPoint m_oInitBlockPos;
	BlockEvent* m_p0BlockEvent;
};

class BlockEventCase2GameFixture : public BlockEventGameFixture
{
protected:
	void setup() override
	{
		BlockEventGameFixture::setup();

		const int32_t nLevel = 0;
		const int32_t nLevelTeam = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		Block oBlock;
		Tile oTile;
		m_nBombTileCharIdx = m_refGame->getNamed().chars().addName("BOMB");
		oTile.getTileChar().setCharIndex(m_nBombTileCharIdx);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTile, 0, 0, true);
		assert(!oBlock.isEmpty());
		//Level* p0Level, Block&& oBlock, int32_t nInitShape, NPoint oInitPos, int32_t nLevelTeam, bool bUser, unique_ptr<TileSelector> refBomb
		const int32_t nInitShape = 0;
		m_oInitBlockPos = BlockEvent::calcInitialPos(p0Level, oBlock, 0, nInitShape);
		auto refCTS = make_unique<CharTraitSet>(make_unique<CharIndexTraitSet>(m_nBombTileCharIdx));
		//TileSelector oTS{make_unique<TileSelector::Trait>(false, std::move(refCTS))};
		auto refSelect = make_unique<TileSelector>(make_unique<TileSelector::Trait>(false, std::move(refCTS)));
		BlockEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_oBlock = std::move(oBlock);
		oInit.m_nInitShape = nInitShape;
		oInit.m_oInitPos = m_oInitBlockPos;
		oInit.m_bControllable = true;
		oInit.m_nLevelTeam = nLevelTeam;
		oInit.m_refBomb = std::move(refSelect);
		auto refBlockEvent = make_unique<BlockEvent>(std::move(oInit));
		m_p0BlockEvent = refBlockEvent.get();
		p0Level->addEvent(std::move(refBlockEvent));
		p0Level->activateEvent(m_p0BlockEvent, 0);
	}
	void teardown() override
	{
		BlockEventGameFixture::teardown();
	}
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		//aBoard[(nBoardW - 1) + (nBoardH - 1) * nBoardW] = oTile;
		for (int32_t nX = 2; nX < 2 + 3; ++nX) {
			for (int32_t nY = 3; nY < 3 + 1; ++nY) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
		}
		{
		Tile oTile;
		oTile.getTileColor().setColorPal(3);
		aBoard[2 + 2 * nBoardW] = oTile;
		}
		{
		Tile oTile;
		oTile.getTileColor().setColorPal(4);
		aBoard[4 + 2 * nBoardW] = oTile;
		}
	}
protected:
	shared_ptr<Level> m_refLevel;
	int32_t m_nBombTileCharIdx;
	NPoint m_oInitBlockPos;
	BlockEvent* m_p0BlockEvent;
};

class BlockEventCase3GameFixture : public BlockEventGameFixture
{
protected:
	void setup() override
	{
		BlockEventGameFixture::setup();

		const int32_t nLevel = 0;
		const int32_t nLevelTeam = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		Block oBlock;
		m_nBombTileCharIdx = m_refGame->getNamed().chars().addName("BOMB");
		Tile oTileA;
		oTileA.getTileColor().setColorPal(1);
		Tile oTileB;
		oTileB.getTileColor().setColorPal(2);
		Tile oTileC;
		oTileC.getTileColor().setColorPal(3);
		oTileC.getTileChar().setCharIndex(m_nBombTileCharIdx);
		Tile oTileD;
		oTileD.getTileColor().setColorPal(4);
		oTileD.getTileChar().setCharIndex(m_nBombTileCharIdx);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTileA, 0, 0, true);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTileB, 1, 0, true);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTileC, 0, 1, true);
		/*const int32_t nBrickId =*/ oBlock.brickAdd(oTileD, 1, 1, true);
		assert(!oBlock.isEmpty());
		//Level* p0Level, Block&& oBlock, int32_t nInitShape, NPoint oInitPos, int32_t nLevelTeam, bool bUser, unique_ptr<TileSelector> refBomb
		const int32_t nInitShape = 0;
		m_oInitBlockPos = BlockEvent::calcInitialPos(p0Level, oBlock, 0, nInitShape);
		auto refCTS = make_unique<CharTraitSet>(make_unique<CharIndexTraitSet>(m_nBombTileCharIdx));
		//TileSelector oTS{make_unique<TileSelector::Trait>(false, std::move(refCTS))};
		auto refSelect = make_unique<TileSelector>(make_unique<TileSelector::Trait>(false, std::move(refCTS)));
		BlockEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_oBlock = std::move(oBlock);
		oInit.m_nInitShape = nInitShape;
		oInit.m_oInitPos = m_oInitBlockPos;
		oInit.m_bControllable = true;
		oInit.m_nLevelTeam = nLevelTeam;
		oInit.m_refBomb = std::move(refSelect);
		auto refBlockEvent = make_unique<BlockEvent>(std::move(oInit));
		m_p0BlockEvent = refBlockEvent.get();
		p0Level->addEvent(std::move(refBlockEvent));
		p0Level->activateEvent(m_p0BlockEvent, 0);
	}
	void teardown() override
	{
		BlockEventGameFixture::teardown();
	}
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		for (int32_t nX = 2; nX < 2 + 3; ++nX) {
			for (int32_t nY = 3; nY < 3 + 2; ++nY) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
	}
protected:
	shared_ptr<Level> m_refLevel;
	int32_t m_nBombTileCharIdx;
	NPoint m_oInitBlockPos;
	BlockEvent* m_p0BlockEvent;
};

class BlockEventCase4GameFixture : public BlockEventGameFixture
{
protected:
	void setup() override
	{
		BlockEventGameFixture::setup();

		const int32_t nLevel = 0;
		const int32_t nLevelTeam = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();
		m_nBombTileCharIdx = m_refGame->getNamed().chars().addName("BOMB");
		Tile oTileA;
		oTileA.getTileColor().setColorPal(1);
		Tile oTileB;
		oTileB.getTileColor().setColorPal(2);
		Tile oTileC;
		oTileC.getTileColor().setColorPal(3);
		oTileC.getTileChar().setCharIndex(m_nBombTileCharIdx);
		Tile oTileD;
		oTileD.getTileColor().setColorPal(4);
		oTileD.getTileChar().setCharIndex(m_nBombTileCharIdx);
		Block oBlock1;
		/*const int32_t nBrickId =*/ oBlock1.brickAdd(oTileA, 0, 0, true);
		/*const int32_t nBrickId =*/ oBlock1.brickAdd(oTileB, 1, 0, true);
		/*const int32_t nBrickId =*/ oBlock1.brickAdd(oTileC, 0, 1, true);
		/*const int32_t nBrickId =*/ oBlock1.brickAdd(oTileD, 1, 1, true);
		Block oBlock2;
		/*const int32_t nBrickId =*/ oBlock2.brickAdd(oTileC, 0, 0, true);
		/*const int32_t nBrickId =*/ oBlock2.brickAdd(oTileD, 1, 0, true);

		const int32_t nInitShape = 0;
		NPoint oInitBlock_1_Pos;
		oInitBlock_1_Pos.m_nX = 3;
		oInitBlock_1_Pos.m_nY = 0;
		NPoint oInitBlock_2_Pos;
		oInitBlock_2_Pos.m_nX = 5;
		oInitBlock_2_Pos.m_nY = 0;
		{
		auto refCTS = make_unique<CharTraitSet>(make_unique<CharIndexTraitSet>(m_nBombTileCharIdx));
		auto refSelect = make_unique<TileSelector>(make_unique<TileSelector::Trait>(false, std::move(refCTS)));
		BlockEvent::Init oInit1;
		oInit1.m_p0Level = p0Level;
		oInit1.m_oBlock = std::move(oBlock1);
		oInit1.m_nInitShape = nInitShape;
		oInit1.m_oInitPos = oInitBlock_1_Pos;
		oInit1.m_bControllable = true;
		oInit1.m_nLevelTeam = nLevelTeam;
		oInit1.m_refBomb = std::move(refSelect);
		auto refBlock_1_Event = make_unique<BlockEvent>(std::move(oInit1));
		m_p0Block_1_Event = refBlock_1_Event.get();
		p0Level->addEvent(std::move(refBlock_1_Event));
		p0Level->activateEvent(m_p0Block_1_Event, 0);
		}
		//
		{
		auto refCTS = make_unique<CharTraitSet>(make_unique<CharIndexTraitSet>(m_nBombTileCharIdx));
		auto refSelect = make_unique<TileSelector>(make_unique<TileSelector::Trait>(false, std::move(refCTS)));
		BlockEvent::Init oInit2;
		oInit2.m_p0Level = p0Level;
		oInit2.m_oBlock = std::move(oBlock2);
		oInit2.m_nInitShape = nInitShape;
		oInit2.m_oInitPos = oInitBlock_2_Pos;
		oInit2.m_bControllable = true;
		oInit2.m_nLevelTeam = nLevelTeam;
		oInit2.m_refBomb = std::move(refSelect);
		auto refBlock_2_Event = make_unique<BlockEvent>(std::move(oInit2));
		m_p0Block_2_Event = refBlock_2_Event.get();
		p0Level->addEvent(std::move(refBlock_2_Event));
		p0Level->activateEvent(m_p0Block_2_Event, 0);
		}
	}
	void teardown() override
	{
		BlockEventGameFixture::teardown();
	}
	void fillBoard(int32_t /*nBoardW*/, int32_t /*nBoardH*/, std::vector<Tile>& /*aBoard*/) override
	{
	}
protected:
	shared_ptr<Level> m_refLevel;
	int32_t m_nBombTileCharIdx;
	BlockEvent* m_p0Block_1_Event;
	BlockEvent* m_p0Block_2_Event;
};

class BlockEventCase5GameFixture : public BlockEventGameFixture
{
protected:
	void setup() override
	{
		BlockEventGameFixture::setup();

		const int32_t nLevel = 0;
		const int32_t nLevelTeam = 0;
		m_refLevel = m_refGame->level(nLevel);
		assert(m_refLevel);
		assert( m_refLevel->boardWidth() == 10 );
		assert( m_refLevel->boardHeight() == 6 );
		Level* p0Level = m_refLevel.get();

		const int32_t nBricks = 4;
		const int32_t nShapes = 3;
		std::vector<Tile> aBricks;
		aBricks.resize(nBricks);
		for (int32_t nBrickIdx = 0; nBrickIdx < nBricks; ++nBrickIdx) {
			aBricks[nBrickIdx].getTileColor().setColorIndex(nBrickIdx + 1);
		}
		std::vector< std::vector< std::tuple<bool, int32_t, int32_t> > > aShapeBrickPos;
		aShapeBrickPos.resize(nShapes);
		for (auto& aBrickPos : aShapeBrickPos) {
			aBrickPos.resize(nBricks);
		}
		//  0123
		// 0 X
		// 1 X
		// 2 XX
		// 3
		aShapeBrickPos[0][0] = std::make_tuple(true, 1, 0);
		aShapeBrickPos[0][1] = std::make_tuple(true, 1, 1);
		aShapeBrickPos[0][2] = std::make_tuple(true, 1, 2);
		aShapeBrickPos[0][3] = std::make_tuple(true, 2, 2);
		//  0123
		// 0
		// 1XXX
		// 2X
		// 3
		aShapeBrickPos[1][0] = std::make_tuple(true, 2, 1);
		aShapeBrickPos[1][1] = std::make_tuple(true, 1, 1);
		aShapeBrickPos[1][2] = std::make_tuple(true, 0, 1);
		aShapeBrickPos[1][3] = std::make_tuple(true, 0, 2);
		//  0123
		// 0XX
		// 1 X
		// 2 X
		// 3
		aShapeBrickPos[2][0] = std::make_tuple(true, 1, 2);
		aShapeBrickPos[2][1] = std::make_tuple(true, 1, 1);
		aShapeBrickPos[2][2] = std::make_tuple(true, 1, 0);
		aShapeBrickPos[2][3] = std::make_tuple(true, 0, 0);
		Block oBlock(nBricks, aBricks, nShapes, aShapeBrickPos);
		assert(!oBlock.isEmpty());
		//Level* p0Level, Block&& oBlock, int32_t nInitShape, NPoint oInitPos, int32_t nLevelTeam, bool bUser, unique_ptr<TileSelector> refBomb
		m_nInitShape = 0;
		m_oInitBlockPos = BlockEvent::calcInitialPos(p0Level, oBlock, 0, m_nInitShape);
		BlockEvent::Init oInit;
		oInit.m_p0Level = p0Level;
		oInit.m_oBlock = std::move(oBlock);
		oInit.m_nInitShape = m_nInitShape;
		oInit.m_oInitPos = m_oInitBlockPos;
		oInit.m_bControllable = true;
		oInit.m_nLevelTeam = nLevelTeam;
		auto refBlockEvent = make_unique<BlockEvent>(std::move(oInit));
		m_p0BlockEvent = refBlockEvent.get();
		p0Level->addEvent(std::move(refBlockEvent));
		p0Level->activateEvent(m_p0BlockEvent, 0);
	}
	void teardown() override
	{
		BlockEventGameFixture::teardown();
	}
	void fillBoard(int32_t nBoardW, int32_t /*nBoardH*/, std::vector<Tile>& aBoard) override
	{
		Tile oTile;
		oTile.getTileColor().setColorPal(5);
		for (int32_t nX = 2; nX < 2 + 3; ++nX) {
			for (int32_t nY = 3; nY < 3 + 2; ++nY) {
				aBoard[nX + nY * nBoardW] = oTile;
			}
		}
	}
protected:
	shared_ptr<Level> m_refLevel;
	NPoint m_oInitBlockPos;
	int32_t m_nInitShape;
	BlockEvent* m_p0BlockEvent;
};

TEST_CASE_METHOD(STFX<BlockEventGameFixture>, "Constructor")
{
	REQUIRE(true);
}

TEST_CASE_METHOD(STFX<BlockEventCase1GameFixture>, "Case1_Down")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 64738;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FREEZED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 55);

	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		LevelBlock* p0LB = aLBs[0];
		assert(p0LB != nullptr);
		REQUIRE( p0LB->blockPos() == NPoint{m_oInitBlockPos.m_nX, m_oInitBlockPos.m_nY});
	}
	assert(m_aKeyDeviceIds.size() > 0);
	const auto nKeyDevId = m_aKeyDeviceIds[0];
	for (int32_t nDowns = 1; nDowns <= 4; ++nDowns) {
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_DOWN);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_DOWN);
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == 1 + nDowns);
		//
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		LevelBlock* p0LB = aLBs[0];
		assert(p0LB != nullptr);
		const NPoint oLBPos = p0LB->blockPos();
		REQUIRE( oLBPos.m_nX == m_oInitBlockPos.m_nX);
//std::cout << "-------------------_-->" << p0LB->blockPosY() << '\n';
		// Take fall into account (at game tick 5)
		REQUIRE( oLBPos.m_nY == m_oInitBlockPos.m_nY + nDowns + ((nDowns >= 4) ? 1 : 0));
	}
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 6);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 7);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 8);
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
	}
	m_refGame->handleTimer(); // freezes the block
	REQUIRE( m_refGame->gameElapsed() == 9);
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 0);
	}
	REQUIRE( LogEvent::msgLog().totBuffered() == 2 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(1);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 64738 );
		REQUIRE( oEntry.m_nGameTick == 8 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 44);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0BlockEvent) );
	}
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 64738 );
		REQUIRE( oEntry.m_nGameTick == 8 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 55);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0BlockEvent) );
	}
}
TEST_CASE_METHOD(STFX<BlockEventCase1GameFixture>, "Case1_Right")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(4);

	assert(m_aKeyDeviceIds.size() > 0);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		LevelBlock* p0LB = aLBs[0];
		assert(p0LB != nullptr);
		REQUIRE( p0LB->blockPos() == NPoint{m_oInitBlockPos.m_nX, m_oInitBlockPos.m_nY});
	}
	assert(p0Level->boardWidth() == 10);
	// To get to the right border
	const int32_t nTotMoveRights = 10 - m_oInitBlockPos.m_nX - 1;
//std::cout << "   nTotMoveRights=" << nTotMoveRights << '\n';
	for (int32_t nRights = 1; nRights <= nTotMoveRights; ++nRights) {
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_RIGHT);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_RIGHT);
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == 1 + nRights);

		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		LevelBlock* p0LB = aLBs[0];
		assert(p0LB != nullptr);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockPos().m_nX == m_oInitBlockPos.m_nX + nRights);
	}
	// Can no longer move right
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_RIGHT);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_RIGHT);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 1 + nTotMoveRights + 1);
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		LevelBlock* p0LB = aLBs[0];
		assert(p0LB != nullptr);
		REQUIRE( p0LB->blockPos().m_nX == m_oInitBlockPos.m_nX + nTotMoveRights);
	}
}

TEST_CASE_METHOD(STFX<BlockEventCase2GameFixture>, "Case2_BombDown")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 72673;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	FakeLevelView oLevelView(m_refGame.get(), p0Level);
	m_refGame->setLevelView(0, &oLevelView);

	LogEvent::msgLog().reset();

	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_DESTROYED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 55);

	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );
	LevelBlock* p0LB = nullptr;
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		p0LB = aLBs[0];
		assert(p0LB != nullptr);
		REQUIRE( p0LB->blockPos() == NPoint{4, 0});
	}
	assert(m_aKeyDeviceIds.size() > 0);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	{
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_LEFT);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_LEFT);
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == 2);

		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockPos().m_nX == 3);
	}
	{
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_DOWN);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_DOWN);
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == 3);

		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockPos().m_nY == 1);
	}
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 4);
	m_refGame->handleTimer(); // fall
	REQUIRE( m_refGame->gameElapsed() == 5);
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockPos().m_nY == 2);
	}
	REQUIRE( p0Level->boardGetOwner(3, 2) == p0LB);
	REQUIRE( ! p0Level->boardGetTile(3, 3).isEmpty());
	{
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_DOWN);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_DOWN);
		m_refGame->handleTimer(); // destroys the block against the board
		REQUIRE( m_refGame->gameElapsed() == 6);

		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 0);
	}
	REQUIRE( p0Level->boardGetOwner(3, 2) == nullptr);
	REQUIRE( p0Level->boardGetTile(3, 3).isEmpty());
	REQUIRE( LogEvent::msgLog().totBuffered() == 2 );
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last(1);
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 72673 );
		REQUIRE( oEntry.m_nGameTick == 5 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 44);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0BlockEvent) );
	}
	{
		const LogEvent::MsgLog::Entry& oEntry = LogEvent::msgLog().last();
		REQUIRE_FALSE( oEntry.isEmpty() );
		REQUIRE( oEntry.m_nTag == 72673 );
		REQUIRE( oEntry.m_nGameTick == 5 );
		REQUIRE( oEntry.m_nLevel== 0 );
		REQUIRE( oEntry.m_nMsg == 55);
		//REQUIRE( oEntry.m_nValue == 0 );
		REQUIRE( oEntry.m_nTriggeringEventAdr == reinterpret_cast<int64_t>(m_p0BlockEvent) );
	}
	// 2 explosion animations should have been created
	REQUIRE( oLevelView.getCalled<FakeLevelView::AnimationCreate>().size() == 1);
}
TEST_CASE_METHOD(STFX<BlockEventCase3GameFixture>, "Case3_BombBlockMutilation")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 90919;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	FakeLevelView oLevelView(m_refGame.get(), p0Level);
	m_refGame->setLevelView(0, &oLevelView);

	LogEvent::msgLog().reset();

	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_DESTROYED, p0LogEvent, 44);
	m_p0BlockEvent->addListener(BlockEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 55);

	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );
	LevelBlock* p0LB = nullptr;
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
		p0LB = aLBs[0];
		assert(p0LB != nullptr);
		REQUIRE( p0LB->blockPos() == NPoint{4, 0});
	}
	assert(m_aKeyDeviceIds.size() > 0);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	{
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_LEFT);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_LEFT);
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == 2);

		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockPos().m_nX == 3);
	}
	{
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_DOWN);
		m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_DOWN);
		m_refGame->handleTimer();
		REQUIRE( m_refGame->gameElapsed() == 3);

		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockPos().m_nY == 1);
		REQUIRE( p0LB->blockSize() == NSize{2,2});
	}
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 4);
	m_refGame->handleTimer(); // fall
	REQUIRE( m_refGame->gameElapsed() == 5);
	{
		auto aLBs = p0Level->blocksGetAll();
		REQUIRE( aLBs.size() == 1);
//std::cout << "-------------------_-->" << p0LB->blockPosX() << '\n';
		REQUIRE( p0LB->blockSize() == NSize{2, 1});
		REQUIRE( p0LB->blockPos().m_nY == 2);
	}
	REQUIRE( p0Level->boardGetOwner(3, 2) == p0LB);
	REQUIRE( p0Level->boardGetOwner(4, 2) == p0LB);
	REQUIRE( p0Level->boardGetOwner(3, 3) == nullptr);
	REQUIRE( p0Level->boardGetOwner(4, 3) == nullptr);
	REQUIRE( p0Level->boardGetTile(3, 3).isEmpty());
	REQUIRE( p0Level->boardGetTile(4, 3).isEmpty());

	REQUIRE( LogEvent::msgLog().totBuffered() == 0 );

	// 2 explosion animations should have been created
	REQUIRE( oLevelView.getCalled<FakeLevelView::AnimationCreate>().size() == 2);
}

TEST_CASE_METHOD(STFX<BlockEventCase4GameFixture>, "Case4_Fusion")
{
	Level* p0Level = m_refLevel.get();

	LogEvent::Init oLInit;
	oLInit.m_p0Level = p0Level;
	oLInit.m_bToStdOut = false;
	oLInit.m_nTag = 85515;
	auto refLogEvent = make_unique<LogEvent>(std::move(oLInit));
	LogEvent* p0LogEvent = refLogEvent.get();
	p0Level->addEvent(std::move(refLogEvent));
	REQUIRE_FALSE( p0LogEvent->isActive() );

	LogEvent::msgLog().reset();

	m_p0Block_1_Event->addListener(BlockEvent::LISTENER_GROUP_COULD_PLACE, p0LogEvent, 44);
	m_p0Block_1_Event->addListener(BlockEvent::LISTENER_GROUP_FUSED_TO, p0LogEvent, 54);
	m_p0Block_1_Event->addListener(BlockEvent::LISTENER_GROUP_FUSED_WITH, p0LogEvent, 64);
	m_p0Block_1_Event->addListener(BlockEvent::LISTENER_GROUP_DESTROYED, p0LogEvent, 74);
	m_p0Block_1_Event->addListener(BlockEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 84);

	m_p0Block_2_Event->addListener(BlockEvent::LISTENER_GROUP_COULD_PLACE, p0LogEvent, 48);
	m_p0Block_2_Event->addListener(BlockEvent::LISTENER_GROUP_FUSED_TO, p0LogEvent, 58);
	m_p0Block_2_Event->addListener(BlockEvent::LISTENER_GROUP_FUSED_WITH, p0LogEvent, 68);
	m_p0Block_2_Event->addListener(BlockEvent::LISTENER_GROUP_DESTROYED, p0LogEvent, 78);
	m_p0Block_2_Event->addListener(BlockEvent::LISTENER_GROUP_FINISHED, p0LogEvent, 88);

	assert(m_aKeyDeviceIds.size() > 0);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	p0Level->setFallEachTicks(4);
	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );
	REQUIRE( LogEvent::msgLog().totBuffered() == 2 );
	REQUIRE( LogEvent::msgLog().findEntry([](const LogEvent::MsgLog::Entry& oEntry)
	{
		return (oEntry.m_nMsg == 44);
	}) >= 0);
	REQUIRE( LogEvent::msgLog().findEntry([](const LogEvent::MsgLog::Entry& oEntry)
	{
		return (oEntry.m_nMsg == 48);
	}) >= 0);
	LogEvent::msgLog().reset();

	auto aLBs = p0Level->blocksGetAll();
	REQUIRE( aLBs.size() == 2);
	REQUIRE(( (m_p0Block_1_Event == aLBs[0]) || (m_p0Block_1_Event == aLBs[1]) ));
	REQUIRE(( (m_p0Block_2_Event == aLBs[0]) || (m_p0Block_2_Event == aLBs[1]) ));

	REQUIRE(( (m_p0Block_1_Event->getPlayer() >= 0) || (m_p0Block_2_Event->getPlayer() >= 0) ));
	LevelBlock* p0Controlled = ((m_p0Block_1_Event->getPlayer() >= 0) ? m_p0Block_1_Event : m_p0Block_2_Event);
	REQUIRE( p0Controlled != nullptr);

	const bool bControlledIsLB1 = (p0Controlled == m_p0Block_1_Event);
	const stmi::HARDWARE_KEY eKey = (bControlledIsLB1 ? stmi::HK_RIGHT : stmi::HK_LEFT);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, eKey);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, eKey);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2);
	REQUIRE( p0Controlled->getPlayer() >= 0);

	REQUIRE( p0Controlled->blockSize() == NSize{4, 2} );

	REQUIRE( LogEvent::msgLog().totBuffered() == 3 );
	REQUIRE( LogEvent::msgLog().findEntry([&](const LogEvent::MsgLog::Entry& oEntry)
	{
		return (oEntry.m_nMsg == (bControlledIsLB1 ? 64 : 68));
	}) >= 0);
	REQUIRE( LogEvent::msgLog().findEntry([&](const LogEvent::MsgLog::Entry& oEntry)
	{
		return (oEntry.m_nMsg == (bControlledIsLB1 ? 88 : 84));
	}) >= 0);
	REQUIRE( LogEvent::msgLog().findEntry([&](const LogEvent::MsgLog::Entry& oEntry)
	{
		return (oEntry.m_nMsg == (bControlledIsLB1 ? 58 : 54));
	}) >= 0);
}

TEST_CASE_METHOD(STFX<BlockEventCase5GameFixture>, "Case5_Rotate")
{
	Level* p0Level = m_refLevel.get();
	p0Level->setFallEachTicks(4);

	assert(m_aKeyDeviceIds.size() > 0);
	const auto nKeyDevId = m_aKeyDeviceIds[0];

	m_refGame->start();
	REQUIRE( m_refGame->gameElapsed() == 0 );
	m_refGame->handleTimer(); // place on board
	REQUIRE( m_refGame->gameElapsed() == 1 );

	auto aLBs = p0Level->blocksGetAll();
	REQUIRE( aLBs.size() == 1);
	assert(aLBs[0] == m_p0BlockEvent);
	REQUIRE( m_p0BlockEvent->blockGetShapeId() == m_nInitShape);

	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_PRESS, stmi::HK_UP);
	m_refDM->simulateKeyEvent(nKeyDevId, stmi::KeyEvent::KEY_RELEASE, stmi::HK_UP);
	m_refGame->handleTimer();
	REQUIRE( m_refGame->gameElapsed() == 2);
	REQUIRE( m_p0BlockEvent->blockGetShapeId() != m_nInitShape);
}



} // namespace testing

} // namespace stmg

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
 * File:   testLoadGames.cxx
 */

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "setupstdconfig.h"
#include "setupxmlgameloader.h"
#include "setupxmlthemeloader.h"

#include "testconfig.h"

#include <stmm-games-xml-gtk/gamediskfiles.h>
#include <stmm-games-xml-gtk/xmlthemeloader.h>
#include <stmm-games-xml-game/xmlgameloader.h>

#include <stmm-games-gtk/theme.h>

#include <stmm-games-fake/fixtureTestBase.h>

#include <stmm-games/stdpreferences.h>
#include <stmm-games/gameowner.h>

#include <stmm-input-fake/fakedevicemanager.h>
#include <stmm-input-fake/fakekeydevice.h>

#include <vector>
#include <memory>

namespace stmg
{

using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;

namespace testing
{

template<int32_t nTeams, int32_t nPlayers, bool bNoSound>
class ReleaseGamesFixture : public TestBaseFixture, public GameOwner
{
public:
	void gameEnded() noexcept override
	{
	}
	void gameInterrupt(GameProxy::INTERRUPT_TYPE /*eInterruptType*/) noexcept override
	{
	}

protected:
	void setup() override
	{
		TestBaseFixture::setup();

		m_refFakeDM = std::make_shared<stmi::testing::FakeDeviceManager>();
		//const int32_t nKeyDevId =
		m_refFakeDM->simulateNewDevice<stmi::testing::FakeKeyDevice>();

		const std::string sJointris = "jointris";
		const std::string sAppVersion = "323.232";
		const bool bNoSoundMode = bNoSound;
		const bool bTestMode = true;
		const bool bTouch = false;
		jointrisSetupStdConfig(m_refStdConfig, m_refFakeDM, sJointris, sAppVersion, bNoSoundMode, bTestMode, bTouch);

		std::vector<File> aGameFiles;
		{
		auto aGameFilesPaths = config::getTestGamesFilePaths();
		for (auto& sFilePath : aGameFilesPaths) {
			aGameFiles.emplace_back(std::move(sFilePath));
		}
		}
		std::vector<File> aThemeFiles;
		{
		auto aThemeFilesPaths = config::getTestThemesFilePaths();
		for (auto& sFilePath : aThemeFilesPaths) {
			aThemeFiles.emplace_back(std::move(sFilePath));
		}
		}
		std::string sIconFile;
		std::string sHighscoresDir;
		std::string sPreferencesFile;
		m_refGameDiskFiles = std::make_shared<GameDiskFiles>(sJointris, false, std::move(aGameFiles), true, std::move(aThemeFiles), true
															, sIconFile, sHighscoresDir, sPreferencesFile);

		jointrisSetupXmlThemeLoader(m_refXmlThemeLoader, m_refStdConfig, m_refGameDiskFiles);
		jointrisSetupXmlGameLoader(m_refXmlGameLoader, m_refStdConfig, m_refGameDiskFiles);

		m_refPrefs = std::make_shared<StdPreferences>(m_refStdConfig);

		m_refPrefs->setTotTeams(nTeams);
		for (int32_t nTeam = 0; nTeam < nTeams; ++nTeam) {
			m_refPrefs->getTeamFull(nTeam)->setTotMates(nPlayers);
		}
	}
	void teardown() override
	{
		TestBaseFixture::teardown();
	}
	void testOneTeam()
	{
		auto refTheme = m_refXmlThemeLoader->getTheme("");
		assert(refTheme);

		auto& oNamed = refTheme->getNamed();
		shared_ptr<Game> refGame;

		auto oPairGame = m_refXmlGameLoader->getNewGame("Classic", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Classic2", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Classic300", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Dummy", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Largo", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Penta", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);

		oPairGame = m_refXmlGameLoader->getNewGame("Permutis", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);
	}
	void testTwoTeams()
	{
		auto refTheme = m_refXmlThemeLoader->getTheme("");
		assert(refTheme);

		auto& oNamed = refTheme->getNamed();
		shared_ptr<Game> refGame;
		auto oPairGame = m_refXmlGameLoader->getNewGame("ClassicVs", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);
		oPairGame = m_refXmlGameLoader->getNewGame("ClassicVsJunk", *this, m_refPrefs, oNamed, shared_ptr<stmg::Highscore>{});
		refGame = std::move(oPairGame.first);
		REQUIRE(refGame);
	}
public:
	shared_ptr<stmi::testing::FakeDeviceManager> m_refFakeDM;
	shared_ptr<StdConfig> m_refStdConfig;
	shared_ptr<GameDiskFiles> m_refGameDiskFiles;
	unique_ptr<XmlGameLoader> m_refXmlGameLoader;
	unique_ptr<XmlThemeLoader> m_refXmlThemeLoader;

	shared_ptr<StdPreferences> m_refPrefs;
};

class ReleaseOneTeamNoSoundGamesFixture : public ReleaseGamesFixture<1, 1, true>
{
};

TEST_CASE_METHOD(STFX<ReleaseOneTeamNoSoundGamesFixture>, "CheckSingleTeamNoSound")
{
	testOneTeam();
}

class ReleaseOneTeamSoundGamesFixture : public ReleaseGamesFixture<1, 1, false>
{
};

TEST_CASE_METHOD(STFX<ReleaseOneTeamSoundGamesFixture>, "CheckSingleTeamSound")
{
	testOneTeam();
}

class ReleaseOneTeamTwoPlayersNoSoundGamesFixture : public ReleaseGamesFixture<1, 2, true>
{
};

TEST_CASE_METHOD(STFX<ReleaseOneTeamTwoPlayersNoSoundGamesFixture>, "CheckSingleTeamTwoPlayersNoSound")
{
	testOneTeam();
}

class ReleaseOneTeamTwoPlayersSoundGamesFixture : public ReleaseGamesFixture<1, 2, false>
{
};

TEST_CASE_METHOD(STFX<ReleaseOneTeamTwoPlayersSoundGamesFixture>, "CheckSingleTeamTwoPlayersSound")
{
	testOneTeam();
}


class ReleaseTwoTeamGamesNoSoundFixture : public ReleaseGamesFixture<2, 1, true>
{
};

TEST_CASE_METHOD(STFX<ReleaseTwoTeamGamesNoSoundFixture>, "CheckTwoTeamsNoSound")
{
	testTwoTeams();
}

class ReleaseTwoTeamGamesSoundFixture : public ReleaseGamesFixture<2, 1, false>
{
};

TEST_CASE_METHOD(STFX<ReleaseTwoTeamGamesSoundFixture>, "CheckTwoTeamsSound")
{
	testTwoTeams();
}

} // namespace testing

} // namespace stmg

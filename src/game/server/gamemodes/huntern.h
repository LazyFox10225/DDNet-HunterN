/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERN_H
#define GAME_SERVER_GAMEMODES_HUNTERN_H
#include <game/server/gamecontroller.h>

class CGameControllerHunterN : public IGameController
{
private: // config
	int m_HunterRatio;
	int m_BroadcastHunterList;
	int m_BroadcastHunterDeath;
	int m_Wincheckdeley;
	int m_GameoverTime;
	//int m_RoundMode;

public:
	CGameControllerHunterN();

	static void OnClassSpawn(CCharacter *pChr);

	// event
	virtual void OnCharacterSpawn(class CCharacter *pChr) override;
	virtual void OnWorldReset() override;
	virtual void OnPlayerJoin(class CPlayer *pPlayer) override;
	virtual int OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) override;
	virtual void DoWincheckMatch() override;
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;

private: // Intelnal function and value
	void SelectClass();
	void ClassWin(int Flag);
	void DoWincheckClass();

	int nHunter; // 有多少个猎人
	int DoWinchenkClassTick; // 终局判断延迟的Tick
	char HunterList[256]; // 猎人列表

	enum HUNTERN_WINFLAG
	{
		FLAG_WIN_NONE = 0,
		FLAG_WIN_NO_CIVIC = 1,
		FLAG_WIN_NO_HUNTER = 2,
		FLAG_WIN_NO_JUG = 4,
	};
};

#endif
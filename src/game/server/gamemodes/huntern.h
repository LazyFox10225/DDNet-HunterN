/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_HUNTERN_H
#define GAME_SERVER_GAMEMODES_HUNTERN_H
#include <game/server/gamecontroller.h>

class CGameControllerHunterN : public IGameController
{
private:
	void SelectClass();
	void ClassWin(int Flag);
	void DoWincheckClass();

	int m_HunterRatio;
	int m_BroadcastHunterList;
	int m_BroadcastHunterDeath;
	int m_Wincheckdeley;
	int m_GameoverTime;
	//int m_RoundMode;

	int nHunter; // 有多少个猎人
	int DoWinchenkClassTick; // 终局判断延迟的Tick
	char HunterList[256];

	enum HUNTERN_WINFLAG
	{
		FLAG_WIN_NONE = 0,
		FLAG_WIN_CIVIC = 1,
		FLAG_WIN_HUNTER = 2,
		FLAG_WIN_JUG = 4,
	};

public:
	CGameControllerHunterN();

	void OnClassSpawn(CCharacter *pChr);

	virtual void OnCharacterSpawn(class CCharacter *pChr) override;
	virtual void OnWorldReset() override;
	virtual int OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion) override;
	virtual void DoWincheckMatch() override;
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
};

#endif
/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include "huntern.h"
#include <game/server/entities/character.h>
#include <game/server/weapons.h>
#include <game/server/classes.h>

CGameControllerHunterN::CGameControllerHunterN() :
	IGameController()
{
	m_pGameType = "HunterN";
	m_GameFlags = IGF_SURVIVAL | IGF_SUDDENDEATH;

	INSTANCE_CONFIG_INT(&m_HunterRatio, "htn_hunt_ratio", 4, 2, MAX_CLIENTS, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "几个玩家里选取一个猎人（整数,默认4,限制2~64）");
	INSTANCE_CONFIG_INT(&m_BroadcastHunterList, "htn_hunt_broadcast_list", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人列表（整数,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_BroadcastHunterDeath, "htn_hunt_broadcast_death", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "是否全体广播猎人死亡（整数,默认0,限制0~1）");
	INSTANCE_CONFIG_INT(&m_Wincheckdeley, "htn_wincheck_deley", 100, 0, 0xFFFFFFF, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "终局判断延时毫秒（整数,默认0,限制0~268435455）");
	//INSTANCE_CONFIG_INT(&m_RoundMode, "htn_round_mode", 0, 0, 1, CFGFLAG_CHAT | CFGFLAG_INSTANCE, "回合模式 正常0 娱乐1（整数,默认0,限制0~1）");

	//m_pInstanceConsole->Register("say", "?r[message]", CFGFLAG_INSTANCE, ConSay, this, "Say in chat in this room");
}

void CGameControllerHunterN::SelectClass()
{
	int PlayerCount = 0; // 玩家计数
	int CanHunterPlayerCount = 0; // 最近没当过猎人的玩家的计数
	int nHunter = 0; // 需要选择多少个猎人
	int rHunter = 0; // 猎人选择随机数

	for(int i = 0; i < MAX_CLIENTS; ++i) // 计数有PlayerCount个玩家 以及有CanHunterPlayerCount个玩家 我们要在m_CanHunter的玩家里面选择猎人
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			pPlayer->SetClass(CLASS_CIVIC); // 重置玩家为平民
			++PlayerCount;
			if(pPlayer->m_CanHunter)
				++CanHunterPlayerCount;
		}
	}

	if(PlayerCount < 2)
		return;

	nHunter = (PlayerCount - 2) / m_HunterRatio + 1;// 我们要多少个猎人
	str_format(HunterList, sizeof(HunterList), "本回合的 %d 个Hunter是: ", nHunter);// Generate Hunter info message 生成猎人列表消息头

	char aBuf[64];
	GameServer()->SendChatTarget(-1, "——————欢迎来到HunterN猎人杀——————");
	str_format(aBuf, sizeof(aBuf), "本回合有 %d 个猎人Hunter has been selected.", nHunter);
	GameServer()->SendChatTarget(-1, aBuf);
	GameServer()->SendChatTarget(-1, "规则：每回合秘密抽选猎人 猎人对战平民 活人看不到死人消息");
	GameServer()->SendChatTarget(-1, "      猎人双倍伤害 有瞬杀锤子(平民无锤)和破片榴弹(对自己无伤)");
	GameServer()->SendChatTarget(-1, "分辨队友并消灭敌人取得胜利！Be warned! Sudden Death.");

	for(int iHunter = 0; iHunter < nHunter; ++iHunter)// 选择nHunter个猎人
	{
		if(CanHunterPlayerCount <= 0) // 先检查m_CanHunter的玩家够不够（即所有玩家是不是最近都当过猎人了） 如果不够就重置所有玩家的m_CanHunter
		{
			for(int i = 0; i < MAX_CLIENTS; ++i) // 重置所有玩家的m_CanHunter
			{
				CPlayer *pPlayer = GetPlayerIfInRoom(i);
				if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
					(!pPlayer->m_RespawnDisabled ||
						(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
				{
					pPlayer->SetCanHunter(true); // 设置m_CanHunter为真（就是让他们"最近没当过"猎人）（包括猎人 主要是为了随机性）
					if(pPlayer->m_Class == CLASS_CIVIC) // 只有平民才可以被计数
						++CanHunterPlayerCount;
				}
			}
		}

		rHunter = rand() % CanHunterPlayerCount; // 在CanHunterPlayerCount个玩家里选择第rHunter个猎人

		for(int i = 0; i < MAX_CLIENTS; ++i) // 在CanHunterPlayerCount个玩家里选择第rHunter个玩家为猎人
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())) &&
					pPlayer->m_CanHunter &&
						(pPlayer->m_Class == CLASS_CIVIC)) // 计数活着、m_CanHunter、平民职业的玩家
			{
				if(rHunter == 0) // 找到了第rHunter个玩家 选择为猎人
				{
					pPlayer->SetClass(CLASS_HUNTER);

					pPlayer->SetCanHunter(false); // 把m_CanHunter设为否 即最近当过猎人
					--CanHunterPlayerCount;

					// Generate Hunter info message 生成猎人列表消息
					str_append(HunterList, Server()->ClientName(i), sizeof(HunterList));
					str_append(HunterList, ", ", sizeof(HunterList));

					break;
				}
				--rHunter;
			}
		}
	}

	if(!m_BroadcastHunterList)
		for(int i = 0; i < MAX_CLIENTS; ++i) // 循环所有旁观者 把猎人列表告诉他们
		{
			CPlayer *pPlayer = GetPlayerIfInRoom(i);
			if(pPlayer && pPlayer->GetTeam() == TEAM_SPECTATORS &&
					(pPlayer->m_RespawnDisabled ||
						!(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
			{
				GameServer()->SendChatTarget(pPlayer->GetCID(), HunterList);
			}
		}
	else
	{
		GameServer()->SendChatTarget(-1, HunterList);
	}
}

void CGameControllerHunterN::OnWorldReset()
{
	DoWinchenkClassTick = -1;

	SelectClass();
}

void CGameControllerHunterN::OnCharacterSpawn(CCharacter *pChr)
{
	pChr->IncreaseHealth(10);
	pChr->GiveWeapon(WEAPON_GUN, WEAPON_ID_PISTOL, 10);

	if(m_GameState == IGS_GAME_RUNNING)
	{
		if(pChr->GetPlayer()->GetClass() == CLASS_CIVIC)
		{
			GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, CmaskOne(pChr->GetPlayer()->GetCID()));
			GameServer()->SendBroadcast("这局你是平民Civic！噶了所有猎人胜利!     \n猎人双倍伤害 有瞬杀锤子和破片榴弹", pChr->GetPlayer()->GetCID(), true);
		}
		else if(pChr->GetPlayer()->GetClass() == CLASS_HUNTER)
		{
			pChr->GiveWeapon(WEAPON_HAMMER, WEAPON_ID_HAMMER, -1);

			GameWorld()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, CmaskOne(pChr->GetPlayer()->GetCID()));
			GameServer()->SendBroadcast("     这局你是猎人Hunter！噶了所有平民胜利!\n     猎人双倍伤害 有瞬杀锤子和破片榴弹", pChr->GetPlayer()->GetCID(), true);
		}
	}
}

int CGameControllerHunterN::OnCharacterTakeDamage(class CCharacter *pChr, vec2 &Force, int &Dmg, int From, int WeaponType, int WeaponID, bool IsExplosion)
{
	if(pChr->GetPlayer()->GetCID() == From && pChr->GetPlayer()->GetClass() == CLASS_HUNTER) // 猎人不能受到来自自己的伤害（这样就不会被逆天榴弹自爆）
		return DAMAGE_NO_DAMAGE | DAMAGE_NO_INDICATOR;
	return DAMAGE_NORMAL;
}

void CGameControllerHunterN::ClassWin(int Class/*, int Mode*/)
{
	if(Class == CLASS_CIVIC)
	{
		GameServer()->SendChatTarget(-1, HunterList);
		GameServer()->SendChatTarget(-1, "平民胜利！");
	}
	else if(Class == CLASS_HUNTER)
	{
		// GameServer()->SendChatTarget(-1, HunterList); // 猎人胜利则不显示猎人列表
		GameServer()->SendChatTarget(-1, "猎人胜利！");
	}
	else
	{
		GameServer()->SendChatTarget(-1, HunterList);
		GameServer()->SendChatTarget(-1, "游戏结束！");
	}

	//GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE, CmaskAll());
	EndMatch();
}

void CGameControllerHunterN::DoWincheckClass() // check for class based win
{
	int CivicCount = 0;
	int HunterCount = 0;

	for(int i = 0; i < MAX_CLIENTS; ++i) // Count Player
	{
		CPlayer *pPlayer = GetPlayerIfInRoom(i);
		if(pPlayer && pPlayer->GetTeam() != TEAM_SPECTATORS &&
			(!pPlayer->m_RespawnDisabled ||
				(pPlayer->GetCharacter() && pPlayer->GetCharacter()->IsAlive())))
		{
			if(pPlayer->m_Class == CLASS_CIVIC)
			{
				++CivicCount;
			}
			else if(pPlayer->m_Class == CLASS_HUNTER)
			{
				++HunterCount;
			}
		}
	}

	if(CivicCount == 0) // 平民寄光了
		ClassWin(CLASS_HUNTER);
	else if(HunterCount == 0) // 猎人寄光了
		ClassWin(CLASS_CIVIC);
}

void CGameControllerHunterN::DoWincheckMatch() // check for time based win
{
	if(!m_SuddenDeath && m_GameInfo.m_TimeLimit > 0 && (Server()->Tick() - m_GameStartTick) >= m_GameInfo.m_TimeLimit * Server()->TickSpeed() * 60)
	{
		ClassWin(0);
	}
	else if(DoWinchenkClassTick != -1)
	{
		if(DoWinchenkClassTick == 0)
		{
			DoWincheckClass();
		}
		--DoWinchenkClassTick;
	}
}

int CGameControllerHunterN::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	if(m_GameState == IGS_GAME_RUNNING)
	{
		if(pVictim->GetPlayer()->GetClass() == CLASS_HUNTER)
		{
			char aBuf[48];
			str_format(aBuf, sizeof(aBuf), "Hunter '%s' was defeated!", Server()->ClientName(pVictim->GetPlayer()->GetCID()));

			if(!m_BroadcastHunterDeath)
			{
				GameServer()->SendChatTarget(-1, aBuf); // 直接全体广播
				GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE, CmaskAll());
			}
			else
			{
				for(int i = 0; i < MAX_CLIENTS; ++i) // 逐个给所有人根据职业发送死亡消息
				{
					CPlayer *pPlayer = GetPlayerIfInRoom(i);
					if(pPlayer)
					{
						if(pPlayer->m_Class == CLASS_HUNTER || pPlayer->GetTeam() == TEAM_SPECTATORS) // 给所有猎人广播他们"队友"的死亡消息
						{
							GameServer()->SendChatTarget(pPlayer->GetCID(), aBuf);
							GameWorld()->CreateSoundGlobal(SOUND_CTF_CAPTURE, CmaskOne(pPlayer->GetCID()));
						}
						else
						{
							GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP, CmaskOne(pPlayer->GetCID()));
						}
					}
				}
			}
		}
		else
			GameWorld()->CreateSoundGlobal(SOUND_CTF_DROP, CmaskAll());

		GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), HunterList);

		if(DoWinchenkClassTick == -1)
			DoWinchenkClassTick = (Server()->TickSpeed() * m_Wincheckdeley / 1000);
	}

	return DEATH_NO_REASON | DEATH_SKIP_SCORE;
}

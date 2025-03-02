#include "grenade.h"
#include <game/generated/server_data.h>
#include <game/server/entities/projectile.h>

#include <game/server/weapons/shotgun.h>

CGrenade::CGrenade(CCharacter *pOwnerChar) :
	CWeapon(pOwnerChar)
{
	m_MaxAmmo = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Maxammo;
	m_AmmoRegenTime = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Ammoregentime;
	m_FireDelay = g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Firedelay;
	m_FullAuto = true;
}

bool CGrenade::GrenadeCollide(CProjectile *pProj, vec2 Pos, CCharacter *pHit, bool EndOfLife)
{
	if(pHit && pHit->GetPlayer()->GetCID() == pProj->GetOwner())
		return false;

	pProj->GameWorld()->CreateSound(Pos, SOUND_GRENADE_EXPLODE);
	pProj->GameWorld()->CreateExplosion(Pos, pProj->GetOwner(), WEAPON_GRENADE, pProj->GetWeaponID(), g_pData->m_Weapons.m_aId[WEAPON_GRENADE].m_Damage, pProj->GetOwner() < 0);

	/* Hunter Start */
	if(pProj->GameServer()->m_apPlayers[pProj->GetOwner()]->GetClass() & CLASS_HUNTER)
	{
		pProj->GameWorld()->CreateExplosionParticle(Pos+vec2(50,50)); // Create Particle
		pProj->GameWorld()->CreateExplosionParticle(Pos+vec2(-50,50));
		pProj->GameWorld()->CreateExplosionParticle(Pos+vec2(50,-50));
		pProj->GameWorld()->CreateExplosionParticle(Pos+vec2(-50,-50));

		CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
		Msg.AddInt(pProj->Controller()->m_HuntFragsNum);

		for(int i = 0; i < pProj->Controller()->m_HuntFragsNum; i++) // Create Fragments
		{
			float a = (rand()%314)/5.0;
			vec2 d = vec2(cosf(a), sinf(a));
			CProjectile *pProjFrag = new CProjectile(
				pProj->GameWorld(),
				WEAPON_SHOTGUN, //Type
				pProj->GetWeaponID(), //WeaponID
				pProj->GetOwner(), //Owner
				Pos + d, //Pos
				d * 0.4f, //Dir
				6.0f, // Radius
				0.33f * pProj->Server()->TickSpeed(), //Span
				CShotgun::BulletCollide);
			
			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProjFrag->FillInfo(&p);

			for(unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);
		}
	}
	/* Hunter End */

	return true;
}

void CGrenade::Fire(vec2 Direction)
{
	int ClientID = Character()->GetPlayer()->GetCID();
	int Lifetime = Character()->CurrentTuning()->m_GrenadeLifetime * Server()->TickSpeed();

	vec2 ProjStartPos = Pos() + Direction * GetProximityRadius() * 0.75f;

	CProjectile *pProj = new CProjectile(
		GameWorld(),
		WEAPON_GRENADE, //Type
		GetWeaponID(), //WeaponID
		ClientID, //Owner
		ProjStartPos, //Pos
		Direction, //Dir
		6.0f, // Radius
		Lifetime, //Span
		GrenadeCollide);

	// pack the Projectile and send it to the client Directly
	CNetObj_Projectile p;
	pProj->FillInfo(&p);

	CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
	Msg.AddInt(1);
	for(unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
		Msg.AddInt(((int *)&p)[i]);

	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
	GameWorld()->CreateSound(Character()->m_Pos, SOUND_GRENADE_FIRE);
}
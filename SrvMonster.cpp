//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright( C ) 2001 Waei Corporation.  All Rights Reserved.
//
//	Author	: DingDong Lee
//	Desc	  : Handle All Monster Action, AI And Data
//	Date	  : 2001-4-19
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SrvMonster.h"

#include "../../Main/BfInit.h"
#include "SrvPlayer.h"
#include "../Skill/SrvSkill.h"
#include "../../SrvClass/Path/SrvPath.h"
#include "../../SrvClass/Skill/SrvSkill.h"
#include "../Event/SrvEvent.h"
#include "time.h"

//#define _DEBUG_INFO_MONSTER_
//#define _DEBUG_SHOW_MONSTER_INFO_
//#define _DEBUG_MONSTER_ALWAYS_REVIVE_
#define _DEBUG_I_WANT_FIND_ITEM_CRASH_
#ifndef _NEW_A_STAR
extern A_STAR		g_PathFinder;
#else
extern AStar		g_PathFinder;
#endif
extern WORD			g_DropItemPosX[24];
extern WORD			g_DropItemPosY[24];
extern DWORD		g_dwMonsterTotal;
#ifdef VERSION40_UNIQUERING
extern CRingManager     *g_pRingManager;
#endif

BOOL            g_bShowMonsterExpLog  = FALSE;
WORD            g_wShowMonsterExpCode = 0;
WORD            g_wShowMonsterExpMap  = 0;
list<CMonster*> g_ListUnreviveMonster;
//The all directions's offset X
										 //0 //1 //2 //3 //4 //5 //6 //7
int	DirOffsetY[529] ={ 0,  0,	 1,  1,	 1,	 0, -1, -1, -1,
											 0,  1,  2,  2,  2,  2,  2,  1,  0, -1, -2, -2, -2, -2, -2, -1,
											 0,  1,  2,  3,  3,  3,  3,  3,  3,  3,  2,  1,  0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1,
											 0,  1,  2,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  2,  1,  0, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -2, -1,
											 0,  1,  2,  3,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -4, -3, -2, -1,
											 0,  1,  2,  3,  4,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -5, -4, -3, -2, -1,
											 0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -6, -5, -4, -3, -2, -1,
                       0,  1,  2,  3,  4,  5,  6,  7,  8,	 8,	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  7,  6,  5,  4,  3,  2,  1,	 0, -1, -2, -3, -4, -5, -6, -7, -8,	-8,	-8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -7, -6, -5, -4, -3, -2, -1,
											 0,  1,  2,  3,  4,  5,  6,  7,  8,	 9,	 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  8,  7,  6,  5,	 4,  3,  2,  1,  0, -1, -2, -3, -4,	-5,	-6, -7, -8, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -8, -7, -6, -5, -4, -3, -2, -1,
											 0,  1,  2,  3,  4,  5,  6,  7,  8,	 9,	10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,  9,	 8,  7,  6,  5,  4,  3,  2,  1,  0,	-1,	-2, -3, -4, -5, -6, -7, -8, -9,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1,
											 0,  1,  2,  3,  4,  5,  6,  7,  8,	 9,	10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,	11, 11, 10,  9,  8,  7,  6,  5,  4,	 3,	 2,  1,  0, -1, -2, -3, -4, -5, -6, -7, -8, -9,-10,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-10, -9, -8, -7, -6, -5, -4, -3, -2, -1, };
//The all directions's offset Y
										 //0 //1 //2 //3 //4 //5 //6 //7
int	DirOffsetX[529] ={ 0, -1, -1,  0,  1,  1,	 1,	 0, -1, 
											-2, -2, -2, -1,  0,  1,  2,  2,  2,  2,  2,  1,  0, -1, -2, -2,
											-3, -3, -3, -3, -2, -1,  0,  1,  2,  3,  3,  3,  3,  3,  3,  3,  2,  1,  0, -1, -2, -3, -3, -3,
											-4, -4, -4, -4, -4, -3, -2, -1,  0,  1,  2,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  2,  1,  0, -1, -2, -3, -4, -4, -4, -4, 
											-5, -5, -5, -5, -5, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -5, -5, -5, -5, 
											-6, -6, -6, -6, -6, -6, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6, -6, -6, -6, -6, -6, 
											-7, -7, -7, -7, -7, -7, -7, -7, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6, -7, -7, -7, -7, -7, -7, -7, 
                      -8, -8, -8, -8, -8, -8, -8, -8, -8, -7, -6, -5, -4, -3, -2, -1,	 0,  1,  2,  3,  4,  5,  6,  7,  8,	 8,	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  7,  6,  5,  4,  3,  2,  1,	 0, -1, -2, -3, -4, -5, -6, -7, -8,	-8,	-8, -8, -8, -8, -8, -8,
											-9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -8, -7, -6, -5, -4, -3,	-2, -1,  0,  1,  2,  3,  4,  5,  6,	 7,	 8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  8,  7,	 6,  5,  4,  3,  2,  1,  0, -1, -2,	-3,	-4, -5, -6, -7, -8, -9, -9, -9, -9, -9, -9, -9, -9, -9,
										 -10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10, -9, -8, -7, -6, -5,	-4, -3, -2, -1,  0,  1,  2,  3,  4,	 5,	 6,  7,  8,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,	10, 10, 10,  9,  8,  7,  6,  5,  4,	 3,	 2,  1,  0, -1, -2, -3, -4, -5, -6, -7, -8, -9,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,
										 -11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-10, -9, -8,	-7, -6, -5, -4, -3, -2, -1,  0,  1,	 2,	 3,  4,  5,  6,  7,  8,  9, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,	11, 11, 11, 11, 11, 11, 11, 11, 10,	 9,	 8,  7,  6,  5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6, -7, -8, -9,-10,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11, };

int g_iEscapeDir[8][5] = {{ 2, 5, 4, 3, 6 },
													{ 7, 4, 6, 5, 3 },
													{ 0, 5, 6, 7, 4 },
													{ 1, 6, 0, 7, 5 },
													{ 2, 7, 0, 1, 6 },
													{ 3, 0, 1, 2, 7 },
													{ 4, 1, 2, 3, 0 },
													{ 5, 2, 3, 4, 1 },};


DWORD		g_IntervalMonsterWalk[8] = {	TIME_WALK_PER_TILE,			// Direction 0
																			TIME_WALK_PER_TILE*2,
																			TIME_WALK_PER_TILE,			// Direction 2
																			TIME_WALK_PER_TILE*2,
																			TIME_WALK_PER_TILE,			// Direction 4
																			TIME_WALK_PER_TILE*2,
																			TIME_WALK_PER_TILE,			// Direction 6
																			TIME_WALK_PER_TILE*2 },

				g_IntervalMonsterRun[8]  = {	TIME_RUN_PER_TILE,			// Direction 0
																			TIME_RUN_PER_TILE*2,
																			TIME_RUN_PER_TILE,			// Direction 2
																			TIME_RUN_PER_TILE*2,
																			TIME_RUN_PER_TILE,			// Direction 4
																			TIME_RUN_PER_TILE*2,
																			TIME_RUN_PER_TILE,			// Direction 6
																			TIME_RUN_PER_TILE*2 };


WORD    g_MonsterSizeTile2X[4]   =
{
	0, -1, -1,  0
};
WORD    g_MonsterSizeTile2Y[4]   =
{
	0,  0,  1,  1
};
WORD    g_MonsterSizeTile3X[9]   = 
{
	0,  1,  1,  0, -1, -1, -1,  0,  1
};
WORD    g_MonsterSizeTile3Y[9]   = 
{
  0,  0, -1, -1, -1,  0,  1,  1,  1
};
WORD    g_MonsterSizeTile4X[16]  = 
{
	0, -1, -2, -3, -3, -2, -1,  0,  0, -1, -2, -3, -3, -2, -1,  0
};
WORD    g_MonsterSizeTile4Y[16]  = 
{
	0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3
};
WORD    g_MonsterSizeTile5X[25]  =
{
	0,  1,  1,  0, -1, -1, -1,  0,  1,  2,  2,  2,  1,  0, -1, -2, -2, -2, -2, -2, -1,  0,  1,  2,  2
};
WORD    g_MonsterSizeTile5Y[25]  =
{
	0,  0, -1, -1, -1,  0,  1,  1,  1,  0, -1, -2, -2, -2, -2, -2, -1,  0,  1,  2,  2,  2,  2,  2,  1
};

int    g_MonsterPursueTableX[8] =
{
   1,  1,  0, -1, -1, -1,  0,  1
};

int    g_MonsterPursueTableY[8] =
{
   0,  1,  1,  1,  0, -1, -1, -1
};

CMDropItemList									*g_pDropItem = NULL;
DWORD														MonsterTickCount = 0;
static char											g_szMonsterLog[MAX_MEMO_MSG_LEN];

#ifdef _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
int                             g_iDamageCount = 0;
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The Member Functions Of CSrvBaseMonster
//
//
//==============================================================================================
//
//
CSrvTmpBaseMonster::CSrvTmpBaseMonster()
{
	m_wId = 0;
#ifdef _REPAIR_SERVER_CRASH_NICK_
	memset( m_szName, 0, MAX_NPC_NAME_LEN );
#else	
	strcpy( m_szName, "" );
#endif
  m_iApBonuRand = 0;
}
//==============================================================================================
//
//
CSrvTmpBaseMonster::~CSrvTmpBaseMonster()
{
	m_wId = 0;
#ifdef _REPAIR_SERVER_CRASH_NICK_
	memset( m_szName, 0, MAX_NPC_NAME_LEN );
#else
	strcpy( m_szName, "" );
#endif
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The Member Functions Of CSrvBaseMonster
//
//
//==============================================================================================
//
//
CSrvDropItem::CSrvDropItem()
{
	m_wId      = 0;
	m_wItemId  = 0;
  //m_iBearPsb = 0;
}
//==============================================================================================
//
//
CSrvDropItem::~CSrvDropItem()
{
	m_wId = 0;
	m_wItemId = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The Member Functions Of CSrvBaseMonster
//
//
//==============================================================================================
//
//
CSrvBaseMonster::CSrvBaseMonster( CSrvTmpBaseMonster * pBase, WORD wId )
{
	memcpy( m_szName, pBase->m_szName, MAX_NPC_NAME_LEN );
  m_szName[MAX_NPC_NAME_LEN-1] = '\0';

	m_wId      = wId;
	m_wBaseId  = pBase->m_wId;
  m_wPicId   = pBase->m_wPicId;
	m_wElement = pBase->m_wElement;
	m_iHp      = pBase->m_iHp;
  m_iAp      = pBase->m_iAp;
  m_iDp      = pBase->m_iDp;
  m_iHit     = pBase->m_iHit;
  m_iDodge   = pBase->m_iDodge;
	m_iInt     = pBase->m_iInt;
  m_iBearPsb = 0;
	m_wHard    = pBase->m_wHard;
  m_wLevel   = 1;

	m_iHpUp    = pBase->m_iHpUp;
	m_iApUp    = pBase->m_iApUp;
	m_iDpUp    = pBase->m_iDpUp;
	m_iHitUp   = pBase->m_iHitUp;
	m_iDodgeUp = pBase->m_iDodgeUp;
	m_iIntUp   = pBase->m_iIntUp;

	m_wSize    = pBase->m_wSize;

	m_iNpcProperty = pBase->m_iNpcProperty;
																			// Monster Property
																			// 1: 不主动攻击; 2: 受害不追击; 4: 受害即逃跑;
																			// 8: Hp低于10%则逃跑; 16: 停止战斗后不回原地
																			// 32: 会捡骨; 64: 超过十个自动消化一个道具
																			// 128: 支援同组(族); 256: 辅助支援同组(族)
																			// 512: 自动使用道具; 1024: 不死; 2048: 不受补血
																			// 4096: 主动攻击; 8192: 可否佣兵; 2^14: 半透明
	m_iAcceptTable = pBase->m_iAcceptTable;
																			// 佣兵可接受的指令
																			// 1: 攻击; 2: 防御; 4: 指定; 8: 等待
																			// 16: 收集; 32: 回家; 64: 解雇

	m_wSpeed      = pBase->m_wSpeed;		// 怪物速度
																			// 0: 不动; 5: 缓速; 10: 步行速度;
																			// 15: 一般速度; 20: 跑步速度; 25: 加速; 30: 极速
	m_wSightRange  = pBase->m_wSightRange;	// 怪物侦测范围
	m_wHelpRange   = pBase->m_wHelpRange;		// 支援同组(族)的范围
	m_wPursueRange = pBase->m_wPursueRange;	// 怪物的持续追击范围
	m_wPursueSpeed = pBase->m_wPursueSpeed;	// 怪物的追击速度
	m_wEscapeSpeed = pBase->m_wEscapeSpeed;	// 怪物的逃跑速度
	m_wHaltRange   = pBase->m_wHaltRange;		// 怪物逃跑后至几格外停止战斗
	m_wExpLtLv     = 10;
	m_wExpLtLvHalf = 5;
  m_dwDeadEvent  = 0;
  m_wParam1      = 0;
  m_iParam2      = 0;
#ifdef _BOSS_CALL_UP_HAND_
  m_wCallUpCount = 30;
#endif
}
//==============================================================================================
//
//
CSrvBaseMonster::~CSrvBaseMonster()
{

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The Member Functions Of CMonster
//
//
//==============================================================================================
//
// the member functions of CMonster
CMonster::CMonster( CSrvBaseMonster *pBaseMonster, DWORD dwCode )
{
	int i = 0;

	ClearLife();

	// Get Base Data
	m_pBase				= pBaseMonster;
	SetCode( dwCode );
  m_wSelfCode = dwCode;
	// About Public Life
	m_dwExp				= pBaseMonster->m_dwExp;
	m_dwExpPer20  = m_dwExp * 20 / 100 ;
	m_dwExpPer80  = m_dwExp * 80 / 100 ;
  m_wLevel			= pBaseMonster->m_wLevel;
  m_wElement		= pBaseMonster->m_wElement;
  m_dwRaceAttri = pBaseMonster->m_dwRaceAttri;
	m_wElemBackup	= pBaseMonster->m_wElement;

	m_iMaxHp	= pBaseMonster->GetHp();
  m_iMaxMp	= 0;
  m_iMaxSp	= 0;
  m_iHp			= pBaseMonster->GetHp();
  m_iMp			= 0;
  m_iSp			= 0;

  m_iAp			    = pBaseMonster->GetAp();
  m_iDp			    = pBaseMonster->GetDp();
  m_iHit		    = pBaseMonster->GetHit();
  m_iDodge	    = pBaseMonster->GetDodge();
	m_iInt		    = pBaseMonster->GetInt();
  m_iBearPsb    = pBaseMonster->GetBearPsb();
  m_iApBonuRand = pBaseMonster->m_iApBonuRand;
	m_wSpeed	    = pBaseMonster->m_wSpeed;

  m_iChangeAp    = pBaseMonster->m_iEquipAp;
  m_iChangeDp    = pBaseMonster->m_iEquipDp;
  m_iChangeHit   = pBaseMonster->m_iEquipHit;
  m_iChangeDodge = pBaseMonster->m_iEquipDodge;
  m_iChangeInt   = pBaseMonster->m_iEquipInt;

	m_iPoison					= 0;
	m_iAddMaxHp				= 0;
	m_iAddMaxMp				= 0;
	m_iAddMaxSp				= 0;
	m_iAddAp					= 0;
	m_iAddDp					= 0;
	m_iAddHit					= 0;
	m_iAddDodge				= 0;
	m_iAddInt					= 0;
	m_iAddSpeed				= 0;
	m_iAddCriticalHit	= 0;
#ifdef _NEW_TALISMAN_
	m_iAddRageAp      = 0;
	m_iAddRageHarm    = 0;
#endif
	m_qwSpecialStatus	= 0;
													// 玩家特殊状态, 又同时代表各Timer的状态

													// 1 PK, 2 砍人, 4 加速, 8 狠, 16 准, 32 稳, 64 快, 128 智
													// 256 必杀, 512 毒, 1024 盲, 2048 定, 2^12 封, 2^13 拙, 2^14 隐身,
													// 2^15 侦测隐身, 2^16 加MaxHp, 2^17 加Hp, 2^18 加MaxMp, 2^19 加Mp,
													// 2^20 加MaxSp, 2^21 加Sp, 2^22 五行, 2^23 禁言, 2^24 通缉

	m_dwCheckTimerNow1 = 0;
	m_dwCheckTimerNow2 = 0;
  m_dwCheckCount     = 0;

	m_iMaxHpPer10 = m_pBase->m_wEscapeHp;

	m_wTargetItemX= 0;
	m_wTargetItemY= 0;
	m_iMidX				= 0;
	m_iMidY				= 0;
	m_iEndX				= 0;
	m_iEndY				= 0;
	m_iInitX			= 0;
	m_iInitY			= 0;

  m_iNpcStatus			= STATUS_MONSTER_STAND;
  SetPoseState( STATUS_MONSTER_POSE_STAND );
	m_iNextStatus			= STATUS_MONSTER_STAND;
	m_iAddiStatus			= STATUS_MONSTER_STAND;
	m_iAddiStatus		  = 0;
	m_qwSpecialStatus = 0;
	m_qwAntiStatus    = pBaseMonster->m_qwAntiStatus;

	m_wSpeed			= pBaseMonster->m_wSpeed;
  m_iMoveDir		= gf_GetRandom( 8 );
	m_iStepCount	= 0;
	//m_dwPathTile  = 0;
	m_PathFindTrigger = 0;

	m_iActiveRange		= 0;
	SetMonsterSight( pBaseMonster->GetSight() );

	m_pUsingSkill			= NULL;
	m_wTargetDis			= 0;

	for( i = 0; i < MAX_MONSTER_SKILL; i++ )
	{
		m_pSkill[i] = NULL;
	}
  m_wSkillCount     = 0;

  m_pLastTarget			= NULL;
	m_pLastHit				= NULL;
	m_pFirstHit				= NULL;
	m_dwTargetCode		= 0;
	m_iPathCount			= 0;

	for( i = 0; i < 4; i++ )
	{
		m_iWanderRange[i] = 0;
	}
	for( i = 0; i < 9; i++ )
	{
		m_dwEnd[i] = 0;
	}
	//m_dwEnd[MST_MOVE_ACT]			= MonsterTickCount + MONSTER_MOVE_TIME;
	m_dwEnd[MST_SEARCH_ACT]		= MonsterTickCount + MONSTER_SEARCH_TIME;
	//m_dwEnd[MST_WANDER_ACT]		= MonsterTickCount + MONSTER_WANDER_TIME;
	//m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + MONSTER_ATTACK_TIME;
	//m_dwEnd[MST_STAND_ACT]		= MonsterTickCount + MONSTER_STAND_TIME;

	m_bReachEnd				= TRUE;

	// Abount Dead Event
	m_pDeadEvent			= NULL;
  m_pNpc            = NULL;

	// About Timers
	//for( i = 0; i < MAX_TIMER_COUNT; i++ )
	//{
	//	m_dwAlarmTime[i]	= 0;
	//	m_wFuncAlarm[i]		= 0;
	//	m_dwTrigger[i][0]	= 0;
	//	m_dwTrigger[i][1]	= 0;
	//	m_wFuncTrigger[i] = 0;
	//}
	memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof( DWORD ) * MAX_MOVE_END );
  m_wWanderPath = 0;
	// About GroundItem List
	//m_pCurrItem				= NULL;
	m_dwPickUpMoney   = 0;
	m_listCollectItem.clear();
	m_listGroundItemPos.clear();
	m_wTeam						= 0;
	m_wNotifyTimes    = 0;
	m_pLastTeamer     = NULL;
	m_pTeam						= NULL;

	m_pListHiter.clear();
	m_BadestHiter.m_pHiter  = NULL;
	m_BadestHiter.m_wHitNum = 0;
	m_BadestHiter1.m_pHiter  = NULL;
	m_BadestHiter1.m_wHitNum = 0;
	m_wStopTimes = 0;

	memset( m_iVar, 0, sizeof( int ) * 4 );
#ifdef _REPAIR_SERVER_CRASH_NICK_
	memset( m_szNickName, 0, MAX_NPC_NAME_LEN );
#else
	strcpy( m_szNickName, "" );
#endif
	m_dwEquipPos = 0;
	m_dwItemEquip = 0;
	m_wNotifyCode = 0;
	m_wHitTotal = 0;
  m_wReviveType = REVIVE_TYPE_ALWAYS;

#ifdef _BOSS_CALL_UP_HAND_
  m_wCallUpNum  = 0;
  m_pFather     = NULL;
#endif
  m_pTrapSkill = NULL;
  m_wMercenaryType = 0;
}
//==============================================================================================
//
//
CMonster::CMonster(CSrvBaseMonster *pBaseMonster, CPlayer *pOwner)
{
	int i = 0;

	ClearLife();

	// Get Base Data
	m_pBase				= pBaseMonster;
	// Set Pet Code, Use Player Code Or Monster Code ???
	// ...

	// About Public Life
	m_dwExp				= pBaseMonster->m_dwExp;
	m_dwExpPer20  = m_dwExp * 20 / 100;
	m_dwExpPer80  = m_dwExp * 80 / 100;
  m_wLevel			= pBaseMonster->m_wLevel;
  m_wElement		= pBaseMonster->m_wElement;
  m_dwRaceAttri = pBaseMonster->m_dwRaceAttri;
	m_wElemBackup	= pBaseMonster->m_wElement;

	m_iMaxHp	= pBaseMonster->GetHp();
  m_iMaxMp	= 0;
  m_iMaxSp	= 0;
  m_iHp			= pBaseMonster->GetHp();
  m_iMp			= 0;
  m_iSp			= 0;

  m_iAp			= pBaseMonster->GetAp();
  m_iDp			= pBaseMonster->GetDp();
  m_iHit		= pBaseMonster->GetHit();
  m_iDodge	= pBaseMonster->GetDodge();
	m_iInt		= pBaseMonster->GetInt();
  m_iApBonuRand = pBaseMonster->m_iApBonuRand;
	m_wSpeed	= pBaseMonster->m_wSpeed;

	m_iPoison					= 0;
	m_iAddMaxHp				= 0;
	m_iAddMaxMp				= 0;
	m_iAddMaxSp				= 0;
	m_iAddAp					= 0;
	m_iAddDp					= 0;
	m_iAddHit					= 0;
	m_iAddDodge				= 0;
	m_iAddInt					= 0;
	m_iAddSpeed				= 0;
	m_iAddCriticalHit	= 0;
#ifdef _NEW_TALISMAN_
  m_iAddRageAp      = 0;
  m_iAddRageHarm    = 0;
#endif
	m_qwSpecialStatus	= 0;
													// 玩家特殊状态, 又同时代表各Timer的状态

													// 1 PK, 2 砍人, 4 加速, 8 狠, 16 准, 32 稳, 64 快, 128 智
													// 256 必杀, 512 毒, 1024 盲, 2048 定, 2^12 封, 2^13 拙, 2^14 隐身,
													// 2^15 侦测隐身, 2^16 加MaxHp, 2^17 加Hp, 2^18 加MaxMp, 2^19 加Mp,
													// 2^20 加MaxSp, 2^21 加Sp, 2^22 五行, 2^23 禁言, 2^24 通缉

	m_dwCheckTimerNow1 = 0;
	m_dwCheckTimerNow2 = 0;
  m_dwCheckCount     = 0;

	m_iMaxHpPer10 = m_pBase->m_wEscapeHp;

	m_wTargetItemX= 0;
	m_wTargetItemY= 0;
	m_iMidX				= 0;
	m_iMidY				= 0;
	m_iEndX				= 0;
	m_iEndY				= 0;
	m_iInitX			= 0;
	m_iInitY			= 0;

  m_iNpcStatus			= STATUS_MONSTER_STAND;
  SetPoseState( STATUS_MONSTER_POSE_STAND );
	m_iNextStatus			= STATUS_MONSTER_STAND;
	m_iAddiStatus			= STATUS_MONSTER_STAND;
	m_iAddiStatus		  = 0;
	m_qwSpecialStatus = 0;
	m_qwAntiStatus    = pBaseMonster->m_qwAntiStatus;

	m_wSpeed			= pBaseMonster->m_wSpeed;
  m_iMoveDir		= gf_GetRandom( 8 );;
	m_iStepCount	= 0;
	//m_dwPathTile  = 0;
	m_PathFindTrigger = 0;

	m_iActiveRange		= 0;
	SetMonsterSight( pBaseMonster->GetSight() );
	m_iCurrSight			= 0;

	m_pUsingSkill			= NULL;
	m_wTargetDis			= 0;

	for( i = 0; i < MAX_MONSTER_SKILL; i++ )
	{
		m_pSkill[i] = NULL;
	}
  m_wSkillCount     = 0;

  m_pLastTarget			= NULL;
	m_pLastHit				= NULL;
	m_pFirstHit				= NULL;
	m_dwTargetCode		= 0;
	m_iPathCount			= 0;

	for( i = 0; i < 4; i++ )
	{
		m_iWanderRange[i] = 0;
	}
	for( i = 0; i < 9; i++ )
	{
		m_dwEnd[i] = 0;
	}
	//m_dwEnd[MST_MOVE_ACT]			= MonsterTickCount + MONSTER_MOVE_TIME;
	m_dwEnd[MST_SEARCH_ACT]		= MonsterTickCount + MONSTER_SEARCH_TIME;
	//m_dwEnd[MST_WANDER_ACT]		= MonsterTickCount + MONSTER_WANDER_TIME;
	//m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + MONSTER_ATTACK_TIME;
	//m_dwEnd[MST_STAND_ACT]		= MonsterTickCount + MONSTER_STAND_TIME;
	
	m_bReachEnd				= TRUE;

	// Abount Dead Event
	m_pDeadEvent			= NULL;
  m_pNpc            = NULL;

	// About Timers
	//for( i = 0; i < MAX_TIMER_COUNT; i++ )
	//{
	//	m_dwAlarmTime[i]	= 0;
	//	m_wFuncAlarm[i]		= 0;
	//	m_dwTrigger[i][0]	= 0;
	//	m_dwTrigger[i][1]	= 0;
	//	m_wFuncTrigger[i] = 0;
	//}
	memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof( DWORD ) * MAX_MOVE_END );
  m_wWanderPath = 0;
	// About GroundItem List
	//m_pCurrItem				= NULL;
	m_dwPickUpMoney   = 0;
	m_listCollectItem.clear();
	m_listGroundItemPos.clear();
	m_wTeam						= 0;
	m_wNotifyTimes    = 0;
	m_pLastTeamer     = NULL;
	m_pTeam						= NULL;

	m_pListHiter.clear();
	m_BadestHiter.m_pHiter  = NULL;
	m_BadestHiter.m_wHitNum = 0;
	m_BadestHiter1.m_pHiter  = NULL;
	m_BadestHiter1.m_wHitNum = 0;
	m_wStopTimes  = 0;
	m_wHitTotal   = 0;
  m_wReviveType = REVIVE_TYPE_ALWAYS;
#ifdef _BOSS_CALL_UP_HAND_
  m_wCallUpNum  = 0;
  m_pFather     = NULL;
#endif
  m_pTrapSkill  = NULL;
  m_wMercenaryType = 0;
}
//==============================================================================================
//
//
CMonster::~CMonster()
{
	//....Release ?
	for( int i = 0; i < MAX_MONSTER_SKILL; i++ )
  {
    g_pGs->DeleteCSkill( m_pSkill[i] );
    m_pSkill[i] = NULL;
  }
}
//==============================================================================================
//
//
CSkill * CMonster::GetBaseSkillByRate()
{
  static int iRandom;

	iRandom = gf_GetRandom5();
  if( iRandom < m_pBase->m_wSkillRate[0] )
  {
    return m_pSkill[0];
  }
  else if( iRandom < ( m_pBase->m_wSkillRate[0] + m_pBase->m_wSkillRate[1] ) )
  {
    return m_pSkill[1];
  }
  else
  {
    return m_pSkill[2];
  }
}
//==============================================================================================
//
//
inline int CMonster::GetAssistantSkill()
{
	if( m_pSkill[3] )
	{
		m_pUsingSkill = m_pSkill[3];
		return m_pUsingSkill->GetBaseShapeRange() + ( m_pBase->m_wSize >> 1 );
	}
	return -1;
}
//==============================================================================================
//
//
void CMonster::SetWanderRange()
{
	// Left
	m_iWanderRange[MST_WANDER_LEFT] = m_iX - (m_pBase->GetSight()*2);
	if( m_iWanderRange[MST_WANDER_LEFT] < 0 )
		m_iWanderRange[MST_WANDER_LEFT] = 0;
	// Right
	m_iWanderRange[MST_WANDER_RIGHT] = m_iX + (m_pBase->GetSight()*2);
	if( m_iWanderRange[MST_WANDER_RIGHT] > m_pInMap->GetSizeX() )
		m_iWanderRange[MST_WANDER_RIGHT] = m_pInMap->GetSizeX();
	// Top
	m_iWanderRange[MST_WANDER_TOP] = m_iY - (m_pBase->GetSight()*2);
	if( m_iWanderRange[MST_WANDER_TOP] < 0 )
		m_iWanderRange[MST_WANDER_TOP] = 0;
	// Bottom
	m_iWanderRange[MST_WANDER_BOTTOM] = m_iY + (m_pBase->GetSight()*2);
	if( m_iWanderRange[MST_WANDER_BOTTOM] > m_pInMap->GetSizeY() )
		m_iWanderRange[MST_WANDER_BOTTOM] = m_pInMap->GetSizeY();
}
//==============================================================================================
//
//
inline BOOL CMonster::GetTargetEnd( CPlayer * pThePlayer, int & EX, int & EY )
{
	static list<CLife*>			*pLifeList;
	static int							x, y, iRangeForTarget = 0;

	for( int i = 1; i <= m_iActiveRange; i++ )
	{
		iRangeForTarget += i * 8;
	}
	if( iRangeForTarget > 288 )	iRangeForTarget = 288;
	if( m_pUsingSkill != NULL && m_iActiveRange > 0 )
	{
		EX = pThePlayer->GetPosX();
		EY = pThePlayer->GetPosY();
		for( int i = 0; i < iRangeForTarget; i++ )
		{
			x  = EX;
			y  = EY;
			x += DirOffsetX[i];
			y += DirOffsetY[i];
			if( NULL != ( m_pInMap->GetTileFlag( x, y ) ) )
			{
				if( pLifeList->empty() )
				{
					m_pInMap->AddTileCode( x, y );
					EX = x;
					EY = y;
					return TRUE;
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}
//==============================================================================================
//
// Use For Pick Up Ground Item, Wait Modify...
inline int CMonster::PathFinding(const WORD & X, const WORD & Y)
{
	static int				iSize = 0, iCount = 0, iTile = 0;
	static LPPATHLIST	pPathList;
	static POINT			*pPath;

  if( m_wSpeed == 0 || m_qwSpecialStatus & SPE_STATUS_UNMOVE )     return FALSE;

#ifdef NEED_SMOOTHPATH
	pPathList = g_PathFinder.PathFinding( m_iX, m_iY, X, Y, &iSize, &iTile, m_pInMap );
	if( pPathList != NULL && iSize > 1 )
	{
		if( !pPathList->empty() )
		{
			pPath = (POINT*)(*pPathList->begin());
			pPathList->pop_front();
		}
		m_MoveObj.dwStartX_Y    = MAKELONG( pPath->y, pPath->x );

		for( iSize = 0; iSize < MAX_MOVE_END-1; iSize++ )
		{
			if( pPathList->empty() )
			{
				m_MoveObj.dwEndX_Y[iSize] = 0xFFFFFFFF;
				break;
			}
			else
			{
				pPath = (POINT*)(*pPathList->begin());
				pPathList->pop_front();
				m_MoveObj.dwEndX_Y[iSize] = MAKELONG( pPath->y, pPath->x );
			}
		}
		//if( iSize < MAX_MOVE_END )
		//{
		//	m_MoveObj.dwEndX_Y[iSize] = 0xFFFFFFFF;
		//}
		m_iMidX					= HIWORD( m_MoveObj.dwEndX_Y[0] );
		m_iMidY					= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( (WORD)pPath->x, (WORD)pPath->y );

		if( iTile > 8 )
		{
			//m_dwPathTile			= iTile >> 1;
		}
		else
		{
			//m_dwPathTile			= 0xFFFF;
		}

    m_wNotifyTimes  = 0;
    m_iPathCount		= iSize;
		m_iStepCount		= 0;
		m_iAddiStatus   = ATTACK_STATUS_MONSTER_COLLECT;
		m_iNpcStatus		= STATUS_MONSTER_PURSUE;
		m_iNextStatus   = STATUS_MONSTER_STAND;
		SetMonsterDir( m_iMidX, m_iMidY );
		// Set Move Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;
		// Send message
		memcpy( MsgMoveObj.Msgs[0].Data, &m_MoveObj, sizeof( SNMMoveObjs ) );

		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );
		return 1;
	}
	return 0;
#else
	pPath = g_PathFinder.PathFinding( m_iX, m_iY, X, Y, &iSize, m_pInMap );
	if( pPath != NULL && iSize > 1 )
	{
		m_MoveObj.dwStartX_Y		= MAKELONG( pPath[iCount].y, pPath[iCount++].x );
		memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof(DWORD) * MAX_MOVE_END );

		for( int i = 0; i < MAX_MOVE_END-1; i++ )
		{
			m_MoveObj.dwEndX_Y[i] = MAKELONG( pPath[iCount].y, pPath[iCount++].x );
			if( iCount >= iSize )
			{
				i++;
				break;
			}
		}
		if( i < MAX_MOVE_END )
		{
			m_MoveObj.dwEndX_Y[i] = 0xFFFFFFFF;
		}

		m_iMidX					= HIWORD(m_MoveObj.dwEndX_Y[0]);		
		m_iMidY					= LOWORD(m_MoveObj.dwEndX_Y[0]);
		SetPathEnd( (WORD)pPath[--iCount].x, (WORD)pPath[iCount].y );

    m_wNotifyTimes  = 0;
		m_iPathCount		= iSize;
		m_iStepCount		= 0;
		m_iAddiStatus   = ATTACK_STATUS_MONSTER_PURSUE;
		m_iNpcStatus		= STATUS_MONSTER_PURSUE;
		SetMonsterDir( m_iMidX, m_iMidY );
		// Set Move Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;
		// Send message
		memcpy( MsgMoveObj.Data[0], &m_MoveObj, sizeof( SNMMoveObjs ) );
		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );
		return 1;
	}
	return 0;
#endif
}
//==============================================================================================
//
//
inline int CMonster::PathFinding(CPlayer * pThePlayer)
{
	static int					iSize = 0, iTile = 0, iCount = 0, EX = 0, EY = 0, iPursueDir = 0;
	static LPPATHLIST		pPathList;
	static POINT				*pPath;

  if( m_wSpeed == 0 || m_qwSpecialStatus & SPE_STATUS_UNMOVE )     return FALSE;
  //
	if( pThePlayer == NULL || pThePlayer->GetMapId() != m_pInMap->GetMapId() )
	{
    m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
    m_iNpcStatus  = STATUS_MONSTER_GOHOME;
		return -1;
	}
  //
	EX  = pThePlayer->GetEndX();
	EY  = pThePlayer->GetEndY();
  if( m_iActiveRange > 0 )
  {
    iPursueDir = CalMoveDir( EX, EY );
    EX += g_MonsterPursueTableX[iPursueDir] * ( m_iActiveRange - 1 );
    EY += g_MonsterPursueTableY[iPursueDir] * ( m_iActiveRange - 1 );
  }
  //
  if( ( m_pInMap->GetTileFlag( EX, EY ) & TILE_MAPOCCULDE ) ||
      ( m_iX == EX && m_iY == EY ) )
  {
	  EX = pThePlayer->GetEndX();
	  EY = pThePlayer->GetEndY();
  }
  //
#ifdef NEED_SMOOTHPATH
	pPathList = g_PathFinder.PathFinding( m_iX, m_iY, EX, EY, &iSize, &iTile, m_pInMap );
	if( pPathList != NULL && iSize > 1 )
	{
		if( !pPathList->empty() )
		{
			pPath = (POINT*)(*pPathList->begin());
			pPathList->pop_front();
		}
		m_MoveObj.dwStartX_Y    = MAKELONG( pPath->y, pPath->x );

		for( iSize = 0; iSize < MAX_MOVE_END-1; iSize++ )
		{
			if( pPathList->empty() )
			{
				m_MoveObj.dwEndX_Y[iSize] = 0xFFFFFFFF;
				break;
			}
			else
			{
				pPath = (POINT*)(*pPathList->begin());
				pPathList->pop_front();
				m_MoveObj.dwEndX_Y[iSize] = MAKELONG( pPath->y, pPath->x );
			}
		}
    //
		m_iMidX					= HIWORD( m_MoveObj.dwEndX_Y[0] );		
		m_iMidY					= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( (WORD)pPath->x, (WORD)pPath->y );

    m_wNotifyTimes  = 0;
		m_iStepCount		= 0;
		m_iPathCount		= iSize;
		if( iTile > 8 )
		{
			//m_dwPathTile	= iTile >> 1;
		}
		else
		{
			//m_dwPathTile	= 0xFFFF;
		}

#ifdef _DEBUG
		//m_dwPathTile		= 0xFFFF;
#endif

		m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
		m_iNpcStatus	= STATUS_MONSTER_PURSUE;

		SetMonsterDir( m_iMidX, m_iMidY );
		// Set Move Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;

		// Send message
		memcpy( MsgMoveObj.Msgs[0].Data, &m_MoveObj, sizeof(SNMMoveObjs) );
		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );
		return 1;
	}
	return 0;
#else
	pPath = g_PathFinder.PathFinding( m_iX, m_iY, EX, EY, &iSize, m_pInMap );
	if( pPath != NULL && iSize > 1 )
	{
		iCount									= 0;
		m_MoveObj.dwStartX_Y    = MAKELONG( pPath[iCount].y, pPath[iCount++].x ); 
		memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof(DWORD) * MAX_MOVE_END );
		for( int i = 0; i < MAX_MOVE_END-1; i++)
		{
			m_MoveObj.dwEndX_Y[i] = MAKELONG( pPath[iCount].y, pPath[iCount++].x );
			if( iCount >= iSize )
				break;
		}
		m_iMidX					= HIWORD( m_MoveObj.dwEndX_Y[0] );		
		m_iMidY					= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( (WORD)pPath[--iCount].x, (WORD)pPath[iCount].y );

    m_wNotifyTimes  = 0;
		m_iPathCount    = iSize;
		m_iStepCount		= 0;

		m_iAddiStatus   = ATTACK_STATUS_MONSTER_PURSUE;
		m_iNpcStatus		= STATUS_MONSTER_PURSUE;

		SetMonsterDir( m_iMidX, m_iMidY );
		// Set Move Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;

		// Send message
		memcpy( MsgMoveObj.Data[0], &m_MoveObj, sizeof( SNMMoveObjs ) );

		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );

#ifndef _DEBUG_NO_ANY_LOG_
		//sprintf( g_szMonsterLog, "Monster[%s][%d] Find Path For Pursuing %s In Map(%d) #", GetName(), GetSelfCode(), m_pLastTarget->GetPlayerName(), GetMapId() );
		//AddMemoMsg( g_szMonsterLog );
#endif

		return 1;
	}
	return 0;
#endif
}
//==============================================================================================
//
//
inline int CMonster::PathFinding(CMonster * pTheTeamer)
{
	static int					iSize = 0, iTile = 0, iCount = 0, EX = 0, EY = 0;
	static LPPATHLIST		pPathList;
	static POINT				*pPath;

  if( m_wSpeed == 0 || m_qwSpecialStatus & SPE_STATUS_UNMOVE )     return FALSE;
  //
	if( pTheTeamer == NULL )
	{
		return 0;
	}
	EX = pTheTeamer->GetEndX();
	EY = pTheTeamer->GetEndY();
	//if( !GetTargetEnd( pThePlayer, EX, EY ) )
	//	return 0;

#ifdef NEED_SMOOTHPATH
	pPathList = g_PathFinder.PathFinding( m_iX, m_iY, EX, EY, &iSize, &iTile, m_pInMap );
	if( pPathList != NULL && iSize > 1 )
	{
		if( !pPathList->empty() )
		{
			pPath = (POINT*)(*pPathList->begin());
			pPathList->pop_front();
		}
		m_MoveObj.dwStartX_Y    = MAKELONG( pPath->y, pPath->x ); 

		for( iSize = 0; iSize < MAX_MOVE_END-1; iSize++ )
		{
			if( pPathList->empty() )
			{
				m_MoveObj.dwEndX_Y[iSize] = 0xFFFFFFFF;
				break;
			}
			else
			{
				pPath = (POINT*)(*pPathList->begin());
				pPathList->pop_front();
				m_MoveObj.dwEndX_Y[iSize] = MAKELONG( pPath->y, pPath->x );
			}
		}
		//if( iSize < MAX_MOVE_END )
		//{
		//	m_MoveObj.dwEndX_Y[iSize] = 0xFFFFFFFF;
		//}

		m_iMidX					= HIWORD( m_MoveObj.dwEndX_Y[0] );		
		m_iMidY					= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( (WORD)pPath->x, (WORD)pPath->y );

		m_iStepCount			= 0;
		m_iPathCount			= iSize;
		if( iTile > 8 )
		{
			//m_dwPathTile			= iTile >> 1;
		}
		else
		{
			//m_dwPathTile			= 0xFFFF;
		}

#ifdef _DEBUG
		//m_dwPathTile			= 0xFFFF;
#endif

		m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
		m_iNpcStatus	= STATUS_MONSTER_PURSUE;

		SetMonsterDir( m_iMidX, m_iMidY );
		// Set Move Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;

		// Send message
    memcpy( MsgMoveObj.Msgs[0].Data, &m_MoveObj, sizeof(SNMMoveObjs) );

		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );

		//sprintf( g_szMonsterLog, "Monster[%s][%d] Find Path For Pursuing %s In Map(%d) Line(%d) #", GetName(), GetSelfCode(), m_pLastTarget->GetPlayerName(), GetMapId(), iSize );
		//AddMemoMsg( g_szMonsterLog );

		return 1;
	}
	return 0;
#else
	pPath = g_PathFinder.PathFinding( m_iX, m_iY, EX, EY, &iSize, m_pInMap );
	if( pPath != NULL && iSize > 1 )
	{
		iCount									= 0;
		m_MoveObj.dwStartX_Y    = MAKELONG( pPath[iCount].y, pPath[iCount++].x ); 
		memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof(DWORD) * MAX_MOVE_END );
		for( int i = 0; i < MAX_MOVE_END-1; i++)
		{
			m_MoveObj.dwEndX_Y[i] = MAKELONG( pPath[iCount].y, pPath[iCount++].x );
			if( iCount >= iSize )
				break;
		}
		m_iMidX					= HIWORD( m_MoveObj.dwEndX_Y[0] );		
		m_iMidY					= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( (WORD)pPath[--iCount].x, (WORD)pPath[iCount].y );

		m_iStepCount		= 0;
		m_iPathCount		= iSize;

		m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
		m_iNpcStatus		= STATUS_MONSTER_PURSUE;

		SetMonsterDir( m_iMidX, m_iMidY );
		// Set Move Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;

		// Send message
		memcpy( MsgMoveObj.Data[0], &m_MoveObj, sizeof( SNMMoveObjs ) );

		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );

		return 1;
	}
	return 0;
#endif
}
//==============================================================================================
//
//
inline void CMonster::SetMonsterDir( const int & EX, const int & EY )
{
	static int		x, y;

	x = EX - m_iX; y = EY - m_iY;
	if(			 x <  0 && y == 0 )		m_iMoveDir = 0;
	else if( x <  0 && y <  0 )		m_iMoveDir = 1;
	else if( x == 0 && y <  0 )		m_iMoveDir = 2;
	else if( x >  0 && y <  0 )		m_iMoveDir = 3;
	else if( x >  0 && y >  0 )		m_iMoveDir = 5;
	else if( x >  0 && y == 0 )		m_iMoveDir = 4;
	else if( x == 0 && y >  0 )		m_iMoveDir = 6;
	else if( x <  0 && y >  0 )		m_iMoveDir = 7;
	else
	{
		m_iMoveDir   = gf_GetRandom( 8 );
    if( m_iNpcStatus != STATUS_MONSTER_ATTACK )
    {
      m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
			m_iNpcStatus  = STATUS_MONSTER_GOHOME;
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG
      FuncName("CMonster::SetMonsterDir");
      _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "Monster[%d][%d] Go Home Because Direction...1", GetCode(), m_pBase->GetId() );
      g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szMonsterLog );
#endif
#endif
    }
	}
#ifdef _DEBUG
	if(      x <  0 && y <  0 && abs(x) != abs(y) )
    x = y;
	else if( x >  0 && y <  0 && abs(x) != abs(y) )
    x = y;
	else if( x >  0 && y >  0 && abs(x) != abs(y) )
    x = y;
	else if( x <  0 && y >  0 && abs(x) != abs(y) )
    x = y;
#endif
}
//=============================================================================================
//
//
inline BOOL CMonster::GoInTile()// 设置怪物所在位置的地物旗标
{
	static WORD			wMonsterX, wMonsterY;
	static int			iLooper, iLooper1;

  m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );
	return TRUE;
  //
  switch( m_pBase->m_wSize )
  {
  case 1:
		m_pInMap->AddTileCode( m_iX, m_iY );
		break;
  case 2:
	  for( iLooper = 0; iLooper < 4; iLooper++ )
		{
			m_pInMap->AddTileCode( m_iX + g_MonsterSizeTile2X[iLooper], m_iY + g_MonsterSizeTile2Y[iLooper] );
		}
		break;
  case 3:
	  for( iLooper = 0; iLooper < 9; iLooper++ )
		{
			m_pInMap->AddTileCode( m_iX + g_MonsterSizeTile3X[iLooper], m_iY + g_MonsterSizeTile3Y[iLooper] );
		}
		break;
  case 4:
	  for( iLooper = 0; iLooper < 16; iLooper++ )
		{
			m_pInMap->AddTileCode( m_iX + g_MonsterSizeTile4X[iLooper], m_iY + g_MonsterSizeTile4Y[iLooper] );
		}
		break;
  case 5:
	  for( iLooper = 0; iLooper < 25; iLooper++ )
		{
			m_pInMap->AddTileCode( m_iX + g_MonsterSizeTile5X[iLooper], m_iY + g_MonsterSizeTile5Y[iLooper] );
		}
		break;
  default:
		break;
  }
  m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );
  return TRUE;
}
//=============================================================================================
//
//
inline BOOL CMonster::GoOutTile()
{
	static int			iLooper2;
	
  m_pInMap->DelLifeFromTile( (CLife*)this );
	return TRUE;

  switch( m_pBase->m_wSize )
  {
  case 1:
	  m_pInMap->DelTileCode( m_iX, m_iY );
		break;
  case 2:
	  for( iLooper2 = 0; iLooper2 < 4; iLooper2++ )
		{
			m_pInMap->DelTileCode( m_iX + g_MonsterSizeTile2X[iLooper2], m_iY + g_MonsterSizeTile2Y[iLooper2] );
		}
		break;
  case 3:
	  for( iLooper2 = 0; iLooper2 < 9; iLooper2++ )
		{
			m_pInMap->DelTileCode( m_iX + g_MonsterSizeTile3X[iLooper2], m_iY + g_MonsterSizeTile3Y[iLooper2] );
		}
		break;
  case 4:
	  for( iLooper2 = 0; iLooper2 < 16; iLooper2++ )
		{
			m_pInMap->DelTileCode( m_iX + g_MonsterSizeTile4X[iLooper2], m_iY + g_MonsterSizeTile4Y[iLooper2] );
		}
		break;
  case 5:
	  for( iLooper2 = 0; iLooper2 < 25; iLooper2++ )
		{
			m_pInMap->DelTileCode( m_iX + g_MonsterSizeTile5X[iLooper2], m_iY + g_MonsterSizeTile5Y[iLooper2] );
		}
		break;
  default:
    break;
  }
  m_pInMap->DelLifeFromTile( (CLife*)this );
  return TRUE;
}
//==============================================================================================
//
//
inline BOOL CMonster::MoveToItem()
{	
	static WORD			wMonsterNX, wMonsterNY, wNTargetDistance;
	static BOOL			bSendMsg = TRUE;

	if( MonsterTickCount < m_dwEnd[MST_MOVE_ACT] )
	{
		return TRUE;
	}

	if( m_iX == m_iEndX && m_iY == m_iEndY )
	{
		// Reach End Point, Set Status To Collect Ground Item
		m_iNpcStatus  = STATUS_MONSTER_COLLECT;
		m_iNextStatus = STATUS_MONSTER_STAND;
		m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
		return TRUE;
	}
	else if( m_iX == m_iMidX && m_iY == m_iMidY )
	{
		// Check Error
		if( ++m_iStepCount >= m_iPathCount )
		{
			SetPathEnd( m_iX, m_iY );
			ClearStatus();
			return FALSE;
		}
		// Is the mid path
		m_iMidX = HIWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
		m_iMidY = LOWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
		SetMonsterDir( m_iMidX, m_iMidY );
	}
	{
		if( GoOutTile() )
		{
			m_iUpdateTurn = 0;	
		}
    else
    {
			SetPathEnd( m_iX, m_iY );
      ClearStatus();
      m_iMoveDir = gf_GetRandom( 8 );
			return FALSE;
    }
		// If Failed, Stop Here
#ifdef _DEBUG
		GetNextStep(1);
#else
    GetNextStep();
#endif
		if( !GoInTile() && m_iX == m_iEndX && m_iY == m_iEndY )
		{
			GetLastStep();
			SetPathEnd( m_iX, m_iY );
			m_iNextStatus = m_iNpcStatus;
			m_iNpcStatus  = STATUS_MONSTER_STOP;
			m_iUpdateTurn = 0;
			m_dwEnd[MST_STOP_ACT] = MonsterTickCount + MONSTER_STOP_INTERVAL;
			return FALSE;
		}
		// Set Next Step Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount + g_SpeedArray[m_wSpeed][m_iMoveDir];
		bSendMsg = TRUE;
	}
	return TRUE;
}
//==============================================================================================
//
//
inline BOOL CMonster::MoveToTeamer()
{
	static WORD			wMonsterNX, wMonsterNY;
	static BOOL			bSendMsg = TRUE;
	static int			iAidResult;

	if( MonsterTickCount < m_dwEnd[MST_MOVE_ACT] )
	{
		return TRUE;
	}
	// Go To Next Step
	if( m_iX == m_iEndX && m_iY == m_iEndY )
	{
		// Reach End Point, Set Status To Collect Ground Item
		m_iNpcStatus  = STATUS_MONSTER_COLLECT;
		m_iNextStatus = STATUS_MONSTER_STAND;
		m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
		return TRUE;
	}
	else if( m_iX == m_iMidX && m_iY == m_iMidY )
	{
		// Check Error
		if( ++m_iStepCount >= m_iPathCount )
		{
			SetPathEnd( m_iX, m_iY );
			ClearStatus();
			return FALSE;
		}
		// Is the mid path
		m_iMidX = HIWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
		m_iMidY = LOWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
		SetMonsterDir( m_iMidX, m_iMidY );
	}
	{
		if( GoOutTile() )
		{
			m_iUpdateTurn = 0;	
		}
    else
    {
			SetPathEnd( m_iX, m_iY );
      ClearStatus();
      m_iMoveDir = gf_GetRandom( 8 );
			return FALSE;
    }
#ifdef _DEBUG
		GetNextStep(2);
#else
    GetNextStep();
#endif
		// If Failed, Stop Here
		if( !GoInTile() && m_iX == m_iEndX && m_iY == m_iEndY )
		{
			m_iNextStatus = m_iNpcStatus;
			m_iNpcStatus  = STATUS_MONSTER_STOP;
			GetLastStep();
			SetPathEnd( m_iX, m_iY );
			m_iUpdateTurn = 0;
			m_dwEnd[MST_STOP_ACT] = MonsterTickCount + MONSTER_STOP_INTERVAL;
			return FALSE;
		}
		m_wTargetDis = GetTeamerDistance();
		// Now Can Attack Target, Then Do It
		if( m_wTargetDis <= m_iActiveRange )
		{
			iAidResult = AidTeamer();
			// 援助成功 
			if( iAidResult == 1 )
			{
				m_iNpcStatus  = STATUS_MONSTER_ATTACK;
				m_iNextStatus = STATUS_MONSTER_ATTACK;
				m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
				m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + MONSTER_ATTACK_TIME;
			}
			// 没有援助招式, 取消援助
			else if( iAidResult == 0 )
			{
				ClearStatus();
			}
			// 目标为空, 需要重新寻找目标
			else if( iAidResult == -1 )
			{
				ClearStatus();
			}
			SetPathEnd( m_iX, m_iY );
		}
		//else if( m_PathFindTrigger > m_dwPathTile )
		//{
			//{
        //int     iPathResult = PathFinding( m_pLastTarget );
				//if( iPathResult == 0 )
				//{
				//	ClearStatus();
				//	return FALSE;
				//}
        //else if( iPathResult == -1 )
        //{
        //  return TRUR;
        //}
				//m_iUpdateTurn     = 0;
				//m_PathFindTrigger = 0;
			//}
		//}
		// Set Next Step Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount + g_SpeedArray[m_wSpeed][m_iMoveDir];
		bSendMsg = TRUE;
	}
	return TRUE;
}
//==============================================================================================
//
//
inline BOOL CMonster::MoveToAim() //find a path to the aim
{

	static int				iMonsteriX, iMonsteriY, iAttackResult;
	static BOOL				bSend = TRUE;

	if( MonsterTickCount < m_dwEnd[MST_MOVE_ACT] )
	{
		return TRUE;
	}
	// Reach End And Compute The Distance Deciding Attack Or Pathfind
	if( m_iX == m_iEndX && m_iY == m_iEndY )
	{
		m_pInMap->DelTileCode( m_iX, m_iY );
		// Reach End Point, Check Attack Or PathFind Again
		{
#ifdef _GO_BACK_TO_OCCUPY_3_8_
      // If map is City Map
      if( m_pInMap && m_pInMap->IsCityWarMap() )
      {
        // Find Occupy Monster, compute it coordinate and distance
        CMonster *pOccupyMonster = NULL;
        pOccupyMonster = m_pInMap->GetOccupyMonster();
        if( pOccupyMonster && IsDefenceOccupy() )
        {
          // if distance more than ?? then warp back
          DWORD l_wOccupyDis = GetDistance( pOccupyMonster->GetEndX(), pOccupyMonster->GetEndY() );
          if( l_wOccupyDis >= m_iActiveRange + 2 )
          {
            ClearStatus();
            m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
						m_iNpcStatus  = STATUS_MONSTER_GOHOME;
            return TRUE;
          }
        }
      }
#endif
			// Check Attack Distance
      m_wTargetDis = GetDistance( m_pLastTarget );
			if( m_wTargetDis <= m_iActiveRange )
			{
				m_dwEnd[MST_ATTACK_ACT] = MonsterTickCount + MONSTER_ATTACK_TIME;
				m_iNpcStatus = STATUS_MONSTER_ATTACK;
        //Attack();
				return TRUE;
			}
			else if( m_wTargetDis <= m_pBase->m_wPursueRange )
			{
				if( !( m_pBase->m_iNpcProperty & NPC_ATTRI_UNAUTO ) )
				{
          ChooseSkill();
					// 千刀万里追
          int     iPathResult = PathFinding( m_pLastTarget );
				  if( iPathResult == 0 )
				  {
				  	ClearStatus();
				  	return FALSE;
				  }
          else if( iPathResult == -1 )
          {
            return TRUE;
          }
					m_iUpdateTurn     = 0;
					m_PathFindTrigger = 0;
				}
				else
				{
					// 视线范围追
					if( m_wTargetDis > GetMonsterSight() )
					{
						ClearStatus();
						return FALSE;
					}
					else
					{
            ChooseSkill();
            int     iPathResult = PathFinding( m_pLastTarget );
				    if( iPathResult == 0 )
				    {
				  	  ClearStatus();
				  	  return FALSE;
				    }
            else if( iPathResult == -1 )
            {
              return TRUE;
            }
						m_iUpdateTurn     = 0;
						m_PathFindTrigger = 0;
					}
				}
			}
			else
			{
				// 战斗后要走回出生地, Warp回去
				if( !( m_pBase->m_iNpcProperty & NPC_ATTRI_UNRESET ) )
				{
          ClearStatus();
					if( m_iX < m_iWanderRange[MST_WANDER_LEFT] || m_iX > m_iWanderRange[MST_WANDER_RIGHT] ||
						  m_iY < m_iWanderRange[MST_WANDER_TOP]  || m_iY > m_iWanderRange[MST_WANDER_BOTTOM] )
					{
            m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
						m_iNpcStatus  = STATUS_MONSTER_GOHOME;
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG
            FuncName("CMonster::SetMonsterDir");
            _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "Monster[%d][%d] Go Home Because Fight...1", GetCode(), m_pBase->GetId() );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
            AddMemoMsg( g_szMonsterLog );
#endif
#endif
						return TRUE;
					}
				}
				else
				{
					ClearStatus();
				}
			}
			return TRUE;
		}
	}
	else if( m_iX == m_iMidX && m_iY == m_iMidY )
	{
		// Check Error
		if( ++m_iStepCount >= m_iPathCount )
		{
			SetPathEnd( m_iX, m_iY );
			ClearStatus();
			return FALSE;
		}
		// Is the mid path
		m_iMidX = HIWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
		m_iMidY = LOWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
		SetMonsterDir( m_iMidX, m_iMidY );
	}
	{
		if( GoOutTile() )
		{
			m_iUpdateTurn = 0;	
			m_PathFindTrigger++;
		}
    else
    {
			SetPathEnd( m_iX, m_iY );
      ClearStatus();
      m_iMoveDir = gf_GetRandom( 8 );
			return FALSE;
    }
#ifdef _DEBUG
		GetNextStep(3);
#else
    GetNextStep();
#endif
		if( !GoInTile() && m_iX == m_iEndX && m_iY == m_iEndY )
		{
			m_iNextStatus = m_iNpcStatus;
			m_iNpcStatus  = STATUS_MONSTER_STOP;
			GetLastStep();
			SetPathEnd( m_iX, m_iY );
			m_iUpdateTurn = 0;
			m_dwEnd[MST_STOP_ACT] = MonsterTickCount + MONSTER_STOP_INTERVAL;
			return FALSE;
		}
		//if( !m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this ) )
    //{
    //  ReinitAll();
    //  return FALSE;
    //}
		//m_pInMap->AddTileCode( m_iX, m_iY );

		// Path Find If The Target Disstance > 3 ??? Or Use Lag
		m_wTargetDis = GetDistance( m_pLastTarget );
		// Now Can Attack Target, Then Do It
		if( m_wTargetDis <= m_iActiveRange &&
        ( m_pInMap->GetTileFlag( m_iX, m_iY ) & TILE_ALLOCCULDE ) )
		{
			iAttackResult = Attack();
			// 攻击成功
			if( iAttackResult == 1 )
			{
				m_iNpcStatus  = STATUS_MONSTER_ATTACK;
				m_iNextStatus = STATUS_MONSTER_STAND;
				m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
        m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount +
                                    m_pUsingSkill->GetBeforeDelay() +
                                    m_pUsingSkill->GetAfterDelay();
			}
			// 需要换一个攻击招式, 同时重新寻路
			else if( iAttackResult == 0 )
			{
				ChooseSkill();
				m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
				m_iNextStatus = STATUS_MONSTER_ATTACK;
				m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
				//m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + MONSTER_ATTACK_TIME;
			}
			// 目标已被打死, 重新切换状态
			else if( iAttackResult == -2 || iAttackResult == -3 )
			{
				ClearStatus();
			}
			// 目标为空, 需要重新寻找目标
			else if( iAttackResult == -1 )
			{
				ClearSimStatus();
			}
			SetPathEnd( m_iX, m_iY );
		}
//		else if( m_PathFindTrigger > m_dwPathTile )
//		{
//      int     iPathResult = PathFinding( m_pLastTarget );
//			if( iPathResult == 0 )
//      {
//        ClearStatus();
//        return FALSE;
//      }
//      else if( iPathResult == -1 )
//      {
//        return TRUE;
//      }
//			m_iUpdateTurn = 0;
//			m_PathFindTrigger = 0;
//		}
		// Set Next Step Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount + g_SpeedArray[m_wSpeed][m_iMoveDir];
		bSend = TRUE;
	}
	return TRUE;
}
//==============================================================================================
//
//
void CMonster::Send_UseSkill()
{
	SNMUseSkill		*pUsingSkill = (SNMUseSkill*)(MsgUseSkill.Msgs[0].Data);
  static int    iUseSkillDir = 0;

  iUseSkillDir = CalMoveDir( m_pLastTarget->GetPosX(), m_pLastTarget->GetPosY() );
	pUsingSkill->dwCode_Dir  = ( GetSelfCode() << 16 ) | iUseSkillDir;
	if( m_pLastTarget )
	{
		pUsingSkill->dwSkillId   = ( m_pLastTarget->GetSelfCode() << 16 ) | ( m_pUsingSkill->GetId() & 0x0000FFFF );
	}
	else if( m_pLastTeamer )
	{
		pUsingSkill->dwSkillId   = ( m_pLastTeamer->GetSelfCode() << 16 ) | m_pUsingSkill->GetId();
	}
	else
	{
		pUsingSkill->dwSkillId   = m_pUsingSkill->GetId();
	}
	pUsingSkill->dwTargetX_Y = ( m_pLastTarget->GetPosX() << 16 ) | ( m_pLastTarget->GetPosY() & 0x0000FFFF );
	pUsingSkill->dwX_Y       = ( m_iX << 16 ) | ( m_iY & 0x0000FFFF );

	m_pInMap->SendMsgNearPosition_Close( MsgUseSkill, m_iX, m_iY );
}
//==============================================================================================
//
//
inline int CMonster::Attack()
{
  if( m_qwSpecialStatus & SPE_STATUS_UNATTACK )     return -1;
	if( m_pLastTarget != NULL && m_pLastTarget->GetStatus() != STATUS_PLAYER_DEAD	
#ifdef FIX_BUG_ATTACKED_WHILE_LOGOUT
      && 	m_pLastTarget->GetClientState() >= CLIENTSTATE_CONNECT
      && !(m_pLastTarget->m_bClientClaimLogout)
#endif
		)
	{
		if( MonsterTickCount > m_dwEnd[MST_ATTACK_ACT] )
		{
			// Change Skill
			if( m_pUsingSkill )
      { // Use the skill
				Send_UseSkill();
				m_wTargetX = m_pLastTarget->GetPosX();
				m_wTargetY = m_pLastTarget->GetPosY();
				if( !m_pUsingSkill->MonsterUseSkill() )
        {
					//m_iUpdateTurn = 0;
					return 0;
        }
				if( m_pLastTarget->GetStatus() == STATUS_PLAYER_DEAD )    return -2;
				else                                                      m_iUpdateTurn = 0;
			}
			else                                                        return -3;
		}
		return 1;
	}
	return -1;
}
//==============================================================================================
//
//
inline int CMonster::AidTeamer()
{
	if( m_pLastTeamer != NULL )
	{
		if( MonsterTickCount > m_dwEnd[MST_ATTACK_ACT] )
		{
			// Change Skill
			if( m_pUsingSkill )
      { // Use the skill
				Send_UseSkill();
				if( !m_pUsingSkill->MonsterUseSkill() )
        {
					m_iUpdateTurn = 0;
					return 0;
        }
				m_iUpdateTurn = 0;
			}
			else                    return 0;
		}
		return 1;
	}
	return -1;
}
//=============================================================================================
//
//
inline BOOL CMonster::CheckRedPlayer()
{
	LPLifeTileList											pLifeList = NULL;
	int																	iDisX, iDisY, i, iEndSight = 0, iStartSight;
	static LifeTileList::iterator				iterLife;

  iStartSight  = m_iCurrSight * m_iQtrSight;
	iEndSight    = iStartSight  + m_iQtrSight;
  m_iCurrSight = (++m_iCurrSight)%4;
	for( i = iStartSight; i < iEndSight; i++ )
	{
		iDisX = m_iX + DirOffsetX[i];		iDisY = m_iY + DirOffsetY[i];
		pLifeList = m_pInMap->GetTileLifeList( iDisX, iDisY );
		if( pLifeList )
		{
			if( pLifeList->empty() )				continue;
			iterLife = pLifeList->begin();
			if( (*iterLife)->IsPlayer() )
      {
        // m_iNpcStatus   = STATUS_MONSTER_PURSUE;
        m_pLastTarget  = (CPlayer*)(*iterLife);
        if( m_pLastTarget->GetInMap() != m_pInMap )
        {
          m_pInMap->DelLifeFromTile( m_pLastTarget );
          m_pLastTarget = NULL;
          break;
        }
        //
				if( m_pLastTarget->GetPkCount() > 1 && m_pLastTarget->GetStatus() != STATUS_PLAYER_DEAD && !( m_pLastTarget->GetGMStatus() & 0x00000003 ) )
        {
          m_dwTargetCode = (*iterLife)->GetCode();
          m_iCurrSight   = 0;
          m_wNotifyTimes = 0;
          ChooseSkill();
          return 1;
        }
        m_pLastTarget = NULL;
      }
      // If this tile have tow life, Abort it
		}
	}
	if( ( GetMonsterSight() - iEndSight ) >= m_iQtrSight )	  return -1;
	return 0;
}
//=============================================================================================
//
//
inline BOOL CMonster::CheckItem()
{
	LPGroundItemTileLsit						      pItemList = NULL;
	static WORD														iCurrX, iCurrY, wCheckCount;
	GroundItemTileLsit::iterator		      iter_GItem;
	CGroundItem										        *pDelGrndItem = NULL;
	CItem													        *pDelCItem = NULL;
  int                                   iStartSight = 0, iEndSight = 0;

  if( m_qwSpecialStatus & SPE_STATUS_UNITEM )          return 0;
	// Check Auto Disappear Item Timer
	if( !m_listCollectItem.empty() )
	{
		for( iter_GItem = m_listCollectItem.begin(); iter_GItem != m_listCollectItem.end(); )
		{
			if( NULL != ( pDelGrndItem = (*iter_GItem) ) )
			{
				if( MonsterTickCount > pDelGrndItem->GetVanishTime() )
				{
					iter_GItem = m_listCollectItem.erase( iter_GItem );
					if( NULL != ( pDelCItem = pDelGrndItem->GetBaseItem() ) )
					{
		        g_pGs->DeleteCItem( pDelCItem, 8 );
            pDelCItem = NULL;
					}
					SAFE_DELETE( pDelGrndItem );
				}
        else
        {
          iter_GItem++;
        }
			}
      else
      {
        iter_GItem = m_listCollectItem.erase( iter_GItem );
      }
		}
	}
	// Check Ground Item
  iStartSight  = m_iCurrSight * m_iQtrSight;
	iEndSight    = iStartSight  + m_iQtrSight;
  m_iCurrSight = (++m_iCurrSight)%4;
	wCheckCount  = 0;
	for( int i = iStartSight; i < iEndSight; i++ )
	{
		iCurrX = m_iX + DirOffsetX[i];
    iCurrY = m_iY + DirOffsetY[i];
    //
		if( NULL != ( pItemList = m_pInMap->GetTileItemList( iCurrX, iCurrY ) ) )
		{
			if( pItemList->empty() )                              continue;
      //
			for( iter_GItem = pItemList->begin(); iter_GItem != pItemList->end(); iter_GItem++ )
			{
				pDelGrndItem  = (*iter_GItem);
        if( pDelGrndItem->GetMasterMailId() && MonsterTickCount < pDelGrndItem->GetMasterTime() )
        {
          continue;
        }
				m_listGroundItemPos.push_back( ( ( pDelGrndItem->GetPosX() << 16 ) | pDelGrndItem->GetPosY() ) );
				m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
				m_iNextStatus = STATUS_MONSTER_COLLECT;
				m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
				wCheckCount++;
			}
    }
	}
  //
  if( wCheckCount > 0 )                                     return 1;
	if( ( GetMonsterSight() - iEndSight ) >= m_iQtrSight )	  return -1;
	return 0;
}
//=============================================================================================
//
//
inline int CMonster::CheckGuildRange()
{
	LPLifeTileList											pLifeList = NULL;
	int																	iDisX, iDisY, i, iEndSight = 0, iStartSight;
	LifeTileList::iterator				      iterLife;

  if( m_qwSpecialStatus & SPE_STATUS_UNATTACK )       return 0;
  //
  iStartSight  = m_iCurrSight * m_iQtrSight;
	iEndSight    = iStartSight  + m_iQtrSight;
  m_iCurrSight = (++m_iCurrSight)%4;
	for( i = iStartSight; i < iEndSight; i++ )
	{
		iDisX = m_iX + DirOffsetX[i];		iDisY = m_iY + DirOffsetY[i];
		pLifeList = m_pInMap->GetTileLifeList( iDisX, iDisY );
		if( pLifeList )
		{
			if( pLifeList->empty() )				continue;
			iterLife = pLifeList->begin();
			if( (*iterLife)->IsPlayer() )
      {
        //m_iNpcStatus   = STATUS_MONSTER_PURSUE;
        m_pLastTarget  = (CPlayer*)(*iterLife);
        if( m_pLastTarget->GetInMap() != m_pInMap )
        {
          m_pInMap->DelLifeFromTile( m_pLastTarget );
          m_pLastTarget = NULL;
          break;
        }
        if( m_pLastTarget->GetStatus() != STATUS_PLAYER_DEAD &&
            m_pLastTarget->GetDeclareCityWarMap() == GetParam1() &&
            !( m_pLastTarget->GetGMStatus() & 0x00000003 ) )
        {
          m_dwTargetCode = (*iterLife)->GetCode();
          m_iCurrSight   = 0;
          m_wNotifyTimes = 0;
          ChooseSkill();
          return 1;
        }
        m_pLastTarget = NULL;
      }
      // If this tile have tow life, Abort it
		}
	}
	if( ( GetMonsterSight() - iEndSight ) >= m_iQtrSight )	  return -1;
	return 0;
}
//=============================================================================================
//
//
inline int CMonster::CheckRange()
{
	LPLifeTileList											pLifeList = NULL;
	int																	iDisX, iDisY, i, iEndSight = 0, iStartSight;
	LifeTileList::iterator				      iterLife;

  if( m_qwSpecialStatus & SPE_STATUS_UNATTACK )       return 0;
  //
  iStartSight  = m_iCurrSight * m_iQtrSight;
	iEndSight    = iStartSight  + m_iQtrSight;
  m_iCurrSight = (++m_iCurrSight)%4;
	for( i = iStartSight; i < iEndSight; i++ )
	{
		iDisX = m_iX + DirOffsetX[i];		iDisY = m_iY + DirOffsetY[i];
		pLifeList = m_pInMap->GetTileLifeList( iDisX, iDisY );
		if( pLifeList )
		{
			if( pLifeList->empty() )				continue;
			iterLife = pLifeList->begin();
			if( (*iterLife)->IsPlayer() )
      {
        //m_iNpcStatus   = STATUS_MONSTER_PURSUE;
        m_pLastTarget  = (CPlayer*)(*iterLife);
        if( m_pLastTarget->GetInMap() != m_pInMap )
        {
          m_pInMap->DelLifeFromTile( m_pLastTarget );
          m_pLastTarget = NULL;
          break;
        }
        if( m_pLastTarget->GetStatus() != STATUS_PLAYER_DEAD && !( m_pLastTarget->GetGMStatus() & 0x00000003 ) )
        {
#ifdef _FUNCTION_RING_3_8_
          if( FALSE == m_pLastTarget->IsDiver() )
#endif
          {
            m_dwTargetCode = (*iterLife)->GetCode();
            m_iCurrSight   = 0;
            m_wNotifyTimes = 0;
            ChooseSkill();
            return 1;
          }
        }
        m_pLastTarget = NULL;
      }
      // If this tile have tow life, Abort it
		}
	}
	if( ( GetMonsterSight() - iEndSight ) >= m_iQtrSight )	  return -1;
	return 0;
}
//==============================================================================================
//
//
inline int CMonster::GetEscapeDir()
{
	if( m_pLastHit == NULL )
		return gf_GetRandom(8);
	int		x = m_pLastHit->GetPosX() - m_iX, y = m_pLastHit->GetPosY() - m_iY;
	
	if(			 x <  0 && y == 0 )		return 0;
	else if( x <  0 && y <  0 )		return 1;
	else if( x == 0 && y <  0 )		return 2;
	else if( x >  0 && y <  0 )		return 3;
	else if( x >  0 && y >  0 )		return 5;
	else if( x >  0 && y == 0 )		return 4;
	else if( x == 0 && y >  0 )		return 6;
	else if( x <  0 && y >  0 )		return 7;
	else													return 0;	
}
//==============================================================================================
//
//
inline BOOL CMonster::Escape()
{
	static int					iSight, EscX = 0, EscY = 0;
	// Check State
	if( m_iNpcStatus == STATUS_MONSTER_WANDER && m_pBase->m_wSpeed )
	{
		return FALSE;
	}
	else
	{
		if( gf_GetRandom( 100 ) < 50 )
		{
			return TRUE;
		}
		m_dwEnd[MST_WANDER_ACT] = 0;
		m_bReachEnd             = TRUE;
		//return TRUE;
	}
	m_iMoveDir = g_iEscapeDir[GetEscapeDir()][gf_GetRandom(5)];
	iSight     = m_pBase->m_wHaltRange;
	switch( m_iMoveDir )
	{
	case 0:
		EscX = m_iX - iSight;		EscY = m_iY;
		break;
	case 1:
		EscX = m_iX - iSight;		EscY = m_iY - iSight;
		break;
	case 2:
		EscX = m_iX;						EscY = m_iY - iSight;
		break;
	case 3:
		EscX = m_iX + iSight;		EscY = m_iY - iSight;
		break;
	case 4:
		EscX = m_iX + iSight;		EscY = m_iY;
		break;
	case 5:
		EscX = m_iX + iSight;		EscY = m_iY + iSight;
		break;
	case 6:
		EscX = m_iX;						EscY = m_iY + iSight;
		break;
	case 7:
		EscX = m_iX - iSight;		EscY = m_iY + iSight;
		break;
	default:
		EscX = m_iX;						EscY = m_iY;
		break;
	}
	if( m_pInMap->IsInClientMap( EscX, EscY ) )
	{	
		//if( iStartX > m_iWanderRange[MST_WANDER_LEFT] && iStartY > m_iWanderRange[MST_WANDER_TOP] && iStartX < m_iWanderRange[MST_WANDER_RIGHT] && iStartY < m_iWanderRange[MST_WANDER_BOTTOM] )
		{
			if( EscX > m_iWanderRange[MST_WANDER_LEFT] && EscY > m_iWanderRange[MST_WANDER_TOP] && EscX < m_iWanderRange[MST_WANDER_RIGHT] && EscY < m_iWanderRange[MST_WANDER_BOTTOM] )
			{
				if( TestNpcPath( m_iX, m_iY, EscX, EscY, m_iMoveDir, m_pInMap ) ) 
				{
					m_wSpeed = m_pBase->m_wEscapeSpeed;
					m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_pBase->m_wEscapeSpeed;
					m_MoveObj.dwStartX_Y		= ( m_iX << 16 ) | ( m_iY & 0x0000FFFF );
					m_MoveObj.dwEndX_Y[0]		= ( EscX << 16 ) | ( EscY & 0x0000FFFF );
					m_MoveObj.dwEndX_Y[1]		= 0xFFFFFFFF;

					m_iMidX	= EscX;		
					m_iMidY	=	EscY;
					SetPathEnd( EscX, EscY );

					m_iPathCount						= 1;
					m_iStepCount						= 0;
					m_iAddiStatus					  = ATTACK_STATUS_MONSTER_ESCAPE;
					m_iNpcStatus						= STATUS_MONSTER_WANDER;
					m_iNextStatus						= STATUS_MONSTER_STAND;
					m_dwEnd[MST_ATTACK_ACT] = MonsterTickCount + g_SpeedArray[m_wSpeed][m_iMoveDir];

					// Send Wander Message
					memcpy( MsgMoveObj.Msgs[0].Data, &m_MoveObj, sizeof( SNMMoveObjs ) );
					
					m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
//==============================================================================================
// Wander Short Path
//
inline BOOL CMonster::WanderShortPath()
{
	static BOOL				bGoToNext1 = TRUE;

	if( m_pBase->m_wSpeed < 1 )		return TRUE;
	// 
	if( m_iX != m_iEndX || m_iY != m_iEndY )
	{
		m_bReachEnd = FALSE;
		if( MonsterTickCount > m_dwEnd[MST_MOVE_ACT] )
		{
			// Try to move the next step
			if( m_iX == m_iMidX && m_iY == m_iMidY )
			{
				// Check Error
				if( ++m_iStepCount >= m_iPathCount )
				{
					SetPathEnd( m_iX, m_iY );
					ClearStatus();
					return FALSE;
				}
				if( m_MoveObj.dwEndX_Y[m_iStepCount] == 0xFFFFFFFF )
				{
					SetPathEnd( m_iX, m_iY );
					return TRUE;
				}
				// Is the mid path
				m_iMidX = HIWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
				m_iMidY = LOWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );	
				SetMonsterDir( m_iMidX, m_iMidY );
			}
			else if( m_iX > m_pInMap->GetSizeX() || m_iY > m_pInMap->GetSizeY() || m_iX == 0 || m_iY == 0 )
			{
				m_iHp = m_pBase->GetHp();
	      m_iX = m_iInitX + gf_GetRandom( m_pBase->m_wSightRange );
	      m_iY = m_iInitY + gf_GetRandom( m_pBase->m_wSightRange );
        if( m_pInMap->GetTileFlag( m_iX, m_iY ) & TILE_ALLOCCULDE )
        {
          m_iX = m_iInitX;
          m_iY = m_iInitY;
        }
				SetPathEnd( m_iX, m_iY );
				ClearStatus();
				m_dwEnd[MST_STAND_ACT]   = MonsterTickCount + MONSTER_SEARCH_TIME;

				//m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );

				return FALSE;
			}
			// Change Monster Position
			if( GoOutTile() )
			{
				m_iUpdateTurn = 0;
				m_PathFindTrigger++;
			}
			else
			{
				SetPathEnd( m_iX, m_iY );
				ClearStatus();
				m_iMoveDir = gf_GetRandom( 8 );
				return FALSE;
			}
#ifdef _DEBUG
		  GetNextStep(4);
#else
      GetNextStep();
#endif
			if( !GoInTile() && m_iX == m_iEndX && m_iY == m_iEndY )
			{
				m_iNextStatus = m_iNpcStatus;
				m_iNpcStatus  = STATUS_MONSTER_STOP;
				GetLastStep();
				SetPathEnd( m_iX, m_iY );
				m_iUpdateTurn = 0;
				m_dwEnd[MST_STOP_ACT] = MonsterTickCount + MONSTER_STOP_INTERVAL;
				return FALSE;
			}
			//m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );
			// Set Next Step Time
			m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount + g_SpeedArray[m_wSpeed][m_iMoveDir];
			return TRUE; 
		}
	}
	else if( m_MoveObj.dwEndX_Y[m_iStepCount+1] == 0xFFFFFFFF )// && ( m_iStepCount >= ( m_iPathCount - 1 ) ) )
	{
		// If reach the end, ready the next wander
		if( m_iNpcStatus == STATUS_MONSTER_WANDER && m_bReachEnd == FALSE )
		{
			if( m_pBase->m_iNpcProperty & NPC_ATTRI_COLLECT )
			{
				m_dwEnd[MST_WANDER_ACT]	= MonsterTickCount + ( MONSTER_WANDER_TIME >> 2 ) - gf_GetRandom( 2500 );
        m_dwEnd[MST_STAND_ACT]  = m_dwEnd[MST_WANDER_ACT];
			}
			else
			{
				m_dwEnd[MST_WANDER_ACT]	= MonsterTickCount + MONSTER_WANDER_TIME - gf_GetRandom( 5000 );
        m_dwEnd[MST_STAND_ACT]  = m_dwEnd[MST_WANDER_ACT];
			}
			m_iNpcStatus  = STATUS_MONSTER_STAND;
			m_iUpdateTurn = 0;
			m_bReachEnd   = TRUE;
//#ifdef _DEBUG
//			if( GetMapId() == 11201 && GetSelfCode() == 16385 )
//			{
//				sprintf( g_szMonsterLog, "%s Get Wander Interval=%d(ms) #", GetName(), ( m_dwEnd[MST_WANDER_ACT] - MonsterTickCount ) / 1000 );
//				AddMemoErrMsg( g_szMonsterLog );
//			}
//#endif
			return TRUE;
		}
		// If the lag time is over, then try to get a wander path
		if( MonsterTickCount > m_dwEnd[MST_WANDER_ACT] )
		{
			// Get a wander path
			if( m_pBase->m_wSpeed < 4 )
			{
				if( GetShortWanderPath() )
				{
//#ifdef _DEBUG
//					if( GetMapId() == 11201 && GetSelfCode() == 16385 )
//					{
//						sprintf( g_szMonsterLog, "%s Get Wander Path [%d][%d] #", GetName(), m_dwEnd[MST_WANDER_ACT], MonsterTickCount );
//						AddMemoErrMsg( g_szMonsterLog );
//					}
//#endif
					// Send the path to client
					m_iNpcStatus    = STATUS_MONSTER_WANDER;
					m_iUpdateTurn   = 0;
					m_iAddiStatus   = ATTACK_STATUS_MONSTER_WANDER;
					m_wSpeed			  = m_pBase->m_wSpeed;
					m_bReachEnd     = FALSE;
					m_wNotifyTimes  = 0;
					return TRUE;
				}
			}
			else
			{
				if( GetWanderPath() )
				{
//#ifdef _DEBUG
//					if( GetMapId() == 11201 && GetSelfCode() == 16385 )
//					{
//						sprintf( g_szMonsterLog, "%s Get Wander Path [%d][%d] #", GetName(), m_dwEnd[MST_WANDER_ACT], MonsterTickCount );
//						AddMemoErrMsg( g_szMonsterLog );
//					}
//#endif
					// Send the path to client
					m_iNpcStatus    = STATUS_MONSTER_WANDER;
					m_iUpdateTurn   = 0;
					m_iAddiStatus   = ATTACK_STATUS_MONSTER_WANDER;
					m_wSpeed			  = m_pBase->m_wSpeed;
					m_bReachEnd     = FALSE;
					m_wNotifyTimes  = 0;
					return TRUE;
				}
			}
			m_dwEnd[MST_WANDER_ACT]	= MonsterTickCount + MONSTER_WANDER_TIME - gf_GetRandom( 5000 );
      m_dwEnd[MST_STAND_ACT]	= m_dwEnd[MST_WANDER_ACT];
		}
    else    m_dwEnd[MST_STAND_ACT]		= MonsterTickCount + 2000;
		return TRUE;
	}
  else
  {
    //m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
		//m_iNpcStatus  = STATUS_MONSTER_GOHOME;
    //SetPathEnd( m_iX, m_iY );
    //ClearStatus();
    //m_dwEnd[MST_SEARCH_ACT] = MonsterTickCount + MONSTER_SEARCH_TIME;
    if( m_iStepCount + 1 < MAX_MOVE_END )      m_MoveObj.dwEndX_Y[m_iStepCount+1] = 0xFFFFFFFF;
  }
	return FALSE;
}
//==============================================================================================
//
//
// Wander Long Path
//
inline BOOL CMonster::Wander()
{
	static BOOL				bGotoNext = TRUE; //bReachEnd = FALSE;

//#ifdef SRVD_NO_MONSTER_WANDER
//	return FALSE;
//#endif

	// Wander Check, Then Check The Range Of Sight
	// ...
	// Clear Target And Wander
	//m_pLastTarget	  = NULL;
	//m_dwTargetCode  = 0;
	if( m_pBase->m_wSpeed < 1 )		return TRUE;
	if( m_iX != m_iEndX || m_iY != m_iEndY )
	{
		m_bReachEnd = FALSE;
		if( MonsterTickCount > m_dwEnd[MST_MOVE_ACT] )
		{
			// Try to move the next step
			if( m_iX == m_iMidX && m_iY == m_iMidY )
			{
				// Check Error
				if( ++m_iStepCount >= m_iPathCount )
				{
					SetPathEnd( m_iX, m_iY );
					ClearStatus();
					return FALSE;
				}
				if( m_MoveObj.dwEndX_Y[m_iStepCount] == 0xFFFFFFFF )
				{
					SetPathEnd( m_iX, m_iY );
					return TRUE;
				}
				// Is the mid path
				m_iMidX = HIWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );
				m_iMidY = LOWORD( m_MoveObj.dwEndX_Y[m_iStepCount] );	
				SetMonsterDir( m_iMidX, m_iMidY );	
			}
			else if( m_iX > m_pInMap->GetSizeX() || m_iY > m_pInMap->GetSizeY() || m_iX == 0 || m_iY == 0 )
			{
				m_iHp = m_pBase->GetHp();
				m_iX = m_iInitX + gf_GetRandom( m_pBase->m_wSightRange );
				m_iY = m_iInitY + gf_GetRandom( m_pBase->m_wSightRange );
				SetPathEnd( m_iX, m_iY );
				ClearStatus();
				m_dwEnd[MST_SEARCH_ACT]   = MonsterTickCount + MONSTER_SEARCH_TIME;

				//m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );

				return FALSE;
			}
			// Change Monster Position
			if( GoOutTile() )
			{
				m_iUpdateTurn = 0;
				m_PathFindTrigger++;
			}
			else
			{
				SetPathEnd( m_iX, m_iY );
				ClearStatus();
				m_iMoveDir = gf_GetRandom( 8 );
				return FALSE;
			}
#ifdef _DEBUG
		  GetNextStep(5);
#else
      GetNextStep();
#endif
			if( !GoInTile() && m_iX == m_iEndX && m_iY == m_iEndY )
			{
				m_iNextStatus = m_iNpcStatus;
				m_iNpcStatus  = STATUS_MONSTER_STOP;
				GetLastStep();
				SetPathEnd( m_iX, m_iY );
				m_iUpdateTurn = 0;
				m_dwEnd[MST_STOP_ACT] = MonsterTickCount + MONSTER_STOP_INTERVAL;
				return FALSE;
			}
			//m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );

			// Set Next Step Time
			m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount + g_SpeedArray[m_wSpeed][m_iMoveDir];
			return TRUE;
		}
	}
	else if( m_MoveObj.dwEndX_Y[m_iStepCount+1] == 0xFFFFFFFF )// && ( m_iStepCount >= ( m_iPathCount - 1 ) ) )
	{
		// If reach the end, ready the next wander
		if( m_iNpcStatus == STATUS_MONSTER_WANDER && m_bReachEnd == FALSE )
		{
			m_dwEnd[MST_WANDER_ACT]	= MonsterTickCount + MONSTER_WANDER_TIME - gf_GetRandom( 5000 );
      m_dwEnd[MST_STAND_ACT]  = m_dwEnd[MST_WANDER_ACT];
			m_wSpeed			= m_pBase->m_wSpeed;
			m_iNpcStatus  = STATUS_MONSTER_STAND;
			m_iUpdateTurn = 0;
			m_bReachEnd   = TRUE;
			return TRUE;
		}
		// If the lag time is over, then try to get a wander path
		if( MonsterTickCount > m_dwEnd[MST_WANDER_ACT] )
		{
			// Get a wander path
			if( GetWanderPath() )
			{
				// Send the path to client
				m_iNpcStatus   = STATUS_MONSTER_WANDER;
				m_iUpdateTurn  = 0;
				m_iAddiStatus  = ATTACK_STATUS_MONSTER_WANDER;
				m_wSpeed			 = m_pBase->m_wSpeed;
				//m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + MONSTER_MOVE_TIME;
				m_bReachEnd    = FALSE;
				m_wNotifyTimes = 0;
				return TRUE;
			}
			m_dwEnd[MST_WANDER_ACT]	= MonsterTickCount + MONSTER_WANDER_TIME - gf_GetRandom( 5000 );
      if( m_pBase->m_iNpcProperty & NPC_ATTRI_COLLECT )
      {
			  m_dwEnd[MST_STAND_ACT]  = MonsterTickCount + gf_GetRandom( 5000 );
      }
      else
      {
			  m_dwEnd[MST_STAND_ACT]  = MonsterTickCount + MONSTER_STAND_TIME - gf_GetRandom( 5000 );
      }
      m_dwEnd[MST_SEARCH_ACT]   = MonsterTickCount + MONSTER_SEARCH_TIME;
		}
    m_dwEnd[MST_STAND_ACT]		= MonsterTickCount + 1000;
		return TRUE;
	}
  else
  {
    //m_iX = m_iInitX + gf_GetRandom( m_pBase->m_wSightRange );
		//m_iY = m_iInitY + gf_GetRandom( m_pBase->m_wSightRange );
    //SetPathEnd( m_iX, m_iY );
    //ClearStatus();
    //m_dwEnd[MST_SEARCH_ACT] = MonsterTickCount + MONSTER_SEARCH_TIME;
    m_MoveObj.dwEndX_Y[m_iStepCount+1] = 0xFFFFFFFF;
  }
	return FALSE;
}
//==============================================================================================
//
//
inline int  CMonster::GetWanderPath()
{
	static int				iOffSetX, iOffSetY, iDir, iStartX, iStartY, iEndX, iEndY, iOffSet;
	       int				iSightRange, i, j, iBigWanderLineCount;
	static BOOL				bPath;

  //
  if( m_wSpeed == 0 || m_qwSpecialStatus & SPE_STATUS_UNMOVE )     return FALSE;
  //
	// Init Vars
	iSightRange = ( m_pBase->GetSight() );
	bPath       = FALSE;
	//memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof( DWORD ) * MAX_MOVE_END );
	iBigWanderLineCount = gf_GetRandom( MAX_WANDER_PATH_COUNT ) + 1;

	for( i = 0, j = 0; i < iBigWanderLineCount; i++ )
	{
		if( j > 0 )
		{
			iStartX = HIWORD( m_MoveObj.dwEndX_Y[j-1] );
			iStartY = LOWORD( m_MoveObj.dwEndX_Y[j-1] ); 
		}
		else
		{
			iStartX = m_iX;
			iStartY = m_iY;
		}
		iOffSetX  = 0; 
		iOffSetY	= 0;
		iDir      = gf_GetRandom( 8 );
		iOffSet   = gf_GetRandom( iSightRange ) + 1;
		switch( iDir )
		{
		case 0:
			iOffSetX -= iOffSet;	iOffSetY += 0;
			break;
		case 1:
			iOffSetX -= iOffSet;	iOffSetY -= iOffSet;
			break;
		case 2:
			iOffSetX += 0;				iOffSetY -= iOffSet;
			break;
		case 3:
			iOffSetX += iOffSet;	iOffSetY -= iOffSet;
			break;
		case 4:
			iOffSetX += iOffSet;	iOffSetY += 0;
			break;
		case 5:
			iOffSetX += iOffSet;	iOffSetY += iOffSet;
			break;
		case 6:
			iOffSetX += 0;				iOffSetY += iOffSet;
			break;
		case 7:
			iOffSetX -= iOffSet;	iOffSetY += iOffSet;
			break;
		default:
			iOffSetX  = 1;				iOffSetY  = 1;	iDir = 5;
			break;
		}
		//if( iOffSetX || iOffSetY )
		{
			iEndX = ( iStartX + iOffSetX );
			iEndY = ( iStartY + iOffSetY );
			if( m_pInMap->IsInClientMap( iEndX, iEndY ) )
			{	
				if( iStartX > m_iWanderRange[MST_WANDER_LEFT] && iStartY > m_iWanderRange[MST_WANDER_TOP] && iStartX < m_iWanderRange[MST_WANDER_RIGHT] && iStartY < m_iWanderRange[MST_WANDER_BOTTOM] )
				{
					if( iEndX > m_iWanderRange[MST_WANDER_LEFT] && iEndY > m_iWanderRange[MST_WANDER_TOP] && iEndX < m_iWanderRange[MST_WANDER_RIGHT] && iEndY < m_iWanderRange[MST_WANDER_BOTTOM] )
					{
						if( TestNpcPath( iStartX, iStartY, iEndX, iEndY, iDir, m_pInMap ) ) 
						{
							bPath = TRUE;
							m_MoveObj.dwEndX_Y[j++] = MAKELONG( iEndY, iEndX );
						}
					}
				}
			}
		}
	}
	if( bPath == TRUE )
	{
		// Set Wander Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		// Set Wander Status And Base Data
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;
		m_MoveObj.dwStartX_Y		= ( m_iX << 16 ) | ( m_iY & 0x0000FFFF );

		m_iMidX									= HIWORD( m_MoveObj.dwEndX_Y[0] );		
		m_iMidY									= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( HIWORD( m_MoveObj.dwEndX_Y[j-1] ), LOWORD( m_MoveObj.dwEndX_Y[j-1] ) );

		m_iPathCount						= j;
    m_wWanderPath           = 0;
		m_iStepCount						= 0;
		m_iAddiStatus					  = ATTACK_STATUS_MONSTER_WANDER;
		m_iNpcStatus						= STATUS_MONSTER_WANDER;

		for( ; j < MAX_WANDER_PATH_COUNT + 1; j++ )
		{
			m_MoveObj.dwEndX_Y[j]	= 0xFFFFFFFF;
		}
		SetMonsterDir( m_iMidX, m_iMidY );

		// Send Wander Message
		memcpy( MsgMoveObj.Msgs[0].Data, &m_MoveObj, sizeof( SNMMoveObjs ) );

		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );
		return TRUE;
	}
	else
	{
		// 多次没找到路径，是不是卡在障碍中，引导Monster走出来
    //if( m_wWanderPath++ > 100 )
    //{
      //int       iMonsterRandX = 0, iMonsterRandY = 0;
      //for( int z = 0; z < 10; z++ )
      //{
      //  iMonsterRandX = gf_GetRandom( m_pInMap->GetSizeX() );
      //  iMonsterRandY = gf_GetRandom( m_pInMap->GetSizeY() );
      //  if( !( m_pInMap->GetTileFlag( iMonsterRandX, iMonsterRandY ) & TILE_ALLOCCULDE ) )
      //  {
      //    m_iInitX      = iMonsterRandX;
      //    m_iInitY      = iMonsterRandY;
    //      m_iNpcStatus  = STATUS_MONSTER_GOHOME;
    //      m_wWanderPath = 0;
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG
    //      FuncName("CMonster::SetMonsterDir");
    //      sprintf( g_szMonsterLog, "Monster[%d][%d][%d] Go Home Because Wander1, Pos(%d,%d)(%d,%d)", GetCode(), m_pBase->GetId(), GetMapId(), m_iX, m_iY, m_iInitX, m_iInitY );
    //      AddMemoMsg( g_szMonsterLog );
#endif
#endif
      //    break;
      //  }
      //}
    //}
	}
	return FALSE;
}
//==============================================================================================
//
//
inline int CMonster::GetShortWanderPath()
{
	static int				iOffSetX, iOffSetY, iDir, iStartX, iStartY, iEndX, iEndY, iOffSet;
	       int				iSightRange, i, j, iWanderLineCount;
	static BOOL				bPath;

  //
  if( m_wSpeed == 0 || m_qwSpecialStatus & SPE_STATUS_UNMOVE )     return FALSE;
  //
	// Init Vars
	iSightRange = ( m_pBase->GetSight() );
	bPath       = FALSE;
	//memset( m_MoveObj.dwEndX_Y, 0xFF, sizeof( DWORD ) * MAX_MOVE_END );
	iWanderLineCount = gf_GetRandom( MAX_WANDER_PATH_COUNT_SHORT ) + 1;

	for( i = 0, j = 0; i < iWanderLineCount; i++ )
	{
		if( j > 0 )
		{
			iStartX = HIWORD( m_MoveObj.dwEndX_Y[j-1] );
			iStartY = LOWORD( m_MoveObj.dwEndX_Y[j-1] ); 
		}
		else
		{
			iStartX = m_iX;
			iStartY = m_iY;
		}
		iOffSetX  = 0;
		iOffSetY	= 0;
		iDir      = gf_GetRandom( 8 );
		iOffSet   = gf_GetRandom( iSightRange >> 1 ) + 1;
		switch( iDir )
		{
		case 0:
			iOffSetX -= iOffSet;	iOffSetY += 0;
			break;
		case 1:
			iOffSetX -= iOffSet;	iOffSetY -= iOffSet;
			break;
		case 2:
			iOffSetX += 0;				iOffSetY -= iOffSet;
			break;
		case 3:
			iOffSetX += iOffSet;	iOffSetY -= iOffSet;
			break;
		case 4:
			iOffSetX += iOffSet;	iOffSetY += 0;
			break;
		case 5:
			iOffSetX += iOffSet;	iOffSetY += iOffSet;
			break;
		case 6:
			iOffSetX += 0;				iOffSetY += iOffSet;
			break;
		case 7:
			iOffSetX -= iOffSet;	iOffSetY += iOffSet;
			break;
		default:
			iOffSetX  = 1;				iOffSetY  = 1;	iDir = 5;
			break;
		}
		//if( iOffSetX || iOffSetY )
		{
			iEndX = ( iStartX + iOffSetX );
			iEndY = ( iStartY + iOffSetY );
			if( m_pInMap->IsInClientMap( iEndX, iEndY ) )
			{	
				if( iStartX > m_iWanderRange[MST_WANDER_LEFT] && iStartY > m_iWanderRange[MST_WANDER_TOP] && iStartX < m_iWanderRange[MST_WANDER_RIGHT] && iStartY < m_iWanderRange[MST_WANDER_BOTTOM] )
				{
					if( iEndX > m_iWanderRange[MST_WANDER_LEFT] && iEndY > m_iWanderRange[MST_WANDER_TOP] && iEndX < m_iWanderRange[MST_WANDER_RIGHT] && iEndY < m_iWanderRange[MST_WANDER_BOTTOM] )
					{
						if( TestNpcPath( iStartX, iStartY, iEndX, iEndY, iDir, m_pInMap ) ) 
						{
							bPath = TRUE;
							m_MoveObj.dwEndX_Y[j++] = MAKELONG( iEndY, iEndX );
						}
					}
				}
			}
		}
	}
	if( bPath == TRUE ) 
	{
		// Set Wander Time
		m_dwEnd[MST_MOVE_ACT]		= MonsterTickCount;// + g_SpeedArray[m_wSpeed][m_iMoveDir];
		// Set Wander Status And Base Data
		m_MoveObj.dwCode_status = ( GetSelfCode() << 16 ) | m_wSpeed;
		m_MoveObj.dwStartX_Y		= ( m_iX << 16 ) | ( m_iY & 0x0000FFFF );

		m_iMidX									= HIWORD( m_MoveObj.dwEndX_Y[0] );		
		m_iMidY									= LOWORD( m_MoveObj.dwEndX_Y[0] );
		SetPathEnd( HIWORD( m_MoveObj.dwEndX_Y[j-1] ), LOWORD( m_MoveObj.dwEndX_Y[j-1] ) );

		m_iPathCount						= j;
		m_iStepCount						= 0;
    m_wWanderPath           = 0;
		m_iAddiStatus					  = ATTACK_STATUS_MONSTER_WANDER;
		m_iNpcStatus						= STATUS_MONSTER_WANDER;

		for( ; j < MAX_WANDER_PATH_COUNT_SHORT + 1; j++ )
		{
			m_MoveObj.dwEndX_Y[j]	= 0xFFFFFFFF;
		}
		SetMonsterDir( m_iMidX, m_iMidY );

		// Send Wander Message
		memcpy( MsgMoveObj.Msgs[0].Data, &m_MoveObj, sizeof( SNMMoveObjs ) );

		m_pInMap->SendMsgNearPosition( MsgMoveObj, m_iX, m_iY );

		return TRUE;
	}
	else
	{
		// 多次没找到路径，是不是卡在障碍中，引导Monster走出来
    //if( m_wWanderPath++ > 100 )
    //{
      //int       iMonsterRandX = 0, iMonsterRandY = 0;
      //for( int z = 0; z < 10; z++ )
      //{
      //  iMonsterRandX = gf_GetRandom( m_pInMap->GetClientSizeX() );
      //  iMonsterRandY = gf_GetRandom( m_pInMap->GetClientSizeY() );
      //  if( !( m_pInMap->GetTileFlag( iMonsterRandX, iMonsterRandY ) & TILE_ALLOCCULDE ) )
      //  {
      //    m_iInitX      = iMonsterRandX;
      //    m_iInitY      = iMonsterRandY;
    //      m_iNpcStatus  = STATUS_MONSTER_GOHOME;
    //      m_wWanderPath = 0;
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG
    //      FuncName("CMonster::SetMonsterDir");
    //      sprintf( g_szMonsterLog, "Monster[%d][%d][%d] Go Home Because Wander2, Pos(%d,%d)(%d,%d)", GetCode(), m_pBase->GetId(), GetMapId(), m_iX, m_iY, m_iInitX, m_iInitY );
    //      AddMemoMsg( g_szMonsterLog );
#endif
#endif
      //    break;
      //  }
      //}
    //}
	}
	return FALSE;	
}
//============================================================================================
//
//#ifdef _DEBUG
//#define _DEBUG_SHOW_RANDOM_REVIVE_MONSTER_INFO_
//#endif
//
void CMonster::RandomMapRevive()
{
  static SNMNpcInfo			*pTheNpcInfo = NULL;
  CGameMap              *pNewMap = g_pGs->GetRandomMonsterReviveMap( GetBaseId() );
  WORD                  wNewCode = 0;
  //
  if( pNewMap && ( wNewCode = pNewMap->GetNewMonsterCode() ) )
  {
    SAdjustPoint        *pAdjustPoint = pNewMap->GetAdjustPoint();
    //
    //SetPathEnd( m_iX, m_iY );
    m_pInMap->DelMonster( GetSelfCode() );
    // Add The Monster Into New Map
    SetCode( wNewCode );
    m_wSelfCode = wNewCode;
    //
    pNewMap->AddMonster( this, pAdjustPoint->wAdjustX, pAdjustPoint->wAdjustY );
	  SetGameMap( pNewMap );
	  InitPos( pAdjustPoint->wAdjustX, pAdjustPoint->wAdjustY, pAdjustPoint->wAdjustX, pAdjustPoint->wAdjustY );
	  SetWanderRange();
    //
#ifdef _DEBUG_SHOW_RANDOM_REVIVE_MONSTER_INFO_
    int       iCltX = pAdjustPoint->wAdjustX, iCltY = pAdjustPoint->wAdjustY;
    //
    pNewMap->ConvertSrv2Cli( &iCltX, &iCltY );
    FuncName("CMonster::Revive");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "<<<=== Monster Random Revive, Code=%d, Id=%d, Map=%d, Pos=(%d,%d)(%d,%d)", GetCode(), m_pBase->GetId(), GetMapId(), m_iX, m_iY, iCltX, iCltY );
    g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( g_szMonsterLog );
#endif
    //
    SMsgData            MstNewMsg;
    MstNewMsg.Init();
    MstNewMsg.dwAID         = A_TALKTOALL;
    MstNewMsg.dwMsgLen	    = 1;
    //
    _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s 竒発瓜[%d], 竚(%d,%d), 叫產尿發...", GetName(), GetMapId(), iCltX, iCltY );
    g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
    MstNewMsg.Msgs[0].Size  = strlen( g_szMonsterLog );
    memcpy( MstNewMsg.Msgs[0].Data, g_szMonsterLog, MstNewMsg.Msgs[0].Size );
    MstNewMsg.Msgs[0].Data[MstNewMsg.Msgs[0].Size] = '\0';
    //
    g_pGs->SendTheMsgToAll( MstNewMsg );
    //
#endif
    // About Notify Times
#ifdef _BOSS_CALL_UP_HAND_
    m_wCallUpNum    = 0;
    m_pFather       = NULL;
#endif
	  m_wNotifyTimes  = 0;
	  m_iMoveDir      = gf_GetRandom( 8 );
	  ClearStatus();
	  m_pLastHit			= NULL;
	  m_pFirstHit			= NULL;
    //
    m_iHp   = m_iMaxHp;
	  m_iMidX = m_iX;
	  m_iMidY = m_iY;
	  SetPathEnd( m_iX, m_iY );

	  GoInTile();
    // Send Revive Message
    MsgPlyRevive.Msgs[0].Size = sizeof( SNMNpcInfo );
    pTheNpcInfo = (SNMNpcInfo*)(MsgPlyRevive.Msgs[0].Data);
	  Get_SNMNpcInfo( pTheNpcInfo );
	  m_pInMap->SendMsgNearPosition_Close( MsgPlyRevive, m_iX, m_iY );
    //
	  g_dwMonsterTotal++;
  }
  // 本地图复活
  else
  {
    m_pBase->m_iNpcProperty &= ~NPC_ATTRI_RANDOM_MAP;
    Revive();
  }
}
//============================================================================================
//
//
void CMonster::Revive()
{
	static SNMNpcInfo			*pTheNpcInfo;
	static int						iMonsterRandX, iMonsterRandY;

  if( m_pBase->m_iNpcProperty & NPC_ATTRI_NEVER_REVIVE || m_wReviveType == REVIVE_TYPE_DELETE )
  {
		GoOutTile();
    g_ListUnreviveMonster.push_back( this );
    m_iNpcStatus = STATUS_MONSTER_DISABLE;
    m_dwEnd[MST_DEAD_ACT] = 0xCFFFFFFF;
    return;
  }
  // Random Map Revive Monster
  if( IsRandomMapRevive() && (!IsCityGate()) && GetTeam() == NULL && GetMercenaryType() == 0 )
  {
    if( m_pInMap->AddRandomMonster( this ) )
    {
      m_iNpcStatus = STATUS_MONSTER_DISABLE;
      m_dwEnd[MST_DEAD_ACT] = 0xCFFFFFFF;
      return;
    }
  }
	// Don't care direction, the value is can not change when monster dead
	m_iHp = m_pBase->GetHp();
	if( m_pTeam )
	{
    //m_pTeam->TeamerRevive();
    if( m_pBase->GetSpeed() )
    {
      // 会否随机出生
      if( m_pBase->m_iNpcProperty & NPC_ATTRI_RANDOM_REVIVE )
      {
		    m_pTeam->GetNowBirthPlace( m_iInitX, m_iInitY, this );
		    if( m_pBase->GetSpeed() )
		    {
			    for( int z = 0; z < 10; z++ )
			    {
				    iMonsterRandX = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
				    iMonsterRandY = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
				    if( !( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )	break;
				    else if( z == 9 )
				    {
					    iMonsterRandX = -1 + gf_GetRandom( 2 );
					    iMonsterRandY = -1 + gf_GetRandom( 2 );
				    }
			    }
		    }
		    else
		    {
			    iMonsterRandX = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
			    iMonsterRandY = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
          if( ( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )
          {
            iMonsterRandX = iMonsterRandY = 0;
          }
		    }
      }
      else
      {
        for( int z = 0; z < 10; z++ )
        {
          iMonsterRandX = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
          iMonsterRandY = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
          if( !( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )	break;
          else if( z == 9 )
          {
			      iMonsterRandX = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
			      iMonsterRandY = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
            if( ( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )
            {
              iMonsterRandX = iMonsterRandY = 0;
            }
          }
        }
      }
    }
    else
    {
      iMonsterRandX = 0;
      iMonsterRandY = 0;
    }
		m_iX = m_iInitX + iMonsterRandX;
		m_iY = m_iInitY + iMonsterRandY;
    // Notify The Teams -- "I Can Help You Again When Player Attack You"
	}
	else
	{
		if( m_pBase->GetSpeed() )
		{
			for( int z = 0; z < 10; z++ )
			{
        // 会否随机出生
        if( m_pBase->m_iNpcProperty & NPC_ATTRI_RANDOM_REVIVE )
        {
				  iMonsterRandX = gf_GetRandom( m_pInMap->GetSizeX() );
				  iMonsterRandY = gf_GetRandom( m_pInMap->GetSizeY() );
				  if( !( m_pInMap->GetTileFlag( iMonsterRandX, iMonsterRandY ) & TILE_ALLOCCULDE ) )
          {
            m_iInitX = iMonsterRandX;
            m_iInitY = iMonsterRandY;
            iMonsterRandX = 0;
            iMonsterRandY = 0;
            break;
          }
				  else if( z == 9 )
				  {
			      iMonsterRandX = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
			      iMonsterRandY = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
            if( ( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )
            {
              iMonsterRandX = iMonsterRandY = 0;
            }
				  }
        }
        else
        {
          iMonsterRandX = gf_GetRandom( 30 ) - gf_GetRandom( 30 );
          iMonsterRandY = gf_GetRandom( 30 ) - gf_GetRandom( 30 );
				  if( !( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )	break;
				  else if( z == 9 )
				  {
			      iMonsterRandX = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
			      iMonsterRandY = gf_GetRandom( 8 ) - gf_GetRandom( 8 );
            if( ( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )
            {
              iMonsterRandX = iMonsterRandY = 0;
            }
				  }
        }
			}
		}
		else
		{
      iMonsterRandX = 0;//gf_GetRandom( 8 ) - gf_GetRandom( 8 );
      iMonsterRandY = 0;//gf_GetRandom( 8 ) - gf_GetRandom( 8 );
      //if( ( m_pInMap->GetTileFlag( m_iInitX + iMonsterRandX, m_iInitY + iMonsterRandY ) & TILE_ALLOCCULDE ) )
      //{
      //  iMonsterRandX = iMonsterRandY = 0;
      //}
		}
    m_iX = m_iInitX + iMonsterRandX;
    m_iY = m_iInitY + iMonsterRandY;
	}
	// About Notify Times
#ifdef _BOSS_CALL_UP_HAND_
  m_wCallUpNum    = 0;
  m_pFather       = NULL;
#endif
	m_wNotifyTimes  = 0;
  SetWanderRange();
	m_iMoveDir      = gf_GetRandom( 8 );
	ClearStatus();
	m_pLastHit			= NULL;
	m_pFirstHit			= NULL;
  //
  if( IsCityGate() )    m_pInMap->CloseCityGate( GetBaseId() );
  //
  m_iHp   = m_iMaxHp;
	m_iMidX = m_iX;
	m_iMidY = m_iY;
	SetPathEnd( m_iX, m_iY );

	GoInTile();
  // Send Revive Message
  MsgPlyRevive.Msgs[0].Size = sizeof( SNMNpcInfo );
  pTheNpcInfo = (SNMNpcInfo*)(MsgPlyRevive.Msgs[0].Data);
	Get_SNMNpcInfo( pTheNpcInfo );
	m_pInMap->SendMsgNearPosition_Close( MsgPlyRevive, m_iX, m_iY );
  //
	//m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );
	m_pInMap->OneMonsterRevive();
	g_dwMonsterTotal++;
//#ifndef _DEBUG_NO_ANY_LOG_
//#ifdef _DEBUG
//  FuncName("CMonster::Revive");
//  sprintf( g_szMonsterLog, "<<<=== Monster Revive, Code=%d, Id=%d, Map=%d, Pos=(%d,%d)", GetCode(), m_pBase->GetId(), GetMapId(), m_iX, m_iY );
//  AddMemoMsg( g_szMonsterLog );
//#endif
//#endif
}
//=====================================================================================
//
//Note:
#ifdef _NEW_CITY_WAR_2005_
void CMonster::ReGoInCityWar()
{
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_WILDCAT_
  FuncName("CMonster::ReGoInCityWar");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
  _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "***** Monster[%s] ReGoInCityWar  *****", GetName() );
  g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
  // Send A_CLEARCODE To Player
  GoOutTile();
  *(DWORD*)MsgClearCode.Msgs[0].Data = GetSelfCode();
  m_pInMap->SendMsgNearPosition( MsgClearCode, m_iX, m_iY );

  m_iHp = m_pBase->GetHp();
  m_iX = m_iInitX;
  m_iY = m_iInitY;
  SetPathEnd( m_iX, m_iY );

  m_iAddiStatus	= ATTACK_STATUS_MONSTER_SEARCH;
  m_iNpcStatus		= STATUS_MONSTER_STAND;
  m_dwTargetCode  = 0;
  m_pLastTarget		= NULL;
  m_pLastHit			= NULL;
  m_pFirstHit			= NULL;
  m_dwEnd[MST_SEARCH_ACT]   = MonsterTickCount + MONSTER_SEARCH_TIME + 1000;
  GoInTile();  
}
#endif
//============================================================================================
//
//
void CMonster::ReinitAll()
{
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_WILDCAT_
  FuncName("CMonster::ReinitAll");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
  _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "***** Monster[%s] Reinitialize All Data *****", GetName() );
  g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif

	// Send A_CLEARCODE To Player
	GoOutTile();
  *(DWORD*)MsgClearCode.Msgs[0].Data = GetSelfCode();
  m_pInMap->SendMsgNearPosition( MsgClearCode, m_iX, m_iY );

	m_iHp = m_pBase->GetHp();
	m_iX = m_iInitX + gf_GetRandom( m_pBase->m_wSightRange );
	m_iY = m_iInitY + gf_GetRandom( m_pBase->m_wSightRange );
  if( m_pInMap->GetTileFlag( m_iX, m_iY ) & TILE_ALLOCCULDE )
  {
    m_iX = m_iInitX;
    m_iY = m_iInitY;
  }
	SetPathEnd( m_iX, m_iY );

	m_iAddiStatus	= ATTACK_STATUS_MONSTER_SEARCH;
	m_iNpcStatus		= STATUS_MONSTER_STAND;
	m_dwTargetCode  = 0;
	m_pLastTarget		= NULL;
	m_pLastHit			= NULL;
	m_pFirstHit			= NULL;
	m_dwEnd[MST_SEARCH_ACT]   = MonsterTickCount + MONSTER_SEARCH_TIME + 1000;
	GoInTile();
	//m_pInMap->InsertLifeToTile( m_iX, m_iY, (CLife*)this );
}
//============================================================================================
//
//
inline void CMonster::GoToDied()
{
	// Send Die Message When The Monster Die
	static list<CHiter>::iterator		Iter_Killer;
	static list<CPlayer*>::iterator Iter_Teamer;
	       SNMMonsterDied						*pMonsterDied = (LPSNMMonsterDied)(MsgDead.Msgs[0].Data);
	       LPSNMCodeExp							pKillerCode   = (LPSNMCodeExp)(MsgDead.Msgs[1].Data);
	static DWORD										dwTeamExpPer80;
	static WORD											wKillCodeCount = 0, wThisExp;
	static CTeam										*pHitTeam = NULL;

	// Send Monster Dead Message To All Client
	wKillCodeCount = 0;
  int iNewPro = 0;
	pMonsterDied->wBonuTime  = ( m_pBase->m_dwBoneTime / 1000 );
	MsgDead.dwMsgLen         = 2;
	MsgDead.Msgs[0].Size     = sizeof( SNMMonsterDied );

  if( m_iAddiStatus == ATTACK_STATUS_MONSTER_GOTO_DEAD )
  {
#ifdef _DEBUG_MONSTER_ALWAYS_REVIVE_
    goto _ALWAYS_REVIVE_;
#endif
	  pMonsterDied->wBigKiller = m_BadestHiter.m_pHiter->GetSelfCode();
    //add for 4.0 吸魂血珀
#ifdef _SUCK_SOUL_
    CItem *SuckSoul = NULL;
    CSrvBaseSkill* pSkill = NULL;
    
    SuckSoul = m_BadestHiter.m_pHiter->GetEquipment( ITEM_EQUIPWHERE_HAND );
    if ( SuckSoul != NULL ) 
    {
      for ( int iLoop=0; iLoop < MAX_ITEM_SLOT; iLoop++) 
      {
        if ( (pSkill = SuckSoul->GetSklTessera(iLoop)) != NULL
           &&(pSkill->GetType()) == SKILL_TYPE_ITEM_SUCKSOUL ) 
        {
          iNewPro += pSkill->GetProbability();
        }
      }
    }
#endif
    if( gf_GetRandom( 10000 ) <= (m_pBase->m_wSoulProb + iNewPro) )
      pMonsterDied->wSoul      = m_pBase->m_wSoul;
    else
      pMonsterDied->wSoul      = 0;
    m_BadestHiter.m_pHiter->AddSoul( pMonsterDied->wSoul );
  }
  else
  {
#ifdef _DEBUG_MONSTER_ALWAYS_REVIVE_
    goto _ALWAYS_REVIVE_;
#endif
	  pMonsterDied->wBigKiller = m_pLastHit->GetSelfCode();
	  pMonsterDied->wSoul      = 0;
  }

	// Check Drop Item List When The Monster Was Dead
	//if( DropItem() )
	//if( !m_pListHiter.empty() )
  if( m_iAddiStatus == ATTACK_STATUS_MONSTER_GOTO_DEAD )
	{
    if( m_pFirstHit->GetInMap() != NULL &&
        m_pFirstHit->GetMapId() == m_dwMapId &&
        m_pFirstHit->GetLevel() < m_pBase->m_wExpLtLv )
    {
		  // 分配经验值
		  pKillerCode->wCode = m_pFirstHit->GetSelfCode();
		  if( m_pFirstHit->GetLevel() <= m_pBase->m_wExpLtLvHalf )
		  {
			  pKillerCode->dwExp = m_dwExpPer20;
		  }
		  else
		  {
			  pKillerCode->dwExp = m_dwExpPer20 >> 1;
		  }
#ifdef VERSION_40_HOUSE_FUNCTION
      m_pFirstHit->AddExpWhenStatusEXP(pKillerCode->dwExp);
#endif
#ifdef MULT_EXP
      if( g_bMultExp )
      {
        pKillerCode->dwExp = pKillerCode->dwExp * g_iMultiple1 + pKillerCode->dwExp * g_iMultiple2 / 100; 
      }
#endif
#ifdef PREVENT_ENTHRALL_SYSTEM
       pKillerCode->dwExp =  pKillerCode->dwExp * m_pFirstHit->GetEnthrallCoste() / 100;
#endif
#ifdef _DEBUG_INFO_MONSTER_
      if( g_bShowMonsterExpLog )
      {
        if( g_wShowMonsterExpCode == GetCode() && g_wShowMonsterExpMap == GetMapId() )
        {
#ifndef _DEBUG_NO_ANY_LOG_
          FuncName("CMonster::GoToDied");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
		      _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "=====%s Get Exp1=%d From %s (%d,%d), HiterList=%d=====", m_pFirstHit->GetPlayerAccount(), pKillerCode->dwExp, GetName(), m_dwExpPer20, m_dwExpPer80, m_pListHiter.size() );
          g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
		      AddMemoMsg( g_szMonsterLog );
#endif
#endif
        }
      }
#endif
      m_pFirstHit->AddExpWhenAttack( this, pKillerCode->dwExp );
      pKillerCode++;
      wKillCodeCount++;
    }
    //
		for( Iter_Killer = m_pListHiter.begin(); Iter_Killer != m_pListHiter.end(); Iter_Killer++ )
		{
			if( GetDistance( (*Iter_Killer).m_pHiter ) < MAX_SHARE_EXP_DISTANCE &&
          (*Iter_Killer).m_pHiter->GetLevel() < m_pBase->m_wExpLtLv && m_wHitTotal )
			{
				pKillerCode->wCode = (*Iter_Killer).m_pHiter->GetSelfCode();
				wThisExp = (*Iter_Killer).m_wHitNum * m_dwExpPer80 / m_wHitTotal;
				if( (*Iter_Killer).m_pHiter->GetLevel() <= m_pBase->m_wExpLtLvHalf )
				{
					pKillerCode->dwExp = wThisExp;
				}
				else
				{
					pKillerCode->dwExp = wThisExp >> 1;
				}
#ifdef VERSION_40_HOUSE_FUNCTION
        (*Iter_Killer).m_pHiter->AddExpWhenStatusEXP(pKillerCode->dwExp);
#endif
#ifdef MULT_EXP
        if( g_bMultExp )
        {
          pKillerCode->dwExp = pKillerCode->dwExp * g_iMultiple1 + pKillerCode->dwExp * g_iMultiple2 / 100; 
        }
#endif
#ifdef PREVENT_ENTHRALL_SYSTEM
        pKillerCode->dwExp =  pKillerCode->dwExp * m_pFirstHit->GetEnthrallCoste() / 100;
#endif
				(*Iter_Killer).m_pHiter->AddExpWhenAttack( this, pKillerCode->dwExp );
#ifdef _DEBUG_INFO_MONSTER_
        if( g_bShowMonsterExpLog )
        {
          if( g_wShowMonsterExpCode == GetCode() && g_wShowMonsterExpMap == GetMapId() )
          {
#ifndef _DEBUG_NO_ANY_LOG_
            FuncName("CMonster::GoToDied");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
		        _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "=====%s Get Exp2=%d From %s (%d,%d), HiterList=%d=====", (*Iter_Killer).m_pHiter->GetPlayerAccount(), pKillerCode->dwExp, GetName(), m_dwExpPer20, m_dwExpPer80, m_pListHiter.size() );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
		        AddMemoMsg( g_szMonsterLog );
#endif
#endif
          }
        }
#endif
				pKillerCode++;
				wKillCodeCount++;
			}
		}
    //
    int       iNearbyNum      = 0;
    CPlayer   **pNearbyTeamer = NULL;
		for( int i = 0; i < MAX_HIT_TEAM_COUNT; i++ )
		{
			if( ( pHitTeam = m_TeamHiter[i].ComputeTeamer() ) != NULL )
			{
        if( iNearbyNum = pHitTeam->ComputeTeamerNearby( this, m_TeamHiter[i].m_wTeamLevel ) )
				{
          if( m_wHitTotal == 0 )      m_wHitTotal = 1;
					dwTeamExpPer80 = ( ( m_TeamHiter[i].m_wHitNum * m_dwExpPer80 / m_wHitTotal ) * ( iNearbyNum * g_iTeamExpBonu + 100 ) ) / 100;
				}
				else
				{
          continue;
				}
        pNearbyTeamer = pHitTeam->GetMembers();
        //
				for( int n = 0; n < MAX_TEAM_MEMBER; n++ )
				{
          if( GetDistance( pNearbyTeamer[n] ) < MAX_SHARE_EXP_DISTANCE &&
              pNearbyTeamer[n]->GetLevel() < m_pBase->m_wExpLtLv )
          {
            if( m_TeamHiter[i].m_wTeamLevel == 0 )    m_TeamHiter[i].m_wTeamLevel = 1;
						wThisExp = ( pNearbyTeamer[n]->GetLevel() * dwTeamExpPer80 / m_TeamHiter[i].m_wTeamLevel );
						pKillerCode->wCode = pNearbyTeamer[n]->GetSelfCode();
						if( pNearbyTeamer[n]->GetLevel() <= m_pBase->m_wExpLtLvHalf )
						{
							pKillerCode->dwExp = wThisExp;
						}
						else
						{
              pKillerCode->dwExp = wThisExp >> 1;
            }
            
#ifdef _RAPID_UPGRADE_LEVEL_
            if(m_TeamHiter[i].m_wHitNum <= 5)
            {
              pKillerCode->dwExp = pKillerCode->dwExp * 120 / 100;
            }
            else if(m_TeamHiter[i].m_wHitNum > 5)
            {
              pKillerCode->dwExp = pKillerCode->dwExp * 150 / 100;
            }
#endif
#ifdef VERSION_40_HOUSE_FUNCTION
            pNearbyTeamer[n]->AddExpWhenStatusEXP(pKillerCode->dwExp);
#endif
#ifdef MULT_EXP //VERSION_40_HOUSE_FUNCTION和MULT_EXP不能同时开放
            if( g_bMultExp )
            {
              pKillerCode->dwExp = pKillerCode->dwExp * g_iMultiple1 + pKillerCode->dwExp * g_iMultiple2 / 100; 
            }
#endif
#ifdef PREVENT_ENTHRALL_SYSTEM
            pKillerCode->dwExp =  pKillerCode->dwExp * m_pFirstHit->GetEnthrallCoste() / 100;
#endif
#ifdef _DEBUG_INFO_MONSTER_
            if( g_bShowMonsterExpLog )
            {
              if( g_wShowMonsterExpCode == GetCode() && g_wShowMonsterExpMap == GetMapId() )
              {
#ifndef _DEBUG_NO_ANY_LOG_
                FuncName("CMonster::GoToDied");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
		            _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "=====%s Get Exp3=%d From %s (%d,%d), HiterList=%d=====", pNearbyTeamer[n]->GetPlayerAccount(), pKillerCode->dwExp, GetName(), m_dwExpPer20, m_dwExpPer80, m_pListHiter.size() );
                g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
		            AddMemoMsg( g_szMonsterLog );
#endif
#endif
              }
            }
#endif
						pNearbyTeamer[n]->AddExpWhenAttack( this, pKillerCode->dwExp );
						pKillerCode++;
						wKillCodeCount++;
          }
				}
			}
		}
	}
#ifdef _DEBUG_MONSTER_ALWAYS_REVIVE_
_ALWAYS_REVIVE_:
#endif

	// Check The Event When I Died
	DeadEvent();
  //
	if( !(m_pBase->m_iNpcProperty & NPC_ATTRI_UNDYING) )
	{
		// Delete The Monster From The Map
		GoOutTile();
		m_pInMap->OneMonsterDied();
		// Clear Data
		m_iNpcStatus	= STATUS_MONSTER_DEAD;
		m_iNextStatus = STATUS_MONSTER_STAND;
		m_iAddiStatus	= ATTACK_STATUS_MONSTER_DROPITEM;
    m_dwEnd[MST_DEAD_ACT] = 0;
		// Clear Special Effect
		if( m_qwSpecialStatus )		ClearSpecialStatus( 0xFFFFFFFFFFFFFFFF );
		g_dwMonsterTotal--;
		pMonsterDied->wCode = GetSelfCode();
	}
	else
	{
		m_iNpcStatus  = STATUS_MONSTER_STAND;
		m_iNextStatus = STATUS_MONSTER_STAND;
		m_iAddiStatus	= ATTACK_STATUS_MONSTER_DROPITEM;
    m_dwEnd[MST_DEAD_ACT] = 0;
		pMonsterDied->wCode   = 0;
		m_iHp = m_iMaxHp;
    // Clear Attack Data, Exp Data
		ClearHitData();
	}
	// Clear Data
	m_iActiveRange    = 0;
	m_pUsingSkill     = NULL;
	m_pLastTarget		  = NULL;
	m_pFirstHit			  = NULL;
	m_pLastHit			  = NULL;
	m_dwTargetCode    = 0;
	pHitTeam					= NULL;
	// Send To Nearly Players
  if( wKillCodeCount )	MsgDead.Msgs[1].Size = sizeof( SNMCodeExp ) * wKillCodeCount;
  else                  MsgDead.Msgs[1].Size = 1;
	m_pInMap->SendMsgNearPosition( MsgDead, m_iX, m_iY );
  if( IsCityGate() )    m_pInMap->CityGateIsCrash( GetBaseId() );
//#ifndef _DEBUG_NO_ANY_LOG_
//#ifdef _DEBUG
//  FuncName("CMonster::GoToDied");
//  sprintf( g_szMonsterLog, "===>>> Monster Died, Code=%d, Id=%d, Map=%d, Pos=(%d,%d)", GetCode(), m_pBase->GetId(), GetMapId(), m_iX, m_iY );
//  AddMemoMsg( g_szMonsterLog );
//#endif
//#endif

}
//============================================================================================
//
//
void CMonster::DiedForFerry()
{
	m_iActiveRange    = 0;
	m_pUsingSkill     = NULL;
	m_pLastTarget		  = NULL;
	m_pFirstHit			  = NULL;
	m_pLastHit			  = NULL;
	m_dwTargetCode    = 0;
	//
	ClearHitData();
	GoOutTile();
	m_pInMap->OneMonsterDied();
	// Clear Data
	m_iNpcStatus	= STATUS_MONSTER_DEAD;
	m_iNextStatus = STATUS_MONSTER_STAND;
	m_iAddiStatus	= ATTACK_STATUS_MONSTER_DROPITEM;
	//
	m_wReviveType = REVIVE_TYPE_DELETE;
	m_dwEnd[MST_DEAD_ACT] = MonsterTickCount + 5000;
	// Clear Special Effect
	g_dwMonsterTotal--;
}
//============================================================================================
//
#ifdef _DEBUG
#define _DEBUG_SHOW_DEAD_MONSTER_DISTANCE_WITH_KILLER_
#endif
//
void CMonster::Died()
{
	// Monster Dead Ready...
	static list<CHiter>::iterator		Iter_Killer;
	static WORD											wTeamCount =  0, wTeamNum   = 0;
	static int											iTeamPos   = -1;
  static BOOL                     bEraseIter = FALSE;

	// Compute The Bigest Killer And Team Hiter
	m_BadestHiter.m_pHiter    = NULL;
	m_BadestHiter.m_wHitNum   = 0;
	m_BadestHiter.m_dwDamage  = 0;
	m_BadestHiter1.m_pHiter   = NULL;
	m_BadestHiter1.m_wHitNum  = 0;
	m_BadestHiter1.m_dwDamage = 0;
  //
	for( wTeamCount = 0; wTeamCount < MAX_HIT_TEAM_COUNT; wTeamCount++ )
	{
		m_TeamHiter[wTeamCount].ClearAll();
	}
	wTeamCount     = 0;
  //
  if( !m_pListHiter.empty() )
  {
    m_pFirstHit = (*m_pListHiter.begin()).m_pHiter;
    //
#ifdef _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
    if( !m_pListHiter.empty() )
    {
      int           iTotalDamage = 0;
      for( list<CHiter>::iterator Iter_Debug = m_pListHiter.begin(); Iter_Debug != m_pListHiter.end(); Iter_Debug++ )
      {
        iTotalDamage += (*Iter_Debug).m_dwDamage;
      }
      for( Iter_Debug = m_pListHiter.begin(); Iter_Debug != m_pListHiter.end(); Iter_Debug++ )
      {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
        FuncName("CMonster::Died");
        _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "==>> Cal Hiter[%s] Damage=(%d,%d,%d,%d), HitCount=(%d,%d)",
          (*Iter_Debug).m_pHiter->GetAccount(), (*Iter_Debug).m_dwDamage, iTotalDamage, m_iMaxHp, (*Iter_Debug).m_dwDamage*100/iTotalDamage,
          (*Iter_Debug).m_wHitNum, g_iDamageCount );
        g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg( g_szMonsterLog );
#endif
      }
    }
#endif
    //
	  for( Iter_Killer = m_pListHiter.begin(); Iter_Killer != m_pListHiter.end(); )
	  {
		  iTeamPos   = -1;
      bEraseIter = FALSE;
      //
      if( GetDistance( (*Iter_Killer).m_pHiter ) > 20 )
      {
#ifdef _DEBUG_SHOW_DEAD_MONSTER_DISTANCE_WITH_KILLER_
        if( (*Iter_Killer).m_pHiter != NULL )
        {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
          FuncName("CMonster::Died");
          _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "==>>>%s: Hiter[%s] Distance Overflow,"
                   "Map=(%d,%d), Damage=%d, HitCount=%d",
                   GetName(), (*Iter_Killer).m_pHiter->GetName(),
                   GetMapId(), (*Iter_Killer).m_pHiter->GetMapId(),
                   (*Iter_Killer).m_dwDamage, (*Iter_Killer).m_wHitNum );
          g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( g_szMonsterLog );
#endif
        }
        else
        {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
          FuncName("CMonster::Died");
          _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "==>>>%s: Find NULL Hiter In Map[%d], Damage=%d, HitCount=%d",
                   GetName(), GetMapId(),
                   (*Iter_Killer).m_dwDamage, (*Iter_Killer).m_wHitNum );
          g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( g_szMonsterLog );
#endif
        }
#endif
        //
        Iter_Killer = m_pListHiter.erase( Iter_Killer );
        continue;
      }
      // 如若是不死的怪物，不必清除玩家状态
      if( !(m_pBase->m_iNpcProperty & NPC_ATTRI_UNDYING) &&
          (*Iter_Killer).m_pHiter->GetStatus() != STATUS_PLAYER_DEAD )
      {
        (*Iter_Killer).m_pHiter->ClearAllTargetAction();
      }
		  // 计算攻击伤害最多的玩家
      if( (*Iter_Killer).m_dwDamage > m_BadestHiter.m_dwDamage )
		  {
			  m_BadestHiter.m_pHiter  = (*Iter_Killer).m_pHiter;
			  m_BadestHiter.m_dwDamage = (*Iter_Killer).m_dwDamage;
		  }
		  else if( (*Iter_Killer).m_dwDamage == m_BadestHiter.m_dwDamage )
		  {
			  m_BadestHiter1.m_pHiter  = (*Iter_Killer).m_pHiter;
			  m_BadestHiter1.m_dwDamage = (*Iter_Killer).m_dwDamage;
		  }
      // 计算组队玩家
		  if( (*Iter_Killer).m_pHiter->GetTeamId() )
		  {
			  for( wTeamNum = 0; wTeamNum < MAX_HIT_TEAM_COUNT; wTeamNum++ )
			  {
				  if( m_TeamHiter[wTeamNum].m_wTeamId == (*Iter_Killer).m_pHiter->GetTeamId() )
				  {
					  m_TeamHiter[wTeamNum].AddHiter( &(*Iter_Killer) );
					  Iter_Killer = m_pListHiter.erase( Iter_Killer );
            bEraseIter  = TRUE;
            break;
				  }
          else if( m_TeamHiter[wTeamNum].m_wTeamId == 0 && iTeamPos < 0 )
          {
            iTeamPos = wTeamNum;
          }
			  }
        //
			  if( iTeamPos >= 0 && iTeamPos < MAX_HIT_TEAM_COUNT && bEraseIter == FALSE )
			  {
				  m_TeamHiter[iTeamPos].AddHiter( &(*Iter_Killer) );
				  Iter_Killer = m_pListHiter.erase( Iter_Killer );
          bEraseIter  = TRUE;
			  }
        //
        if( bEraseIter == FALSE )     Iter_Killer++;
		  }
      else
      {
        Iter_Killer++;
      }
	  }
  }
#ifdef _DEBUG
  else
  {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CMonster::Died");
    AddMemoErrMsg( "==>> Hiter List Is Empty" );
#endif
  }
#endif
  //
	if( m_BadestHiter.m_pHiter != NULL )
	{
		m_iAddiStatus = ATTACK_STATUS_MONSTER_GOTO_DEAD;
	}
	else
	{
#ifdef _DEBUG
    FuncName("CMonster::Died");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    AddMemoMsg( "===>>> Monster Dead, But Don't Drop Item..." );
#endif
#endif
		m_iAddiStatus = ATTACK_STATUS_MONSTER_DEAD;
	}
	m_iNpcStatus  = STATUS_MONSTER_GOTO_DEAD;
	SetDeadPathEnd( m_iX, m_iY );
  m_pTrapSkill  = NULL;
}
//============================================================================================
//
//
inline void CMonster::DeadEvent()
{
	if( m_pDeadEvent )
	{
		m_pDeadEvent->DoMonsterDeadEvent( this );
	}
}
//============================================================================================
//
//
inline BOOL CMonster::DropItem()
{
	static CSrvBaseItem							*pDropBaseItem = NULL;
	       SNMAddGroundItem					*pAddGroundItem = (SNMAddGroundItem*)(MsgDropItem.Msgs[0].Data);
	static int											iListNo, iCurrArray, iDropItemCount;
	static CItem										*pDropCItem;
	static CGroundItem							*pDropGItem;
	static CSrvDropItem							*pItemFix;
	static WORD											wRandDropX, wRandDropY, wRandNum;
	static list<CHiter>::iterator		Iter_Killer;
	static SPlayerItemExC						*pPlayerItem;
	static LPSNMMonsterDrop					pMonsterDrop;
	static SMsgData									*pNewDropMsg, *pNewRareMsg;
         DWORD                    dwRandMoney = 0;
         WORD                     wAddItemPos = 0;
	int															iPackage = 0, bPackItem = TRUE;
	list<CGroundItem*>::iterator		Iter_GItem;
  //
  static WORD                     g_BossItemNum;
  WORD                            *pBossDropIdList = (WORD*)(MsgBossDrop.Msgs[0].Data);
//#ifdef FUNCTION_LUCKY_ITEM
  static DWORD                    dwRate;
//#endif
#ifdef FUNCTION_TALKTOALL_DROPITEM
  string                           szItem = "";
#endif

	if( m_BadestHiter.m_pHiter == NULL )
	{
		return FALSE;
	}
  g_BossItemNum  = 0;
	iDropItemCount = 0;
  pNewDropMsg    = NULL;
	if( GetDistance( m_BadestHiter.m_pHiter ) > 8 ||
      m_BadestHiter.m_pHiter->GetMapId() != m_dwMapId ||
      m_BadestHiter.m_pHiter == m_BadestHiter1.m_pHiter )
	{
		bPackItem = FALSE;
	}
	for( iListNo = 0; iListNo < 5; iListNo++ )
	{
		if( m_pBase->m_wDropRate[iListNo] == 0 )  break;
		// From Drop Begin Id To Drop End Id
		for( iCurrArray = m_pBase->m_wDropBegin[iListNo]; iCurrArray <= m_pBase->m_wDropEnd[iListNo]; iCurrArray++ )
		{
			// Get Current Drop Item Fix Table
			if( NULL != ( pItemFix = g_pDropItem->GetDropItem( iCurrArray ) ) )
			{
#ifdef VERSION40_UNIQUERING
        if( g_pBase->GetBaseItem(pItemFix->GetId())->GetSpecialAttr() & ITEMX_PROPERTY_UNIQUEITEM )
        {
          if( g_pRingManager->m_bDrop )
            continue;
        }
#endif
#ifdef PREVENT_ENTHRALL_SYSTEM
        dwRate = m_pBase->m_wDropRate[iListNo] * m_BadestHiter.m_pHiter->GetEnthrallCoste() / 100;
#else
        dwRate = m_pBase->m_wDropRate[iListNo];
#endif
				// Drop Item By The Drop Rate
#ifdef FUNCTION_LUCKY_ITEM
        if(m_BadestHiter.m_pHiter->GetSpecialStatus() & SPE_STATUS_LUCKY)
        {
          //dwRate*=2;
          CSrvBaseSkill *pBaseSkill = m_BadestHiter.m_pHiter->GetFuncAlarm(32);
          if( pBaseSkill )
          {
            dwRate = dwRate * ( 100 + pBaseSkill->GetProbability() );
            dwRate /= 100;
          }

        }
#endif
				if( gf_GetRandom_G(10000) < dwRate )
				{
					// Random OK, Drop The Item After Fix The Item
					if( NULL != ( pDropCItem = g_pGs->CreateCItem( pItemFix, this, 1 ) ) )
					{
            // 如果是Boss, 将掉落的物品显示给周围的玩家看
            if( GetType() & NPC_ATTRI_BOSS )
            {
              *pBossDropIdList = pDropCItem->GetId();
              g_BossItemNum++;
              pBossDropIdList++;
#ifdef FUNCTION_TALKTOALL_DROPITEM
              const char *szItemName = NULL;
              szItemName             = pDropCItem->GetName();
              szItem += ' ';
              szItem += szItemName;
#endif //FUNCTION_TALKTOALL_DROPITEM
//
#ifdef _BOSS_DROP_ON_GROUND_
              goto DROP_ON_GROUND;        
#endif
            }

						// 直接放到伤害最多的玩家Package中
						if( bPackItem )
						{
              wAddItemPos = 0xFFFF;
							for( int i = 0; i < MAX_OWN_ITEM; i++ )
							{
								if( m_BadestHiter.m_pHiter->AddPackageItem( pDropCItem, i ) )
								{
                  wAddItemPos = i;
                  pDropCItem->SetMailId( m_BadestHiter.m_pHiter->GetMailId() );
                  // Record Rare Item And Send Monster Drop Log
                  if( pDropCItem->GetRareType() > 3 )
                  {
                    m_BadestHiter.m_pHiter->SendMonsterDropLog( pDropCItem, this );
                  }
                  //
									if( iPackage == 0 )
									{
										if( NULL == ( pNewDropMsg = g_pGs->NewMsgBuffer( m_BadestHiter.m_pHiter->GetSelfCode() ) ) )
										{
                      if( m_BadestHiter.m_pHiter->DelPackageItem( pDropCItem ) )
                      {
#ifdef _DEBUG_WILDCAT_
  #ifdef _REPAIR_SERVER_CRASH_NICK_
												SafeStrcpy( g_szMonsterLog, "Cannot New Msg Buffer, Old Version Crash When Delete Item 1 #", MAX_MEMO_MSG_LEN );
  #else
                        strcpy( g_szMonsterLog, "Cannot New Msg Buffer, Old Version Crash When Delete Item 1 #" );
  #endif
                        AddErrLogOnly( g_szMonsterLog );
#endif
                        //
#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
                        if( pDropCItem->GetPackagePos() != 0 )
                        {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
                          FuncName( "CMonster::DropItem" );
				                  _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "***** 2 Delete Error Item=%d, Package Pos=%d ! *****", pDropCItem->GetId(), pDropCItem->GetPackagePos() );
                          g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
				                  AddMemoErrMsg( g_szMonsterLog );
#endif
                        }
#endif
                        //
		                    g_pGs->DeleteCItem( pDropCItem, 9 );
                        pDropCItem = NULL;
                      }
											//return FALSE;
											goto GO_TO_SEND_MESSAGE;
										}
										pNewDropMsg->Init();
										pNewDropMsg->dwAID = A_ADDMONSTERITEM;
										pMonsterDrop = (LPSNMMonsterDrop)(pNewDropMsg->Msgs[0].Data);
										pPlayerItem  = (SPlayerItemExC*)( pNewDropMsg->Msgs[0].Data + sizeof(SNMMonsterDrop) );
									}
									// Get The Drop CItem Info
									//pDropCItem->SetUniqueData( m_BadestHiter.m_pHiter );
									m_BadestHiter.m_pHiter->Get_SPlayerItemExC( pDropCItem, pPlayerItem );
									iPackage++;
									pPlayerItem++;
									if( iPackage == 20 )
									{
										pPlayerItem  = (SPlayerItemExC*)(pNewDropMsg->Msgs[1].Data);
									}
									else if( iPackage >= 39 )
									{
										bPackItem = FALSE;
										goto DROP_ON_GROUND;
									}
									break;
								}
							}
              //
							if( wAddItemPos == 0xFFFF )
							{
								bPackItem = FALSE;
								goto DROP_ON_GROUND;
							}
						}
						else
						{
DROP_ON_GROUND:
							wRandNum   = gf_GetRandom(24);
							wRandDropX = m_iX + g_DropItemPosX[wRandNum];
							wRandDropY = m_iY + g_DropItemPosY[wRandNum];
							if( m_pInMap->GetTileFlag( wRandDropX, wRandDropY ) & TILE_ALLOCCULDE )
							{
								wRandDropX = m_iX;
								wRandDropY = m_iY;
							}
              //
							if( NULL != ( pDropGItem = new CGroundItem( m_pInMap, 1, wRandDropX, wRandDropY, MonsterTickCount + g_pGs->GetItemInterval() ) ) )
							{
								if( m_pInMap->AddGroundItem( pDropGItem ) )
								{
                  pDropGItem->SetMasterMailId( m_BadestHiter.m_pHiter->GetMailId() );
									pDropGItem->SetBaseItem( pDropCItem );
#ifdef _BOSS_DROP_ON_GROUND_
                  pDropGItem->SetMasterTime(7000);  // ClientTickCount+parameter
#endif
                  //
									pAddGroundItem->dwCode_Id = ( pDropGItem->GetCode() << 16 ) | pDropCItem->GetId();
									pAddGroundItem->dwX_Y			= ( ((DWORD)wRandDropX) << 16 ) | wRandDropY;
									pAddGroundItem->dwCount   = ( 1 << 16 ) |
                                              ( pDropCItem->GetHoleNumber() << 8 ) |
                                              ( pDropCItem->GetLevel() );
                  //
                  pAddGroundItem->dwG_BColor= pDropCItem->GetBColor() & 0x0000FFFF;
                  pAddGroundItem++;
                  iDropItemCount++;
#ifdef VERSION40_UNIQUERING
                  if( pDropCItem->IsVanishLater() )
                  {
                    SUniqueItem* pUniItem = new SUniqueItem;
                    if( pUniItem )
                    {
                      pUniItem->dwCode = 0xffffffff;
                      pUniItem->dwId = pDropCItem->GetId();
                      if( m_pInMap )
                        pUniItem->dwMapId = m_pInMap->GetMapId();
                      pUniItem->wX = wRandDropX;
                      pUniItem->wY = wRandDropY;
                      pUniItem->dwStartTime = MonsterTickCount;
                      pUniItem->dwEndTime = MonsterTickCount + pDropCItem->GetVar(0);
                      g_pRingManager->m_listUniItem.push_back(pUniItem);
                    }
                    g_pRingManager->m_bDrop = TRUE;
                    //需要由mcc广播其他服务器，无双戒已经产生
                  }
#endif
								}
								else
								{
									SAFE_DELETE( pDropGItem );
#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
                  if( pDropCItem->GetPackagePos() != 0 )
                  {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
                    FuncName( "CMonster::DropItem" );
                    _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "***** 3 Delete Error Item=%d, Package Pos=%d ! *****", pDropCItem->GetId(), pDropCItem->GetPackagePos() );
                    g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
                    AddMemoErrMsg( g_szMonsterLog );
#endif
                  }
#endif
                  //
		              g_pGs->DeleteCItem( pDropCItem, 10 );
                  pDropCItem = NULL;
								}
							}
							else
							{
#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
                if( pDropCItem->GetPackagePos() != 0 )
                {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
                  FuncName( "CMonster::DropItem" );
                  _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "***** 4 Delete Error Item=%d, Package Pos=%d ! *****", pDropCItem->GetId(), pDropCItem->GetPackagePos() );
                  g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
                  AddMemoErrMsg( g_szMonsterLog );
#endif
                }
#endif
                //
		            g_pGs->DeleteCItem( pDropCItem, 11 );
                pDropCItem = NULL;
							}
						}
					}
          else
          {
            goto GO_TO_SEND_MESSAGE;
          }
				}
			}
			else	break;
		}
	}
#ifdef FUNCTION_TALKTOALL_DROPITEM
  if((g_BossItemNum != 0) && !(szItem.empty()))
  {
    char szTemp[300];
    sprintf(szTemp, "產%s驹秤%s莉眔%s", m_BadestHiter.m_pHiter->GetName(), GetName(), szItem.c_str());
    
    SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
    if( NULL != pNewMsg )
    {
      pNewMsg->dwAID        = A_TALKTOALL;
      pNewMsg->dwMsgLen	    = 1;
      pNewMsg->Msgs[0].Size = strlen( szTemp );
#ifdef _REPAIR_SERVER_CRASH_NICK_
      SafeStrcpy( pNewMsg->Msgs[0].Data, szTemp, MAXMSGDATASIZE );
#else
      strcpy( pNewMsg->Msgs[0].Data, szTemp );
#endif
      g_pGs->SendTheMsgToAll( *pNewMsg );
      g_pGs->ReleaseMsg( pNewMsg );
    }
  }
#endif //FUNCTION_TALKTOALL_DROPITEM
	// 捡的物品全掉在地上
	if( !m_listCollectItem.empty() )
	{
		for( Iter_GItem = m_listCollectItem.begin(); Iter_GItem != m_listCollectItem.end(); Iter_GItem++ )
		{
			wRandNum   = gf_GetRandom(24);
			wRandDropX = m_iX + g_DropItemPosX[wRandNum];
			wRandDropY = m_iY + g_DropItemPosY[wRandNum];
			if( m_pInMap->GetTileFlag( wRandDropX, wRandDropY ) & TILE_ALLOCCULDE )
			{
				wRandDropX = m_iX;
				wRandDropY = m_iY;
			}
			pDropGItem = (CGroundItem*)(*Iter_GItem);
			pDropGItem->SetXY( wRandDropX, wRandDropY );
			pDropGItem->SetVanishTime( MonsterTickCount + g_pGs->GetItemInterval() );
			if( m_pInMap->AddGroundItem( pDropGItem ) )
			{
				pAddGroundItem->dwCode_Id = ( pDropGItem->GetCode() << 16 ) | pDropGItem->GetId();
				pAddGroundItem->dwX_Y			= ( wRandDropX << 16 ) | wRandDropY;
        
        if( pDropCItem = pDropGItem->GetBaseItem() )
        {
#ifdef VERSION40_UNIQUERING
          if( pDropCItem->IsVanishLater() )
          {
            SUniqueItem* pUniItem = new SUniqueItem;
            if( pUniItem )
            {
              pUniItem->dwCode = 0xffffffff;
              pUniItem->dwId = pDropCItem->GetId();
              if( m_pInMap )
                pUniItem->dwMapId = m_pInMap->GetMapId();
              pUniItem->wX = wRandDropX;
              pUniItem->wY = wRandDropY;
              pUniItem->dwStartTime = MonsterTickCount;
              pUniItem->dwEndTime = MonsterTickCount + pDropCItem->GetVar(0);
              g_pRingManager->m_listUniItem.push_back(pUniItem);
            }
          }
          if( pDropCItem->IsUniqueItem() )
          {
            SUniqueRingInfo* pInfo = new SUniqueRingInfo;
            if( pInfo )
            {
              pInfo->dwOwnerMailId = 0;
              pInfo->dwMapId = m_pInMap->GetMapId();
              pInfo->wX = wRandDropX;
              pInfo->wY = wRandDropY;
              g_pRingManager->SendMccRingStatus(pInfo);
            }
           SAFE_DELETE( pInfo );
          }
#endif
          pAddGroundItem->dwCount   = ( pDropGItem->GetCount() << 16 ) |
            ( pDropCItem->GetHoleNumber() << 8 ) |
            ( pDropCItem->GetLevel() );
          pAddGroundItem->dwG_BColor= ( pDropCItem->GetBColor() & 0x0000FFFF );
        }
        else
        {
          pAddGroundItem->dwCount   = ( 1 << 16 );
          pAddGroundItem->dwG_BColor= 0;
        }
        pDropCItem = NULL;
				pAddGroundItem++;
				iDropItemCount++;
			}
			else
			{
				// 不能丢在地上, 则全部删除
				if( NULL != ( pDropCItem = pDropGItem->GetBaseItem() ) )
				{
#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
          if( pDropCItem->GetPackagePos() != 0 )
          {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
            FuncName( "CMonster::DropItem" );
            _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "***** 5 Delete Error Item=%d, Package Pos=%d ! *****", pDropCItem->GetId(), pDropCItem->GetPackagePos() );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
            AddMemoErrMsg( g_szMonsterLog );
#endif
          }
#endif
		      g_pGs->DeleteCItem( pDropCItem, 12 );
          pDropCItem = NULL;
				}
				SAFE_DELETE( pDropGItem );
			}
		}
		m_listCollectItem.clear();
	}
GO_TO_SEND_MESSAGE:
  BOOL            bSendGroundMoney = FALSE;
  // 如果背包空间不足， 钱也掉在地上
  if( ( m_BadestHiter.m_pHiter->GetMoney() + m_pBase->m_dwMoney + m_dwPickUpMoney ) > MAX_PLAYER_MONEY )
  {
    bSendGroundMoney = TRUE;
		if( m_pBase->m_dwMoney + m_dwPickUpMoney > 0 && gf_GetRandom4() < 700 )
		{
			wRandNum   = gf_GetRandom(24);
			wRandDropX = m_iX + g_DropItemPosX[wRandNum];
			wRandDropY = m_iY + g_DropItemPosY[wRandNum];
			if( m_pInMap->GetTileFlag( wRandDropX, wRandDropY ) & TILE_ALLOCCULDE )
			{
				wRandDropX = m_iX;
				wRandDropY = m_iY;
			}

      dwRandMoney = gf_GetRandom( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 20 / 100 ) +
                                  ( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 80 / 100 );
#ifdef PREVENT_ENTHRALL_SYSTEM
      dwRandMoney = dwRandMoney * m_BadestHiter.m_pHiter->GetEnthrallCoste() / 100;
#endif
			if( NULL != ( pDropGItem = new CGroundItem( m_pInMap, dwRandMoney, wRandDropX, wRandDropY, MonsterTickCount + g_pGs->GetItemInterval() ) ) )
			{
				if( m_pInMap->AddGroundItem( pDropGItem ) )
				{
          pDropGItem->SetMasterMailId( m_BadestHiter.m_pHiter->GetMailId() );
					pAddGroundItem->dwCode_Id = ( pDropGItem->GetCode() << 16 ) | GROUNDITEM_MONEY_ID;
					pAddGroundItem->dwCount   = dwRandMoney;
					pAddGroundItem->dwX_Y			= ( wRandDropX << 16 ) | wRandDropY;
          pAddGroundItem->dwG_BColor= 0;

					pAddGroundItem++;
					iDropItemCount++;
				}
				else
				{
					SAFE_DELETE( pDropGItem );
				}
			}
			m_dwPickUpMoney = 0;
		}
  }
  // 发送玩家获得的物品
  if( iPackage )
  {
    // 发送消息到Client
    if( iPackage > 0 && iPackage <= 20 )
    {
      pMonsterDrop->wCode    = GetSelfCode();
      pNewDropMsg->dwMsgLen  = 1;
      pNewDropMsg->Msgs[0].Size = sizeof( SNMMonsterDrop ) + sizeof( SPlayerItemExC ) * iPackage;
      if( bSendGroundMoney == FALSE && m_pBase->m_dwMoney + m_dwPickUpMoney > 0 )
      {
        dwRandMoney = gf_GetRandom( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 20 / 100 ) +
          ( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 80 / 100 );
        
#ifdef PREVENT_ENTHRALL_SYSTEM
         pMonsterDrop->dwMoney  = dwRandMoney * m_BadestHiter.m_pHiter->GetEnthrallCoste() / 100;
#else 
         pMonsterDrop->dwMoney  = dwRandMoney;
#endif
        m_BadestHiter.m_pHiter->AddMoney( pMonsterDrop->dwMoney );
      }
      else
      {
        pMonsterDrop->dwMoney = 0;
      }
      m_dwPickUpMoney = 0;
      m_BadestHiter.m_pHiter->AddSendMsg( pNewDropMsg );
    }
    else if( iPackage > 20 && iPackage <= 40 )
    {
      pMonsterDrop->wCode    = GetSelfCode();
      pNewDropMsg->dwMsgLen  = 2;
      pNewDropMsg->Msgs[0].Size = sizeof( SNMMonsterDrop ) + sizeof( SPlayerItemExC ) * 20;
      pNewDropMsg->Msgs[1].Size = sizeof( SPlayerItemExC ) * ( iPackage - 20 );
      if( bSendGroundMoney == FALSE && m_pBase->m_dwMoney + m_dwPickUpMoney > 0 )
      {
        dwRandMoney = gf_GetRandom( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 20 / 100 ) +
          ( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 80 / 100 );
        
#ifdef PREVENT_ENTHRALL_SYSTEM
         pMonsterDrop->dwMoney  = dwRandMoney * m_BadestHiter.m_pHiter->GetEnthrallCoste() / 100;
#else
         pMonsterDrop->dwMoney  = dwRandMoney;
#endif
        m_BadestHiter.m_pHiter->AddMoney( pMonsterDrop->dwMoney );
      }
      else
      {
        pMonsterDrop->dwMoney = 0;
      }
      m_dwPickUpMoney = 0;
      m_BadestHiter.m_pHiter->AddSendMsg( pNewDropMsg );
    }
  }
  else if( iPackage == 0 && bSendGroundMoney == FALSE )
  {
    if( pNewDropMsg )    
		{
			g_pGs->ReleaseMsg( pNewDropMsg );
			pNewDropMsg = NULL;
		}
    //
    if( NULL != ( pNewDropMsg = g_pGs->NewMsgBuffer( m_BadestHiter.m_pHiter->GetSelfCode() ) ) )
    {
      pNewDropMsg->Init();
      pNewDropMsg->dwAID        = A_ADDMONSTERITEM;
      pNewDropMsg->dwMsgLen     = 1;
      pNewDropMsg->Msgs[0].Size = sizeof( SNMMonsterDrop );
      //
      pMonsterDrop              = (LPSNMMonsterDrop)(pNewDropMsg->Msgs[0].Data);
      pMonsterDrop->wCode       = GetSelfCode();
      if( m_pBase->m_dwMoney + m_dwPickUpMoney > 0 )
      {
        dwRandMoney = gf_GetRandom( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 20 / 100 ) +
          ( ( m_pBase->m_dwMoney + m_dwPickUpMoney ) * 80 / 100 );       
#ifdef PREVENT_ENTHRALL_SYSTEM
         pMonsterDrop->dwMoney  = dwRandMoney * m_BadestHiter.m_pHiter->GetEnthrallCoste() / 100;
#else
         pMonsterDrop->dwMoney  = dwRandMoney;
#endif
        m_BadestHiter.m_pHiter->AddMoney( pMonsterDrop->dwMoney );
      }
      else
      {
        pMonsterDrop->dwMoney = 0;
      }
      m_dwPickUpMoney = 0;
      m_BadestHiter.m_pHiter->AddSendMsg( pNewDropMsg );
    }
  }
	// 发送地上物品增加的消息
	if( iDropItemCount )
	{
		// Send Message To Nearly Player
		MsgDropItem.Msgs[0].Size = sizeof( SNMAddGroundItem ) * iDropItemCount;
		m_pInMap->SendMsgNearPosition( MsgDropItem, m_iX, m_iY );
	}
  // Send Rare Item Message
  if( g_BossItemNum )
  {
    MsgBossDrop.Msgs[0].Size = sizeof( WORD ) * g_BossItemNum;

    *(WORD*)(MsgBossDrop.Msgs[1].Data) = GetBaseId();
    *(WORD*)(MsgBossDrop.Msgs[1].Data+sizeof(WORD)) = m_BadestHiter.m_pHiter->GetSelfCode();
    m_pInMap->SendMsgNearPosition_Close( MsgBossDrop, m_iX, m_iY );
    //
    g_BossItemNum = 0;
  }
	return TRUE;
}
//============================================================================================
//
//
int CMonster::GetDistance( const WORD & wDesX, const WORD & wDesY )
{
  static WORD wDistanceX, wDistanceY;

	wDistanceX = abs( wDesX - m_iX );
	wDistanceY = abs( wDesY - m_iY );

  return wDistanceX > wDistanceY ? wDistanceX : wDistanceY;
}
//============================================================================================
//
//
inline int CMonster::GetDistance( CPlayer * pThePlayer )
{
  static WORD       wDisX, wDisY;

  if( pThePlayer == NULL || pThePlayer->GetStatus() == STATUS_PLAYER_DEAD ||
      pThePlayer->GetInMap() == NULL ||
      pThePlayer->GetInMap() != m_pInMap )
	{
    return 0x7FFFFFFF;
	}
  wDisX = abs( pThePlayer->GetPosX() - m_iX );
  wDisY = abs( pThePlayer->GetPosY() - m_iY );
  return ( wDisX > wDisY ? wDisX : wDisY );
}
//============================================================================================
//
//Functions:
//
inline BOOL CMonster::SetMonsterSight( int iSight )
{
	if( iSight > 11 || iSight < 0 )
  {
    m_iSightForCheck = 289;
    m_iQtrSight      = m_iSightForCheck >> 2;
    m_iCurrSight     = 0;
    return FALSE;
  }
	int			iRange = 0;
	for( int i = 1; i <= iSight; i++ )
	{
		iRange += i * 8;
	}
	if( iRange > 528 )
  {
    m_iSightForCheck = 441;
    m_iQtrSight      = m_iSightForCheck >> 2;
    m_iCurrSight     = 0;
    return FALSE;
  }
	else
	{
		m_iSightForCheck = ( iRange + 1 );
    m_iQtrSight      = m_iSightForCheck >> 2;
    m_iCurrSight     = 0;
		return TRUE;
	}
}
//============================================================================================
//
//Functions:
inline void CMonster::GetLastStep()
{
	switch( m_iMoveDir )
	{
	case 0:
		m_iX += 1;
		break;
	case 1:
		m_iX += 1;  m_iY += 1;
		break;
	case 2:
		m_iY += 1;
		break;
	case 3:
		m_iX -= 1;	m_iY += 1;
		break;
	case 4:
		m_iX -= 1;
		break;
	case 5:
		m_iX -= 1;	m_iY -= 1;
		break;
	case 6:
		m_iY -= 1;
		break;
	case 7:
		m_iX += 1;	m_iY -= 1;
		break;
	default:
		SetPathEnd( m_iX, m_iY );
		break;
	}
}
//============================================================================================
//
//Functions:
//
#ifdef _DEBUG
inline void CMonster::GetNextStep(const WORD & iType)
#else
inline void CMonster::GetNextStep()
#endif
{
  static WORD     wMagicCode;
  static CMagic   *pTheMagic = NULL;
  
	switch( m_iMoveDir )
	{
	case 0:
		m_iX -= 1;
		break;
	case 1:
		m_iX -= 1;  m_iY -= 1;
		break;
	case 2:
		m_iY -= 1;
		break;
	case 3:
		m_iX += 1;	m_iY -= 1;
		break;
	case 4:
		m_iX += 1;
		break;
	case 5:
		m_iX += 1;	m_iY += 1;
		break;
	case 6:
		m_iY += 1;
		break;
	case 7:
		m_iX -= 1;	m_iY += 1;
		break;
	default:
		SetPathEnd( m_iX, m_iY );
		break;
	}
  //
#ifdef _DEBUG_SHOW_MONSTER_MOVE_IN_OBSTACLE_
  if( m_pInMap->GetTileFlag( m_iX, m_iY ) & TILE_MAPOCCULDE )
  {
#ifdef _DEBUG
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName( "CMonster::GetNextStep" );
    _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "=%d==>>%s[%d][%d] Move(%d) Into Obstacle(%d), Pos(%d,%d), Flag=%08x", iType, GetName(), GetCode(), m_pBase->GetId(), m_iMoveDir, GetMapId(), m_iX, m_iY, m_pInMap->GetTileFlag( m_iX, m_iY ) );
    g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szMonsterLog );
#endif
#endif
    m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
    m_iNpcStatus  = STATUS_MONSTER_GOHOME;
  }
#endif

  if( m_iX > m_pInMap->GetSizeX() || m_iY > m_pInMap->GetSizeY() )
  {
    m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 200;
    m_iNpcStatus  = STATUS_MONSTER_GOHOME;
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG
    FuncName("CMonster::SetMonsterDir");
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "=%d==>>%s[%d][%d] Go Home In Map(%d) Because Next Step...1", iType, GetName(), GetCode(), m_pBase->GetId(), GetMapId() );
    g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
  }
  //
  if( wMagicCode = m_pInMap->GetMagicCode( m_iX, m_iY ) )
  {
    if( pTheMagic = m_pInMap->GetMagic( wMagicCode ) )
    {
      pTheMagic->BeActivation( this );
    }
  }
}
//============================================================================================
//
//Functions:
//
BOOL CMonster::UpdateTurnCheck()
{
  static int dwUpdateCheck;
	
	dwUpdateCheck = m_iUpdateTurn;
  if( m_iUpdateTurn <= 0 )
  {
    m_iUpdateTurn = MonsterTickCount + UPDATE_NEAR_TIME_MS;
  }
  else if( MonsterTickCount > m_iUpdateTurn )
  {
    m_iUpdateTurn = 0;
  }
	return ( dwUpdateCheck <= 0 );
}
//============================================================================================
//
//Functions:
void CMonster::DoOdds(CPlayer * pPlayer, int iIndex)
{
	bool						bStop = false;
  CEventRunning		*pTheRunNode = NULL;

	if(pPlayer->GetSelfCode() != m_pLastHit->GetSelfCode() )		return;
  if(!m_pDeadEvent)																		return;

  switch(m_pDeadEvent->GetPointType())
  {
    case EVENT_POINTTYPE_NORMAL:
    case EVENT_POINTTYPE_CYCLE:
    case EVENT_POINTTYPE_BACKMUSIC:
    case EVENT_POINTTYPE_HOUSE:
    case EVENT_POINTTYPE_HOUSE_NEW:
    case EVENT_POINTTYPE_PUTMONSTER:
			return;
		case EVENT_POINTTYPE_MONSTER:
      //if(pPlayer->IsTalkingWithNpc())
      { // Continue the Running Event
        while(pTheRunNode = pPlayer->GetBack_EventRunning())
        {
          bStop = (pTheRunNode->GetRunOdds())->RunToNextMsg(pPlayer, pPlayer->GetEventNpcCode(), iIndex, pTheRunNode->GetNextNodeIndex());
          if(bStop)
          {
            
          }
				}
        if(pPlayer->GetSubEvent())
				  bStop = (pPlayer->GetSubEvent())->RunToNextMsg(pPlayer, pPlayer->GetEventNpcCode(), pPlayer->GetNextEventRunNode());
        if(bStop)
        {

        }
        else
        {
					// Event is finished
          pPlayer->Send_A_CANCEL_NPC(GetSelfCode());
          pPlayer->ClearEventState();
          pPlayer->SetState( STATUS_NPC_STAND );
        }
      }
      return;
      break;
    default:
      break;
  }
}
//============================================================================================
//
// Return Value: 1: 成功捡起道具;  0: 已超过十个道具;  -1: 无法捡起该道具
inline int CMonster::PickUpGItem()
{
	static CGroundItem			*pPickItem, *pAutoDropItem;
	static CItem						*pPickCItem;
	static SMsgData					NewPickMsg;

	if( m_wTargetItemX && m_wTargetItemY )
	{
		if( NULL != ( pPickItem = m_pInMap->GetTileFirstItem( m_wTargetItemX, m_wTargetItemY ) ) )
		{
			if( m_pInMap->DelGroundItem( pPickItem->GetCode(), FALSE ) )
			{
//#ifndef _DEBUG_NO_ANY_LOG_
//#ifdef _DEBUG_WILDCAT_
//				sprintf( g_szMonsterLog, "Monster[%s] Pick Up GroundItem(%d) In Tile(%d,%d) #", GetName(), pPickItem->GetSelfCode(), pPickItem->GetPosX(), pPickItem->GetPosY() );
//				AddMemoMsg( g_szMonsterLog );
//#endif
//#endif
				if( pPickItem->GetId() == GROUNDITEM_MONEY_ID )
				{
					if( m_pBase->m_dwDelTime > 0 )
					{
						m_dwPickUpMoney += pPickItem->GetCount();
					}
					*(DWORD*)(MsgClearCode.Msgs[0].Data)			  = pPickItem->GetCode();
					m_pInMap->SendMsgNearPosition( MsgClearCode, m_iX, m_iY );
					SAFE_DELETE( pPickItem );
				}
				else
				{
					if( m_pBase->m_dwDelTime > 0 )
					{
						pPickItem->SetVanishTime( MonsterTickCount + m_pBase->m_dwDelTime );
						m_listCollectItem.push_back( pPickItem );
						*(DWORD*)(MsgClearCode.Msgs[0].Data)				= pPickItem->GetCode();
						m_pInMap->SendMsgNearPosition( MsgClearCode, m_iX, m_iY );
					}
					else
					{
						*(DWORD*)(MsgClearCode.Msgs[0].Data)				= pPickItem->GetCode();
						m_pInMap->SendMsgNearPosition( MsgClearCode, m_iX, m_iY );
						pPickCItem = pPickItem->GetBaseItem();
//#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
//            if( pPickCItem->GetPackagePos() != 0 )
//            {
//              FuncName( "CMonster::PickUpGItem" );
//              sprintf( g_szMonsterLog, "***** 6 Delete Error Item=%d, Package Pos=%d ! *****", pPickCItem->GetId(), pPickCItem->GetPackagePos() );
//              AddMemoErrMsg( g_szMonsterLog );
//            }
//#endif
		        g_pGs->DeleteCItem( pPickCItem, 13 );
            pPickCItem = NULL;
						SAFE_DELETE( pPickItem );
					}
				}
				pPickItem = NULL;
				// Send The Delete Ground Item Message To Near Client
				//*(DWORD*)(MsgPickItem.Msgs[0].Data)				= pPickItem->GetSelfCode();
				//*(((DWORD*)(MsgPickItem.Msgs[0].Data))+1)	= GetSelfCode();

				// If Collect Ground Item > 10 And Attribute & NPC_ATTRI_DROP, Auto Drop The First Item
				if( m_listCollectItem.size() > 10 && m_pBase->m_iNpcProperty & NPC_ATTRI_DROP )
				{
					pAutoDropItem = (*m_listCollectItem.begin());
					m_listCollectItem.pop_front();
					pPickCItem = pAutoDropItem->GetBaseItem();
          //
		      g_pGs->DeleteCItem( pPickCItem, 14 );
          pPickCItem = NULL;
					SAFE_DELETE( pAutoDropItem );
					return 0;
				}
				return 1;
			}
		}
	}
	// 物品已被捡走, 无法捡起该道具
	return -1;
}
//#define _DEBUG_SHOW_MONSTER_MOVE_INTERVAL_
//============================================================================================
//
//Functions:
void CMonster::DoActionEX()
{
#ifdef _DEBUG_SHOW_MONSTER_INFO_
	FuncName("CMonster::DoActionEX");
#endif

#ifdef _DEBUG_SHOW_MONSTER_MOVE_INTERVAL_
	FuncName("CMonster::DoActionEX");
  static DWORD    dwMonsterMove = MonsterTickCount;
#endif

  static int			iReval;

	MonsterTickCount = TimeGetTime();
  if( m_qwSpecialStatus )   CheckTimer();
//add by zetorchen
#ifdef _MONSTER_RESTORE_HP_ // Monster Auto Restore HP every 10s;
  int RestoreLev = 0;
  if ( GetHp()<GetMaxHp() && 0 != (RestoreLev=CanRestore()) )
  {
    if ( MonsterTickCount > m_dwAlarmTime[31] )
    {
      int iAddHp=0;
      if ( RestoreLev == 1 )
        iAddHp = ( g_iMonsterHP1 / 10000.0 ) * GetMaxHp() ;
      else if( RestoreLev == 2 )
        iAddHp = ( g_iMonsterHP2 / 10000.0 ) * GetMaxHp() ;
      else if( RestoreLev == 3 )
        iAddHp = ( g_iMonsterHP3 / 10000.0 ) * GetMaxHp() ;
      AddHp(iAddHp);
      MsgUpdateMstHp.Msgs[0].Size = sizeof( SNMUpdateMonsterHp );
      SNMUpdateMonsterHp    *pUpdate = (SNMUpdateMonsterHp*)(MsgUpdateMstHp.Msgs[0].Data);
      pUpdate->wCode  = GetCode();
      pUpdate->iMapHp = GetMaxHp();
      pUpdate->iHp    = GetHp();
      m_pInMap->SendMsgNearPosition_Far( MsgUpdateMstHp, GetPosX(), GetPosY() );
      m_dwAlarmTime[31] = MonsterTickCount + 10000 ;
    }
  }
#endif
	switch( m_iNpcStatus )
	{
	// 追击目标, 捡骨时移动
	case STATUS_MONSTER_PURSUE:
		{
			// 追击攻击目标时移动, 包含战斗支援
			if( m_iAddiStatus == ATTACK_STATUS_MONSTER_PURSUE )
			{
				MoveToAim();
			}
			// 辅助支援
			else if( m_iAddiStatus == ATTACK_STATUS_MONSTER_ASSAID )
			{
				MoveToTeamer();
			}
			// 收集道具时移动
			else if( m_iAddiStatus == ATTACK_STATUS_MONSTER_COLLECT )
			{
				MoveToItem();
			}
			// 巡逻时移动
			else if( m_iAddiStatus == ATTACK_STATUS_MONSTER_WANDER )
			{
				WanderShortPath();
			}
      else
      {
			  // 若不是以上状态, 清空并重新循环
			  ClearStatus();
      }
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_SHOW_MONSTER_MOVE_INTERVAL_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
      _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s Move, Dir=%d, Pos=(%d,%d), Itrv=%d", m_pBase->m_szName, m_iMoveDir, m_iX, m_iY, MonsterTickCount - dwMonsterMove );
      g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szMonsterLog );
#endif
      dwMonsterMove = MonsterTickCount;
#endif
#endif
      return;
		}
		break;
	// 找寻巡逻路径, 巡逻时移动
	case STATUS_MONSTER_WANDER:
		{
			// 找寻巡逻路径, 巡逻时移动
			WanderShortPath();
			return;
		}
	// 站立状态
	case STATUS_MONSTER_STAND:
		{
			if( MonsterTickCount > m_dwEnd[MST_STAND_ACT] )
			{
#ifdef _DEBUG_MONSTER_ALWAYS_REVIVE_
        Died();
        return;
#endif
        m_wNotifyTimes  = 0;
				// 主动攻击, 先检查周围有没有玩家, 再决定是否巡逻
				if( !( m_pBase->m_iNpcProperty & NPC_ATTRI_UNAUTO ) )
				{
#ifndef _DEBUG_NO_ANY_LOG_
#if 0
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
          _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s: Check Range Target When Stand End #", m_pBase->m_szName );
          g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
          if( IsDefenceGuild() )    iReval = CheckGuildRange();
          else                      iReval = CheckRange();
					if( iReval == 0 )
					{
						// 没有可攻击玩家, 则巡逻
						WanderShortPath();
					}
					else if( iReval == 1 )
					{
						// 找到可攻击玩家
						m_wTargetDis = GetDistance( m_pLastTarget );
						if( m_wTargetDis <= m_iActiveRange )//&& ( m_pInMap->GetTileFlag( m_iX, m_iY ) & TILE_ALLOCCULDE ) )
						{
							// 切换到攻击状态
							m_iNpcStatus  = STATUS_MONSTER_ATTACK;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
							m_dwEnd[MST_ATTACK_ACT]	= 0;
              if( m_iX != m_iEndX && m_iY != m_iEndY )    SetPathEnd( m_iX, m_iY );
						}
						else if( m_wTargetDis != 0x7FFFFFFF )
						{
							// 切换状态到寻路
							m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
							return;
						}
            else
            {
              ClearStatus();
              return;
            }
					}
          else
          {
            return;
          }
					ClearHitData();
					return;
				}
				// 捡骨类, 先检查周围有没有可捡的道具, 再决定是否巡逻
				else if( m_pBase->m_iNpcProperty & NPC_ATTRI_COLLECT )
				{
					// 若找到地上物品, 则自动切换到捡骨寻路状态
          iReval = CheckItem();
					if( iReval == 0 )
					{
						ClearHitData();
						WanderShortPath();
					}
					else if( iReval == -1 )
					{
//#ifndef _DEBUG_NO_ANY_LOG_
//#ifdef _DEBUG_SHOW_MONSTER_INFO_
//						sprintf( g_szMonsterLog, "%s Ready To Pick Up Item In Map(%d) #", m_pBase->m_szName, m_dwMapId );
//						AddMemoMsg( g_szMonsterLog );
//#endif
//#endif
					}
					return;
				}
				else if( m_pBase->m_iNpcProperty & NPC_ATTRI_AUTORED )
				{
          iReval = CheckRedPlayer();
					if( iReval == 1 )
					{
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_SHOW_MONSTER_INFO_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
						_snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s Ready To Attack Red Player %s In Map(%d) #", m_pBase->m_szName, m_pLastTarget->GetPlayerName(), m_dwMapId );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
						AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
						// 找到可攻击玩家
						m_wTargetDis = GetDistance( m_pLastTarget );
						if( m_wTargetDis < m_iActiveRange )
						{
							// 切换到攻击状态
							m_iNpcStatus  = STATUS_MONSTER_ATTACK;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
							m_dwEnd[MST_ATTACK_ACT]	= 0;
						}
						else if( m_wTargetDis != 0x7FFFFFFF )
						{
							// 切换状态到寻路
							m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
              return;
						}
            else
            {
              ClearStatus();
              return;
            }
					}
					else if( iReval == 0 )
					{
						WanderShortPath();
					}
          else
          {
            return;
          }
					ClearHitData();
					return;
				}
				// 其他类型都直接巡逻
				else if( !( m_pBase->m_iNpcProperty & NPC_ATTRI_UNPURSUE ) )
				{
					ClearHitData();
					// 若不是不会移动的类型，则巡逻
					if( m_pBase->m_wSpeed > 0 )
					{
						WanderShortPath();
					}
					return;
				}
			}
      else if( MonsterTickCount > m_dwEnd[MST_SEARCH_ACT] )
      {
        m_wNotifyTimes  = 0;
        if( !( m_pBase->m_iNpcProperty & NPC_ATTRI_UNAUTO ) )
				{
#ifndef _DEBUG_NO_ANY_LOG_
#if 0
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
          _snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s: Check Range Target #", m_pBase->m_szName );
          g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
          if( IsDefenceGuild() )    iReval = CheckGuildRange();
          else                      iReval = CheckRange();
					if( iReval == 1 )
					{
						// 找到可攻击玩家
						m_wTargetDis = GetDistance( m_pLastTarget );
						if( m_wTargetDis <= m_iActiveRange )
						{
							// 切换到攻击状态
							m_iNpcStatus  = STATUS_MONSTER_ATTACK;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
							m_dwEnd[MST_ATTACK_ACT]	= 0;
              if( m_iX != m_iEndX && m_iY != m_iEndY )    SetPathEnd( m_iX, m_iY );
						}
						else if( m_wTargetDis != 0x7FFFFFFF )
						{
							// 切换状态到寻路
							m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
              return;
						}
            else
            {
              ClearStatus();
              return;
            }
					}
				}
				// 捡骨类, 先检查周围有没有可捡的道具, 再决定是否巡逻
				else if( m_pBase->m_iNpcProperty & NPC_ATTRI_COLLECT )
				{
					// 若找到地上物品, 则自动切换到捡骨寻路状态
					CheckItem();
				}
				else if( m_pBase->m_iNpcProperty & NPC_ATTRI_AUTORED )
				{
          iReval = CheckRedPlayer();
					if( iReval == 1 )
					{
						// 找到可攻击玩家
						m_wTargetDis = GetDistance( m_pLastTarget );
						if( m_wTargetDis < m_iActiveRange )
						{
							// 切换到攻击状态
							m_iNpcStatus  = STATUS_MONSTER_ATTACK;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
							m_dwEnd[MST_ATTACK_ACT]	= 0;
						}
						else if( m_wTargetDis != 0x7FFFFFFF )
						{
							// 切换状态到寻路
							m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
							m_iNextStatus = STATUS_MONSTER_ATTACK;
							m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
              return;
						}
            else
            {
              ClearStatus();
              return;
            }
					}
				}
        m_dwEnd[MST_SEARCH_ACT]   = MonsterTickCount + MONSTER_SEARCH_TIME;
      }
			return;
		}
		break;
	// 攻击状态
	case STATUS_MONSTER_ATTACK:
		{
			// 攻击状态, 并判断攻击效果
			if( MonsterTickCount > m_dwEnd[MST_ATTACK_ACT] )
			{
#ifdef _BOSS_CALL_UP_HAND_
        if( m_pBase->m_iNpcProperty & NPC_ATTRI_CHANGE_SKILL &&
            m_wCallUpNum < m_pBase->GetCallUpCount() &&
            gf_GetRandom4() < 1000 )
        {
          static SMsgData       MsgCallHand;

          // Call Up Hand
          CMonster				*pNewMonster;
				  WORD						wCreateCount = 0, wRandX, wRandY, wRandNum, wRandCall;

          wRandCall = ( gf_GetRandom( 100 ) % 3 ) + 1;

				  SNMNpcInfo			*pTheNpcInfo = (SNMNpcInfo*)(MsgCallHand.Msgs[1].Data);
          for( int i = 0; i < wRandCall; i++ )
          {
            wRandNum  = gf_GetRandom( 48 );
            wRandX    = m_pLastTarget->GetPosX() + DirOffsetX[wRandNum];
            wRandY    = m_pLastTarget->GetPosY() + DirOffsetY[wRandNum];
				    if( NULL != ( pNewMonster = m_pInMap->CreateMonster( 37, wRandX, wRandY ) ) )
					  {
              pNewMonster->SetReviveType( REVIVE_TYPE_DELETE );
              pNewMonster->SetFather( this );
						  pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
						  pTheNpcInfo++;
						  wCreateCount++;
            }
					}
		      if( wCreateCount )
				  {
            m_wCallUpNum += wCreateCount;
            // Init Message
            MsgCallHand.Init();
            MsgCallHand.dwAID        = A_PLAYERINFO;
				    MsgCallHand.dwMsgLen     = 2;
					  MsgCallHand.Msgs[0].Size = 1;
					  MsgCallHand.Msgs[1].Size = sizeof( SNMNpcInfo ) * wCreateCount;

					  m_pInMap->SendMsgNearPosition( MsgCallHand, m_iX, m_iY );
				  }
          m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + 1000;
          return;
        }
#endif
        // 每次更换招式
        if( m_pBase->m_iNpcProperty & NPC_ATTRI_CHANGE_SKILL )
        {
          ChooseSkill( TRUE );
        }
				m_wTargetDis = GetDistance( m_pLastTarget );
				if( m_wTargetDis <= m_iActiveRange )
				{
					iReval = Attack();
				}
				else if( m_wTargetDis != 0x7FFFFFFF )
				{
					m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
					m_iNextStatus = STATUS_MONSTER_ATTACK;
					m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
          return;
				}
        else
        {
          ClearStatus();
          return;
        }

				// 攻击成功
				if( iReval == 1 )
				{
          // 魔王一秒攻击一次
					if( m_pBase->m_iNpcProperty & NPC_ATTRI_CHANGE_SKILL )
          {
					  m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount +
                                        //MONSTER_ATTACK_TIME +
                                        m_pUsingSkill->GetBeforeDelay() +
                                        m_pUsingSkill->GetAfterDelay();
            if( m_pUsingSkill->GetBeforeDelay() + m_pUsingSkill->GetAfterDelay() < 1000 )
            {
              m_dwEnd[MST_ATTACK_ACT] += MONSTER_ATTACK_TIME;
            }
          }
          else
          {
					  m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount +
                                        //MONSTER_HALF_ATTACK_TIME +
                                        m_pUsingSkill->GetBeforeDelay() +
                                        m_pUsingSkill->GetAfterDelay();
            if( m_pUsingSkill->GetBeforeDelay() + m_pUsingSkill->GetAfterDelay() < 1000 )
            {
              m_dwEnd[MST_ATTACK_ACT] += MONSTER_HALF_ATTACK_TIME;
            }
          }
				}
				// 需要换一个攻击招式, 同时重新寻路
				else if( iReval == 0 )
				{
					//ChooseSkill();
					//m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
					//m_iNextStatus = STATUS_MONSTER_ATTACK;
					//m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
					//m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + MONSTER_ATTACK_TIME;
					//ClearStatusNoTimer();
          //m_dwEnd[MST_STAND_ACT] = MonsterTickCount + 500 + gf_GetRandom( 2000 );
          m_dwEnd[MST_ATTACK_ACT]		=   MonsterTickCount +
                                        MONSTER_ATTACK_TIME +
                                        m_pUsingSkill->GetBeforeDelay() +
                                        m_pUsingSkill->GetAfterDelay();
				}
				// 目标已被打死, 重新切换状态
				else if( iReval == -2 || iReval == -3 )
				{
					ClearStatus();
				}
				// 目标为空, 需要重新寻找目标
				else if( iReval == -1 )
				{
					ClearSimStatus();
				}
			}
			return;
		}
		break;
	// 追击目标时寻路, 捡骨时寻路
	case STATUS_MONSTER_PATHFIND:
		{
			// 巡逻
			//if( m_iAddiStatus == ATTACK_STATUS_MONSTER_WANDER )
			//{
			//	
			//}
			// 追杀目标
			//else
			if( m_iAddiStatus == ATTACK_STATUS_MONSTER_PURSUE )
			{
				// 检查是否需要寻路
				m_wTargetDis = GetDistance( m_pLastTarget );
				if( m_wTargetDis <= m_iActiveRange )//&& ( m_pInMap->GetTileFlag( m_iX, m_iY ) & TILE_ALLOCCULDE ) )
				{
					// 切换到攻击状态
					m_iNpcStatus  = STATUS_MONSTER_ATTACK;
					m_iNextStatus = STATUS_MONSTER_ATTACK;
					m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
					m_dwEnd[MST_ATTACK_ACT]	= 0;
          //
          if( m_iX != m_iEndX && m_iY != m_iEndY )    SetPathEnd( m_iX, m_iY );
					return;
				}
				else if( m_wTargetDis != 0x7FFFFFFF )
				{
					// 寻路
					if( PathFinding( m_pLastTarget ) )		return;
				}
        else
        {
          ClearStatus();
          return;
        }
			}
			// 捡骨
			else if( m_iAddiStatus == ATTACK_STATUS_MONSTER_COLLECT )
			{
				if( GetFirstFoundItem() )
				{
					// 查看道具与怪物的距离是否需要寻路
					m_wTargetDis = GetDistance( m_wTargetItemX, m_wTargetItemY );
					if( m_wTargetDis < 2 )
					{ // 直接捡起道具
						m_iNpcStatus  = STATUS_MONSTER_COLLECT;
						m_iNextStatus = STATUS_MONSTER_STAND;
						m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
						return;
					}
					else if( m_wTargetDis != 0x7FFFFFFF )
					{ // 寻路
						if( PathFinding( m_wTargetItemX, m_wTargetItemY ) )  	return;
					}
          else
          {
            ClearStatus();
            return;
          }
				}
			}
			// 辅助支援
			else if( m_iAddiStatus == ATTACK_STATUS_MONSTER_ASSAID )
			{
				if( GetTeamerDistance() <= m_iActiveRange )
				{
					m_iNpcStatus  = STATUS_MONSTER_AID;
					m_iNextStatus = STATUS_MONSTER_STAND;
					return;
				}
				else
				{
					if( PathFinding( m_pLastTeamer ) )
					{
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_SHOW_MONSTER_INFO_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
						_snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s Get Aid Teamer Path In Map(%d) #", m_pBase->m_szName, m_dwMapId );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
						AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
						return;
					}
				}
			}
			// 清空状态, 重新循环
			ClearStatus();
			return;
		}
		break;
	// 受伤状态
	case STATUS_MONSTER_INJURED:
		{
			// 受伤状态
			if( MonsterTickCount > m_dwEnd[MST_INJURED_ACT] )
			{
				m_iNpcStatus  = m_iNextStatus;
				m_iNextStatus = STATUS_MONSTER_STAND;
        m_dwEnd[MST_ATTACK_ACT] += 200;
        if( m_iActiveRange < 1 )
        {
          if( m_iNpcStatus == STATUS_MONSTER_PATHFIND && m_iAddiStatus == ATTACK_STATUS_MONSTER_PURSUE )
          {
            ChooseSkill();
          }
        }
			}
			return;
		}
		break;
	// 死亡计算经验值
	case STATUS_MONSTER_GOTO_DEAD:
		{
			// Compute Exp And Send To Client
			GoToDied();
		}
		break;
	// 死亡状态
	case STATUS_MONSTER_DEAD:
		{
			// 死亡, 等待复活
			if( MonsterTickCount > m_dwEnd[MST_DEAD_ACT] )
			{
				if( m_iAddiStatus == ATTACK_STATUS_MONSTER_DROPITEM )
				{
					DropItem();
					// Clear Attack Data, Exp Data
					ClearHitData();
          //
					m_iAddiStatus = ATTACK_STATUS_MONSTER_DEAD;
          m_dwEnd[MST_DEAD_ACT] = MonsterTickCount + m_pBase->m_dwReviveTime;
				}
				else
				{
					Revive();
				}
			}
			return;
		}
		break;
	// 防御, 使用道具
	case STATUS_MONSTER_DEFENCE:
		{
			// 使用补充类道具
			
		}
		break;
	// 捡骨
	case STATUS_MONSTER_COLLECT:
		{
			// 捡骨动作
			iReval = PickUpGItem();
			// 成功捡起道具
			if( iReval == 1 )
			{
				//m_wTargetItemX = HIWORD( (*m_listGroundItemPos.begin()) );
				//m_wTargetItemY = LOWORD( (*m_listGroundItemPos.begin()) );
				m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
				m_iNextStatus = STATUS_MONSTER_STAND;
				m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
			}
			// 无法捡起地上物品, 已经被捡走 
			else if( iReval == -1 )
			{
				// 没有已经找到的道具位置, 切换状态至站立
				if( m_listGroundItemPos.empty() )
				{
					ClearStatus();
				}
				else
				{
					m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
					m_iNextStatus = STATUS_MONSTER_STAND;
					m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
				}
			}
			// 自动消化一样道具
			else if( iReval == 0 )
			{
				if( m_listGroundItemPos.empty() )
				{
					ClearStatus();
				}
				else
				{
					m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
					m_iNextStatus = STATUS_MONSTER_STAND;
					m_iAddiStatus = ATTACK_STATUS_MONSTER_COLLECT;
				}
			}
			return;
		}
		break;
	// 逃跑
	case STATUS_MONSTER_ESCAPE:
		{
			if( !Escape() )
			{
				// Set Next Status
				m_iNpcStatus  = STATUS_MONSTER_STAND;
				m_iNextStatus = STATUS_MONSTER_STAND;
			}
			return;
		}
		break;
	// 行走时遇到障碍或是其他状况, 被迫停止
	case STATUS_MONSTER_STOP:
		{
			if( MonsterTickCount > m_dwEnd[MST_STOP_ACT] )
			{
				if( m_wStopTimes > 2 )
				{
					ClearStatus();
					m_wStopTimes = 0;
				}
				//ClearStatusNoTimer();
				m_iNpcStatus  = m_iNextStatus;
				m_iNextStatus = STATUS_MONSTER_STAND;
				m_wStopTimes++;
				return;
			}
			// Try Do What Monster Already Do
			// ...
		}
		break;
	// 援助准备
	case STATUS_MONSTER_AIDREADY:
		{
			// Check Aid Limit And Aid Type, How To Aid
			// 战斗支援
			if( m_pBase->m_iNpcProperty & NPC_ATTRI_ATTACKAID )
			{
				if( ChooseSkill() > 0 )
				{
					// 找到可攻击玩家
					m_wTargetDis = GetDistance( m_pLastTarget );
					if( m_wTargetDis < m_iActiveRange )
					{
						// 切换到攻击状态
						m_iNpcStatus  = STATUS_MONSTER_ATTACK;
						m_iNextStatus = STATUS_MONSTER_ATTACK;
						m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
						m_dwEnd[MST_ATTACK_ACT]	= 0;
            if( m_iX != m_iEndX && m_iY != m_iEndY )    SetPathEnd( m_iX, m_iY );
					}
					else if( m_wTargetDis != 0x7FFFFFFF )
					{
						// 切换状态到寻路
						m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
						m_iNextStatus = STATUS_MONSTER_ATTACK;
						m_iAddiStatus = ATTACK_STATUS_MONSTER_PURSUE;
					}
          else
          {
            ClearStatus();
          }
					return;
				}
				ClearStatusNoTimer();
				return;
			}
			// 辅助支援
			else if( m_pBase->m_iNpcProperty & NPC_ATTRI_ASSIAID )
			{
				if( ( m_iActiveRange = GetAssistantSkill() ) > 0 )
				{
					if( GetTeamerDistance() <= m_iActiveRange )
					{
						m_iNpcStatus  = STATUS_MONSTER_AID;
						m_iAddiStatus = ATTACK_STATUS_MONSTER_ASSAID;
						m_dwEnd[MST_ATTACK_ACT]	= 0;
						return;
					}
					else
					{
						m_iNpcStatus  = STATUS_MONSTER_PATHFIND;
						m_iAddiStatus = ATTACK_STATUS_MONSTER_ASSAID;
						return;
					}
				}
				ClearStatus();
				return;
			}
			else
			{
				// Set Next Status
			}
		}
		break;
	case STATUS_MONSTER_GOHOME:
		if( MonsterTickCount > m_dwEnd[MST_STAND_ACT] )
		{
			GoOutTile();
      //
      WORD          XXX = m_iX, YYY = m_iY;
			// Random X, Y
			for( int z = 0; z < 3; z++ )
			{
				m_iX = gf_GetRandom( 4 ) - gf_GetRandom( 4 );
				m_iY = gf_GetRandom( 4 ) - gf_GetRandom( 4 );
				if( !( m_pInMap->GetTileFlag( m_iInitX + m_iX, m_iInitY + m_iY ) & TILE_ALLOCCULDE ) )	break;
				else if( z == 2 )
				{
					m_iX = -1 + gf_GetRandom( 2 );
					m_iY = -1 + gf_GetRandom( 2 );
				}
			}
      //
			m_iX += m_iInitX;		m_iY += m_iInitY;
			SetPathEnd( m_iX, m_iY );
			GoInTile();
			// Warp 回家
			*(WORD*)(MsgGoHome.Msgs[0].Data)       = GetSelfCode();
			*(((WORD*)(MsgGoHome.Msgs[0].Data))+1) = m_iX;
			*(((WORD*)(MsgGoHome.Msgs[0].Data))+2) = m_iY;
      m_pInMap->SendMsgNearPosition( MsgGoHome, XXX, YYY );
			// Clear Fight Data
			ClearHitData();
			// Set Next Status And Interval
      ClearStatus();
		}
		break;
	// 援助同组怪物
	case STATUS_MONSTER_AID:
		{
			// 战斗支援
			//if( m_iAddiStatus == ATTACK_STATUS_MONSTER_ATTACKAID )
			//{
			//	
			//}
			// 辅助支援
			//else
			if( m_iAddiStatus == ATTACK_STATUS_MONSTER_ASSAID )
			{
				// Now Can Attack Target, Then Do It
				if( GetTeamerDistance() <= m_iActiveRange )
				{
					iReval = AidTeamer();
					// 援助成功
					if( iReval == 1 )
					{
						m_dwEnd[MST_ATTACK_ACT]		= MonsterTickCount + MONSTER_ATTACK_TIME;
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_SHOW_MONSTER_INFO_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
						_snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s Assistant Teamer In Map(%d) #", m_pBase->m_szName, m_dwMapId );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
						AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
					}
					// 没有援助招式, 取消援助
					else if( iReval == 0 )
					{
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_SHOW_MONSTER_INFO_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
						_snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s Assistant Teamer Failed Because No Skill In Map(%d) #", m_pBase->m_szName, m_dwMapId );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
						AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
						ClearStatus();
					}
					// 目标为空, 需要重新寻找目标
					else if( iReval == -1 )
					{
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG_SHOW_MONSTER_INFO_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
						_snprintf( g_szMonsterLog, MAX_MEMO_MSG_LEN-1, "%s Assistant Teamer Failed Because No Target In Map(%d) #", m_pBase->m_szName, m_dwMapId );
            g_szMonsterLog[MAX_MEMO_MSG_LEN-1] = '\0';
						AddMemoMsg( g_szMonsterLog );
#endif
#endif
#endif
						ClearStatus();
					}
				}
				return;
			}
			// 切换状态
			ClearStatus();
			return;
		}
		break;
  case STATUS_MONSTER_CHANGE_TARGET:
    if( IsBoss() )
    {
      m_pLastTarget		= NULL;
		  m_dwTargetCode	= 0;
		  m_iNpcStatus		= STATUS_MONSTER_STAND;
		  m_iNextStatus   = STATUS_MONSTER_STAND;
		  m_iAddiStatus   = ATTACK_STATUS_MONSTER_SEARCH;
		  m_iStepCount    = 0;
      m_iCurrSight    = 0;
      m_dwEnd[MST_STAND_ACT] = 0;
    }
    else  ClearStatus();
    break;
  default:
    break;
	}
}
//====================================================================================================================================
//
//
inline void CMonster::CheckTimer()
{
	static CSrvBaseSkill				*pFuncSkill;
	static int									iSpecialCount = 0;

	// Check Short Timer - 1s
	if( ClientTickCount > m_dwCheckTimerNow1 )
	{
    SNMSetSpecialStatus	*pSetSpecial = (SNMSetSpecialStatus*)(MsgMstCheckTimer.Msgs[0].Data);
    iSpecialCount = 0;
    //===============================================
    // Check Special Status
    //
		// 增加(减少)Hp -- Needless Send All
		if( m_qwSpecialStatus & SPE_STATUS_HP )
		{
			if( m_wFuncTrigger[17] )
			{
				if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncTrigger[17] ) ) )
				{
					pFuncSkill->Heal( this, NULL );
				}
			}
			if( ClientTickCount > m_dwAlarmTime[17] )
			{
        m_qwSpecialStatus &= ~SPE_STATUS_HP;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[17];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[17]   = 0;
        m_wFuncTrigger[17]  = 0;
        m_wFuncAlarm[17]    = 0;
			}
		}
		// 增加(减少)Mp -- Needless Send All
		if( m_qwSpecialStatus & SPE_STATUS_MP )
		{
			if( m_wFuncTrigger[19] )
			{
				if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncTrigger[19] ) ) )
				{
					pFuncSkill->Heal( this, NULL );
				}
			}
			if( ClientTickCount > m_dwAlarmTime[19] )
			{
        m_qwSpecialStatus &= ~SPE_STATUS_MP;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[19];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[19]   = 0;
        m_wFuncTrigger[19]  = 0;
        m_wFuncAlarm[19]    = 0;
			}
		}
		// 增加(减少)Sp -- Needless Send All
		if( m_qwSpecialStatus & SPE_STATUS_SP )
		{
			if( m_wFuncTrigger[21] )
			{
				if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncTrigger[21] ) ) )
				{
					pFuncSkill->Heal( this, NULL );
				}
			}
			if( ClientTickCount > m_dwAlarmTime[21] )
			{
        m_qwSpecialStatus &= ~SPE_STATUS_SP;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[21];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[21]   = 0;
        m_wFuncTrigger[21]  = 0;
        m_wFuncAlarm[21]    = 0;
			}
		}
		// 中毒
		if( m_qwSpecialStatus & SPE_STATUS_POISON )
		{
			if( m_wFuncTrigger[9] )
			{
				if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncTrigger[9] ) ) )
				{
					pFuncSkill->Heal( this, NULL );
				}
			}
			if( m_dwAlarmTime[9] < ClientTickCount )
      {
        m_qwSpecialStatus &= ~SPE_STATUS_POISON;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[9];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[9]   = 0;
        m_wFuncTrigger[9]  = 0;
        m_wFuncAlarm[9]    = 0;
      }
		}
		// 增加(减少)攻击力 -- Needless Send All
		if( m_qwSpecialStatus & SPE_STATUS_AP )
		{
			if( m_dwAlarmTime[3] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[3] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_AP;
        //
        //pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[3];
        //pSetSpecial->qwStatus     = m_qwSpecialStatus;
        //pSetSpecial++;
        //iSpecialCount++;
        //
        m_dwAlarmTime[3]   = 0;
        m_wFuncTrigger[3]  = 0;
        m_wFuncAlarm[3]    = 0;
      }
		}
		// 增加(减少)防御力 -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_DP )
		{
			if( m_dwAlarmTime[5] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[5] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_DP;
        //
        //pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[5];
        //pSetSpecial->qwStatus     = m_qwSpecialStatus;
        //pSetSpecial++;
        //iSpecialCount++;
        //
        m_dwAlarmTime[5]   = 0;
        m_wFuncTrigger[5]  = 0;
        m_wFuncAlarm[5]    = 0;
      }
		}
		// 增加(减少)致命一击 -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_CRITICAL )
		{
			if( m_dwAlarmTime[8] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[8] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_CRITICAL;
        //
        //pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[8];
        //pSetSpecial->qwStatus     = m_qwSpecialStatus;
        //pSetSpecial++;
        //iSpecialCount++;
        //
        m_dwAlarmTime[8]   = 0;
        m_wFuncTrigger[8]  = 0;
        m_wFuncAlarm[8]    = 0;
      }
		}
		// 增加(减少)命中率 -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_HIT )
		{
			if( m_dwAlarmTime[4] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[4] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_HIT;
        //
        //pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[4];
        //pSetSpecial->qwStatus     = m_qwSpecialStatus;
        //pSetSpecial++;
        //iSpecialCount++;
        //
        m_dwAlarmTime[4]   = 0;
        m_wFuncTrigger[4]  = 0;
        m_wFuncAlarm[4]    = 0;
      }
		}
		// 增加速度 -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_HIGHSPEED )
		{
			if( m_dwAlarmTime[2] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[2] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_HIGHSPEED;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[2];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[2]   = 0;
        m_wFuncTrigger[2]  = 0;
        m_wFuncAlarm[2]    = 0;
      }
		}
		// 增加(减少)躲闪率 -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_DODGE )
		{
			if( m_dwAlarmTime[6] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[6] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_DODGE;
        //
        //pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[6];
        //pSetSpecial->qwStatus     = m_qwSpecialStatus;
        //pSetSpecial++;
        //iSpecialCount++;
        //
        m_dwAlarmTime[6]   = 0;
        m_wFuncTrigger[6]  = 0;
        m_wFuncAlarm[6]    = 0;
      }
		}
		// 增加(减少)智力 -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_INT )
		{
			if( m_dwAlarmTime[7] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[7] ) ) )
        {
          pFuncSkill->ChangeChar( this );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_INT;
        //
        //pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[7];
        //pSetSpecial->qwStatus     = m_qwSpecialStatus;
        //pSetSpecial++;
        //iSpecialCount++;
        //
        m_dwAlarmTime[7]   = 0;
        m_wFuncTrigger[7]  = 0;
        m_wFuncAlarm[7]    = 0;
      }
		}
		// 增加(减少)MaxHp -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_MAXHP )
		{
			if( m_dwAlarmTime[16] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[16] ) ) )
        {
          pFuncSkill->Heal( this, NULL );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_MAXHP;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[16];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[16]   = 0;
        m_wFuncTrigger[16]  = 0;
        m_wFuncAlarm[16]    = 0;
      }
		}
		// 增加(减少)MaxMp -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_MAXMP )
		{
			if( m_dwAlarmTime[18] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[18] ) ) )
        {
          pFuncSkill->Heal( this, NULL );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_MAXMP;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[18];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[18]   = 0;
        m_wFuncAlarm[18]    = 0;
        m_wFuncTrigger[18]  = 0;
      }
		}
		// 增加(减少)MaxSp -- Needless Send All
		else if( m_qwSpecialStatus & SPE_STATUS_MAXSP )
		{
			if( m_dwAlarmTime[20] < ClientTickCount )
      {
        if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[20] ) ) )
        {
          pFuncSkill->Heal( this, NULL );
        }
        m_qwSpecialStatus &= ~SPE_STATUS_MAXSP;
        //
        pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[20];
        pSetSpecial->qwStatus     = m_qwSpecialStatus;
        pSetSpecial++;
        iSpecialCount++;
        //
        m_dwAlarmTime[20]   = 0;
        m_wFuncAlarm[20]    = 0;
        m_wFuncTrigger[20]  = 0;
      }
		}
		m_dwCheckTimerNow1 = ClientTickCount + CHECK_TIMER_INTERVAL_SHORT;
		// Check Longer Timer -- 5ms
		if( ClientTickCount > m_dwCheckTimerNow2 )
		{
			// 不能移动
			if( m_qwSpecialStatus & SPE_STATUS_UNMOVE )
			{
				if( ClientTickCount > m_dwAlarmTime[11] )
				{
					m_qwSpecialStatus &= ~SPE_STATUS_UNMOVE;
          //
          pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[11];
          pSetSpecial->qwStatus     = m_qwSpecialStatus;
          pSetSpecial++;
          iSpecialCount++;
          //
					m_dwAlarmTime[11]  = 0;
					m_wFuncAlarm[11]   = 0;
          m_wFuncTrigger[11] = 0;
				}
			}
			// 盲
			if( m_qwSpecialStatus & SPE_STATUS_BLIND )
			{
				if( ClientTickCount > m_dwAlarmTime[10] )
				{
					m_qwSpecialStatus &= ~SPE_STATUS_BLIND;
          //
          pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[10];
          pSetSpecial->qwStatus     = m_qwSpecialStatus;
          pSetSpecial++;
          iSpecialCount++;
          //
					m_dwAlarmTime[10]  = 0;
					m_wFuncAlarm[10]   = 0;
          m_wFuncTrigger[10] = 0;
				}
			}
			// 不能攻击
			if( m_qwSpecialStatus & SPE_STATUS_UNATTACK )
			{
				if( ClientTickCount > m_dwAlarmTime[12] )
				{
					m_qwSpecialStatus &= ~SPE_STATUS_UNATTACK;
          //
          pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[12];
          pSetSpecial->qwStatus     = m_qwSpecialStatus;
          pSetSpecial++;
          iSpecialCount++;
          //
					m_dwAlarmTime[12]  = 0;
					m_wFuncAlarm[12]   = 0;
          m_wFuncTrigger[12] = 0;
				}
			}
			// 隐身
			if( m_qwSpecialStatus & SPE_STATUS_UNSEE )
			{
				if( ClientTickCount > m_dwAlarmTime[14] )
				{
					m_qwSpecialStatus &= ~SPE_STATUS_UNSEE;
          //
          pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[14];
          pSetSpecial->qwStatus     = m_qwSpecialStatus;
          pSetSpecial++;
          iSpecialCount++;
          //
					m_dwAlarmTime[14]  = 0;
					m_wFuncAlarm[14]   = 0;
          m_wFuncTrigger[14] = 0;
				}
			}
			// 不能使用道具
			if( m_qwSpecialStatus & SPE_STATUS_UNITEM )
			{
				if( ClientTickCount > m_dwAlarmTime[13] )
				{
					m_qwSpecialStatus &= ~SPE_STATUS_UNITEM;
          //
          pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[13];
          pSetSpecial->qwStatus     = m_qwSpecialStatus;
          pSetSpecial++;
          iSpecialCount++;
          //
					m_dwAlarmTime[13]  = 0;
					m_wFuncAlarm[13]   = 0;
          m_wFuncTrigger[13] = 0;
				}
			}
			// 侦测隐身 -- Needless Send All
			if( m_qwSpecialStatus & SPE_STATUS_SEE )
			{
				if( ClientTickCount > m_dwAlarmTime[15] )
				{
					m_qwSpecialStatus &= ~SPE_STATUS_SEE;
          //
          pSetSpecial->dwCode_Skill = ( GetCode() << 16 ) | m_wFuncAlarm[15];
          pSetSpecial->qwStatus     = m_qwSpecialStatus;
          pSetSpecial++;
          iSpecialCount++;
          //
					m_dwAlarmTime[15]  = 0;
					m_wFuncAlarm[15]   = 0;
          m_wFuncTrigger[15] = 0;
				}
			}
			// 改变Monster五行属性 -- Needless Send All
			if( m_qwSpecialStatus & SPE_STATUS_ELEMENT )
			{
				if( ClientTickCount > m_dwAlarmTime[22] )
				{
          m_wElement         = m_wElemBackup;
          //
					m_qwSpecialStatus &= ~SPE_STATUS_ELEMENT;
					m_dwAlarmTime[22]  = 0;
					m_wFuncAlarm[22]   = 0;
          m_wFuncTrigger[22] = 0;
				}
			}
      // 霸体
      if( m_qwSpecialStatus & SPE_STATUS_UNINJURED )
      {
				if( ClientTickCount > m_dwAlarmTime[25] )
				{
					if( NULL != ( pFuncSkill = g_pBase->GetBaseSkill( m_wFuncAlarm[25] ) ) )
          {
            pFuncSkill->ChangeBearPos( this, NULL );
          }
          m_qwSpecialStatus &= ~SPE_STATUS_UNINJURED;
					m_dwAlarmTime[25]  = 0;
					m_wFuncAlarm[25]   = 0;
          m_wFuncTrigger[25] = 0;
				}
      }
			m_dwCheckTimerNow2 = ClientTickCount + CHECK_TIMER_INTERVAL_LONG;
		}
		if( iSpecialCount && m_pInMap != NULL )
		{
			MsgMstCheckTimer.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * iSpecialCount;
			m_pInMap->SendMsgNearPosition( MsgMstCheckTimer, m_iX, m_iY );
      iSpecialCount = 0;
		}
	}
}
//============================================================================================
//
//Functions:
void DeleteAllUnreviveMonster()
{
  static CMonster                     *g_pRMonster;
  static list<CMonster*>::iterator    Iter_MRvv;

  if( !g_ListUnreviveMonster.empty() )
  {
    for( Iter_MRvv = g_ListUnreviveMonster.begin(); Iter_MRvv != g_ListUnreviveMonster.end(); Iter_MRvv++ )
    {
      g_pRMonster = (*Iter_MRvv);
      // 如果该怪物有队伍，要将其从队伍中Erase掉
      if( g_pRMonster->GetTeam() )
      {
        (g_pRMonster->GetTeam())->DelMonster( g_pRMonster );
      }
      // 如果其他地方还有该怪物的指针存在，要将该指针NULL (Npc,Player,Map)
      if( g_pRMonster->GetMercenaryType() )
      {
        CGameMap          *pGameMap = g_pRMonster->GetInMap();
        if( pGameMap )    pGameMap->DelMercenaryCount();
      }
      (g_pRMonster->GetInMap())->DelMonster( g_pRMonster->GetSelfCode() );
      SAFE_DELETE( g_pRMonster );
    }
    g_ListUnreviveMonster.clear();
  }
}
//============================================================================================
//
//Functions:
void CMonster::MonsterWarpInServer( const WORD & X, const WORD & Y )
{
  
}
//============================================================================================
//
//Functions:
void CMonster::InitDeadEvent()
{
  if( m_pBase->m_dwDeadEvent )
  {
    m_pDeadEvent = g_pBase->GetEventPoint( m_pBase->m_dwDeadEvent );
  }
  else
  {
    m_pDeadEvent = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright( C ) 2001 Waei Corporation.  All Rights Reserved.
//
//	Author	: DingDong Lee, Qinming He
//	Desc	  : Player Skill And Magic Data, Compute Attack Judge, Skill Damage, Magic Result
//	Date	  : 2001-4-19
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include<algorithm>
#include "stdafx.h"
#include "SrvSkill.h"
#include "assert.h"
#include "../Item/srvItem.h"
#include "../Life/SrvPlayer.h"
#include "../Life/SrvMonster.h"
#include "../../Main/SrvGsData.h"
#include "../Guild/SrvGuild.h"

#ifdef _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
extern int g_iDamageCount;
#endif

#define SKILL_INCREASE			    2
#define SKILL_INCREASE_PERCENT	0
#define SKILL_DECREASE			    3
#define SKILL_DECREASE_PERCENT	1

enum WARP_CASE
{
	WARP_APPOINT_POS = 1,
	WARP_RANDOM_POS,
	WARP_RELIVE_POS,
	WARP_REMEMBER_POS,
	WARP_BASE_VAR,
  WARP_REVIVE_POS,
};

enum TARGET_TYPE
{
	TARGET_SELF,
	TARGET_TEAM,
	TARGET_OBJ,
	TARGET_NONE,
};

extern CGsData *g_pGs;

enum
{
  ET_ERROR,
  ET_NULL = 1,
  ET_GROW,
  ET_JINX,
  ET_JXED,
  ET_SAME,
};

BYTE g_EleTable[17][17] = 
{
  // none    wood      fire        earth             metal                  water
  { ET_NULL, ET_NULL, ET_NULL, 0, ET_NULL, 0, 0, 0, ET_NULL, 0,0,0,0,0,0,0, ET_NULL }, //none
  { ET_NULL, ET_NULL, ET_GROW, 0, ET_JINX, 0, 0, 0, ET_JXED, 0,0,0,0,0,0,0, ET_NULL }, //wood
  { ET_NULL, ET_NULL, ET_NULL, 0, ET_GROW, 0, 0, 0, ET_JINX, 0,0,0,0,0,0,0, ET_JXED }, //fire
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { ET_NULL, ET_JXED, ET_NULL, 0, ET_NULL, 0, 0, 0, ET_GROW, 0,0,0,0,0,0,0, ET_JINX }, //earth
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { ET_NULL, ET_JINX, ET_JXED, 0, ET_NULL, 0, 0, 0, ET_NULL, 0,0,0,0,0,0,0, ET_GROW }, //metal
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0, 0 }, 
  { ET_NULL, ET_GROW, ET_JINX, 0, ET_JXED, 0, 0, 0, ET_NULL, 0,0,0,0,0,0,0, ET_NULL }, //water
};

BYTE g_iSword_HitBonus[10][10] =
{
  // A1, B1, C1, A2, B2, C2, A3, B3, C3, SP 
  {   0,  1,  0,  0,  1,  0,  0,  0,  0,  0  }, // A1
  {   0,  0,  1,  1,  0,  1,  0,  0,  0,  0  }, // B1     <---|
  {   0,  0,  0,  0,  0,  1,  0,  0,  0,  1  }, // C1         |
  {   0,  0,  0,  0,  1,  0,  0,  1,  0,  0  }, // A2         |
  {   0,  0,  0,  0,  0,  1,  1,  0,  1,  0  }, // B2         |前一招的招式
  {   0,  0,  0,  0,  0,  0,  0,  0,  1,  1  }, // C2					|
  {   0,  0,  0,  0,  0,  0,  0,  1,  0,  0  }, // A3        
  {   0,  0,  0,  0,  0,  0,  0,  0,  1,  0  }, // B3
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  1  }, // C3
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0  }  // SP    
};

BYTE g_iBlade_HitBonus[10][10] =
{
  // A1, B1, C1, A2, B2, C2, A3, B3, C3, SP  <----当前的出招招式
  {   0,  1,  0,  0,  1,  0,  0,  0,  0,  0  }, // A1
  {   0,  0,  1,  1,  0,  0,  0,  0,  0,  0  }, // B1
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  1  }, // C1
  {   0,  0,  0,  0,  1,  0,  0,  1,  0,  0  }, // A2
  {   0,  0,  0,  0,  0,  1,  1,  0,  0,  0  }, // B2
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  1  }, // C2
  {   0,  0,  0,  0,  0,  0,  0,  1,  0,  0  }, // A3
  {   0,  0,  0,  0,  0,  0,  0,  0,  1,  0  }, // B3
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  1  }, // C3
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0  }  // SP
};

BYTE g_iPike_HitBonus[10][10] =
{
  // A1, B1, C1, A2, B2, C2, A3, B3, C3, SP 
  {   0,  1,  0,  0,  1,  0,  0,  0,  0,  0  }, // A1
  {   0,  0,  1,  0,  0,  0,  0,  0,  0,  0  }, // B1
  {   0,  0,  0,  0,  0,  1,  0,  0,  0,  1  }, // C1
  {   0,  0,  0,  0,  1,  0,  0,  1,  0,  0  }, // A2
  {   0,  0,  0,  0,  0,  1,  0,  0,  0,  0  }, // B2
  {   0,  0,  0,  0,  0,  0,  0,  0,  1,  1  }, // C2
  {   0,  0,  0,  0,  0,  0,  0,  1,  0,  0  }, // A3
  {   0,  0,  0,  0,  0,  0,  0,  0,  1,  0  }, // B3
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  1  }, // C3
  {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0  }  // SP
};
// Index 1 = Metal  0
// Index 2 = Wood		1
// Index 4 = Water	2
// Index 8 = Fire		3
// Index 16 = Earth	4
// 用于计算招式相生的全局表
int g_iEleUp[17] = 
{
	ELEMENT_ERROR,
		// water <--(生)-- metal
	ELEMENT_FIRE,//Wood
	ELEMENT_EARTH,//Fire
	ELEMENT_ERROR,
	ELEMENT_METAL,//Earth
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_WATER,//Metal
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_WOOD,//Water
};

// 用于计算招式相克的全局表
int g_iEleDown[17] =
{
	ELEMENT_ERROR,
		// metal --(克)--> wood
	ELEMENT_METAL,//Wood
	ELEMENT_WATER,//Fire
	ELEMENT_ERROR,
	ELEMENT_WOOD,//Earth
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_FIRE,//Metal
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_ERROR,
	ELEMENT_EARTH,//Water
};
// 陷阱相关的攻击形状 -- 顺着万家的方向
// ================================ 1 X 3
const POINT g_ptTrap1X3[8][3] =
{
  { {-1, 0 }, { 0, 0 }, { 1, 0 }, },
  { {-1,-1 }, { 0, 0 }, { 1, 1 }, },
  { { 0,-1 }, { 0, 0 }, { 0, 1 }, },
  { { 1,-1 }, { 0, 0 }, {-1, 1 }, },
  { {-1, 0 }, { 0, 0 }, { 1, 0 }, },
  { {-1,-1 }, { 0, 0 }, { 1, 1 }, },
  { { 0,-1 }, { 0, 0 }, { 0, 1 }, },
  { { 1,-1 }, { 0, 0 }, {-1, 1 }, },
};
// ================================ 1 X 5
const POINT g_ptTrap1X5[8][5] =
{
  { {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, },
  { {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, },
  { { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, },
  { { 2,-2 }, { 1,-1 }, { 0, 0 }, {-1, 1 }, {-2, 2 }, },
  { {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, },
  { {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, },
  { { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, },
  { { 2,-2 }, { 1,-1 }, { 0, 0 }, {-1, 1 }, {-2, 2 }, },
};
// ================================ 1 X 7
const POINT g_ptTrap1X7[8][7] =
{
  { {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, },
  { {-3,-3 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, },
  { { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, },
  { { 3,-3 }, { 2,-2 }, { 1,-1 }, { 0, 0 }, {-1, 1 }, {-2, 2 }, {-3, 3 }, },
  { {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, },
  { {-3,-3 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, },
  { { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, },
  { { 3,-3 }, { 2,-2 }, { 1,-1 }, { 0, 0 }, {-1, 1 }, {-2, 2 }, {-3, 3 }, },
};
// ================================ 1 X 9
const POINT g_ptTrap1X9[8][9] =
{
  { {-4, 0 }, {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, },
  { {-4,-4 }, {-3,-3 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, },
  { { 0,-4 }, { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, },
  { { 4,-4 }, { 3,-3 }, { 2,-2 }, { 1,-1 }, { 0, 0 }, {-1, 1 }, {-2, 2 }, {-3, 3 }, {-4, 4 }, },
  { {-4, 0 }, {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, },
  { {-4,-4 }, {-3,-3 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, },
  { { 0,-4 }, { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, },
  { { 4,-4 }, { 3,-3 }, { 2,-2 }, { 1,-1 }, { 0, 0 }, {-1, 1 }, {-2, 2 }, {-3, 3 }, {-4, 4 }, },
};
// ================================ 3 X 3
const POINT g_ptTrap3X3[9] =
{
  { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, {-1, 1 }, {-1, 0 },
};
// ================================ 3 X 5
const POINT g_ptTrap3X5[8][15] =
{
  { {-2,-1 }, {-1,-1 }, { 0,-1 }, { 1,-1 }, { 2,-1 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, {-2, 1 }, {-1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, },
  { {-2, 0 }, {-1, 1 }, { 0, 2 }, {-2,-1 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, {-1,-2 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, },
  { {-1,-2 }, {-1,-1 }, {-1, 0 }, {-1, 1 }, {-1, 2 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1,-2 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, },
  { {-2, 0 }, {-1, 1 }, { 0, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 1,-2 }, { 2,-1 }, },
  { {-2,-1 }, {-1,-1 }, { 0,-1 }, { 1,-1 }, { 2,-1 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, {-2, 1 }, {-1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, },
  { {-2, 0 }, {-1, 1 }, { 0, 2 }, { 1, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, },
  { {-1,-2 }, {-1,-1 }, {-1, 0 }, {-1, 1 }, {-1, 2 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1,-2 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, },
  { {-2, 0 }, {-1, 1 }, { 0, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, {-2, 1 }, {-1, 2 }, },
};
// ================================ 3 X 7
const POINT g_ptTrap3X7[8][21] =
{
  { {-3,-1 }, {-2,-1 }, {-1,-1 }, { 0,-1 }, { 1,-1 }, { 2,-1 }, { 3,-1 }, {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, {-3, 1 }, {-2, 1 }, {-1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, },
  { {-2, 0 }, {-1, 1 }, { 0, 2 }, { 1, 3 }, {-2,-1 }, {-1, 0 }, { 0, 1 }, { 1, 2 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, {-1,-2 }, { 0,-1 }, { 1, 0 }, { 2, 1 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 3, 1 }, },
  { {-1,-3 }, {-1,-2 }, {-1,-1 }, {-1, 0 }, {-1, 1 }, {-1, 2 }, {-1, 3 }, { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1,-3 }, { 1,-2 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, },
  { {-3, 1 }, {-2, 2 }, {-1, 3 }, {-2, 1 }, {-1, 2 }, {-2, 0 }, {-1, 1 }, { 0, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 1,-2 }, { 2,-1 }, { 2,-2 }, },
  { {-3,-1 }, {-2,-1 }, {-1,-1 }, { 0,-1 }, { 1,-1 }, { 2,-1 }, { 3,-1 }, {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, {-3, 1 }, {-2, 1 }, {-1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, },
  { {-2, 0 }, {-1, 1 }, { 0, 2 }, {-3,-1 }, {-2,-1 }, {-1, 0 }, { 0, 1 }, { 1, 2 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, {-1,-2 }, { 0,-1 }, { 1, 0 }, { 2, 1 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, {-1,-3 }, },
  { {-1,-3 }, {-1,-2 }, {-1,-1 }, {-1, 0 }, {-1, 1 }, {-1, 2 }, {-1, 3 }, { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1,-3 }, { 1,-2 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, },
  { { 1,-3 }, {-2, 2 }, { 3,-1 }, {-2, 1 }, {-1, 2 }, {-2, 0 }, {-1, 1 }, { 0, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 1,-2 }, { 2,-1 }, { 2,-2 }, },
};
// ================================ 3 X 9
const POINT g_ptTrap3X9[8][27] =
{
  { {-4, 1 }, {-4, 0 }, {-4,-1 }, {-3,-1 }, {-2,-1 }, {-1,-1 }, { 0,-1 }, { 1,-1 }, { 2,-1 }, { 3,-1 }, {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, {-3, 1 }, {-2, 1 }, {-1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 4, 0 }, { 4,-1 }, },
  { {-3,-1 }, {-3,-2 }, {-2,-3 }, {-1,-3 }, {-2, 0 }, {-1, 1 }, { 0, 2 }, { 1, 3 }, {-2,-1 }, {-1, 0 }, { 0, 1 }, { 1, 2 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, {-1,-2 }, { 0,-1 }, { 1, 0 }, { 2, 1 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 3, 1 }, { 2, 3 }, { 3, 2 }, },
  { {-1,-4 }, { 0,-4 }, { 1,-4 }, {-1,-3 }, {-1,-2 }, {-1,-1 }, {-1, 0 }, {-1, 1 }, {-1, 2 }, {-1, 3 }, { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1,-3 }, { 1,-2 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, {-1, 4 }, { 0, 4 }, { 1, 4 }, },
  { { 1,-3 }, { 2,-3 }, { 3,-2 }, { 3,-1 }, {-3, 1 }, {-2, 2 }, {-1, 3 }, {-2, 1 }, {-1, 2 }, {-2, 0 }, {-1, 1 }, { 0, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 1,-2 }, { 2,-1 }, { 2,-2 }, {-3, 2 }, {-2, 3 }, },
  { {-4, 1 }, {-4, 0 }, {-4,-1 }, {-3,-1 }, {-2,-1 }, {-1,-1 }, { 0,-1 }, { 1,-1 }, { 2,-1 }, { 3,-1 }, {-3, 0 }, {-2, 0 }, {-1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, {-3, 1 }, {-2, 1 }, {-1, 1 }, { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 }, { 4, 1 }, { 4, 0 }, { 4,-1 }, },
  { { 3, 1 }, { 3, 2 }, { 2, 3 }, { 1, 3 }, {-2, 0 }, {-1, 1 }, { 0, 2 }, {-3,-1 }, {-2,-1 }, {-1, 0 }, { 0, 1 }, { 1, 2 }, {-2,-2 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 2, 2 }, {-1,-2 }, { 0,-1 }, { 1, 0 }, { 2, 1 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, {-1,-3 }, {-2,-3 }, {-3,-2 }, },
  { {-1,-4 }, { 0,-4 }, { 1,-4 }, {-1,-3 }, {-1,-2 }, {-1,-1 }, {-1, 0 }, {-1, 1 }, {-1, 2 }, {-1, 3 }, { 0,-3 }, { 0,-2 }, { 0,-1 }, { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1,-3 }, { 1,-2 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, {-1, 4 }, { 0, 4 }, { 1, 4 }, },
  { {-3, 1 }, {-3, 2 }, {-1, 3 }, {-2, 3 }, { 1,-3 }, {-2, 2 }, { 3,-1 }, {-2, 1 }, {-1, 2 }, {-2, 0 }, {-1, 1 }, { 0, 2 }, {-1, 0 }, { 0, 1 }, {-1,-1 }, { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1, 0 }, { 0,-2 }, { 1,-1 }, { 2, 0 }, { 1,-2 }, { 2,-1 }, { 2,-2 }, { 2,-3 }, { 3,-2 }, },
};
// ================================ 5 X 5
const POINT g_ptTrap5X5[25] =
{
  { 0, 0 }, { 1, 1 }, { 0,-1 }, { 1,-1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, {-1, 1 }, {-1, 0 }, {-2,-2 }, {-1,-2 }, { 0,-2 }, { 1,-2 }, { 2,-2 }, { 2,-1 }, { 2, 0 }, { 2, 1 }, { 2, 2 }, { 1, 2 }, { 0, 2 }, {-1, 2 }, {-2, 2 }, {-2, 1 }, {-2, 0 }, {-2,-1 },
};
// About Base Skill Target List
list<CLife*>			g_BaseTargetList;
static char				g_szDamageLog[MAX_MEMO_MSG_LEN];
//**************************************************************************************
// 
//    用于 checkHit（检查攻击范围） 的全局表
//
//**************************************************************************************
const POINT g_ptOffsetLineD[8][9]=
{
	{		{0,0},		{-1, 0},		{-2, 0},		{-3, 0},		{-4, 0},		{-5, 0},		{-6, 0},		{-7, 0},	  {-8, 0},		},//LEFT_UP
	{		{0,0},		{-1,-1},	  {-2,-2},	  {-3,-3},	  {-4,-4},	  {-5,-5},	  {-6,-6},	  {-7,-7},    {-8,-8},		},//UP
  {		{0,0},		{ 0,-1},		{ 0,-2},		{ 0,-3},		{ 0,-4},		{ 0,-5},		{ 0,-6},		{ 0,-7},	  { 0,-8},		},//RIGHT_UP
  {		{0,0},		{ 1,-1},		{ 2,-2},		{ 3,-3},		{ 4,-4},		{ 5,-5},		{ 6,-6},		{ 7,-7},	  { 8,-8},		},//RIGHT
  {		{0,0},		{ 1, 0},		{ 2, 0},		{ 3, 0},		{ 4, 0},		{ 5, 0},		{ 6, 0},		{ 7, 0},	  { 8, 0},		},//RIGHT_DOWN
  {		{0,0},		{ 1, 1},		{ 2, 2},		{ 3, 3},		{ 4, 4},		{ 5, 5},		{ 6, 6},		{ 7, 7},  	{ 8, 8},  	},//DOWN
  {		{0,0},		{ 0, 1},		{ 0, 2},		{ 0, 3},		{ 0, 4},		{ 0, 5},		{ 0, 6},		{ 0, 7},	  { 0, 8},		},//LEFT_DOWN
  {		{0,0},		{-1, 1},		{-2, 2},		{-3, 3},		{-4, 4},		{-5, 5},		{-6, 6},		{-7, 7},	  {-8, 8},		},//LEFT
};

const POINT	g_ptOffsetFan[8][8]=
{
  {		{0,0},		{-1, 0},		{-1,-1},	  {-1, 1},		{ 0, 1},		{0,-1},	 { 1,-1},		{ 1, 1},		},//LEFT_UP /*0*/   
  {		{0,0},		{-1,-1},		{ 0,-1},		{-1, 0},		{-1, 1},		{1,-1},	 { 0, 1},		{ 1, 0},		},//UP        /*1*/ 
  {		{0,0},		{ 0,-1},		{-1,-1},		{ 1,-1},		{-1, 0},	  {1, 0},	 {-1, 1},		{ 1, 1},		},//RIGHT_UP/*2*/   
  {		{0,0},		{ 1,-1},		{ 0,-1},		{ 1, 0},		{-1,-1},		{1, 1},	 {-1, 0},		{ 0, 1},		},//RIGHT   /*3*/   
  {		{0,0},		{ 1, 0},		{ 1, 1},		{ 1,-1},	  { 0,-1},	  {0, 1},	 {-1, 1},		{-1,-1},  	},//RIGHT_DOWN/*4*/ 
  {		{0,0},		{ 1, 0},		{ 1, 1},		{ 1,-1},		{-1, 1},	  {1,-1},	 {-1, 0},		{ 0,-1},		},//LEFT  /*5*/           
  {		{0,0},		{ 0, 1},	  {-1, 1},		{ 1, 1},		{-1, 0},	  {1, 0},	 {-1,-1},	  { 1,-1},  	},//DOWN      /*6*/     
  {		{0,0},		{-1, 1},		{-1, 0},		{ 0, 1},		{-1,-1},		{1, 1},	 { 0,-1},	  { 1, 0},		},//LEFT_DOWN /*7*/       
};

const POINT g_ptOffsetLineW[8][19] = 
{
	{		{0,0},		{-1, 0},		{-2, 0},		{-1, 1},		{-1,-1},		{-2, 1},		{-2,-1},	{-3, 0},		{-4, 0},		{-3, 1},	  {-3,-1},		{-4, 1},		{-4,-1},	  {-5,0},		  {-5, 1},		{-5,-1},	  {-6,0},		{-6, 1},		{-6,-1},	  },//LEFT_UP
  {		{0,0},		{-1,-1},		{-2,-2},		{-1, 0},		{ 0,-1},		{-2,-1},		{-1,-2},	{-3,-3},	  {-4,-4},	  {-3,-2},	  {-2,-3},	  {-4,-3},	  {-3,-4},	  {-5,-5},		{-4, -5},		{-5,-4},	  {-6,-6},	{-6, -5},		{-5,-6},	  },//UP
  {		{0,0},		{ 0,-1},		{ 0,-2},		{-1,-1},		{ 1,-1},		{-1,-2},	  { 1,-2},	{ 0,-3},		{ 0,-4},	  {-1,-3},		{ 1,-3},		{-1,-4},	  { 1,-4},		{0,-5}, 		{1, -5},		{-1,-5},	  {0,-6},	  {1, -6},		{-1,-6},	  },//RIGHT_UP
  {		{0,0},		{ 1,-1},		{ 2,-2},		{ 0,-1},		{ 1, 0},		{ 1,-2},		{ 2,-1},	{ 3,-3},		{ 4,-4},		{ 2,-3},		{ 3,-2},		{ 3,-4},		{ 4,-3},		{5,-5}, 		{4, -5},		{5,-4},	    {6,-6},	  {5, -6},		{6,-5},     },//RIGHT
  {		{0,0},		{ 1, 0},		{ 2, 0},		{ 1,-1},		{ 1, 1},		{ 2,-1},		{ 2, 1},	{ 3, 0},		{ 4, 0},		{ 3,-1},		{ 3, 1},		{ 4,-1},		{ 4, 1},		{5,0}, 	  	{5, -1},		{5,1},	    {6,0},	  {6, -1},		{6,1},      },//RIGHT_DOWN
  {		{0,0},		{ 1, 1},	  { 2, 2},		{ 0, 1},		{ 1, 0},		{ 1, 2},		{ 2, 1},	{ 3, 3},		{ 4, 4},		{ 3, 2},		{ 2, 3},		{ 4, 3},		{ 3, 4},		{5,5}, 	  	{5, 4},		  {4,5},	    {6,6},	  {6, 5},		  {5,6},      },//DOWN
  {		{0,0},		{ 0, 1},		{ 0, 2},		{-1, 1},		{ 1, 1},		{-1, 2},	  { 1, 2},	{ 0, 3},		{ 0, 4},		{-1, 3},		{ 1, 3},		{-1, 4},		{ 1, 4},		{0,5}, 	  	{-1, 5},		{1,5},	    {0,6},	  {-1, 6},		{1,6},      },//LEFT_DOWN
  {		{0,0},		{-1, 1},		{-2, 2},		{-1, 0},		{ 0, 1},		{-2, 1},		{-1, 2},	{-3, 3},		{-4, 4},		{-3, 2},		{-2, 3},		{-4, 3},		{-3, 4},		{-5,5}, 	 	{-5, 4},		{-4,5},	    {-6,6},	  {-6, 5},		{-5,6},      },//LEFT
};

const POINT g_ptOffsetLineW_Fan[8][9] = 
{
  {		{0,0},		{-1, 0},		{ 0, 1},	  { 0,-1},		{-1, 1},		{-1,-1},	{-2, 0},		{-2, 1},	{-2,-1},		},//LEFT_UP
  {		{0,0},		{-1,-1},		{-1, 0},    { 0,-1},		{-1, 1},    { 1,-1},	{-2,-2},	  {-2,-1},	{-1,-2},		},//UP
  {		{0,0},		{ 0,-1},		{-1, 0},		{ 1, 0},		{-1,-1},		{ 1,-1},	{ 0,-2},		{-1,-2},	{ 1,-2},		},//RIGHT_UP
  {		{0,0},		{ 1,-1},		{ 0,-1},		{ 1, 0},		{-1,-1},		{ 1, 1},	{ 2,-2},    { 1,-2},	{ 2,-1},		},//RIGHT
  {		{0,0},		{ 1, 0},		{ 0,-1},		{ 0, 1},		{ 1,-1},		{ 1, 1},	{ 2, 0},	  { 2,-1},	{ 2, 1},		},//RIGHT_DOWN
  {		{0,0},		{ 1, 1},	  { 0, 1},		{ 1, 0},		{ 1,-1},		{-1, 1},	{ 2, 2},		{ 1, 2},	{ 2, 1},		},//DOWN
  {		{0,0},		{ 0, 1},		{-1, 0},		{ 1, 0},		{-1, 1},		{ 1, 1},	{ 0, 2},		{-1, 2},	{ 1, 2},		},//LEFT_DOWN
  {		{0,0},		{-1, 1},		{-1, 0},		{ 0, 1},		{-1,-1},		{ 1, 1},	{-2, 2},		{-2, 1},	{-1, 2},		},//LEFT
};

const POINT g_ptOffsetLineW_Round[8][12] = 
{
  {		{0,0},		{-1, 0},		{ 0,-1},		{ 0, 1},		{-1,-1},		{-1, 1},		{ 1, 0},		{ 1, 1},	{ 1,-1},		{-2, 0},		{-2, 1},		{-2,-1},    },//LEFT_UP
  {		{0,0},		{-1,-1},		{-1, 0},		{ 0,-1},		{-1, 1},	  { 1,-1},		{ 0, 1},		{ 1, 0},	{ 1, 1},		{-2,-2},		{-2,-1},		{-1,-2},		},//UP
  {		{0,0},		{ 0,-1},		{-1, 0},		{ 1, 0},		{-1,-1},		{ 1,-1},	  { 0, 1},		{-1, 1},	{ 1, 1},		{ 0,-2},		{-1,-2},		{ 1,-2},		},//RIGHT_UP
  {		{0,0},		{ 1,-1},		{ 1, 0},		{ 0,-1},		{ 1, 1},		{-1,-1},		{-1, 0},		{ 0, 1},	{-1, 1},		{ 2,-2},		{ 1,-2},		{ 2,-1},		},//RIGHT
  {		{0,0},		{ 1, 0},		{ 0, 1},		{ 0,-1},		{ 1, 1},		{ 1,-1},		{-1, 0},		{-1,-1},	{-1, 1},		{ 2, 0},		{ 2,-1},		{ 2, 1},	  },//RIGHT_DOWN
  {		{0,0},		{ 1, 1},	  { 0, 1},		{ 1, 0},		{-1, 1},		{ 1,-1},		{-1, 0},		{ 0,-1},	{-1,-1},	  { 2, 2},	  { 2, 1},		{ 1, 2},		},//DOWN
  {		{0,0},		{ 0, 1},		{-1, 0},		{ 1, 0},		{-1, 1},		{ 1, 1},	  { 0,-1},		{-1,-1},	{ 1,-1},		{ 0, 2},		{ 1, 2},	  {-1, 2},		},//LEFT_DOWN
  {		{0,0},		{-1, 1},		{-1, 0},		{ 0, 1},		{-1,-1},		{ 1, 1},		{ 0,-1},		{ 1, 0},	{ 1,-1},		{-2, 2},		{-2, 1},		{-1, 2},		},//LEFT
};

const int	g_ptOffsetYRound[81] =
{ 
	0,  0,	1,  1,	1,	0, -1, -1, -1,
	0,  1,  2,  2,  2,  2,  2,  1,  0, -1, -2, -2, -2, -2, -2, -1,
	0,  1,  2,  3,  3,  3,  3,  3,  3,  3,  2,  1,  0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1,
	0,  1,  2,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  2,  1,  0, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -2, -1,
};
const int	g_ptOffsetXRound[81] =
{
	 0, -1, -1,  0,  1,  1,	 1,	 0, -1,
	-2, -2, -2, -1,  0,  1,  2,  2,  2,  2,  2,  1,  0, -1, -2, -2,
	-3, -3, -3, -3, -2, -1,  0,  1,  2,  3,  3,  3,  3,  3,  3,  3,  2,  1,  0, -1, -2, -3, -3, -3,
	-4, -4, -4, -4, -4, -3, -2, -1,  0,  1,  2,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  3,  2,  1,  0, -1, -2, -3, -4, -4, -4, -4, 
};
//****************** 新的修正所添加的类型***********************************

const POINT g_ptOffsetLine4_Round3[8][12] = 
{
  {		{0,0},		{-1, 0},		{ 0,-1},		{ 0, 1},		{-1,-1},		{-1, 1},		{ 1, 0},		{ 1, 1},	{ 1,-1},		{-2, 0},		{-3, 0},		{-4, 0},    },//LEFT_UP
  {		{0,0},		{-1,-1},		{-1, 0},		{ 0,-1},		{-1, 1},	  { 1,-1},		{ 0, 1},		{ 1, 0},	{ 1, 1},		{-2,-2},		{-3,-3},		{ 0, 0},		},//UP
  {		{0,0},		{ 0,-1},		{-1, 0},		{ 1, 0},		{-1,-1},		{ 1,-1},	  { 0, 1},		{-1, 1},	{ 1, 1},		{ 0,-2},		{ 0,-3},		{ 0,-4},		},//RIGHT_UP
  {		{0,0},		{ 1,-1},		{ 1, 0},		{ 0,-1},		{ 1, 1},		{-1,-1},		{-1, 0},		{ 0, 1},	{-1, 1},		{ 2,-2},		{ 3,-3},		{ 0, 0},		},//RIGHT
  {		{0,0},		{ 1, 0},		{ 0, 1},		{ 0,-1},		{ 1, 1},		{ 1,-1},		{-1, 0},		{-1,-1},	{-1, 1},		{ 2, 0},		{ 3, 0},		{ 4, 0},	  },//RIGHT_DOWN
  {		{0,0},		{ 1, 1},	  { 0, 1},		{ 1, 0},		{-1, 1},		{ 1,-1},		{-1, 0},		{ 0,-1},	{-1,-1},	  { 2, 2},	  { 3, 3},		{ 0, 0},		},//DOWN
  {		{0,0},		{ 0, 1},		{-1, 0},		{ 1, 0},		{-1, 1},		{ 1, 1},	  { 0,-1},		{-1,-1},	{ 1,-1},		{ 0, 2},		{ 0, 3},	  { 0, 4},		},//LEFT_DOWN
  {		{0,0},		{-1, 1},		{-1, 0},		{ 0, 1},		{-1,-1},		{ 1, 1},		{ 0,-1},		{ 1, 0},	{ 1,-1},		{-2, 2},		{-3, 3},		{ 0, 0},		},//LEFT
};
const POINT g_ptOffsetFan3L[8][4] =
{
	{		{0,0},		{-1,0},		{0,1},		{-1,1},		},//LEFT_UP	
	{		{0,0},		{-1,-1},	{-1,0},		{0,-1},		},//UP
	{		{0,0},		{0,-1},		{1,0},		{1,-1},		},//RIGHT_UP
	{		{0,0},		{1,-1},		{0,-1},		{1,0},		},//RIGHT
	{		{0,0},		{1,0},		{0,1},		{1,1},		},//RIGHT_DOWN
	{		{0,0},		{1,1},		{1,0},		{0,1},		},//DOWN
	{		{0,0},		{0,1},		{1,0},		{1,1},		},//LEFT_DOWN
	{		{0,0},		{1,1},		{-1,0},		{0,1},		},//LEFT
};

const POINT g_ptOffsetFan3S[8][4] =
{
	{		{0,0},		{-1,0},		{-1,-1},	{-1,-2},	},//LEFT_UP	
	{		{0,0},		{-1,-1},	{0,-1},		{1,-1},		},//UP
	{		{0,0},		{0,-1},		{1,-1},		{1,-2},		},//RIGHT_UP
	{		{0,0},		{1,-1},		{1,0},		{1,1},		},//RIGHT
	{		{0,0},		{1,0},		{1,1},		{1,2},		},//RIGHT_DOWN
	{		{0,0},		{1,1},		{0,1},		{-1,1},		},//DOWN
	{		{0,0},		{0,1},		{-1,1},		{-2,1},		},//LEFT_DOWN
	{		{0,0},		{1,1},		{-1,0},		{-1,-1},	},//LEFT
};
const POINT g_ptOffsetFanR[8][6] =
{
	{		{0,0},		{-1,0},		{-1,-1},	{-1,1},		{0,-1},		{1,-1},		},//LEFT_UP	
	{		{0,0},		{-1,-1},	{0,-1},		{-1,0},		{1,0},		{2,1},		},//UP
	{		{0,0},		{0,-1},		{1,-1},		{-1,-1},	{1,0},		{1,1},		},//RIGHT_UP
	{		{0,0},		{1,-1},		{1,0},		{0,-1},		{1,1},		{0,1},		},//RIGHT
	{		{0,0},		{1,0},		{1,1},		{1,-1},		{0,1},		{-1,1},		},//RIGHT_DOWN
	{		{0,0},		{1,1},		{0,1},		{1,0},		{-1,0},		{-2,-1},	},//DOWN
	{		{0,0},		{0,1},		{-1,1},		{1,1},		{-1,0},		{-1,-1},	},//LEFT_DOWN
	{		{0,0},		{1,1},		{-1,0},		{0,1},		{-1,-1},	{-1,0},		},//LEFT
};
const POINT g_ptOffsetLineS[8][4] = 
{
	{		{0,0},		{1,0},		{-1,0},		{-2,0},		},//LEFT_UP	
	{		{0,0},		{1,1},		{-1,-1},	{-2,-2},	},//UP
	{		{0,0},		{0,1},		{0,-1},		{0,-2},		},//RIGHT_UP
	{		{0,0},		{-1,1},		{1,-1},		{2,-2},		},//RIGHT
	{		{0,0},		{-1,0},		{1,0},		{2,0},		},//RIGHT_DOWN
	{		{0,0},		{-1,-1},	{1,1},		{2,2},		},//DOWN
	{		{0,0},		{0,-1},		{0,1},		{0,2},		},//LEFT_DOWN
	{		{0,0},		{1,-1},		{-1,1},		{-2,2},		},//LEFT
};


//=======================================================================================
// 不含无形
//
inline WORD GetBitCountFor_Element( const DWORD & dwData )
{
  static WORD      g_BitCount1 = 0;

  g_BitCount1 = 0;
  for( int i = 0; i < 11; i++ )
  {
    if( ( dwData & ( 1 << i ) ) ) g_BitCount1++;
  }
  return g_BitCount1;
}
//=======================================================================================
// 不含无形
//
inline WORD GetBitCountFor_Race( const DWORD & dwData )
{
  static WORD      g_BitCount2 = 0;

  g_BitCount2 = 0;
  for( int i = 0; i < 23; i++ )
  {
    if( ( dwData & ( 1 << i ) ) ) g_BitCount2++;
  }
  return g_BitCount2;
}
//
static DWORD	g_dwBlock;
static POINT	g_ptTemp;
static POINT	g_ptAllTargets[MAX_RANGE_POINT];
static BYTE		g_iSize;
//=======================================================================================
// desc:
// parameter: CMonster * pUseMonster, CSkill * pUseSkill
// return:
//=======================================================================================
void CheckLineD(const POINT &ptUser, CGameMap *pTheMap, const int &iDir,const int &nLineLength)
{
	static int			g_LineDC;
	g_iSize = 0;

	for( g_LineDC = 0; g_LineDC < nLineLength; g_LineDC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetLineD[iDir][g_LineDC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetLineD[iDir][g_LineDC].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckLineS(const POINT &ptUser,CGameMap *pTheMap, const int &iDir,const int &nLineLength)
{
	static int		g_LineSC;

	g_iSize = 0;
	for( g_LineSC = 0; g_LineSC < nLineLength; g_LineSC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetLineS[iDir][g_LineSC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetLineS[iDir][g_LineSC].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );		
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckLineW(const POINT &ptUser, CGameMap *pTheMap, const int &iDir,const int &nTilesRange)
{
	static int		g_LineWC;

	g_iSize = 0;
	for( g_LineWC = 0; g_LineWC < nTilesRange; g_LineWC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetLineW[iDir][g_LineWC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetLineW[iDir][g_LineWC].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y;
		g_iSize++;
	}
}
//=======================================================================================
// desc:		扇型检查
// parameter:
// return:
//=======================================================================================
void CheckFan(const POINT &ptUser, CGameMap *pTheMap, const int &iDir,const int &nFanWidth)
{
	static int		g_FanC;

	g_iSize = 0;
	for( g_FanC = 0; g_FanC < nFanWidth; g_FanC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetFan[iDir][g_FanC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetFan[iDir][g_FanC].y;

		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );

		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
		{
			break;
		}
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y;
		g_iSize++;
	}
}

//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckLINEW2_FAN5(const POINT &ptUser, CGameMap *pTheMap, const int &iDir)
{
	static int		g_Fan5C;

	g_iSize = 0;	
	for( g_Fan5C = 0; g_Fan5C < 9; g_Fan5C++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetLineW_Fan[iDir][g_Fan5C].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetLineW_Fan[iDir][g_Fan5C].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckLINEW2_ROUND3(const POINT &ptUser, CGameMap *pTheMap, const int &iDir)
{
	static int			g_Round3C;

	g_iSize = 0;	
	for( g_Round3C = 0; g_Round3C < 12; g_Round3C++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetLineW_Round[iDir][g_Round3C].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetLineW_Round[iDir][g_Round3C].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckLINE4_ROUND3(const POINT &ptUser, CGameMap *pTheMap, const int &iDir)
{
	static int			g_Round3C1;

	g_iSize = 0;	
	for( g_Round3C1 = 0; g_Round3C1 < 12; g_Round3C1++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetLine4_Round3[iDir][g_Round3C1].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetLine4_Round3[iDir][g_Round3C1].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckRound(const POINT &ptUser, CGameMap *pTheMap,const int &nTilesRange)
{
	static int			g_RoundC;

	g_iSize = 0;
  if( nTilesRange <= 81 )
  {
	  for( g_RoundC = 0; g_RoundC < nTilesRange; g_RoundC++ )
	  {
		  g_ptTemp.x = ptUser.x + g_ptOffsetXRound[g_RoundC];
		  g_ptTemp.y = ptUser.y + g_ptOffsetYRound[g_RoundC];
		  g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		  if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			  break;
		  g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		  g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		  g_iSize++;
	  }
  }
  else
  {
	  for( g_RoundC = 0; g_RoundC < nTilesRange; g_RoundC++ )
	  {
		  g_ptTemp.x = ptUser.x + DirOffsetX[g_RoundC];
		  g_ptTemp.y = ptUser.y + DirOffsetY[g_RoundC];
		  g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		  if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			  break;
		  g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		  g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		  g_iSize++;
	  }
  }
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckFan3L(const POINT &ptUser, CGameMap *pTheMap, const int &iDir)
{
	static int		g_Fan3LC;

	g_iSize = 0;
	for( g_Fan3LC = 0; g_Fan3LC < 4; g_Fan3LC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetFan3L[iDir][g_Fan3LC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetFan3L[iDir][g_Fan3LC].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckFan3S(const POINT &ptUser, CGameMap *pTheMap, const int &iDir)
{
	static int			g_Fan3SC;

	g_iSize = 0;
	for( g_Fan3SC = 0; g_Fan3SC < 4; g_Fan3SC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetFan3S[iDir][g_Fan3SC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetFan3S[iDir][g_Fan3SC].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}
//=======================================================================================
// desc:
// parameter:
// return:
//=======================================================================================
void CheckFanR(const POINT &ptUser, CGameMap *pTheMap, const int &iDir,const int &nWidth)
{
	static int			g_FanRC;

	g_iSize = 0;
	for( g_FanRC = 0; g_FanRC < nWidth; g_FanRC++ )
	{
		g_ptTemp.x = ptUser.x + g_ptOffsetFanR[iDir][g_FanRC].x;
		g_ptTemp.y = ptUser.y + g_ptOffsetFanR[iDir][g_FanRC].y;
		g_dwBlock = pTheMap->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
			break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y;
		g_iSize++;
	}
}

//=====================================================================================
//
// class  CSrvBaseSkill implement
//
//=====================================================================================
// constructor 2
CSrvBaseSkill::CSrvBaseSkill()
{
  m_wId = 0;
#ifdef _REPAIR_SERVER_CRASH_NICK_
	memset( m_szName, 0, MAX_SKILL_NAME_LEN );
#else	
  strcpy( m_szName, "" );
#endif
	m_wIconId=0;				
	m_wPracticeMax=0;	
	m_wAdvanceSkillId=0;
	m_wType=0;					
	m_wElement=0;		
	m_iEleEffect=0;
	m_iCostHp=0;			
	m_iCostHpPer=0;		
	m_iCostMp=0;			
	m_iCostMpPer=0;		
	m_iCostSp=0;			
	m_iCostSpPer=0;		
	m_iCostSoul=0;
	m_bBlock=0;
	m_iAlarmTime=0;		
	m_wAlarmSkillId=0;	
	m_iTriggerTime=0;	
	m_wTriggerSkillId=0;
	m_iHpChange=0;		
	m_iMpChange=0;		
	m_iSpChange=0;		
	m_iSoulChange=0;	
	m_iApChange=0;		
	m_iHitChange=0;		
	m_iDpChange=0;		
	m_iDgChange=0;		
	m_iIntChange=0;
  m_iBearPsbChange=0;
	m_iRandom=0;	
	m_iCriticalHit=0;
	m_iHard=0;			
	m_qwStatus=0;	
	m_wSkillLearnId=0;
	m_bPosition=0;
	m_bColor=0;
	m_bCase=0;
	m_wWarpMapId=0;
	m_wWarpX=0;
	m_wWarpY=0;
	m_wFuncChangeId=0;
	m_iItemHm=0;
	m_iItemHp=0;
	m_iItemHard=0;
	m_iItemHu=0;
	m_iTimes=0;
	m_iProbability=0;
	m_wRace=0;
	m_wMonsterId=0;
	m_wChangeItemId=0;
	m_wUseEffect=0;
	m_wObjEffect=0;
	m_wBeforeDelay=0;
	m_wAfterDelay=0;
	m_bTarget=0;
	m_bShape=0;
	m_bRange=0;
  m_wElement=0;
  m_dwRaceAttri=0;
  m_wRaceBonuRate=0;
  m_dwBossCode=0;
  m_wBossBonuRate=0;
  m_wOwnAttrEffect=0;
  m_wOwnAttrBonuRate=0;
  m_bDrain=0;
  m_iChain=0;
  m_iIntonateTime = 0;
  m_iBossSpecialTimeRate = 0;
  ///////////////////////////////////
  //Add by CECE 2004-04-05
  m_wCostMana = 0;
  ///////////////////////////////////
}
//======================================================================================
//
//
CSrvBaseSkill::~CSrvBaseSkill()
{
}
//======================================================================================
// desc:			获取这个base skill的攻击范围
// parameter:
// return:
//======================================================================================
BYTE CSrvBaseSkill::GetHitRange( const POINT & ptUser, CGameMap *pTheMap, const int & iDir, POINT* pAllTargets )
{
	switch( m_bShape )
	{
	////////////// FAN \\\\\\\\\\\\\\\\\\\/
	case SKILL_SHAPE_FAN_W1:
		CheckFan( ptUser, pTheMap, iDir, 2 );
		break;
	case SKILL_SHAPE_FAN_W3:
		CheckFan( ptUser, pTheMap, iDir, 4 );
		break;
	case SKILL_SHAPE_FAN_W5:
		CheckFan( ptUser, pTheMap, iDir, 6 );
		break;
	case SKILL_SHAPE_FAN_W7:
		CheckFan( ptUser, pTheMap, iDir, 8 );
		break;
		//==========new add=========
	case SKILL_SHAPE_FAN3_L:
		CheckFan3L( ptUser, pTheMap, iDir );
		break;
	case SKILL_SHAPE_FAN3_S:
		CheckFan3S( ptUser, pTheMap, iDir);
		break;
	case SKILL_SHAPE_FAN4_R:
		CheckFanR( ptUser, pTheMap, iDir, 5 );
		break;
	case SKILL_SHAPE_FAN5_R:
		CheckFanR( ptUser, pTheMap, iDir, 6 );
		break;
  case SKILL_SHAPE_LINEW_3:
		CheckLineW( ptUser, pTheMap, iDir, 10);
    break;
   case SKILL_SHAPE_LINEW_6:
		CheckLineW( ptUser, pTheMap, iDir, 19);
     break;
	/////////////// LINE \\\\\\\\\\\\\\\\\\\\\/
	case SKILL_SHAPE_LINE_D1:
		CheckLineD( ptUser, pTheMap, iDir,2 );
		break;
	case SKILL_SHAPE_LINE_D2:
		CheckLineD( ptUser, pTheMap, iDir,3 );
		break;
	case SKILL_SHAPE_LINE_D3:
		CheckLineD( ptUser, pTheMap, iDir,4 );
		break;
	case SKILL_SHAPE_LINE_D4:
		CheckLineD( ptUser, pTheMap, iDir,5 );
		break;
	case SKILL_SHAPE_LINE_D5:
		CheckLineD( ptUser, pTheMap, iDir,6 );
		break;
	case SKILL_SHAPE_LINE_D6:
		CheckLineD( ptUser, pTheMap, iDir,7 );
		break;
	case SKILL_SHAPE_LINE_D7:
		CheckLineD( ptUser, pTheMap, iDir,8 );
		break;
	case SKILL_SHAPE_LINE_D8:
		CheckLineD( ptUser, pTheMap, iDir,9 );
		break;
		//==========new add=========
	case SKILL_SHAPE_LINE3_S:
		CheckLineS( ptUser, pTheMap, iDir, 3 );
		break;
	case SKILL_SHAPE_LINE4_S:
		CheckLineS( ptUser, pTheMap, iDir, 4 );
		break;
	/////////////// LINE_W \\\\\\\\\\\\\\\\\\\\/
	case SKILL_SHAPE_LINEW_D2:
		CheckLineW( ptUser, pTheMap, iDir, 7);
		break;
	case SKILL_SHAPE_LINEW_D4:
		CheckLineW( ptUser, pTheMap, iDir, 13 );
		break;
	////////////// 混合型 \\\\\\\\\\\\\\\\\\/
	case SKILL_SHAPE_LINEW2_FAN5:
		CheckLINEW2_FAN5(ptUser, pTheMap, iDir );
		break;
	case SKILL_SHAPE_LINEW2_ROUND3:
		CheckLINEW2_ROUND3( ptUser, pTheMap, iDir);
		break;
		//==========new add=========
	case SKILL_SHAPE_LINE4_ROUND3:
		CheckLINE4_ROUND3( ptUser, pTheMap, iDir);
		break;
	///////////// ROUND \\\\\\\\\\\\\\\\\\\\\\\/
	case SKILL_SHAPE_ROUND_1X1:
		CheckRound( ptUser, pTheMap, 2 );
		break;
	case SKILL_SHAPE_ROUND_3X3:
		CheckRound( ptUser, pTheMap,9 );
		break;
	case SKILL_SHAPE_ROUND_5X5:
		CheckRound( ptUser, pTheMap,25 );
		break;
	case SKILL_SHAPE_ROUND_7X7:
		CheckRound( ptUser, pTheMap,49 );
		break;
	case SKILL_SHAPE_ROUND_9X9:
		CheckRound( ptUser, pTheMap,81 );
		break;
  /////////////////////////////
  //Add by CECE 2004-04-22
  case SKILL_SHAPE_ROUND_11X11:
		CheckRound( ptUser, pTheMap, 121 );
    break;
  case SKILL_SHAPE_ROUND_13X13:
    CheckRound( ptUser, pTheMap, 169 );
    break;
  /////////////////////////////
	default:
		return 0;
	}
	if( pAllTargets != NULL ) pAllTargets = g_ptAllTargets;
	return g_iSize;
}
//======================================================================================
// public:
//					功能函数部分
//
//======================================================================================
//==========================================================================================
//desc:				清除陷阱的魔法效果
//parameter:	
//return   :  bool
//==========================================================================================
void CSrvBaseSkill::ClearTrap(CPlayer *pUser)
{
  static WORD   wTrapCount, wTrapX, wTrapY, wLoopNum, wMagicCode;
  WORD          XXX = pUser->GetPosX(), YYY = pUser->GetPosY();
  CGameMap      *pTheMap   = pUser->GetInMap();
  CMagic        *pTheMagic = NULL;
  SMsgData      *pNewMsg   = NULL;
  SNMClearTrap  *pClearTrap;

  if( pTheMap == NULL )
  {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName( "CSrvBaseSkill::ClearTrap" );
    _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "%s: User Map Ptr=NULL, MapId=%d, When User Clear Trap Magic #", pUser->GetAccount(), pUser->GetMapId() );
    g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szDamageLog );
#endif
    return;
  }
  // Init Variable
  wTrapCount = 0;
  wLoopNum   = 0;
  if( NULL == ( pNewMsg = g_pGs->NewMsgBuffer( pUser->GetSelfCode() ) ) )
  {
    if( NULL == ( pNewMsg = g_pGs->NewMsgBuffer() ) )   return;
  }
  pNewMsg->Init();
  pNewMsg->dwAID    = A_CLEARTRAP;
  pNewMsg->dwMsgLen = 1;
  pClearTrap = (SNMClearTrap*)(pNewMsg->Msgs[0].Data);
  //
  switch( GetShape() )
  {
  // 5X5
  case SKILL_SHAPE_ROUND_5X5:
    wLoopNum = 25;
    break;
  // 7X7
  case SKILL_SHAPE_ROUND_7X7:
    wLoopNum = 49;
    break;
  // 9X9
  case SKILL_SHAPE_ROUND_9X9:
    wLoopNum = 81;
    break;
  /////////////////////////////
  //Add by CECE 2004-04-22
  // 11X11
  case SKILL_SHAPE_ROUND_11X11:
    wLoopNum = 121;
    break;
  // 13X13
  case SKILL_SHAPE_ROUND_13X13:
    wLoopNum = 169;
    break;
  /////////////////////////////
  // 3X3
  default:
  case SKILL_SHAPE_ROUND_3X3:
    wLoopNum = 9;
    break;
  }
  for( int i = 0; i < wLoopNum && i < 81; i++ )
  {
    wTrapX = XXX + g_ptOffsetXRound[i];
    wTrapY = YYY + g_ptOffsetYRound[i];
    if( 0 != ( wMagicCode = pTheMap->GetMagicCode( wTrapX, wTrapY ) ) )
    {
      if( NULL != ( pTheMagic = pTheMap->GetMagic( wMagicCode ) ) )
      {
        pClearTrap->wTrapCode = wMagicCode;
        pClearTrap->wPosX     = wTrapX;
        pClearTrap->wPosY     = wTrapY;
        pClearTrap++;
        wTrapCount++;
        // Clear Magic
        pTheMagic->SetState( MAGICSTATE_DEAD );
      }
    }
  }
  // Send Message To Client
  if( wTrapCount )
  {
    pNewMsg->Msgs[0].Size = wTrapCount * sizeof( SNMClearTrap );
    pUser->AddSendMsg( pNewMsg );
  }
  else
  {
    g_pGs->ReleaseMsg( pNewMsg );
  }
}
//======================================================================================
// 
//Heal
void CSrvBaseSkill::Heal(CLife *pLife,CSkill* pSkill)
{
//#ifdef _DEBUG_CHECK_FUNC_STATE_
//  CCheckFuncState callStackCheck("CSrvBaseSkill::Heal()");
//#endif
  if( NULL == pLife->m_pInMap )   return;
  // 6 个特殊状态
	LPSNMSetSpecialStatus		pHealMsg = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);
	BYTE										bHealCount = 0;

	if( m_bTarget == SKILL_TARGET_SELF)
	{
		switch( m_wType )
		{
		case SKILL_TYPE_ITEM_HEAL1:
			Heal1( pLife );
			break;
		case SKILL_TYPE_ITEM_HEAL2:
			Heal2( pLife );
			break;
		case SKILL_TYPE_ITEM_HEAL3:
			Heal3( pLife );
			break;
		case SKILL_TYPE_ITEM_HEAL4:
			Heal4( pLife );
			break;
		}
    pHealMsg->dwCode_Skill = ( pLife->GetCode() << 16 ) | m_wId;
    pHealMsg->qwStatus     = pLife->GetSpecialStatus();
    // Send Set Special Status To Client
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
    pLife->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pLife->m_iX, pLife->m_iY );
  }
	else if( m_bTarget == SKILL_TARGET_TEAM )
	{
		if( pLife->IsPlayer() )
		{
	    CPlayer						*pHealPlayer   = NULL;
	    CTeam							*pHealTeam     = NULL;
			if( NULL != ( pHealTeam = ((CPlayer*)pLife)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pHealPlayer = pHealTeam->GetMember( i ) ) &&
              pHealPlayer->GetStatus() != STATUS_PLAYER_DEAD &&
              pHealPlayer->GetLifeDis( pLife ) < MAX_SPE_STATUS_DIS )
					{
						switch( m_wType )
						{
						case SKILL_TYPE_ITEM_HEAL1:
							Heal1( pHealPlayer );
							break;
						case SKILL_TYPE_ITEM_HEAL2:
							Heal2( pHealPlayer );
							break;
						case SKILL_TYPE_ITEM_HEAL3:
							Heal3( pHealPlayer );
							break;
						case SKILL_TYPE_ITEM_HEAL4:
							Heal4( pHealPlayer );
							break;
						}
            // Send Set Special Status To Client
            pHealMsg->dwCode_Skill = ( pHealPlayer->GetCode() << 16 ) | m_wId;
            pHealMsg->qwStatus     = pHealPlayer->GetSpecialStatus();
            pHealMsg++;
            bHealCount++;
					}
				}
			}
      else
      {
        switch( m_wType )
        {
        case SKILL_TYPE_ITEM_HEAL1:
          Heal1( pLife );
          break;
        case SKILL_TYPE_ITEM_HEAL2:
          Heal2( pLife );
          break;
        case SKILL_TYPE_ITEM_HEAL3:
          Heal3( pLife );
          break;
        case SKILL_TYPE_ITEM_HEAL4:
          Heal4( pLife );
          break;
        }
        pHealMsg->dwCode_Skill = ( pLife->GetCode() << 16 ) | m_wId;
        pHealMsg->qwStatus     = pLife->GetSpecialStatus();
        bHealCount++;
      }
		}
    else if( pLife->IsMonster() )
    {
      // Monster Revive All Teamer
      CMonsterTeamers     *pMstTeam = NULL;
      WORD                wMstCount = 0;
      CMonster            **pMonsters = NULL;

      if( NULL != ( pMstTeam = ((CMonster*)(pLife))->GetTeam() ) )
      {
        wMstCount = pMstTeam->GetCount();
        pMonsters = pMstTeam->GetMonsters();
        for( int i = 0; i < wMstCount; i++ )
        {
          if( pMonsters[i] &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_DEAD &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_GOTO_DEAD &&
              pMonsters[i]->GetDistance( pLife->GetPosX(), pLife->GetPosY() ) < MAX_SPE_STATUS_DIS )
          {
						switch( m_wType )
						{
						case SKILL_TYPE_ITEM_HEAL1:
							Heal1( pMonsters[i] );
							break;
						case SKILL_TYPE_ITEM_HEAL2:
							Heal2( pMonsters[i] );
							break;
						case SKILL_TYPE_ITEM_HEAL3:
							Heal3( pMonsters[i] );
							break;
						case SKILL_TYPE_ITEM_HEAL4:
							Heal4( pMonsters[i] );
							break;
						}
            //
            pHealMsg->dwCode_Skill      = ( pMonsters[i]->GetSelfCode() << 16 ) | m_wId;
            pHealMsg->qwStatus          = pMonsters[i]->GetSpecialStatus();
            pHealMsg++;
            bHealCount++;
          }
        }
      }
      else
      {
        switch( m_wType )
        {
        case SKILL_TYPE_ITEM_HEAL1:
          Heal1( pLife );
          break;
        case SKILL_TYPE_ITEM_HEAL2:
          Heal2( pLife );
          break;
        case SKILL_TYPE_ITEM_HEAL3:
          Heal3( pLife );
          break;
        case SKILL_TYPE_ITEM_HEAL4:
          Heal4( pLife );
          break;
        }
        pHealMsg->dwCode_Skill      = ( pLife->GetCode() << 16 ) | m_wId;
        pHealMsg->qwStatus          = pLife->GetSpecialStatus();
        bHealCount++;
      }
    }
    if( bHealCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bHealCount;
      pLife->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pLife->m_iX, pLife->m_iY );
    }
	}
	else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE)
	{
    if( pSkill == NULL )    return;
  	static LifeTileList::iterator	  Iter_Hl;
	  CLife							              *pHealLife     = NULL;
		for( Iter_Hl = pSkill->m_listTargets.begin(); Iter_Hl != pSkill->m_listTargets.end(); Iter_Hl++ )
		{
      pHealLife = (*Iter_Hl);
      switch( m_wType )
      {
      case SKILL_TYPE_ITEM_HEAL1:
        Heal1( pHealLife );
        break;
      case SKILL_TYPE_ITEM_HEAL2:
        Heal2( pHealLife );
        break;
      case SKILL_TYPE_ITEM_HEAL3:
        Heal3( pHealLife );
        break;
      case SKILL_TYPE_ITEM_HEAL4:
        Heal4( pHealLife );
        break;
      }
      pHealMsg->dwCode_Skill = ( pHealLife->GetCode() << 16 ) | m_wId;
      pHealMsg->qwStatus     = pHealLife->GetSpecialStatus();
      pHealMsg++;
      bHealCount++;
		}
    if( bHealCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
      pLife->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pLife->m_iX, pLife->m_iY );
    }
	}
}
//======================================================================================
// 
// Heal1
void CSrvBaseSkill::Heal1(CLife *pLife)
{
  if( pLife->IsMonster() && ((CMonster*)pLife)->m_qwAntiStatus & 0x00000000002a0000 )   return;
	if( m_iHpChange )
	{
    ((CPlayer*)pLife)->Send_A_REFRESHTEAMDATA();
		// Add Or Sub Hp
		pLife->m_iHp += m_iHpChange;
		if( pLife->m_iHp > pLife->m_iMaxHp )	pLife->m_iHp = pLife->m_iMaxHp;
		else if( pLife->m_iHp < 1 )						pLife->m_iHp = 1;
	}
	if( m_iMpChange )
	{	
		// Add Or Sub Mp
		pLife->m_iMp += m_iMpChange;
		if( pLife->m_iMp > pLife->m_iMaxMp )	pLife->m_iMp = pLife->m_iMaxMp;
		else if( pLife->m_iMp < 1 )						pLife->m_iMp = 1;
	}
	if( m_iSpChange )
	{
		// Add Or Sub Sp
		pLife->m_iSp += m_iSpChange;
		if( pLife->m_iSp > pLife->m_iMaxSp )	pLife->m_iSp = pLife->m_iMaxSp;
		else if( pLife->m_iSp < 1 )						pLife->m_iSp = 1;
	}
  if(pLife->IsPlayer())  //Add or Zero Soul Player
  {
    CPlayer *pPlayer = (CPlayer*)pLife;
    if(m_iSoulChange>0) 
    {
      pPlayer->m_dwSoul += m_iSoulChange;
      if(pPlayer->m_dwSoul > pPlayer->m_dwMaxSoul)	pPlayer->m_dwSoul = pPlayer->m_dwMaxSoul;
    }
    else if( pLife->m_iSp < 0 ) pPlayer->m_dwSoul = 0;
  }
  pLife->m_iUpdateTurn = 0;
}
//======================================================================================
// 
//Heal2
void CSrvBaseSkill::Heal2(CLife *pLife)
{
  if( pLife->IsMonster() && ((CMonster*)pLife)->m_qwAntiStatus & 0x00000000002a0000 )   return;
	if( m_iHpChange )
	{
    ((CPlayer*)pLife)->Send_A_REFRESHTEAMDATA();
		// Add Or Sub Hp
		pLife->m_iHp += pLife->m_iMaxHp * m_iHpChange / 100;
		if( pLife->m_iHp > pLife->m_iMaxHp )	pLife->m_iHp = pLife->m_iMaxHp;
		else if( pLife->m_iHp < 1 )						pLife->m_iHp = 1;
	}
	if( m_iMpChange )
	{
		// Add Or Sub Mp
		pLife->m_iMp += pLife->m_iMaxMp * m_iMpChange / 100;
		if( pLife->m_iMp > pLife->m_iMaxMp )	pLife->m_iMp = pLife->m_iMaxMp;
		else if( pLife->m_iMp < 1 )						pLife->m_iMp = 1;
	}
	if( m_iSpChange )
	{
		// Add Or Sub Sp
		pLife->m_iSp += pLife->m_iMaxSp * m_iSpChange / 100;
		if( pLife->m_iSp > pLife->m_iMaxSp )	pLife->m_iSp = pLife->m_iMaxSp;
		else if( pLife->m_iSp < 1 )						pLife->m_iSp = 1;
	}
  pLife->m_iUpdateTurn = 0;
}
//======================================================================================
// 
// Heal3
void CSrvBaseSkill::Heal3(CLife *pLife)
{
  if( pLife->IsMonster() && ((CMonster*)pLife)->m_qwAntiStatus & 0x00000000002a0000 )   return;
	if( m_iHpChange )
	{
    ((CPlayer*)pLife)->Send_A_REFRESHTEAMDATA();
		// Add Or Sub MaxHp
		pLife->m_iMaxHp += m_iHpChange;
		if( pLife->m_iMaxHp > MAX_PLAYER_HP )	pLife->m_iMaxHp = MAX_PLAYER_HP;
		else if( pLife->m_iMaxHp < 1 )				pLife->m_iMaxHp = 1;
	}
	if( m_iMpChange )
	{
		// Add Or Sub MaxMp
		pLife->m_iMaxMp += m_iMpChange;
		if( pLife->m_iMaxMp > MAX_PLAYER_MP )	pLife->m_iMaxMp = MAX_PLAYER_SP;
		else if( pLife->m_iMaxMp < 1 )				pLife->m_iMaxMp = 1;
	}
	if( m_iSpChange )
	{
		// Add Or Sub MaxSp
		pLife->m_iMaxSp += m_iSpChange;
		if( pLife->m_iMaxSp > MAX_PLAYER_SP )	pLife->m_iMaxSp = MAX_PLAYER_HP;
		else if( pLife->m_iMaxSp < 1 )				pLife->m_iMaxSp = 1;
	}
  pLife->m_iUpdateTurn = 0;
}
//======================================================================================
// 
// Heal4
void CSrvBaseSkill::Heal4(CLife *pLife)
{
  if( pLife->IsMonster() && ((CMonster*)pLife)->m_qwAntiStatus & 0x00000000002a0000 )   return;
	// Set Alarm Time, Alarm Function, Trigger Time And Trigger Function
	if( m_qwStatus & SPE_STATUS_HP && m_iHpChange )
	{
		pLife->m_qwSpecialStatus	|= SPE_STATUS_HP;
		pLife->m_dwAlarmTime[17]	 = ClientTickCount + m_iAlarmTime;
		pLife->m_wFuncAlarm[17]	   = m_wAlarmSkillId;
		pLife->m_dwTrigger[17][0]  = ClientTickCount + m_iTriggerTime;
		pLife->m_wFuncTrigger[17]  = m_wTriggerSkillId;
		//pLife->m_iHp						  += m_iHpChange;
		//pHeal4Life->m_iAddHp				  += m_iHpChange;
	}
	if( m_qwStatus & SPE_STATUS_MP && m_iMpChange )
	{
		pLife->m_qwSpecialStatus	|= SPE_STATUS_MP;
		pLife->m_dwAlarmTime[19]	 = ClientTickCount + m_iAlarmTime;
		pLife->m_wFuncAlarm[19]	   = m_wAlarmSkillId;
		pLife->m_dwTrigger[19][0]  = ClientTickCount + m_iTriggerTime;
		pLife->m_wFuncTrigger[19]  = m_wTriggerSkillId;
		//pLife->m_iHp						  += m_iMpChange;
		//pHeal4Life->m_iAddHp				  += m_iMpChange;
	}
	if( m_qwStatus & SPE_STATUS_SP && m_iSpChange )
	{
		pLife->m_qwSpecialStatus	|= SPE_STATUS_SP;
		pLife->m_dwAlarmTime[21]	 = ClientTickCount + m_iAlarmTime;
		pLife->m_wFuncAlarm[21]	   = m_wAlarmSkillId;
		pLife->m_dwTrigger[21][0]  = ClientTickCount + m_iTriggerTime;
		pLife->m_wFuncTrigger[21]  = m_wTriggerSkillId;
		//pLife->m_iHp						  += m_iSpChange;
		//pHeal4Life->m_iAddHp				  += m_iSpChange;
	}
  pLife->m_iUpdateTurn = 0;
}
//======================================================================================
// 
//
void CSrvBaseSkill::DoChangeChar(CLife * pUser, CSkill *pSkill)
{
	LPSNMSetSpecialStatus		pChangeChar = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);
	BYTE										bCharLifeCount = 0;

  //
  if( NULL == pUser->m_pInMap )     return;
  //
	if( m_bTarget == SKILL_TARGET_SELF )
	{
#ifdef EVILWEAPON_3_6_VERSION
        CPlayer* pcPlayer = NULL;
        if( pUser->IsPlayer() )
        {
          pcPlayer = (CPlayer*)pUser;
        }
        //如果是玩家而且已经中了妖器召唤
        if( pcPlayer && pcPlayer->IsEvilAttack() ) return;
#endif
		ChangeChar( pUser );
		pChangeChar->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
		pChangeChar->qwStatus     = pUser->m_qwSpecialStatus;
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
		pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
	}
	else if( m_bTarget == SKILL_TARGET_TEAM )
	{
		if( pUser->IsPlayer() )
		{
	    CTeam										*pCharTeam   = NULL;
      CPlayer                 *pCharPlayer = NULL;
			if( NULL != ( pCharTeam = ((CPlayer*)pUser)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pCharPlayer = pCharTeam->GetMember( i ) ) &&
              pCharPlayer->GetStatus() != STATUS_PLAYER_DEAD &&
              pCharPlayer->GetLifeDis(pUser) < MAX_SPE_STATUS_DIS )
					{
#ifdef EVILWEAPON_3_6_VERSION
        CPlayer* pcPlayer = NULL;
        if( pCharPlayer->IsPlayer() )
        {
          pcPlayer = (CPlayer*)pCharPlayer;
        }
        //如果是玩家而且已经中了妖器召唤
        if( pcPlayer && pcPlayer->IsEvilAttack() ) continue;
#endif
						ChangeChar( pCharPlayer );
						pChangeChar->dwCode_Skill = ( pCharPlayer->GetCode() << 16 ) | m_wId;
						pChangeChar->qwStatus     = pCharPlayer->m_qwSpecialStatus;
            pChangeChar++;
            bCharLifeCount++;
					}
				}
			}
		}
    else if( pUser->IsMonster() )
    {
      // Monster Revive All Teamer
      CMonsterTeamers     *pMstTeam = NULL;
      WORD                wMstCount = 0;
      CMonster            **pMonsters = NULL;

      if( NULL != ( pMstTeam = ((CMonster*)(pUser))->GetTeam() ) )
      {
        wMstCount = pMstTeam->GetCount();
        pMonsters = pMstTeam->GetMonsters();
        for( int i = 0; i < wMstCount; i++ )
        {
          if( pMonsters[i] &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_DEAD &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_GOTO_DEAD &&
              pMonsters[i]->GetDistance( pUser->GetPosX(), pUser->GetPosY() ) < MAX_SPE_STATUS_DIS )
          {
						ChangeChar( pMonsters[i] );
            //
            pChangeChar->dwCode_Skill      = ( pMonsters[i]->GetSelfCode() << 16 ) | m_wId;
            pChangeChar->qwStatus          = pMonsters[i]->GetSpecialStatus();
            pChangeChar++;
            bCharLifeCount++;
          }
        }
      }
      else
      {
		    ChangeChar( pUser );
        //
	      pChangeChar->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
		    pChangeChar->qwStatus		 = pUser->m_qwSpecialStatus;
        bCharLifeCount++;
      }
    }
    //
    if( bCharLifeCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bCharLifeCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
	}
	else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE)
	{
    if( pSkill == NULL )    return;
    //
  	static LifeTileList::iterator	  Iter_Ch;
	  CLife										        *pCharLife = NULL;
    //
		for( Iter_Ch = pSkill->m_listTargets.begin(); Iter_Ch != pSkill->m_listTargets.end(); Iter_Ch++ )
		{
      pCharLife = (*Iter_Ch);
#ifdef EVILWEAPON_3_6_VERSION
        CPlayer* pcPlayer = NULL;
        if( pCharLife->IsPlayer() )
        {
          pcPlayer = (CPlayer*)pCharLife;
        }
        //如果是玩家而且已经中了妖器召唤
        if( pcPlayer && pcPlayer->IsEvilAttack() ) continue;
#endif
      ChangeChar( pCharLife );
      pChangeChar->dwCode_Skill = ( pCharLife->GetCode() << 16 ) | m_wId;
      pChangeChar->qwStatus			= pCharLife->m_qwSpecialStatus;
      pChangeChar++;
      bCharLifeCount++;
		}
    if( bCharLifeCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bCharLifeCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
	}
}
//======================================================================================
// 
//改变角色的AP,DP,HIT,DG,INT,Critical
inline void CSrvBaseSkill::ChangeChar(CLife * pCharLife)
{
	// 1 PK, 2 砍人, 4 加速, 8 狠, 16 准, 32 稳, 64 快, 128 智
	// 256 必杀, 512 毒, 1024 盲, 2048 定, 2^12 封, 2^13 拙, 2^14 隐身,
	// 2^15 侦测隐身, 2^16 加MaxHp, 2^17 加Hp, 2^18 加MaxMp, 2^19 加Mp,
	// 2^20 加MaxSp, 2^21 加Sp, 2^22 五行, 2^23 禁言, 2^24 通缉

  // 6 个特殊状态
  if( pCharLife->IsMonster() &&
      ((CMonster*)pCharLife)->m_qwAntiStatus & 0x00000000000001F8 )
  {
    return;
  }
	if( m_qwStatus & SPE_STATUS_AP )
	{
    pCharLife->m_qwSpecialStatus|= SPE_STATUS_AP;
		pCharLife->m_dwAlarmTime[3]  = ClientTickCount + m_iAlarmTime;
		pCharLife->m_wFuncAlarm[3]   = m_wAlarmSkillId;
    pCharLife->m_iAp						 = pCharLife->m_iAp - pCharLife->m_iAddAp;
    int iAdd = pCharLife->GetAp() * m_iApChange / 100;
		if( iAdd == 0 && m_iApChange > 0 )  iAdd =  1;
		if( iAdd == 0 && m_iApChange < 0 )  iAdd = -1;
		pCharLife->m_iAp						+= iAdd;
		pCharLife->m_iAddAp						 = iAdd;
#ifdef _DEBUG_WRITE_SPECIAL_STATUS_LOG_
    FuncName("CSrvBaseSkill::ChangeChar");
    if( pCharLife->IsPlayer() )
    {
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Add Ap [%s]%s SpecialStatus=%08x,\n"
               "Alarm3_Ap=(%d,%d,%d),\n"
               "Alarm4_Hit=(%d,%d,%d),\n"
               "Alarm5_Dp=(%d,%d,%d),\n"
               "Alarm6_Dodge=(%d,%d,%d),\n"
               "Alarm7_Int=(%d,%d,%d)",
               ((CPlayer*)pCharLife)->m_szAccount, ((CPlayer*)pCharLife)->m_szName, pCharLife->m_qwSpecialStatus, 
               pCharLife->m_dwAlarmTime[3], pCharLife->m_iAddAp, pCharLife->m_iAp,
               pCharLife->m_dwAlarmTime[4], pCharLife->m_iAddHit, pCharLife->m_iHit,
               pCharLife->m_dwAlarmTime[5], pCharLife->m_iAddDp, pCharLife->m_iDp,
               pCharLife->m_dwAlarmTime[6], pCharLife->m_iAddDodge, pCharLife->m_iDodge,
               pCharLife->m_dwAlarmTime[7], pCharLife->m_iAddInt, pCharLife->m_iInt );
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szDamageLog );
#endif
    }
#endif
	}
	if( m_qwStatus & SPE_STATUS_HIT )
	{
		pCharLife->m_qwSpecialStatus|= SPE_STATUS_HIT;
    pCharLife->m_dwAlarmTime[4]  = ClientTickCount + m_iAlarmTime;
    pCharLife->m_wFuncAlarm[4]   = m_wAlarmSkillId;
    pCharLife->m_iHit						 = pCharLife->m_iHit - pCharLife->m_iAddHit;
    int iAdd = pCharLife->GetHit() * m_iHitChange / 100;
		if( iAdd == 0 && m_iHitChange > 0 )  iAdd =  1;
		if( iAdd == 0 && m_iHitChange < 0 )  iAdd = -1;
    pCharLife->m_iHit						+= iAdd;
    pCharLife->m_iAddHit				 = iAdd;
#ifdef _DEBUG_WRITE_SPECIAL_STATUS_LOG_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CSrvBaseSkill::ChangeChar");
    if( pCharLife->IsPlayer() )
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Add Hit [%s]%s SpecialStatus=%08x,\n"
           "Alarm3_Ap=(%d,%d,%d),\n"
           "Alarm4_Hit=(%d,%d,%d),\n"
           "Alarm5_Dp=(%d,%d,%d),\n"
           "Alarm6_Dodge=(%d,%d,%d),\n"
           "Alarm7_Int=(%d,%d,%d)",
           ((CPlayer*)pCharLife)->m_szAccount, ((CPlayer*)pCharLife)->m_szName, pCharLife->m_qwSpecialStatus, 
           pCharLife->m_dwAlarmTime[3], pCharLife->m_iAddAp, pCharLife->m_iAp,
           pCharLife->m_dwAlarmTime[4], pCharLife->m_iAddHit, pCharLife->m_iHit,
           pCharLife->m_dwAlarmTime[5], pCharLife->m_iAddDp, pCharLife->m_iDp,
           pCharLife->m_dwAlarmTime[6], pCharLife->m_iAddDodge, pCharLife->m_iDodge,
           pCharLife->m_dwAlarmTime[7], pCharLife->m_iAddInt, pCharLife->m_iInt );
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szDamageLog );
    }
#endif
#endif
	}
	if( m_qwStatus & SPE_STATUS_DP )
	{
    pCharLife->m_qwSpecialStatus|= SPE_STATUS_DP;
    pCharLife->m_dwAlarmTime[5]  = ClientTickCount + m_iAlarmTime;
    pCharLife->m_wFuncAlarm[5]   = m_wAlarmSkillId;
    pCharLife->m_iDp						 = pCharLife->m_iDp - pCharLife->m_iAddDp;
    int iAdd = pCharLife->GetDp() * m_iDpChange / 100;
		if( iAdd == 0 && m_iDpChange > 0 )  iAdd =  1;
		if( iAdd == 0 && m_iDpChange < 0 )  iAdd = -1;
    pCharLife->m_iDp						+= iAdd;
    pCharLife->m_iAddDp					 = iAdd;
#ifdef _DEBUG_WRITE_SPECIAL_STATUS_LOG_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CSrvBaseSkill::ChangeChar");
    if( pCharLife->IsPlayer() )
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Add Dp [%s]%s SpecialStatus=%08x,\n"
               "Alarm3_Ap=(%d,%d,%d),\n"
               "Alarm4_Hit=(%d,%d,%d),\n"
               "Alarm5_Dp=(%d,%d,%d),\n"
               "Alarm6_Dodge=(%d,%d,%d),\n"
               "Alarm7_Int=(%d,%d,%d)",
               ((CPlayer*)pCharLife)->m_szAccount, ((CPlayer*)pCharLife)->m_szName, pCharLife->m_qwSpecialStatus, 
               pCharLife->m_dwAlarmTime[3], pCharLife->m_iAddAp, pCharLife->m_iAp,
               pCharLife->m_dwAlarmTime[4], pCharLife->m_iAddHit, pCharLife->m_iHit,
               pCharLife->m_dwAlarmTime[5], pCharLife->m_iAddDp, pCharLife->m_iDp,
               pCharLife->m_dwAlarmTime[6], pCharLife->m_iAddDodge, pCharLife->m_iDodge,
               pCharLife->m_dwAlarmTime[7], pCharLife->m_iAddInt, pCharLife->m_iInt );
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szDamageLog );
    }
#endif
#endif
	}
	if( m_qwStatus & SPE_STATUS_DODGE )
	{
		pCharLife->m_qwSpecialStatus|= SPE_STATUS_DODGE;
		pCharLife->m_dwAlarmTime[6]  = ClientTickCount + m_iAlarmTime;
		pCharLife->m_wFuncAlarm[6]	 = m_wAlarmSkillId;
		pCharLife->m_iDodge					 = pCharLife->m_iDodge - pCharLife->m_iAddDodge;
    int iAdd = pCharLife->GetDodge() * m_iDgChange / 100;
		if( iAdd == 0 && m_iDgChange > 0 )  iAdd =  1;
		if( iAdd == 0 && m_iDgChange < 0 )  iAdd = -1;
		pCharLife->m_iDodge					+= iAdd;
		pCharLife->m_iAddDodge			 = iAdd;
#ifdef _DEBUG_WRITE_SPECIAL_STATUS_LOG_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CSrvBaseSkill::ChangeChar");
    if( pCharLife->IsPlayer() )
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Add Dodge [%s]%s SpecialStatus=%08x,\n"
               "Alarm3_Ap=(%d,%d,%d),\n"
               "Alarm4_Hit=(%d,%d,%d),\n"
               "Alarm5_Dp=(%d,%d,%d),\n"
               "Alarm6_Dodge=(%d,%d,%d),\n"
               "Alarm7_Int=(%d,%d,%d)",
               ((CPlayer*)pCharLife)->m_szAccount, ((CPlayer*)pCharLife)->m_szName, pCharLife->m_qwSpecialStatus, 
               pCharLife->m_dwAlarmTime[3], pCharLife->m_iAddAp, pCharLife->m_iAp,
               pCharLife->m_dwAlarmTime[4], pCharLife->m_iAddHit, pCharLife->m_iHit,
               pCharLife->m_dwAlarmTime[5], pCharLife->m_iAddDp, pCharLife->m_iDp,
               pCharLife->m_dwAlarmTime[6], pCharLife->m_iAddDodge, pCharLife->m_iDodge,
               pCharLife->m_dwAlarmTime[7], pCharLife->m_iAddInt, pCharLife->m_iInt );
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szDamageLog );
    }
#endif
#endif
	}
	if( m_qwStatus & SPE_STATUS_INT )
	{
		pCharLife->m_qwSpecialStatus|= SPE_STATUS_INT;
		pCharLife->m_dwAlarmTime[7]  = ClientTickCount + m_iAlarmTime;
		pCharLife->m_wFuncAlarm[7]   = m_wAlarmSkillId;
		pCharLife->m_iInt						 = pCharLife->m_iInt - pCharLife->m_iAddInt;
    int iAdd = pCharLife->GetInt() * m_iIntChange / 100;
		if( iAdd == 0 && m_iIntChange > 0 )  iAdd =  1;
		if( iAdd == 0 && m_iIntChange < 0 )  iAdd = -1;
		pCharLife->m_iInt						+= iAdd;
		pCharLife->m_iAddInt				 = iAdd;
#ifdef _DEBUG_WRITE_SPECIAL_STATUS_LOG_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CSrvBaseSkill::ChangeChar");
    if( pCharLife->IsPlayer() )
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Add Int [%s]%s SpecialStatus=%08x,\n"
               "Alarm3_Ap=(%d,%d,%d),\n"
               "Alarm4_Hit=(%d,%d,%d),\n"
               "Alarm5_Dp=(%d,%d,%d),\n"
               "Alarm6_Dodge=(%d,%d,%d),\n"
               "Alarm7_Int=(%d,%d,%d)",
               ((CPlayer*)pCharLife)->m_szAccount, ((CPlayer*)pCharLife)->m_szName, pCharLife->m_qwSpecialStatus, 
               pCharLife->m_dwAlarmTime[3], pCharLife->m_iAddAp, pCharLife->m_iAp,
               pCharLife->m_dwAlarmTime[4], pCharLife->m_iAddHit, pCharLife->m_iHit,
               pCharLife->m_dwAlarmTime[5], pCharLife->m_iAddDp, pCharLife->m_iDp,
               pCharLife->m_dwAlarmTime[6], pCharLife->m_iAddDodge, pCharLife->m_iDodge,
               pCharLife->m_dwAlarmTime[7], pCharLife->m_iAddInt, pCharLife->m_iInt );
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szDamageLog );
    }
#endif
#endif
	}
	if( m_qwStatus & SPE_STATUS_CRITICAL )
	{
		pCharLife->m_qwSpecialStatus|= SPE_STATUS_CRITICAL;
		pCharLife->m_dwAlarmTime[8]  = ClientTickCount + m_iAlarmTime;
		pCharLife->m_wFuncAlarm[8]   = m_wAlarmSkillId;
		pCharLife->m_iAddCriticalHit = m_iCriticalHit;
	}
#ifdef _NEW_TALISMAN_
	if( m_qwStatus & SPE_STATUS_RAGE )
	{
		pCharLife->m_qwSpecialStatus |= SPE_STATUS_RAGE;
		pCharLife->m_dwAlarmTime[35]  = ClientTickCount + m_iAlarmTime;
		pCharLife->m_wFuncAlarm[35]   = m_wAlarmSkillId;
    pCharLife->m_wFuncTrigger[35] = m_wId; //to reset the value when load the character;
		pCharLife->m_iAddRageAp     = m_iApChange;
		pCharLife->m_iAddRageHarm   = m_iHitChange;
	}
#endif
}
//======================================================================================
// 
// 功能函数之--设置特殊状态
// return 1 : right
// return 0 : Wrong
int CSrvBaseSkill::DoSetSpecialState(CLife * pUser,CSkill* pSkill)
{
	LPSNMSetSpecialStatus		pSpecialStatus = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);
	BYTE										bSpecialLifeCount = 0;

  //
  if( NULL == pUser->m_pInMap )     return 1;
  //
	if( m_bTarget == SKILL_TARGET_SELF )
	{
#ifdef EVILWEAPON_3_6_VERSION
    CPlayer* pcPlayer = NULL;
    if( pUser->IsPlayer() )
    {
      pcPlayer = (CPlayer*)pUser;
    }
    //如果是玩家而且已经中了妖器召唤
    if( pcPlayer && pcPlayer->IsEvilAttack() ) return 1;
#endif
#ifdef _FUNCTION_RING_3_8_
    if( ( ( m_qwStatus & SPE_STATUS_UNSEE ) || ( m_qwStatus & SPE_STATUS_DIVE ) )&&
        ( ClientTickCount < pcPlayer->m_dwAttackStateTime + PLAYER_ATTACK_STATUS_TIME ) )
        return 0; // 此处由client端绣字提示玩家不能"战斗中不能使用隐身术(潜行术)"
#endif
		SetSpecialState( pUser );
		pSpecialStatus->dwCode_Skill  = ( pUser->GetCode() << 16 ) | m_wId;
		pSpecialStatus->qwStatus      = pUser->m_qwSpecialStatus;
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
		pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
	}
	else if( m_bTarget == SKILL_TARGET_TEAM ) // Michael changed it 2003-10-28
	{
		if( pUser->IsPlayer() )
		{
	    CTeam										*pSpecialTeam = NULL;
      CPlayer                 *pSpecialPlayer = NULL;
			if( NULL != ( pSpecialTeam = ((CPlayer*)pUser)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pSpecialPlayer = pSpecialTeam->GetMember( i ) ) &&
              pSpecialPlayer->GetStatus() != STATUS_PLAYER_DEAD &&
              pSpecialPlayer->GetLifeDis( pUser ) < MAX_SPE_STATUS_DIS )
					{
#ifdef EVILWEAPON_3_6_VERSION
            CPlayer* pcPlayer = NULL;
            if( pSpecialPlayer->IsPlayer() )
            {
              pcPlayer = (CPlayer*)pSpecialPlayer;
            }
            //如果是玩家而且已经中了妖器召唤
            if( pcPlayer && pcPlayer->IsEvilAttack() ) continue;
#endif
					  SetSpecialState( pSpecialPlayer );
					  pSpecialStatus->dwCode_Skill = ( pSpecialPlayer->GetCode() << 16 ) | m_wId;
					  pSpecialStatus->qwStatus     = pSpecialPlayer->m_qwSpecialStatus;
            pSpecialStatus++;
            bSpecialLifeCount++;
					}
				}
			}
		}
    else if( pUser->IsMonster() )
    {
      // Monster Revive All Teamer
      CMonsterTeamers     *pMstTeam = NULL;
      WORD                wMstCount = 0;
      CMonster            **pMonsters = NULL;

      if( NULL != ( pMstTeam = ((CMonster*)(pUser))->GetTeam() ) )
      {
        wMstCount = pMstTeam->GetCount();
        pMonsters = pMstTeam->GetMonsters();
        for( int i = 0; i < wMstCount; i++ )
        {
          if( pMonsters[i] &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_DEAD &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_GOTO_DEAD &&
              pMonsters[i]->GetDistance( pUser->GetPosX(), pUser->GetPosY() ) < MAX_SPE_STATUS_DIS )
          {
						SetSpecialState( pMonsters[i] );
            //
            pSpecialStatus->dwCode_Skill      = ( pMonsters[i]->GetSelfCode() << 16 ) | m_wId;
            pSpecialStatus->qwStatus          = pMonsters[i]->GetSpecialStatus();
            pSpecialStatus++;
            bSpecialLifeCount++;
          }
        }
      }
      else
      {
		    SetSpecialState( pUser );
        //
	      pSpecialStatus->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
		    pSpecialStatus->qwStatus		 = pUser->m_qwSpecialStatus;
        bSpecialLifeCount++;
      }
    }
    //
    if( bSpecialLifeCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bSpecialLifeCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
	}
	else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE)
	{
    if( pSkill == NULL )    return 1;
    //
    static LifeTileList::iterator	  Iter_Sp;
	  CLife										        *pSpecialLife = NULL;
    //
		for( Iter_Sp = pSkill->m_listTargets.begin(); Iter_Sp != pSkill->m_listTargets.end(); Iter_Sp++ )
		{
      pSpecialLife = (*Iter_Sp);
#ifdef EVILWEAPON_3_6_VERSION
        CPlayer* pcPlayer = NULL;
        if( pSpecialLife->IsPlayer() )
        {
          pcPlayer = (CPlayer*)pSpecialLife;
        }
        //如果是玩家而且已经中了妖器召唤
        if( pcPlayer && pcPlayer->IsEvilAttack() ) continue;
#endif
      SetSpecialState( pSpecialLife );
			pSpecialStatus->dwCode_Skill = ( pSpecialLife->GetCode() << 16 ) | m_wId;
			pSpecialStatus->qwStatus		 = pSpecialLife->m_qwSpecialStatus;
      pSpecialStatus++;
      bSpecialLifeCount++;
    }
    if( bSpecialLifeCount )
    {
		  MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bSpecialLifeCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
	}
  return 1;
}
//======================================================================================
// 
// 设置目标的状态
//速度 中毒 致盲 不能移动 不能攻击 不能使用物品 隐身 侦测隐身
inline void CSrvBaseSkill::SetSpecialState(CLife * pSpecialLife)
{
	// 1 PK, 2 砍人, 4 加速, 8 狠, 16 准, 32 稳, 64 快, 128 智
	// 256 必杀, 512 毒, 1024 盲, 2048 定, 2^12 封, 2^13 拙, 2^14 隐身,
	// 2^15 侦测隐身, 2^16 加MaxHp, 2^17 加Hp, 2^18 加MaxMp, 2^19 加Mp,
	// 2^20 加MaxSp, 2^21 加Sp, 2^22 五行, 2^23 禁言, 2^24 潜行

  // 8 个特殊状态


  DWORD dwRealAlarmTime = 0;
  if(pSpecialLife->IsMonster() && ((CMonster*)pSpecialLife)->IsBoss() )
  {
     dwRealAlarmTime = ClientTickCount + m_iAlarmTime*m_iBossSpecialTimeRate/100;
  }
  else
  {
     dwRealAlarmTime = ClientTickCount + m_iAlarmTime;
  }


  // 改变速度
	if( m_qwStatus & SPE_STATUS_HIGHSPEED )
	{
    if( pSpecialLife->IsMonster() &&
        (!(((CMonster*)pSpecialLife)->m_qwAntiStatus & SPE_STATUS_HIGHSPEED)) )
    {
		  // 加速
		  if( m_iTimes > 0 )
		  {
			  if( pSpecialLife->m_wSpeed )
			  {
//				  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_HIGHSPEED )
//				  {
//					  pSpecialLife->m_dwAlarmTime[2] += m_iAlarmTime;
//				  }
//				  else
				  
          {
					  pSpecialLife->m_dwAlarmTime[2]  = dwRealAlarmTime;
				  }
				  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_HIGHSPEED;
				  pSpecialLife->m_wFuncAlarm[2]    = m_wAlarmSkillId;
          //
				  pSpecialLife->m_wSpeed			     = pSpecialLife->m_wSpeed - pSpecialLife->m_iAddSpeed + m_iTimes;
			    pSpecialLife->m_iAddSpeed		     = m_iTimes;
			  }
		  }
		  // 减速
		  else if( m_iTimes < 0 )
		  {
			  if( pSpecialLife->m_wSpeed > 1 )
			  {
//				  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_HIGHSPEED )
//				  {
//					  pSpecialLife->m_dwAlarmTime[2] += m_iAlarmTime;
//				  }
//				  else
				  {
					  pSpecialLife->m_dwAlarmTime[2]  = dwRealAlarmTime;					
				  }
				  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_HIGHSPEED;
				  pSpecialLife->m_wFuncAlarm[2]    = m_wAlarmSkillId;
          //
				  pSpecialLife->m_wSpeed				   = pSpecialLife->m_wSpeed - pSpecialLife->m_iAddSpeed + m_iTimes;
				  pSpecialLife->m_iAddSpeed			   = m_iTimes;
			  }
		  }
    }
    else if( pSpecialLife->IsPlayer() )
    {
		  // 加速
		  if( m_iTimes > 0 )
		  {
			  if( pSpecialLife->m_wSpeed )
			  {
//				  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_HIGHSPEED )
//				  {
//					  pSpecialLife->m_dwAlarmTime[2] += m_iAlarmTime;
//				  }
//				  else
				  {
					  pSpecialLife->m_dwAlarmTime[2]  = dwRealAlarmTime;
				  }
				  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_HIGHSPEED;
				  pSpecialLife->m_wFuncAlarm[2]    = m_wAlarmSkillId;
          //
				  pSpecialLife->m_wSpeed			     = pSpecialLife->m_wSpeed - pSpecialLife->m_iAddSpeed + m_iTimes;
			    pSpecialLife->m_iAddSpeed		     = m_iTimes;
			  }
		  }
		  // 减速
		  else if( m_iTimes < 0 )
		  {
			  if( pSpecialLife->m_wSpeed > 1 )
			  {
//				  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_HIGHSPEED )
//				  {
//					  pSpecialLife->m_dwAlarmTime[2] += m_iAlarmTime;
//				  }
//				  else
				  {
					  pSpecialLife->m_dwAlarmTime[2]  = dwRealAlarmTime;					
				  }
				  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_HIGHSPEED;
				  pSpecialLife->m_wFuncAlarm[2]    = m_wAlarmSkillId;
          //
				  pSpecialLife->m_wSpeed				   = pSpecialLife->m_wSpeed - pSpecialLife->m_iAddSpeed + m_iTimes;
				  pSpecialLife->m_iAddSpeed			   = m_iTimes;
			  }
		  }
    }
	}
  // 中毒
	if( m_qwStatus & SPE_STATUS_POISON )
	{
    if( pSpecialLife->IsMonster() &&
        (!(((CMonster*)pSpecialLife)->m_qwAntiStatus & SPE_STATUS_POISON)) )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_POISON )
//		  {
//			  pSpecialLife->m_dwAlarmTime[9]  += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus|= SPE_STATUS_POISON;
			  pSpecialLife->m_dwAlarmTime[9]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[9]		= m_wAlarmSkillId;
		  }
    }
    else if( pSpecialLife->IsPlayer() )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_POISON )
//		  {
//			  pSpecialLife->m_dwAlarmTime[9]  += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus|= SPE_STATUS_POISON;
			  pSpecialLife->m_dwAlarmTime[9]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[9]		= m_wAlarmSkillId;
		  }
    }
	}
  // 致盲
	if( m_qwStatus & SPE_STATUS_BLIND )
	{
    if( pSpecialLife->IsMonster() &&
        (!(((CMonster*)pSpecialLife)->m_qwAntiStatus & SPE_STATUS_BLIND)) )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_BLIND )
//		  {
//			  pSpecialLife->m_dwAlarmTime[10] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_BLIND;
			  pSpecialLife->m_dwAlarmTime[10]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[10]   = m_wAlarmSkillId;
		  }
    }
    else if( pSpecialLife->IsPlayer() )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_BLIND )
//		  {
//			  pSpecialLife->m_dwAlarmTime[10] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_BLIND;
			  pSpecialLife->m_dwAlarmTime[10]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[10]   = m_wAlarmSkillId;
		  }
    }
	}
  // 不能移动
	if( m_qwStatus & SPE_STATUS_UNMOVE )
	{
    if( pSpecialLife->IsMonster() &&
        (!(((CMonster*)pSpecialLife)->m_qwAntiStatus & SPE_STATUS_UNMOVE)) )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNMOVE )
//		  {
//			  pSpecialLife->m_dwAlarmTime[11] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNMOVE;
			  pSpecialLife->m_dwAlarmTime[11]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[11]   = m_wAlarmSkillId;
        ((CMonster*)(pSpecialLife))->ClearStatusNoTimer();
		  }
    }
    else if( pSpecialLife->IsPlayer() )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNMOVE )
//		  {
//			  pSpecialLife->m_dwAlarmTime[11] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNMOVE;
			  pSpecialLife->m_dwAlarmTime[11]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[11]   = m_wAlarmSkillId;
        ((CPlayer*)(pSpecialLife))->ClearMoveAction();
		  }
    }
	}
  // 不能攻击
	if( m_qwStatus & SPE_STATUS_UNATTACK )
	{
    if( pSpecialLife->IsMonster() &&
        (!(((CMonster*)pSpecialLife)->m_qwAntiStatus & SPE_STATUS_UNATTACK)) )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNATTACK )
//		  {
//			  pSpecialLife->m_dwAlarmTime[12] += m_iAlarmTime;
//        ((CMonster*)(pSpecialLife))->ClearStatusNoTimer();
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNATTACK;
			  pSpecialLife->m_dwAlarmTime[12]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[12]   = m_wAlarmSkillId;
        ((CMonster*)(pSpecialLife))->ClearStatusNoTimer();
		  }
    }
    else if( pSpecialLife->IsPlayer() )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNATTACK )
//		  {
//			  pSpecialLife->m_dwAlarmTime[12] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNATTACK;
			  pSpecialLife->m_dwAlarmTime[12]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[12]   = m_wAlarmSkillId;
		  }
    }
	}
  // 不能使用物品
	if( m_qwStatus & SPE_STATUS_UNITEM )
	{
    if( pSpecialLife->IsMonster() &&
        (!(((CMonster*)pSpecialLife)->m_qwAntiStatus & SPE_STATUS_UNITEM)) )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNITEM )
//		  {
//			  pSpecialLife->m_dwAlarmTime[13] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNITEM;
			  pSpecialLife->m_dwAlarmTime[13]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[13]   = m_wAlarmSkillId;
		  }
    }
    else if( pSpecialLife->IsPlayer() )
    {
//		  if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNITEM )
//		  {
//			  pSpecialLife->m_dwAlarmTime[13] += m_iAlarmTime;
//		  }
//		  else
		  {
			  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNITEM;
			  pSpecialLife->m_dwAlarmTime[13]  = dwRealAlarmTime;
			  pSpecialLife->m_wFuncAlarm[13]   = m_wAlarmSkillId;
		  }
    }
	}
  // 隐身
	if( m_qwStatus & SPE_STATUS_UNSEE )
	{
//		if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_UNSEE )
//		{
//			pSpecialLife->m_dwAlarmTime[14] += m_iAlarmTime;
//		}
//		else
		{
			pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNSEE;
			pSpecialLife->m_dwAlarmTime[14]  = ClientTickCount + m_iAlarmTime;
			pSpecialLife->m_wFuncAlarm[14]   = m_wAlarmSkillId;
		}
	}
  // 侦测隐身
	if( m_qwStatus & SPE_STATUS_SEE )
	{
//		if( pSpecialLife->m_qwSpecialStatus & SPE_STATUS_SEE )
//		{
//			pSpecialLife->m_dwAlarmTime[15] += m_iAlarmTime;
//		}
//		else
		{
			pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_SEE;
			pSpecialLife->m_dwAlarmTime[15]  = ClientTickCount + m_iAlarmTime;
			pSpecialLife->m_wFuncAlarm[15]   = m_wAlarmSkillId;
		}
	}
  ///////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-08
#ifdef  EVILWEAPON_3_6_VERSION
  // 玩家的无敌状态
  if( m_qwStatus & SPE_STATUS_GODEMODE )
  {
	  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_GODEMODE;
	  pSpecialLife->m_dwAlarmTime[29]  = ClientTickCount + m_iAlarmTime;
		pSpecialLife->m_wFuncAlarm[29]   = m_wAlarmSkillId;
  }
  //霸体
  if( m_qwStatus & SPE_STATUS_UNINJURED )
  {
	  pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_UNINJURED;
	  pSpecialLife->m_dwAlarmTime[25]  = ClientTickCount + m_iAlarmTime;
		pSpecialLife->m_wFuncAlarm[25]   = m_wAlarmSkillId;
    //改变玩家得霸体值
    pSpecialLife->m_iBearPsb	+= GetBearPosChange();
  }
#endif
#ifdef _FUNCTION_RING_3_8_
  // 玩家的潜行状态
  if( m_qwStatus & SPE_STATUS_DIVE )
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_DIVE;
	  pSpecialLife->m_dwAlarmTime[24]  = ClientTickCount + m_iAlarmTime;
		pSpecialLife->m_wFuncAlarm[24]   = m_wAlarmSkillId;
  }
  // 玩家的直觉状态
  if( m_qwStatus & SPE_STATUS_EAGLE_EYE )
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_EAGLE_EYE;
	  pSpecialLife->m_dwAlarmTime[30]  = ClientTickCount + m_iAlarmTime;
		pSpecialLife->m_wFuncAlarm[30]   = m_wAlarmSkillId;
#ifdef VERSION_38_FUNCTION
    CPlayer *pPlayer = (CPlayer*)pSpecialLife;
    if(pPlayer)
      pPlayer->Send_A_ALLTRAP();
#endif
  }
#endif
#ifdef FUNCTION_LUCKY_ITEM
  if( m_qwStatus & SPE_STATUS_LUCKY )
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_LUCKY;
	  pSpecialLife->m_dwAlarmTime[32]  = ClientTickCount + m_iAlarmTime;
		pSpecialLife->m_wFuncAlarm[32]   = m_wAlarmSkillId;
    pSpecialLife->m_pFuncAlarm[32]   = g_pBase->GetBaseSkill(m_wAlarmSkillId);
  }
#endif
  if( m_qwStatus & SPE_STATUS_EXP )
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_EXP;
    //pSpecialLife->m_dwAlarmTime[33]  = ClientTickCount + m_iAlarmTime;
    pSpecialLife->m_wFuncAlarm[33]   = m_wAlarmSkillId;
  }
  //=======================================================
  //迟缓符 add by jack.ren for 4.0
#ifdef _SLOW_JUJU_
  if(  m_qwStatus & SPE_STATUS_SLOWJUJU  )
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_SLOWJUJU;
    pSpecialLife->m_dwAlarmTime[34]  = ClientTickCount + m_iAlarmTime;
    pSpecialLife->m_wFuncAlarm[34]   = m_wAlarmSkillId;
    pSpecialLife->m_pFuncAlarm[34] = g_pBase->GetBaseSkill(m_wAlarmSkillId);
  }
#endif
#ifdef RESTRAIN_TRAP
  if( m_qwStatus & SPE_STATUS_RESTRAIN_TRAP )
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_RESTRAIN_TRAP;
    pSpecialLife->m_dwAlarmTime[36]  = ClientTickCount + m_iAlarmTime;
    pSpecialLife->m_wFuncAlarm[36]   = m_wAlarmSkillId;
    pSpecialLife->m_pFuncAlarm[36] = g_pBase->GetBaseSkill(m_wAlarmSkillId);
  }
#endif
  // Set The Player Revived Status 
  if(m_qwStatus & SPE_STATUS_REVIVED)
  {
    pSpecialLife->m_qwSpecialStatus |= SPE_STATUS_REVIVED;
    pSpecialLife->m_dwAlarmTime[37]  = ClientTickCount + m_iAlarmTime;
    pSpecialLife->m_wFuncAlarm[37]   = m_wAlarmSkillId;
  }
}
//======================================================================================
//
// 学习技能
bool CSrvBaseSkill::LearnSkill(CPlayer* pPlayer)
{
	CSrvBaseSkill				*pNewLearnBase;
	CSkill							*pNewLearnSkill;

  if( NULL != ( pNewLearnSkill = pPlayer->GetOwnSkill( m_wSkillLearnId ) ) )
  {
    pPlayer->AddSendErrorMsg( ERROR_CODE_LEARN_SKILL );
    return false;
  }

#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_ 

	if(	GetType() == SKILL_TYPE_GUILDMASTER_SKILL  ) 
	{
		if(		pPlayer->GetGuild() == NULL									 ||
				(	pPlayer->GetGuild())->IsMaster(pPlayer->GetMailId()) < 2	)
		{    
			pPlayer->AddSendErrorMsg( ERROR_CODE_LEARN_SKILL );
			return false;
		}
	}
#endif

  if( pPlayer->m_iSkillCount >= MAX_OWN_SKILL )
  {
    pPlayer->AddSendErrorMsg( ERROR_CODE_LEARN_SKILL_MAX );
    return false;
  }
	// Get New Base Skill
	pNewLearnBase = g_pBase->GetBaseSkill( m_wSkillLearnId );
	if( pNewLearnBase == NULL )
  {
    pPlayer->AddSendErrorMsg( ERROR_CODE_LEARN_SKILL );
    return false;
  }
	// new Skill
	pNewLearnSkill = g_pGs->CreateCSkill( pNewLearnBase, 0, pPlayer, 1 );
	if( pNewLearnSkill == NULL )
  {
    pPlayer->AddSendErrorMsg( ERROR_CODE_LEARN_SKILL );
    return false;
  }
	// Add new Useable Skill To Player
	if( -1 == pPlayer->AddNewSkill( pNewLearnSkill ) )// 在AddNewSkill()里已经发送了消息到client
	{
    g_pGs->DeleteCSkill( pNewLearnSkill );
    pNewLearnSkill = NULL;
    pPlayer->AddSendErrorMsg( ERROR_CODE_LEARN_SKILL );
		return false;
	}
	return true;
}
//======================================================================================
// 
// 改变头、身体的颜色,m_bPosition为变色位置，0为头，1为身体
bool CSrvBaseSkill::Color(CPlayer *pPlayer)
{
	if( m_bPosition == 0 )
	{
		pPlayer->m_wPicId = ( m_bColor << 8 ) | ( pPlayer->m_wPicId & 0x00FF );
		return true;
	}
	else if( m_bPosition == 1 )
	{
		pPlayer->m_wPicId = ( pPlayer->m_wPicId & 0xFF00 ) | m_bColor;
		return true;
	}
	// 不需要发送消息, Client自己完成效果
	return false;
}
//======================================================================================
// 
// 传送功能
bool CSrvBaseSkill::Warp( CPlayer* pUser, CItem * pItem )
{
//#ifdef _DEBUG_CHECK_FUNC_STATE_
//  CCheckFuncState callStackCheck("CSrvBaseSkill::Warp()");
//#endif

	SWarpPoint		ptWarp;
	WORD					wRndX, wRndY, wWarpCounter;
	DWORD				  dwWarpMapFlag;
  //
  if( NULL == pUser->m_pInMap )     return false;
  //
	switch( m_bCase )
	{
		case WARP_APPOINT_POS:
      // 攻城战地图 攻城方 不能使用 随机传送卷轴
      //if( pUser->m_pInMap->IsCityWarMap() &&
      //    (!pUser->IsCityWarDefencer( pUser->GetMapId() )) )       return false;

      
			ptWarp.wTargetMapId = m_wWarpMapId;
			ptWarp.wTargetMapX   = m_wWarpX;
			ptWarp.wTargetMapY   = m_wWarpY;
			pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_TOOL );
			break;
		case WARP_RANDOM_POS:
			{
        // 攻城战地图 攻城方 不能使用 随机传送卷轴
        if( pUser->m_pInMap->IsCityWarMap() /*&&
            (!pUser->IsCityWarDefencer( pUser->GetMapId() ))*/ )       return false;

				// 船上不能用随机传送卷轴
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
        if( pUser->m_pInMap->IsBoatMap() )                           return false;
#endif
        //
				for( wWarpCounter = 0; wWarpCounter < 20; wWarpCounter++ )
				{
					wRndX = gf_GetRandom( pUser->m_pInMap->GetClientSizeX() );
					wRndY = gf_GetRandom( pUser->m_pInMap->GetClientSizeY() );
					dwWarpMapFlag = pUser->m_pInMap->GetTileFlag( wRndX, wRndY );
					if( !( dwWarpMapFlag & TILE_ALLOCCULDE ) )
					{
						ptWarp.wTargetMapId = pUser->GetMapId();
						ptWarp.wTargetMapX   = wRndX;
						ptWarp.wTargetMapY   = wRndY;
						pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_TOOL_RANDOM );
						return true;
					}
				}
				// 失败, 此次随机传送无效
        SAdjustPoint      *pAdjPoint = pUser->m_pInMap->GetAdjustPoint();
        if( pAdjPoint )
        {
          ptWarp.wTargetMapId = pUser->GetMapId();
          ptWarp.wTargetMapX   = pAdjPoint->wAdjustX;
          ptWarp.wTargetMapY   = pAdjPoint->wAdjustY;
        }
        else
        {
          ptWarp.wTargetMapId = pUser->GetMapId();
          ptWarp.wTargetMapX   = pUser->GetPosX();
          ptWarp.wTargetMapY   = pUser->GetPosY();
        }
        pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_TOOL_RANDOM );
			}
			break;
		case WARP_RELIVE_POS:
      // 攻城战地图不能使用 回城卷轴
      //if( pUser->m_pInMap->IsCityWarMap() )       return false;
      //
			ptWarp.wTargetMapId = pUser->m_dwSavedPoint;
			ptWarp.wTargetMapX   = pUser->m_iSavePoint_X;
			ptWarp.wTargetMapY   = pUser->m_iSavePoint_Y;
			pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_TOOL );
			break;
		case WARP_REMEMBER_POS:
      // 攻城战地图不能记忆 记点传送卷轴的坐标
        if( pUser->m_pInMap->IsCityWarMap() &&
            (!pUser->IsCityWarDefencer( pUser->GetMapId() )) )       return false;
      //
      if( pItem == NULL )   return true;
			pItem->m_iVar[0] = pUser->GetInMap()->GetMapId();
			pItem->m_iVar[1] = pUser->GetPosX();
			pItem->m_iVar[2] = pUser->GetPosY();
			pItem->m_wFuncDbc = m_wFuncChangeId;
      pItem->m_pSklDbClick = g_pBase->GetBaseSkill( m_wFuncChangeId );
      //pUser->Send_A_REMEMBERPOSOK();
			return true;
		case WARP_BASE_VAR:
      // 攻城战地图不能使用 记点传送卷轴
        if( pUser->m_pInMap->IsCityWarMap() &&
            (!pUser->IsCityWarDefencer( pUser->GetMapId() )) )       return false;
      //
      if( pItem == NULL )   return true;
			ptWarp.wTargetMapId = pItem->m_iVar[0];
			ptWarp.wTargetMapX   = pItem->m_iVar[1];
			ptWarp.wTargetMapY   = pItem->m_iVar[2];
			pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_TOOL );
			break;
    case WARP_REVIVE_POS:
      {
        // 攻城战地图 攻城方 不能使用 随机传送卷轴
        if( pUser->m_pInMap->IsCityWarMap() &&
            (!pUser->IsCityWarDefencer( pUser->GetMapId() )) )       return false;
        //
				for( wWarpCounter = 0; wWarpCounter < 20; wWarpCounter++ )
				{
					wRndX = gf_GetRandom( pUser->m_pInMap->GetClientSizeX() );
					wRndY = gf_GetRandom( pUser->m_pInMap->GetClientSizeY() );
					dwWarpMapFlag = pUser->m_pInMap->GetTileFlag( wRndX, wRndY );
					if( !( dwWarpMapFlag & TILE_ALLOCCULDE ) )
					{
						ptWarp.wTargetMapId = pUser->GetMapId();
						ptWarp.wTargetMapX   = wRndX;
						ptWarp.wTargetMapY   = wRndY;
						pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_GOAT_REVIVE );
						return true;
					}
				}
				// 失败, 此次随机传送无效
        SAdjustPoint      *pAdjPoint = pUser->m_pInMap->GetAdjustPoint();
        if( pAdjPoint )
        {
          ptWarp.wTargetMapId = pUser->GetMapId();
          ptWarp.wTargetMapX   = pAdjPoint->wAdjustX;
          ptWarp.wTargetMapY   = pAdjPoint->wAdjustY;
        }
        else
        {
          ptWarp.wTargetMapId = pUser->GetMapId();
          ptWarp.wTargetMapX   = pUser->GetPosX();
          ptWarp.wTargetMapY   = pUser->GetPosY();
        }
        pUser->Send_A_WARP( ptWarp, PLAYER_WARP_TYPE_GOAT_REVIVE );
			}
      break;
		default:
			return false;
	}
	return true;
}
//======================================================================================
//
// 改变ITEM的数值
void CSrvBaseSkill::SetItem(CItem *pItem)
{
	pItem->m_wMaxDurability += m_iItemHm;
	pItem->m_wDurability		+= m_iItemHp;
	pItem->m_wHard					+= m_iItemHard;
	pItem->m_wForging				+= m_iItemHu;
}
//======================================================================================
//
// 设置目标的五行
bool CSrvBaseSkill::ChangeElement(CLife * pUser, CSkill *pSkill)
{
  LPSNMSetSpecialStatus		        pSpecialStatus = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);;
  BYTE                            bElemCount = 0;

  //
  if( NULL == pUser->m_pInMap )     return false;
  //
	if( m_bTarget == SKILL_TARGET_SELF )
	{
		pUser->m_qwSpecialStatus |= SPE_STATUS_ELEMENT;
		pUser->m_wElement         = m_wElement;
		pUser->m_dwAlarmTime[22]  = ClientTickCount + m_iAlarmTime;
		pUser->m_wFuncAlarm[22]   = m_wAlarmSkillId;
    //
    pSpecialStatus->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
    pSpecialStatus->qwStatus     = pUser->m_qwSpecialStatus;
    //
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
		pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
		return true;
	}
	else if( m_bTarget == SKILL_TARGET_TEAM )
	{
		if( pUser->IsPlayer() )
		{
      CTeam                           *pElemTeam = NULL;
      CPlayer                         *pElemPlayer = NULL;
			if( NULL != ( pElemTeam = ((CPlayer*)pUser)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pElemPlayer = pElemTeam->GetMember( i ) ) &&
              pElemPlayer->GetStatus() != STATUS_PLAYER_DEAD &&
              pElemPlayer->GetLifeDis( pUser ) < MAX_SPE_STATUS_DIS )
					{
		        pElemPlayer->m_qwSpecialStatus |= SPE_STATUS_ELEMENT;
		        pElemPlayer->m_wElement         = m_wElement;
		        pElemPlayer->m_dwAlarmTime[22]  = ClientTickCount + m_iAlarmTime;
		        pElemPlayer->m_wFuncAlarm[22]   = m_wAlarmSkillId;
						// Send Message To Nearly Client
						pSpecialStatus->dwCode_Skill = ( pElemPlayer->GetCode() << 16 ) | m_wId;
						pSpecialStatus->qwStatus		 = pElemPlayer->m_qwSpecialStatus;
            pSpecialStatus++;
            bElemCount++;
					}
          if( bElemCount )
          {
            MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bElemCount;
            pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
          }
				}
			}
		}
		return true;
	}
	else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE )
	{
		pUser->m_qwSpecialStatus |= SPE_STATUS_ELEMENT;
		pUser->m_wElement         = m_wElement;
		pUser->m_dwAlarmTime[22]  = ClientTickCount + m_iAlarmTime;
		pUser->m_wFuncAlarm[22]   = m_wAlarmSkillId;
    //
    pSpecialStatus->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
    pSpecialStatus->qwStatus     = pUser->m_qwSpecialStatus;
    //
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
		pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    SMsgData* pMsg = g_pGs->NewMsgBuffer();
    if(pMsg)
    {
      pMsg->Init();
      pMsg->dwAID = A_ITEM2SKILL;
      pMsg->dwMsgLen = 2;
      pMsg->Msgs[0].Size = sizeof(WORD);
      pMsg->Msgs[1].Size = sizeof(WORD);
      *(WORD*)pMsg->Msgs[0].Data = 1; //被师法者
      *(WORD*)pMsg->Msgs[1].Data = GetType();
      ((CPlayer*)pUser)->AddSendMsg(pMsg);
    }
		return true;
    /*
    if( pSkill == NULL )    return true;
	  static list<CLife*>::iterator	  Iter_Elem;
	  CLife											      *pElemLife;
		for( Iter_Elem = pSkill->m_listTargets.begin(); Iter_Elem != pSkill->m_listTargets.end(); Iter_Elem++ )
    {
      pElemLife = (*Iter_Elem);
      if( pElemLife->IsMonster() &&
          ((CMonster*)pElemLife)->m_qwAntiStatus & SPE_STATUS_ELEMENT )
      {
        continue;
      }
      pElemLife->m_qwSpecialStatus |= SPE_STATUS_ELEMENT;
      pElemLife->m_wElement         = m_wElement;
      pElemLife->m_dwAlarmTime[22]  = ClientTickCount + m_iAlarmTime;
      pElemLife->m_wFuncAlarm[22]   = m_wAlarmSkillId;
      //
      pSpecialStatus->dwCode_Skill = ( pElemLife->GetCode() << 16 ) | m_wId;
      pSpecialStatus->qwStatus		 = pElemLife->m_qwSpecialStatus;
      pSpecialStatus++;
      bElemCount++;
    }
    if( bElemCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bElemCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
		return true;
    */
	}
	return false;
}
//======================================================================================
//
// 清除状态功能
//#define SPE_STATUS_POISON							0x00000200
//#define SPE_STATUS_BLIND							0x00000400
//#define SPE_STATUS_UNMOVE							0x00000800
//#define SPE_STATUS_UNATTACK						0x00001000
//#define SPE_STATUS_UNITEM							0x00002000
//#define SPE_STATUS_UNSEE							0x00004000
//#define SPE_STATUS_SEE								0x00008000
void CSrvBaseSkill::ClearStatus(CLife * pUser,CSkill* pSkill)
{
	LPSNMSetSpecialStatus		pSpecialStatus = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);;
	BYTE										bClearLifeCount = 0;

  //
  if( NULL == pUser->m_pInMap )     return;
	// 寻找Target
	if( m_bTarget == SKILL_TARGET_SELF )
	{
		pUser->ClearSpecialStatus( m_qwStatus );
	  // Send Message To Nearly Client
	  pSpecialStatus->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
		pSpecialStatus->qwStatus		 = pUser->m_qwSpecialStatus;
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );
		pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
	}
	else if( m_bTarget == SKILL_TARGET_TEAM )
	{
		if( pUser->IsPlayer() )
		{
	    CTeam										*pClearTeam    = NULL;
	    CPlayer									*pClearPlayer  = NULL;
			if( NULL != ( pClearTeam = ((CPlayer*)pUser)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pClearPlayer = pClearTeam->GetMember( i ) ) &&
              pClearPlayer->GetStatus() != STATUS_PLAYER_DEAD &&
              pClearPlayer->GetLifeDis( pUser ) < MAX_SPE_STATUS_DIS )
					{
						pClearPlayer->ClearSpecialStatus( m_qwStatus );
						// Send Message To Nearly Client
						pSpecialStatus->dwCode_Skill = ( pClearPlayer->GetCode() << 16 ) | m_wId;
						pSpecialStatus->qwStatus		 = pClearPlayer->m_qwSpecialStatus;
            pSpecialStatus++;
            bClearLifeCount++;
					}
          if( bClearLifeCount )
          {
            MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bClearLifeCount;
            pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
          }
				}
			}
		}
    else if( pUser->IsMonster() )
    {
      // Monster Revive All Teamer
      CMonsterTeamers     *pMstTeam = NULL;
      WORD                wMstCount = 0;
      CMonster            **pMonsters = NULL;

      if( NULL != ( pMstTeam = ((CMonster*)(pUser))->GetTeam() ) )
      {
        wMstCount = pMstTeam->GetCount();
        pMonsters = pMstTeam->GetMonsters();
        for( int i = 0; i < wMstCount; i++ )
        {
          if( pMonsters[i] &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_DEAD &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_GOTO_DEAD &&
              pMonsters[i]->GetDistance( pUser->GetPosX(), pUser->GetPosY() ) < MAX_SPE_STATUS_DIS )
          {
						pMonsters[i]->ClearSpecialStatus( m_qwStatus );
            //
            pSpecialStatus->dwCode_Skill      = ( pMonsters[i]->GetSelfCode() << 16 ) | m_wId;
            pSpecialStatus->qwStatus          = pMonsters[i]->GetSpecialStatus();
            pSpecialStatus++;
            bClearLifeCount++;
          }
        }
      }
      else
      {
		    pUser->ClearSpecialStatus( m_qwStatus );
        //
	      pSpecialStatus->dwCode_Skill = ( pUser->GetCode() << 16 ) | m_wId;
		    pSpecialStatus->qwStatus		 = pUser->m_qwSpecialStatus;
        bClearLifeCount++;
      }
    }
    if( bClearLifeCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bClearLifeCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
	}
	else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE )
	{
    if( pSkill == NULL )    return;
    //
    static LifeTileList::iterator	  Iter_Hl;
	  CLife										        *pClearLife    = NULL;
    //
		for( Iter_Hl = pSkill->m_listTargets.begin(); Iter_Hl != pSkill->m_listTargets.end(); Iter_Hl++ )
    {
      pClearLife = (*Iter_Hl);
      pClearLife->ClearSpecialStatus( m_qwStatus );
      pSpecialStatus->dwCode_Skill = ( pClearLife->GetCode() << 16 ) | m_wId;
      pSpecialStatus->qwStatus		 = pClearLife->m_qwSpecialStatus;
      pSpecialStatus++;
      bClearLifeCount++;
    }
    if( bClearLifeCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bClearLifeCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
	}
}
//======================================================================================
//
// 把一个ITEM更换为另外一个ITEM
void CSrvBaseSkill::ChangeItem(CItem* pItemForChange)
{
	pItemForChange->m_pBase = g_pBase->GetBaseItem( m_wChangeItemId );
}
//======================================================================================
//
//妖刀功能（不用再此处处理,已经在入魂功能中处理了）
void CSrvBaseSkill::EvilSword(CItem * pItem)
{
	
}
//======================================================================================
//
//镶嵌功能（不用再此处处理，已经在道具镶嵌中处理了）
void CSrvBaseSkill::Insert()
{

}
//======================================================================================
//
// 在一个ITEM上设置Alarm
void CSrvBaseSkill::SetAlarm(CItem * pItem)
{
	pItem->m_iAlarmTime   = m_iAlarmTime;
	pItem->m_wFuncAlarm   = m_wAlarmSkillId;
	pItem->m_iTriggerTime = m_iTriggerTime;
	pItem->m_wFuncTrigger = m_wTriggerSkillId;
}
//======================================================================================
//
// 更换招式五行
void CSrvBaseSkill::SetSkillEle(CSkill * pSkill)
{
  // 只能设置物理攻击招式的五行
  if( pSkill->GetType() >= SKILL_TYPE_WIZARD_ATTACK )    return;
  //
  if( m_wElement & 0x1F )
  {
    pSkill->m_wElement &= 0xFFE0;
	  pSkill->m_wElement |= m_wElement;
  }
  else
  {
	  pSkill->m_wElement |= m_wElement;
  }
  pSkill->m_wElementCount = 0;
  memset( pSkill->m_wSimElement, 0, sizeof( WORD ) * 16 );
  for( int i = 0; i < 16; i++ )
  {
    if( ( pSkill->m_wElement & ( 1 << i ) ) )
    {
      pSkill->m_wSimElement[pSkill->m_wElementCount] = ( 1 << i );
      pSkill->m_wElementCount++;
    }
  }
}
//======================================================================================
//
//
void CSrvBaseSkill::ChangeBearPos(CLife *pUser, CSkill *pSkill)
{
	LPSNMSetSpecialStatus		pChangeBear = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);
	BYTE										bCharBearCount = 0;

  //
  if( NULL == pUser->m_pInMap )     return;
  //
  if( m_bTarget == SKILL_TARGET_SELF )
  {
    pUser->m_qwSpecialStatus|= SPE_STATUS_UNINJURED;
    pUser->m_dwAlarmTime[25] = ClientTickCount + m_iAlarmTime;
    pUser->m_wFuncAlarm[25]  = m_wAlarmSkillId;
    pUser->m_iBearPsb				+= m_iBearPsbChange;
    //
    pChangeBear->dwCode_Skill      = ( pUser->GetCode() << 16 ) | m_wId;
    pChangeBear->qwStatus          = pUser->GetSpecialStatus();
    //
    MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bCharBearCount;
    pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
  }
  else if( m_bTarget == SKILL_TARGET_TEAM )
  {
    if( pUser->IsPlayer() )
		{
      CPlayer     *pBearPlayer  = NULL;
      CTeam       *pBearTeam    = NULL;
      //
			if( NULL != ( pBearTeam = ((CPlayer*)pUser)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pBearPlayer = pBearTeam->GetMember( i ) ) &&
              pBearPlayer->GetStatus() != STATUS_PLAYER_DEAD &&
              pBearPlayer->GetLifeDis( pUser ) < MAX_SPE_STATUS_DIS )
          {
            pBearPlayer->m_qwSpecialStatus|= SPE_STATUS_UNINJURED;
            pBearPlayer->m_dwAlarmTime[25] = ClientTickCount + m_iAlarmTime;
            pBearPlayer->m_wFuncAlarm[25]  = m_wAlarmSkillId;
            pBearPlayer->m_iBearPsb				+= m_iBearPsbChange;
            //
            pChangeBear->dwCode_Skill      = ( pBearPlayer->GetSelfCode() << 16 ) | m_wId;
            pChangeBear->qwStatus          = pBearPlayer->GetSpecialStatus();
            pChangeBear++;
            bCharBearCount++;
          }
        }
      }
      else
      {
        pUser->m_qwSpecialStatus|= SPE_STATUS_UNINJURED;
        pUser->m_dwAlarmTime[25] = ClientTickCount + m_iAlarmTime;
        pUser->m_wFuncAlarm[25]  = m_wAlarmSkillId;
        pUser->m_iBearPsb				+= m_iBearPsbChange;
        //
        pChangeBear->dwCode_Skill      = ( pUser->GetCode() << 16 ) | m_wId;
        pChangeBear->qwStatus          = pUser->GetSpecialStatus();
        bCharBearCount++;
      }
    }
    else if( pUser->IsMonster() )
    {
      // Monster Revive All Teamer
      CMonsterTeamers     *pMstTeam = NULL;
      WORD                wMstCount = 0;
      CMonster            **pMonsters = NULL;

      if( NULL != ( pMstTeam = ((CMonster*)(pUser))->GetTeam() ) )
      {
        wMstCount = pMstTeam->GetCount();
        pMonsters = pMstTeam->GetMonsters();
        for( int i = 0; i < wMstCount; i++ )
        {
          if( pMonsters[i] &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_DEAD &&
              pMonsters[i]->GetStatus() != STATUS_MONSTER_GOTO_DEAD &&
              pMonsters[i]->GetDistance( pUser->GetPosX(), pUser->GetPosY() ) < MAX_SPE_STATUS_DIS )
          {
            pMonsters[i]->m_qwSpecialStatus|= SPE_STATUS_UNINJURED;
            pMonsters[i]->m_dwAlarmTime[25] = ClientTickCount + m_iAlarmTime;
            pMonsters[i]->m_wFuncAlarm[25]  = m_wAlarmSkillId;
            pMonsters[i]->m_iBearPsb				+= m_iBearPsbChange;
            //
            pChangeBear->dwCode_Skill      = ( pMonsters[i]->GetSelfCode() << 16 ) | m_wId;
            pChangeBear->qwStatus          = pMonsters[i]->GetSpecialStatus();
            pChangeBear++;
            bCharBearCount++;
          }
        }
      }
      else
      {
        pUser->m_qwSpecialStatus|= SPE_STATUS_UNINJURED;
        pUser->m_dwAlarmTime[25] = ClientTickCount + m_iAlarmTime;
        pUser->m_wFuncAlarm[25]  = m_wAlarmSkillId;
        pUser->m_iBearPsb				+= m_iBearPsbChange;
        //
        pChangeBear->dwCode_Skill      = ( pUser->GetCode() << 16 ) | m_wId;
        pChangeBear->qwStatus          = pUser->GetSpecialStatus();
        bCharBearCount++;
      }
    }
    if( bCharBearCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bCharBearCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
  }
  else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE )
  {
    if( pSkill == NULL )      return;
    //
    CLife                           *pBPLife = NULL;
    static LifeTileList::iterator	  Iter_Rv;
    //
    for( Iter_Rv = pSkill->m_listTargets.begin(); Iter_Rv != pSkill->m_listTargets.end(); Iter_Rv++ )
    {
      pBPLife = (*Iter_Rv);
      if( pBPLife->IsMonster() &&
          ((CMonster*)pBPLife)->m_qwAntiStatus & SPE_STATUS_UNINJURED )
      {
        continue;
      }
      pBPLife->m_qwSpecialStatus|= SPE_STATUS_UNINJURED;
      pBPLife->m_dwAlarmTime[25] = ClientTickCount + m_iAlarmTime;
      pBPLife->m_wFuncAlarm[25]  = m_wAlarmSkillId;
      pBPLife->m_iBearPsb				+= m_iBearPsbChange;
      //
      pChangeBear->dwCode_Skill      = ( pBPLife->GetCode() << 16 ) | m_wId;
      pChangeBear->qwStatus          = pBPLife->GetSpecialStatus();
      pChangeBear++;
      bCharBearCount++;
    }
    if( bCharBearCount )
    {
      MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bCharBearCount;
      pUser->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pUser->m_iX, pUser->m_iY );
    }
  }
}
//======================================================================================
//
// 复活
void CSrvBaseSkill::Revive(CLife *pUser, CSkill *pSkill)
{
  CPlayer     *pRvPlayer = NULL;
  WORD        wSkillId = 0;

  if( pSkill )    wSkillId = pSkill->GetId();
  if( m_bTarget == SKILL_TARGET_SELF )
  {
    if( pUser->IsPlayer() )
    {
      pRvPlayer = (CPlayer*)(pUser);
      if( pRvPlayer->GetStatus() == STATUS_PLAYER_DEAD )
      {
        SetSpecialState(pRvPlayer);  // Set Revive Status
        if(m_iHpChange>65535&&m_iHpChange<=65635)
        {
          pRvPlayer->AddHp( pRvPlayer->GetMaxHp()* (m_iHpChange-65535) / 100 );
          pRvPlayer->Revive( REVIVE_BY_MAGIC, wSkillId );
        }
        else
        {
          pRvPlayer->AddHp( m_iHpChange );
          pRvPlayer->Revive( REVIVE_BY_MAGIC, wSkillId );
        }
      }
    }
  }
  else if( m_bTarget == SKILL_TARGET_TEAM )
  {
    if( pUser->IsPlayer() )
		{
      CTeam       *pReviveTeam = NULL;
			if( NULL != ( pReviveTeam = ((CPlayer*)pUser)->GetTeam() ) )
			{
				for( int i = 0; i < MAX_TEAM_MEMBER; i++ )
				{
					if( NULL != ( pRvPlayer = pReviveTeam->GetMember( i ) ) &&
            pRvPlayer->GetStatus() == STATUS_PLAYER_DEAD &&
            pRvPlayer->GetLifeDis( pUser ) < MAX_SPE_STATUS_DIS )
          {
            SetSpecialState(pRvPlayer);
            if(m_iHpChange>65535&&m_iHpChange<=65635)
            {
              pRvPlayer->AddHp( pRvPlayer->GetMaxHp()* (m_iHpChange-65535) / 100 );
              pRvPlayer->Revive( REVIVE_BY_MAGIC, wSkillId );
            }
            else
            {
              pRvPlayer->AddHp( m_iHpChange );
              pRvPlayer->Revive( REVIVE_BY_MAGIC, wSkillId );
            }
          }
        }
      }
    }
    else if( pUser->IsMonster() )
    {
      // Monster Revive All Teamer
      // ...
    }
  }
  else if( m_bTarget == SKILL_TARGET_OBJECT || m_bTarget == SKILL_TARGET_NONE )
  {
    if( pUser->IsPlayer() )
    {
      if( pSkill == NULL )      return;
      //
      static LifeTileList::iterator	  Iter_Rv;
      CLife                           *pReviver = NULL;
      //
      for( Iter_Rv = pSkill->m_listTargets.begin(); Iter_Rv != pSkill->m_listTargets.end(); Iter_Rv++ )
      {
        pReviver = (*Iter_Rv);
        if( pReviver->IsPlayer() )
        {
          pRvPlayer = (CPlayer*)(pReviver);
          if( pRvPlayer->GetStatus() == STATUS_PLAYER_DEAD )
          {
            SetSpecialState(pRvPlayer);          // Set Revive Status
            if(m_iHpChange>65535&&m_iHpChange<=65635)
            {     
              pRvPlayer->AddHp( pRvPlayer->GetMaxHp()* (m_iHpChange-65535) / 100 );
              pRvPlayer->Revive( REVIVE_BY_MAGIC, wSkillId );
            }
            else
            {
              pRvPlayer->AddHp( m_iHpChange );
              pRvPlayer = (CPlayer*)(pReviver);
              pRvPlayer->Revive( REVIVE_BY_MAGIC, wSkillId );
            }
          }
        }
      }
    }
    else if( pUser->IsMonster() )
    {
      // Monster Revive Other Monster
      // ...
    }
  }
}
//======================================================================================
//
//item2skill复活
#ifdef _REVIVE_JUJU_
BOOL CSrvBaseSkill::ReviveItem2Skill(CPlayer *pPlayer)
{
  if( pPlayer->GetStatus() == STATUS_PLAYER_DEAD )
  {
    SetSpecialState(pPlayer);
    if(m_iHpChange>65535&&m_iHpChange<=65635)
    {
      pPlayer->AddHp( pPlayer->GetMaxHp()* (m_iHpChange-65535) / 100 );
      pPlayer->Revive( REVIVE_BY_ITEM, GetId() );
    }
    else
    {
      pPlayer->AddHp( m_iHpChange );
      pPlayer->Revive( REVIVE_BY_ITEM, GetId() );
    }
    return TRUE;
  }
  return FALSE;
}
#endif
//==============================================================================
//迟缓
#ifdef _SLOW_JUJU_
void CSrvBaseSkill::SlowJuJuItem2Skill(CPlayer *pPlayer, CSrvBaseSkill *pSkill)
{
  if(!pPlayer)
    return;
  SetSpecialState((CLife*)pPlayer);
  pPlayer->Send_A_SETDYSFUNC(NULL,GetId());
  return;
}
#endif
//====================================================================================
//
//
#ifdef DEL_INSERT
BOOL CSrvBaseSkill::DelInsert(CPlayer * pUser, int iWhere)
{
  CItem* pEquip = NULL;
  if( !pUser )  
    return FALSE;
  pEquip = pUser->GetItemByPos( iWhere );
  if( !pEquip ) 
    return FALSE;
  SMsgData* pItemMsg = g_pGs->NewMsgBuffer(pUser->GetSelfCode());
  if( !pItemMsg )
    return FALSE;
  for(int j = 0; j < MAX_ITEM_SLOT; j++ )
  {
    int iItemId = pEquip->m_wTesseraItemID[j];
    CSrvBaseItem *pItemSlot = g_pBase->GetBaseItem(iItemId);
    if(pItemSlot)
      pEquip->DelTessera(pItemSlot);
  }
  pUser->UncalTesseraResult( pEquip );
  pUser->Send_A_UPDATEHPMPSP( FALSE );
  pUser->Send_A_UPDATESOUL( NULL );
  for(int i = 0; i < MAX_ITEM_SLOT; i++)
  {
    pEquip->m_wTesseraItemID[i] = 0;
    pEquip->m_pSklTessera[i]  = NULL;
  }
  pEquip->m_wTesseraNum = 0;
  pItemMsg->Init();
  pItemMsg->dwAID = A_ITEMDISENCHASE;
  pItemMsg->dwMsgLen = 1;
  pItemMsg->Msgs[0].Size = sizeof(SPlayerItemExC);
  pUser->Get_SPlayerItemExC( pEquip, (SPlayerItemExC*)(pItemMsg->Msgs[0].Data) );
  pUser->AddSendMsg(pItemMsg);
  return TRUE;
}
#endif
//====================================================================================
//
// Change Player Sex
#ifdef FUNCTION_DENATURALIZATION
BOOL CSrvBaseSkill::ChangeSex(CPlayer *pUser, CItem *pDbcItem)
{
  //if(!pDbcItem->CheckLimit(pUser)) { return FALSE; }
  //
  WORD     wOccupation = -1;
  SMsgData *pTheMsg    = g_pGs->NewMsgBuffer();
 
  if(pTheMsg == NULL)  { return FALSE; }
  //
  if ((pUser->m_dwBetrothal == 0) && (pUser->m_pInMap != NULL) && !(pUser->IsTalkingWithNpc()))
  {
    wOccupation = pUser->GetOccupation();
    switch(wOccupation) 
    {
    case OCCU_SWORDMAN:
      pUser->m_wOccupation = OCCU_SWORDMANF;
      break;
    case OCCU_SWORDMANF:
      pUser->m_wOccupation = OCCU_SWORDMAN;
      break;
    case OCCU_BLADEMAN:
      pUser->m_wOccupation = OCCU_BLADEMANF;
      break;
    case OCCU_BLADEMANF:
      pUser->m_wOccupation = OCCU_BLADEMAN;
      break;
    case OCCU_PIKEMAN:
      pUser->m_wOccupation = OCCU_PIKEMANF;
      break;
    case OCCU_PIKEMANF:
      pUser->m_wOccupation = OCCU_PIKEMAN;
      break;
    case OCCU_WIZARD:
      pUser->m_wOccupation = OCCU_WIZARDF;
      break;
    case OCCU_WIZARDF:
      pUser->m_wOccupation = OCCU_WIZARD;
      break;
    default:
      pUser->AddSendErrorMsg(ERROR_CODE_OTHER, pTheMsg);
      return FALSE;
      break;
    }
    //
    pTheMsg->Init();
    pTheMsg->dwAID        = A_DENATURALI;
    pTheMsg->dwMsgLen     = 1;
    pTheMsg->Msgs[0].Size = sizeof(WORD);
    *(DWORD*)pTheMsg->Msgs[0].Data = pUser->m_wOccupation;
    pUser->AddSendMsg(pTheMsg);
    return TRUE;    
  }
  // Error MSG
  if(pUser->m_dwBetrothal == 1) //已经订婚
  {
    pUser->AddSendErrorMsg(ERROR_CODE_ESPOUSAL, pTheMsg);
    return FALSE;
  }
  if (pUser->m_dwBetrothal == 2) //已经结婚
  {   
    pUser->AddSendErrorMsg(ERROR_CODE_MARRY, pTheMsg);
    return FALSE;
  } 
  pUser->AddSendErrorMsg(ERROR_CODE_OTHER, pTheMsg);
  return FALSE;
}
#endif 
//====================================================================================
//
//音乐盒
#ifdef _MUSIC_BOX_
BOOL CSrvBaseSkill::UseMusicBox(CLife *pUser, CItem *pDbcItem)
{
  CPlayer * pPlayer = (CPlayer*)(pUser );
  SMsgData * pTheMsg = NULL;
  int iVar = 0;
	if( !( pDbcItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_MUSICBOX ) )
	{
		return FALSE;
	}
  /*
	if( !pDbcItem->CheckLimit( pPlayer ))
	{
		return FALSE;
	}
  */
  //
  iVar = pDbcItem->GetVar(0);

  CGameMap *pGameMap = pPlayer->GetInMap();
  if( NULL==pGameMap )
  {
		return FALSE;
  }
  //
  SGameMapType *pMapType = g_pBase->GetMapType( pGameMap->GetMapId() );
  if( pMapType && (pMapType->wLimit == 1 || pMapType->wLimit == 2 || pMapType->wLimit == 3 || pMapType->wLimit == 5) )
  {
    return FALSE;
  }
  //
  pTheMsg = g_pGs->NewMsgBuffer();
  if(NULL != pTheMsg)
  {
    pTheMsg->Init();
    pTheMsg->dwAID = A_USEMUSICBOX;
    pTheMsg->dwMsgLen = 1;
    pTheMsg->Msgs[0].Size    = sizeof(int);
    *(int*)(pTheMsg->Msgs[0].Data) = iVar;
    pGameMap->SendMsgNearPosition( *pTheMsg, pPlayer->GetPosX(), pPlayer->GetPosY() );
    pPlayer->AddSendMsg(pTheMsg);
  }
  return TRUE;  
}
#endif
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
//====================================================================================
//
//
void CSrvBaseSkill::SetPlayerSuitSkill( CPlayer * pPlayer )
{
  switch( m_wType )
  {
  case SKILL_TYPE_SUIT_PLAYER_POINT:
    pPlayer->m_iSuitAp      += m_iApChange;
    pPlayer->m_iSuitHit     += m_iHitChange;
    pPlayer->m_iSuitDp      += m_iDpChange;
    pPlayer->m_iSuitDodge   += m_iDgChange;
    pPlayer->m_iSuitInt     += m_iIntChange;
    //
    pPlayer->m_iSuitMaxHp   += m_iHpChange;
    pPlayer->m_iSuitMaxMp   += m_iMpChange;
    pPlayer->m_iSuitMaxSp   += m_iSpChange;
    //
		pPlayer->m_iSuitMaxSoul  += m_iSoulChange;
    pPlayer->m_iSuitSpeed   += m_iTimes;
    break;
  case SKILL_TYPE_SUIT_PLAYER_COST:
    pPlayer->m_iSuitCostHp  += m_iHpChange;
    pPlayer->m_iSuitCostMp  += m_iMpChange;
    pPlayer->m_iSuitCostSp  += m_iSpChange;
    break;
  case SKILL_TYPE_SUIT_PLAYER_ATTACK:
    pPlayer->m_iSuitSuckHp  += m_iHpChange;
    pPlayer->m_iSuitSuckMp  += m_iMpChange;
    pPlayer->m_iSuitSuckSp  += m_iSpChange;
		pPlayer->m_iSuitSuckOdds+= m_iProbability;

    {
      if( m_wFuncChangeId )
      {
        CSrvBaseSkill         *pBaseSkill = g_pBase->GetBaseSkill( m_wFuncChangeId );
        if( pBaseSkill )      pPlayer->m_listSuitSkill.push_back( pBaseSkill );
      }
    }
    break;
  case SKILL_TYPE_SUIT_PLAYER_DEFENCE:
    pPlayer->m_iSuitRebound			+= m_iHpChange;
 		pPlayer->m_iSuitReboundOdds += m_iProbability;
		pPlayer->m_iSuitCritical		+= m_iCriticalHit;
    break;
  default:
    break;
  }
}
#endif

//************************************************************************************
//
// public:		检查消费能力
//
// return: false表示没有足够的点数使用SKILL
//
//************************************************************************************
bool CSrvBaseSkill::CheckCost(CPlayer *pTheUser)
{
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  if( pTheUser->HaveSuitInEquip() )
  {
	  if( ( pTheUser->m_iMp    < ( m_iCostMp * ( 100 - pTheUser->m_iSuitCostMp ) / 100 ) ) ||
		    ( pTheUser->m_iSp    < ( m_iCostSp * ( 100 - pTheUser->m_iSuitCostSp ) / 100 ) ) ||
			  ( pTheUser->m_iHp    < ( m_iCostHp * ( 100 - pTheUser->m_iSuitCostHp ) / 100 ) ) ||
			  ( pTheUser->m_dwSoul < m_iCostSoul ) )
	  {
		  return false;
	  }
  }
  else
#endif
  {
    if( ( pTheUser->m_iMp    < m_iCostMp ) ||
		    ( pTheUser->m_iSp    < m_iCostSp ) ||
			  ( pTheUser->m_iHp    < m_iCostHp ) ||
			  ( pTheUser->m_dwSoul < m_iCostSoul ) )
	  {
		  return false;
	  }
  }
	return true;
}
//************************************************************************************
//
// public: 消费计算
//
//************************************************************************************
bool CSrvBaseSkill::DoCost(CPlayer *pTheUser)
{
  static int iTheHp, iTheMp, iTheSp, iSoul = 0;

#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  if( pTheUser->HaveSuitInEquip() )
  {
    iTheSp = pTheUser->GetSp()   - ( m_iCostSp * ( 100 - pTheUser->m_iSuitCostSp ) / 100 );
	  iTheMp = pTheUser->GetMp()   - ( m_iCostMp * ( 100 - pTheUser->m_iSuitCostMp ) / 100 );
	  iTheHp = pTheUser->GetHp()   - ( m_iCostHp * ( 100 - pTheUser->m_iSuitCostHp ) / 100 );
    iSoul  = pTheUser->GetSoul() - m_iCostSoul;
  }
  else
#endif
  {
    iTheSp = pTheUser->GetSp()   - m_iCostSp;
	  iTheMp = pTheUser->GetMp()   - m_iCostMp;
	  iTheHp = pTheUser->GetHp()   - m_iCostHp;
    iSoul  = pTheUser->GetSoul() - m_iCostSoul;
  }
	if( iTheSp < 0 || iTheMp < 0 || iTheHp <= 0 || iSoul < 0 )
	{
		return false;
	}
	// Set Player Cost Data
	pTheUser->SetPlayerHpMpSpSoul( iTheHp, iTheMp, iTheSp, iSoul );
  return true;
}
//=======================================================================================
WORD CSrvBaseSkill::GetBitCountForRace( const DWORD & dwData )
{
  static WORD      g_BitCount2 = 0;

  g_BitCount2 = 0;
  for( int i = 0; i < m_wRaceCount; i++ )
  {
    if( dwData & m_dwSimRaceAttr[i] ) g_BitCount2++;
  }
  return g_BitCount2;
}
////////////////////////////////////////////////////////////////////////////////////////
//																																										//
//																																										//
//                 class CSkill implement																							//
//																																										//
//																																										//
////////////////////////////////////////////////////////////////////////////////////////
//constructor
CSkill::CSkill()
{
  m_dwSrvCode  = 0;
  m_wMemState = 0;
}
//========================================================================================
//
// Destructor
CSkill::~CSkill()
{
	m_listTargets.clear();
	m_listMiss.clear();
}
//========================================================================================
//
bool CSkill::DoWizardCost(CPlayer *pTheUser, CWizardSkillData * pNowWizardData)
{
  static int iTheHp, iTheMp, iTheSp, iSoul;
  //
  if( pNowWizardData == NULL )      return false;
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  if( pTheUser->HaveSuitInEquip() )
  {
    iTheSp = pTheUser->GetSp()   - ( pNowWizardData->m_iFinalCostSp * ( 100 - pTheUser->m_iSuitCostSp ) / 100 );
	  iTheMp = pTheUser->GetMp()   - ( pNowWizardData->m_iFinalCostMp * ( 100 - pTheUser->m_iSuitCostMp ) / 100 );
	  iTheHp = pTheUser->GetHp()   - ( pNowWizardData->m_iFinalCostHp * ( 100 - pTheUser->m_iSuitCostHp ) / 100 );
    iSoul  = pTheUser->GetSoul() - pNowWizardData->m_iFinalCostSoul;
  }
  else
#endif
  {
    iTheSp = pTheUser->GetSp()   - pNowWizardData->m_iFinalCostSp;
	  iTheMp = pTheUser->GetMp()   - pNowWizardData->m_iFinalCostMp;
	  iTheHp = pTheUser->GetHp()   - pNowWizardData->m_iFinalCostHp;
    iSoul  = pTheUser->GetSoul() - pNowWizardData->m_iFinalCostSoul;
  }
	if( iTheSp < 0 || iTheMp < 0 || iTheHp <= 0 || iSoul < 0)
	{
    return false;
	}
	// Set Player Cost Data
	pTheUser->SetPlayerHpMpSpSoul( iTheHp, iTheMp, iTheSp, iSoul );
  return true;
}
//========================================================================================
//
// private: 计算招式之间的加成
//
// return :返回加成
//
//========================================================================================
inline WORD CSkill::GetChainHitRate()
{
	BYTE	chainRate;
	BYTE	curIndex;
	BYTE	preIndex;

  curIndex = m_pBase->GetType() - SKILL_TYPE_OCCU_A1;
	// check pre skill type,and anew set pre skill type
	preIndex = m_pPlayer->GetPreSkillType() - SKILL_TYPE_OCCU_A1;
	m_pPlayer->SetPreSkillType( m_pBase->GetType() );
 	if (preIndex >= 10)
	{
		m_pPlayer->SetHitBonus(0);
		return 0;
	}
  // Calculate the bonus
  switch(m_pPlayer->GetOccupation())
  {
    case OCCU_BLADEMAN:
    case OCCU_BLADEMANF:
      chainRate = g_iBlade_HitBonus[preIndex][curIndex];
			break;
    case OCCU_SWORDMAN:
    case OCCU_SWORDMANF:
      chainRate = g_iSword_HitBonus[preIndex][curIndex];
			break;
    case OCCU_PIKEMAN:
    case OCCU_PIKEMANF:
      chainRate = g_iPike_HitBonus[preIndex][curIndex];
			break;
    default:
      chainRate = 0;
  }
	if (chainRate == 0)	m_pPlayer->SetHitBonus(0);
	else								m_pPlayer->AddHitBonus(chainRate);
	return m_pPlayer->GetHitBonus();
}
//==========================================================================================
//desc:				将这个SKILL升级到另一个SKILL，
//parameter:	new base skill 
//return   :  bool
//==========================================================================================
BOOL CSkill::UpdateLevel( CSrvBaseSkill * pBaseSkill, SMsgData * pTheMsg )
{
//#ifdef _DEBUG_CHECK_FUNC_STATE_
//  CCheckFuncState callStackCheck("CSkill::UpdateLevel()");
//#endif

	if( pBaseSkill )
	{
    if( IsPlayer() )
    {
      // send msg to player
		  if( pTheMsg == NULL )
      {
        pTheMsg = g_pGs->NewMsgBuffer( m_pPlayer->GetCode() );
		    if( pTheMsg == NULL )		return FALSE;
      }
		  // Send Client The Skill Update
		  pTheMsg->Init();
		  pTheMsg->dwAID      = A_UPDATESLEVEL;
		  pTheMsg->dwMsgLen   = 1;
		  pTheMsg->Msgs[0].Size = sizeof(SUpdateLevelInfo);

		  SUpdateLevelInfo *pInfo = (SUpdateLevelInfo*)pTheMsg->Msgs[0].Data;
		  pInfo->wOldId						= m_pBase->GetId();
		  pInfo->wNewId						= pBaseSkill->GetId();
      pInfo->wPos             = m_wPos;
		  m_pPlayer->AddSendMsg( pTheMsg );
      // update advantage skill
		  m_pBase				 = pBaseSkill;
		  m_iExp				 = 0;
		  return TRUE;
    }
	}
  if( pTheMsg )
  {
    g_pGs->ReleaseMsg( pTheMsg );
  }
	return FALSE;
}
//==========================================================================================
//desc:				物理攻击产生的特殊状态
//parameter:	
//return   :  bool
//==========================================================================================
inline void CSkill::CalAllSpecialStatus( CLife *pTarget )
{
  switch( GetColor() )
  {
  case SKILL_TYPE_ITEM_SET_STATUS:
	  m_pBase->SetSpecialState( pTarget );  
    break;
  case SKILL_TYPE_ITEM_HEAL1:
    m_pBase->Heal1( pTarget );
    break;
  case SKILL_TYPE_ITEM_HEAL2:
    m_pBase->Heal2( pTarget );
    break;
  case SKILL_TYPE_ITEM_HEAL3:
    m_pBase->Heal3( pTarget );
    break;
  case SKILL_TYPE_ITEM_HEAL4:
    m_pBase->Heal4( pTarget );
    break;
  case SKILL_TYPE_ITEM_CHANGE_CHAR:
	  m_pBase->ChangeChar( pTarget );
    break;
  case SKILL_TYPE_ITEM_CLEAR_STATUS:
	  pTarget->ClearSpecialStatus( m_pBase->m_qwStatus );
    break;
  default:
    break;
  }
}
//==========================================================================================
//desc:				玩家使用辅助魔法
//parameter:	
//return   :  bool
//==========================================================================================
void CSkill::PlayerDoMagicDamage()
{
  switch( GetType() )
  {
  case SKILL_TYPE_ITEM_HEAL1:
  case SKILL_TYPE_ITEM_HEAL2:
  case SKILL_TYPE_ITEM_HEAL3:
  case SKILL_TYPE_ITEM_HEAL4:
	  m_pBase->Heal( m_pPlayer, this );
    break;
  case SKILL_TYPE_ITEM_SET_STATUS:
	  m_pBase->DoSetSpecialState( m_pPlayer, this );
    break;
  case SKILL_TYPE_ITEM_CHANGE_CHAR:
	  m_pBase->DoChangeChar( m_pPlayer, this );
    break;
  case SKILL_TYPE_ITEM_CLEAR_STATUS:
	  m_pBase->ClearStatus( m_pPlayer, this );
    break;
  case SKILL_TYPE_ITEM_CHANGE_ELEMENT:
	  m_pBase->ChangeElement( m_pPlayer, this );
    break;
  case SKILL_TYPE_ITEM_WARP:
    m_pBase->Warp( m_pPlayer, NULL );
    break;
  case SKILL_TYPE_ITEM_RESURRECT:
    m_pBase->Revive( m_pPlayer, this );
    break;
  case SKILL_TYPE_CHANGE_BEARPOS:
    m_pBase->ChangeBearPos( m_pPlayer, this );
    break;
  case SKILL_TYPE_CLEAR_TRAP:
    m_pBase->ClearTrap( m_pPlayer );
    break;

#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_
	case SKILL_TYPE_GUILDMASTER_SKILL:
		m_pBase->DoGuildSkill( this );
		break;

#endif

  default:
    break;
  }
}
//==========================================================================================
//desc:				Trap对目标作攻击
//parameter:	
//return   :  bool
//==========================================================================================
#define _DEBUG_TRAP_MUST_DAMAGE_
inline void CSkill::DoDamageForTrap( CMagic *pMagic )
{
  static WORD           wMaxDmgCount1  = ( MAXMSGDATASIZE / sizeof( SNMDamage ) ) - 2;
         LifeTileIter   Iter_Tr;
  static CLife          *pTrapTgt     = NULL;
  WORD                  wRealDmgCount = 0;
  int                   iTC = 0, iCalHit = 0, iTrapDamage = 0;
  SNMMgcDmgHeader       *pHeader = (SNMMgcDmgHeader*)(MsgMagicDamage.Msgs[0].Data);
  SNMDamage	            *pDamage = (SNMDamage*)(MsgMagicDamage.Msgs[0].Data+sizeof(SNMMgcDmgHeader));
  //Add by CECE 2004-06-23
  LPSNMSetSpecialStatus		pSpecial = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);
  static WORD             wMaxCharCount = ( MAXMSGDATASIZE / sizeof( SNMSetSpecialStatus ) ) - 2;
	BYTE										bCharLifeCount = 0;
  //
#ifdef RESTRAIN_TRAP
  float                   fResult = -1.0f;
#endif
  // Fill Header Data
  pHeader->wCode      = pMagic->m_wCode;
  pHeader->wId        = GetId();
  pHeader->wOwnerCode = pMagic->GetUserCode();
  pHeader->wX         = pMagic->GetPosX();
  pHeader->wY         = pMagic->GetPosY();
  // Fill Damage Data

  if( m_pPlayer )
  {
    for( Iter_Tr = m_listTargets.begin(); Iter_Tr != m_listTargets.end(); Iter_Tr++ )
    {
      if( wRealDmgCount >= wMaxDmgCount1 )                break;
      //
      pTrapTgt = (*Iter_Tr);
      if( pTrapTgt->GetMapId() != pMagic->GetMapId() )    continue;
      // Cal The Hit For Target ( Including Critical Attack )
      //iCalHit = m_pPlayer->GetTotalInt() / ( GetItemHu() * GetElement() + 1 );
      //if( iCalHit < g_MinHit )        iCalHit = g_MinHit;
      //else if( iCalHit > g_MaxHit )   iCalHit = g_MaxHit;
#ifdef _DEBUG_TRAP_MUST_DAMAGE_
      iCalHit = 101;
#endif
      // Miss The Target
      if( iCalHit < 1 )
      {
        pDamage->qwSpecial    = 0;
        pDamage->wSubHp       = 0;
        pDamage->wTargetCode  = pTrapTgt->GetCode();
        wRealDmgCount++;
        pDamage++;
        iTC++;
      }
      // Sure Hit The Target
      else
#ifdef  RESTRAIN_TRAP
        if( pTrapTgt->IsPlayer() && (pTrapTgt->GetSpecialStatus() & SPE_STATUS_RESTRAIN_TRAP) )
        {
          pDamage->qwSpecial    = pTrapTgt->GetSpecialStatus();
          pDamage->wSubHp       = 0;
          pDamage->wTargetCode  = pTrapTgt->GetCode();
          wRealDmgCount++;
          pDamage++;
          iTC++;
        }
        else
#endif
      {
        // Clear All Status For Target
        //if( pTrapTgt->IsMonster() )       ((CMonster*)pTrapTgt)->BeAttacked( m_pPlayer );
        //else                              ((CPlayer*)pTrapTgt)->BeAttacked();
        // Cal Damage And Special Status
        switch( GetType() )
        {
        case SKILL_TYPE_TRAP_DURATIVE_NORMAL:
        case SKILL_TYPE_TRAP_NORMAL:
          // 普通战斗公式(不含五行相生相克) + 克怪物种族
          {
            iTrapDamage            = GetOwnAttributeForHuan() * GetAttrBonu();
            if( pTrapTgt->IsMonster() )
            {
              float   RaceJinx;
              if( 0 != ( RaceJinx  = m_pBase->m_dwRaceAttri & ((CMonster*)(pTrapTgt))->m_dwRaceAttri ) )
              {
                RaceJinx = m_pBase->GetBitCountForRace( RaceJinx );
                RaceJinx = RaceJinx * m_pBase->m_wRaceBonuRate / 100;
                iTrapDamage += iTrapDamage * RaceJinx;
              }
            }

            pDamage->wSubHp      = iTrapDamage;
            pDamage->wTargetCode = pTrapTgt->GetCode();
#ifdef RESTRAIN_TRAP
            fResult = RestrainFortrap();            
            if( fResult - (-1.0f) > 0.000001f )
            { 
              pDamage->wSubHp    *= fResult;
            }
            else
#endif
            if( m_pBase->m_iRandom >= gf_GetRandom(100) )
            {
              m_pBase->SetSpecialState( pTrapTgt );
            }
            pDamage->qwSpecial   = pTrapTgt->GetSpecialStatus();
            if( pTrapTgt->IsPlayer() )
            {
              //
#ifdef _FUNCTION_RING_3_8_
              if( FALSE == ((CPlayer*)pTrapTgt)->TrapNoUseForMe() )
#endif
              ((CPlayer* )(pTrapTgt))->Damage( pDamage, m_pPlayer,
                                               pTrapTgt->GetMaxMp() * GetMpChange() / 100,
                                               pTrapTgt->GetMaxSp() * GetSpChange() / 100 );
            }
            else
            {
              ((CMonster*)(pTrapTgt))->Damage( pDamage, m_pPlayer );
            }
            wRealDmgCount++;
            pDamage++;
            iTC++;
          }
          break;
        case SKILL_TYPE_TRAP_DURATIVE_EXPRESSION1:
        case SKILL_TYPE_TRAP_EXPRESSION1:
          // 陷阱战斗公式 1
          {
            iTrapDamage = (m_pPlayer->GetTotalHit()+50)*(1+m_pPlayer->GetTotalInt()/200)*GetElement();
            if( iTrapDamage < 1 )  iTrapDamage = 1;
            //
            if( pTrapTgt->IsMonster() )
            {
              float   RaceJinx;
              if( 0 != ( RaceJinx  = m_pBase->m_dwRaceAttri & ((CMonster*)(pTrapTgt))->m_dwRaceAttri ) )
              {
                RaceJinx = m_pBase->GetBitCountForRace( RaceJinx );
                RaceJinx = RaceJinx * m_pBase->m_wRaceBonuRate / 100;
                iTrapDamage += iTrapDamage * RaceJinx;
              }
            }
            //
            pDamage->wSubHp      = iTrapDamage;
            pDamage->wTargetCode = pTrapTgt->GetCode();
            pDamage->qwSpecial   = 0;
#ifdef RESTRAIN_TRAP
            fResult = RestrainFortrap();
            if( fResult - (-1.0) > 0.000001  )
            { 
              pDamage->wSubHp    *= fResult;              
            }
#endif
            if( pTrapTgt->IsPlayer() )
            {
              //
#ifdef _FUNCTION_RING_3_8_
              if( FALSE == ((CPlayer*)pTrapTgt)->TrapNoUseForMe() )
#endif
              ((CPlayer* )(pTrapTgt))->Damage( pDamage, m_pPlayer,
                                               pTrapTgt->GetMaxMp() * GetMpChange() / 100,
                                               pTrapTgt->GetMaxMp() * GetSpChange() / 100 );
            }
            else
            {
              ((CMonster*)(pTrapTgt))->Damage( pDamage, m_pPlayer );
            }
            wRealDmgCount++;
            pDamage++;
            iTC++;
          }
          break;
        case SKILL_TYPE_TRAP_DURATIVE_EXPRESSION2:
        case SKILL_TYPE_TRAP_EXPRESSION2:
          // 陷阱战斗公式 2
          {
            
          }
          break;
        case SKILL_TYPE_TRAP_STATUS_HEAL:
          // 陷阱异常 -- Heal
          {

            pDamage->wTargetCode = pTrapTgt->GetCode();
            pDamage->wSubHp      = GetHpChange();
            //
#ifdef RESTRAIN_TRAP
            fResult = RestrainFortrap();          
            if( fResult - (-1.0) > 0.000001 )
            {
              if(GetColor() == SKILL_TYPE_ITEM_SET_STATUS)
              {
                pTrapTgt->ClearSpecialStatus(m_pBase->m_qwStatus);
              }
              pDamage->wSubHp   *= fResult;
              pDamage->qwSpecial = pTrapTgt->GetSpecialStatus();
            }
            else
#endif
            if( m_pBase->m_iRandom >= gf_GetRandom(100) )
            {
              CalAllSpecialStatus( pTrapTgt );
            }
            //
            pDamage->qwSpecial   = pTrapTgt->GetSpecialStatus();
            if( pTrapTgt->IsPlayer() )
            {
              //
#ifdef _FUNCTION_RING_3_8_
              if( FALSE == ((CPlayer*)pTrapTgt)->TrapNoUseForMe() )
#endif
              ((CPlayer* )(pTrapTgt))->Damage( pDamage, m_pPlayer, GetMpChange(), GetSpChange() );
            }
            else
            {
              ((CMonster*)(pTrapTgt))->Damage( pDamage, m_pPlayer );
            }
            //
            wRealDmgCount++;
            pDamage++;
            iTC++;
          }
          break;
        //相消术
        case SKILL_TYPE_TRAP_STATUS_CHANGE_CHAR:
          // 陷阱异常 -- Change Char Ap Hit Dp Dodge Int Critical
          {
            //Add by CECE 2004-07-15
            if( pTrapTgt->IsPlayer() )
            {
              if( m_pBase->m_iRandom >= gf_GetRandom(100) )
              {
                m_pBase->ChangeChar( pTrapTgt );
                //Add by CECE 2004-06-23
                //if( pTrapTgt->IsPlayer() )
                //{
                CPlayer *pPlayer = (CPlayer*)pTrapTgt;
                if( bCharLifeCount < wMaxCharCount )
                {
                  pSpecial->dwCode_Skill = ( pPlayer->GetSelfCode() << 16 ) | GetId();
                  pSpecial->qwStatus     = pPlayer->GetSpecialStatus();
                  pSpecial++;
                  bCharLifeCount++;
                }
                //}
              }
              //
              pDamage->qwSpecial   = pTrapTgt->GetSpecialStatus();
              pDamage->wTargetCode = pTrapTgt->GetCode();
              pDamage->wSubHp      = GetHpChange();
#ifdef RESTRAIN_TRAP
            fResult = RestrainFortrap();
            if( fResult - (-1.0f) > 0.000001f )
            {
              pDamage->wSubHp    *= fResult;
            }
#endif            //
              //if( pTrapTgt->IsPlayer() )
              //{
#ifdef _FUNCTION_RING_3_8_
              if( FALSE == ((CPlayer*)pTrapTgt)->TrapNoUseForMe() )
#endif
              ((CPlayer* )(pTrapTgt))->Damage( pDamage, m_pPlayer, GetMpChange(), GetSpChange() );
              //}
              //else
              //{
              //  ((CMonster*)(pTrapTgt))->Damage( pDamage, m_pPlayer );
              //}
              wRealDmgCount++;
              pDamage++;
              iTC++;
            }
            /*
            else
            {
              iTC++;
            }*/
          }
          break;
        case SKILL_TYPE_TRAP_STATUS_SET_STATUS:
          // 陷阱异常 -- SetSpecialState
          {

            pDamage->wTargetCode = pTrapTgt->GetCode();
            pDamage->wSubHp      = GetHpChange();
            //
#ifdef RESTRAIN_TRAP
            fResult = RestrainFortrap();
            if( fResult - (-1.0f) > 0.000001f )
            {
              pDamage->qwSpecial = pTrapTgt->GetSpecialStatus();
              pDamage->wSubHp    *= fResult;              
            }
            else
#endif
            if( m_pBase->m_iRandom >= gf_GetRandom(100) )
            {
              m_pBase->SetSpecialState( pTrapTgt );
            }
            //
            pDamage->qwSpecial   = pTrapTgt->GetSpecialStatus();
            if( pTrapTgt->IsPlayer() )
            {
#ifdef _FUNCTION_RING_3_8_
              if( FALSE == ((CPlayer*)pTrapTgt)->TrapNoUseForMe() )
#endif
              ((CPlayer* )(pTrapTgt))->Damage( pDamage, m_pPlayer, GetMpChange(), GetSpChange() );
            }
            else
            {
              ((CMonster*)(pTrapTgt))->Damage( pDamage, m_pPlayer );
            }
            //
            wRealDmgCount++;
            pDamage++;
            iTC++;
          }
          break;
        default:  // SKILL_TYPE_TRAP_NORMAL
          break;
        }
      }
    }
    AddExp( 1 );
    pHeader->wTCount = iTC;
    MsgMagicDamage.Msgs[0].Size = sizeof(SNMMgcDmgHeader) + sizeof(SNMDamage) * iTC;
	  (pMagic->GetInMap())->SendMsgNearPosition_Close( MsgMagicDamage, pMagic->GetPosX(), pMagic->GetPosY() );
    //Add by CECE 2004-06-23
    if( bCharLifeCount )
    {
       MsgSpecialStatus.Msgs[0].Size  = sizeof( SNMSetSpecialStatus ) * bCharLifeCount;
       (pMagic->GetInMap())->SendMsgNearPosition_Close( MsgSpecialStatus, pMagic->GetPosX(), pMagic->GetPosY() );
    }
    //
  }
  else if( m_pMonster )
  {
    MsgMagicDamage.Msgs[0].Size = sizeof(SNMMgcDmgHeader) + sizeof(SNMDamage) * iTC;
	  (pMagic->GetInMap())->SendMsgNearPosition_Close( MsgMagicDamage, pMagic->GetPosX(), pMagic->GetPosY() );
  }
}
// SKILL_TYPE_TRAP_NORMAL        = 150,  // 普通战斗公式 + 克怪物种族
// SKILL_TYPE_TRAP_EXPRESSION1,          // 陷阱战斗公式 1
// SKILL_TYPE_TRAP_EXPRESSION2,          // 陷阱战斗公式 2
// SKILL_TYPE_TRAP_STATUS_HEAL,          // 陷阱异常 -- 6个 MaxHp, Hp, MaxMp, Mp, MaxSp, Sp
// SKILL_TYPE_TRAP_STATUS_CHANGE_CHAR,   // 陷阱异常 -- 6个 改变目标的 Ap Hit Dp Dodge Int Critical
// SKILL_TYPE_TRAP_STATUS_SET_STATUS,    // 陷阱异常 -- 8个 改变目标的 Speed 和 五种特殊状态 以及 隐身, 侦测隐身
// SKILL_TYPE_SETSKILL_ELEMENT,          // 设置玩家招式的五行
// SKILL_TYPE_FOUND_TRAP,                // 发现陷阱, 周围7X7的范围内的陷阱会被看到 -- No Use Now
// SKILL_TYPE_CLEAR_TRAP,                // 清除陷阱, 周围5X5的范围内的陷阱会被清除
// //
// SKILL_TYPE_TRAP_DURATIVE_NORMAL,      // 持续性陷阱 == 普通战斗公式 + 克怪物种族
// SKILL_TYPE_TRAP_DURATIVE_EXPRESSION1, // 持续性陷阱 == 陷阱战斗公式 1
// SKILL_TYPE_TRAP_DURATIVE_EXPRESSION2, // 持续性陷阱 == 陷阱战斗公式 2
// //
//==========================================================================================
//desc:				monster对目标作攻击
//parameter:	
//return   :  bool
//==========================================================================================
BOOL CSkill::MonsterUseSkill()
{
  static WORD             wMaxDmgCount2  = ( MAXMSGDATASIZE / sizeof( SNMDamage ) ) - 2;
	static LifeTileIter	    iter;
	if( !FindTarget() )     return false;

	int               iCounter = 0;
  SNMSklDmgHeader   *pHeader = (SNMSklDmgHeader*)(MsgDamage.Msgs[0].Data);
  SNMDamage         *pDamage = (SNMDamage*)(pHeader+1);
  //
  pHeader->wSkillCode = m_pBase->GetId();
  pHeader->wUserCode = m_pMonster->GetCode();

	for( iter = m_listTargets.begin(); iter != m_listTargets.end(); iter++ )
	{
    if( iCounter >= wMaxDmgCount2 )       break;
		// 对每个目标作被攻击的处理
//		if( (*iter)->IsPlayer() )
//    {
//      if( ((CPlayer*)(*iter))->m_pEquip[ITEM_EQUIPWHERE_HAND] != NULL )
//		  {
//			  ((CPlayer*)(*iter))->m_pEquip[ITEM_EQUIPWHERE_HAND]->OnUnderAttack( (CPlayer*)*iter,m_pMonster );
//	  	}
//      if( ((CPlayer*)(*iter))->GetStatus() == STATUS_PLAYER_DEAD )    continue;
//    }
		MonsterAttackPlayer( (CPlayer*)*iter, pDamage );
		pDamage++;
		iCounter++;
	}
  if( MsgDamage.dwAID != A_DAMAGE || MsgDamage.dwMsgLen != 1 || MsgDamage.Msgs[1].Size != 0 )
  {
    _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "The Damage Msg's AID Is Error(Monster), AID=%d, MsgLen=%d, Size0=%d, Size1=%d, Count=%d !\n",
             MsgDamage.dwAID, MsgDamage.dwMsgLen, MsgDamage.Msgs[0].Size, MsgDamage.Msgs[1].Size, iCounter );
    g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
    g_DamageLog.Write( g_szDamageLog );
    MsgDamage.Init();
	  MsgDamage.dwAID        = A_DAMAGE;
	  MsgDamage.dwMsgLen     = 1;
    MsgDamage.Msgs[1].Size = 0;
  }
	MsgDamage.Msgs[0].Size = sizeof(SNMSklDmgHeader)+sizeof(SNMDamage)*iCounter;

	CGameMap *pMapIn = m_pMonster->GetInMap();
	pMapIn->SendMsgNearPosition_Close( MsgDamage, m_pMonster->GetPosX(), m_pMonster->GetPosY() );
	return TRUE;
}
//==========================================================================================
//desc:				玩家对目标作攻击
//parameter:
//return   :  bool
//==========================================================================================
///////////////////////////////////////////////////////////////////////////////
//做攻击计算
BOOL CSkill::PlayerUseSkill()
{
	int						                          iCounter			= 0;
	int						                          iAddiDamage		= 0;
	CLife*					                        pTarget       = NULL;
  static CSrvBaseSkill                    *g_pAddiSkill = NULL;
  static list<CSrvBaseSkill*>::iterator   Iter_Sbs;
	static LifeTileList::iterator	          Iter_Lf;

  ////////////////////////////////////////////////
  //Add by CECE 2004-04-07
#ifdef EVILWEAPON_3_6_VERSION
  LifeTileList::iterator Iter_Evil;
  BOOL      bEvilWeaponHit = FALSE;
  LPCEvilWeapon pEvilWeapon = NULL;              
  //找到拥有玩家
  if( m_pPlayer )   
    pEvilWeapon = m_pPlayer->GetEvilWeapon();
  else
    return true;
#endif
  ////////////////////////////////////////////////
  //add by cece 2004-07-14
#ifdef ELYSIUM_3_7_VERSION
  int iMatchValue = 0;
#endif
  ////////////////////////////////////////////////

  //
  static WORD                     wMaxDmgCount3  = ( MAXMSGDATASIZE / sizeof( SNMDamage ) ) - 2;

	if( m_listTargets.empty() && m_listMiss.empty() )
  {
    // Copy The Last Target...???
    // ...
    return true;
  }
  //Add by CECE 2004-08-19
  if( NULL == m_pPlayer || NULL == m_pPlayer->GetInMap() )
    return true;
  //
  SNMSklDmgHeader   *pHeader     = (SNMSklDmgHeader*)(MsgDamage.Msgs[0].Data);
	SNMDamage			    *pDamage     = (SNMDamage*)(pHeader + 1);
	//
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
	SNMAddiDamage     *pAddiDamage = (SNMAddiDamage*)(MsgAddiDamage.Msgs[0].Data);
#endif
  //
  pHeader->wSkillCode = m_pBase->GetId();
  pHeader->wUserCode  = m_pPlayer->GetCode();

	// 其他职业招式
	if( m_pBase->IsCommonSkill() )
	{
    if( !m_listTargets.empty() )
    {
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
      // Do Random Suit Equip Skill Of Attacker ( Just One )
      if( m_pPlayer->HaveSuitInEquip() && !m_pPlayer->m_listSuitSkill.empty() )
      {
        for( Iter_Sbs = m_pPlayer->m_listSuitSkill.begin();
             Iter_Sbs != m_pPlayer->m_listSuitSkill.end(); Iter_Sbs++ )
        {
          g_pAddiSkill = (*Iter_Sbs);
          if( g_pAddiSkill->m_iProbability > gf_GetRandom( 100 ) )
          {
            // Record The Suit Addi Skill
            iAddiDamage += gf_GetRandom( g_pAddiSkill->m_iHpChange ) + 1;
						pAddiDamage->wUserCode      = m_pPlayer->GetSelfCode();
						pAddiDamage->wAddiSkillCode = g_pAddiSkill->GetId();
						pAddiDamage->wSelfSkillCode = GetId();						
            break;
          }
        }
      }
      // Do Tessera Random Skill Of Attacker
			if( 0 == iAddiDamage && !m_pPlayer->m_listTesseraAttack.empty() )
			{
				for( Iter_Sbs  = m_pPlayer->m_listTesseraAttack.begin();
						 Iter_Sbs != m_pPlayer->m_listTesseraAttack.end(); Iter_Sbs++ )
				{
					g_pAddiSkill = (*Iter_Sbs);
					if( g_pAddiSkill->m_iProbability > gf_GetRandom( 100 ) )
					{
#ifdef _GHOST_TESSERA_
						if( g_pAddiSkill->m_iHpChange>65535 && g_pAddiSkill->m_iHpChange<=75535)
            {
               iAddiDamage +=((*m_listTargets.begin())->GetHp()* ( (g_pAddiSkill->m_iHpChange)-65535) / 10000 );
            }
            else
#endif            
            {
            // Record The Tessera Addi Skill
						iAddiDamage += gf_GetRandom( g_pAddiSkill->m_iHpChange ) + 1;
            }
						pAddiDamage->wUserCode      = m_pPlayer->GetSelfCode();
						pAddiDamage->wAddiSkillCode = g_pAddiSkill->GetId();
						pAddiDamage->wSelfSkillCode = GetId();						
            
            break;
					}
				}
			}
			if( 0 == iAddiDamage )		pDamage = (SNMDamage*)(pHeader + 1);
			else                      pDamage = (SNMDamage*)(pAddiDamage + 1);
#endif
			//
		  for( Iter_Lf = m_listTargets.begin(); Iter_Lf != m_listTargets.end(); Iter_Lf++ )
		  {
        if( iCounter >= wMaxDmgCount3 )     break;
        //
        if( PlayerCommonAttack( (*Iter_Lf), pDamage, iAddiDamage ) )
        {

#ifdef EVILWEAPON_3_6_VERSION
          if( (*Iter_Lf)->IsMonster() ) bEvilWeaponHit = TRUE;
#endif

#ifdef ELYSIUM_3_7_VERSION
          iMatchValue += pDamage->wSubHp;
#endif

			    pDamage++;
			    iCounter++;
        }
		  }

#ifdef ELYSIUM_3_7_VERSION
      LPCFighter pFightInfo = NULL;
      if( m_pPlayer )    pFightInfo = m_pPlayer->GetFightInfo();
      if( pFightInfo ) pFightInfo->AddValue( iMatchValue );
#endif

      //如果击中增加妖力值
#ifdef EVILWEAPON_3_6_VERSION
      if( bEvilWeaponHit && pEvilWeapon )
			{
				if( gf_GetRandom( 10000 ) <= pEvilWeapon->GetCurGetOdds() )
				{
					pEvilWeapon->SetMana( pEvilWeapon->GetMana()+1 );
					//发送显示妖力值
					if( m_pPlayer ) m_pPlayer->SendEvilPtMsg();
				}
			}
 #endif

    }
	}
	// 幻道咒术
	else if( m_pBase->IsAttackCurse() )
	{
    if( !m_listTargets.empty() )
    {
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
      // Do Random Suit Equip Skill Of Attacker ( Just One )
      if( m_pPlayer->HaveSuitInEquip() && !m_pPlayer->m_listSuitSkill.empty() )
      {
        for( Iter_Sbs = m_pPlayer->m_listSuitSkill.begin();
             Iter_Sbs != m_pPlayer->m_listSuitSkill.end(); Iter_Sbs++ )
        {
          g_pAddiSkill = (*Iter_Sbs);
          if( g_pAddiSkill->m_iProbability > gf_GetRandom( 100 ) )
          {
            // Record The Suit Addi Skill
            iAddiDamage += gf_GetRandom( g_pAddiSkill->m_iHpChange ) + 1;
						pAddiDamage->wUserCode      = m_pPlayer->GetSelfCode();
						pAddiDamage->wAddiSkillCode = g_pAddiSkill->GetId();
						pAddiDamage->wSelfSkillCode = GetId();						
            break;
          }
        }
      }
      // Do Tessera Random Skill Of Attacker
			if( 0 == iAddiDamage && !m_pPlayer->m_listTesseraAttack.empty() )
			{
				for( Iter_Sbs  = m_pPlayer->m_listTesseraAttack.begin();
						 Iter_Sbs != m_pPlayer->m_listTesseraAttack.end(); Iter_Sbs++ )
				{
					g_pAddiSkill = (*Iter_Sbs);
					if( g_pAddiSkill->m_iProbability > gf_GetRandom( 100 ) )
					{            
#ifdef _GHOST_TESSERA_
						if( g_pAddiSkill->m_iHpChange>65535 && g_pAddiSkill->m_iHpChange<=65635)
            {
               iAddiDamage +=((*m_listTargets.begin())->GetHp()* ( (g_pAddiSkill->m_iHpChange)-65535) / 10000 );
            }
            else
#endif 
            {            
						// Record The Tessera Addi Skill
						iAddiDamage += gf_GetRandom( g_pAddiSkill->m_iHpChange ) + 1;
            }
						pAddiDamage->wUserCode      = m_pPlayer->GetSelfCode();
						pAddiDamage->wAddiSkillCode = g_pAddiSkill->GetId();
						pAddiDamage->wSelfSkillCode = GetId();						
						break;            
					}
				}
			}
			if( 0 == iAddiDamage )		pDamage = (SNMDamage*)(pHeader + 1);
			else                      pDamage = (SNMDamage*)(pAddiDamage + 1);
#endif
			//
      for( Iter_Lf = m_listTargets.begin(); Iter_Lf != m_listTargets.end(); Iter_Lf++ )
		  {
        if( iCounter >= wMaxDmgCount3 )     break;
		    pTarget = (*Iter_Lf);
				//
/*
#ifdef EVILWEAPON_3_6_VERSION
        CPlayer* pcPlayer = NULL;
        if( pTarget->IsPlayer() )
        {
          pcPlayer = (CPlayer*)pTarget;
        }
        //如果是玩家而且已经中了妖器召唤
        if( pcPlayer && pcPlayer->IsEvilAttack() ) continue;
#endif
*/
        if( PlayerCurseAttack( pTarget, pDamage, iAddiDamage))
        {
#ifdef EVILWEAPON_3_6_VERSION
          if( (*Iter_Lf)->IsMonster() ) bEvilWeaponHit = TRUE;
#endif

#ifdef ELYSIUM_3_7_VERSION
          iMatchValue += pDamage->wSubHp;
#endif
////////////////////////////////////////////////

		      pDamage++;
		      iCounter++;
        }
      }

#ifdef ELYSIUM_3_7_VERSION
      LPCFighter pFightInfo = NULL;
      if( m_pPlayer )   pFightInfo = m_pPlayer->GetFightInfo();
      if( pFightInfo )  pFightInfo->AddValue( iMatchValue );
#endif

      //如果击中增加妖力值
#ifdef EVILWEAPON_3_6_VERSION
			if( bEvilWeaponHit && pEvilWeapon )
			{
        if( gf_GetRandom( 10000 ) <= pEvilWeapon->GetCurGetOdds() )
				{
          pEvilWeapon->SetMana( pEvilWeapon->GetMana()+1 );
					//发送显示妖力值
					if( m_pPlayer ) m_pPlayer->SendEvilPtMsg();
        }
			}
#endif
      ///////////////////////////////////////////////
    }
	}
  //妖器怪物召唤
#ifdef EVILWEAPON_3_6_VERSION
  //如果是召唤攻击
  else if( m_pBase->IsEvilSummon() )
  {      
    for( Iter_Evil = m_listTargets.begin(); Iter_Evil != m_listTargets.end(); Iter_Evil++ )
    {
      if( iCounter >= wMaxDmgCount3 )     break;
      pTarget = (*Iter_Evil);
      if( DoEvilDamage( pTarget, pDamage ) )
      {

#ifdef ELYSIUM_3_7_VERSION
        iMatchValue += pDamage->wSubHp;
#endif
////////////////////////////////////////////////
        pDamage++;
        iCounter++;
      }
    }

#ifdef ELYSIUM_3_7_VERSION
      LPCFighter pFightInfo = NULL;
      if( m_pPlayer )    pFightInfo = m_pPlayer->GetFightInfo();
      if( pFightInfo ) pFightInfo->AddValue( iMatchValue );
#endif
  }
#endif
  ///////////////////////////////////////////////////////////////////////////
	// 玩家送来的错误的攻击对象
	if( !m_listMiss.empty() )
	{
		for( Iter_Lf = m_listMiss.begin(); Iter_Lf != m_listMiss.end(); Iter_Lf++ )
		{
      if( iCounter >= wMaxDmgCount3 )     break;
      //
			pTarget = (*Iter_Lf);
      pDamage->wTargetCode = pTarget->GetCode();
		  pDamage->wSubHp      = 0;
      pDamage->qwSpecial   = 0;
      //
      if( pTarget->IsPlayer() )
      {
        if( m_pPlayer->IsEnemyPlayer( (CPlayer*)pTarget ) ) pDamage->qwSpecial |= SPE_STATUS_ENEMY_GUILD;
      }
			pDamage++;
			iCounter++;
		}
	}
//	// 检查MsgDamage
//	if( MsgDamage.dwAID != A_DAMAGE || MsgDamage.dwMsgLen != 1 || MsgDamage.Msgs[1].Size != 0 )
//	{
//		sprintf( g_szDamageLog, "The Damage Msg's AID Is Error(Player), AID=%d, MsgLen=%d, Size0=%d, Size1=%d, Count=%d !\n",
//						 MsgDamage.dwAID, MsgDamage.dwMsgLen, MsgDamage.Msgs[0].Size, MsgDamage.Msgs[1].Size, iCounter );
//		g_DamageLog.Write( g_szDamageLog );
//		MsgDamage.Init();
//		MsgDamage.dwAID        = A_DAMAGE;
//		MsgDamage.dwMsgLen     = 1;
//		MsgDamage.Msgs[1].Size = 0;
//	}
  //
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  if( 0 != iAddiDamage )
  {
    // 如果本次攻击带有镶嵌或套装所激发的附加技能
    MsgAddiDamage.Msgs[0].Size =  sizeof(SNMAddiDamage)  + sizeof(SNMDamage) * iCounter;
    m_pPlayer->GetInMap()->SendMsgNearPosition_Close( MsgAddiDamage,  m_pPlayer->GetPosX(), m_pPlayer->GetPosY() );
  }
  else
#endif
	{
		// 正常攻击，无附加效果
#ifdef _DEBUG
    FuncName("SEND A_DAMAGE");
    AddMemoMsg("SEND A_DAMAGE");
#endif
		MsgDamage.Msgs[0].Size = sizeof(SNMSklDmgHeader) + sizeof(SNMDamage) * iCounter;
		m_pPlayer->GetInMap()->SendMsgNearPosition_Close( MsgDamage, m_pPlayer->GetPosX(), m_pPlayer->GetPosY() );
	}
	// Clear All Targets
	m_listTargets.clear();
	m_listMiss.clear();
	return TRUE;
}
//==========================================================================================
//desc:				对参数指定的生物做攻击 ,用于幻道的咒语
//parameter:	pTarget(目标),pDamageInfo(填充伤害信息)
//return   :  bool
//==========================================================================================
inline bool	CSkill::PlayerCurseAttack( CLife *pTarget, SNMDamage *pDamageInfo, int iAddiDamage )
{
#ifdef _DEBUG_OPEN_ACCESS_BAD_PTR_
  if( IsBadReadPtr( pTarget, sizeof( CLife ) ) )
  {
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CSkill::PlayerCurseAttack");
    _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "===>>>%s Find Bad Ptr(%08x) When Curse Attack Life", m_pPlayer->GetAccount(), pTarget );
    g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szDamageLog );
#endif
#endif
    return false;
  }
#endif
  // If Target Is GM And Target Have Never Died Status
  /////////////////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-09
#ifdef  EVILWEAPON_3_6_VERSION
  if( pTarget->IsPlayer() && (
       ( ((CPlayer*)pTarget)->GetGMStatus() & GM_STATUS_NODIED ) ||
       ( ((CPlayer*)pTarget)->GetSpecialStatus() & SPE_STATUS_GODEMODE )
     ) )
#else
  if( pTarget->IsPlayer() && ((CPlayer*)pTarget)->GetGMStatus() & GM_STATUS_NODIED ) 
#endif
  /////////////////////////////////////////////////////////////////////////////////////
  {
    pDamageInfo->wTargetCode = pTarget->GetCode();
		pDamageInfo->wSubHp      = 0;
		//pDamageInfo->wSubMp      = 0;
		//pDamageInfo->wSubSp      = 0;
		//pDamageInfo->wTargetX    = pTarget->GetPosX();
		//pDamageInfo->wTargetY    = pTarget->GetPosY();
    pDamageInfo->qwSpecial   = 0;
		return true;
  }
#ifdef FIX_BUG_ATTACKED_WHILE_LOGOUT
	if( pTarget->IsPlayer() &&
      (
        ((CPlayer*)pTarget)->GetClientState() < CLIENTSTATE_CONNECT ||
        ((CPlayer*)pTarget)->m_bClientClaimLogout
      )
    )
	{
		return true;
	}
#endif
	short   nDecMp = 0, nDecSp = 0;
  short   nDamage = CalDamageForWizard( pTarget, iAddiDamage );
  // Do Special Status
  CalAllSpecialStatus( pTarget );
  //
  pDamageInfo->wTargetCode = pTarget->GetCode();
	pDamageInfo->wSubHp      = nDamage;
	//pDamageInfo->wSubMp      = 0;
	//pDamageInfo->wSubSp      = 0;
	//pDamageInfo->wTargetX    = pTarget->GetPosX();
	//pDamageInfo->wTargetY    = pTarget->GetPosY();
  pDamageInfo->qwSpecial   = pTarget->GetSpecialStatus();
  //
	if( pTarget->IsPlayer() )
	{
		CPlayer* pTargetPlayer = (CPlayer*)(pTarget);
    ////////////////////////////////////////////////////////
    // 霸体功能处理
    ////////////////////////////////////////////////////////
    int psbParam=1;
    switch( pTargetPlayer->GetOccupation() )
    {
    case OCCU_BLADEMAN: //诡道
    case OCCU_BLADEMANF:
      psbParam = g_BearGui;
      break;
    case OCCU_SWORDMAN: //剑宗
    case OCCU_SWORDMANF:
      psbParam = g_BearJian;
      break;
    case OCCU_PIKEMAN:  //戟门
    case OCCU_PIKEMANF:
      break;
    case OCCU_WIZARD:   //幻道
    case OCCU_WIZARDF:
      psbParam = g_BearHuan;
      break;
    default:
      psbParam = g_BearJi;
      break;
    }
    psbParam = pTarget->GetMaxHp() * psbParam / 100;
    if( nDamage <= psbParam )               pDamageInfo->qwSpecial |= SPE_STATUS_UNINJURED;
    else
    {
      psbParam = pTarget->GetBearPsb();
      if( psbParam >= gf_GetRandom(100) )   pDamageInfo->qwSpecial |= SPE_STATUS_UNINJURED;
    }
    if( m_pPlayer->IsEnemyPlayer( (CPlayer*)pTarget ) ) pDamageInfo->qwSpecial |= SPE_STATUS_ENEMY_GUILD;
    //
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
    // 套装反弹
    if( pTargetPlayer->HaveSuitInEquip() )
    {
      if( pTargetPlayer->m_iSuitRebound && pTargetPlayer->m_iSuitReboundOdds > gf_GetRandom(100) ) 
      {
        m_pPlayer->m_iHp -= ( pDamageInfo->wSubHp * pTargetPlayer->m_iSuitRebound / 100 );
        if( m_pPlayer->m_iHp < 0 )      m_pPlayer->m_iHp = 1;
        pDamageInfo->qwSpecial |= SPE_STATUS_REBOUND_ATTACK;
      }
    }
    // 套装吸收
    if( m_pPlayer->HaveSuitInEquip() && m_pPlayer->m_iSuitSuckOdds > gf_GetRandom(100)  ) 
    {
      if( m_pPlayer->m_iSuitSuckHp )
      {
        m_pPlayer->m_iHp += ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckHp / 100 );
        if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckMp )
      {
        nDecMp = ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckMp / 100 );
        m_pPlayer->m_iMp     += nDecMp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iMp -= nDecMp;
        if( pTargetPlayer->m_iMp < 0 )                pTargetPlayer->m_iMp = 0;
#endif
        if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckSp )
      {
        nDecSp = ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckSp / 100 );
        m_pPlayer->m_iSp     += nDecSp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iSp -= nDecSp;
        if( pTargetPlayer->m_iSp < 0 )                pTargetPlayer->m_iSp = 0;
#endif
        if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
    }
    // 镶嵌反弹
    if( pTargetPlayer->m_iTesseraRebound && pTargetPlayer->m_iTesseraReboundOdds > gf_GetRandom(100) )
    {	
#ifdef DEPRESS_TESS_DEFENCE
      m_pPlayer->m_iHp -= ( pDamageInfo->wSubHp * pTargetPlayer->m_iTesseraRebound * DepressDefence() / 10000 );	
#else
      m_pPlayer->m_iHp -= ( pDamageInfo->wSubHp * pTargetPlayer->m_iTesseraRebound / 100 );	
#endif
      if( m_pPlayer->m_iHp < 0 )      m_pPlayer->m_iHp = 1;
      pDamageInfo->qwSpecial |= SPE_STATUS_REBOUND_ATTACK;
    }
		// 镶嵌吸收被攻击方的 HP MP SP
		if(	m_pPlayer->m_iTesseraSuckOdds > gf_GetRandom(100) )
		{
			if( m_pPlayer->m_iTesseraSuckHp )
			{
				m_pPlayer->m_iHp += ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckHp / 100 );
				if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckMp )
			{
        nDecMp = ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckMp / 100 );
        m_pPlayer->m_iMp     += nDecMp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iMp -= nDecMp;
        if( pTargetPlayer->m_iMp < 0 )                pTargetPlayer->m_iMp = 0;
#endif
				if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckSp )
			{
        nDecSp = ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckSp / 100 );
        m_pPlayer->m_iSp     += nDecSp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iSp -= nDecSp;
        if( pTargetPlayer->m_iSp < 0 )                pTargetPlayer->m_iSp = 0;
#endif
				if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
		}
		// 镶嵌防守技能
		if( !pTargetPlayer->m_listTesseraDefence.empty() )
		{
			static CSrvBaseSkill									 *g_pDefenceSkill = NULL;
			static list<CSrvBaseSkill*>::iterator   Iter_Sbs;
			static int															iDefenceDamage  = 0;
			int																			iDefenceConter  = 0;
			//
			SNMDefenceDamage     *pDefenceDamage  = (SNMDefenceDamage*)(MsgDefeDamage.Msgs[0].Data);
			//
			for( Iter_Sbs  = pTargetPlayer->m_listTesseraDefence.begin();
					 Iter_Sbs != pTargetPlayer->m_listTesseraDefence.end(); Iter_Sbs++ )
			{
				g_pDefenceSkill = (*Iter_Sbs);
				if( g_pDefenceSkill->m_iProbability > gf_GetRandom( 100 ) )
				{
					// Record The Addi Skill
					iDefenceDamage    = gf_GetRandom( g_pDefenceSkill->m_iHpChange ) + 1;
					m_pPlayer->m_iHp -= iDefenceDamage;
					if( m_pPlayer->m_iHp < 0 )      m_pPlayer->m_iHp = 1;

					pDefenceDamage->wUserCode   = pTargetPlayer->GetSelfCode();
					pDefenceDamage->wSkillCode  = g_pDefenceSkill->GetId();
					pDefenceDamage++;
					iDefenceConter++;
					break;  // 如果允许多重激发，Mark这一行即可
				}
			}
			if( 0 != iDefenceConter )
			{
				MsgDefeDamage.Msgs[0].Size =  sizeof(SNMDefenceDamage) * iDefenceConter;
				m_pPlayer->GetInMap()->SendMsgNearPosition_Close( MsgDefeDamage,  m_pPlayer->GetPosX(), m_pPlayer->GetPosY() );
			}
		}
#endif
    //
#ifdef _FUNCTION_RING_3_8_
    if( ( pTargetPlayer->m_qwSpecialStatus & SPE_STATUS_UNSEE ) ||
        ( pTargetPlayer->m_qwSpecialStatus & SPE_STATUS_DIVE ) )
    {
      pTargetPlayer->ClearSpecialStatus( SPE_STATUS_UNSEE );
      pTargetPlayer->ClearSpecialStatus( SPE_STATUS_DIVE );
      pTargetPlayer->SendSpecialStatusToNeighbor();
    }
#endif
		pTargetPlayer->Damage( pDamageInfo, (CLife*)m_pPlayer, nDecMp, nDecSp );
	}
	else
	{
    ////////////////////////////////////////////////////////
    // 霸体功能处理
    ////////////////////////////////////////////////////////
    int psbParam = pTarget->GetBearPsb();
    if( psbParam >= gf_GetRandom(100) )     pDamageInfo->qwSpecial |= SPE_STATUS_UNINJURED;
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
    // 套装吸收
    if( m_pPlayer->HaveSuitInEquip() && m_pPlayer->m_iSuitSuckOdds > gf_GetRandom(100)  ) 
    {
      if( m_pPlayer->m_iSuitSuckHp )
      {
        m_pPlayer->m_iHp       += ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckHp / 100 );
        if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckMp )
      {
        m_pPlayer->m_iMp       +=  pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckMp / 100;
        if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckSp )
      { 
        m_pPlayer->m_iSp       += pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckSp / 100;
        if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
    }
		// 镶嵌吸收被攻击方的 HP MP SP
		if(	m_pPlayer->m_iTesseraSuckOdds > gf_GetRandom(100) )
		{
			if( m_pPlayer->m_iTesseraSuckHp )
			{
				m_pPlayer->m_iHp       += pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckHp / 100;
				if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckMp )
			{
        m_pPlayer->m_iMp       += pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckMp / 100;
				if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckSp )
			{
        m_pPlayer->m_iSp       += pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckSp / 100;
				if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
		}
#endif // _DEBUG_MICHAEL_TESSERA_EX_

#ifdef _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
    g_iDamageCount++;
#endif
		((CMonster*)pTarget)->Damage( pDamageInfo , m_pPlayer );
	}
	CalItemWaste( pTarget );
	return true;
}

//==========================================================================================
//desc:				对参数指定的生物做攻击
//parameter:	pTarget(目标),pDamageInfo(填充伤害信息)
//return   :  bool
//==========================================================================================
inline bool CSkill::PlayerCommonAttack( CLife *pTarget, SNMDamage *pDamageInfo, int iAddiDamage )
{
	short       nDamage = 0, nDecMp = 0, nDecSp = 0;
	CPlayer     *pTargetPlayer = NULL;
	QWORD			  qwFlag = 0;

#ifdef _DEBUG_OPEN_ACCESS_BAD_PTR_
  if( IsBadReadPtr( pTarget, sizeof( CLife ) ) )
  {
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
    FuncName("CSkill::PlayerCommonAttack");
    _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "===>>>%s Find Bad Ptr(%08x) When Common Attack Life", m_pPlayer->GetAccount(), pTarget );
    g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szDamageLog );
#endif
#endif
    return false;
  }
#endif
  // pTarget is GM ,and is GM_STATUS_NODIED status 
  //////////////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-09
#ifdef  EVILWEAPON_3_6_VERSION
  if( pTarget->IsPlayer() && (
       ( ((CPlayer*)pTarget)->GetGMStatus() & GM_STATUS_NODIED ) ||
       ( ((CPlayer*)pTarget)->GetSpecialStatus() & SPE_STATUS_GODEMODE )
     ) )
#else
  if( pTarget->IsPlayer() && ((CPlayer*)pTarget)->GetGMStatus() & GM_STATUS_NODIED ) 
#endif
  ///////////////////////////////////////////////////////////////////////////////////
  {
    pDamageInfo->wTargetCode = pTarget->GetCode();
		pDamageInfo->wSubHp      = 0;
		//pDamageInfo->wSubMp      = 0;
		//pDamageInfo->wSubSp      = 0;
		//pDamageInfo->wTargetX    = pTarget->GetPosX();
		//pDamageInfo->wTargetY    = pTarget->GetPosY();
    pDamageInfo->qwSpecial   = 0;
		return true;
  }

//#ifdef _DEBUG_LOG_SKILL_
//	sprintf(g_szDamageLog,"%s:**************** start to attck one target ******************",m_pPlayer->GetAccount() );
//	g_DamageLog.Write(g_szDamageLog);
//#endif

#ifdef FIX_BUG_ATTACKED_WHILE_LOGOUT
	if( pTarget->IsPlayer() &&
      (
        ((CPlayer*)pTarget)->GetClientState() < CLIENTSTATE_CONNECT ||
        ((CPlayer*)pTarget)->m_bClientClaimLogout
      )
    )
	{
		return true;
	}
#endif

  int iThisCritical = IsPlayerCritical( pTarget );
	if( iThisCritical == 1 )
	{
		// 必殺成功
		qwFlag   |= SPE_STATUS_CRITICAL;
		nDamage   = CalDamageForPlayer( pTarget, TRUE, iAddiDamage );
	}
	else
	{
		if( !PlayerCheckHit( pTarget ) )
		{
			//handle when not hit the target
      pDamageInfo->wTargetCode = pTarget->GetCode();
		  pDamageInfo->wSubHp      = 0;
		  //pDamageInfo->wSubMp      = 0;
		  //pDamageInfo->wSubSp      = 0;
		  //pDamageInfo->wTargetX    = pTarget->GetPosX();
		  //pDamageInfo->wTargetY    = pTarget->GetPosY();
      pDamageInfo->qwSpecial   = 0;

      // 无视必杀
			if( iThisCritical == -1 ) 
			{
				pDamageInfo->qwSpecial |= SPE_STATUS_CRITICAL;
				pDamageInfo->qwSpecial |= SPE_STATUS_UNCRITICAL;
			}
      //
      if( pTarget->IsMonster() )
      {
#ifdef _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
        g_iDamageCount++;
#endif
        ((CMonster*)pTarget)->Damage( pDamageInfo , m_pPlayer );
      }
      else
      {
        if( m_pPlayer->IsEnemyPlayer( (CPlayer*)pTarget ) ) pDamageInfo->qwSpecial |= SPE_STATUS_ENEMY_GUILD;
      }
			return true;
		}
		nDamage = CalDamageForPlayer( pTarget, FALSE, iAddiDamage );
	}
  //
  CalAllSpecialStatus( pTarget );
  qwFlag |= pTarget->GetSpecialStatus() & (~SPE_STATUS_CRITICAL);

  //
  pDamageInfo->wTargetCode = pTarget->GetCode();
  pDamageInfo->wSubHp      = nDamage;
  //pDamageInfo->wSubMp      = 0;
  //pDamageInfo->wSubSp      = 0;
  //pDamageInfo->wTargetX    = pTarget->GetPosX();
  //pDamageInfo->wTargetY    = pTarget->GetPosY();
  //
  if( pTarget->IsPlayer() )
	{
		pTargetPlayer = (CPlayer*)(pTarget);
    ////////////////////////////////////////////////////////
    // 霸体功能处理
    ////////////////////////////////////////////////////////
    int     psbParam = 1;
    switch( pTargetPlayer->GetOccupation() )
    {
    case OCCU_BLADEMAN: //诡道
    case OCCU_BLADEMANF:
      psbParam = g_BearGui;
      break;
    case OCCU_SWORDMAN://剑宗
    case OCCU_SWORDMANF:
      psbParam = g_BearJian;
      break;
    case OCCU_PIKEMAN://戟门
    case OCCU_PIKEMANF:
      psbParam = g_BearJi;
      break;
    case OCCU_WIZARD://幻道
    case OCCU_WIZARDF:
      psbParam = g_BearHuan;
      break;
    default:
      psbParam = g_BearJi;
      break;
    }
    psbParam = pTarget->GetMaxHp() * psbParam / 100;
    if( nDamage <= psbParam )
    {
      qwFlag |= SPE_STATUS_UNINJURED;
    }
    else
    {
      psbParam = pTarget->GetBearPsb();
      if( psbParam >= gf_GetRandom(100) )
      {
        qwFlag |= SPE_STATUS_UNINJURED;
      }
    }
    //
    if( m_pPlayer->IsEnemyPlayer( pTargetPlayer ) )     qwFlag |= SPE_STATUS_ENEMY_GUILD;
    //
    pDamageInfo->qwSpecial = qwFlag;

#ifdef _DEBUG_MICHAEL_TESSERA_EX_
    // About Suit Equip Data Cal
    if( iThisCritical == -1 )
		{
			pDamageInfo->qwSpecial |= SPE_STATUS_CRITICAL;
			pDamageInfo->qwSpecial |= SPE_STATUS_UNCRITICAL;
		}
    if( pTargetPlayer->HaveSuitInEquip() )
    {
      // Target Rebound Damage
      if( pTargetPlayer->m_iSuitRebound && pTargetPlayer->m_iSuitReboundOdds > gf_GetRandom(100) )
      {
        m_pPlayer->m_iHp -= ( pDamageInfo->wSubHp * pTargetPlayer->m_iSuitRebound / 100 );
        if( m_pPlayer->m_iHp < 0 )    m_pPlayer->m_iHp = 1;
        pDamageInfo->qwSpecial |= SPE_STATUS_REBOUND_ATTACK;
      }
    }
    // Attacker Suck Hp Mp Sp
    if( m_pPlayer->HaveSuitInEquip() && m_pPlayer->m_iSuitSuckOdds > gf_GetRandom(100)  )
    {
      if( m_pPlayer->m_iSuitSuckHp )
      {
        m_pPlayer->m_iHp += ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckHp / 100 );
        if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckMp )
      {
        nDecMp = ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckMp / 100 );
        m_pPlayer->m_iMp     += nDecMp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iMp -= nDecMp;
        if( pTargetPlayer->m_iMp < 0 )                pTargetPlayer->m_iMp = 0;
#endif
        if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;

        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckSp )
      {
        nDecSp = ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckSp / 100 );
        m_pPlayer->m_iSp     += nDecSp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iSp -= nDecSp;
        if( pTargetPlayer->m_iSp < 0 )                pTargetPlayer->m_iSp = 0;
#endif
        if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
        //
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
    }

    if( pTargetPlayer->m_iTesseraRebound && pTargetPlayer->m_iTesseraReboundOdds > gf_GetRandom(100) )
    {	
      // 反弹
#ifdef DEPRESS_TESS_DEFENCE
      m_pPlayer->m_iHp -= ( pDamageInfo->wSubHp * pTargetPlayer->m_iTesseraRebound * DepressDefence() / 10000 );	
#else
		  m_pPlayer->m_iHp -= ( pDamageInfo->wSubHp * pTargetPlayer->m_iTesseraRebound / 100 );	
#endif
			if( m_pPlayer->m_iHp < 0 )      m_pPlayer->m_iHp = 1;
			pDamageInfo->qwSpecial |= SPE_STATUS_REBOUND_ATTACK;
    }
		//
		if( !pTargetPlayer->m_listTesseraDefence.empty() )
		{
			// 被攻击时激发出技能
			static CSrvBaseSkill									 *g_pDefenceSkill = NULL;
			static list<CSrvBaseSkill*>::iterator   Iter_Sbs;
			static int															iDefenceDamage  = 0;
			int																			iDefenceConter  = 0;
			//
			SNMDefenceDamage     *pDefenceDamage  = (SNMDefenceDamage*)(MsgDefeDamage.Msgs[0].Data);
			//
			for( Iter_Sbs  = pTargetPlayer->m_listTesseraDefence.begin();
					 Iter_Sbs != pTargetPlayer->m_listTesseraDefence.end(); Iter_Sbs++ )
			{
				g_pDefenceSkill = (*Iter_Sbs);
				if( g_pDefenceSkill->m_iProbability > gf_GetRandom( 100 ) )
				{
					// Record The Addi Skill
					iDefenceDamage    = gf_GetRandom( g_pDefenceSkill->m_iHpChange ) + 1;
					m_pPlayer->m_iHp -= iDefenceDamage;
					if( m_pPlayer->m_iHp < 0 )      m_pPlayer->m_iHp = 1;

					pDefenceDamage->wUserCode   = pTargetPlayer->GetSelfCode();
					pDefenceDamage->wSkillCode  = g_pDefenceSkill->GetId();
					pDefenceDamage++;
					iDefenceConter++;
					break; 
				}
			}
			if( 0 != iDefenceConter )
			{
				MsgDefeDamage.Msgs[0].Size =  sizeof(SNMDefenceDamage) * iDefenceConter;
				m_pPlayer->GetInMap()->SendMsgNearPosition_Close( MsgDefeDamage,  m_pPlayer->GetPosX(), m_pPlayer->GetPosY() );
			}
		}

		if(	m_pPlayer->m_iTesseraSuckOdds > gf_GetRandom(100) )
		{
			// 吸收被攻击方的 HP MP SP
			if( m_pPlayer->m_iTesseraSuckHp )
			{
				m_pPlayer->m_iHp += ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckHp / 100 );
				if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckMp )
			{
        nDecMp = ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckMp / 100 );
				m_pPlayer->m_iMp += nDecMp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iMp -= nDecMp;
        if( pTargetPlayer->m_iMp < 0 )                pTargetPlayer->m_iMp = 0;
#endif
				if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckSp )
			{
        nDecSp = ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckSp / 100 );
        m_pPlayer->m_iSp     += nDecSp;
        //
#ifdef _DEBUG_SUCK_MP_SP_MUST_DECREATE_TARGET_MP_SP_
        pTargetPlayer->m_iSp -= nDecSp;
        if( pTargetPlayer->m_iSp < 0 )                pTargetPlayer->m_iSp = 0;
#endif
				if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
		}
#endif
    //
#ifdef _FUNCTION_RING_3_8_
    if( ( pTargetPlayer->m_qwSpecialStatus & SPE_STATUS_UNSEE ) ||
        ( pTargetPlayer->m_qwSpecialStatus & SPE_STATUS_DIVE ) )
    {
      pTargetPlayer->ClearSpecialStatus( SPE_STATUS_UNSEE );
      pTargetPlayer->ClearSpecialStatus( SPE_STATUS_DIVE );
      pTargetPlayer->SendSpecialStatusToNeighbor();
    }
#endif
    //
    pTargetPlayer->Damage( pDamageInfo, (CLife*)m_pPlayer, nDecMp, nDecSp );
#ifdef _DEBUG_SHOW_PLAYER_PK_DAMAGE_DATA_
    //
    
    //
#endif
	}
	else
	{
    ////////////////////////////////////////////////////////
    // 霸体功能处理
    ////////////////////////////////////////////////////////
    int psbParam = pTarget->GetBearPsb();
    if( psbParam >= gf_GetRandom(100) )
    {
      qwFlag |= SPE_STATUS_UNINJURED;
    }
    //
    pDamageInfo->qwSpecial = qwFlag;
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
    // Attacker Suck Hp Mp Sp
    if( m_pPlayer->HaveSuitInEquip() &&  m_pPlayer->m_iSuitSuckOdds > gf_GetRandom(100)  )
    {
      if( m_pPlayer->m_iSuitSuckHp )
      {
        m_pPlayer->m_iHp += ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckHp / 100 );
        if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckMp )
      {
        m_pPlayer->m_iMp += ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckMp / 100 );
        if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
      if( m_pPlayer->m_iSuitSuckSp )
      {
        m_pPlayer->m_iSp += ( pDamageInfo->wSubHp * m_pPlayer->m_iSuitSuckSp / 100 );
        if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
        pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
      }
    }

		if(	m_pPlayer->m_iTesseraSuckOdds > gf_GetRandom(100) )
		{
			// 吸收被攻击方的 HP MP SP
			if( m_pPlayer->m_iTesseraSuckHp )
			{
				m_pPlayer->m_iHp += ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckHp / 100 );
				if( m_pPlayer->m_iHp > m_pPlayer->m_iMaxHp )  m_pPlayer->m_iHp = m_pPlayer->m_iMaxHp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckMp )
			{
				m_pPlayer->m_iMp += ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckMp / 100 );
				if( m_pPlayer->m_iMp > m_pPlayer->m_iMaxMp )  m_pPlayer->m_iMp = m_pPlayer->m_iMaxMp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
			if( m_pPlayer->m_iTesseraSuckSp )
			{
				m_pPlayer->m_iSp += ( pDamageInfo->wSubHp * m_pPlayer->m_iTesseraSuckSp / 100 );
				if( m_pPlayer->m_iSp > m_pPlayer->m_iMaxSp )  m_pPlayer->m_iSp = m_pPlayer->m_iMaxSp;
				pDamageInfo->qwSpecial |= SPE_STATUS_SUCK_TARGET;
			}
		}
#endif
    //
#ifdef _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
    g_iDamageCount++;
#endif
		((CMonster*)pTarget)->Damage( pDamageInfo , m_pPlayer );
	}
  //
	CalItemWaste( pTarget );
	return true;
}
//==========================================================================================
//desc:			 monster 攻击 player
//parameter:	CPlayer *pTarget(目标player),pDamageInfo(填充伤害信息)
//return   :  bool
//==========================================================================================
inline bool CSkill::MonsterAttackPlayer( CPlayer *pTarget , SNMDamage *pDamageInfo )
{
//#ifdef _DEBUG_WILDCAT_
//	FuncName( "CSkill::MonsterAttackPlayer" );
//#endif
	
	short int			nDamage;
	QWORD					qwFlag = 0;

  // pTarget is GM ,and is GM_STATUS_NODIED status 
  /////////////////////////////////////////////////
  //Add by CECE 2004-04-09
#ifdef  EVILWEAPON_3_6_VERSION
  if( ( pTarget->GetGMStatus() & GM_STATUS_NODIED ) ||
      ( pTarget->GetSpecialStatus() & SPE_STATUS_GODEMODE ) )
#else
  if( pTarget->GetGMStatus() & GM_STATUS_NODIED ) 
#endif
  /////////////////////////////////////////////////
  {
    pDamageInfo->wTargetCode = pTarget->GetCode();
		pDamageInfo->wSubHp      = 0;
		//pDamageInfo->wSubMp      = 0;
		//pDamageInfo->wSubSp      = 0;
		//pDamageInfo->wTargetX    = pTarget->GetPosX();
		//pDamageInfo->wTargetY    = pTarget->GetPosY();
    pDamageInfo->qwSpecial   = 0;
		return false;
  }
  //
  int     iThisCritical = IsMonsterCritical( pTarget );
	if( iThisCritical == 1 )
	{
		// 必殺成功
		qwFlag  |= SPE_STATUS_CRITICAL;
		nDamage = CalDamageForMonster( pTarget, TRUE );
	}
	else
	{
		if( !MonsterCheckHit( pTarget ) )
		{
			// handle when not hit the target
      pDamageInfo->wTargetCode = pTarget->GetCode();
		  pDamageInfo->wSubHp      = 0;
		  //pDamageInfo->wSubMp      = 0;
		  //pDamageInfo->wSubSp      = 0;
		  //pDamageInfo->wTargetX    = pTarget->GetPosX();
		  //pDamageInfo->wTargetY    = pTarget->GetPosY();
      pDamageInfo->qwSpecial   = 0;
      if( iThisCritical == -1 ) 
			{
				pDamageInfo->qwSpecial |= SPE_STATUS_UNCRITICAL;
				pDamageInfo->qwSpecial |= SPE_STATUS_CRITICAL;
			}
			return false;
		}
		nDamage = CalDamageForMonster( pTarget, FALSE );
	}
  //
  CalAllSpecialStatus( pTarget );
  qwFlag |= pTarget->GetSpecialStatus();
  //
	// add skill exp
	AddExp( 1 );
  ////////////////////////////////////////////////////////
  // 霸体功能处理
  ////////////////////////////////////////////////////////
  int psbParam = 1;
  switch( pTarget->GetOccupation() )
  {
  case OCCU_BLADEMAN: //诡道
  case OCCU_BLADEMANF:
    psbParam = g_BearGui;
    break;
  case OCCU_SWORDMAN://剑宗
  case OCCU_SWORDMANF:
    psbParam = g_BearJian;
    break;
  case OCCU_PIKEMAN://戟门
  case OCCU_PIKEMANF:
    psbParam = g_BearJi;
    break;
  case OCCU_WIZARD://幻道
  case OCCU_WIZARDF:
    psbParam = g_BearHuan;
    break;
  }
  psbParam = pTarget->GetMaxHp() * psbParam / 100;
  if( nDamage <= psbParam )
  {
    qwFlag |= SPE_STATUS_UNINJURED;
  }
  else
  {
    psbParam = pTarget->GetBearPsb();
    if( psbParam >= gf_GetRandom(100) )
    {
      qwFlag |= SPE_STATUS_UNINJURED;
    }
  }
  pDamageInfo->wTargetCode = pTarget->GetCode();
	pDamageInfo->wSubHp      = nDamage;
  //pDamageInfo->wSubMp      = 0;
  //pDamageInfo->wSubSp      = 0;
  //pDamageInfo->wTargetX    = pTarget->GetPosX();
  //pDamageInfo->wTargetY    = pTarget->GetPosY();
  pDamageInfo->qwSpecial   = qwFlag;
#ifdef _DEBUG_MICHAEL_TESSERA_EX_

  if( iThisCritical == -1 ) 
	{
		pDamageInfo->qwSpecial |= SPE_STATUS_UNCRITICAL;
		pDamageInfo->qwSpecial |= SPE_STATUS_CRITICAL;
	}
  //
  if( pTarget->HaveSuitInEquip() )
  {
    if( pTarget->m_iSuitRebound && pTarget->m_iSuitReboundOdds > gf_GetRandom(100))
    {
#ifdef _DEBUG_VERSION_FOR_BEIJING_
      if( m_pMonster->GetType() & NPC_ATTRI_BOSS )      //Boss 对反弹修改
      {
        m_pMonster->m_iHp    -= ( pDamageInfo->wSubHp * pTarget->m_iSuitRebound / 100 ) * 10 / 100;
      }
      else
        m_pMonster->m_iHp    -= ( pDamageInfo->wSubHp * pTarget->m_iSuitRebound / 100 );
#else
      m_pMonster->m_iHp      -= ( pDamageInfo->wSubHp * pTarget->m_iSuitRebound / 100 );
#endif
      if( m_pMonster->m_iHp < 0 )    m_pMonster->m_iHp = 1;
      pDamageInfo->qwSpecial |= SPE_STATUS_REBOUND_ATTACK;
    }
  }

  if( pTarget->m_iTesseraRebound && pTarget->m_iTesseraReboundOdds > gf_GetRandom(100) )
  {	
#ifdef _DEBUG_VERSION_FOR_BEIJING_
    if(m_pMonster->GetType() & NPC_ATTRI_BOSS) 
    {
      m_pMonster->m_iHp     -= ( pDamageInfo->wSubHp * pTarget->m_iTesseraRebound / 100 ) * 10 / 100;  
    }
    else
      m_pMonster->m_iHp     -= ( pDamageInfo->wSubHp * pTarget->m_iTesseraRebound / 100 );
#else
		m_pMonster->m_iHp       -= ( pDamageInfo->wSubHp * pTarget->m_iTesseraRebound / 100 );
#endif
		if( m_pMonster->m_iHp < 0 )      m_pMonster->m_iHp = 1;
		pDamageInfo->qwSpecial |= SPE_STATUS_REBOUND_ATTACK;
  }


	// 镶嵌防守技能
	if( !pTarget->m_listTesseraDefence.empty() )
	{
		static CSrvBaseSkill									 *g_pDefenceSkill = NULL;
		static list<CSrvBaseSkill*>::iterator   Iter_Sbs;
		static int															iDefenceDamage  = 0;
		int																			iDefenceConter  = 0;
		//
		SNMDefenceDamage     *pDefenceDamage  = (SNMDefenceDamage*)(MsgDefeDamage.Msgs[0].Data);
		//
		for( Iter_Sbs  = pTarget->m_listTesseraDefence.begin();
				 Iter_Sbs != pTarget->m_listTesseraDefence.end(); Iter_Sbs++ )
		{
			g_pDefenceSkill = (*Iter_Sbs);
			if( g_pDefenceSkill->m_iProbability > gf_GetRandom( 100 ) )
			{
				// Record The Addi Skill
				iDefenceDamage     = gf_GetRandom( g_pDefenceSkill->m_iHpChange ) + 1;
				m_pMonster->m_iHp -= iDefenceDamage;
				if( m_pMonster->m_iHp < 0 )      m_pMonster->m_iHp = 1;

				pDefenceDamage->wUserCode   = pTarget->GetSelfCode();
				pDefenceDamage->wSkillCode  = g_pDefenceSkill->GetId();
				pDefenceDamage++;
				iDefenceConter++;
				break;  // 如果允许多重激发，Mark这一行即可
			}
		}
		if( 0 != iDefenceConter )
		{
			MsgDefeDamage.Msgs[0].Size =  sizeof(SNMDefenceDamage) * iDefenceConter;
			m_pMonster->GetInMap()->SendMsgNearPosition_Close( MsgDefeDamage,  m_pMonster->GetPosX(), m_pMonster->GetPosY() );
		}
	}
#endif
  //
	pTarget->Damage( pDamageInfo, m_pMonster, 0, 0 );
	CalItemWaste(pTarget);
	return true;
}

//==========================================================================================
//desc:				判断必杀几率
//parameter:	
//return   :  bool
//==========================================================================================
inline int CSkill::IsMonsterCritical(CPlayer * pTarget)
{
  static int  iAddiCrititical1 = 0;
  int         iCritical = ( m_pBase->GetCriticalHit() > gf_GetRandom( 100 ) ) ? 1 : 0;
  //
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  if( iCritical && 0 != ( iAddiCrititical1 = pTarget->m_iSuitCritical + pTarget->m_iTesseraCritical ) )
  {
    if( iAddiCrititical1 > gf_GetRandom( 100 ) )
    {
#ifdef _DEBUG
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
      FuncName("CSkill::IsMonsterCritical");
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "==>>> %s Dodge Monster[%s]'s Critical Attack By %d",
               ((CPlayer*)pTarget)->GetAccount(), m_pMonster->GetName(),
               ((CPlayer*)pTarget)->m_iSuitCritical );
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szDamageLog );
#endif
#endif
      iCritical = -1;
    }
  }
#endif // _DEBUG_MICHAEL_TESSERA_EX_
	return iCritical;
}
//==========================================================================================
inline int CSkill::IsPlayerCritical( CLife * pTarget )
{
  static int  iAddiCritical = 0;
  int         iCritical = ( ( m_pBase->GetCriticalHit() + m_pPlayer->GetAllCritical() ) > gf_GetRandom( 100 ) ) ? 1 : 0;
  //
  if( iCritical && pTarget->IsPlayer() )
  {
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
		if( 0 != ( iAddiCritical = ((CPlayer*)pTarget)->m_iSuitCritical + ((CPlayer*)pTarget)->m_iTesseraCritical ) )
    {
      if( iAddiCritical > gf_GetRandom( 100 ) )
      {
#ifdef _DEBUG
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
        FuncName("CSkill::IsPlayerCritical");
        _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "==>>> %s Dodge Player[%s]'s Critical Attack By %d",
                ((CPlayer*)pTarget)->GetAccount(), m_pPlayer->GetAccount(),
                ((CPlayer*)pTarget)->m_iSuitCritical );
        g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoMsg( g_szDamageLog );
#endif
#endif
        iCritical = -1;
      }
    }
#endif // _DEBUG_MICHAEL_TESSERA_EX_
  }
 	return iCritical;
}
//==========================================================================================
//desc:				计算player 攻击 life 是否命中
//parameter:	pTarget(目标life)
//return   :  bool
//==========================================================================================
inline bool CSkill::PlayerCheckHit(CLife *pTarget)
{
  int maxHit, minHit;
  int occuEnemy = OCCU_NONE;
  if( pTarget->IsPlayer() )
  {
    occuEnemy = ((CPlayer*)pTarget)->GetOccupation();
  }
  int nDodgeEnsure;
  switch( m_pPlayer->GetOccupation() )
  {
    case OCCU_BLADEMAN:
    case OCCU_BLADEMANF: //诡道
      minHit       = g_MinHit_Gui;
      nDodgeEnsure = g_MinFlee_Gui;
      break;
    case OCCU_SWORDMAN:
    case OCCU_SWORDMANF: //剑宗
      minHit       = g_MinHit_Jian;
      nDodgeEnsure = g_MinFlee_Jian;
      break;
    case OCCU_PIKEMAN:
    case OCCU_PIKEMANF: //戟门
      minHit       = g_MinHit_Ji;
      nDodgeEnsure = g_MinFlee_Ji;
      break;
    case OCCU_WIZARD:
    case OCCU_WIZARDF: //幻道
      minHit       = g_MinHit_Huan;
      nDodgeEnsure = g_MinFlee_Huan;
      break;
    default:
      minHit       = g_MinHit;
      break;
  }
  switch( occuEnemy )
  {
    case OCCU_BLADEMAN:
    case OCCU_BLADEMANF: //诡道
      maxHit       = g_MaxHit_Gui;
      nDodgeEnsure = g_MaxFlee_Gui;
      break;
    case OCCU_SWORDMAN:
    case OCCU_SWORDMANF: //剑宗
      maxHit       = g_MaxHit_Jian;
      nDodgeEnsure = g_MaxFlee_Jian;
      break;
    case OCCU_PIKEMAN:
    case OCCU_PIKEMANF: //戟门
      maxHit       = g_MaxHit_Ji;
      nDodgeEnsure = g_MaxFlee_Ji;
      break;
    case OCCU_WIZARD:
    case OCCU_WIZARDF: //幻道
      maxHit       = g_MaxHit_Huan;
      nDodgeEnsure = g_MaxFlee_Huan;
      break;
    default:
      maxHit       = g_MaxHit;
      break;
  }
  
  ////////////////////////////////////////////////////////
  // 公式:
  //      nPHit=P_Level*2+P_Hit*0.4+INT(P_Hit/25)^2/2
  //      nEnemyDodge=T_Level+T_Dodge*0.4+INT(T_Dodge/25)^2/2
  //
  //      nLastHit=nEnemyDodge+20-nPHit+DodgeEnsure
  ////////////////////////////////////////////////////////
  int nPHit       = m_pPlayer->GetLevel()*2 + m_pPlayer->GetHit() * 0.4 + 
                    (m_pPlayer->GetHit() * m_pPlayer->GetHit())/625;
  //nPHit+=GetChainHitRate()*g_ChainHit;
  //
  int nEnemyDodge = pTarget->GetLevel() + pTarget->GetDodge() * 0.4 + 
                    (pTarget->GetDodge() * pTarget->GetDodge())/625;
  //
  int nFinalHit = nPHit + 20 - nEnemyDodge + nDodgeEnsure;
  if( nFinalHit > maxHit )      nFinalHit = maxHit;
  else if( nFinalHit < minHit ) nFinalHit = minHit;
  int nRandomHit = gf_GetRandom( 100 );

#ifdef _DEBUG
  if( !strcmp( m_pPlayer->GetAccount(), "TEST8" ) ||
      !strcmp( m_pPlayer->GetAccount(), "TEST6" ) )
  {
    if( nFinalHit > nRandomHit )
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Damage, %s Level(%d),Hit(%d) Attack %d Level(%d),Hit(%d),  ",
               m_pPlayer->GetAccount(), m_pPlayer->GetLevel(), m_pPlayer->GetHit(),
               pTarget->GetCode(), pTarget->GetLevel(), pTarget->GetDodge() );
    }
    else
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Miss, %s Level(%d),Hit(%d) Attack %d Level(%d),Hit(%d),  ",
               m_pPlayer->GetAccount(), m_pPlayer->GetLevel(), m_pPlayer->GetHit(),
               pTarget->GetCode(), pTarget->GetLevel(), pTarget->GetDodge() );
    }
    g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
    g_DamageLog.Write( g_szDamageLog );
  }
#endif

	return (nFinalHit > nRandomHit);
}
//==========================================================================================
//desc:				计算monster 攻击 player 是否命中
//parameter:	pTarget(目标player)
//return   :  bool
//==========================================================================================
inline bool CSkill::MonsterCheckHit(CPlayer *pTarget)
{
	int			P_HIT, T_DG;

  P_HIT = m_pMonster->GetLevel() + m_pMonster->GetHit() * 0.4 + 
          (m_pMonster->GetHit() * m_pMonster->GetHit())/625 + m_pBase->GetHitChange();
  //
  T_DG  = pTarget->GetLevel() + pTarget->GetDodge() * 0.4 + 
          (pTarget->GetDodge() * pTarget->GetDodge())/625;
  //
  P_HIT = P_HIT + 20 - T_DG + g_BaseHit;
	// + ChainHit %(monster have no)
	if( P_HIT > g_MaxHit )      P_HIT = g_MaxHit;
	else if( P_HIT < g_MinHit ) P_HIT = g_MinHit;
  int nRandomHit = gf_GetRandom( 100 );
#ifdef _FUNCTION_RING_3_8_
  pTarget->m_dwAttackStateTime = ClientTickCount + PLAYER_ATTACK_STATUS_TIME;
  if( ( pTarget->m_qwSpecialStatus & SPE_STATUS_UNSEE ) ||
      ( pTarget->m_qwSpecialStatus & SPE_STATUS_DIVE ) )
  {
    pTarget->ClearSpecialStatus( SPE_STATUS_UNSEE );
    pTarget->ClearSpecialStatus( SPE_STATUS_DIVE );
    // 将玩家的状态发送给周围的玩家
    pTarget->SendSpecialStatusToNeighbor();
  }
#endif
#ifdef _DEBUG
  if( pTarget->IsPlayer() &&
      (!strcmp( ((CPlayer*)pTarget)->GetAccount(), "TEST8" ) ||
      !strcmp( ((CPlayer*)pTarget)->GetAccount(), "TEST6" ) ) )
  {
    if( P_HIT > nRandomHit )
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Damage, %d Level(%d),Hit(%d) Attack %s Level(%d),Hit(%d)",
               m_pMonster->GetCode(), m_pMonster->GetLevel(), m_pMonster->GetDodge(),
               ((CPlayer*)pTarget)->GetAccount(), ((CPlayer*)pTarget)->GetLevel(), ((CPlayer*)pTarget)->GetHit() );
    }
    else
    {
      _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "Miss, %d Level(%d),Hit(%d) Attack %s Level(%d),Hit(%d)",
               m_pMonster->GetCode(), m_pMonster->GetLevel(), m_pMonster->GetDodge(),
               ((CPlayer*)pTarget)->GetAccount(), ((CPlayer*)pTarget)->GetLevel(), ((CPlayer*)pTarget)->GetHit() );
    }
    g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
    g_DamageLog.Write( g_szDamageLog );
  }
#endif

	return ( P_HIT >= nRandomHit );
}
///////////////////////////////////////////////////////////////////////////
//
//  计算防具镶嵌宝石的效果.
//
inline float CSkill::GetShieldTesseraJinx(CPlayer* pTarget, CLife *pAttacker)
{
  if( !pAttacker->IsMonster() )       return 0;
  //
  CMonster    *pMAttacker = (CMonster*)pAttacker;
  CItem       *pEquip;
  float       nTotal = 0;
  //
  for( int i = ITEM_EQUIPWHERE_BODY; i < ITEM_EQUIPWHERE_MAX; i++ )
  {
    pEquip = pTarget->GetEquipment( i );
    if( pEquip != NULL )
    {
      // 克怪物体形，相性
      if( pEquip->m_wElement&0xFFE0 )
      {
        float   FMJinx;
        if( 0 != ( FMJinx = ( ( pEquip->m_wElement & pMAttacker->m_wElement ) & 0xFE0 ) ) )
        {
          FMJinx = GetBitCountFor_Element( FMJinx );
          nTotal += FMJinx * g_MyJinx;
        }
      }
      // 武器克怪物种族
      if( pEquip->m_dwRaceAttri )
      {
        float   RaceJinx;
        if( 0 != ( RaceJinx = pEquip->m_dwRaceAttri & pMAttacker->m_dwRaceAttri ) )
        {
          RaceJinx = GetBitCountFor_Race( RaceJinx );
          nTotal += RaceJinx * pEquip->m_wRaceBonuRate;
        }
      }
      // 武器克魔王
      if( pMAttacker->GetBaseId() == pEquip->m_dwBossCode )
      {
        nTotal += pEquip->m_wBossBonuRate;
      }
    }
  }
  return (nTotal / 100);
}
///////////////////////////////////////////////////////////////////////////
//
//  计算攻击方武器镶嵌宝石的效果
//
inline float CSkill::GetWeaponTesseraJinx(CLife *pAttacker, CLife* pTarget)
{
  if( !pTarget->IsMonster() || !pAttacker->IsPlayer() )    return 0;
  //
  CPlayer   *pPAttack   = (CPlayer*)pAttacker;
  CMonster  *pMRecovery = (CMonster*)pTarget;
  CItem     *pEquip;
  float     nTotal = 0;
  //
  pEquip = pPAttack->GetEquipment( ITEM_EQUIPWHERE_HAND );
  if( pEquip != NULL )
  {
#ifdef RESTRAIN_BODY
    CSrvBaseItem  *pTesseraTemp = NULL;
    WORD           wElement;
    WORD           wElementTemp = 0;
    for( int iLoop = 0; iLoop<MAX_ITEM_SLOT; iLoop++)
    {
      pTesseraTemp = g_pBase->GetBaseItem(pEquip->m_wTesseraItemID[iLoop]);     
      if( pTesseraTemp!=NULL ) 
      {
        wElement = pTesseraTemp->GetElement();
        if ( wElement == 32 || wElement == 64 || wElement == 128 || wElement == 256 ) 
        {
          wElementTemp += wElement;
        }
      }
    }
#endif //RESTRAIN_BODY
    // 克怪物五行，体形，相性
    if( pEquip->m_wElement&0xFFE0 )
    {
      float FMJinx;
      if( 0 != ( FMJinx = ( (pEquip->m_wElement
      #ifdef RESTRAIN_BODY //除去克体型血泊的Element
         - wElementTemp
      #endif //RESTRAIN_BODY        
        ) & pMRecovery->m_wElement & 0xFFE0 ) ) )
      {
        FMJinx = GetBitCountFor_Element( FMJinx );
        nTotal += FMJinx * g_MyJinx;
      }
    }
    // 武器克怪物种族
    if( pEquip->m_dwRaceAttri )
    {
      float RaceJinx;
      if( 0 != ( RaceJinx = pEquip->m_dwRaceAttri & pMRecovery->m_dwRaceAttri ) )
      {
        RaceJinx = GetBitCountFor_Race( RaceJinx );
        nTotal += RaceJinx * pEquip->m_wRaceBonuRate;
      }
    }
    // 武器克魔王
    if( pMRecovery->GetBaseId() == pEquip->m_dwBossCode )
    {
      nTotal += pEquip->m_wBossBonuRate;
    }
  }
  return (nTotal / 100);
}
///////////////////////////////////////////////////////////////////////////
//
//    计算自己五行属性、招式五行和对方五行属性的相克加成
//     还有招式属性对怪物的克制（如果敌人是怪物的情况下）
//
inline float CSkill::GetElementJinx(CLife* pAttacker,CLife* pTarget)
{
  float nTotal = ( pAttacker->GetElement() == g_iEleDown[(pTarget->GetElement()&0x1F)] ) ? g_MyJinx : 0;
  if( pTarget->IsMonster() )
  {
    CMonster *pTMonster = (CMonster*)pTarget;
    // 招式克怪物体形，相性
    if( m_wElement&0xFFE0 )
    {
      float FMJinx;
      if( 0 != ( FMJinx = ( ( m_wElement & pTMonster->m_wElement ) & 0xFFE0 ) ) )
      {
        FMJinx = GetBitCountFor_Element( FMJinx );
        nTotal += FMJinx * g_MyJinx;
      }
    }
    // 招式克怪物种族
    if( m_pBase->m_dwRaceAttri )
    {
      float RaceJinx;
      if( 0 != ( RaceJinx = m_pBase->m_dwRaceAttri & pTMonster->m_dwRaceAttri ) )
      {
        RaceJinx = m_pBase->GetBitCountForRace( RaceJinx );
        nTotal += RaceJinx * m_pBase->m_wRaceBonuRate;
      }
    }
    // 招式克魔王
    if( pTMonster->GetBaseId() == m_pBase->m_dwBossCode )
    {
      nTotal += m_pBase->m_wBossBonuRate;
    }
  }
  return (nTotal / 100);
}
//==========================================================================================
//desc:				计算 player 对目标的伤害（专用于幻道咒术）
//parameter:	CLife *pTarget(目标)
//return   :  返回伤害值
//==========================================================================================
inline short int CSkill::CalDamageForWizard(CLife *pTarget, int iAddiDamage)
{
  int damage   = 0;
  int       enemyEle = 0;
  float     S_DP_1   = 0, DpBouns_1 = 0, Amr_Int  = 0, Mdef_Limit = 0, Element = 0;
  float     F_M_Jinx = 0, MyJinx    = 0, RaceJinx = 0, BossJinx   = 0;
//#ifdef RESTRAIN_EVIL
  int       iTempDamage = 0;
  int       iTempDamage1 = 0;
//#endif
  //////////////////////////////////////////////////////////////////////////////////////
  //  公式：
  //     (必杀)  damage=(S_DP_1*DpBouns_1%)*(1-Amr_Int/Mdef_Limit) *
  //                (1+AddDamage%+FigureJinx%+MyJinx%+MoralJinx%+RaceJinx%+BossJinx%) * 
  //                Element%
  //////////////////////////////////////////////////////////////////////////////////////
  
  S_DP_1    = GetOwnAttributeForHuan();
  DpBouns_1 = GetAttrBonu();
  if( pTarget->IsMonster() )
    Amr_Int = ((CMonster*)pTarget)->GetChangeInt();
  else
    Amr_Int = ((CPlayer*)pTarget)->GetAllEquipAddInt();
  Mdef_Limit = g_iMdef_Limit;
  //
  // 计算各种Jinx量
  //
  //
  enemyEle = pTarget->GetElement() & 0x1F;
  MyJinx   = ( m_pPlayer->GetElement() == g_iEleDown[enemyEle] ) ? g_MyJinx : 0;
  if( pTarget->IsMonster() )
  {
    GetMonsterRelateJinx( (CMonster*)pTarget, F_M_Jinx, RaceJinx, BossJinx );
  }
  //
  // 计算Element量
  //
#ifdef _DEBUG
  int   iValueJinx = g_EleTable[(m_wElement&0x1F)][enemyEle];
#endif
  switch( g_EleTable[(m_wElement&0x1F)][enemyEle] )
  {
  case ET_NULL: Element = g_ElementNull; break;
  case ET_GROW: Element = g_ElementGrow; break;
  case ET_JINX: Element = g_ElementJinx; break;
  case ET_JXED: Element = g_ElementJxed; break;
  case ET_SAME: Element = g_ElementSame; break;
  }
  //
  // 代入公式
  //
  damage = S_DP_1*DpBouns_1*(1.0f-Amr_Int/Mdef_Limit) * 
           ( 1+m_pPlayer->m_pNowWizardData->m_iFinalInt + HQM_DIV100(F_M_Jinx)+HQM_DIV100(MyJinx)+
           HQM_DIV100(RaceJinx)+HQM_DIV100(BossJinx) );
//#ifdef RESTRAIN_EVIL
  iTempDamage = damage;        
//#endif
  //
  if( Element != 0 )    damage *= HQM_DIV100(Element);
  iTempDamage1 = damage;
  //
#if 0
#ifdef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_ 
#else
  FuncName("CSkill::CalDamageForWizard");
  _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "==>>%s Curse Damage=%d, Values(%04f,%04f,%04f,%04f,%04f,%04f,%04f,%04f,%04f,%04f)",
           m_pPlayer->GetAccount(), damage, S_DP_1, DpBouns_1, Amr_Int, Mdef_Limit,
           m_pPlayer->m_pNowWizardData->m_iFinalInt, F_M_Jinx, MyJinx, RaceJinx, BossJinx, Element );
  g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szDamageLog );
#endif
#endif
  //
  damage+=iAddiDamage;
  {
#ifdef _NEW_TRADITIONS_WEDDING_
    int iRageDamage = damage * m_pPlayer->GetAddApRatio();
#else
    int iRageDamage =0;
#endif    
#ifdef _NEW_TALISMAN_
    iRageDamage += damage * m_pPlayer->GetAddRageAP() / 100;
    iRageDamage += damage * pTarget->GetAddRageHarm() / 100;
    damage += iRageDamage;
#endif
  }
#ifdef RESTRAIN_EVIL
   damage += iTempDamage*RestrainForevil(pTarget); 
#endif
#ifdef RESTRAIN_BODY
   damage += iTempDamage1*RestrainForbody(pTarget);
#endif
	if( damage < 1 )      damage  = 1;
	else
  {
    if(damage>32767)
    {
      damage = 30000+gf_GetRandom(2700);
    }
    if( pTarget->IsMonster() && 
      (((CMonster*)pTarget)->GetType() & NPC_ATTRI_PRACTICE) &&
      m_pPlayer->GetLevel() <= (((CMonster*)pTarget)->GetLevel()+g_LevelLimit))
    {
      AddExp( 1 );
    }
    else if( pTarget->IsMonster() &&
      damage > ((CMonster*)pTarget)->GetMaxHpPercent(g_MaxHpPercent) &&
      m_pPlayer->GetLevel() <= (((CMonster*)pTarget)->GetLevel()+g_LevelLimit) )
    {
      AddExp( 1 );
    }
  }
  //
  // 附加有吸血效果
  //
  if( m_pPlayer->m_pNowWizardData->m_bDrain )
  {
    m_pPlayer->AddHp( damage );
  }
	return damage;
}
///////////////////////////////////////////////////////////////////////////
//
//     包含 FigureJinx,MoralJinx,RaceJinx,BossJinx
//     为计算方便，FigureJinx和MoralJinx合成为一个变量F_M_Jinx
//
inline void CSkill::GetMonsterRelateJinx(CMonster *pTMonster,float& F_M_Jinx,float& RaceJinx,float& BossJinx)
{
#ifdef RESTRAIN_BODY
    CSrvBaseItem  *pTesseraTemp = NULL;
    WORD           wElement;
    WORD           wElementTemp = 0;
    CItem         *pEquip       = NULL;
  if( pTMonster->IsMonster() ) 
  {
    pEquip = m_pPlayer->GetEquipment( ITEM_EQUIPWHERE_HAND );
    if(pEquip != NULL)
    {
      for( int iLoop = 0; iLoop<MAX_ITEM_SLOT; iLoop++)
      {
        pTesseraTemp = g_pBase->GetBaseItem(pEquip->m_wTesseraItemID[iLoop]);     
        if( pTesseraTemp!=NULL ) 
        {
          wElement = pTesseraTemp->GetElement();
          if ( wElement == 32 || wElement == 64 || wElement == 128 || wElement == 256 ) 
          {
            wElementTemp += wElement;
          }
        }
      }
    }
  }
#endif //RESTRAIN_BODY   
  // 招式克怪物体形，相性 ( 五行在最外面克制 )
  if( (m_wElement&0xFFE0) )
  {
    if( 0 != ( F_M_Jinx = ( (m_wElement&0xFFE0) & pTMonster->m_wElement ) ) )
    {
      F_M_Jinx = GetBitCountFor_Element( F_M_Jinx );
      F_M_Jinx = F_M_Jinx * g_MyJinx;
    }
  }
  // 招式克怪物种族
  if( m_pBase->m_dwRaceAttri )
  {
    if( 0 != ( RaceJinx = m_pBase->m_dwRaceAttri & pTMonster->m_dwRaceAttri ) )
    {
      RaceJinx = m_pBase->GetBitCountForRace( RaceJinx );
      RaceJinx = RaceJinx * m_pBase->m_wRaceBonuRate;
    }
  }
  // 招式克魔王
  if( pTMonster->GetBaseId() == m_pBase->m_dwBossCode )
  {
    BossJinx = m_pBase->m_wBossBonuRate;
  }
  CItem *pWeapon = m_pPlayer->GetEquipment(ITEM_EQUIPWHERE_HAND);
  if( pWeapon != NULL )
  {
    // 武器克怪物体形，相性
    if( (pWeapon->m_wElement
#ifdef RESTRAIN_BODY
      - wElementTemp
#endif //RESTRAIN_BODY
      )&0xFFE0 )
    {
      float   weaponFMJinx;
      if( 0 != ( weaponFMJinx = ( (pWeapon->m_wElement
#ifdef RESTRAIN_BODY
       - wElementTemp
#endif //RESTRAIN_BODY       
        ) & pTMonster->m_wElement & 0xFFE0 ) ) )
      {
        weaponFMJinx = GetBitCountFor_Element( weaponFMJinx );
        F_M_Jinx += weaponFMJinx * g_MyJinx;
      }
    }
    // 武器克怪物种族
    if( pWeapon->m_dwRaceAttri )
    {
      float weaponRaceJinx;
      if( 0 != ( weaponRaceJinx = pWeapon->m_dwRaceAttri & pTMonster->m_dwRaceAttri ) )
      {
        weaponRaceJinx = GetBitCountFor_Race( weaponRaceJinx );
        RaceJinx += weaponRaceJinx * pWeapon->m_wRaceBonuRate;
      }
    }
    // 武器克魔王
    if( pTMonster->GetBaseId() == pWeapon->m_dwBossCode )
    {
      BossJinx += pWeapon->m_wBossBonuRate;
    }
  }
}

//==========================================================================================
//desc:				计算 player 对目标的伤害（专用于一般职业）
//parameter:	CLife *pTarget(目标),BOOL bCritical(是否是必殺)
//return   :  返回伤害值
//==========================================================================================
//#define _DEBUG_HQM_PRINT_NEW_DAMAGE_
inline short int CSkill::CalDamageForPlayer(CLife *pTarget, BOOL bCritical, int iAddiDamage)
{
  static float    g_SKILLJINX = g_SkillJinx / 100;
  int       damage;
  int             P_AP;
  float           NoWeaponAp, maxWithWeaponAP;
#ifdef RESTRAIN_EVIL
  int             iTempDamage = 0;
  float           iTemp = 0;
#endif
  //////////////////////////////////////////////////////////////////////////////////////
  //  公式：
  //               NoWeaponAp=P_AP*0.4+INT(P_AP/25)^2 + INT(P_Hit/5)
  //               maxWithWeaponAP=NoWeaponAp+P_Hit
  //               minWithWeaponAp=NoWeaponAp+Weapon_AP
  //                      {minWithWeaponAp<=maxWithWeaponAP}
  //               WithWeaponAp=randomize(minWithWeaponAp,maxWithWeaponAp)
  //               AbsorbedRecovery=T_Equip_DP(except necklace)+T_Equip_Element_Jinx
  //               TargetBasicRecovery=T_DP+T_necklace_DP
  //               SkillAp=S_DP*DpBouns%*(1+SkillJinx%+ChainJinx%)(1-S_Random%)
  //               WithWeaponDamage=[(WithWeaponAp+SkillAp)*(1-AbsorbedRecovery/100)-
  //                                TargetBasicRecovery] *
  //                                (1+属性相克加成+武器镶嵌宝石效果)
  //                      { 1-AbsorbedRecovery/100 >= 0.01 }
  //                      { (WithWeaponAp+SkillAp)*(1-AbsorbedRecovery/100)-
  //                                  TargetBasicRecovery] >= 1 }
  //
  //     (必杀)  damage=maxWeaponAp
  //     (!必杀) damage=WithWeaponDamage
  //////////////////////////////////////////////////////////////////////////////////////
  P_AP = m_pPlayer->GetTotalAp()-m_pPlayer->GetAllEquipAddAp();
  NoWeaponAp      = P_AP*0.4+(P_AP/25)*(P_AP/25)+
                    (m_pPlayer->GetTotalHit()-m_pPlayer->GetAllEquipAddHit())/5;
  maxWithWeaponAP = NoWeaponAp+m_pPlayer->GetAllEquipAddAp();
  //
  float     S_DP, DpBouns, SkillJinx, ChainJinx, SkillAp;
  S_DP      = GetOwnAttribute();
  DpBouns   = GetAttrBonu();
  ChainJinx = GetChainJinx();
  SkillJinx = ((m_wElement&0x1F) == g_iEleDown[(pTarget->GetElement()&0x1F)]) ? ((float)g_SkillJinx)/100 : 0;
  if( bCritical )
  {
    SkillAp = S_DP * DpBouns * ( 1 + SkillJinx + ChainJinx );
    damage  = maxWithWeaponAP + SkillAp;
#ifdef RESTRAIN_EVIL
    iTempDamage = damage;
#endif
    damage  = damage * ( 1 + GetElementJinx( m_pPlayer, pTarget ) +
                             GetWeaponTesseraJinx( m_pPlayer, pTarget ) );
  }
  else
  {
    float   WithWeaponAp, minWithWeaponAp;
    float   AbsorbedRecovery, TargetBasicRecovery, S_Random;

    S_Random  = m_pBase->GetRandom();
    SkillAp   = S_DP * DpBouns * ( 1 + SkillJinx + ChainJinx ) * ( 1 + S_Random );

    minWithWeaponAp = NoWeaponAp+m_pPlayer->GetTotalHit()-m_pPlayer->GetAllEquipAddHit();
    if( minWithWeaponAp > maxWithWeaponAP ) minWithWeaponAp = maxWithWeaponAP;
    WithWeaponAp    = minWithWeaponAp + gf_GetRandom((int)(maxWithWeaponAP-minWithWeaponAp));
    if( pTarget->IsPlayer() )
    {
      AbsorbedRecovery = ((CPlayer*)pTarget)->GetAllEquipAddDp() -
                         ((CPlayer*)pTarget)->GetNecklaceAddDp() +
                         GetShieldTesseraJinx((CPlayer*)pTarget,m_pPlayer);
      //
      TargetBasicRecovery = pTarget->GetDp() - ((CPlayer*)pTarget)->GetAllEquipAddDp() +
                            ((CPlayer*)pTarget)->GetNecklaceAddDp();
    }
    else
    {
      AbsorbedRecovery    = ((CMonster*)pTarget)->GetChangeDp();
      TargetBasicRecovery = pTarget->GetDp();
    }
    float situation = (1-AbsorbedRecovery/100);
    if( situation < 0.01f ) situation = 0.01f;
    //
    situation = ( WithWeaponAp + SkillAp ) * situation - TargetBasicRecovery;
    //
    if( situation < 1.0f ) situation = 1.0f;
    //   
#ifdef RESTRAIN_EVIL
    iTempDamage = situation; 
#endif
    damage = situation * ( 1 + GetElementJinx( m_pPlayer, pTarget ) +
                               GetWeaponTesseraJinx( m_pPlayer, pTarget ) );

#ifdef _DEBUG
 //   if( strcmp( m_pPlayer->GetAccount(), "TEST2" ) == 0 && !bCritical )
    {
	    _snprintf( g_szDamageLog, MAX_MEMO_MSG_LEN-1, "NoWeaponAp=%.0f,minWeaponAp=%.0f,maxWeaponAp=%.0f,\
               FinalWeaponAp=%.0f,ChainJinx=%d,SkillJinx=%d,skillAp=%.0f,AbsorbedDp=%.0f,BasicDp=%.0f,damage=%d",
               NoWeaponAp,minWithWeaponAp,maxWithWeaponAP,WithWeaponAp,ChainJinx,SkillJinx,SkillAp,AbsorbedRecovery,
               TargetBasicRecovery,damage);
      g_szDamageLog[MAX_MEMO_MSG_LEN-1] = '\0';
	    g_DamageLog.Write(g_szDamageLog);
    }
#endif
  }
  damage+=iAddiDamage;
  {
#ifdef _NEW_TRADITIONS_WEDDING_
    int iRageDamage = damage * m_pPlayer->GetAddApRatio();
#else
    int iRageDamage = 0;
#endif
#ifdef _NEW_TALISMAN_
    iRageDamage += damage * m_pPlayer->GetAddRageAP() / 100;
    iRageDamage += damage * pTarget->GetAddRageHarm() / 100;
    damage += iRageDamage;
#endif
  }
#ifdef RESTRAIN_EVIL
  iTemp = RestrainForevil( pTarget );
  if( iTemp != 0 )
  {
    damage += iTempDamage*iTemp;
  }
#endif
#ifdef RESTRAIN_BODY
   damage += iTempDamage*RestrainForbody(pTarget);
#endif
  if( damage < 1 )
  {
    damage = 1;
  }
	else
  {
    // add skill exp
    if(damage>32767)
    {
      damage = 30000+gf_GetRandom(2700);
    }
    if( pTarget->IsMonster() && 
      (((CMonster*)pTarget)->GetType() & NPC_ATTRI_PRACTICE) &&
      m_pPlayer->GetLevel() <= (((CMonster*)pTarget)->GetLevel()+g_LevelLimit))
    {
      AddExp( 1 );
    }
    else if( pTarget->IsMonster() &&
      damage > ((CMonster*)pTarget)->GetMaxHpPercent(g_MaxHpPercent) &&
      m_pPlayer->GetLevel() <= ((CMonster*)pTarget)->GetLevel()+g_LevelLimit)
    {
      AddExp( 1 );
    }
  }
  //
	return damage;
}

//#define _DEBUG_MONSTER_PRINT_NEW_DAMAGE_
//==========================================================================================
//desc:				计算对monster 对 player的 伤害
//parameter:	pTarget(目标),BOOL bCritical(是否是必殺)
//return   :  返回伤害值
//==========================================================================================
inline short int CSkill::CalDamageForMonster(CPlayer *pTarget ,BOOL bCritical)
{
  int   damage;
  int         P_AP;
  float       NoWeaponAp, maxWithWeaponAP;
  //////////////////////////////////////////////////////////////////////////////////////
  //  公式：
  //               NoWeaponAp=P_AP*0.4+INT(P_AP/25)^2 + INT(P_Hit/5)
  //               maxWithWeaponAP=NoWeaponAp+P_Hit
  //               minWithWeaponAp=NoWeaponAp+Weapon_AP
  //                      {minWithWeaponAp<=maxWithWeaponAP}
  //               WithWeaponAp=randomize(minWithWeaponAp,maxWithWeaponAp)
  //               AbsorbedRecovery=T_Equip_DP(except necklace)+T_Equip_Element_Jinx
  //               TargetBasicRecovery=T_DP+T_necklace_DP
  //               SkillAp=S_DP*DpBouns%*(1+SkillJinx%+ChainJinx%)(1-S_Random%)
  //               WithWeaponDamage=[(WithWeaponAp+SkillAp)*(1-AbsorbedRecovery/100)-
  //                                  TargetBasicRecovery] *
  //                                (1+属性相克加成+武器镶嵌宝石效果)
  //                      { 1-AbsorbedRecovery/100 >= 0.01 }
  //                      { (WithWeaponAp+SkillAp)*(1-AbsorbedRecovery/100)-
  //                                  TargetBasicRecovery] >= 1 }
  //
  //     (必杀)  damage=maxWeaponAp
  //     (!必杀) damage=WithWeaponDamage
  //////////////////////////////////////////////////////////////////////////////////////
  P_AP = m_pMonster->GetAp();
  NoWeaponAp      = P_AP*0.4+(P_AP/25)*(P_AP/25)+m_pMonster->GetHit()/5;
  maxWithWeaponAP = NoWeaponAp+m_pMonster->GetChangeAp();
  float S_DP,DpBouns,SkillJinx,SkillAp;
  S_DP      = GetOwnAttribute();
  DpBouns   = GetAttrBonu();
  SkillJinx = (m_pBase->GetElement()==g_iEleDown[pTarget->GetElement()&0x1F]) ? g_SkillJinx : 0;
  if( bCritical )
  {
    SkillAp = S_DP*DpBouns*(1+SkillJinx);
    damage = maxWithWeaponAP + SkillAp;
  }
  else
  {
    float WithWeaponAp,minWithWeaponAp;
    float AbsorbedRecovery,TargetBasicRecovery,S_Random;
    S_Random  = m_pBase->GetRandom();
    SkillAp = S_DP*DpBouns*(1+SkillJinx)*(1-S_Random);

    minWithWeaponAp  = NoWeaponAp+m_pMonster->GetHit();
    //
    if( minWithWeaponAp > maxWithWeaponAP ) minWithWeaponAp = maxWithWeaponAP;
    //
    WithWeaponAp        = minWithWeaponAp + gf_GetRandom((int)(maxWithWeaponAP-minWithWeaponAp));    
    //
    AbsorbedRecovery    = pTarget->GetAllEquipAddDp() -
                          pTarget->GetNecklaceAddDp() +
                          GetShieldTesseraJinx(pTarget,m_pMonster);
    //
    TargetBasicRecovery = pTarget->GetDp() -
                          pTarget->GetAllEquipAddDp() +
                          pTarget->GetNecklaceAddDp();
    //
    float situation = (1-AbsorbedRecovery/100);
    if( situation < 0.01f ) situation = 0.01f;
    situation = (WithWeaponAp+SkillAp) * situation - TargetBasicRecovery;
    if( situation < 1.0f ) situation = 1.0f;
    damage = situation * (1 + GetElementJinx(m_pMonster,pTarget));
#ifdef _NEW_TALISMAN_
    {
      damage += damage * pTarget->GetAddRageHarm() / 100;
    }
#endif
  }

  if( damage < 1 )
  {
    damage = 1;
  }
	else
  {
    if(damage>32767)
    {
      damage = 30000+gf_GetRandom(2700);
    }
    // add skill exp
    AddExp( 1 );
  }

	return damage;
}

//*********************************************************************************
// private:
//					攻击方的武器与敌人的防具的损耗计算
//*********************************************************************************
inline bool CSkill::CalItemWaste( CLife *pTarget )
{
  //在新的耐久度计算中item的hard栏位表示item掉耐久度几率
#ifdef NEW_ENDURE
  BYTE      pos = ITEM_EQUIPWHERE_BODY;
  CItem*    pAttackItem = NULL;
  CItem*    pBlockItem = NULL;
  int       iRandomNum;
  int       iBlockHard, iAttackHard;
  
  iRandomNum = gf_GetRandom(100);
  if( iRandomNum > 49 )				  pos = ITEM_EQUIPWHERE_BODY;		  // 50%的几率是身体防具
  else if( iRandomNum > 29 )		pos = ITEM_EQUIPWHERE_HEAD;		  // 20%的几率是头防具
  else if( iRandomNum > 9 )		  pos = ITEM_EQUIPWHERE_FOOT;		  // 20%的几率是足防具
  else if( iRandomNum > 6 )		  pos = ITEM_EQUIPWHERE_ORNAMENT3;// 03%的几率是项链防具
  else if( iRandomNum > 3 )		  pos = ITEM_EQUIPWHERE_ORNAMENT1;// 03%的几率是修饰1防具
  else											    pos = ITEM_EQUIPWHERE_ORNAMENT2;// 04%的几率是修饰2防具
  
  if(pTarget->IsPlayer())//target是player的话，计算attack和block的耐久度
  {
    pBlockItem =  ((CPlayer*)pTarget)->m_pEquip[pos];
    if(!m_pPlayer) 
      return false;
    pAttackItem = m_pPlayer->m_pEquip[ITEM_EQUIPWHERE_HAND];
    if(m_pBase->IsCommonSkill())
    {
      if(!pAttackItem) 
        return false;
      else
      {
        if(pAttackItem->m_wDurability == 0)
        {
          m_pPlayer->ReleaseOneEquipAddData( ITEM_EQUIPWHERE_HAND );
          return false;
        }
        iAttackHard = pAttackItem->GetHard();
        if(iAttackHard >= gf_GetRandom(100))
        {
          pAttackItem->m_wDurability--;
          if(pAttackItem->m_wDurability < 0)
          {
            pAttackItem->m_wDurability = 0;
          }
          m_pPlayer->Send_A_SetDurability( ITEM_EQUIPWHERE_HAND, pAttackItem->m_wDurability );
          if( pAttackItem->m_wDurability == 0 )
          {
            m_pPlayer->ReleaseOneEquipAddData( ITEM_EQUIPWHERE_HAND );
          }
        }
      }
      
      if(pBlockItem)
      {
        iBlockHard = pBlockItem->GetHard();
        if(iBlockHard >= gf_GetRandom(100))
        {
          if(pBlockItem->m_wDurability == 0)
          {
            ((CPlayer*)pTarget)->ReleaseOneEquipAddData(pos);
            return false;
          }
          pBlockItem->m_wDurability--;
          if(pBlockItem->m_wDurability < 0)
          {
            pBlockItem->m_wDurability = 0;
          }
          ((CPlayer*)pTarget)->Send_A_SetDurability(pos, pBlockItem->m_wDurability);
          if(pBlockItem->m_wDurability == 0)
          {
            ((CPlayer*)pTarget)->ReleaseOneEquipAddData(pos);
          }
        }
      }
    }
  }
  else// target is monster
  {
    pAttackItem = m_pPlayer->m_pEquip[ITEM_EQUIPWHERE_HAND];
    if(m_pBase->IsCommonSkill())
    {
      if(!pAttackItem) 
        return false;
      else
      {
        if(pAttackItem->m_wDurability == 0)
        {
          m_pPlayer->ReleaseOneEquipAddData( ITEM_EQUIPWHERE_HAND );
          return false;
        }
        iAttackHard = pAttackItem->GetHard();
        if(iAttackHard >= gf_GetRandom(100))
        {
          pAttackItem->m_wDurability--;
          if(pAttackItem->m_wDurability < 0)
          {
            pAttackItem->m_wDurability = 0;
          }
          m_pPlayer->Send_A_SetDurability( ITEM_EQUIPWHERE_HAND, pAttackItem->m_wDurability );
          if( pAttackItem->m_wDurability == 0 )
          {
            m_pPlayer->ReleaseOneEquipAddData( ITEM_EQUIPWHERE_HAND );
          }
        }
      }
    }
  }
  
  return true;
#else
	BYTE			pos = ITEM_EQUIPWHERE_BODY;
	CItem			*pItemAttack, *pItemBlock;
	int				iRndNum, iSub;
	int				iBlockHard, iAttackHard;

	iRndNum = gf_GetRandom(100);
	if( iRndNum > 49 )				pos = ITEM_EQUIPWHERE_BODY;		  // 50%的几率是身体防具
	else if( iRndNum > 29 )		pos = ITEM_EQUIPWHERE_HEAD;		  // 20%的几率是头防具
	else if( iRndNum > 9 )		pos = ITEM_EQUIPWHERE_FOOT;		  // 20%的几率是足防具
	else if( iRndNum > 6 )		pos = ITEM_EQUIPWHERE_ORNAMENT3;// 03%的几率是项链防具
	else if( iRndNum > 3 )		pos = ITEM_EQUIPWHERE_ORNAMENT1;// 03%的几率是修饰1防具
	else											pos = ITEM_EQUIPWHERE_ORNAMENT2;// 04%的几率是修饰2防具

	pItemBlock  = NULL;
	pItemAttack = NULL;
	/////////////////////////////////////////////////////////////
	////////////  获取防御的hard  ///////////////////////////////
	/////////////////////////////////////////////////////////////
	if( pTarget->IsPlayer() )
	{
		pItemBlock = ((CPlayer*)pTarget)->m_pEquip[pos]; // 获取pos位置处的防具
		//防御道具不存在
		if( !pItemBlock )
		{
			return false;
		}
		//防御道具无损耗
		if( !pItemBlock->GetBaseItem()->CanBad() )
		{
			return false;
		}
		//防御道具已经损坏
		if( pItemBlock->m_wDurability == 0 )
		{
			return false;
		}
		iBlockHard = pItemBlock->GetHard();
	}
	else
	{
		iBlockHard = ((CMonster*)pTarget)->GetHard();
	}
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//////////  获取攻击的hard   ////////////////////////////////
	/////////////////////////////////////////////////////////////
	if( IsPlayer() )
	{
		pItemAttack = m_pPlayer->m_pEquip[ITEM_EQUIPWHERE_HAND];//获取攻击方的武器
		if( !pItemAttack )
		{
			return false;
		}
		//攻击方的武器已经损坏
		if( pItemAttack->m_wDurability == 0 )
		{
			return false;
		}
		iAttackHard = pItemAttack->GetHard();
	}
	else
	{
		iAttackHard = m_pMonster->GetHard();
	}
	/////////////////////////////////////////////////////////////
	//////////  开始计算道具损伤  ////////////////////////////////
	/////////////////////////////////////////////////////////////

	//  普通攻击和幻道攻击
	if( m_pBase->IsCommonSkill() )
	{
		iSub = iAttackHard - iBlockHard;
		if( iSub > 0 )
		{	//防御方武器 损坏
			if( pItemBlock == NULL )
			{
        return false;
      }
      if( iSub > 100 )
      {
        pItemBlock->m_wDurability = 0;
      }
      else
      {
        ///////////////////////////////
        // 判断损伤几率
        if( iSub < gf_GetRandom(100) )
          iSub=0;
        if( iSub > pItemBlock->m_wDurability )	pItemBlock->m_wDurability = 0;
        else																		pItemBlock->m_wDurability -= iSub;
			}
			if( pTarget->IsPlayer() && iSub > 0 )
			{
				//发送消息到防御方
				((CPlayer*)pTarget)->Send_A_SetDurability( pos, pItemBlock->m_wDurability );
				if( pItemBlock->m_wDurability == 0 )
				{
					((CPlayer*)pTarget)->ReleaseOneEquipAddData( pos );
          //CalSuitResult( m_pEquip[pos]->GetSuitId() );
				}
			}
		}
		else if( iSub < 0 )
		{	//攻击方武器损坏
			if( pItemAttack == NULL )
			{
				return false;
			}
      iSub = abs(iSub);
      if( iSub > 100 )
      {
        pItemAttack->m_wDurability = 0;
      }
      else
      {
        ///////////////////////////////
        // 判断损伤几率
        if( iSub < gf_GetRandom(100) )
          iSub=0;
        if( iSub > pItemAttack->m_wDurability )	pItemAttack->m_wDurability = 0;
        else																		pItemAttack->m_wDurability -= iSub;
      }
			//发送消息到攻击方
			if( IsPlayer() && iSub > 0 )
			{
				m_pPlayer->Send_A_SetDurability( ITEM_EQUIPWHERE_HAND, pItemAttack->m_wDurability );
				if( pItemAttack->m_wDurability == 0 )
				{
					m_pPlayer->ReleaseOneEquipAddData( ITEM_EQUIPWHERE_HAND );
          //CalSuitResult( m_pEquip[ITEM_EQUIPWHERE_HAND]->GetSuitId() );
				}
			}
		}
	}
	//////////其他的法术攻击////////////////////////
	else
	{
#ifdef _FOR_TOM_
		iSub = m_pBase->m_iHard - iBlockHard + m_iHardAdd;//m_iHardAdd为法术追加硬度
		if( iSub > 0)
		{
			if( pItemBlock == NULL )
			{
#ifdef _DEBUG_LOG_SKILL_
			  g_DamageLog.Write( "-------- Block Item Is NULL, So Cancel #" );
#endif
        return false;
      }
      if( iSub > 100 )	pItemBlock->m_wDurability = 0;
      else
      {
        iSub = iSub * pItemBlock->m_wDurability / 100;
        if( iSub == 0 )	iSub = 1;
        if( iSub > pItemBlock->m_wDurability )	pItemBlock->m_wDurability = 0;
        else																		pItemBlock->m_wDurability -= iSub;
      }
			if( pTarget->IsPlayer() )
			{
#ifdef _DEBUG_LOG_SKILL_
				sprintf(g_szDamageLog,"-------- %s: (For Blocker) Damage Equip Pos=%d, Equip Durb=%d, Sub=%d #%d# --------",((CPlayer*)pTarget)->m_szAccount,pos,pItemBlock->m_wDurability,iSub,ClientTickCount);
				g_DamageLog.Write(g_szDamageLog);
#endif
				//发送消息到目标
				((CPlayer*)pTarget)->Send_A_SetDurability( pos, pItemBlock->m_wDurability );
				if( pItemBlock->m_wDurability == 0 )
				{
					((CPlayer*)pTarget)->ReleaseOneEquipAddData( pos );
          //CalSuitResult( m_pEquip[pos]->GetSuitId() );
				}
			}
		}
#endif
	}

	return  true;
#endif
}
//==========================================================================================
//desc:				查找monster攻击范围内的all player,保存到list中
//parameter:	
//return   :  bool
//==========================================================================================
bool CSkill::FindTarget()
{
	static LifeTileList::iterator	          pIte;
  static map<DWORD, CPlayer*>::iterator   gIter_Ply;
  //
	int							        g_FindC = 0;
	CLife										*pFindLife = NULL, *pFirstLife = NULL;
 	CGameMap								*pTheMap = NULL;
	LifeTileList						*pSimPlayer;
	POINT										m_ptUser;
  CPlayer                 *pPlayer = NULL; 

	m_listTargets.clear();
   // Get the monster attack target
	pTheMap = m_pMonster->GetInMap();

	m_ptUser.x = ((CLife*)m_pMonster)->GetTargetX();
	m_ptUser.y = ((CLife*)m_pMonster)->GetTargetY();

  if( GetMonsterHitRange() <= 0 )
	  return FALSE;
  //
  if( m_pMonster->IsBoss() )
  {
    for( gIter_Ply = pTheMap->m_mapCodePlayer.begin(); gIter_Ply != pTheMap->m_mapCodePlayer.end(); gIter_Ply++ )
    {
      pPlayer  = gIter_Ply->second;
      
      //if( m_pMonster->GetDistance( pPlayer ) <= (GetBaseShapeRange() + (m_pMonster->m_pBase->GetSize() >> 1) ) )
      //{
      if( m_pMonster->GetDistance( pPlayer ) <= (GetBaseShapeRange() ) )
      {
        // pFindLife is GM ,and is GM_STATUS_ANCHORET status 
        if( pPlayer->GetGMStatus() & GM_STATUS_ANCHORET )
        {
          continue;
        }
        //Add by CECE 2004-09-20
        if( pPlayer->GetStatus() == STATUS_PLAYER_DEAD )
        {
          continue;
        }
        // Input This Target
        m_listTargets.push_back( pPlayer );          
        if( ++g_FindC > m_pBase->GetLoopCount() )    break;
      }
    }
  }
  else
  {
    for( g_FindC = 0; g_FindC < g_iSize; g_FindC++ )
    {
      pSimPlayer = pTheMap->GetTileLifeList( g_ptAllTargets[g_FindC].x, g_ptAllTargets[g_FindC].y );
      pFindLife  = NULL;
      pFirstLife = NULL;
      if( ( pSimPlayer != NULL )&&( !pSimPlayer->empty() ) )
      {
        for( pIte = pSimPlayer->begin(); pIte != pSimPlayer->end(); ++pIte )
        {
          if( (*pIte)->GetCode() >= CODE_MIN_PLAYER && (*pIte)->GetCode() <= CODE_MAX_PLAYER )
          {
            pFindLife = (CLife*)(*pIte);
            
            if( pFirstLife == NULL && ((CPlayer*)pFindLife)->GetPkCount() )    pFirstLife = pFindLife;

            // pFindLife is GM ,and is GM_STATUS_ANCHORET status 
            if( ((CPlayer*)pFindLife)->GetGMStatus() & GM_STATUS_ANCHORET )
            {
              pFindLife = NULL;
              continue;
            }
            if( pFindLife->GetInMap() != pTheMap )
            {
              pTheMap->DelLifeFromTile( pFindLife );
              pFindLife = NULL;
              break;
            }
          }
        }
      }
      if( pFirstLife )      m_listTargets.push_back( pFirstLife );
      else if( pFindLife )  m_listTargets.push_back( pFindLife );
	  }
  }

	return (!m_listTargets.empty());
}
//============================================================================================
//
//	class CMagic	Implement -- 陷阱
//
//============================================================================================
// Constructor
CMagic::CMagic()
{
	m_pBase         = NULL;
	m_pPlayer       = NULL;
  m_pMonster      = NULL;
  m_bCanPK        = FALSE;
  m_iX = m_iEndX  = 0;
  m_iY = m_iEndY  = 0;
  m_iDir          = 0;
  m_pInMap        = NULL;
	m_dwLifeTime    = 0;
  m_dwActionTime  = 0;
  m_iStatus       = 0;
  m_wUpdateTurn   = 0;
  m_pFirstTarget  = NULL;
  m_wCode         = 0;
}
//============================================================================================
//
// Destructor
//==========================================================================================
CMagic::~CMagic()
{
	m_pBase->ClearAllTarget();
  // 清除地图标记
}
//==========================================================================================
//	public:
//		设置地图标记
//==========================================================================================
inline void CMagic::SetMyMapSign()
{
  // 1 触 n
  //if( m_pBase->GetType() < SKILL_TYPE_TRAP_DURATIVE_NORMAL )
  //{
  //  m_pInMap->AddMagicCode( m_iX, m_iY, m_wCode );
  //}
  //// n 触 n
  //else
  {
    int     iCount = 0;

    switch( m_pBase->GetShape() )
    {
    case SKILL_SHAPE_TRAP_1X1:
      m_pInMap->AddMagicCode( m_iX, m_iY, m_wCode );
      break;
    case SKILL_SHAPE_TRAP_1X3:
      for( iCount = 0; iCount < 3; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap1X3[m_iDir][iCount].x, m_iY+g_ptTrap1X3[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_1X5:
      for( iCount = 0; iCount < 5; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap1X5[m_iDir][iCount].x, m_iY+g_ptTrap1X5[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_1X7:
      for( iCount = 0; iCount < 7; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap1X7[m_iDir][iCount].x, m_iY+g_ptTrap1X7[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_1X9:
      for( iCount = 0; iCount < 9; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap1X9[m_iDir][iCount].x, m_iY+g_ptTrap1X9[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_3X3:
      for( iCount = 0; iCount < 9; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap3X3[iCount].x, m_iY+g_ptTrap3X3[iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_3X5:
      for( iCount = 0; iCount < 15; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap3X5[m_iDir][iCount].x, m_iY+g_ptTrap3X5[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_3X7:
      for( iCount = 0; iCount < 21; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap3X7[m_iDir][iCount].x, m_iY+g_ptTrap3X7[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_3X9:
      for( iCount = 0; iCount < 27; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap3X9[m_iDir][iCount].x, m_iY+g_ptTrap3X9[m_iDir][iCount].y, m_wCode );
      }
      break;
    case SKILL_SHAPE_TRAP_5X5:
      for( iCount = 0; iCount < 25; iCount++ )
      {
        m_pInMap->AddMagicCode( m_iX+g_ptTrap5X5[iCount].x, m_iY+g_ptTrap5X5[iCount].y, m_wCode );
      }
      break;
    default:
      m_pInMap->AddMagicCode( m_iX, m_iY, m_wCode );
      break;
    }
  }
}
//==========================================================================================
//	public:
//    清除地图标记
//==========================================================================================
void CMagic::ReleaseMyMapSign()
{
  // 1 触 n
  //if( m_pBase->GetType() < SKILL_TYPE_TRAP_DURATIVE_NORMAL )
  //{
  //  m_pInMap->DelMagicCode( m_iX, m_iY );
  //}
  // n 触 n
  //else
  {
    int     iCount = 0;

    switch( m_pBase->GetShape() )
    {
    case SKILL_SHAPE_TRAP_1X1:
      m_pInMap->DelMagicCode( m_iX, m_iY );
      break;
    case SKILL_SHAPE_TRAP_1X3:
      for( iCount = 0; iCount < 3; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap1X3[m_iDir][iCount].x, m_iY+g_ptTrap1X3[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_1X5:
      for( iCount = 0; iCount < 5; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap1X5[m_iDir][iCount].x, m_iY+g_ptTrap1X5[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_1X7:
      for( iCount = 0; iCount < 7; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap1X7[m_iDir][iCount].x, m_iY+g_ptTrap1X7[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_1X9:
      for( iCount = 0; iCount < 9; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap1X9[m_iDir][iCount].x, m_iY+g_ptTrap1X9[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_3X3:
      for( iCount = 0; iCount < 9; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap3X3[iCount].x, m_iY+g_ptTrap3X3[iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_3X5:
      for( iCount = 0; iCount < 15; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap3X5[m_iDir][iCount].x, m_iY+g_ptTrap3X5[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_3X7:
      for( iCount = 0; iCount < 21; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap3X7[m_iDir][iCount].x, m_iY+g_ptTrap3X7[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_3X9:
      for( iCount = 0; iCount < 27; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap3X9[m_iDir][iCount].x, m_iY+g_ptTrap3X9[m_iDir][iCount].y );
      }
      break;
    case SKILL_SHAPE_TRAP_5X5:
      for( iCount = 0; iCount < 25; iCount++ )
      {
        m_pInMap->DelMagicCode( m_iX+g_ptTrap5X5[iCount].x, m_iY+g_ptTrap5X5[iCount].y );
      }
      break;
    default:
      m_pInMap->DelMagicCode( m_iX, m_iY );
      break;
    }
  }
}
//==========================================================================================
//	public:
//		自己在地图上移动
//==========================================================================================
void CMagic::Move()
{
  ReleaseMyMapSign();
  // Change My Pos By Move Direction
  // ...
  SetMyMapSign();
}
//==========================================================================================
//	public:
//		判断Life是否应该被陷阱攻击
//==========================================================================================
inline void CMagic::IsTrapTarget( CLife * pTarget )
{
	if( NULL == m_pInMap )			return;
	//
  if( pTarget->IsMonster() )
  {
    m_pBase->AddTarget( pTarget );
		return;
  }
	if( pTarget->IsPlayer() && m_pPlayer )
	{
		if( m_pInMap->CanNotPK() )		return;

    // 除攻城战外对玩家低等级玩家进行PK保护
//////////////////////////////////////////////////////////////////////
#ifdef ELYSIUM_3_7_VERSION
          if( m_pInMap->IsMatchField() )
          {
            if( m_pPlayer->GetFightInfo() == NULL || ((CPlayer*)pTarget)->GetFightInfo() == NULL )
            {
              return;
            }
          }
          else
          {
#endif
            if( m_pPlayer->GetLevel() < LEVEL_PROTECT && !m_pInMap->IsCityWarMap() )
            {
              return;
            }
            if( pTarget->GetLevel() < LEVEL_PROTECT && !m_pInMap->IsCityWarMap() )
            {
              return;
            }

//////////////////////////////////////////////////////////////////////
#ifdef ELYSIUM_3_7_VERSION
          }
#endif
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////


		// 已经死亡的玩家 -_-!
    if( ((CPlayer*)pTarget)->GetStatus() == STATUS_PLAYER_DEAD )    return;

		// 红名一定被攻击
		if( ((CPlayer*)pTarget)->GetPkCount() > 0 )
		{
			m_pBase->AddTarget( pTarget );
			return;
		}
    // 蓝名队友不能攻击
    if( m_pPlayer->IsTeamer( ((CPlayer*)pTarget) ) )		return;

		// 攻城战时，不是本盟的人都要被攻击 
		if( ( (m_pPlayer->IsDeclareWar() || m_pInMap->IsCityWarMap())  &&
					!m_pPlayer->IsLeaguePlayer( ((CPlayer*)pTarget) ) )				) 
		{
			m_pBase->AddTarget( pTarget );
			return;
		}

		// 敌对帮会的人，杀
		if( m_pPlayer->IsEnemyPlayer( ((CPlayer*)pTarget) ) )
		{
			m_pBase->AddTarget( pTarget );
			return;
		}

		// 开了PK放的陷阱
		if( m_bCanPK )
		{
			// 恶意PK要设置恶人状态
			if( (!(m_pInMap->NoDeadPunish())) &&(!m_pInMap->IsCityWarMap()) ) 
			{
				m_pPlayer->SetElfStatus();
			}
			// 添加成为攻击目标
			m_pBase->AddTarget( pTarget );
		}
		else
		{
			return;
		}
	}
	// 怪物设置的陷阱
	else
	{
		m_pBase->AddTarget( pTarget );
		return;
	}
}
//==========================================================================================
//	public:
//		做持续的法术工作
//==========================================================================================
inline void CMagic::CheckRange()
{
  int     iCount = 0;
  CLife   *pTarget = NULL, *pUser;

  if( m_pPlayer )   pUser = m_pPlayer;
  else              pUser = m_pMonster;
  m_pBase->ClearAllTarget();
  //
  m_pBase->AddTarget( m_pFirstTarget );
  //
  switch( m_pBase->GetShape() )
  {
  case SKILL_SHAPE_TRAP_1X1:
    m_pInMap->DelMagicCode( m_iX, m_iY );
    break;
  case SKILL_SHAPE_TRAP_1X3:
    for( iCount = 0; iCount < 3; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap1X3[m_iDir][iCount].x, m_iY+g_ptTrap1X3[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_1X5:
    for( iCount = 0; iCount < 5; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap1X5[m_iDir][iCount].x, m_iY+g_ptTrap1X5[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_1X7:
    for( iCount = 0; iCount < 7; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap1X7[m_iDir][iCount].x, m_iY+g_ptTrap1X7[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_1X9:
    for( iCount = 0; iCount < 9; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap1X9[m_iDir][iCount].x, m_iY+g_ptTrap1X9[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_3X3:
    for( iCount = 0; iCount < 9; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap3X3[iCount].x, m_iY+g_ptTrap3X3[iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_3X5:
    for( iCount = 0; iCount < 15; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap3X5[m_iDir][iCount].x, m_iY+g_ptTrap3X5[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_3X7:
    for( iCount = 0; iCount < 21; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap3X7[m_iDir][iCount].x, m_iY+g_ptTrap3X7[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_3X9:
    for( iCount = 0; iCount < 27; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap3X9[m_iDir][iCount].x, m_iY+g_ptTrap3X9[m_iDir][iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  case SKILL_SHAPE_TRAP_5X5:
    for( iCount = 0; iCount < 25; iCount++ )
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX+g_ptTrap5X5[iCount].x, m_iY+g_ptTrap5X5[iCount].y );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  default:
    {
      pTarget = m_pInMap->GetFirstLifeInTile( m_iX, m_iY );
      if( pTarget && pTarget != pUser )
      {
        IsTrapTarget( pTarget );
      }
    }
    break;
  }
}
//==========================================================================================
//	public:
//		陷阱和法术的攻击
//==========================================================================================
inline void CMagic::Attack()
{
  CheckRange();
  m_pBase->DoDamageForTrap( this );
}
//==========================================================================================
//	public:
//		做持续的法术工作
//==========================================================================================
void CMagic::DoAction()
{
  switch( m_iStatus ) 
  {
  case MAGICSTATE_NONE:
  case MAGICSTATE_WAIT:
    if( ClientTickCount >= m_dwLifeTime )
    {
      m_iStatus = MAGICSTATE_DEAD;
    }
    break;
  // 做完攻击马上消失
  case MAGICSTATE_ATTACK:
    // Find Target And Attack
    Attack();
    m_iStatus = MAGICSTATE_DEAD;
    break;
  // 持续性攻击
  case MAGICSTATE_DURATIVE_ATTACK:
    if( ClientTickCount >= m_dwActionTime )
    {
      // Find Target And Attack
      Attack();
    }
    if( ClientTickCount >= m_dwLifeTime )
    {
      m_iStatus = MAGICSTATE_DEAD;
    }
    break;
  default:
    if( ClientTickCount >= m_dwLifeTime )
    {
      m_iStatus = MAGICSTATE_DEAD;
    }
    break;
  }
}
//==========================================================================================
//	public:
//		获取x,y和这个magic的距离
//==========================================================================================
int	CMagic::GetDistance( int X, int Y )
{
	int iDistance = abs(X - m_iX) > abs(Y - m_iY) ? abs(X - m_iX) : abs(Y - m_iY);
  return iDistance;
}
//============================================================================================
//
//Functions:
//
//============================================================================================
BOOL CMagic::UpdateTurnCheck()
{
  DWORD dwNow = TimeGetTime();
	DWORD dwUpdateCheck = m_wUpdateTurn;

  if( m_wUpdateTurn <= 0 )
  {
    m_wUpdateTurn = dwNow + UPDATE_NEAR_TIME_MS;
  }
  else if( dwNow > m_wUpdateTurn )
  {
    m_wUpdateTurn = 0;
  }
	return ( dwUpdateCheck <= 0 );
}
//==========================================================================================
//desc:				Trap对目标作攻击
//parameter:	
//return   :  bool
//==========================================================================================
inline WORD CMagic::GetUserCode()
{
  if( m_pPlayer )       return m_pPlayer->GetSelfCode();
  else if( m_pMonster ) return m_pMonster->GetSelfCode();
  return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void CSkill::CheckLineD()
{
//#ifdef _DEBUG_CHECK_FUNC_STATE_
//  CCheckFuncState callStackCheck("CSkill::CheckLineD()");
//#endif

	int				g_LoopCountD = 0, g_CountD = 0;
	CGameMap	*g_TheMapD = NULL;
	WORD			g_SkillXD, g_SkillYD;
	WORD			iDir;

	g_LoopCountD = m_pBase->m_wLoopCount - m_pMonster->m_wTargetDis + m_pBase->m_bRange;
  if( g_LoopCountD > m_pBase->m_wLoopCount )
    g_LoopCountD = m_pBase->m_wLoopCount;
	g_TheMapD    = m_pMonster->GetInMap();
	if( m_pBase->m_bRange == 0 )
	{
		g_SkillXD    = m_pMonster->GetPosX();
		g_SkillYD    = m_pMonster->GetPosY();
	}
	else
	{
		g_SkillXD    = m_pMonster->GetTargetX();
		g_SkillYD    = m_pMonster->GetTargetY();
	}
	g_iSize      = 0;
	iDir				 = m_pMonster->GetMonsterDir();

	for( g_CountD = 0; g_CountD < g_LoopCountD; g_CountD++ )
	{
		g_ptTemp.x = g_SkillXD + g_ptOffsetLineD[iDir][g_CountD].x;
		g_ptTemp.y = g_SkillYD + g_ptOffsetLineD[iDir][g_CountD].y;
		g_dwBlock = g_TheMapD->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		//if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
		//	break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y;
		g_iSize++;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void CSkill::CheckLineW()
{
//#ifdef _DEBUG_CHECK_FUNC_STATE_
//  CCheckFuncState callStackCheck("CSkill::CheckLineW()");
//#endif

	int				g_LoopCountW = 0, g_CountW = 0;
	CGameMap	*g_TheMapW = NULL;
	WORD			g_SkillXW, g_SkillYW;
	WORD			iDir;

	g_LoopCountW = m_pBase->m_wLoopCount - m_pMonster->m_wTargetDis + m_pBase->m_bRange;
  if( g_LoopCountW > m_pBase->m_wLoopCount )
    g_LoopCountW = m_pBase->m_wLoopCount;

	if( m_pBase->m_bRange == 0 )
	{
		g_SkillXW    = m_pMonster->GetPosX();
		g_SkillYW    = m_pMonster->GetPosY();
	}
	else
	{
		g_SkillXW    = m_pMonster->GetTargetX();
		g_SkillYW    = m_pMonster->GetTargetY();
	}
	g_TheMapW    = m_pMonster->GetInMap();
	g_iSize      = 0;
	iDir				 = m_pMonster->GetMonsterDir();

	for( g_CountW = 0; g_CountW < g_LoopCountW; g_CountW++ )
	{
		g_ptTemp.x = g_SkillXW + g_ptOffsetLineW[iDir][g_CountW].x;
		g_ptTemp.y = g_SkillYW + g_ptOffsetLineW[iDir][g_CountW].y;
		g_dwBlock = g_TheMapW->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		//if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
		//	break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y;
		g_iSize++;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
inline void CSkill::CheckRound()
{
//#ifdef _DEBUG_CHECK_FUNC_STATE_
//  CCheckFuncState callStackCheck("CSkill::CheckRound()");
//#endif

	int				g_LoopCountR = 0, g_CountR = 0;
	CGameMap	*g_TheMapR = NULL;
	WORD			g_SkillXR, g_SkillYR;
	
	if( m_pBase->m_bRange == 0 )
	{
		g_SkillXR    = m_pMonster->GetPosX();
		g_SkillYR    = m_pMonster->GetPosY();
	}
	else
	{
		g_SkillXR    = m_pMonster->GetTargetX();
		g_SkillYR    = m_pMonster->GetTargetY();
	}
	g_TheMapR      = m_pMonster->GetInMap();
	g_LoopCountR   = m_pBase->m_wLoopCount;
	g_iSize        = 0;

	for( g_CountR = 0; g_CountR < g_LoopCountR; g_CountR++ )
	{
		g_ptTemp.x = g_SkillXR + g_ptOffsetXRound[g_CountR];
		g_ptTemp.y = g_SkillYR + g_ptOffsetYRound[g_CountR];
		g_dwBlock = g_TheMapR->GetTileFlag( g_ptTemp.x, g_ptTemp.y );
		//if( (g_dwBlock & TILE_GROUNDOCCULDE) || ( g_dwBlock == 0xFFFFFFFF) )
		//	break;
		g_ptAllTargets[g_iSize].x = g_ptTemp.x;
		g_ptAllTargets[g_iSize].y = g_ptTemp.y; 
		g_iSize++;
	}
}

//=======================================================================================
// desc:			获取这个base skill的攻击范围
// parameter:
// return:
//=======================================================================================
inline BYTE CSkill::GetMonsterHitRange()
{
	switch( m_pBase->m_bShape )
	{
		/////////////// LINE \\\\\\\\\\\\\\\\\\\\\/
		case SKILL_SHAPE_LINE_D1:
		case SKILL_SHAPE_LINE_D2:
		case SKILL_SHAPE_LINE_D3:
		case SKILL_SHAPE_LINE_D4:
		case SKILL_SHAPE_LINE_D5:
		case SKILL_SHAPE_LINE_D6:
		case SKILL_SHAPE_LINE_D7:
		case SKILL_SHAPE_LINE_D8:
			CheckLineD();
			break;
		///////////// ROUND \\\\\\\\\\\\\\\\\\\\\\\/
		case SKILL_SHAPE_ROUND_1X1:
		case SKILL_SHAPE_ROUND_3X3:
		case SKILL_SHAPE_ROUND_5X5:
		case SKILL_SHAPE_ROUND_7X7:
		case SKILL_SHAPE_ROUND_9X9:
    /////////////////////////////
    //Add by CECE 2004-04-22
    case SKILL_SHAPE_ROUND_11X11:
    case SKILL_SHAPE_ROUND_13X13:
    /////////////////////////////
			CheckRound();
			break;
    case SKILL_SHAPE_LINEW_3:
    case SKILL_SHAPE_LINEW_6:
      CheckLineW();
      break;
    case SKILL_SHAPE_TARGET:
      {
        g_iSize = 1;
        g_ptAllTargets[0].x = 0x0FFFFFFF;
        g_ptAllTargets[0].y = 0x0FFFFFFF;
        CPlayer* pTarget = NULL;
        pTarget = m_pMonster->GetLastTarget();
        if(!pTarget)
          break;
        m_listTargets.push_back(pTarget);
      }
      break;
		default:
      g_ptAllTargets[0].x = m_pMonster->GetTargetX();
      g_ptAllTargets[0].y = m_pMonster->GetTargetY();
			g_iSize = 1;
			break;
	}
	return g_iSize;
}
//=======================================================================================
// desc:			获取连续招式的加成
// parameter:
// return:
//=======================================================================================
inline float CSkill::GetChainJinx()
{
	if( m_wElement & g_iEleUp[m_pPlayer->GetPreSkillElement()] )
	{
		m_pPlayer->AddApBonus( 1 );
  }
	else
	{
		m_pPlayer->SetApBonus( 0 );
	}
	m_pPlayer->SetPreSkillElement( (m_wElement&0x1F) );
	return ( m_pPlayer->GetApBonus() * (float)g_ChainJinx / 100 );
}
//=======================================================================================
// desc:			获取获取此招式对人物(怪物)属性的影响
// parameter:
// return:
//=======================================================================================
inline int CSkill::GetOwnAttribute()
{
  if( m_pBase->m_wOwnAttrEffect == 0 )    return 1;
  int sum = 1;
  if( IsPlayer() )
  {
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_AP )
    {
      sum += m_pPlayer->GetTotalAp() - m_pPlayer->GetAllEquipAddAp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_HIT )
    {
      sum += m_pPlayer->GetTotalHit() - m_pPlayer->GetAllEquipAddHit();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DP )
    {
      sum += m_pPlayer->GetTotalDp() - m_pPlayer->GetAllEquipAddDp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DG )
    {
      sum += m_pPlayer->GetTotalDodge() - m_pPlayer->GetAllEquipAddDodge();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_INT )
    {
      sum += m_pPlayer->GetTotalInt() - m_pPlayer->GetAllEquipAddInt();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EAP )
    {
      sum += m_pPlayer->GetAllEquipAddAp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EHIT )
    {
      sum += m_pPlayer->GetAllEquipAddHit();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EDP )
    {
      sum += m_pPlayer->GetAllEquipAddDp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EDG )
    {
      sum += m_pPlayer->GetAllEquipAddDodge();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EINT )
    {
      sum += ( m_pPlayer->GetInt() * m_pPlayer->GetWeaponAddInt() / 100 );
    }
  }
  else
  {
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_AP )
    {
      sum += m_pMonster->GetTotalAp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_HIT )
    {
      sum += m_pMonster->GetTotalHit();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DP )
    {
      sum += m_pMonster->GetTotalDp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DG )
    {
      sum += m_pMonster->GetTotalDodge();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_INT )
    {
      sum += m_pMonster->GetTotalInt();
    }
  }
  return sum;
}
//=======================================================================================
// desc:			获取获取此招式对人物(怪物)属性的影响
// parameter:
// return:
//=======================================================================================
inline int CSkill::GetOwnAttributeForHuan()
{
  if( m_pBase->m_wOwnAttrEffect == 0 )    return 0;
  int         sum = 0;
  if( IsPlayer() )
  {
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_AP )
    {
      sum += m_pPlayer->GetTotalAp() - m_pPlayer->GetAllEquipAddAp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_HIT )
    {
      sum += m_pPlayer->GetTotalHit() - m_pPlayer->GetAllEquipAddHit();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DP )
    {
      sum += m_pPlayer->GetTotalDp() - m_pPlayer->GetAllEquipAddDp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DG )
    {
      sum += m_pPlayer->GetTotalDodge() - m_pPlayer->GetAllEquipAddDodge();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_INT )
    {
      sum += m_pPlayer->GetTotalInt() - m_pPlayer->GetAllEquipAddInt();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EAP )
    {
      sum += m_pPlayer->GetAllEquipAddAp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EHIT )
    {
      sum += m_pPlayer->GetAllEquipAddHit();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EDP )
    {
      sum += m_pPlayer->GetAllEquipAddDp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EDG )
    {
      sum += m_pPlayer->GetAllEquipAddDodge();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_EINT )
    {
      sum += ( m_pPlayer->GetInt() * m_pPlayer->GetWeaponAddInt() / 100 );
    }
  }
  else
  {
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_AP )
    {
      sum += m_pMonster->GetTotalAp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_HIT )
    {
      sum += m_pMonster->GetTotalHit();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DP )
    {
      sum += m_pMonster->GetTotalDp();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_DG )
    {
      sum += m_pMonster->GetTotalDodge();
    }
    if( m_pBase->m_wOwnAttrEffect & SKILL_OWN_ATTR_INT )
    {
      sum += m_pMonster->GetTotalInt();
    }
  }
  return sum;
}
//=======================================================================================
// desc:			获取人物武器的使用效能
// parameter:
// return:
//=======================================================================================
inline int CSkill::GetWpRandom(CLife *pTarget)
{
  if( IsMonster() )   return 0;
  ///////////////////////////////////////////////////////////////////
  // 公式:
  //     WpNum=P_HIT+P_Add_Hit/(P_HIT+P_Add_Hit+T_DG+T_Add_DG)
  ///////////////////////////////////////////////////////////////////
  int         WpNum, T_DG;
  //
	if( pTarget->IsPlayer() )
		T_DG = ((CPlayer *)pTarget)->GetDodge();
	else
		T_DG = ((CMonster*)pTarget)->GetDodge();
  int divider = m_pPlayer->GetTotalHit() + T_DG;
  if( divider == 0 )
  {
//#ifdef _DEBUG_WILDCAT_
//    sprintf( g_szDamageLog, "%s: Player Divider Error, Ap=%d,Hit=%d,Dp=%d,Dodge=%d,Int=%d,T_DG=%d #",
//                                    m_pPlayer->GetPlayerAccount(), m_pPlayer->GetTotalAp(), 
//                                    m_pPlayer->GetTotalHit(), m_pPlayer->GetTotalDp(),
//                                    m_pPlayer->GetTotalDodge(), m_pPlayer->GetTotalInt(), T_DG );
//    g_DamageLog.Write( g_szDamageLog );
//#endif
    divider = 1;
  }
  WpNum = m_pPlayer->GetTotalHit() * 100 / divider;

  return ( gf_GetRandom( WpNum ) + 1 );
}
//=======================================================================================
// desc:			获取人物武器的使用效能
// parameter:
// return:
//=======================================================================================
void CMagic::BeActivation( CLife *pTarget )
{
  if( NULL == m_pInMap )
  {
    m_iStatus = MAGICSTATE_DEAD;
    return;
  }
  //
  if( m_iStatus == MAGICSTATE_DEAD || pTarget == ((CLife*)m_pPlayer) || pTarget == ((CLife*)m_pMonster) )
    return;
  if( pTarget->IsPlayer() && m_pPlayer )
  {
    // 红名一定被攻击
		//if( ((CPlayer*)pTarget)->GetPkCount() == 0 )
		{
      if( m_pPlayer->IsTeamer( ((CPlayer*)pTarget) ) )		      return;
      if( m_pInMap->IsCityWarMap() )
      {
        if( m_pPlayer->IsLeaguePlayer( (CPlayer*)pTarget ) )    return;
      }
      else
      {
        if( !m_bCanPK || m_pInMap->CanNotPK() )                 return;
//////////////////////////////////////////////////////////////////////
#ifdef ELYSIUM_3_7_VERSION
          if( m_pInMap->IsMatchField() )
          {
            if( m_pPlayer->GetFightInfo() == NULL || ((CPlayer*)pTarget)->GetFightInfo() == NULL )
            {
              return;
            }
          }
          else
          {
#endif
//////////////////////////////////////////////////////////////////////
            if( m_pPlayer->GetLevel() < LEVEL_PROTECT || pTarget->GetLevel() < LEVEL_PROTECT  )
                return;
//////////////////////////////////////////////////////////////////////
#ifdef ELYSIUM_3_7_VERSION
          }
#endif
//////////////////////////////////////////////////////////////////////

      }
		}
  }
  else
  {
    if( m_pInMap->CanNotPK() )                                  return;
  }
  ////////////////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-09
#ifdef  EVILWEAPON_3_6_VERSION
  if( pTarget->IsPlayer() && ( 
     ( ((CPlayer*)pTarget)->GetGMStatus() & GM_STATUS_NODIED) ||
     ( ((CPlayer*)pTarget)->GetSpecialStatus() & SPE_STATUS_GODEMODE )
     ))
#else
  if( pTarget->IsPlayer() && ((CPlayer*)(pTarget))->GetGMStatus() & GM_STATUS_NODIED )
#endif
  /////////////////////////////////////////////////////////////////////////////////////
    return;
  //
  m_pFirstTarget = pTarget;
  if( m_pBase->GetType() < SKILL_TYPE_TRAP_DURATIVE_NORMAL )
  {
    // 陷阱1, 陷阱2, 陷阱4 会移动的魔法
    m_iStatus = MAGICSTATE_ATTACK;
  }
  else if( m_pBase->GetType() <= SKILL_TYPE_TRAP_DURATIVE_EXPRESSION2 )
  {
    // 陷阱3持续性的陷阱或魔法
    m_iStatus      = MAGICSTATE_DURATIVE_ATTACK;
    m_dwActionTime = ClientTickCount + m_pBase->m_pBase->GetTriggerTime();
  }
  else
  {
    m_iStatus = MAGICSTATE_DEAD;
  }
}
#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_

//===========================================================
// Function : Use GuildSkill Effect To Guild Members
// Param    : the GuildSkill 
// Return   : BOOL
BOOL CSrvBaseSkill::DoGuildSkill( CSkill* pSkill )
{
	LPSNMSetSpecialStatus		pSpecialStatus    = (LPSNMSetSpecialStatus)(MsgSpecialStatus.Msgs[0].Data);
	BYTE										bSpecialLifeCount = 0;
  //
  if( NULL == pSkill || NULL == pSkill->m_pPlayer || NULL == pSkill->m_pPlayer->m_pInMap )     return FALSE;
	if( m_iTimes > 4   || m_iTimes < -4 )												    return FALSE;
  //
  static LifeTileList::iterator	   Iter_Sp;
	CLife										        *pSpecialLife = NULL;
  //
	for( Iter_Sp = pSkill->m_listTargets.begin(); Iter_Sp != pSkill->m_listTargets.end(); Iter_Sp++ )
	{
    pSpecialLife = (*Iter_Sp);
		if( !pSpecialLife->IsPlayer() )						continue;
		//
		SetGuildSkillState(pSpecialLife);
		//
		pSpecialStatus->dwCode_Skill      = ( pSpecialLife->GetCode() << 16 ) | m_wId;
		pSpecialStatus->qwStatus		      =   pSpecialLife->m_qwSpecialStatus;
    pSpecialStatus++;
    bSpecialLifeCount++;
  }
  if( bSpecialLifeCount )
  {
		MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus ) * bSpecialLifeCount;
    pSkill->m_pPlayer->m_pInMap->SendMsgNearPosition( MsgSpecialStatus, pSkill->m_pPlayer->m_iX, pSkill->m_pPlayer->m_iY );
  }
	return TRUE;
}

//===========================================================
// Function : Set GuildSkill Effect To Player and No Stacks
// Param    : the Player be Setted ( Speed Ap Hit Dp Dg )
// Return   : BOOL
BOOL CSrvBaseSkill::SetGuildSkillState(CLife* pLife)
{
	if( !pLife->IsPlayer() )				 return FALSE;
	//
	CPlayer * pPlayer								 = (CPlayer *)pLife;
	for( int iLoop=2; iLoop<7; iLoop++ ) // SPEED AP HIT DP DG
	{
		pPlayer->m_dwAlarmTime[iLoop]	 = ClientTickCount + m_iAlarmTime;					
		pPlayer->m_wFuncAlarm[iLoop]	 = m_wAlarmSkillId;
	}

	pPlayer->m_qwSpecialStatus			&=~(SPE_STATUS_HIGHSPEED|SPE_STATUS_AP|SPE_STATUS_HIT|SPE_STATUS_DP|SPE_STATUS_DODGE);
	pPlayer->m_qwSpecialStatus			|= m_qwStatus;
	
	pPlayer->m_iAddSpeed             = m_iTimes;
	pPlayer->ComputeSpeed();

  pPlayer->m_iAp									 = pPlayer->m_iAp - pPlayer->m_iAddAp;
  pPlayer->m_iAddAp							   = pPlayer->GetAp() * m_iApChange / 100;
	pPlayer->m_iAp								  += pPlayer->m_iAddAp;

  pPlayer->m_iHit							     = pPlayer->m_iHit - pPlayer->m_iAddHit;
  pPlayer->m_iAddHit							 = pPlayer->GetHit() * m_iHitChange / 100;
	pPlayer->m_iHit								  += pPlayer->m_iAddHit;

  pPlayer->m_iDp									 = pPlayer->m_iDp - pPlayer->m_iAddDp;
  pPlayer->m_iAddDp								 = pPlayer->GetDp() * m_iDpChange / 100;
	pPlayer->m_iDp								  += pPlayer->m_iAddDp;

  pPlayer->m_iDodge								 = pPlayer->m_iDodge - pPlayer->m_iAddDodge;
  pPlayer->m_iAddDodge						 = pPlayer->GetDodge() * m_iDgChange / 100;
	pPlayer->m_iDodge							  += pPlayer->m_iAddDodge;

	return TRUE;
}
#endif
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
//===========================================================
// Function : 处理武器装备触发的效果（镶嵌）
// Note:     
//
void CSrvBaseSkill::SetPlayerTesseraSkill( CPlayer * pPlayer, BOOL bAdjust )
{
  switch( m_wType )
  {
  case SKILL_TYPE_TESS_PLAYER_POINT:
  // Set Tessera Add Data Into Player Data
		pPlayer->m_iChangeMaxHp			+= m_iHpChange;
		pPlayer->m_iChangeMaxMp			+= m_iMpChange;
		pPlayer->m_iChangeMaxSp			+= m_iSpChange;
		pPlayer->m_iChangeSpeed			+= m_iTimes;
		pPlayer->m_wTesseraElement   = m_wElement;


		pPlayer->m_iMaxHp					  += m_iHpChange;
		pPlayer->m_iMaxMp	          += m_iMpChange;
		pPlayer->m_iMaxSp	          += m_iSpChange;

    if( bAdjust ) // kuang
    {
      if( pPlayer->m_iHp > pPlayer->m_iMaxHp ) pPlayer->m_iHp = pPlayer->m_iMaxHp; 
      if( pPlayer->m_iMp > pPlayer->m_iMaxMp ) pPlayer->m_iMp = pPlayer->m_iMaxMp; 
      if( pPlayer->m_iSp > pPlayer->m_iMaxSp ) pPlayer->m_iSp = pPlayer->m_iMaxSp;      
    }

		pPlayer->ComputeSpeed();

		pPlayer->m_dwMaxSoul				+= m_iSoulChange;

    if( bAdjust ) // kuang
    {
      if( pPlayer->m_dwSoul > pPlayer->m_dwMaxSoul ) 
        pPlayer->m_dwSoul = pPlayer->m_dwMaxSoul;
    }
    
		if( m_wElement )
		{
      if(!(pPlayer->m_qwSpecialStatus & SPE_STATUS_ELEMENT))
      {
        //pPlayer->m_qwSpecialStatus |= SPE_STATUS_ELEMENT;
        //pPlayer->m_dwAlarmTime[22]  = 0;
        pPlayer->m_wElement = m_wElement;
        pPlayer->Send_A_SETELEMENT();
      }
      else
      {
        SMsgData *pTheMsg = g_pGs->NewMsgBuffer();
        if( pTheMsg == NULL ) return;
        pTheMsg->Init();
        pTheMsg->dwAID				= A_SETELEMENT;
        pTheMsg->Msgs[0].Size = sizeof(DWORD);
        pTheMsg->dwMsgLen			= 1;
        *(DWORD*)(pTheMsg->Msgs[0].Data) = ( ((DWORD)pPlayer->m_wElement << 16 ) | m_wElement);
        pPlayer->AddSendMsg(pTheMsg);
      }
		}
		//
    break;
  case SKILL_TYPE_TESS_PLAYER_ATTACK:
    if (m_iHpChange < 100) //消除和克体型血泊的关联冲突
    {
      pPlayer->m_iTesseraSuckHp		+= m_iHpChange;
    }
    pPlayer->m_iTesseraSuckMp		+= m_iMpChange;
    pPlayer->m_iTesseraSuckSp		+= m_iSpChange;
    pPlayer->m_iTesseraSuckOdds	+= m_iProbability;
    if( m_wFuncChangeId )
    {
      CSrvBaseSkill         *pBaseSkill = g_pBase->GetBaseSkill( m_wFuncChangeId );
      if( pBaseSkill )       pPlayer->m_listTesseraAttack.push_back( pBaseSkill );
    }
    break;
  case SKILL_TYPE_TESS_PLAYER_DEFENCE:
    pPlayer->m_iTesseraRebound		 += m_iHpChange;
#ifdef _DEBUG_VERSION_FOR_BEIJING_
    pPlayer->m_iTesseraReboundOdds += m_iProbability * 70 / 100;
#else
		pPlayer->m_iTesseraReboundOdds += m_iProbability;
#endif
    pPlayer->m_iTesseraCritical		 += m_iCriticalHit;
    if( m_wFuncChangeId )
    {
      CSrvBaseSkill         *pBaseSkill = g_pBase->GetBaseSkill( m_wFuncChangeId );
      if( pBaseSkill )       pPlayer->m_listTesseraDefence.push_back( pBaseSkill );
    }
    break;
  default:
    break;
  }
}
//===========================================================
// Function : 处理取下武器装备触发的效果（镶嵌）
// Note:     
//
void CSrvBaseSkill::UnsetPlayerTesseraSkill( CPlayer * pPlayer )
{
  switch( m_wType )
  {
  case SKILL_TYPE_TESS_PLAYER_POINT:
		// Set Tessera Add Data Into Player Data
		pPlayer->m_iChangeMaxHp	   -= m_iHpChange;
		pPlayer->m_iChangeMaxMp	   -= m_iMpChange;
		pPlayer->m_iChangeMaxSp	   -= m_iSpChange;
		pPlayer->m_iChangeSpeed	   -= m_iTimes;
		pPlayer->m_wTesseraElement  = 0;


		pPlayer->m_iMaxHp					 -= m_iHpChange;
		pPlayer->m_iMaxMp	         -= m_iMpChange;
		pPlayer->m_iMaxSp	         -= m_iSpChange;

		if( pPlayer->m_iHp > pPlayer->m_iMaxHp )  pPlayer->m_iHp = pPlayer->m_iMaxHp;
		if( pPlayer->m_iMp > pPlayer->m_iMaxMp )  pPlayer->m_iMp = pPlayer->m_iMaxMp;
		if( pPlayer->m_iSp > pPlayer->m_iMaxSp )  pPlayer->m_iSp = pPlayer->m_iMaxSp;

		pPlayer->ComputeSpeed();
 
		pPlayer->m_dwMaxSoul       -= m_iSoulChange;
    if( pPlayer->m_dwSoul > pPlayer->m_dwMaxSoul ) pPlayer->m_dwSoul = pPlayer->m_dwMaxSoul;

		if( m_wElement )
		{
      if(!(pPlayer->m_qwSpecialStatus & SPE_STATUS_ELEMENT))
      {
        //pPlayer->m_qwSpecialStatus |= SPE_STATUS_ELEMENT;
			  //pPlayer->m_dwAlarmTime[22]  = 0;
        pPlayer->m_wElement = pPlayer->m_wElemBackup;
        pPlayer->Send_A_SETELEMENT();
      }
      else
      {
        SMsgData *pTheMsg = g_pGs->NewMsgBuffer();
        if( pTheMsg == NULL ) return;
        pTheMsg->Init();
        pTheMsg->dwAID				= A_SETELEMENT;
        pTheMsg->Msgs[0].Size = sizeof(DWORD);
        pTheMsg->dwMsgLen			= 1;
        *(DWORD*)(pTheMsg->Msgs[0].Data) = ( ((DWORD)pPlayer->m_wElement << 16 ) | pPlayer->m_wElemBackup);
        pPlayer->AddSendMsg(pTheMsg);
      }
		}
    
		break;
  case SKILL_TYPE_TESS_PLAYER_ATTACK:
    if (m_iHpChange < 100) //消除和克体型血泊的关联冲突
    {
      pPlayer->m_iTesseraSuckHp    -= m_iHpChange;
    }
    pPlayer->m_iTesseraSuckMp    -= m_iMpChange;
    pPlayer->m_iTesseraSuckSp    -= m_iSpChange;
		pPlayer->m_iTesseraSuckOdds  -= m_iProbability;

    if( m_wFuncChangeId )
    {
      CSrvBaseSkill         *pBaseSkill = g_pBase->GetBaseSkill( m_wFuncChangeId );
      if( pBaseSkill )       pPlayer->m_listTesseraAttack.remove( pBaseSkill );
    }
    break;
  case SKILL_TYPE_TESS_PLAYER_DEFENCE:
    pPlayer->m_iTesseraRebound     -= m_iHpChange;
#ifdef _DEBUG_VERSION_FOR_BEIJING_
    pPlayer->m_iTesseraReboundOdds -= m_iProbability * 70 / 100;
#else
		pPlayer->m_iTesseraReboundOdds -= m_iProbability;
#endif
    pPlayer->m_iTesseraCritical    -= m_iCriticalHit;
    if( m_wFuncChangeId )
    {
      CSrvBaseSkill         *pBaseSkill = g_pBase->GetBaseSkill( m_wFuncChangeId );
      if( pBaseSkill )       pPlayer->m_listTesseraDefence.remove( pBaseSkill );
    }
    break;
  default:
    break;
  }
}
#endif
/*
//=================================================================================
// Function : 获取获取此招式对人物(怪物)属性的影响
// Note:     
//        
inline int CSrvBaseSkill::GetOwnAttributeForHuan(CLife* pUser)
{
  if( m_wOwnAttrEffect == 0 )    return 0;
  int				 sum = 0;
  if( pUser->IsPlayer() )
  {
		CPlayer* pPlayer = (CPlayer*)pUser;
		//
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_AP )
    {
      sum += pPlayer->GetTotalAp() - pPlayer->GetAllEquipAddAp();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_HIT )
    {
      sum += pPlayer->GetTotalHit() - pPlayer->GetAllEquipAddHit();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_DP )
    {
      sum += pPlayer->GetTotalDp() - pPlayer->GetAllEquipAddDp();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_DG )
    {
      sum += pPlayer->GetTotalDodge() - pPlayer->GetAllEquipAddDodge();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_INT )
    {
      sum += pPlayer->GetTotalInt() - pPlayer->GetAllEquipAddInt();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_EAP )
    {
      sum += pPlayer->GetAllEquipAddAp();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_EHIT )
    {
      sum += pPlayer->GetAllEquipAddHit();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_EDP )
    {
      sum += pPlayer->GetAllEquipAddDp();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_EDG )
    {
      sum += pPlayer->GetAllEquipAddDodge();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_EINT )
    {
      sum += ( pPlayer->GetInt() * pPlayer->GetWeaponAddInt() / 100 );
    }
  }
  else if( pUser->IsMonster() )
  {
		CMonster* pMonster = (CMonster*)pUser;
		//
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_AP )
    {
      sum += pMonster->GetTotalAp();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_HIT )
    {
      sum += pMonster->GetTotalHit();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_DP )
    {
      sum += pMonster->GetTotalDp();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_DG )
    {
      sum += pMonster->GetTotalDodge();
    }
    if( m_wOwnAttrEffect & SKILL_OWN_ATTR_INT )
    {
      sum += pMonster->GetTotalInt();
    }
  }
  return sum;
}
//=================================================================================
// Function : 处理怪物属性相克
// Note:      为计算方便，FigureJinx和MoralJinx合成为一个变量F_M_Jinx
//						CSrvBaseSkill里不处理武器相克的部分
//
inline void CSrvBaseSkill::GetMonsterRelateJinx( CMonster * pTMonster, float & F_M_Jinx,float & RaceJinx,float & BossJinx )
{
  // 招式克怪物体形，相性 ( 五行在最外面克制 )
  if( (m_wElement&0xFFE0) )
  {
    if( 0 != ( F_M_Jinx = ( (m_wElement&0xFFE0) & pTMonster->m_wElement ) ) )
    {
      F_M_Jinx = GetBitCountFor_Element( F_M_Jinx );
      F_M_Jinx = F_M_Jinx * g_MyJinx;
    }
  }
  // 招式克怪物种族
  if( m_dwRaceAttri )
  {
    if( 0 != ( RaceJinx = m_dwRaceAttri & pTMonster->m_dwRaceAttri ) )
    {
      RaceJinx = GetBitCountForRace( RaceJinx );
      RaceJinx = RaceJinx * m_wRaceBonuRate;
    }
  }
  // 招式克魔王
  if( pTMonster->GetBaseId() == m_dwBossCode )
  {
    BossJinx = m_wBossBonuRate;
  }
}
*/
//////////////////////////////////////////////////////////////////////////
//Add by CECE 2004-04-06
#ifdef EVILWEAPON_3_6_VERSION
BOOL CSkill::DoEvilDamage( CLife *pLife,SNMDamage *pDamageInfo )
{
  BOOL bResult = FALSE;
  int  iLifeType = 0;
  CPlayer* pPlayer = GetPlayerUser();
  CMonster* pMonster = NULL;
  LPCEvilWeapon pEvilWeapon = NULL;
  if( pPlayer ) pEvilWeapon = pPlayer->GetEvilWeapon();
  if( pLife->IsPlayer() )       iLifeType = 1; //Player
  else if( pLife->IsMonster() ) iLifeType = 2; //Monster
  if( iLifeType > 0 )
  {
    pDamageInfo->wTargetCode = pLife->GetCode();          //Damage Code
    pDamageInfo->wSubHp      = GetHpChange();
    pDamageInfo->qwSpecial   = 0;
    //
    bResult = TRUE;
    //Damage用来计算HP(MP,SP)的伤害
    int iSoul;
    if( iLifeType==1 ) //Player
    {
      pPlayer = (CPlayer*)pLife;
#ifdef FIX_BUG_ATTACKED_WHILE_LOGOUT
			if( pPlayer->GetClientState() < CLIENTSTATE_CONNECT || pPlayer->m_bClientClaimLogout)
			{
				return FALSE;
			}
#endif
      //如果不是无敌状态
      if( !(
          ( pPlayer->GetGMStatus() & GM_STATUS_NODIED ) ||
          ( pPlayer->GetSpecialStatus() & SPE_STATUS_GODEMODE )
         ) )
      {
#ifdef _MODIFY_EVILWEAPON_
				{
					//将金刚妖器召唤效果修改为: 吸取对方的魂魄数量80%转化给自己。
					int iUserSoulA, iUserSoulB;
					CPlayer* pPlayerUser = GetPlayerUser();
					if (pPlayerUser)
					{
						iUserSoulA = pPlayerUser->GetSoul();
						iUserSoulB = pPlayer->GetSoul();
						if ((iUserSoulB - GetSoulChange()) > 0)
							iUserSoulB = GetSoulChange();
						pPlayerUser->SetSoul(iUserSoulA+iUserSoulB*8/10); // 0.8
					}
				}
#endif //_MODIFY_EVILWEAPON_
				//
				iSoul = pPlayer->GetSoul() - GetSoulChange();
        if( iSoul  < 0 )  iSoul  = 0;
        pPlayer->SetSoul( iSoul );
      }
      ///////////////////////////////////////////////////////////////////////////////////////////
      //技能的特殊伤害死亡以后照样计算
      switch( GetBaseSkill()->m_wType )
      {
      case SKILL_TYPE_EVILWEAPON_ATTACK:   //妖器辅助-强袭
        {
          /////////////////////////////////////
          // set player's attack status
          /////////////////////////////////////
          if( pPlayer->GetStatus() == STATUS_PLAYER_MOVE ) pPlayer->ClearMoveAction();
          //如果不是无敌状态
          if( !(
          ( pPlayer->GetGMStatus() & GM_STATUS_NODIED ) ||
          ( pPlayer->GetSpecialStatus() & SPE_STATUS_GODEMODE )
            ) )
          {          
            pDamageInfo->wSubHp = pPlayer->GetHp()-1;
            pPlayer->Damage( pDamageInfo, GetPlayerUser(), 0, 0 );
            m_pBase->ChangeChar( pLife );        //如果是AP,HIT,DP,DG,INT,
            m_pBase->SetSpecialState( pLife );   //如果有速度变化
            pDamageInfo->qwSpecial = pPlayer->GetSpecialStatus();
            //
            //发给自己
            pPlayer->Send_A_SETDYSFUNC( NULL, m_pBase->GetId() );
            pPlayer->Send_A_UPDATESOUL( NULL );	
          }
          else
          {
            pDamageInfo->wSubHp  = 0;
          }
        }
        break;
      case SKILL_TYPE_EVILWEAPON_NORMAL:   //妖器召唤-普通
      case SKILL_TYPE_EVILWEAPON_DELAP:    //妖器召唤-减狠
      case SKILL_TYPE_EVILWEAPON_DELHIT:   //妖器召唤-减准
      case SKILL_TYPE_EVILWEAPON_DELDP:    //妖器召唤-减稳
      case SKILL_TYPE_EVILWEAPON_DELDGE:   //妖器召唤-减快
      case SKILL_TYPE_EVILWEAPON_DELINT:   //妖器召唤-减智
        {
          //如果不是无敌状态
          if( !(
            ( pPlayer->GetGMStatus() & GM_STATUS_NODIED ) ||
            ( pPlayer->GetSpecialStatus() & SPE_STATUS_GODEMODE )
            ) )
          {
            pPlayer->Damage( pDamageInfo, GetPlayerUser(), m_pBase->m_iMpChange, m_pBase->m_iSpChange ); //HP
            m_pBase->ChangeChar( pLife );        //如果是AP,HIT,DP,DG,INT,
            m_pBase->SetSpecialState( pLife );   //如果有速度变化
            pDamageInfo->qwSpecial = pPlayer->GetSpecialStatus();
            //
            //发给自己
            pPlayer->Send_A_SETDYSFUNC( NULL, m_pBase->GetId() );
            pPlayer->Send_A_UPDATESOUL( NULL );
            //
            switch( GetBaseSkill()->m_wType )
            {
              case SKILL_TYPE_EVILWEAPON_DELAP:    //妖器召唤-减狠
              case SKILL_TYPE_EVILWEAPON_DELHIT:   //妖器召唤-减准
              case SKILL_TYPE_EVILWEAPON_DELDP:    //妖器召唤-减稳
              case SKILL_TYPE_EVILWEAPON_DELDGE:   //妖器召唤-减快
              case SKILL_TYPE_EVILWEAPON_DELINT:   //妖器召唤-减智
                   pPlayer->SetEvilAttack( TRUE );
                   break;
            }
            //
          }
          else
          {
            pDamageInfo->wSubHp  = 0;
          }
          //增加技能熟练度
          if( pEvilWeapon ) pEvilWeapon->AddSkillValue();
          /////////////////////////////////////
          // set player's attack status
          /////////////////////////////////////
          if( pPlayer->GetStatus() == STATUS_PLAYER_MOVE ) pPlayer->ClearMoveAction();
        }
        break;
      }
    }
    //Monster只减少HP
    else
    {
      switch( GetBaseSkill()->m_wType )
      {
      case SKILL_TYPE_EVILWEAPON_NORMAL:   //妖器召唤-普通
      case SKILL_TYPE_EVILWEAPON_DELAP:    //妖器召唤-减狠
      case SKILL_TYPE_EVILWEAPON_DELHIT:   //妖器召唤-减准
      case SKILL_TYPE_EVILWEAPON_DELDP:    //妖器召唤-减稳
      case SKILL_TYPE_EVILWEAPON_DELDGE:   //妖器召唤-减快
      case SKILL_TYPE_EVILWEAPON_DELINT:   //妖器召唤-减智
          pMonster = (CMonster*)pLife;
          pMonster->Damage( pDamageInfo, GetPlayerUser() );
          m_pBase->SetSpecialState( pLife );   //如果有速度变化
          //增加技能熟练度
          if( pEvilWeapon ) pEvilWeapon->AddSkillValue();
          break;
      }
    }
  }
  return bResult;
}
#endif
//////////////////////////////////////////////////////////////////////////
//
// Player对Monster的克恶 ，不包括对其他因素伤害的加成
//
//////////////////////////////////////////////////////////////////////////

#ifdef RESTRAIN_EVIL
float CSkill::RestrainForevil( CLife *pTarget )
{
  CItem          *pUseWeapon  = NULL;
  CSrvBaseItem   *TesseraItem = NULL;
  int            iRaceBonu    = 0;  
  int            iElement     = 0;
  float          fResult      = 0.0f;
  if( pTarget->IsMonster() ) 
  {
    pUseWeapon = m_pPlayer->m_pEquip[ITEM_EQUIPWHERE_HAND];
    if( pUseWeapon != NULL )
    { 
      for( int iSlot = 0; iSlot < MAX_ITEM_SLOT; iSlot++ )
      {  
        TesseraItem = g_pBase->GetBaseItem( pUseWeapon->m_wTesseraItemID[iSlot] ); //效率比较低
        if( TesseraItem != NULL ) 
        {
          iElement  = TesseraItem->GetElement();
          iRaceBonu = TesseraItem->GetFuncDbc();
          if( iRaceBonu != 0 && ( iElement & 1 ) )    //克恶血珀Element栏位为1
             fResult += (float)iRaceBonu*0.01f; 
        }
      }
    }
  }
 return fResult;
}
#endif //RESTRAIN_EVIL

//////////////////////////////////////////////////////////////////////////
//
//免疫风水术
//
//////////////////////////////////////////////////////////////////////////

#ifdef RESTRAIN_TRAP 
float CSkill::RestrainFortrap()
{
  CLife          *pTarget     = NULL;
  CItem          *pEquipment  = NULL;
  CSrvBaseItem   *TesseraItem = NULL;
  CSrvBaseSkill  *TesseraSkil = NULL;
  LifeTileIter   Iter_Tr; 
  float          fResult = -1.0;
  for( Iter_Tr = m_listTargets.begin(); Iter_Tr != m_listTargets.end(); Iter_Tr++ ) 
  {
    pTarget = (*Iter_Tr);
    if( pTarget != NULL && pTarget->IsPlayer() )
    {
      for( int iEquip = 1; iEquip < MAX_EQUIPWHERE; iEquip++ ) //该血珀不能镶嵌武器
      {
        pEquipment = ((CPlayer*)pTarget)->GetEquipment( iEquip );
        if( pEquipment != NULL )
        {      
          for( int iSlot = 0; iSlot < MAX_ITEM_SLOT; iSlot++ )
          {
            TesseraItem = g_pBase->GetBaseItem( pEquipment->m_wTesseraItemID[iSlot] );
            TesseraSkil = pEquipment->GetSklTessera(iSlot);
            if( TesseraItem != NULL && TesseraSkil != NULL )
            {
              if( (TesseraSkil->GetType() == SKILL_TYPE_RESTRAIN_TRAP ) && (TesseraItem->m_wFuncEffect == m_pBase->m_wRace || TesseraItem->m_wFuncEffect == 110) ) //当TesseraItem->m_wFuncEffect=110免疫所有风水术
              {
                if(gf_GetRandom(100) <= TesseraSkil->m_iProbability)
                {
                  if( TesseraSkil->m_iHpChange >= 65535 && TesseraSkil->m_iHpChange <= 65635 ) 
                  {
                    fResult = 1.0f - (TesseraSkil->m_iHpChange - 65535)*0.01f;
                    if ( fResult <= 0.001f ) 
                    {
                      fResult = 0.0f;
                    }
                  }
                }
              }   
            }
          }
        }
      }
    }
  }
  return fResult;
}                                       
#endif //RESTRAIN_TRAP
//////////////////////////////////////////////////////////////////////////
//
//克体型血珀
//
//////////////////////////////////////////////////////////////////////////
#ifdef RESTRAIN_BODY
float CSkill::RestrainForbody(CLife *pTarget)
{
  CItem          *pUseWeapon   = NULL;
  CSrvBaseItem   *TesseraItem  = NULL;
  int            iRaceBonu     = 0;  
  int            iElement      = 0;
  float          fResult       = 0.0f;
  CSrvBaseSkill  *TesseraSkill = NULL;

  if( pTarget->IsMonster() ) 
  {
    pUseWeapon = m_pPlayer->m_pEquip[ITEM_EQUIPWHERE_HAND];
    if( pUseWeapon != NULL )
    { 
      for( int iSlot = 0; iSlot < MAX_ITEM_SLOT; iSlot++ )
      {  
        TesseraItem = g_pBase->GetBaseItem( pUseWeapon->m_wTesseraItemID[iSlot] );
        if( TesseraItem != NULL ) 
        {
          iElement     = TesseraItem->GetElement();
          TesseraSkill = pUseWeapon->GetSklTessera(iSlot);        
          if( (TesseraSkill != NULL) && (iElement == 32 || iElement == 64 || iElement == 128 || iElement == 256) && (pTarget->m_wElement & iElement) )
          {
            fResult += (TesseraSkill->m_iHpChange - 65535)*0.01f;
            if ( fResult <= 0.001f ) 
            {
              fResult = 0.0f;
            }
          }
        }
      }
    }
  }
  return fResult;
}
#endif //RESTRAIN_BODY

//////////////////////////////////////////////////////////////////////////
//
//降低反弹血珀:  几率叠加 反弹比例取最大值
//
//////////////////////////////////////////////////////////////////////////
#ifdef DEPRESS_TESS_DEFENCE
int CSkill::DepressDefence()
{
  CItem         *pTemItem    = NULL;
  int           iProSum      = 0;
  int           iTemp        = 0;
  CSrvBaseSkill *pSkill      = NULL;
  int           iProbability = 0;
  
  pTemItem = m_pPlayer->GetEquipment(ITEM_EQUIPWHERE_FOOT);
  if(pTemItem != NULL) 
  {
    for( int iLoop=0; iLoop < MAX_ITEM_SLOT; iLoop++ ) 
    {
      if( (pSkill = pTemItem->GetSklTessera(iLoop)) != NULL
        &&(pSkill->GetType() == SKILL_TYPE_DEPRESS_DEFENCE) ) 
      {
        iProbability += pSkill->GetProbability(); 
        iTemp = (pSkill->m_iHpChange - 65535);
        if(iTemp > iProSum) 
        {
          iProSum = iTemp;
        }
      }
    }
  }
  if(iProbability > gf_GetRandom(100)) 
  {
    return iProSum;
  }
  else
    return 100;        //Meanse: renturn values 1 = 100/100
}
#endif

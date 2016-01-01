#include "stdafx.h"
#include "BfInit.h"

#include "SrvGsData.h"
#include "../Network/SrvClientData.h"
#include "../Mcc/SrvMccInfo.h"
#include "../SrvClass/Map/SrvMap.h"
//#include "../SrvClass/Life/SrvLife.h"
#include "../SrvClass/Life/SrvMonster.h"
#include "../SrvClass/Life/SrvPlayer.h"
#include "../SrvClass/Item/SrvItem.h"
#include "../SrvClass/Skill/SrvSkill.h"
#include "../SrvClass/Talk/SrvTalk.h"
#include "../SrvClass/Event/SrvEvent.h"
#include "../SrvClass/Time/GameTime.h"
#include "../DB Thread/DBThread.h"
#include "../SrvClass/Guild/SrvGuild.h"
//add by cece
#include <string>
#include <map>
#include <list>
#include <fstream>
#include <strstream>
using namespace std;
//
int  g_CriticalRate = 0,  g_MaxCritical = 0,  g_MaxHit        = 0,  g_MinHit       = 0;
int  g_MyJinx       = 0,  g_SkillJinx   = 0,  g_ChainJinx     = 0,  g_BaseHit      = 0, g_ChainHit    = 0;
int  g_DpRate       = 0,  g_DpLimit     = 0,  g_HitLvRate     = 0;
int  g_MinHit_Ji    = 0,  g_MaxHit_Ji   = 0,  g_MinHit_Jian   = 0,  g_MaxHit_Jian  = 0;
int  g_MinHit_Gui   = 0,  g_MaxHit_Gui  = 0,  g_MinHit_Huan   = 0,  g_MaxHit_Huan  = 0;
int  g_MinFlee_Ji   = 0,  g_MaxFlee_Ji  = 0,  g_MinFlee_Jian  = 0,  g_MaxFlee_Jian = 0;
int  g_MinFlee_Gui  = 0,  g_MaxFlee_Gui = 0,  g_MinFlee_Huan  = 0,  g_MaxFlee_Huan = 0;
int  g_MaxHpPercent = 0,  g_LevelLimit  = 0,  g_BearJian      = 1,  g_BearJi       = 1, g_BearGui     = 1, g_BearHuan = 1;
int  g_iMdef_Limit  = 0,  g_iTeamExpBonu= 0;
int  g_ElementNull  = 0,  g_ElementGrow = 0,  g_ElementJinx   = 0,  g_ElementJxed  = 0, g_ElementSame = 0, g_iPK_Punish = 0, g_iPK_Expend = 0;
int  g_iDeadLoseExp_1 = 0, g_iDeadLoseExp_2 = 0, g_iDeadLoseExp_3 = 0, g_iDeadLoseExp_4 = 0, g_iDeadLoseExp_5 = 0;
int  g_iMaxTaxRate  = 20;
//add by zetorchen
#ifdef _MONSTER_RESTORE_HP_
int  g_iMonsterHP1 = 0;
int  g_iMonsterHP2 = 0;
int  g_iMonsterHP3 = 0;
#endif
#ifdef MULT_EXP
int   g_iMultiple1 = 0;
int   g_iMultiple2 = 0;
BOOL  g_bMultExp  = FALSE; 
#endif
#ifdef VERSION40_UNIQUERING
listUniqueItem              g_listUniItem;
BOOL                        g_bHaveUniqueRing = FALSE;
CRingManager*               g_pRingManager = NULL;
#endif
//add by Jack.Ren
#ifdef MARKITEM_40_MARKRULST
  int   g_iMarkItemCost = 0;
  int   g_iMarkItemId   = 0;
#endif

#ifdef _AUTO_RUN_CITY_WAR_
  int   g_iAutoWeek     = -1;  //以周为单位
  int   g_iAutoHour     = -1;  //
  int   g_iAutoMinute   = -1;  //
  int   g_iAutoMapId    = -1;  //攻城战地图ID
  int   g_iAutoStanding = -1;  //每场持续时间
#endif

#ifdef _AUTO_ADD_WARP_POINT_
  map<int, _AddWarpPoint*> g_mapAddWarpPiont;  // <week, struct>
#endif
  
#ifdef FUNCTION_MAKEHOLE_ITEM
  int   g_iMakeFirstHole = 50; //开第一个孔的几率
  int   g_iMakeSeconHole = 30; //开第二个孔的几率
  int   g_iMakeThirdHole = 10; //开第三个孔的几率
#endif
#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_
int  g_iGuildSkill_MemberMin = MAX_GUILD_MEMBER;
#endif
#ifdef _DEBUG_MICHAEL_ONLINE_UPLEVEL
int  g_iOnline_UpLevel = 6; // 小时
int  g_iOnline_Percent = 6; // 上涨的经验万分比
#endif
///////////////////////////////////////////////////////////////////////////////////////////////
//
//
// BF Game Server Global Data
char _szMsgMCC_[MAX_MEMO_MSG_LEN];
char _szMsgMain_[MAX_MEMO_MSG_LEN];
char _szMsgClient_[MAX_MEMO_MSG_LEN];
//added by nick 
CRITICAL_SECTION     g_cs_MsgLock;
//
DWORD g_dwMCCThreadID     = 0;
DWORD g_dwClientThreadID  = 0;
DWORD g_dwReadWriteThreadID = 0;

CMain_network_thread	          *g_pAcceptThread = NULL;
CMain_Client_ReadWrite_Thread   *g_pNetThread = NULL;
CMcc_Network_Thread		          *g_pMccThread  = NULL;

CGsShowMessage				*g_pShowMessage = NULL;
CLogFile							SrvLog;
CLogFile							SrvErrLog;
CLogFile							g_DamageLog;
CLogFile							g_PlayerLog;
CLogFile              g_CharLog;
CLogFile							g_ReloginLog;
CLogFile							g_AERRORLog;
CLogFile							g_MccLog;
CLogFile							g_OnlinePairLog;
CLogFile							g_NetMessLog;
CLogFile							g_NetMessMccLog;
CLogFile              g_AIDLog;
CLogFile              g_CrashLog;
CLogFile              g_TalkLog;
CLogFile              g_ShutdownLog;
CLogFile              g_ClickNpc;
CLogFile              g_ComboLog;
CLogFile              g_MapPlayerCountLog;
//Added by nick
#ifdef _DEBUG_NICK_FIND_TAX_BUG_
CLogFile              g_GetTaxLog;
#endif
//

SYSTEMTIME            g_SysTime;

#ifdef _DEBUG_CHECK_FUNC_STATE_
CLogFile     g_CheckClientLog, g_CheckMainLog, g_CheckMccLog;
#endif

#ifdef _DEBUG_CHECK_ACTION_STATE_
CLogFile              g_Action;
#endif
#ifdef _DEBUG_CHECK_UNIQUE_ITEN_
CSrvBaseItem *g_pCopyItem = NULL;
#endif
CSessionNum						g_MsgId;
CSrvBaseData					*g_pBase = NULL;
CGsData								*g_pGs = NULL;
CReloginServer				*g_pReloginServer;
// About Warp Point List
CWarpMap						  *g_pWaprPointList = NULL;
CMixItem							*g_pMixItemList = NULL;

#ifdef _NEW_TRADITIONS_WEDDING_
GSWeddingMgr          *g_GSWeddingMgr= NULL;
CAwaneMapMgr          *g_AwaneMapMgr= NULL;
#endif

CPlayer               *g_pMultiSendPlayer[MAX_MULTISEND_PLAYER];
CPlayer               *g_pMultiSendPlayer_2[MAX_MULTISEND_PLAYER];
CPlayer               *g_pMultiSendPlayer_3[MAX_MULTISEND_PLAYER];

CClientList						*g_pClientList = NULL;
CMccInfo							*g_pMccDB = NULL;
CMccInfo							*g_pMccChat = NULL;
CDB_Handle_Thread     *g_pSqlDB = NULL;
Query                 *g_pQuery = NULL;
CChatroomList					*g_pChatroomList = NULL;
CMercenaryManager     *g_MercenaryMgr = NULL;
#ifdef VERSION_40_HOUSE_FUNCTION
CPutMonster           *g_pPutMonster = NULL;
CBackMusic            *g_pBackMusic = NULL;
#endif
#ifdef VERSION40_FIELD
CField                *g_pField = NULL;
CRecurStone           *g_pStone = NULL;
#endif
CCityWarTimeManager   *g_CityWarTimeMng = NULL;
SGsTopTenManager      *g_pTopTenMng = NULL;
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
CFerryManager				  *g_pFerryMng  = NULL; // 渡船管理 michael adde 2003-11-17
#endif
SMsgData              MsgClearCode;			// Clear Code
SMsgData							MsgPickItem;			// Monster Pick Up Ground Item
SMsgData							MsgDamage;				// Someone Damage Others
SMsgData							MsgDefenceDamage;	// Someone Defence Damage Others
SMsgData							MsgSpecialStatus;	// Set Players Special Status
SMsgData              MsgPlyCheckTimer; // Player Check Timer About Special Status
SMsgData              MsgMstCheckTimer; // Monster Check Timer About Special Status
SMsgData							MsgDropItem;			// Drop Item When Player Drop Or Monster Dead Drop
SMsgData							MsgDead;					// About Player And Monster Dead
SMsgData              MsgGoHome;
SMsgData              MsgMoveObj;
SMsgData              MsgUseSkill;
SMsgData              MsgUseItem;
SMsgData              MsgSetAspect;
SMsgData              MsgSetLevel;
SMsgData              MsgChangeMap;
SMsgData              MsgChangeMapOK;
SMsgData              MsgPlayerJoin;
SMsgData              MsgChangeTitle;
SMsgData              MsgMagicDamage;
SMsgData              MsgPlyRevive;
SMsgData              MsgMagicResult;
SMsgData              MsgGuildId;
SMsgData              MsgClearPK;
SMsgData              MsgUpdateMstHp;
SMsgData              MsgSwitch;
SMsgData              MsgBossDrop;
////////////////////////////////////////
//Add by CECE 2004-04-16
#ifdef EVILWEAPON_3_6_VERSION
SMsgData              MsgEvilWarp;
SMsgData              MsgEvilSummon;
#endif
#ifdef _VERSION40_CARRYDOOR
WORD*              g_pVar = NULL;
CDoorManager*         g_pDoorManager = NULL;
//extern CSessionNum g_CodeCarryDoor; 
#endif
///////////////////////////////////////

#ifdef _DEBUG_MICHAEL_TESSERA_EX_
SMsgData              MsgAddiDamage;
SMsgData              MsgDefeDamage;
#endif
//=============================================
// 怪物掉道具
SMccMsgData           g_MonsterDropMsg;
SMonsterDropItem      *g_pMstDropLog = (SMonsterDropItem*)(g_MonsterDropMsg.Msgs[0].Data);
WORD                  g_wMstDropNum  = 0;
// 道具合成
SMccMsgData           g_MixItemMsg;
SItemMix              *g_pMixItemLog = (SItemMix*)(g_MixItemMsg.Msgs[0].Data);
WORD                  g_wMixItemNum  = 0;
// 道具镶嵌
SMccMsgData           g_TesseraItemMsg;
SItemTessera          *g_pTesseraItemLog = (SItemTessera*)(g_TesseraItemMsg.Msgs[0].Data);
WORD                  g_wTesseraItemNum  = 0;
// 道具加持
SMccMsgData           g_BlessItemMsg;
SItemBless            *g_pBlessItemLog = (SItemBless*)(g_BlessItemMsg.Msgs[0].Data);
WORD                  g_wBlessItemNum  = 0;
// 人物删除，登录，创建记录
SMccMsgData           g_AlterItemMsg;
SPlayerAlterLog       *g_pAlterItemLog = (SPlayerAlterLog*)(g_AlterItemMsg.Msgs[0].Data);
WORD                  g_wAlterItemNum  = 0;

//////////////////////////////////////////
//	added by michael 
//	帮会的删除、创建、传位
SMccMsgData						g_GuildInfoLogMsg;
SGuildInfoLog					*g_pGuildInfoLog			= (SGuildInfoLog*)(g_GuildInfoLogMsg.Msgs[0].Data);
WORD                  g_wGuildInfoLogNum		= 0;
//  存取仓库的钱以及从税收中提钱
SMccMsgData						g_PickDropMoneyMsg;
SPickDropMoney				*g_pPickDropMoneyLog	= (SPickDropMoney*)(g_PickDropMoneyMsg.Msgs[0].Data);
WORD          				g_wPickDropMoneyNum		= 0;
//  added end
//////////////////////////////////////////

//add by cece
//
//=============================================

WORD									g_PlayerOnlineCount = 0;
WORD					        g_MccOnlineCount = 0;

WORD									g_MccMsgBufferUsed = 0;
WORD									g_TimeoutMsgBufferUsed = 0;
int                   g_MccRecvMsgBufferUsed = 0;
int                   g_SysMsgBufferUsed = 0;

CGsWhisper            *g_pWhispers = NULL;

WORD  g_ShapeCount[MAX_SKILL_SHAPE_COUNT] = {
	1, 9, 25, 49, 81, 1, 3, 5, 7, 2,
		4, 6, 8, 6, 12, 3, 5, 7, 8, 11
};

//DWORD g_SpeedArray[7][8] = {
//	{   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 }, // 每 秒 0 格
//	{ 1000, 2000, 1000, 2000, 1000, 2000, 1000, 2000 }, // 每 秒 1 格
//	{  500, 1000,  500, 1000,  500, 1000,  500, 1000 }, // 每 秒 2 格
//	{  333,  666,  333,  666,  333,  666,  333,  666 }, // 每 秒 3 格
//	{  250,  500,  250,  500,  250,  500,  250,  500 }, // 每 秒 4 格
//	{  200,  400,  200,  400,  200,  400,  200,  400 }, // 每 秒 5 格
//	{  166,  332,  166,  332,  166,  332,  166,  332 }, // 每 秒 6 格
//};
//DWORD g_SpeedArray[7][8] = {
//	{   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 }, // 每 秒 0 格
//	{ 1000, 2000, 1000, 2000, 1000, 2000, 1000, 2000 }, // 每 秒 1 格
//	{  500, 1000,  500, 1000,  500, 1000,  500, 1000 }, // 每 秒 2 格
//	{  333,  666,  333,  666,  333,  666,  333,  666 }, // 每 秒 3 格
//	{  250,  500,  250,  500,  250,  500,  250,  500 }, // 每 秒 4 格
//	{  200,  400,  200,  400,  200,  400,  200,  400 }, // 每 秒 5 格
//	{  166,  333,  166,  333,  166,  333,  166,  333 }, // 每 秒 6 格
//};
DWORD g_SpeedArray[9][9] = {
	{   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 }, // 每 秒 0 格
	{ 1000, 2000, 1000, 2000, 1000, 2000, 1000, 2000, 1500 }, // 每 秒 1 格
	{  500, 1000,  500, 1000,  500, 1000,  500, 1000, 750  }, // 每 秒 2 格
	{  380,  760,  380,  760,  380,  760,  380,  760, 570  }, // 每 秒 3 格
	{  300,  600,  300,  600,  300,  600,  300,  600, 450  }, // 每 秒 4 格
	{  250,  500,  250,  500,  250,  500,  250,  500, 375  }, // 每 秒 5 格
	{  200,  400,  200,  400,  200,  400,  200,  400, 300  }, // 每 秒 6 格
	{  150,  300,  150,  300,  150,  300,  150,  300, 225  }, // 每 秒 7 格
	{  100,  200,  100,  200,  100,  200,  100,  200, 150  }, // 每 秒 8 格
};
//DWORD g_SpeedArray[4][8] = {
//	{   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1 },
//	{  500, 1000,  500, 1000,  500, 1000,  500, 1000 },
//	{  250,  500,  250,  500,  250,  500,  250,  500 },
//	{  167,  334,  167,  334,  167,  334,  167,  334 },
//};

// 新建人物时的初始属性－－包含(正确顺序):
// Ap; Hit; Dp; Dodge; Int; MaxHp; MaxMp; MaxSp; UpdateLevelBonu, UpdateMaxSoul
int g_NewCharAttr[4][10];

// 人物升级时点选不同的升级项目所影响的MaxHp, MaxMp, MaxSp
int g_PlayerUpdateAttr[4][5][3];                    // [剑宗][狠][MaxHp]
                                                    // [剑宗][狠][MaxMp]
                                                    // [剑宗][狠][MaxSp]
                                                    // [剑宗][准][MaxHp]
                                                    // [剑宗][准][MaxMp]
                                                    // ...
// 人物升级时系统分配的－－狠, 准, 稳, 快,  智, MaxHp, MaxMp, MaxSp, 攻击力乱数
// 当前等级系统累加Ap, Hit, Dp, Dodge, Int, MaxHp, MaxMp, MaxSp -- Server 自己算
int g_SystemUpdateAttr[4][MAX_PLAYER_LEVEL+1][17];  // [剑宗][Level][Ap]

// 人物升级时点数分配限制--狠, 准, 稳, 快, 智(剑, 戟, 诡, 幻)
int g_OccupAddLimit[4][MAX_PLAYER_LEVEL+1][5];
///////////////////////////////////////////////////////////////////////////////////////////////
//
// Extern Global Variable
//  
extern list<CLife*>		  g_BaseTargetList;
extern list<CMonster*>  g_ListUnreviveMonster;
extern CGuildManager    *g_pGuildMng;
///////////////////////////////////////////////////////////////////////////////////////////////
//
//
//

//============================================================================================================
//
// Extern Global Variables
extern CMDropItemList	*g_pDropItem;

//============================================================================================================
//
// global functions for init and release
bool InitGlobalVar(HWND hTheWnd)
{ // Init Global variable
  SrvLog.Init(BFSERVER_LOGNAME, BFSERVER_LOGPATH);
  SrvErrLog.Init(BFSERVER_ERR_LOGNAME, BFSERVER_ERR_LOGPATH);
	g_DamageLog.Init(BFSERVER_DAMAGE_LOGNAME,BFSERVER_DAMAGE_LOGPATH);
	g_PlayerLog.Init(BFSERVER_PLAYER_LOGNAME,BFSERVER_PLAYER_LOGPATH);
  g_CharLog.Init(BFSERVER_CHAR_LOGNAME,BFSERVER_CHAR_LOGPATH);
	g_ReloginLog.Init(BFSERVER_RELOGIN_LOGNAME,BFSERVER_RELOGIN_LOGPATH);
	g_AERRORLog.Init(BFSERVER_AERROR_LOGNAME,BFSERVER_AERROR_LOGPATH);
	g_MccLog.Init(BFSERVER_MCC_LOGNAME,BFSERVER_MCC_LOGPATH);
	g_OnlinePairLog.Init(BFSERVER_ONLINE_LOGNAME,BFSERVER_ONLINE_LOGPATH);
  g_NetMessLog.Init(BFSERVER_NETMESS_LOGNAME,BFSERVER_NETMESS_LOGPATH);
  g_NetMessMccLog.Init(BFSERVER_NETMESS_MCC_LOGNAME,BFSERVER_NETMESS_MCC_LOGPATH);
  //
  g_AIDLog.Init( BFSERVER_NETMESS_AID_LOGNAME, BFSERVER_NETMESS_AID_LOGPATH );
  g_CrashLog.Init( BFSERVER_NETMESS_CRASH_LOGNAME, BFSERVER_NETMESS_CRASH_LOGPATH );
  g_TalkLog.Init( BFSERVER_NETMESS_TALK_LOGNAME, BFSERVER_NETMESS_TALK_LOGPATH );
  g_ShutdownLog.Init( BFSERVER_SHUTDOWN_LOGNAME, BFSERVER_SHUTDOWN_LOGPATH );
  g_ClickNpc.Init( BFSERVER_NPC_LOGNAME, BFSERVER_NPC_LOGPATH );
  g_ComboLog.Init( "Combo", "Server Log\\ComboSkill" );
#ifdef _DEBUG_CHECK_ACTION_STATE_
  g_Action.Init(BFSERVER_ACTION_LOGNAME,BFSERVER_ACTION_LOGPATH);
#endif
//Added by nick
#ifdef _DEBUG_NICK_FIND_TAX_BUG_
	g_GetTaxLog.Init( BFSERVER_GET_TAX_LOGNAME, BFSERVER_TAX_LOGPATH );
#endif
//
#ifdef _DEBUG_CHECK_FUNC_STATE_
  g_CheckClientLog.Init("CltFunc","Server Log\\Client Func");
  g_CheckMainLog.Init("MainFunc","Server Log\\Main Func");
  g_CheckMccLog.Init("MccFunc","Server Log\\Mcc Func");
#endif

  g_MapPlayerCountLog.Init( "MapPlayerCount", "Server Log\\MapPlayerCount" );
  // Create Server Log Folder
  // ...
  //
 
  g_pShowMessage    = new CGsShowMessage;
  g_pGs             = new CGsData( hTheWnd );
  g_pMccDB          = new CMccInfo;
	g_pMccChat        = new CMccInfo;
  g_pBase           = new CSrvBaseData;
  g_pChatroomList   = new CChatroomList();
	g_pReloginServer  = new CReloginServer;
  //
  g_pClientList     = new CClientList;
	g_pDropItem       = new CMDropItemList;

	// About Warp Point List
	g_pWaprPointList  = new CWarpMap();
	g_pMixItemList    = new CMixItem();
#ifdef _NEW_TRADITIONS_WEDDING_
  g_AwaneMapMgr     = new CAwaneMapMgr();
  g_GSWeddingMgr    = new GSWeddingMgr();
#endif

  g_pWhispers       = new CGsWhisper;
  // About Unrevive Monster
  g_ListUnreviveMonster.clear();
	// Set Base Money
  g_pGuildMng       = new CGuildManager;
  //
  g_MercenaryMgr    = new CMercenaryManager;
#ifdef VERSION_40_HOUSE_FUNCTION
  g_pPutMonster     = new CPutMonster;
  //
  g_pBackMusic      = new CBackMusic;
#endif
#ifdef VERSION40_FIELD
  g_pField          = new CField;
#endif
#ifdef VERSION40_UNIQUERING
  g_pRingManager    = new CRingManager; 
#endif
  //
  g_CityWarTimeMng  = new CCityWarTimeManager;
  //
  g_pTopTenMng      = new SGsTopTenManager;
  //
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
	g_pFerryMng       = new CFerryManager;
#endif
	//
  memset( g_pMultiSendPlayer, NULL, sizeof( DWORD ) * MAX_MULTISEND_PLAYER );
  memset( g_pMultiSendPlayer_2, NULL, sizeof( DWORD ) * MAX_MULTISEND_PLAYER );
  memset( g_pMultiSendPlayer_3, NULL, sizeof( DWORD ) * MAX_MULTISEND_PLAYER );
	// About Clear Code Message
  MsgClearCode.Init();
  MsgClearCode.dwAID        = A_CLEARCODE;
  MsgClearCode.dwMsgLen     = 1;
  MsgClearCode.Msgs[0].Size = sizeof( DWORD );
  MsgClearCode.Msgs[1].Size = 0;

	// About Monster Pick Up Ground Item
	MsgPickItem.Init();
	MsgPickItem.dwAID        = A_MONSTERPICKITEM;
	MsgPickItem.dwMsgLen     = 1;
	MsgPickItem.Msgs[0].Size = ( sizeof( DWORD ) << 1 );
  MsgPickItem.Msgs[1].Size = 0;

	// About Init Damage Message
	MsgDamage.Init();
	MsgDamage.dwAID        = A_DAMAGE;
	MsgDamage.dwMsgLen     = 1;
  MsgDamage.Msgs[1].Size = 0;

  // About Tessera Defence Damage
	MsgDefenceDamage.Init();
	MsgDefenceDamage.dwAID        = A_DAMAGE;
	MsgDefenceDamage.dwMsgLen     = 1;
  MsgDefenceDamage.Msgs[1].Size = 0;

	// About Clear Special Status
	MsgSpecialStatus.Init();
	MsgSpecialStatus.dwAID        = A_SETDYSFUNC;
	MsgSpecialStatus.dwMsgLen     = 1;
  MsgSpecialStatus.Msgs[1].Size = 0;
	//MsgSpecialStatus.Msgs[0].Size = sizeof( SNMSetSpecialStatus );

	// About Clear Special Status
	MsgPlyCheckTimer.Init();
	MsgPlyCheckTimer.dwAID        = A_SETDYSFUNC;
	MsgPlyCheckTimer.dwMsgLen     = 1;
  MsgPlyCheckTimer.Msgs[1].Size = 0;
	//MsgPlyCheckTimer.Msgs[0].Size = sizeof( SNMSetSpecialStatus );

	// About Clear Special Status
	MsgMstCheckTimer.Init();
	MsgMstCheckTimer.dwAID        = A_SETDYSFUNC;
	MsgMstCheckTimer.dwMsgLen     = 1;
  MsgMstCheckTimer.Msgs[1].Size = 0;
	//MsgMstCheckTimer.Msgs[0].Size = sizeof( SNMSetSpecialStatus );

	// About Player And Monster Drop Item Or Money
	MsgDropItem.Init();
	MsgDropItem.dwAID        = A_ADDGROUNDITEM;
	MsgDropItem.dwMsgLen     = 1;
  MsgDropItem.Msgs[1].Size = 0;

	// About Player And Monster Dead
	MsgDead.Init();
	MsgDead.dwAID = A_DIED;

  // About Monster Go Home
  MsgGoHome.Init();
  MsgGoHome.dwAID        = A_MONSTERGOHOME;
  MsgGoHome.dwMsgLen     = 1;
  MsgGoHome.Msgs[0].Size = sizeof( WORD ) * 3;
  MsgGoHome.Msgs[1].Size = 0;

  // About Move Obj
  MsgMoveObj.Init();
  MsgMoveObj.dwAID        = A_MOVEOBJ;
  MsgMoveObj.dwMsgLen     = 1;
  MsgMoveObj.Msgs[0].Size = sizeof( SNMMoveObjs );

  // About Use Skill
  MsgUseSkill.Init();
  MsgUseSkill.dwAID        = A_USESKILL;
  MsgUseSkill.dwMsgLen     = 1;
  MsgUseSkill.Msgs[0].Size = sizeof( SNMUseSkill );
  MsgUseSkill.Msgs[1].Size = 0;

  // About Use Item
  MsgUseItem.Init();
  MsgUseItem.dwAID    = A_USEITEMINFO;
  MsgUseItem.dwMsgLen = 1;
  MsgUseItem.Msgs[0].Size = sizeof( WORD ) * 2;
  MsgUseItem.Msgs[1].Size = 0;
  
  // About Set Aspect
  MsgSetAspect.Init();
  MsgSetAspect.dwAID        = A_SETASPECT;
	MsgSetAspect.dwMsgLen     = 1;
	MsgSetAspect.Msgs[0].Size = sizeof(WORD) * 3;

  // About Set Level
  MsgSetLevel.Init();
	MsgSetLevel.dwAID					= A_SETLEVEL;
	MsgSetLevel.dwMsgLen			= 1;
	MsgSetLevel.Msgs[0].Size  = sizeof( SNMUpgrade );

  // About Change Map
  MsgChangeMap.Init();
  MsgChangeMap.dwAID        = A_CHANGEMAP;
  MsgChangeMap.dwMsgLen     = 1;
  MsgChangeMap.Msgs[0].Size = sizeof( SNMMapPosition );

  // About Change Map OK
  // ...

  // About Player Info
  MsgPlayerJoin.Init();
  MsgPlayerJoin.dwAID    = A_PLAYERJOIN;
  MsgPlayerJoin.dwMsgLen = 1;
  MsgPlayerJoin.Msgs[0].Size = sizeof( SNMPlayerInfo ) + sizeof( WORD );

  // About Change Title
  MsgChangeTitle.Init();
  MsgChangeTitle.dwAID    = A_CHANGETITLE;
  MsgChangeTitle.dwMsgLen = 1;
  MsgChangeTitle.Msgs[0].Size = sizeof( SNMChangeTitle );

  // About Magic Damage
  MsgMagicDamage.Init();
  MsgMagicDamage.dwAID    = A_MAGICDAMAGE;
  MsgMagicDamage.dwMsgLen = 1;
  MsgMagicDamage.Msgs[0].Size = sizeof( SNMMgcDmgHeader );
  MsgMagicDamage.Msgs[1].Size = 0;

  // About Player Revive By Item Or Magic
  MsgPlyRevive.Init();
  MsgPlyRevive.dwAID        = A_REVIVE;
  MsgPlyRevive.dwMsgLen     = 1;
  //MsgPlyRevive.Msgs[0].Size = sizeof( SNMReviveData );

  // About Magic Result -- Add Hp, Mp, Sp
  MsgMagicResult.Init();
  MsgMagicResult.dwAID        = A_MAGICRESULT;
  MsgMagicResult.dwMsgLen     = 1;
  MsgMagicResult.Msgs[0].Size = 0;

  // About Guild Id
  MsgGuildId.Init();
  MsgGuildId.dwAID        = A_SETGUILDID;
  MsgGuildId.dwMsgLen     = 1;
  MsgGuildId.Msgs[0].Size = sizeof( SNMSetGuildId );
  MsgGuildId.Msgs[1].Size = 0;

  //
  MsgClearPK.Init();
  MsgClearPK.dwAID        = A_CLEARPK;
  MsgClearPK.dwMsgLen     = 1;
  MsgClearPK.Msgs[0].Size = sizeof( DWORD );
  MsgClearPK.Msgs[1].Size = 0;
  
  //
  MsgUpdateMstHp.Init();
  MsgUpdateMstHp.dwAID        = A_UPDATEMONSTERHP;
  MsgUpdateMstHp.dwMsgLen     = 1;

  //
  MsgSwitch.Init();
  MsgSwitch.dwAID             = A_SWITCH;
  MsgSwitch.dwMsgLen          = 1;
  MsgSwitch.Msgs[0].Size      = sizeof( DWORD ) + sizeof( WORD );

#ifdef _DEBUG_MICHAEL_TESSERA_EX_

  MsgAddiDamage.Init();
  MsgAddiDamage.dwAID             = A_ADDIDAMAGE;
  MsgAddiDamage.dwMsgLen          = 1;

  MsgDefeDamage.Init();
  MsgDefeDamage.dwAID             = A_DEFENCEDAMAGE;
  MsgDefeDamage.dwMsgLen          = 1;

#endif

  MsgBossDrop.Init();
  MsgBossDrop.dwAID               = A_BOSSDROPITEM;
  MsgBossDrop.dwMsgLen            = 2;
  MsgBossDrop.Msgs[1].Size        = sizeof( WORD ) + sizeof( WORD );

/////////////////////////////////////////////////////////////////////
//Add by CECE 2004-04-16
#ifdef EVILWEAPON_3_6_VERSION
  MsgEvilWarp.Init();
  MsgEvilWarp.dwAID               = A_PLAYERINFO;
  MsgEvilWarp.dwMsgLen            = 2;
  MsgEvilWarp.Msgs[0].Size        = 1;
  MsgEvilWarp.Msgs[1].Size        = sizeof( SNMNpcInfo );
  //
  MsgEvilSummon.Init();
  MsgEvilSummon.dwAID               = A_USEEVILSUMMON;
  MsgEvilSummon.dwMsgLen            = 1;
  MsgEvilSummon.Msgs[0].Size        = sizeof(SEvilSumoon);;
#endif
///////////////////////////////////////////////////////////////////////

	// Added by nick 
	InitializeCriticalSection(&g_cs_MsgLock);
  
	return true;
}
//===============================================================================================
//
//
bool ReleaseGlobalVar()
{ // Release Global Variable
  delete g_pClientList;

  delete g_pChatroomList;
	delete g_pReloginServer;

  delete g_pMccDB;
	delete g_pMccChat;
  delete g_pGs;
  delete g_pBase;
#ifdef VERSION_40_HOUSE_FUNCTION
  delete g_pHouseMgr;
#endif

  delete g_pShowMessage;

	delete g_pDropItem;

	delete g_pWaprPointList;
	delete g_pMixItemList;

  SAFE_DELETE( g_pGuildMng );
  SAFE_DELETE( g_MercenaryMgr );
#ifdef VERSION_40_HOUSE_FUNCTION
  SAFE_DELETE( g_pPutMonster );
  SAFE_DELETE( g_pBackMusic );
#endif
#ifdef VERSION40_FIELD
  SAFE_DELETE( g_pField );
#endif
#ifdef VERSION40_UNIQUERING
  SAFE_DELETE( g_pRingManager ); 
#endif
  SAFE_DELETE( g_pTopTenMng );
  SAFE_DELETE( g_CityWarTimeMng );
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
	SAFE_DELETE( g_pFerryMng );
#endif

	// Added by nick 
	DeleteCriticalSection(&g_cs_MsgLock);
  
	return true;
}
//===============================================================================================
//
//
bool IntiAllData()
{
  FuncName("IntiAllData");
  AddMemoMsg("Init All Data ...");

	// Init Exp And Reset Attribute Cost Money
	//g_dwExpByLevel[0] = 0;
	// About CSrvBaseSkill Target List -- On Functionarly Functions
	g_BaseTargetList.clear();
  // Init Msg ID;
  g_MsgId.Init(1, 65535);
  // Init Main Data
  //初始化系统参数和分配静态内存
  if(!g_pGs->Init())    // including initialization of Winsock
  {
    AddMemoMsg( "g_pGs->Init() fail #" );
    return false;
  }
  //从指定文件输入游戏数据
  if(!g_pBase->Init())
  {
    AddMemoMsg( "Initialize Server Data Error #" );
    return false;
  }
  //初始化MCC的网路连接
#ifndef _CHECK_SERVER_DATA_FILE_VERSION_
  if(!g_pMccDB->Init(TRUE))
  {
    AddMemoMsg("g_pMccDB->Init() fail #");
    return false;
  }
	if(!g_pMccChat->Init(FALSE))
  {
    AddMemoMsg("g_pMccChat->Init() fail #");
    return false;
  }
#endif
//Add by CECE 2004-07-28
#ifdef ELYSIUM_3_7_VERSION
  if( !g_FightFieldMgr.LoadMatchInfo((char*) string(string(g_pBase->GetMapFilePath())+string(MATCH_INFO_FILE)).c_str() ) )
  {
    AddMemoErrMsg("Init Match Info fail #");
    return false;
  }
  g_CheckItemTable.Load((char*) string( string(g_pBase->GetMapFilePath())+string(MATCH_CHECK_FILE) ).c_str() );
#endif
  AddMemoMsg("Init All Data OK");
  return true;
}
//===============================================================================================
//
//
bool ReleaseAllData()
{
  g_pClientList->ReleaseMaxList();
	
  return true;
}
//===============================================================================================
//
//
CSrvBaseData::CSrvBaseData()
{
	m_mapBaseMap.clear();       // DWORD: Map ID
  m_mapBaseSkill.clear();     // DWORD: Skill ID
  m_mapBaseItem.clear();      // DWORD: Item ID
  m_mapBaseMonster.clear();   // DWORD: Monster ID
  m_mapEventPoint.clear();    // DWORD: (Hi Word) Event ID, (Lo Word) the Index of EventPoint in the Event
	m_mapTask.clear();					// DWORD: Task ID
	m_mapIdMapType.clear();			// DWORD: All Map's Type Info
  m_mapSuitSkill.clear();     // WORD : Suit Equip Id
#ifdef SAVE_TRANS_FUNC
  m_mapSavePointItem.clear();
#endif
//
#ifdef _MUSIC_BOX_
  m_mapMusicBoxItem.clear();
#endif
//
#ifdef _VERSION40_CARRYDOOR
  m_listCarryDoor.clear();
#endif
//
#ifdef FUNCTION_MAKEHOLE_ITEM
  m_mapMixHoleItem.clear(); 
#endif
}
//===============================================================================================
//
//
CSrvBaseData::~CSrvBaseData()
{
  ReleaseBaseMonster();
  ReleaseBaseEvent();
  ReleaseBaseItem();
  ReleaseBaseSkill();
  ReleaseBaseMap();
  ReleaseSuitEquip();
  ////////////////////////
  //Add by CECE 2004-04-05
#ifdef EVILWEAPON_3_6_VERSION
  ReleaseBaseEvilWeapon();
#endif
  ////////////////////////
}
//===============================================================================================
//
//
bool CSrvBaseData::Init()
{
  FuncName("CSrvBaseData::Init");

  CIniFile BfIni(BF_INI_FILE);

  // Read Ini File
  BfIni.ReadString("GameData", "ObjFile", m_szObjFile, MAX_FILE_PATH);
  g_pGs->m_maxPlayerLevel = BfIni.ReadInteger("GameData","MaxPlayerLevel",200) ;
  g_pGs->m_BeBornMap = BfIni.ReadInteger("GameData","bebornmap",10005) ;
  g_pGs->m_BeBornMapX = BfIni.ReadInteger("GameData","bebornmapx",24) ;
  g_pGs->m_BeBornMapY = BfIni.ReadInteger("GameData","bebornmapy",30) ;
  //
#ifndef _DEBUG_NO_ANY_LOG_
  _snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "ObjFile Path: %s", m_szObjFile);
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(szInitLog);
#endif
  //
  BfIni.ReadString("GameData", "MapFile", m_szMapFile, MAX_FILE_PATH);
  //
#ifndef _DEBUG_NO_ANY_LOG_
  _snprintf(szInitLog,MAX_MEMO_MSG_LEN-1,"MapFile Path: %s", m_szMapFile);
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(szInitLog);
#endif
  //

  // Load All Srv Base Data From Files, Must Load Skill Before Load Item
  if( LoadBasePlayer() == false )
  {
		AddMemoErrMsg("***** Server Load Base Player Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Player Error ! *****", "WARNING", MB_OK );
    return false;
  }

  if( LoadBaseMap(MAP_ID_LIST_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Base Map Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Map Error ! *****", "WARNING", MB_OK );
    return false;
  }

  if( LoadBaseSkill(BF_SKILL_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Base Skill Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Skill Error ! *****", "WARNING", MB_OK );
    return false;
  }
  // Load Suit Equip Data
  if( !LoadBaseSuitEquip( BF_SUIT_EUQIP_FILE ) )
  {
    AddMemoErrMsg("***** Server Load Suit Equip Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Suit Equip Error ! *****", "WARNING", MB_OK );
    return false;
  }
  //////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-05
#ifdef EVILWEAPON_3_6_VERSION
  if( LoadBaseEvilWeapon(BF_EVILWEAPON_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Evil Weapon Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Evil Weapon Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
#ifdef SAVE_TRANS_FUNC
  if( LoadSavePointItem(BF_SAVEPOINTITEM_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load SavePoint Item Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load SavePoint Item Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
#ifdef _MUSIC_BOX_
  if( LoadMusicBoxItem(BF_MUSICBOX_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load MusicBox Item Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load MusicBox Item Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
#ifdef _NEW_TRADITIONS_WEDDING_
  if (!LoadRestoreItem(BF_RESTOREITEM_FILE))
  {
		AddMemoErrMsg("***** Server Load Restore Hp Item Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Restore Hp Item file  Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
#ifdef _VERSION40_CARRYDOOR
  if( LoadBaseCarryDoor(BF_CARRYDOOR_FILE) <= 0)
  {
		AddMemoErrMsg("***** Server Load CarryPoint Item Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load CarryPointFile  Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
//////////////////////////////////////////////////////////////
//Add by zetorchen 20050125
#ifdef _COMBINE_RANDOM_
  if( !LoadCombineItemOdds(BF_MIXITEMCOMBINE) )
  {
    m_mapCombineItemOdds.clear();
    AddMemoErrMsg("***** Server Load Combine Item Odds Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Combine Item Odds Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
  //////////////////////////////////////////////////////////////
  if( LoadBaseItem(BF_ITEM_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Base Item Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Item Error ! *****", "WARNING", MB_OK );
    return false;
  }

  if( LoadBaseEvent(BF_EVENT_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Base Event Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Event Error ! *****", "WARNING", MB_OK );
    return false;
  }
#ifdef _NPC_TRANSLUCENCY_
  if( LoadNpcTranslucency(BF_NPCTRANSLUCENCY_FILE) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Base Npc Translucency Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Npc Translucency Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif


  if( LoadMonsterDropItem( BF_MONSTER_DROPITEM_FILE ) <= 0 )
	{
		AddMemoErrMsg("***** Server Load Monster Drop Item File Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Monster Drop Item File Error ! *****", "WARNING", MB_OK );
    return false;		
	}

  if( LoadBaseMonster( BF_BASE_MONSTER_FILE, BF_MONSTER_FILE ) <= 0 )
  {
		AddMemoErrMsg("***** Server Load Base Monster Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base Monster Error ! *****", "WARNING", MB_OK );
    return false;
  }
  //add by Jack.ren for 4.0
#ifdef MARKITEM_40_MARKRULST
  if( LoadBaseMarkItem( BF_MARKITEM_FILE ) <= 0 )
  {
    AddMemoErrMsg("***** Server Load Base MarkItem Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base MarkItem Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif

#ifdef _AUTO_RUN_CITY_WAR_
  if(LoadAutoCityWar(BF_AUTORUN_FILE)<=0)
  {
    AddMemoErrMsg("***** Server Load Base Auto Run City War Error! *****");
    MessageBox(GetActiveWindow(), "***** Server Load Base Auto Run City War Error! *****", "WARNING", MB_OK);
    return false;
  }
#endif

#ifdef _AUTO_ADD_WARP_POINT_
  if(LoadAutoAddWarpPoint(BF_ADD_WARPPOINT)<=0)
  {
    AddMemoErrMsg("***** Server LOad Base Auto Add WarpPoint Error! *****");
    MessageBox(GetActiveWindow(), "***** Server LOad Base Auto Add WarpPoint Error! *****", "WARNING", MB_OK);
    return false;
  }
#endif

#ifdef FUNCTION_MAKEHOLE_ITEM
  if(LoadMixHoleItem( BF_MIXHOLEITEM_FILE ) <= 0) 
  {
    AddMemoErrMsg("***** Server Load Base MixHoleIteem Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Base MixHoleItem Error ! *****", "WARNING", MB_OK );
    return false; 
  }
#endif
 // Load Mercenary Data
  if( !g_MercenaryMgr->Load( (char*)((string(m_szObjFile)+string(BF_MERCENARY_FILE)).c_str()) ) )
  {
    AddMemoErrMsg("***** Server Load Mercenary File Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Mercenary File Error ! *****", "WARNING", MB_OK );
    return false;
  }
#ifdef VERSION_40_HOUSE_FUNCTION
  g_pPutMonster->Load( (char*)((string(m_szObjFile)+string(BF_PUTMONSTER_FILE)).c_str()) );
  g_pBackMusic->Load( (char*)((string(m_szObjFile)+string(BF_BACKMUSIC_FILE)).c_str()) );
#endif
  if( !g_MercenaryMgr->InitBaseMonster() )
  {
    AddMemoErrMsg("***** Server Cannot Init Mercenary ! *****");
    MessageBox( GetActiveWindow(), "***** Server Cannot Init Mercenary ! *****", "WARNING", MB_OK );
    return false;
  }

#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  if( !g_pFerryMng->Load( (char*)((string(m_szObjFile)+string(BF_FERRY_FILE)).c_str()) )  )
  {
    AddMemoErrMsg("***** Server Load Ferry File Error ! *****");
    MessageBox( GetActiveWindow(), "***** Server Load Ferry File Error ! *****", "WARNING", MB_OK );
    return false;
  }
#endif
	if( !CheckGameData() )
  {
    MessageBox( GetActiveWindow(),"***CheckData***error!","WARNING", MB_OK);
    return false;
  }
	//
  return true;
}
//===============================================================================================
//
//
SGameMapType * CSrvBaseData::GetMapType(int iMapId)
{
	map<int,SGameMapType*>::iterator		  Iter_Gt = m_mapIdMapType.find( iMapId );
  //
	if( Iter_Gt != m_mapIdMapType.end() )
	{
		return (SGameMapType*)(Iter_Gt->second);
	}
	return NULL;
}
//===============================================================================================
//
//
bool CSrvBaseData::LoadBasePlayer()
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBasePlayer");
	char			szFileName[256];
	FILE			*pFile = NULL;
	DWORD			dwLevel = 0, dwExp = 0;
  int       i = 0, j = 0;
  // Read Level Exp
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else	
	strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_PLAYER_LEVEL_EXP_FILE );
  CInStream BasePlayer(szFileName);
	g_dwExpByLevel[0] = 0;
  if ( !BasePlayer.fail() && BasePlayer.GetFileSize() != 0 ) 
  {
    for ( i=1; i<=MAX_PLAYER_LEVEL; i++ ) 
    {
      dwLevel = dwExp = 0;
      if( BasePlayer>>dwLevel>>dwExp ) //end-of-file-false
      {
        if( dwLevel == i )		
          g_dwExpByLevel[dwLevel] = dwExp;
        else
        {
          AddMemoErrMsg( "***** Cannot Scan Player Level Exp Table, Because Level Is Not Consecutive! *****" );
          return false;
        }
      }
      else
      {
        AddMemoErrMsg( "***** Cannot Scan Player Level Exp Table ! *****" );
        return false;
      } 
    }
  }
  else
  {
    AddMemoErrMsg( "***** Cannot Open Player Level Exp Table File ! *****" );
    return false;
  }
  //////////////////////////////////////////////////////////////////////////
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else
	strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_PLAYER_NEWCHAR_FILE );
  //
  int         iOccupation = 0;
  CInStream   BaseAttr( szFileName );
  ZeroMemory( g_NewCharAttr, sizeof( g_NewCharAttr ) );
  if ( !BaseAttr.fail() && BaseAttr.GetFileSize() != 0 ) 
  {
    iOccupation = 0;
    for ( i = 0; i < 4; i++ ) 
    {
      if( BaseAttr >> iOccupation
                   >> g_NewCharAttr[i][0]
                   >> g_NewCharAttr[i][1]
                   >> g_NewCharAttr[i][2]
                   >> g_NewCharAttr[i][3]
                   >> g_NewCharAttr[i][4]
                   >> g_NewCharAttr[i][5]
                   >> g_NewCharAttr[i][6]
                   >> g_NewCharAttr[i][7]
                   >> g_NewCharAttr[i][8]
                   >> g_NewCharAttr[i][9]
         )
      {
        if( ( iOccupation != i + 1 ) || g_NewCharAttr[i][0] == 0 || g_NewCharAttr[i][1] == 0 || 
            g_NewCharAttr[i][2] == 0 || g_NewCharAttr[i][3] == 0 || g_NewCharAttr[i][4] == 0 ||
            g_NewCharAttr[i][5] == 0 || g_NewCharAttr[i][6] == 0 || g_NewCharAttr[i][7] == 0 )
        {
          AddMemoErrMsg( "***** The Player Base Attribute Data Error ! *****" );
          return false;
        }
        
      }
     else
      {
        AddMemoErrMsg( "***** Scan Player Base Attribute File Error ! *****" );
        return false;
      }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot Open BaseAttr.txt", "Warning...", MB_OK );
    return false;
  }
  //////////////////////////////////////////////////////////////////////////
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else
	strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_PLAYER_UPDATE_FILE );
  CInStream PlyUpAttr( szFileName );
  ZeroMemory( g_PlayerUpdateAttr, sizeof( g_PlayerUpdateAttr ) );
  if ( !PlyUpAttr.fail() && PlyUpAttr.GetFileSize() != 0 ) 
  {
    for ( i = 0; i < 5; i++ ) 
    {
      if ( PlyUpAttr >> g_PlayerUpdateAttr[0][i][0]
                     >> g_PlayerUpdateAttr[0][i][1]
                     >> g_PlayerUpdateAttr[0][i][2]
                     >> g_PlayerUpdateAttr[1][i][0]
                     >> g_PlayerUpdateAttr[1][i][1]
                     >> g_PlayerUpdateAttr[1][i][2]
                     >> g_PlayerUpdateAttr[2][i][0]
                     >> g_PlayerUpdateAttr[2][i][1]
                     >> g_PlayerUpdateAttr[2][i][2]
                     >> g_PlayerUpdateAttr[3][i][0]
                     >> g_PlayerUpdateAttr[3][i][1]
                     >> g_PlayerUpdateAttr[3][i][2]
        )
      {
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, 
                   "The Attr=%d Update Player Bonu Hp0=%d Mp0=%d Sp0=%d \
                   Hp1=%d Mp1=%d Sp1=%d \
                   Hp2=%d Mp2=%d Sp2=%d \
                   Hp3=%d Mp3=%d Sp3=%d #",
                   i, g_PlayerUpdateAttr[0][i][0], g_PlayerUpdateAttr[0][i][1], g_PlayerUpdateAttr[0][i][2],
                   g_PlayerUpdateAttr[1][i][0], g_PlayerUpdateAttr[1][i][1], g_PlayerUpdateAttr[1][i][2],
                   g_PlayerUpdateAttr[2][i][0], g_PlayerUpdateAttr[2][i][1], g_PlayerUpdateAttr[2][i][2],
                   g_PlayerUpdateAttr[3][i][0], g_PlayerUpdateAttr[3][i][1], g_PlayerUpdateAttr[3][i][2] 
                  );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoMsg( szInitLog );
     }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot OPen PlyUpAttr.txt", "Warning...", MB_OK );
    return false;
  }
  //////////////////////////////////////////////////////////////////////////
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256  );
#else
  strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_SYSTEM_UPDATE_FILE );
  CInStream           SysUpAttr( szFileName );
  ZeroMemory( g_SystemUpdateAttr, sizeof( g_SystemUpdateAttr ) );
  if ( !SysUpAttr.fail() && SysUpAttr.GetFileSize() != 0 ) 
  {
    int       iLevel = 0;
    for( i = 0; i < MAX_PLAYER_LEVEL; i++ )
    {
      SysUpAttr >> iLevel;
      SysUpAttr >> g_SystemUpdateAttr[0][i][0];
      SysUpAttr >> g_SystemUpdateAttr[0][i][1];
      SysUpAttr >> g_SystemUpdateAttr[0][i][2];
      SysUpAttr >> g_SystemUpdateAttr[0][i][3];
      SysUpAttr >> g_SystemUpdateAttr[0][i][4];
      SysUpAttr >> g_SystemUpdateAttr[0][i][5];
      SysUpAttr >> g_SystemUpdateAttr[0][i][6];
      SysUpAttr >> g_SystemUpdateAttr[0][i][7];
      SysUpAttr >> g_SystemUpdateAttr[0][i][8];
      SysUpAttr >> g_SystemUpdateAttr[1][i][0];
      SysUpAttr >> g_SystemUpdateAttr[1][i][1];
      SysUpAttr >> g_SystemUpdateAttr[1][i][2];
      SysUpAttr >> g_SystemUpdateAttr[1][i][3];
      SysUpAttr >> g_SystemUpdateAttr[1][i][4];
      SysUpAttr >> g_SystemUpdateAttr[1][i][5];
      SysUpAttr >> g_SystemUpdateAttr[1][i][6];
      SysUpAttr >> g_SystemUpdateAttr[1][i][7];
      SysUpAttr >> g_SystemUpdateAttr[1][i][8];
      SysUpAttr >> g_SystemUpdateAttr[2][i][0];
      SysUpAttr >> g_SystemUpdateAttr[2][i][1];
      SysUpAttr >> g_SystemUpdateAttr[2][i][2];
      SysUpAttr >> g_SystemUpdateAttr[2][i][3];
      SysUpAttr >> g_SystemUpdateAttr[2][i][4];
      SysUpAttr >> g_SystemUpdateAttr[2][i][5];
      SysUpAttr >> g_SystemUpdateAttr[2][i][6];
      SysUpAttr >> g_SystemUpdateAttr[2][i][7];
      SysUpAttr >> g_SystemUpdateAttr[2][i][8];
      SysUpAttr >> g_SystemUpdateAttr[3][i][0];
      SysUpAttr >> g_SystemUpdateAttr[3][i][1];
      SysUpAttr >> g_SystemUpdateAttr[3][i][2];
      SysUpAttr >> g_SystemUpdateAttr[3][i][3];
      SysUpAttr >> g_SystemUpdateAttr[3][i][4];
      SysUpAttr >> g_SystemUpdateAttr[3][i][5];
      SysUpAttr >> g_SystemUpdateAttr[3][i][6];
      SysUpAttr >> g_SystemUpdateAttr[3][i][7];
      SysUpAttr >> g_SystemUpdateAttr[3][i][8];
      //
       if( iLevel != 1 && i > 0 )
       {
         for( j = 0; j < 4; j++ )
         {
           g_SystemUpdateAttr[j][i][9]  = g_SystemUpdateAttr[j][i-1][9]  + g_SystemUpdateAttr[j][i][0];
           g_SystemUpdateAttr[j][i][10] = g_SystemUpdateAttr[j][i-1][10] + g_SystemUpdateAttr[j][i][1];
           g_SystemUpdateAttr[j][i][11] = g_SystemUpdateAttr[j][i-1][11] + g_SystemUpdateAttr[j][i][2];
           g_SystemUpdateAttr[j][i][12] = g_SystemUpdateAttr[j][i-1][12] + g_SystemUpdateAttr[j][i][3];
           g_SystemUpdateAttr[j][i][13] = g_SystemUpdateAttr[j][i-1][13] + g_SystemUpdateAttr[j][i][4];
           g_SystemUpdateAttr[j][i][14] = g_SystemUpdateAttr[j][i-1][14] + g_SystemUpdateAttr[j][i][5];// +
           g_SystemUpdateAttr[j][i][15] = g_SystemUpdateAttr[j][i-1][15] + g_SystemUpdateAttr[j][i][6];// +
           g_SystemUpdateAttr[j][i][16] = g_SystemUpdateAttr[j][i-1][16] + g_SystemUpdateAttr[j][i][7];// +
         }
       }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot OPen SysUpAttr.txt", "Warning...", MB_OK );
    return false;
  }
  //////////////////////////////////////////////////////////////////////////
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else
  strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_ADD_LIMIT_FILE );
  CInStream           AddLimit( szFileName );
  ZeroMemory( g_OccupAddLimit, sizeof( g_OccupAddLimit ) );
  if ( !AddLimit.fail() && AddLimit.GetFileSize() != 0 ) 
  {
    int     iLevel2 = 0;
    for( i = 0; i < MAX_PLAYER_LEVEL; i++ )
    {
      AddLimit >> iLevel2;
      AddLimit >> g_OccupAddLimit[0][i][0];
      AddLimit >> g_OccupAddLimit[0][i][1];
      AddLimit >> g_OccupAddLimit[0][i][2];
      AddLimit >> g_OccupAddLimit[0][i][3];
      AddLimit >> g_OccupAddLimit[0][i][4];
      AddLimit >> g_OccupAddLimit[1][i][0];
      AddLimit >> g_OccupAddLimit[1][i][1];
      AddLimit >> g_OccupAddLimit[1][i][2];
      AddLimit >> g_OccupAddLimit[1][i][3];
      AddLimit >> g_OccupAddLimit[1][i][4];
      AddLimit >> g_OccupAddLimit[2][i][0];
      AddLimit >> g_OccupAddLimit[2][i][1];
      AddLimit >> g_OccupAddLimit[2][i][2];
      AddLimit >> g_OccupAddLimit[2][i][3];
      AddLimit >> g_OccupAddLimit[2][i][4];
      AddLimit >> g_OccupAddLimit[3][i][0];
      AddLimit >> g_OccupAddLimit[3][i][1];
      AddLimit >> g_OccupAddLimit[3][i][2];
      AddLimit >> g_OccupAddLimit[3][i][3];
      AddLimit >> g_OccupAddLimit[3][i][4];
      //
      if( iLevel2 != i + 1 )
      {
        AddMemoErrMsg( "***** The Add Update Point Limit Table Is Not Consecutive ! *****" );
        return false;
      }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot OPen AddLimit.txt", "Warning...", MB_OK );
    return false;
  }
  return true;
#else
//
  FuncName("CSrvBaseData::LoadBasePlayer");

	char			szFileName[256];
	FILE			*pFile = NULL;
	DWORD			dwLevel = 0, dwExp = 0;
  int       i = 0, j = 0;
  // Read Level Exp
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else	
	strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_PLAYER_LEVEL_EXP_FILE );
  //
	g_dwExpByLevel[0] = 0;
	if( NULL != ( pFile = fopen( szFileName, "r" ) ) )
	{
		for( i = 1; i <= MAX_PLAYER_LEVEL; i++ )
		{
      dwLevel = dwExp = 0;
			if( 2 == fscanf( pFile, "%d %d ", &dwLevel, &dwExp ) )
			{
				if( dwLevel == i )		g_dwExpByLevel[dwLevel] = dwExp;
				else
				{
					AddMemoErrMsg( "***** Cannot Scan Player Level Exp Table, Because Level Is Not Consecutive! *****" );
					fclose( pFile );
					return false;
				}
			}
			else
			{
				AddMemoErrMsg( "***** Cannot Scan Player Level Exp Table ! *****" );
				fclose( pFile );
				return false;
			}
		}
		fclose( pFile );
    pFile = NULL;
	}
	else
	{
		AddMemoErrMsg( "***** Cannot Open Player Level Exp Table File ! *****" );
		return false;
	}
  // Read New Char Base Attribute
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else
	strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_PLAYER_NEWCHAR_FILE );
  //
  int     iOccupation = 0;
  //
  ZeroMemory( g_NewCharAttr, sizeof( g_NewCharAttr ) );
	if( NULL != ( pFile = fopen( szFileName, "r" ) ) )
	{
    iOccupation = 0;
    //
		for( i = 0; i < 4; i++ )
    {
      if( 11 == fscanf( pFile, "%d %d %d %d %d %d %d %d %d %d %d",
                        &iOccupation,
                        &(g_NewCharAttr[i][0]), &(g_NewCharAttr[i][1]), &(g_NewCharAttr[i][2]),
                        &(g_NewCharAttr[i][3]), &(g_NewCharAttr[i][4]), &(g_NewCharAttr[i][5]),
                        &(g_NewCharAttr[i][6]), &(g_NewCharAttr[i][7]), &(g_NewCharAttr[i][8]),
                        &(g_NewCharAttr[i][9]) ) )
      {
        if( ( iOccupation != i + 1 ) || g_NewCharAttr[i][0] == 0 || g_NewCharAttr[i][1] == 0 || g_NewCharAttr[i][2] == 0 ||
            g_NewCharAttr[i][3] == 0 || g_NewCharAttr[i][4] == 0 || g_NewCharAttr[i][5] == 0 ||
            g_NewCharAttr[i][6] == 0 || g_NewCharAttr[i][7] == 0 )
        {
          fclose( pFile );
          AddMemoErrMsg( "***** The Player Base Attribute Data Error ! *****" );
		      return false;
        }
      }
      else
      {
        fclose( pFile );
        AddMemoErrMsg( "***** Scan Player Base Attribute File Error ! *****" );
        return false;
      }
    }
		fclose( pFile );
    pFile = NULL;
	}
	else
	{
		AddMemoErrMsg( "***** Cannot Open Player Base Attribute Table File ! *****" );
		return false;
	}
  // Read Update Player Assign Attribute
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else
	strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_PLAYER_UPDATE_FILE );
  //
  ZeroMemory( g_PlayerUpdateAttr, sizeof( g_PlayerUpdateAttr ) );
  if( NULL != ( pFile = fopen( szFileName, "r" ) ) )
  {
    //
    for( i = 0; i < 5; i++ )
    {
      if( 12 == fscanf( pFile, "%d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d ",
                        &(g_PlayerUpdateAttr[0][i][0]), &(g_PlayerUpdateAttr[0][i][1]), &(g_PlayerUpdateAttr[0][i][2]),
                        &(g_PlayerUpdateAttr[1][i][0]), &(g_PlayerUpdateAttr[1][i][1]), &(g_PlayerUpdateAttr[1][i][2]),
                        &(g_PlayerUpdateAttr[2][i][0]), &(g_PlayerUpdateAttr[2][i][1]), &(g_PlayerUpdateAttr[2][i][2]),
                        &(g_PlayerUpdateAttr[3][i][0]), &(g_PlayerUpdateAttr[3][i][1]), &(g_PlayerUpdateAttr[3][i][2])
        ) )
      {
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "The Attr=%d Update Player Bonu Hp0=%d Mp0=%d Sp0=%d \
                                                         Hp1=%d Mp1=%d Sp1=%d \
                                                         Hp2=%d Mp2=%d Sp2=%d \
                                                         Hp3=%d Mp3=%d Sp3=%d #",
                         i, g_PlayerUpdateAttr[0][i][0], g_PlayerUpdateAttr[0][i][1], g_PlayerUpdateAttr[0][i][2],
                            g_PlayerUpdateAttr[1][i][0], g_PlayerUpdateAttr[1][i][1], g_PlayerUpdateAttr[1][i][2],
                            g_PlayerUpdateAttr[2][i][0], g_PlayerUpdateAttr[2][i][1], g_PlayerUpdateAttr[2][i][2],
                            g_PlayerUpdateAttr[3][i][0], g_PlayerUpdateAttr[3][i][1], g_PlayerUpdateAttr[3][i][2] );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoMsg( szInitLog );
      }
      else
      {
        fclose( pFile );
		    AddMemoErrMsg( "***** Cannot Scan Player Assign Attribute Table File ! *****" );
		    return false;
      }
    }
    fclose( pFile );
    pFile = NULL;
  }
  else
  {
		AddMemoErrMsg( "***** Cannot Open Player Assign Attribute Table File ! *****" );
		return false;
  }
  // Read Update System Assign Attribute
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256  );
#else
  strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_SYSTEM_UPDATE_FILE );
  //
  ZeroMemory( g_SystemUpdateAttr, sizeof( g_SystemUpdateAttr ) );
  //
  if( NULL != ( pFile = fopen( szFileName, "r" ) ) )
  {
    int       iLevel = 0;
    for( i = 0; i < MAX_PLAYER_LEVEL; i++ )
    {
      if( 37 == fscanf( pFile, "%d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d",
          &iLevel,
          &g_SystemUpdateAttr[0][i][0], &g_SystemUpdateAttr[0][i][1], &g_SystemUpdateAttr[0][i][2],  &g_SystemUpdateAttr[0][i][3],
          &g_SystemUpdateAttr[0][i][4], &g_SystemUpdateAttr[0][i][5], &g_SystemUpdateAttr[0][i][6],  &g_SystemUpdateAttr[0][i][7], &g_SystemUpdateAttr[0][i][8],
          &g_SystemUpdateAttr[1][i][0], &g_SystemUpdateAttr[1][i][1], &g_SystemUpdateAttr[1][i][2],  &g_SystemUpdateAttr[1][i][3],
          &g_SystemUpdateAttr[1][i][4], &g_SystemUpdateAttr[1][i][5], &g_SystemUpdateAttr[1][i][6],  &g_SystemUpdateAttr[1][i][7], &g_SystemUpdateAttr[1][i][8],
          &g_SystemUpdateAttr[2][i][0], &g_SystemUpdateAttr[2][i][1], &g_SystemUpdateAttr[2][i][2],  &g_SystemUpdateAttr[2][i][3],
          &g_SystemUpdateAttr[2][i][4], &g_SystemUpdateAttr[2][i][5], &g_SystemUpdateAttr[2][i][6],  &g_SystemUpdateAttr[2][i][7], &g_SystemUpdateAttr[2][i][8],
          &g_SystemUpdateAttr[3][i][0], &g_SystemUpdateAttr[3][i][1], &g_SystemUpdateAttr[3][i][2],  &g_SystemUpdateAttr[3][i][3],
          &g_SystemUpdateAttr[3][i][4], &g_SystemUpdateAttr[3][i][5], &g_SystemUpdateAttr[3][i][6],  &g_SystemUpdateAttr[3][i][7], &g_SystemUpdateAttr[3][i][8]
         ) )
      {
        if( iLevel != 1 && i > 0 )
        {
          for( j = 0; j < 4; j++ )
          {
            g_SystemUpdateAttr[j][i][9]  = g_SystemUpdateAttr[j][i-1][9]  + g_SystemUpdateAttr[j][i][0];
            g_SystemUpdateAttr[j][i][10] = g_SystemUpdateAttr[j][i-1][10] + g_SystemUpdateAttr[j][i][1];
            g_SystemUpdateAttr[j][i][11] = g_SystemUpdateAttr[j][i-1][11] + g_SystemUpdateAttr[j][i][2];
            g_SystemUpdateAttr[j][i][12] = g_SystemUpdateAttr[j][i-1][12] + g_SystemUpdateAttr[j][i][3];
            g_SystemUpdateAttr[j][i][13] = g_SystemUpdateAttr[j][i-1][13] + g_SystemUpdateAttr[j][i][4];
            //
            g_SystemUpdateAttr[j][i][14] = g_SystemUpdateAttr[j][i-1][14] + g_SystemUpdateAttr[j][i][5];// +
                                           //g_PlayerUpdateAttr[j][0][0] * g_SystemUpdateAttr[j][i][0] +
                                           //g_PlayerUpdateAttr[j][1][0] * g_SystemUpdateAttr[j][i][1] +
                                           //g_PlayerUpdateAttr[j][2][0] * g_SystemUpdateAttr[j][i][2] +
                                           //g_PlayerUpdateAttr[j][3][0] * g_SystemUpdateAttr[j][i][3] +
                                           //g_PlayerUpdateAttr[j][4][0] * g_SystemUpdateAttr[j][i][4];

            g_SystemUpdateAttr[j][i][15] = g_SystemUpdateAttr[j][i-1][15] + g_SystemUpdateAttr[j][i][6];// +
                                           //g_PlayerUpdateAttr[j][0][1] * g_SystemUpdateAttr[j][i][0] +
                                           //g_PlayerUpdateAttr[j][1][1] * g_SystemUpdateAttr[j][i][1] +
                                           //g_PlayerUpdateAttr[j][2][1] * g_SystemUpdateAttr[j][i][2] +
                                           //g_PlayerUpdateAttr[j][3][1] * g_SystemUpdateAttr[j][i][3] +
                                           //g_PlayerUpdateAttr[j][4][1] * g_SystemUpdateAttr[j][i][4];

            g_SystemUpdateAttr[j][i][16] = g_SystemUpdateAttr[j][i-1][16] + g_SystemUpdateAttr[j][i][7];// +
                                           //g_PlayerUpdateAttr[j][0][2] * g_SystemUpdateAttr[j][i][0] +
                                           //g_PlayerUpdateAttr[j][1][2] * g_SystemUpdateAttr[j][i][1] +
                                           //g_PlayerUpdateAttr[j][2][2] * g_SystemUpdateAttr[j][i][2] +
                                           //g_PlayerUpdateAttr[j][3][2] * g_SystemUpdateAttr[j][i][3] +
                                           //g_PlayerUpdateAttr[j][4][2] * g_SystemUpdateAttr[j][i][4];
          }
        }
      }
      else
      {
        fclose( pFile );
        AddMemoErrMsg( "***** Cannot Scan System Assign Occupation Jian Attribute Table File ! *****" );
		    return false;
      }
    }
  }
  else
  {
		AddMemoErrMsg( "***** Cannot Open System Assign Occupation Jian Attribute Table File ! *****" );
		return false;
  }
  // Load Base Add Limit
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szFileName, m_szObjFile, 256 );
#else
  strcpy( szFileName, m_szObjFile );
#endif
	strcat( szFileName, "/" );
	strcat( szFileName, BF_ADD_LIMIT_FILE );
  //
  ZeroMemory( g_OccupAddLimit, sizeof( g_OccupAddLimit ) );
  //
  if( NULL != ( pFile = fopen( szFileName, "r" ) ) )
  {
    int     iLevel2 = 0;
    for( i = 0; i < MAX_PLAYER_LEVEL; i++ )
    {
      if( 21 == fscanf( pFile, "%d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d %d %d %d %d \
                                %d ",
                                &iLevel2,
                                &g_OccupAddLimit[0][i][0], &g_OccupAddLimit[0][i][1], &g_OccupAddLimit[0][i][2],&g_OccupAddLimit[0][i][3], &g_OccupAddLimit[0][i][4],
                                &g_OccupAddLimit[1][i][0], &g_OccupAddLimit[1][i][1], &g_OccupAddLimit[1][i][2],&g_OccupAddLimit[1][i][3], &g_OccupAddLimit[1][i][4],
                                &g_OccupAddLimit[2][i][0], &g_OccupAddLimit[2][i][1], &g_OccupAddLimit[2][i][2],&g_OccupAddLimit[2][i][3], &g_OccupAddLimit[2][i][4],
                                &g_OccupAddLimit[3][i][0], &g_OccupAddLimit[3][i][1], &g_OccupAddLimit[3][i][2],&g_OccupAddLimit[3][i][3], &g_OccupAddLimit[3][i][4]
                                ) )
      {
        if( iLevel2 != i + 1 )
        {
          fclose( pFile );
          AddMemoErrMsg( "***** The Add Update Point Limit Table Is Not Consecutive ! *****" );
          return false;
        }
      }
      else
      {
        fclose( pFile );
        AddMemoErrMsg( "***** Cannot Scan Add Update Point Limit Table File ! *****" );
        return false;
      }
    }
    fclose( pFile );
    pFile = NULL;
  }
  else
  {
    AddMemoErrMsg( "***** Cannot Open Add Update Point Limit Table File ! *****" );
		return false;
  }
  //if( !g_CityWarTimeMng->LoadCityWarTime() )    return false;
  return true;
#endif //_DEBUG_JAPAN_DECRYPT_
}
//===============================================================================================
//
//
int CSrvBaseData::LoadBaseMap(char *szIdListFilePath)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseMap");
  CSrvBaseMap* pNewBaseMap = NULL;
  string              strFilePath = "";
  string              strTemp = "";
  map<DWORD, string>  mapMapIdList;
  int                 iNum = 0, iLoop = 0;
//////////////////////////////////////////////////////////////////////////
  {
		int							iCount = 0, iMapId = 0, iTemp[11], iReviveData[6]; 
    DWORD           dwRandom;
		SGameMapType		*pMapType = NULL;
	//	strFilePath = string(m_szMapFile);
	//	strFilePath += "/MapType.txt";
  char szInitLog[MAX_MEMO_MSG_LEN];
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szMapFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szMapFile );
#endif
#ifdef REPLACE_STRCAT
  SafeStrcat( szInitLog, "/", MAX_MEMO_MSG_LEN); 
  SafeStrcat( szInitLog, "/MapType.txt", MAX_MEMO_MSG_LEN); 
#else
  strcat( szInitLog, "/" );
  strcat( szInitLog, "/MapType.txt" );
#endif

    //
    CInStream     MapType(szInitLog);
		if( MapType.fail() || MapType.GetFileSize() == 0 )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Cannot Find The Map Type File: %s", strFilePath.c_str() );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return false;
		}

    MapType >> iCount;
		for( iLoop = 0; iLoop < iCount; iLoop++ )
		{
      memset( iReviveData, 0, sizeof( int ) * 6 );
      memset( iTemp, 0, sizeof( int ) * 9 );
      iMapId = dwRandom = 0;
			if( MapType >> iMapId >> iTemp[0] >> iTemp[1] >> iTemp[2]
                  >> iTemp[3] >> iTemp[4] >> iTemp[5] >> dwRandom
                  >> iReviveData[0] >> iReviveData[1] >> iReviveData[2]
                  >> iReviveData[3] >> iReviveData[4] >> iReviveData[5]
                  >> iTemp[6] >> iTemp[7] >> iTemp[8] >> iTemp[9] >> iTemp[10] )          
			{
				pMapType                    = new SGameMapType;
				pMapType->iMapId            = iMapId;
        pMapType->dwWeatherRandom   = dwRandom;
				pMapType->dwType            = iTemp[0] | iTemp[1] | iTemp[2] | iTemp[3] | iTemp[4] | iTemp[5]; 
				pMapType->wAttribute        = iTemp[6];
        pMapType->wReviveMap        = iReviveData[0];     
        pMapType->wReviveX          = iReviveData[1];
        pMapType->wReviveY          = iReviveData[2];
        //
        pMapType->wDefenceReviveMap = iReviveData[3];
        pMapType->wDefenceReviveX   = iReviveData[4];
        pMapType->wDefenceReviveY   = iReviveData[5];
        pMapType->wMaxMercenary     = iTemp[7];
				pMapType->wDescribe         = iTemp[8];   // michael added for B031120-1
        ///////////////////////////////////////
        pMapType->wColor            = iTemp[9];
        ///////////////////////////////////////
        pMapType->wLimit            = iTemp[10];
        //
        m_mapIdMapType.insert( map<int,SGameMapType*>::value_type( iMapId, pMapType ) );
			}
			else
			{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Scan Error The Map Type File(%d): %s", iLoop, strFilePath.c_str() );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg( szInitLog );
#endif
				return false;
			}
		}
	}
#else
//
  FuncName("CSrvBaseData::LoadBaseMap");

  CSrvBaseMap* pNewBaseMap = NULL;

  string              strFilePath = "";
  string              strTemp = "";
  map<DWORD, string>  mapMapIdList;
  int                 iNum = 0, iLoop = 0;
  //
	{
		FILE						*pFile = NULL;

		int							iCount = 0, iMapId = 0, iTemp[11], iReviveData[6]; 
    DWORD           dwRandom;
		SGameMapType		*pMapType = NULL;
		// Load All Map Type Info
		strFilePath = string(m_szMapFile);
		strFilePath += "/MapType.txt";
		pFile = fopen(strFilePath.c_str(), "r");
		if( !pFile )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Cannot Find The Map Type File: %s", strFilePath.c_str() );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return false;
		}
		fscanf( pFile, "%d ", &iCount );
		for( iLoop = 0; iLoop < iCount; iLoop++ )
		{
      memset( iReviveData, 0, sizeof( int ) * 6 );
      memset( iTemp, 0, sizeof( int ) * 9 );
      iMapId = dwRandom = 0;
			if( 19 == fscanf( pFile, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                               &iMapId, &iTemp[0], &iTemp[1], &iTemp[2],
                               &iTemp[3], &iTemp[4], &iTemp[5], &dwRandom,
                               &iReviveData[0], &iReviveData[1], &iReviveData[2], 
                               &iReviveData[3], &iReviveData[4], &iReviveData[5],
                               &iTemp[6], &iTemp[7], &iTemp[8], &iTemp[9], &iTemp[10] ) )         
			{
				pMapType                    = new SGameMapType;
				pMapType->iMapId            = iMapId;
        pMapType->dwWeatherRandom   = dwRandom;
				pMapType->dwType            = iTemp[0] | iTemp[1] | iTemp[2] | iTemp[3] | iTemp[4] | iTemp[5]; 
				pMapType->wAttribute        = iTemp[6];
        pMapType->wReviveMap        = iReviveData[0];     
        pMapType->wReviveX          = iReviveData[1];
        pMapType->wReviveY          = iReviveData[2];
        //
        pMapType->wDefenceReviveMap = iReviveData[3];
        pMapType->wDefenceReviveX   = iReviveData[4];
        pMapType->wDefenceReviveY   = iReviveData[5];
        pMapType->wMaxMercenary     = iTemp[7];
				pMapType->wDescribe         = iTemp[8];   // michael added for B031120-1
        ///////////////////////////////////////
        pMapType->wColor            = iTemp[9];
        ///////////////////////////////////////
        pMapType->wLimit            = iTemp[10];
        //
        m_mapIdMapType.insert( map<int,SGameMapType*>::value_type( iMapId, pMapType ) );
			}
			else
			{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Scan Error The Map Type File(%d): %s", iLoop, strFilePath.c_str() );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg( szInitLog );
#endif
				return false;
			}
		}
		fclose( pFile );
	}
#endif //_DEBUG_JAPAN_DECRYPT_

  // Load ID-FileName List
  strFilePath  = string( m_szMapFile );
  strFilePath += "/";
  strFilePath += string( szIdListFilePath );

  ifstream  FileIn( strFilePath.c_str(), ios::in|ios::binary );
  char      szBuffer[MAX_READFILE_BUFFER];
  DWORD     dwTheMapId = 0;

  memset( szBuffer, 0, MAX_READFILE_BUFFER );

  if( !FileIn )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Cannot Find The Map List File: %s", strFilePath.c_str() );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szInitLog );
#endif
    return false;
  }

  while( 1 )
  {
    FileIn >> dwTheMapId;
    if( !FileIn )
    {
      break;
    }
    FileIn.getline( szInitLog, MAX_READFILE_BUFFER );
    if( EOF == sscanf( szInitLog, "%s", szBuffer ) )
    {
      break;
    }
    mapMapIdList.insert(map<DWORD, string>::value_type( dwTheMapId, szBuffer ) );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Read Mps Id File [%d - %s] OK", dwTheMapId, szBuffer );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
  }
	
  FileIn.close();
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Load Map List File [%s] OK", strFilePath.c_str() );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
	
  // Create All Base Map
  strTemp  = string( m_szMapFile );
  strTemp += "/";
  iNum     = 0;
  for( map<DWORD, string>::iterator iter = mapMapIdList.begin(); iter != mapMapIdList.end(); iter++)
  {
    pNewBaseMap  = new CSrvBaseMap;
    strFilePath  = strTemp;
    strFilePath += iter->second;
    if( !pNewBaseMap->Init( strFilePath.c_str(), iter->first ) )
    {
      SAFE_DELETE( pNewBaseMap );
      // Release the All Base Map data
      //...
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "Init Base Map Failed : %s", strFilePath.c_str());
      szInitLog[MAX_MEMO_MSG_LEN-1] ='\0';
			AddMemoErrMsg(szInitLog);
#endif
      return FALSE;
    }
    m_mapBaseMap.insert( map<DWORD, CSrvBaseMap*>::value_type( pNewBaseMap->GetMapId(), pNewBaseMap ) );
    iNum++;
  }
  return iNum;
}
//===============================================================================================
//
//
int CSrvBaseData::LoadBaseSkill(char *szFilePath)
{

#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseSkill");
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
#ifdef REPLACE_STRCAT
  SafeStrcat( szInitLog, "/", MAX_MEMO_MSG_LEN); 
  SafeStrcat( szInitLog, szFilePath, MAX_MEMO_MSG_LEN); 
#else
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
#endif

  CInStream         BaseSkill( szInitLog );
  int               iCount = 0, iTemCount = 0;    
  CSrvBaseSkill		  *pBaseSkill = NULL;
  char              szSkillName[1024], szSkillCon[1024];
  DWORD             dwSpecialTemp[2];  //dwSpecialTemp[0]:low DWORD, dwSpecialTemp[1]:high DWORD
	int               dwSkillData[66];
  if ( !BaseSkill.fail() && BaseSkill.GetFileSize() !=0 ) 
  {
    BaseSkill>>iCount;
    if ( iCount == 1 || iCount == 0 ) 
    {
      MessageBox(GetActiveWindow(), "Load Bace Skill iCount Error ", "Warning...", MB_OK);
      return false;
    }
    for( iTemCount = 0; iTemCount<iCount; iTemCount++ )
    {
      pBaseSkill = new CSrvBaseSkill;
      assert( pBaseSkill );
      // Init All Skill Data
      ZeroMemory( dwSkillData, sizeof( dwSkillData ) );
      BaseSkill >> dwSkillData[0] >> szSkillName >> szSkillCon >> dwSkillData[1] >> dwSkillData[2];
      BaseSkill >> dwSkillData[3] >> dwSkillData[4] >> dwSkillData[5] >> dwSkillData[6] >> dwSkillData[7];
      BaseSkill >> dwSkillData[8] >> dwSkillData[9] >> dwSkillData[10] >> dwSkillData[11] >> dwSkillData[12];
      BaseSkill >> dwSkillData[13] >> dwSkillData[14] >> dwSkillData[15] >> dwSkillData[16] >> dwSkillData[17];
      BaseSkill >> dwSkillData[18] >> dwSkillData[19] >> dwSkillData[20] >> dwSkillData[21] >> dwSkillData[22];
      BaseSkill >> dwSkillData[23] >> dwSkillData[24] >> dwSkillData[25] >> dwSkillData[26] >> dwSkillData[27];
      BaseSkill >> dwSkillData[28] >> dwSkillData[29] >> dwSkillData[30] >> dwSkillData[31] >> dwSkillData[32];
      BaseSkill >> dwSkillData[33] >> dwSkillData[34] >> dwSkillData[35] >> dwSkillData[36] >> dwSkillData[37];
      BaseSkill >> dwSpecialTemp[1] >> dwSpecialTemp[0] >> dwSkillData[39] >> dwSkillData[40] >> dwSkillData[41] >> dwSkillData[42];
      BaseSkill >> dwSkillData[43] >> dwSkillData[44] >> dwSkillData[45] >> dwSkillData[46] >> dwSkillData[47];
      BaseSkill >> dwSkillData[48] >> dwSkillData[49] >> dwSkillData[50] >> dwSkillData[51] >> dwSkillData[52];
      BaseSkill >> dwSkillData[53] >> dwSkillData[54] >> dwSkillData[55] >> dwSkillData[56] >> dwSkillData[57];
      BaseSkill >> dwSkillData[58] >> dwSkillData[59] >> dwSkillData[60] >> dwSkillData[61] >> dwSkillData[62];
      BaseSkill >> dwSkillData[63] >> dwSkillData[64] >> dwSkillData[65] >> szInitLog; 
      // Init All Skill Data 
      memcpy( pBaseSkill->m_szName, szSkillName, MAX_SKILL_NAME_LEN );
      pBaseSkill->m_szName[MAX_SKILL_NAME_LEN-1] = '\0';
      pBaseSkill->m_wId               = dwSkillData[0];
      pBaseSkill->m_wIconId           = dwSkillData[1];
      pBaseSkill->m_wPracticeMax      = dwSkillData[2];
      pBaseSkill->m_wAdvanceSkillId   = dwSkillData[3];
      pBaseSkill->m_wType             = dwSkillData[4];
      pBaseSkill->m_wElement          = dwSkillData[5];
      pBaseSkill->m_dwRaceAttri       = dwSkillData[6];
      pBaseSkill->m_wRaceBonuRate     = dwSkillData[7];
      pBaseSkill->m_dwBossCode        = dwSkillData[8];
      pBaseSkill->m_wBossBonuRate     = dwSkillData[9];
      pBaseSkill->m_wOwnAttrEffect    = dwSkillData[10];
      pBaseSkill->m_wOwnAttrBonuRate  = dwSkillData[11];
      pBaseSkill->m_iEleEffect        = dwSkillData[12];
      pBaseSkill->m_iCostHp           = dwSkillData[13];		
      pBaseSkill->m_iCostHpPer        = dwSkillData[14];
      pBaseSkill->m_iCostMp           = dwSkillData[15]; 
      pBaseSkill->m_iCostMpPer        = dwSkillData[16];
      pBaseSkill->m_iCostSp           = dwSkillData[17]; 
      pBaseSkill->m_iCostSpPer        = dwSkillData[18];
      pBaseSkill->m_iCostSoul         = dwSkillData[19];
      pBaseSkill->m_bBlock            = dwSkillData[20];
      pBaseSkill->m_iAlarmTime        = dwSkillData[21];
      pBaseSkill->m_wAlarmSkillId     = dwSkillData[22];
      pBaseSkill->m_iTriggerTime      = dwSkillData[23];
      pBaseSkill->m_wTriggerSkillId   = dwSkillData[24];
      pBaseSkill->m_iHpChange         = dwSkillData[25];
      pBaseSkill->m_iMpChange         = dwSkillData[26];
      pBaseSkill->m_iSpChange         = dwSkillData[27];
      pBaseSkill->m_iSoulChange       = dwSkillData[28];
      pBaseSkill->m_iApChange         = dwSkillData[29];
      pBaseSkill->m_iHitChange        = dwSkillData[30];
      pBaseSkill->m_iDpChange         = dwSkillData[31];
      pBaseSkill->m_iDgChange         = dwSkillData[32];
      pBaseSkill->m_iIntChange        = dwSkillData[33];
      pBaseSkill->m_iBearPsbChange    = dwSkillData[34];
      pBaseSkill->m_iRandom           = dwSkillData[35];
      pBaseSkill->m_iCriticalHit      = dwSkillData[36];
      pBaseSkill->m_iHard             = dwSkillData[37];
      pBaseSkill->m_qwStatus          = MAKEQWORD(dwSpecialTemp[0],dwSpecialTemp[1]);
      pBaseSkill->m_wSkillLearnId     = dwSkillData[39];
      pBaseSkill->m_bPosition         = dwSkillData[40];
      pBaseSkill->m_bColor            = dwSkillData[41];
      pBaseSkill->m_bCase             = dwSkillData[42];
      pBaseSkill->m_wWarpMapId        = dwSkillData[43];
      pBaseSkill->m_wWarpX            = dwSkillData[44];
      pBaseSkill->m_wWarpY            = dwSkillData[45];
      pBaseSkill->m_wFuncChangeId     = dwSkillData[46];
      pBaseSkill->m_iItemHm           = dwSkillData[47];
      pBaseSkill->m_iItemHp           = dwSkillData[48];
      pBaseSkill->m_iItemHard         = dwSkillData[49];
      pBaseSkill->m_iItemHu           = dwSkillData[50];
      pBaseSkill->m_iTimes            = dwSkillData[51];
      pBaseSkill->m_iProbability      = dwSkillData[52];
      pBaseSkill->m_wRace             = dwSkillData[53];
      pBaseSkill->m_wMonsterId        = dwSkillData[54];
      pBaseSkill->m_wChangeItemId     = dwSkillData[55];
      pBaseSkill->m_wUseEffect        = dwSkillData[56];
      pBaseSkill->m_wObjEffect        = dwSkillData[57];
      pBaseSkill->m_iIntonateTime     = dwSkillData[58];
      pBaseSkill->m_wBeforeDelay      = dwSkillData[59];
      pBaseSkill->m_wAfterDelay       = dwSkillData[60];
      pBaseSkill->m_bTarget           = dwSkillData[61];
      pBaseSkill->m_bShape            = dwSkillData[62];
      pBaseSkill->m_bRange            = dwSkillData[63];
      pBaseSkill->m_iBossSpecialTimeRate = dwSkillData[64];
      pBaseSkill->m_wCostMana         = dwSkillData[65];
      {
        
        if( pBaseSkill->m_bShape > SKILL_SHAPE_TARGET )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Skill=%d Shape=%d *****", pBaseSkill->m_wId, pBaseSkill->m_bShape );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;
        }
        if( pBaseSkill->m_bTarget > SKILL_TARGET_THE_END )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Skill=%d Target=%d *****", pBaseSkill->m_wId, pBaseSkill->m_bTarget );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;
        }
        // Set Max Target Count
        pBaseSkill->m_wTargetCount = g_ShapeCount[pBaseSkill->m_bShape==0?0:pBaseSkill->m_bShape-1];
        // Set The Temp Attack Range
        switch( pBaseSkill->m_bShape )
        {
        case SKILL_SHAPE_FAN_W1:
          pBaseSkill->m_wLoopCount = 1 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_FAN_W3:
          pBaseSkill->m_wLoopCount = 3 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_FAN_W5:
          pBaseSkill->m_wLoopCount = 5 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_FAN_W7:
          pBaseSkill->m_wLoopCount = 7 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_FAN3_L:
          pBaseSkill->m_wLoopCount = 3 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_FAN3_S:
          pBaseSkill->m_wLoopCount = 3 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_FAN4_R:
          pBaseSkill->m_wLoopCount = 4 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
          /////////////// LINE \\\\\\\\\\\\\\\\\\\\\/
        case SKILL_SHAPE_LINE_D1:
          pBaseSkill->m_wLoopCount = 1 + 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D2:
          pBaseSkill->m_wLoopCount = 2 + 1;
          pBaseSkill->m_wRangeForShape = 1 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D3:
          pBaseSkill->m_wLoopCount = 3 + 1;
          pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D4:
          pBaseSkill->m_wLoopCount = 4 + 1;
          pBaseSkill->m_wRangeForShape = 3 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D5:
          pBaseSkill->m_wLoopCount = 5 + 1;
          pBaseSkill->m_wRangeForShape = 4 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D6:
          pBaseSkill->m_wLoopCount = 6 + 1;
          pBaseSkill->m_wRangeForShape = 5 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D7:
          pBaseSkill->m_wLoopCount = 7 + 1;
          pBaseSkill->m_wRangeForShape = 6 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE_D8:
          pBaseSkill->m_wLoopCount = 8 + 1;
          pBaseSkill->m_wRangeForShape = 7 + pBaseSkill->m_bRange;
          break;
          //==========new add=========
        case SKILL_SHAPE_LINE3_S:
          pBaseSkill->m_wLoopCount = 2 + 1;
          pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE4_S:
          pBaseSkill->m_wLoopCount = 3 + 1;
          pBaseSkill->m_wRangeForShape = 3 + pBaseSkill->m_bRange;
          break;
          /////////////// LINE_W \\\\\\\\\\\\\\\\\\\\/
        case SKILL_SHAPE_LINEW_D2:
          pBaseSkill->m_wLoopCount = 6 + 1;
          pBaseSkill->m_wRangeForShape = 1 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINEW_D4:
          pBaseSkill->m_wLoopCount = 12 + 1;
          pBaseSkill->m_wRangeForShape = 3 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINEW_3://new add
          pBaseSkill->m_wLoopCount = 9 + 1;
          pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINEW_6://new add
          pBaseSkill->m_wLoopCount = 18 + 1;
          pBaseSkill->m_wRangeForShape = 5 + pBaseSkill->m_bRange;
          break;
          ////////////// 混合型 \\\\\\\\\\\\\\\\\\/
        case SKILL_SHAPE_LINEW2_FAN5:
          pBaseSkill->m_wLoopCount = 8 + 1;
          pBaseSkill->m_wRangeForShape = 4 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINEW2_ROUND3:
          pBaseSkill->m_wLoopCount = 11 + 1;
          pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_LINE4_ROUND3:
          pBaseSkill->m_wLoopCount = 11 + 1;
          pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
          break;
          ///////////// ROUND \\\\\\\\\\\\\\\\\\\\\\\/
        case SKILL_SHAPE_ROUND_1X1:
          pBaseSkill->m_wLoopCount = 0 + 1;
          pBaseSkill->m_wRangeForShape = 0 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_ROUND_3X3:
          pBaseSkill->m_wLoopCount = 8 + 1;
          pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_ROUND_5X5:
          pBaseSkill->m_wLoopCount = 24 + 1;
          pBaseSkill->m_wRangeForShape = 4 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_ROUND_7X7:
          pBaseSkill->m_wLoopCount = 48 + 1;
          pBaseSkill->m_wRangeForShape = 6 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_ROUND_9X9:
          pBaseSkill->m_wLoopCount = 80 + 1;
          pBaseSkill->m_wRangeForShape = 8 + pBaseSkill->m_bRange;
          break;
          /////////////////////////////
          //Add by CECE 2004-04-22
        case SKILL_SHAPE_ROUND_11X11:
          pBaseSkill->m_wLoopCount = 120 + 1;
          pBaseSkill->m_wRangeForShape = 10 + pBaseSkill->m_bRange;
          break;
        case SKILL_SHAPE_ROUND_13X13:
          pBaseSkill->m_wLoopCount = 168 + 1;
          pBaseSkill->m_wRangeForShape = 12 + pBaseSkill->m_bRange;
          break;
        default:
          pBaseSkill->m_wLoopCount = 1;
          pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
          break;
      }
      // Init Special Skill
      pBaseSkill->m_wSpecialCount = 0;
      memset( pBaseSkill->m_dwSimSpecial, 0, sizeof( DWORD ) *  32);
      for( int z = 0; z < 32; z++ )
      {
        if( ( pBaseSkill->m_qwStatus & ( 1 << z ) ) )
        {
          pBaseSkill->m_wSpecialCount++;
          pBaseSkill->m_dwSimSpecial[z] = ( 1 << z );
        }
      }
      pBaseSkill->m_wRaceCount = 0;
      memset( pBaseSkill->m_dwSimRaceAttr, 0, sizeof( DWORD ) * 32 );
      for( z = 0; z < 32; z++ )
      {
        if( ( pBaseSkill->m_dwRaceAttri & (1 << z ) ) )
        {
          pBaseSkill->m_dwSimRaceAttr[pBaseSkill->m_wRaceCount] = ( 1 << z );
          pBaseSkill->m_wRaceCount++;
        }
      }
      m_mapBaseSkill.insert(map<DWORD, CSrvBaseSkill*>::value_type(pBaseSkill->GetId(), pBaseSkill));
    }
  }
 }
 else
 {
    MessageBox( GetActiveWindow(), "Cannot find the Base Skill file", "Warning...", MB_OK );
    return false;
 }

//////////////////////////////////////////////////////////////////////////
  
  char szFilePath2[] = "Skill/AddSkill.txt";
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
#ifdef REPLACE_STRCAT
  SafeStrcat(szInitLog, "/",MAX_MEMO_MSG_LEN);
  SafeStrcat(szInitLog, szFilePath2, MAX_MEMO_MSG_LEN);
#else
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath2 );
#endif

  CInStream    Addbaseskill( szInitLog );
//CInStream    Addbaseskill(".\\GameData\\Skill\\AddSkill.txt");
  char         szTemp[1024];

  if ( !Addbaseskill.fail() && Addbaseskill.GetFileSize() !=0 ) 
  {
    if( !( Addbaseskill>>szTemp>>g_MyJinx ) )             return false;
    if( !( Addbaseskill>>szTemp>>g_SkillJinx ) )          return false;
    if( !( Addbaseskill>>szTemp>>g_ChainJinx ) )          return false;
    if( !( Addbaseskill>>szTemp>>g_BaseHit ) )            return false;
    if( !( Addbaseskill>>szTemp>>g_ChainHit ) )           return false;
    if( !( Addbaseskill>>szTemp>>g_MaxCritical ) )        return false;
    if( !( Addbaseskill>>szTemp>>g_CriticalRate ) )       return false;
    if( !( Addbaseskill>>szTemp>>g_MaxHit ) )             return false;
    if( !( Addbaseskill>>szTemp>>g_MinHit ) )             return false;
    if( !( Addbaseskill>>szTemp>>g_DpRate ) )             return false;
    if( !( Addbaseskill>>szTemp>>g_DpLimit ) )            return false;
    if( !( Addbaseskill>>szTemp>>g_HitLvRate ) )          return false;
    if( !( Addbaseskill>>szTemp>>g_MinHit_Ji ) )          return false;
    if( !( Addbaseskill>>szTemp>>g_MaxHit_Ji ) )          return false;
    if( !( Addbaseskill>>szTemp>>g_MinHit_Jian ) )        return false;
    if( !( Addbaseskill>>szTemp>>g_MaxHit_Jian ) )        return false;
    if( !( Addbaseskill>>szTemp>>g_MinHit_Gui ) )         return false;
    if( !( Addbaseskill>>szTemp>>g_MaxHit_Gui ) )         return false;
    if( !( Addbaseskill>>szTemp>>g_MinHit_Huan ) )        return false;
    if( !( Addbaseskill>>szTemp>>g_MaxHit_Huan ) )        return false;
    if( !( Addbaseskill>>szTemp>>g_MaxHpPercent ) )       return false;
    if( !( Addbaseskill>>szTemp>>g_LevelLimit ) )         return false;
    if( !( Addbaseskill>>szTemp>>g_BearJian) )            return false;
    if( !( Addbaseskill>>szTemp>>g_BearJi) )              return false;
    if( !( Addbaseskill>>szTemp>>g_BearGui) )             return false;
    if( !( Addbaseskill>>szTemp>>g_BearHuan) )            return false;
    if( !( Addbaseskill>>szTemp>>g_iMdef_Limit) )         return false;
    if( !( Addbaseskill>>szTemp>>g_ElementNull) )         return false;
    if( !( Addbaseskill>>szTemp>>g_ElementGrow) )         return false;
    if( !( Addbaseskill>>szTemp>>g_ElementJinx) )         return false;
    if( !( Addbaseskill>>szTemp>>g_ElementJxed) )         return false;
    if( !( Addbaseskill>>szTemp>>g_ElementSame) )         return false;
    if( !( Addbaseskill>>szTemp>>g_iPK_Punish) )          return false;
    if( !( Addbaseskill>>szTemp>>g_iPK_Expend) )          return false;
    if( !( Addbaseskill>>szTemp>>g_iDeadLoseExp_1) )      return false;
    if( !( Addbaseskill>>szTemp>>g_iDeadLoseExp_2) )      return false;
    if( !( Addbaseskill>>szTemp>>g_iDeadLoseExp_3) )      return false;
    if( !( Addbaseskill>>szTemp>>g_iDeadLoseExp_4) )      return false;
    if( !( Addbaseskill>>szTemp>>g_iDeadLoseExp_5) )      return false;
    if( !( Addbaseskill>>szTemp>>g_iTeamExpBonu) )        return false;
    if( !( Addbaseskill>>szTemp>>g_iMaxTaxRate) )         return false;
#ifdef _MONSTER_RESTORE_HP_
    if( !( Addbaseskill>>szTemp>>g_iMonsterHP1) )         return false;
    if( !( Addbaseskill>>szTemp>>g_iMonsterHP2) )         return false;
    if( !( Addbaseskill>>szTemp>>g_iMonsterHP3) )         return false;
#endif
#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_
    if( !( Addbaseskill>>szTemp>>g_iGuildSkill_MemberMin)) return false;
#endif
#ifdef _DEBUG_MICHAEL_ONLINE_UPLEVEL
    if( !( Addbaseskill>>szTemp>>g_iOnline_UpLevel ) )      return false;
    if( !( Addbaseskill>>szTemp>>g_iOnline_Percent ) )      return false;
#endif
    szTemp[1024-1] = '\0';
    //
    g_iPK_Expend = g_iPK_Expend * PK_PUNISH_TIME_COUNT;
    //
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Load Additional Skill Succeed, MyJinx=%d, SkillJinx=%d, ChainJinx=%d, BaseHit=%d, ChainHit=%d #", g_MyJinx, g_SkillJinx, g_ChainJinx, g_BaseHit, g_ChainHit );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MaxCritical=%d, CriticalRate=%d, MaxHit=%d, MinHit=%d #", g_MaxCritical, g_CriticalRate, g_MaxHit, g_MinHit );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "DpRate=%d, DpLimit=%d, HitLvRate=%d#", g_DpRate, g_DpLimit, g_HitLvRate );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MinHit_Ji=%d, MaxHit_Ji=%d, MinHit_Jian=%d, MaxHit_Jian=%d#", g_MinHit_Ji, g_MaxHit_Ji, g_MinHit_Jian,g_MaxHit_Jian );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MinHit_Gui=%d, MaxHit_Gui=%d, MinHit_Huan=%d, MaxHit_Huan=%d#", g_MinHit_Gui, g_MaxHit_Gui, g_MinHit_Huan,g_MaxHit_Huan );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MaxHpPercent=%d, LevelLimit=%d", g_MaxHpPercent,g_LevelLimit );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "PKPunish=%d, PK Expend=%d", g_iPK_Punish, g_iPK_Expend );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );	 
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot Open the AddSkill file", "warning..", MB_OK );
    return false;
  }
  return 1;
#else
//
  FuncName("CSrvBaseData::LoadBaseSkill");

  string					strPath = string(m_szObjFile);
	int							iCount = 0, i = 0, j = 0;//, iSpecialEffect[16], iOccuLimit[8], iWeaponLimit[11];
  CSrvBaseSkill		*pBaseSkill = NULL;
	FILE						*pFile = NULL;
  char            szSkillName[1024], szSkillCon[1024];

  strPath += "/";
  strPath += string(szFilePath);

  pFile = fopen(strPath.c_str(), "r");
  if( !pFile )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "Cannot find the Base Skill file: %s", strPath.c_str());
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szInitLog);
#endif
    return false;
  }
	// Get the total
	fscanf(pFile,"%d ",&iCount);
	if( iCount < 1 )            return false;

	int       dwSkillData[66];
  DWORD     dwSpecialTemp[2];  //dwSpecialTemp[0]:low DWORD, dwSpecialTemp[1]:high DWORD
	for( i = 0, j = 0; i < iCount; i++ )
	{
		//SkillID				Name						Describe			Icon				PracticeMax AdvanceID		Type		Element			ElementEffect CostHp
		//CostHp%				CostMp					CostMp%				CostSp			CostSp%			CostSoul		Block		AlarmTime		AlarmSkill		TriggerTime
		//TriggerSkill	Hp							Mp						Sp					Soul				Ap					Hit			Dp					Dg						Int
		//Random				Critical				Hard					Status			SKillLearn	Position		Color		Case				WarpMap				WarpX
		//WarpY					ChangeFunction	ItemHm				ItemHp			ItemHard		ItemHu			Times		Probability	Race					MonsterID
		//ChangeItem		UseEffect				ObjectEffect	BeforeDelay	AfterDelay	Target			Shape		Range  CostMana
		pBaseSkill = new CSrvBaseSkill;
    ZeroMemory( dwSkillData, sizeof( dwSkillData ) );
		if( 70 == ( j = fscanf( pFile,
			                      " %d %s %s %d %d \
                              %d %d %d %d %d \
                              %d %d %d %d %d %d \
                              %d %d %d %d %d %d \
                              %d %d %d %d %d \
                              %d %d %d %d %d %d \
                              %d %d %d %d %d %d \
                              %d %d %d %d %d \
                              %d %d %d %d %d \
                              %d %d %d %d %d \
                              %d %d %d %d %d \
                              %d %d %d %d %d %d \
                              %d %d %d %d %s ",
      &dwSkillData[0],  szSkillName,      szSkillCon,       &dwSkillData[1],  &dwSkillData[2],
      &dwSkillData[3],  &dwSkillData[4],  &dwSkillData[5],  &dwSkillData[6],  &dwSkillData[7],
      &dwSkillData[8],  &dwSkillData[9],  &dwSkillData[10], &dwSkillData[11], &dwSkillData[12],
      &dwSkillData[13], &dwSkillData[14], &dwSkillData[15], &dwSkillData[16], &dwSkillData[17],
      &dwSkillData[18], &dwSkillData[19], &dwSkillData[20], &dwSkillData[21], &dwSkillData[22],
      &dwSkillData[23], &dwSkillData[24], &dwSkillData[25], &dwSkillData[26], &dwSkillData[27],
      &dwSkillData[28], &dwSkillData[29], &dwSkillData[30], &dwSkillData[31], &dwSkillData[32],
      &dwSkillData[33], &dwSkillData[34], &dwSkillData[35], &dwSkillData[36], &dwSkillData[37],
      &dwSpecialTemp[1], &dwSpecialTemp[0], &dwSkillData[39], &dwSkillData[40], &dwSkillData[41], &dwSkillData[42],
      &dwSkillData[43], &dwSkillData[44], &dwSkillData[45], &dwSkillData[46], &dwSkillData[47],
      &dwSkillData[48], &dwSkillData[49], &dwSkillData[50], &dwSkillData[51], &dwSkillData[52],
      &dwSkillData[53], &dwSkillData[54], &dwSkillData[55], &dwSkillData[56], &dwSkillData[57],
      &dwSkillData[58], &dwSkillData[59], &dwSkillData[60], &dwSkillData[61], &dwSkillData[62],
      &dwSkillData[63], &dwSkillData[64], &dwSkillData[65], szInitLog ) ) )
		{
      //strcpy( szInitLog, "SkillData=" );
      //for( int h = 0; h < 63; h++ )
      //{
      //  sprintf( szSkillCon, "%d,", dwSkillData[h] );
      //  strcat( szInitLog, szSkillCon );
      //}
      //AddMemoMsg( szInitLog );
      // Copy Name And Content
      memcpy( pBaseSkill->m_szName, szSkillName, MAX_SKILL_NAME_LEN );
      pBaseSkill->m_szName[MAX_SKILL_NAME_LEN-1]       = '\0';
      // Init All Skill Data
		  pBaseSkill->m_wId               = dwSkillData[0];
		  pBaseSkill->m_wIconId           = dwSkillData[1];
		  pBaseSkill->m_wPracticeMax      = dwSkillData[2];
      pBaseSkill->m_wAdvanceSkillId   = dwSkillData[3];
		  pBaseSkill->m_wType             = dwSkillData[4];
      pBaseSkill->m_wElement          = dwSkillData[5];
		  pBaseSkill->m_dwRaceAttri       = dwSkillData[6];
      pBaseSkill->m_wRaceBonuRate     = dwSkillData[7];
		  pBaseSkill->m_dwBossCode        = dwSkillData[8];
      pBaseSkill->m_wBossBonuRate     = dwSkillData[9];
      pBaseSkill->m_wOwnAttrEffect    = dwSkillData[10];
      pBaseSkill->m_wOwnAttrBonuRate  = dwSkillData[11];
		  pBaseSkill->m_iEleEffect        = dwSkillData[12];
      pBaseSkill->m_iCostHp           = dwSkillData[13];		
		  pBaseSkill->m_iCostHpPer        = dwSkillData[14];
      pBaseSkill->m_iCostMp           = dwSkillData[15]; 
		  pBaseSkill->m_iCostMpPer        = dwSkillData[16];
      pBaseSkill->m_iCostSp           = dwSkillData[17]; 
		  pBaseSkill->m_iCostSpPer        = dwSkillData[18];
      pBaseSkill->m_iCostSoul         = dwSkillData[19];
		  pBaseSkill->m_bBlock            = dwSkillData[20];
			pBaseSkill->m_iAlarmTime        = dwSkillData[21];
		  pBaseSkill->m_wAlarmSkillId     = dwSkillData[22];
      pBaseSkill->m_iTriggerTime      = dwSkillData[23];
		  pBaseSkill->m_wTriggerSkillId   = dwSkillData[24];
      pBaseSkill->m_iHpChange         = dwSkillData[25];
		  pBaseSkill->m_iMpChange         = dwSkillData[26];
      pBaseSkill->m_iSpChange         = dwSkillData[27];
		  pBaseSkill->m_iSoulChange       = dwSkillData[28];
      pBaseSkill->m_iApChange         = dwSkillData[29];
		  pBaseSkill->m_iHitChange        = dwSkillData[30];
      pBaseSkill->m_iDpChange         = dwSkillData[31];
		  pBaseSkill->m_iDgChange         = dwSkillData[32];
      pBaseSkill->m_iIntChange        = dwSkillData[33];
      pBaseSkill->m_iBearPsbChange    = dwSkillData[34];
		  pBaseSkill->m_iRandom           = dwSkillData[35];
			pBaseSkill->m_iCriticalHit      = dwSkillData[36];
		  pBaseSkill->m_iHard             = dwSkillData[37];
      pBaseSkill->m_qwStatus          = MAKEQWORD(dwSpecialTemp[0],dwSpecialTemp[1]);
		  pBaseSkill->m_wSkillLearnId     = dwSkillData[39];
      pBaseSkill->m_bPosition         = dwSkillData[40];
		  pBaseSkill->m_bColor            = dwSkillData[41];
			pBaseSkill->m_bCase             = dwSkillData[42];
		  pBaseSkill->m_wWarpMapId        = dwSkillData[43];
      pBaseSkill->m_wWarpX            = dwSkillData[44];
		  pBaseSkill->m_wWarpY            = dwSkillData[45];
			pBaseSkill->m_wFuncChangeId     = dwSkillData[46];
		  pBaseSkill->m_iItemHm           = dwSkillData[47];
			pBaseSkill->m_iItemHp           = dwSkillData[48];
		  pBaseSkill->m_iItemHard         = dwSkillData[49];
      pBaseSkill->m_iItemHu           = dwSkillData[50];
		  pBaseSkill->m_iTimes            = dwSkillData[51];
			pBaseSkill->m_iProbability      = dwSkillData[52];
		  pBaseSkill->m_wRace             = dwSkillData[53];
      pBaseSkill->m_wMonsterId        = dwSkillData[54];
		  pBaseSkill->m_wChangeItemId     = dwSkillData[55];
      pBaseSkill->m_wUseEffect        = dwSkillData[56];
		  pBaseSkill->m_wObjEffect        = dwSkillData[57];
      pBaseSkill->m_iIntonateTime     = dwSkillData[58];
      pBaseSkill->m_wBeforeDelay      = dwSkillData[59];
		  pBaseSkill->m_wAfterDelay       = dwSkillData[60];
      pBaseSkill->m_bTarget           = dwSkillData[61];
		  pBaseSkill->m_bShape            = dwSkillData[62];
			pBaseSkill->m_bRange            = dwSkillData[63];
			pBaseSkill->m_iBossSpecialTimeRate = dwSkillData[64];
      /////////////////////////////////////////////////////
      //Add by CECE 2004-04-05
      pBaseSkill->m_wCostMana         = dwSkillData[65];
      /////////////////////////////////////////////////////
      // Check Data
      if( pBaseSkill->m_bShape > SKILL_SHAPE_TARGET )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Skill=%d Shape=%d *****", pBaseSkill->m_wId, pBaseSkill->m_bShape );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
        fclose( pFile );
			  return false;
      }
      if( pBaseSkill->m_bTarget > SKILL_TARGET_THE_END )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Skill=%d Target=%d *****", pBaseSkill->m_wId, pBaseSkill->m_bTarget );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
        fclose( pFile );
			  return false;
      }
      //
      //sprintf( szInitLog, "Class Skill=%d,%d,%d,%d,%d,%d,%d,%d",
      //         pBaseSkill->m_wId,      pBaseSkill->m_wIconId, pBaseSkill->m_wPracticeMax,
      //         pBaseSkill->m_wElement, pBaseSkill->m_iCostHp, pBaseSkill->m_iCriticalHit,
      //         pBaseSkill->m_dwRaceAttri, pBaseSkill->m_iTimes );
      //AddMemoMsg( szInitLog );
      //
			// Set Max Target Count
			pBaseSkill->m_wTargetCount = g_ShapeCount[pBaseSkill->m_bShape==0?0:pBaseSkill->m_bShape-1];
			// Set The Temp Attack Range
			switch( pBaseSkill->m_bShape )
			{
			////////////// FAN \\\\\\\\\\\\\\\\\\\/
			case SKILL_SHAPE_FAN_W1:
				pBaseSkill->m_wLoopCount = 1 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_FAN_W3:
				pBaseSkill->m_wLoopCount = 3 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_FAN_W5:
				pBaseSkill->m_wLoopCount = 5 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_FAN_W7:
				pBaseSkill->m_wLoopCount = 7 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_FAN3_L:
				pBaseSkill->m_wLoopCount = 3 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_FAN3_S:
				pBaseSkill->m_wLoopCount = 3 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_FAN4_R:
				pBaseSkill->m_wLoopCount = 4 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			/////////////// LINE \\\\\\\\\\\\\\\\\\\\\/
			case SKILL_SHAPE_LINE_D1:
				pBaseSkill->m_wLoopCount = 1 + 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D2:
				pBaseSkill->m_wLoopCount = 2 + 1;
				pBaseSkill->m_wRangeForShape = 1 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D3:
				pBaseSkill->m_wLoopCount = 3 + 1;
				pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D4:
				pBaseSkill->m_wLoopCount = 4 + 1;
				pBaseSkill->m_wRangeForShape = 3 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D5:
				pBaseSkill->m_wLoopCount = 5 + 1;
				pBaseSkill->m_wRangeForShape = 4 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D6:
				pBaseSkill->m_wLoopCount = 6 + 1;
				pBaseSkill->m_wRangeForShape = 5 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D7:
				pBaseSkill->m_wLoopCount = 7 + 1;
				pBaseSkill->m_wRangeForShape = 6 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE_D8:
				pBaseSkill->m_wLoopCount = 8 + 1;
				pBaseSkill->m_wRangeForShape = 7 + pBaseSkill->m_bRange;
				break;
			//==========new add=========
			case SKILL_SHAPE_LINE3_S:
				pBaseSkill->m_wLoopCount = 2 + 1;
				pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE4_S:
				pBaseSkill->m_wLoopCount = 3 + 1;
				pBaseSkill->m_wRangeForShape = 3 + pBaseSkill->m_bRange;
				break;
			/////////////// LINE_W \\\\\\\\\\\\\\\\\\\\/
			case SKILL_SHAPE_LINEW_D2:
				pBaseSkill->m_wLoopCount = 6 + 1;
				pBaseSkill->m_wRangeForShape = 1 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINEW_D4:
				pBaseSkill->m_wLoopCount = 12 + 1;
				pBaseSkill->m_wRangeForShape = 3 + pBaseSkill->m_bRange;
				break;
      case SKILL_SHAPE_LINEW_3://new add
				pBaseSkill->m_wLoopCount = 9 + 1;
				pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
        break;
      case SKILL_SHAPE_LINEW_6://new add
				pBaseSkill->m_wLoopCount = 18 + 1;
				pBaseSkill->m_wRangeForShape = 5 + pBaseSkill->m_bRange;
        break;
			////////////// 混合型 \\\\\\\\\\\\\\\\\\/
			case SKILL_SHAPE_LINEW2_FAN5:
				pBaseSkill->m_wLoopCount = 8 + 1;
				pBaseSkill->m_wRangeForShape = 4 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINEW2_ROUND3:
				pBaseSkill->m_wLoopCount = 11 + 1;
				pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_LINE4_ROUND3:
				pBaseSkill->m_wLoopCount = 11 + 1;
				pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
				break;
			///////////// ROUND \\\\\\\\\\\\\\\\\\\\\\\/
			case SKILL_SHAPE_ROUND_1X1:
				pBaseSkill->m_wLoopCount = 0 + 1;
				pBaseSkill->m_wRangeForShape = 0 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_ROUND_3X3:
				pBaseSkill->m_wLoopCount = 8 + 1;
				pBaseSkill->m_wRangeForShape = 2 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_ROUND_5X5:
				pBaseSkill->m_wLoopCount = 24 + 1;
				pBaseSkill->m_wRangeForShape = 4 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_ROUND_7X7:
				pBaseSkill->m_wLoopCount = 48 + 1;
				pBaseSkill->m_wRangeForShape = 6 + pBaseSkill->m_bRange;
				break;
			case SKILL_SHAPE_ROUND_9X9:
				pBaseSkill->m_wLoopCount = 80 + 1;
				pBaseSkill->m_wRangeForShape = 8 + pBaseSkill->m_bRange;
				break;
      /////////////////////////////
      //Add by CECE 2004-04-22
      case SKILL_SHAPE_ROUND_11X11:
	      pBaseSkill->m_wLoopCount = 120 + 1;
				pBaseSkill->m_wRangeForShape = 10 + pBaseSkill->m_bRange;
        break;
      case SKILL_SHAPE_ROUND_13X13:
				pBaseSkill->m_wLoopCount = 168 + 1;
				pBaseSkill->m_wRangeForShape = 12 + pBaseSkill->m_bRange;
        break;
      /////////////////////////////
			default:
				pBaseSkill->m_wLoopCount = 1;
				pBaseSkill->m_wRangeForShape = pBaseSkill->m_bRange;
				break;
			}
      // Init Special Skill
      pBaseSkill->m_wSpecialCount = 0;
      memset( pBaseSkill->m_dwSimSpecial, 0, sizeof( DWORD ) *  32);
      for( int z = 0; z < 32; z++ )
      {
          if( ( pBaseSkill->m_qwStatus & ( 1 << z ) ) )
          {
            pBaseSkill->m_wSpecialCount++;
            pBaseSkill->m_dwSimSpecial[z] = ( 1 << z );
          }
      }
      pBaseSkill->m_wRaceCount = 0;
      memset( pBaseSkill->m_dwSimRaceAttr, 0, sizeof( DWORD ) * 32 );
      for( z = 0; z < 32; z++ )
      {
        if( ( pBaseSkill->m_dwRaceAttri & (1 << z ) ) )
        {
          pBaseSkill->m_dwSimRaceAttr[pBaseSkill->m_wRaceCount] = ( 1 << z );
          pBaseSkill->m_wRaceCount++;
        }
      }
      //
			m_mapBaseSkill.insert(map<DWORD, CSrvBaseSkill*>::value_type(pBaseSkill->GetId(), pBaseSkill));
		}
		else
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base Skill=%d File Failed, Line=%d *****", pBaseSkill->m_wId, i );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg(szInitLog);
#endif
      fclose(pFile);
			return false;
		}
	}
  fclose(pFile);

  _snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "Load Base Skill File[%s] ok", strPath.c_str());
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(szInitLog);

  strPath = string( m_szObjFile );
  strPath += "/";
  strPath += string( "Skill/AddSkill.txt" );

	pFile = fopen( strPath.c_str(), "r" );
	if( pFile == NULL )
	{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
		_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, " Read %s File Failed ", strPath.c_str());
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoErrMsg(szInitLog);
#endif
    return 0;
	}

	if( 1 != fscanf( pFile, "MyJinx=%d\n", &g_MyJinx ) )              return false;
	if( 1 != fscanf( pFile, "SkillJinx=%d\n", &g_SkillJinx ) )        return false;
	if( 1 != fscanf( pFile, "ChainJinx=%d\n", &g_ChainJinx ) )        return false;
	if( 1 != fscanf( pFile, "BaseHit=%d\n", &g_BaseHit ) )            return false;
	if( 1 != fscanf( pFile, "ChainHit=%d\n", &g_ChainHit ) )          return false;
	if( 1 != fscanf( pFile, "MaxCritical=%d\n", &g_MaxCritical ) )    return false;
	if( 1 != fscanf( pFile, "CriticalRate=%d\n", &g_CriticalRate ) )  return false;
	if( 1 != fscanf( pFile, "MaxHit=%d\n", &g_MaxHit ) )              return false;
	if( 1 != fscanf( pFile, "MinHit=%d\n", &g_MinHit ) )              return false;
	if( 1 != fscanf( pFile, "DpRate=%d\n", &g_DpRate ) )              return false;
	if( 1 != fscanf( pFile, "DpLimit=%d\n", &g_DpLimit ) )            return false;
	if( 1 != fscanf( pFile, "HitLvRate=%d\n", &g_HitLvRate ) )        return false;
	if( 1 != fscanf( pFile, "MinHit_Ji=%d\n", &g_MinHit_Ji ) )        return false;
	if( 1 != fscanf( pFile, "MaxHit_Ji=%d\n", &g_MaxHit_Ji ) )        return false;
	if( 1 != fscanf( pFile, "MinHit_Jian=%d\n", &g_MinHit_Jian ) )    return false;
	if( 1 != fscanf( pFile, "MaxHit_Jian=%d\n", &g_MaxHit_Jian ) )    return false;
	if( 1 != fscanf( pFile, "MinHit_Gui=%d\n", &g_MinHit_Gui ) )      return false;
	if( 1 != fscanf( pFile, "MaxHit_Gui=%d\n", &g_MaxHit_Gui ) )      return false;
	if( 1 != fscanf( pFile, "MinHit_Huan=%d\n", &g_MinHit_Huan ) )    return false;
	if( 1 != fscanf( pFile, "MaxHit_Huan=%d\n", &g_MaxHit_Huan ) )    return false;
	if( 1 != fscanf( pFile, "MaxHpPercent=%d\n", &g_MaxHpPercent ) )  return false;
	if( 1 != fscanf( pFile, "LevelLimit=%d\n", &g_LevelLimit ) )      return false;
  //
  if( 1 != fscanf( pFile, "BearPos_Jian=%d\n", &g_BearJian) )       return false;
  if( 1 != fscanf( pFile, "BearPos_Ji=%d\n", &g_BearJi) )           return false;
  if( 1 != fscanf( pFile, "BearPos_Gui=%d\n", &g_BearGui) )         return false;
  if( 1 != fscanf( pFile, "BearPos_Huan=%d\n", &g_BearHuan) )       return false;
  //
  if( 1 != fscanf( pFile, "Mdef_Limit=%d\n", &g_iMdef_Limit) )      return false;
  //
  if( 1 != fscanf( pFile, "Element_Null=%d\n", &g_ElementNull) )    return false;
  if( 1 != fscanf( pFile, "Element_Grow=%d\n", &g_ElementGrow) )    return false;
  if( 1 != fscanf( pFile, "Element_Jinx=%d\n", &g_ElementJinx) )    return false;
  if( 1 != fscanf( pFile, "Element_Jxed=%d\n", &g_ElementJxed) )    return false;
  if( 1 != fscanf( pFile, "Element_Same=%d\n", &g_ElementSame) )    return false;
  //
  if( 1 != fscanf( pFile, "PK_Punish=%d\n", &g_iPK_Punish) )        return false;
  if( 1 != fscanf( pFile, "PK_Expend=%d\n", &g_iPK_Expend) )        return false;
  //
  if( 1 != fscanf( pFile, "DeadLoseExp1=%d\n", &g_iDeadLoseExp_1) ) return false;
  if( 1 != fscanf( pFile, "DeadLoseExp2=%d\n", &g_iDeadLoseExp_2) ) return false;
  if( 1 != fscanf( pFile, "DeadLoseExp3=%d\n", &g_iDeadLoseExp_3) ) return false;
  if( 1 != fscanf( pFile, "DeadLoseExp4=%d\n", &g_iDeadLoseExp_4) ) return false;
  if( 1 != fscanf( pFile, "DeadLoseExp5=%d\n", &g_iDeadLoseExp_5) ) return false;
  //
  if( 1 != fscanf( pFile, "TeamExpBonu=%d\n", &g_iTeamExpBonu) )    return false;
  //
  if( 1 != fscanf( pFile, "MaxTaxRate=%d\n", &g_iMaxTaxRate) )      return false;
//add by zetorchen
#ifdef _MONSTER_RESTORE_HP_
  if( 1 != fscanf( pFile, "MonsterHP1=%d\n", &g_iMonsterHP1) )      return false;
  if( 1 != fscanf( pFile, "MonsterHP2=%d\n", &g_iMonsterHP2) )      return false;
  if( 1 != fscanf( pFile, "MonsterHP3=%d\n", &g_iMonsterHP3) )      return false;
#endif
  //
#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_
  if( 1 != fscanf( pFile, "GuildSkill_MemberMin=%d\n", &g_iGuildSkill_MemberMin) )      return false;
#endif

#ifdef _DEBUG_MICHAEL_ONLINE_UPLEVEL
  if( 1 != fscanf( pFile, "Online_UpLevel=%d\n", &g_iOnline_UpLevel ) )      return false;
  if( 1 != fscanf( pFile, "Online_Percent=%d\n", &g_iOnline_Percent ) )      return false;
#endif

//#ifdef _DEBUG
  //g_iPK_Expend = 3;
//#else
  g_iPK_Expend = g_iPK_Expend * PK_PUNISH_TIME_COUNT;
//#endif
  //
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Load Additional Skill Succeed, MyJinx=%d, SkillJinx=%d, ChainJinx=%d, BaseHit=%d, ChainHit=%d #", g_MyJinx, g_SkillJinx, g_ChainJinx, g_BaseHit, g_ChainHit );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MaxCritical=%d, CriticalRate=%d, MaxHit=%d, MinHit=%d #", g_MaxCritical, g_CriticalRate, g_MaxHit, g_MinHit );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "DpRate=%d, DpLimit=%d, HitLvRate=%d#", g_DpRate, g_DpLimit, g_HitLvRate );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MinHit_Ji=%d, MaxHit_Ji=%d, MinHit_Jian=%d, MaxHit_Jian=%d#", g_MinHit_Ji, g_MaxHit_Ji, g_MinHit_Jian,g_MaxHit_Jian );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MinHit_Gui=%d, MaxHit_Gui=%d, MinHit_Huan=%d, MaxHit_Huan=%d#", g_MinHit_Gui, g_MaxHit_Gui, g_MinHit_Huan,g_MaxHit_Huan );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "MaxHpPercent=%d, LevelLimit=%d", g_MaxHpPercent,g_LevelLimit );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "PKPunish=%d, PK Expend=%d", g_iPK_Punish, g_iPK_Expend );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );	




	fclose( pFile );
  // Load Suit Equip Skill File
  /*strPath = string( m_szObjFile );
  strPath += "/";
  strPath += string( "Skill/SuitEquipSkill.txt" );

	pFile = fopen( strPath.c_str(), "r" );
	if( pFile == NULL )
	{
		sprintf( szInitLog, " Read %s File Failed, No Suit Equip Data...", strPath.c_str() );
		AddMemoErrMsg( szInitLog );
    return 1;
	}
  //
  fscanf( pFile, "%d ", &iCount );
	if( iCount < 1 )
  {
    sprintf( szInitLog, " Read SuitEquipSkill File Error, Count < 1..." );
		AddMemoErrMsg( szInitLog );
    return 1;
  }
  //
  CSuitEquipData        *pSuitSkill = NULL;
  for( i = 0; i < iCount; i++ )
  {
    if( 12 == fscanf( pFile, "%d %d %d %d %d"
                             "%d %d %d %d %d"
                             "%d %d",
                             &dwSkillData[0], &dwSkillData[1], &dwSkillData[2], &dwSkillData[3], &dwSkillData[4],
                             &dwSkillData[5], &dwSkillData[6], &dwSkillData[7], &dwSkillData[8], &dwSkillData[9],
                             &dwSkillData[10], &dwSkillData[11] ) )
    {
      if( pSuitSkill = new CSuitEquipData )
      {
        if( dwSkillData[0] <= 0 || dwSkillData[0] > 60000 ||
            dwSkillData[1] <= 0 || dwSkillData[0] > MAX_SUIT_EQUIP_COUNT )
        {
          sprintf( szInitLog, " Read SuitEquipSkill Data Error 1, Looper=%d", i );
		      AddMemoErrMsg( szInitLog );
          return false;
        }
        pSuitSkill->m_wId    = dwSkillData[0];
        pSuitSkill->m_wCount = dwSkillData[1];
        for( j = 0; j < MAX_SUIT_EQUIP_COUNT; j++ )
        {
          if( dwSkillData[j+2] )
          {
            if( NULL == ( pBaseSkill = g_pBase->GetBaseSkill( dwSkillData[j+2] ) ) )
            {
              sprintf( szInitLog, " Read SuitEquipSkill Data Error 2, Looper=%d, SkillId=%d", i, dwSkillData[j+2] );
		          AddMemoErrMsg( szInitLog );
              return false;
            }
            else  pSuitSkill->m_pBaseSkill[j] = pBaseSkill;
          }
          else    pSuitSkill->m_pBaseSkill[j] = NULL;
        }
        //
        m_mapSuitSkill.insert( map<WORD,CSuitEquipData*>::value_type( pSuitSkill->GetId(), pSuitSkill ) );
      }
    }
    else
    {
      sprintf( szInitLog, "Scan SuitEquipSkill File Error, Looper=%d...", i );
		  AddMemoErrMsg( szInitLog );
      return false;
    }
  }
  sprintf( szInitLog, "===>>> Load Suit Equip Skill Total=%d", m_mapSuitSkill.size() );
  AddMemoMsg( szInitLog );*/
  //
  return 1;
#endif
}
//===============================================================================================
//
//
int CSrvBaseData::LoadBaseItem(char *szFilePath)
{
      //日本版本server端加密档解密
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseItem");
  int             iTempCount = 0, iCount = 0, iTemp;
  char            szItemContent[1024];
  int             dwItemData[4];
  CSuitEquipData  *pSuitEquip = NULL;
  CSrvBaseSkill   *pBaseSkill = NULL;
  CSrvBaseItem	  *pItem			= NULL;

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  CInStream  Baseitem( szInitLog ); 
  assert(Baseitem); 
//  CInStream  Baseitem(".\\GameData\\Item\\Item.txt");
  if (!Baseitem.fail() && Baseitem.GetFileSize() != 0 ) 
  { 
    Baseitem>>iCount;
    if ( iCount == 1||iCount == 0 ) 
    {
      MessageBox(GetActiveWindow(),"Baseitem Error","iCount Error", MB_OK);
      return false;
    }
    while ( iTempCount<iCount ) 
    {
      pItem = new CSrvBaseItem(0);
      assert(pItem);
      Baseitem >> pItem->m_wId;
      Baseitem >> pItem->m_szName;
      Baseitem >> szItemContent;
      Baseitem >> pItem->m_wIconId;
      Baseitem >> pItem->m_wGroundIconId; 
      Baseitem >> pItem->m_wEquipPicId;    
      Baseitem >> pItem->m_wLevelLimit;    
      Baseitem >> pItem->m_wApLimit;      
      Baseitem >> pItem->m_wHitLimit;     
      Baseitem >> pItem->m_wDpLimit;       
      Baseitem >> pItem->m_wDgeLimit;     
      Baseitem >> pItem->m_wIntLimit;    
      Baseitem >> pItem->m_dwPrice;        
      Baseitem >> pItem->m_wSizeX;         
      Baseitem >> pItem->m_wSizeY;         
      Baseitem >> pItem->m_dwOccuLimit;    
      Baseitem >> pItem->m_dwSpecialAttr;  
      Baseitem >> pItem->m_dwType;         
      Baseitem >> pItem->m_wRareType;      
      Baseitem >> iTemp;                   
      Baseitem >> pItem->m_iAp;            
      Baseitem >> pItem->m_iHit;           
      Baseitem >> pItem->m_iDp;            
      Baseitem >> pItem->m_iDodge;         
      Baseitem >> pItem->m_iInt;           
      Baseitem >> pItem->m_iBearPsb;       
      Baseitem >> pItem->m_wLevel;         
      Baseitem >> pItem->m_wBlessLevel;    
      Baseitem >> pItem->m_wApUp;          
      Baseitem >> pItem->m_wHitUp;         
      Baseitem >> pItem->m_wDpUp;          
      Baseitem >> pItem->m_wDgeUp;         
      Baseitem >> pItem->m_wIntUp;         
      Baseitem >> pItem->m_wApFixLevel;    
      Baseitem >> pItem->m_wHitFixLevel;   
      Baseitem >> pItem->m_wDpFixLevel;    
      Baseitem >> pItem->m_wDodgeFixLevel; 
      Baseitem >> pItem->m_wIntFixLevel;   
      Baseitem >> pItem->m_wApFix;         
      Baseitem >> pItem->m_wHitFix;        
      Baseitem >> pItem->m_wDpFix;         
      Baseitem >> pItem->m_wDodgeFix;      
      Baseitem >> pItem->m_wIntFix;        
      Baseitem >> pItem->m_wCriticalHit;   
      Baseitem >> pItem->m_wHoleNumber;    
      Baseitem >> pItem->m_wMaxDurability; 
      Baseitem >> pItem->m_wDurability;    
      Baseitem >> pItem->m_wDurabilityUp;  
      Baseitem >> pItem->m_wForging;       
      Baseitem >> pItem->m_wForgingDblUp;  
      Baseitem >> pItem->m_wHard;          
      Baseitem >> pItem->m_wStable;        
      Baseitem >> pItem->m_wMaxWrap;       
      Baseitem >> iTemp;                   
      Baseitem >> pItem->m_wElement;       
      Baseitem >> pItem->m_dwRaceAttri;    
      Baseitem >> pItem->m_wRaceBonuRate;  
      Baseitem >> pItem->m_dwBossCode;     
      Baseitem >> pItem->m_wBossBonuRate;  
      Baseitem >> pItem->m_wFuncDbc;       
      Baseitem >> pItem->m_iEffectTime;    
      Baseitem >> pItem->m_wFuncEffect;    
      Baseitem >> pItem->m_wFuncEquip;     
      Baseitem >> pItem->m_wFuncTakeOff;  
      Baseitem >> pItem->m_wFuncTouch;
      Baseitem >> dwItemData[0];
      Baseitem >> dwItemData[1];
      Baseitem >> dwItemData[2];
      Baseitem >> dwItemData[3];
      Baseitem >> szInitLog;
      //
      pItem->m_szName[MAX_ITEM_NAME_LEN-1] = '\0';
      szItemContent[1024-1] = '\0';
      //
      {
        iTempCount++;
        pSuitEquip = NULL; 
        /////////////////////////////////////////
        if( dwItemData[0] && NULL == ( pSuitEquip = GetSuitEquip( dwItemData[0] ) ) )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) Suit Equip(%d) Failed *****",
            pItem->m_wId, dwItemData[0] );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;
        }
        pItem->m_pSuitEquip     = pSuitEquip;
        pItem->m_wSuitPRI       = dwItemData[1];
        if( dwItemData[2] && NULL == ( pBaseSkill = GetBaseSkill( dwItemData[2] ) ) )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) Suit Skill(%d) Failed *****",
            pItem->m_wId, dwItemData[2] );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;
        }
        pItem->m_pSuitSkill     = pBaseSkill;
        //////////////////////////////////////////////
        //Add by CECE 2004-04-05
        pItem->m_wEvilWeapon    = dwItemData[3];
        //////////////////////////////////////////////
        // Check Data
        if( pItem->m_wSizeX >= MAX_PACKAGE_SIZEX || pItem->m_wSizeY >= MAX_PACKAGE_SIZEY || pItem->m_wLevel > 15 )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) SizeX=%d, SizeY=%d Level=%d *****", pItem->m_wId, pItem->m_wSizeX, pItem->m_wSizeY, pItem->m_wLevel );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;
        }
        if( pItem->m_wHoleNumber > MAX_ITEM_SLOT || pItem->m_wStable >= MAX_STABLE_TYPE || pItem->m_wMaxWrap > 255 )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) HoleNumber=%d, Stable=%d MaxWrap=%d *****",
            pItem->m_wId, pItem->m_wHoleNumber, pItem->m_wStable, pItem->m_wMaxWrap );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;
        }
        if( pItem->m_dwSpecialAttr & ITEMX_PROPERTY_CONSUMPTIVE && m_mapBaseSkill.find(pItem->m_wFuncDbc) == m_mapBaseSkill.end() )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) Attr=%d, CONSUMPTIVE but FuncDbc=%d",
            pItem->m_wId, pItem->m_dwSpecialAttr, pItem->m_wFuncDbc );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return false;	
        }
        // Insert The New Base Item
        m_mapBaseItem.insert( map<DWORD, CSrvBaseItem*>::value_type( pItem->m_wId, pItem ) );
#ifdef _DEBUG_CHECK_UNIQUE_ITEN_
        if(pItem->m_wId == 1653)
        {
          g_pCopyItem = pItem;
        }
#endif        
      }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot Open BaseItem.txt", "Warning..." ,MB_OK );
    return false;
  }

  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Load %d Base Items From %s", iTempCount, szFilePath );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  return 1;
#else
  //
  FuncName("CSrvBaseData::LoadBaseItem");
	
  CSrvBaseItem	  *pItem			 = NULL;
  FILE					  *pFile			 = NULL;
  int						  iCount       = 0;
	int						  iTemp;
  char            szItemName[1024], szItemContent[1024];
  int             dwItemData[67];
	CSuitEquipData  *pSuitEquip = NULL;
  CSrvBaseSkill   *pBaseSkill = NULL;

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  pFile = fopen(szInitLog, "r"); 
  if( !pFile )
  {
    return false; 
  }
	// Get the total
	fscanf( pFile,"%d ", &iCount );
	if( iCount < 1 )		  return false;

	//	int	nElement;
	for( int i = 0, j = 0; i < iCount; i++ )
	{
		// 1:Id; 2:Name; 3:Content; 4:IconId; 5:GroundPicId; 6:WeaponPicId; 7:Lv_Lmt;
		// 8:Ap_Lmt; 9:Hit_Lmt; 10:Dp_Lmt; 11:Dg_lmt; 12:Int_lmt; 13:Price; 14:SizeX;
		// 15:SizeY; 16:O_Lmt; 17:Attri; 18:Type; 19:Item_Id; 20:Ap; 21:Hit; 22:Dp; 23:Dg;
		// 24:Int; 25:Level; 26:Ap_Up; 27:Hit_Up; 28:Dp_Up; 29:Dg_Up; 30:Int_Up; 31:ApFixLv;
		// 32:HitFixLv; 33:DpFixLv; 34:DgFixLv; 35:IntFixLv; 36:Ap_Fix; 37:Hit_Fix;
		// 38:Dp_Fix; 39:Dg_Fix; 40:Int_Fix; 41:CriticalHit; 42:Hole_Num; 43:Hm; 44:Hp;
		// 45:Hm_Up; 46:Hu; 47:Hu_Up; 48:Hard; 49:Stable; 50:Wrap_Max; 51:Wrap; 52:Func_Dbc
		// 53:Effect_Times; 54:Func_Effect; 55:Func_Puton; 56:Func_TakeOff; 57:Func_Touch;
    // 58:EvilWeaponID;59:End&;
		pItem = new CSrvBaseItem( 0 );
    ZeroMemory( dwItemData, sizeof( dwItemData ) );
		if( 70 == fscanf( pFile,
											 "%d %s %s %d %d"
                       "%d %d %d %d %d"
											 "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %d"
                       "%d %d %d %d %s",
                       &dwItemData[0],  szItemName,      szItemContent,   &dwItemData[1],  &dwItemData[2],
                       &dwItemData[3],  &dwItemData[4],  &dwItemData[5],  &dwItemData[6],  &dwItemData[7],
                       &dwItemData[8],  &dwItemData[9],  &dwItemData[10], &dwItemData[11], &dwItemData[12],
                       &dwItemData[13], &dwItemData[14], &dwItemData[15], &dwItemData[16], &dwItemData[17],
                       &dwItemData[18], &dwItemData[19], &dwItemData[20], &dwItemData[21], &dwItemData[22],
                       &dwItemData[23], &dwItemData[24], &dwItemData[25], &dwItemData[26], &dwItemData[27],
                       &dwItemData[28], &dwItemData[29], &dwItemData[30], &dwItemData[31], &dwItemData[32],
                       &dwItemData[33], &dwItemData[34], &dwItemData[35], &dwItemData[36], &dwItemData[37],
                       &dwItemData[38], &dwItemData[39], &dwItemData[40], &dwItemData[41], &dwItemData[42],
                       &dwItemData[43], &dwItemData[44], &dwItemData[45], &dwItemData[46], &dwItemData[47],
                       &dwItemData[48], &dwItemData[49], &dwItemData[50], &dwItemData[51], &dwItemData[52],
                       &dwItemData[53], &dwItemData[54], &dwItemData[55], &dwItemData[56], &dwItemData[57],
                       &dwItemData[58], &dwItemData[59], &dwItemData[60], &dwItemData[61], &dwItemData[62],
                       &dwItemData[63], &dwItemData[64], &dwItemData[65], &dwItemData[66], szInitLog ) )
		{
      //
      //strcpy( szInitLog, "ItemData=" );
      //for( int h = 0; h < 63; h++ )
      //{
      //  sprintf( szItemContent, "%d,", dwItemData[h] );
      //  strcat( szInitLog, szItemContent );
      //}
      //AddMemoMsg( szInitLog );
			// Copy Name And Content
      memcpy( pItem->m_szName, szItemName, MAX_ITEM_NAME_LEN );
      pItem->m_szName[MAX_ITEM_NAME_LEN-1] = '\0';

      //
      
			pItem->m_wId            = dwItemData[0];
      pItem->m_wIconId        = dwItemData[1];
      pItem->m_wGroundIconId  = dwItemData[2];
      pItem->m_wEquipPicId    = dwItemData[3];
			pItem->m_wLevelLimit    = dwItemData[4];
      pItem->m_wApLimit       = dwItemData[5];
      pItem->m_wHitLimit      = dwItemData[6];
      pItem->m_wDpLimit       = dwItemData[7];
      pItem->m_wDgeLimit      = dwItemData[8];
      pItem->m_wIntLimit      = dwItemData[9];
			pItem->m_dwPrice        = dwItemData[10];
      pItem->m_wSizeX         = dwItemData[11];
      pItem->m_wSizeY         = dwItemData[12];
      pItem->m_dwOccuLimit    = dwItemData[13];
      pItem->m_dwSpecialAttr  = dwItemData[14];
      pItem->m_dwType         = dwItemData[15];
      pItem->m_wRareType      = dwItemData[16];
			iTemp                   = dwItemData[17];
      pItem->m_iAp            = dwItemData[18];
      pItem->m_iHit           = dwItemData[19];
      pItem->m_iDp            = dwItemData[20];
      pItem->m_iDodge         = dwItemData[21];
      pItem->m_iInt           = dwItemData[22];
      pItem->m_iBearPsb       = dwItemData[23];
      pItem->m_wLevel         = dwItemData[24];
      pItem->m_wBlessLevel    = dwItemData[25];
      pItem->m_wApUp          = dwItemData[26];
			pItem->m_wHitUp         = dwItemData[27];
      pItem->m_wDpUp          = dwItemData[28];
      pItem->m_wDgeUp         = dwItemData[29];
      pItem->m_wIntUp         = dwItemData[30];
      pItem->m_wApFixLevel    = dwItemData[31];
      pItem->m_wHitFixLevel   = dwItemData[32];
      pItem->m_wDpFixLevel    = dwItemData[33];
      pItem->m_wDodgeFixLevel = dwItemData[34];
			pItem->m_wIntFixLevel   = dwItemData[35];
      pItem->m_wApFix         = dwItemData[36];
      pItem->m_wHitFix        = dwItemData[37];
      pItem->m_wDpFix         = dwItemData[38];
      pItem->m_wDodgeFix      = dwItemData[39];
      pItem->m_wIntFix        = dwItemData[40];
      pItem->m_wCriticalHit   = dwItemData[41];
			pItem->m_wHoleNumber    = dwItemData[42];
      pItem->m_wMaxDurability = dwItemData[43];
      pItem->m_wDurability    = dwItemData[44];
      pItem->m_wDurabilityUp  = dwItemData[45];
      pItem->m_wForging       = dwItemData[46];
      pItem->m_wForgingDblUp  = dwItemData[47];
			pItem->m_wHard          = dwItemData[48];
      pItem->m_wStable        = dwItemData[49];
      pItem->m_wMaxWrap       = dwItemData[50];
      iTemp                   = dwItemData[51];
      pItem->m_wElement       = dwItemData[52];
      pItem->m_dwRaceAttri    = dwItemData[53];
      pItem->m_wRaceBonuRate  = dwItemData[54];
      pItem->m_dwBossCode     = dwItemData[55];
      pItem->m_wBossBonuRate  = dwItemData[56];
      pItem->m_wFuncDbc       = dwItemData[57];
      pItem->m_iEffectTime    = dwItemData[58];
      pItem->m_wFuncEffect    = dwItemData[59];
      pItem->m_wFuncEquip     = dwItemData[60];
      pItem->m_wFuncTakeOff   = dwItemData[61];
      pItem->m_wFuncTouch     = dwItemData[62];
      /////////////////////////////////////////
      //Add by CECE 2004-04-19
      pSuitEquip              = NULL;
      /////////////////////////////////////////
      if( dwItemData[63] && NULL == ( pSuitEquip = GetSuitEquip( dwItemData[63] ) ) )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) Suit Equip(%d) Failed *****",
                 pItem->m_wId, dwItemData[63] );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
        fclose( pFile );
			  return false;
      }
      pItem->m_pSuitEquip     = pSuitEquip;
      pItem->m_wSuitPRI       = dwItemData[64];
      if( dwItemData[65] && NULL == ( pBaseSkill = GetBaseSkill( dwItemData[65] ) ) )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) Suit Skill(%d) Failed *****",
                 pItem->m_wId, dwItemData[65] );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
        fclose( pFile );
			  return false;
      }
      pItem->m_pSuitSkill     = pBaseSkill;
      //////////////////////////////////////////////
      //Add by CECE 2004-04-05
      pItem->m_wEvilWeapon    = dwItemData[66];
      //////////////////////////////////////////////
      // Check Data
      if( pItem->m_wSizeX >= MAX_PACKAGE_SIZEX || pItem->m_wSizeY >= MAX_PACKAGE_SIZEY || pItem->m_wLevel > 15 )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) SizeX=%d, SizeY=%d Level=%d *****", pItem->m_wId, pItem->m_wSizeX, pItem->m_wSizeY, pItem->m_wLevel );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
        fclose( pFile );
			  return false;
      }
      if( pItem->m_wHoleNumber > MAX_ITEM_SLOT || pItem->m_wStable >= MAX_STABLE_TYPE || pItem->m_wMaxWrap > 255 )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) HoleNumber=%d, Stable=%d MaxWrap=%d *****",
                 pItem->m_wId, pItem->m_wHoleNumber, pItem->m_wStable, pItem->m_wMaxWrap );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
        fclose( pFile );
			  return false;
      }
			if( pItem->m_dwSpecialAttr & ITEMX_PROPERTY_CONSUMPTIVE && m_mapBaseSkill.find(pItem->m_wFuncDbc) == m_mapBaseSkill.end() )
			{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Check Base Item(%d) Attr=%d, CONSUMPTIVE but FuncDbc=%d",
                 pItem->m_wId, pItem->m_dwSpecialAttr, pItem->m_wFuncDbc );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			  AddMemoErrMsg( szInitLog );
#endif
				fclose( pFile );
				return false;	
			}
			// Insert The New Base Item
			m_mapBaseItem.insert( map<DWORD, CSrvBaseItem*>::value_type( pItem->m_wId, pItem ) );
#ifdef _DEBUG_CHECK_UNIQUE_ITEN_
      if(pItem->m_wId == 1653)
      {
        g_pCopyItem = pItem;
      }
#endif
		}
		else
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base Item File Failed (Index = %d)! *****", i );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
      fclose( pFile );
			return false;
		}
	}
  //
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Load %d Base Items From %s", i, szFilePath );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );

  fclose( pFile );

  return 1;
#endif
}
/////////////////////////////////////////////////////////////////
//
//音乐盒
#ifdef _MUSIC_BOX_
int CSrvBaseData::LoadMusicBoxItem( char *szFileName )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadMusicBoxItem");

  int						  iCount       = 0;
  DWORD           dwItemId;
  DWORD           dwSongNum;
  //
  string strFileName = m_szObjFile;
  strFileName += szFileName;
  CInStream MusicBoxItem((char*)strFileName.c_str());
  if (MusicBoxItem.fail()||MusicBoxItem.GetFileSize() == 0) 
  {
    return 0;
  }
  m_mapMusicBoxItem.clear();
  MusicBoxItem >> iCount;
  if (iCount != 0) 
  {
    for ( int i=0; i<iCount; i++ ) 
    {
      MusicBoxItem>>dwItemId>>dwSongNum;
      m_mapMusicBoxItem.insert( map<DWORD, DWORD>::value_type(dwItemId, dwSongNum) );
    }
  }
  return 1;
#else
   FuncName("CSrvBaseData::LoadMusicBoxItem");

  FILE					  *pFile			 = NULL;
  int						  iCount       = 0;
  DWORD           dwItemId;
  DWORD           dwSongNum;
  //
  string strFileName = m_szObjFile;
  strFileName += szFileName;
  pFile = fopen(strFileName.c_str(), "r");
  if( NULL==pFile ) return 0;
  m_mapMusicBoxItem.clear();

	fscanf( pFile,"%d ",&iCount );
  for( int i=0; i<iCount; i++ )
  {
    if(2 == fscanf( pFile,
            "%d %d",
             &dwItemId,&dwSongNum) )
    {
      m_mapMusicBoxItem.insert( map<DWORD, DWORD>::value_type(dwItemId, dwSongNum) );
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load MusicBox Item Error (Id=%d), Scan File Data Error ! *****", dwItemId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return 0;
    }
  }
  fclose( pFile );
  pFile = NULL;
  //
  return 1;
#endif //_DEBUG_JAPAN_DECRYPT_
}
#endif
#ifdef _NEW_TRADITIONS_WEDDING_
int CSrvBaseData::LoadRestoreItem(char *szFileName)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  int       iCount = 0;
  string strFileName = m_szObjFile;
  strFileName += szFileName;
  CInStream RestoreItem((char*)strFileName.c_str());
  if (RestoreItem.fail()||RestoreItem.GetFileSize() == 0)
    return 0;

  RestoreItemInfo restoreItem;
  m_BaseRestoreItemList.clear();

  RestoreItem>>iCount;
  if ( iCount!=0 ) 
  {
    for ( int i=0; i<iCount; i++ ) 
    {
      RestoreItem>>restoreItem.iItemId
                 >>restoreItem.iVal[0]
                 >>restoreItem.iVal[1]
                 >>restoreItem.iVal[2]
                 >>restoreItem.iVal[3];
      m_BaseRestoreItemList.push_back(restoreItem);
    }
  }
  return iCount;
#else
  FILE      *pFile = NULL;
  int       iCount = 0;

  string strFileName = m_szObjFile;
  strFileName += szFileName;
  pFile = fopen(strFileName.c_str(), "r");
  if( NULL==pFile ) return 0;

  RestoreItemInfo restoreItem;
  m_BaseRestoreItemList.clear();
	fscanf( pFile,"%d ",&iCount );
  for( int i=0; i<iCount; i++ )
  {
    if(5 == fscanf( pFile,
            "%d %d %d %d %d",
             &restoreItem.iItemId,
             &restoreItem.iVal[0],
             &restoreItem.iVal[1],
             &restoreItem.iVal[2],
             &restoreItem.iVal[3]))
    {
      m_BaseRestoreItemList.push_back(restoreItem);
    }
  }
  fclose( pFile );
  pFile = NULL;
  return iCount;
#endif//_DEBUG_JAPAN_DECRYPT_
}
#endif
/////////////////////////////////////////////////////////////////
//
//Add By Zetorchen
#ifdef SAVE_TRANS_FUNC
int  CSrvBaseData::LoadSavePointItem(char *szFileName)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadSavePointItem");
  int						  iCount       = 0;
  DWORD           dwItemId;
  WORD            wItemUseCount;
  WORD            wItemOdds;
  DWORD           dwTemp[2] = {0,0};
  //
  string strFileName = m_szObjFile;
  strFileName += szFileName;
  CInStream SavePiont( (char*)strFileName.c_str());
  if (SavePiont.fail()||SavePiont.GetFileSize() == 0) 
  {
#ifdef _DEBUG
   MessageBox( GetActiveWindow(), "SavePiont Open File fail", "Waring.....", MB_OK);
#endif
    return 0;
  }
  m_mapSavePointItem.clear();
  SavePiont >> iCount;
  for( int i=0; i<iCount; i++ )
  {
    if(SavePiont>>dwItemId>>dwTemp[0]>>dwTemp[1])
    {
      wItemUseCount = dwTemp[0];
      wItemOdds     = dwTemp[1];
      m_mapSavePointItem.insert( map<DWORD, DWORD>::value_type(dwItemId, MAKELONG(wItemOdds,wItemUseCount)) );
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Save Point Item Error (Id=%d), Scan File Data Error ! *****", dwItemId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return 0;
    }
  }
  //
  return 1;
#else
  FuncName("CSrvBaseData::LoadSavePointItem");

  FILE					  *pFile			 = NULL;
  int						  iCount       = 0;
  DWORD           dwItemId;
  WORD            wItemUseCount;
  WORD            wItemOdds;
  DWORD           dwTemp[2] = {0,0};
  //
  string strFileName = m_szObjFile;
  strFileName += szFileName;
  pFile = fopen(strFileName.c_str(), "r");
  if( NULL==pFile ) return 0;
  m_mapSavePointItem.clear();

	fscanf( pFile,"%d ",&iCount );
  for( int i=0; i<iCount; i++ )
  {
    if(3 == fscanf( pFile,
            "%d %d %d",
             &dwItemId,&dwTemp[0],&dwTemp[1] ) )
    {
      wItemUseCount = dwTemp[0];
      wItemOdds     = dwTemp[1];
      m_mapSavePointItem.insert( map<DWORD, DWORD>::value_type(dwItemId, MAKELONG(wItemOdds,wItemUseCount)) );
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Save Point Item Error (Id=%d), Scan File Data Error ! *****", dwItemId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return 0;
    }
  }
  fclose( pFile );
  pFile = NULL;
  //
  return 1;
#endif //_DEBUG_JAPAN_DECRYPT_
}
#endif

///////////////////////////////////////////////////////////
//Add by CECE 2004-04-05
#ifdef EVILWEAPON_3_6_VERSION

int  CSrvBaseData::LoadBaseEvilWeapon(char *szFileName)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseEvilWeapon");
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFileName );
  CInStream BaseEvilWeapon( szInitLog );
//  CInStream BaseEvilWeapon(".\\GameData\\Item\\EvilWeapon.txt");
  SBaseEvilWeapon *pEvilWeapon = NULL;
  int						  iCount       = 0;
  int             iTemCount    = 0;
  
  if ( !BaseEvilWeapon.fail() && BaseEvilWeapon.GetFileSize() != 0 ) 
  {
    BaseEvilWeapon >> iCount;
    if ( iCount == 1 ) 
    {
      MessageBox(GetActiveWindow(), "LoadBaseEvilWeapon iCount Error", "Waring", MB_OK);
      return 0;
    }
    for ( ; iTemCount<iCount; iTemCount++ ) 
    {
      pEvilWeapon = new SBaseEvilWeapon;
      assert( pEvilWeapon );
      BaseEvilWeapon >> pEvilWeapon->m_wId;
      BaseEvilWeapon >> pEvilWeapon->m_wMonPicId;
      BaseEvilWeapon >> pEvilWeapon->m_wMonEffect;
      BaseEvilWeapon >> pEvilWeapon->m_wPractice[0];
      BaseEvilWeapon >> pEvilWeapon->m_wPractice[1];
      BaseEvilWeapon >> pEvilWeapon->m_wSkillLevel[0];
      BaseEvilWeapon >> pEvilWeapon->m_wSkillLevel[1];
      BaseEvilWeapon >> pEvilWeapon->m_wSkillLevel[2];
      BaseEvilWeapon >> pEvilWeapon->m_wManaLimit;
      BaseEvilWeapon >> pEvilWeapon->m_wManaLimitGrow;
      BaseEvilWeapon >> pEvilWeapon->m_wManaGetOdds;
      BaseEvilWeapon >> pEvilWeapon->m_wManaGetOddsGrow;
      BaseEvilWeapon >> pEvilWeapon->m_wSkillId;
      {
        m_mapBaseEvilWeapon.insert( map<int, SBaseEvilWeapon*>::value_type(pEvilWeapon->m_wId, pEvilWeapon ) );
      } 
    }     
  } 
  else
  {
    MessageBox(GetActiveWindow(), "Open EvilWeapon.txt Error", "Waring...", MB_OK);
    return 0;
  }
  return 1;  
#else 
  FuncName("CSrvBaseData::LoadBaseEvilWeapon");

  SBaseEvilWeapon *pEvilWeapon = NULL;
  FILE					  *pFile			 = NULL;
  int						  iCount       = 0;
  //
  string strFileName = m_szObjFile;
  strFileName += '/';
  strFileName += szFileName;
  pFile = fopen(strFileName.c_str(), "r");
  if( NULL==pFile ) return 0;
  //
	// Get Monster Drop Item Data
	fscanf( pFile,"%d ",&iCount );
  for( int i=0; i<iCount; i++ )
  {
    pEvilWeapon = new SBaseEvilWeapon;
    if(13== fscanf( pFile,
            "%hd %hd %hd"
            "%hd %hd %hd"
            "%hd %hd %hd"
            "%hd %hd %hd"
            "%hd",
            &pEvilWeapon->m_wId, &pEvilWeapon->m_wMonPicId, &pEvilWeapon->m_wMonEffect,
            &pEvilWeapon->m_wPractice[0], &pEvilWeapon->m_wPractice[1], &pEvilWeapon->m_wSkillLevel[0],
            &pEvilWeapon->m_wSkillLevel[1], &pEvilWeapon->m_wSkillLevel[2], &pEvilWeapon->m_wManaLimit,
            &pEvilWeapon->m_wManaLimitGrow, &pEvilWeapon->m_wManaGetOdds, &pEvilWeapon->m_wManaGetOddsGrow,
            &pEvilWeapon->m_wSkillId ) )
    {
      m_mapBaseEvilWeapon.insert( map<int, SBaseEvilWeapon*>::value_type(pEvilWeapon->m_wId, pEvilWeapon ) );
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load EvilWeapon Item Error (Id=%d), Scan File Data Error ! *****", pEvilWeapon->m_wId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return 0;
    }
  }
  fclose( pFile );
  pFile = NULL;
  //
  return 1;
#endif
}
#endif

#ifdef _NEW_TRADITIONS_WEDDING_
BOOL CSrvBaseData::GetRestoreItemInfo(DWORD dwItemId,int& iVal1,int& iVal2,int& iVal3,int& iVal4)
{
  RestoreItemInfo restoreItem;
  list<RestoreItemInfo>::iterator itr = m_BaseRestoreItemList.begin();
  for(; itr != m_BaseRestoreItemList.end(); itr++)
  {
    restoreItem = *(itr);
    if (restoreItem.iItemId == dwItemId)
    {
      iVal1 = restoreItem.iVal[0];
      iVal2 = restoreItem.iVal[1];
      iVal3 = restoreItem.iVal[2];
      iVal4 = restoreItem.iVal[3];
      return TRUE;
    }
  }
  iVal1 = 0;
  iVal2 = 0;
  iVal3 = 0;
  iVal4 = 0;
  return FALSE;
}
#endif
///////////////////////////////////////////////////////////
#ifdef FUNCTION_MAKEHOLE_ITEM
WORD CSrvBaseData::GetMixHoleItem(WORD wItemId)
{
  map<int,int>::iterator Itor;
  if((Itor = m_mapMixHoleItem.find(wItemId)) != m_mapMixHoleItem.end()) 
  {
    return Itor->second;
  }
  else
    return 0;  //不是列表的Item
}
#endif

//===============================================================================================
//Add by zetorchen for Odds in Combine Item
//
#ifdef _COMBINE_RANDOM_
//===============================================================================================
//Add by zetorchen for Odds in Combine Item
//
#ifndef _NEW_COMBINE_SYSTEM_
DWORD CSrvBaseData::FindCombineItemOdds( const DWORD &dwId )
{
  if( m_mapCombineItemOdds.size() )
  {
    map<DWORD,DWORD>::iterator It;
    if ( (It=m_mapCombineItemOdds.find(dwId))!=m_mapCombineItemOdds.end() )
    {
      return It->second;
    }
    else
    {
      return 100;
    }
  }
  else
  {
    return 100;
  }
}
#endif // _NEW_COMBINE_SYSTEM_

#ifdef _NEW_COMBINE_SYSTEM_
DWORD CSrvBaseData::FindCombineItemOdds(const DWORD &dwId, WORD wHoleNum)
{
  if( m_mapCombineItemOdds.size() )
  {
    map<DWORD,CombineOdds*>::iterator It;
    if ( (It=m_mapCombineItemOdds.find(dwId))!=m_mapCombineItemOdds.end() )
    { 
      CombineOdds *pCombineOdds = It->second;
      if(wHoleNum>=0 && wHoleNum<=MAX_ITEM_SLOT) { return pCombineOdds->dwOddsHole[wHoleNum]; }
      else { return 0; /*pCombineOdds->dwOddsHole[0];*/ }
    }
    else
    {
      return 0;
    }
  }
  else
  {
    return 100;
  }
}
#endif
//===============================================================================================
//Add by zetorchen for Odds in Combine Item
//
int CSrvBaseData::LoadCombineItemOdds(char *szFilePath)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadCombineItemOdds");
  m_mapCombineItemOdds.clear();
  int iCount=0;
  int iTemp1,iTemp2;
#ifdef _REPAIR_SERVER_CRASH_NICK_
  SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, szFilePath);
  CInStream       CombineItemOdds( szInitLog );
  if ( CombineItemOdds.fail() && CombineItemOdds.GetFileSize() != 0 ) 
  {
#ifdef _DEBUG
    MessageBox( GetActiveWindow(), "Open CombineItemOdds.txt Error....", "Warning...", MB_OK );
#endif
    return 0;
  }
  CombineItemOdds >> iCount;
  if( iCount < 1 )
  {
    AddMemoErrMsg( "***** Load Combine Item Odds Error, The Count < 1 ! *****" );
    return 0;
  }
  for(int i=0; i<iCount; i++)
  {
    if ( CombineItemOdds >> iTemp1 >> iTemp2 ) 
    {
      m_mapCombineItemOdds[iTemp1]=iTemp2;
    }
    else 
    {
#ifdef _DEBUG
      MessageBox( GetActiveWindow(), "CombineItemOdds Error...","Warning", MB_OK );
#endif
      return 0;
    }
    
  }
  return 1;
#else
//
  FuncName("CSrvBaseData::LoadCombineItemOdds");
  m_mapCombineItemOdds.clear();
  FILE *pFile;
  int iCount=0;
  int iTemp1,iTemp2=0;
#ifdef _REPAIR_SERVER_CRASH_NICK_
  SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, szFilePath);
  pFile = fopen( szInitLog, "r" );
  if( !pFile )
  {
    return 0;
  }
  fscanf( pFile,"%d",&iCount );
  if( iCount < 1 )
  {
    AddMemoErrMsg( "***** Load Combine Item Odds Error, The Count < 1 ! *****" );
    return 0;
  }
#ifdef _NEW_COMBINE_SYSTEM_
  CombineOdds *pCombineOdds = NULL;
  for(int i=0; i<iCount; i++)
  {
    pCombineOdds = new CombineOdds;
    if( 5!=fscanf(pFile, "%d %d %d %d %d", 
                        &iTemp1, 
                        &(pCombineOdds->dwOddsHole[0]),
                        &(pCombineOdds->dwOddsHole[1]),
                        &(pCombineOdds->dwOddsHole[2]),
                        &(pCombineOdds->dwOddsHole[3])) )
    { 
      delete pCombineOdds;
      return 0;
    }
    m_mapCombineItemOdds.insert(map<DWORD, CombineOdds*>::value_type(iTemp1,pCombineOdds));
  }
#else
  for(int i=0; i<iCount; i++)
  {
    if(2!=fscanf( pFile, "%d %d", &iTemp1,&iTemp2 )) return 0;
    m_mapCombineItemOdds[iTemp1]=iTemp2;
  }
#endif

  return 1;
#endif//_DEBUG_JAPAN_DECRYPT_
}
#endif
//===============================================================================================
//
//
#ifdef _VERSION40_CARRYDOOR
int CSrvBaseData::LoadBaseCarryDoor(char *szFilePath)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseCarryDoor");
  g_pDoorManager = new CDoorManager;
  m_listCarryDoor.clear();
  CCarryDoor * pCarryDoor = NULL;
  SMapPoint *  pMp = NULL;
  WORD wCount = 0;//传送门总数
  WORD wAimCount = 0;//到达点树目，至少为1
  WORD wNumPerGroup = 0;
  WORD wGroupNum = 0;
  DWORD dwValue[7] = {0,0,0,0,0};//
  DWORD dwAimValue[3] = {0,0,0};//暂存目标点
  strcpy(szInitLog, m_szObjFile);
  strcat(szInitLog, "/");
  strcat(szInitLog, szFilePath);
  
  CInStream CarryDoor(szInitLog);
  if (CarryDoor.fail()||CarryDoor.GetFileSize()==0)
    return 0;

  CarryDoor>>wCount;
  if ( wCount<1 ) 
  {
    AddMemoErrMsg( "***** Load Carry Door Error, The Count < 1 ! *****" );
    return 0;
  }
  for(int iLoop = 0; iLoop < wCount; iLoop++)
  {
    pCarryDoor = new CCarryDoor;
    CarryDoor>>dwValue[0]>>dwValue[1]
             >>dwValue[2]>>dwValue[3]
             >>dwValue[4]>>dwValue[5]
             >>dwValue[6];

      pCarryDoor->SetMailId(dwValue[0]);
      pCarryDoor->SetType(dwValue[1]);
      pCarryDoor->SetStatus(dwValue[2]);
      pCarryDoor->SetTimeCtlType(dwValue[3]);
      pCarryDoor->SetCarryPoint(dwValue[4], dwValue[5], dwValue[6]);

    CarryDoor>>wAimCount;
    if( wAimCount < 1)
      return 0;
    else
    {
      pCarryDoor->SetAimNum(wAimCount);
      for(int i = 0; i < wAimCount; i++)
      {
        pMp = new SMapPoint;
        CarryDoor>>dwAimValue[0]>>dwAimValue[1]>>dwAimValue[2];

        pMp->MapId = dwAimValue[0];
        pMp->x = dwAimValue[1];
        pMp->y = dwAimValue[2];
        pCarryDoor->m_vecAimMp.push_back(*pMp); 
      }
    }
    m_listCarryDoor.push_back(pCarryDoor); 
  } 

  CarryDoor>>wGroupNum;
  if(wGroupNum < 1)
  {
    return 0;
  }
  else
  {
    CarryDoor>>wNumPerGroup;
    WORD wSize = wGroupNum*wNumPerGroup;
    g_pVar = new WORD[wSize];
    for(int i = 0; i < wSize; i++)
    {
      CarryDoor>>g_pVar[i];
    }
  }
  for(list<CCarryDoor*>::iterator iter_CDoor = m_listCarryDoor.begin(); iter_CDoor != m_listCarryDoor.end(); iter_CDoor++ )
  {
    g_pDoorManager->m_vecCarryDoor.push_back(*iter_CDoor);
  }
  DWORD dwTimeValue[3] = {0,0,0};

  CarryDoor>>dwTimeValue[0]>>dwTimeValue[1]>>dwTimeValue[2];

  g_pDoorManager->m_dwTimeOut = dwTimeValue[0];
  g_pDoorManager->m_dwSelfInter = dwTimeValue[1];
  g_pDoorManager->m_dwTimeOutSelf = dwTimeValue[2];
  g_pDoorManager->m_wDoorNum = wNumPerGroup;
  g_pDoorManager->m_wGroupNum = wGroupNum;
  g_pDoorManager->m_pwMailId = g_pVar;
  
  return 1;
#else
  FuncName("CSrvBaseData::LoadBaseCarryDoor");
  g_pDoorManager = new CDoorManager;
  m_listCarryDoor.clear();
  FILE * fp;
  CCarryDoor * pCarryDoor = NULL;
  SMapPoint *  pMp = NULL;
  WORD wCount = 0;//传送门总数
  WORD wAimCount = 0;//到达点树目，至少为1
  WORD wNumPerGroup = 0;
  WORD wGroupNum = 0;
  DWORD dwValue[7] = {0,0,0,0,0};//
  DWORD dwAimValue[3] = {0,0,0};//暂存目标点
  strcpy(szInitLog, m_szObjFile);
  strcat(szInitLog, "/");
  strcat(szInitLog, szFilePath);
  fp = fopen(szInitLog,"r");
  if(NULL == fp)
  {
    return 0;
  }
  fscanf(fp,"%hd",&wCount);
  if( wCount < 1 )
  {
    AddMemoErrMsg( "***** Load Carry Door Error, The Count < 1 ! *****" );
    return 0;
  }
  for(int iLoop = 0; iLoop < wCount; iLoop++)
  {
    pCarryDoor = new CCarryDoor;
    if(7 != fscanf(fp,"%d %d %d %d %d %d %d",
      &dwValue[0],&dwValue[1],&dwValue[2],&dwValue[3],&dwValue[4] ,&dwValue[5], &dwValue[6]) )
    {
      return 0;
    }
    else
    {
      pCarryDoor->SetMailId(dwValue[0]);
      pCarryDoor->SetType(dwValue[1]);
      pCarryDoor->SetStatus(dwValue[2]);
      pCarryDoor->SetTimeCtlType(dwValue[3]);
      pCarryDoor->SetCarryPoint(dwValue[4], dwValue[5], dwValue[6]);
    }
    if(1 != fscanf(fp,"%hd",&wAimCount) )
      return 0;
    else
    {
      if( wAimCount < 1)
        return 0;
      else
      {
        pCarryDoor->SetAimNum(wAimCount);
        for(int i = 0; i < wAimCount; i++)
        {
          pMp = new SMapPoint;
          if(3 != fscanf(fp,"%d %d %d",&dwAimValue[0],&dwAimValue[1],&dwAimValue[2]))
            return 0;
          else
          {
            pMp->MapId = dwAimValue[0];
            pMp->x = dwAimValue[1];
            pMp->y = dwAimValue[2];
            pCarryDoor->m_vecAimMp.push_back(*pMp);
            
          }
        }
      }
    }
    m_listCarryDoor.push_back(pCarryDoor); 
  }
  if(1 != fscanf(fp,"%hd", &wGroupNum) )
    return 0;
  else
  {
    if(wGroupNum < 1)
    {
      return 0;
    }
    else
    {
      if(1 != fscanf(fp,"%hd",&wNumPerGroup))
        return 0;
      //    char szFormat[32] = {"%d"};
      //    for(int j = 1; j < wNumPerGroup; j++)
      //    {
      //     strcat(szFormat," %d");
      //    }
      WORD wSize = wGroupNum*wNumPerGroup;
      g_pVar = new WORD[wSize];
      for(int i = 0; i < wSize; i++)
      {
        if(1 != fscanf(fp,"%hd ",&g_pVar[i]))
          return 0;
      }
    }
  }
  for(list<CCarryDoor*>::iterator iter_CDoor = m_listCarryDoor.begin(); iter_CDoor != m_listCarryDoor.end(); iter_CDoor++ )
  {
    g_pDoorManager->m_vecCarryDoor.push_back(*iter_CDoor);
  }
  DWORD dwTimeValue[3] = {0,0,0};
  if(3 != fscanf(fp,"%d %d %d",&dwTimeValue[0],&dwTimeValue[1],&dwTimeValue[2]))
    return 0;
  g_pDoorManager->m_dwTimeOut = dwTimeValue[0];
  g_pDoorManager->m_dwSelfInter = dwTimeValue[1];
  g_pDoorManager->m_dwTimeOutSelf = dwTimeValue[2];
  g_pDoorManager->m_wDoorNum = wNumPerGroup;
  g_pDoorManager->m_wGroupNum = wGroupNum;
  g_pDoorManager->m_pwMailId = g_pVar;
  fclose(fp);
  return 1;
#endif//_DEBUG_JAPAN_DECRYPT_
}
#endif
//===============================================================================================
// 
//
int CSrvBaseData::LoadMonsterDropItem(char *szFilePath)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
	FuncName("CSrvBaseData::LoadMonsterDropItem");
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else 
	strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  CInStream MonsterDropItem( szInitLog );
//  CInStream MonsterDropItem(".\\GameData\\Monster\\MonsterFixItem.txt");
  CSrvDropItem   *pDropItem = NULL;
  int            iCount     = 0;
  if( !MonsterDropItem.fail() && MonsterDropItem.GetFileSize != 0 )
  {
    MonsterDropItem >> iCount;
    if( iCount < 1 )
    {
      AddMemoErrMsg( "***** Load Monster Drop Item Error, The Count = Zero ! *****" );
      return 0;
    }
    // Check Error
    if( NULL == ( g_pDropItem->m_pDropItem = new CSrvDropItem[iCount] ) )
    {
      AddMemoErrMsg( "***** Load Monster Drop Item Error, Cannot New CSrvDropItem ! *****" );
      return 0;
    }
    g_pDropItem->m_dwCount = 0;
    for( int iNumber = 1; iNumber <= iCount; iNumber++ )
    {
      pDropItem = &(g_pDropItem->m_pDropItem[iNumber-1]);
      //
      MonsterDropItem >> pDropItem->m_wId;
      MonsterDropItem >> pDropItem->m_wItemId;
      MonsterDropItem >> pDropItem->m_iAp;
      MonsterDropItem >> pDropItem->m_iDp;
      MonsterDropItem >> pDropItem->m_iHit;
      MonsterDropItem >> pDropItem->m_iDodge;
      MonsterDropItem >> pDropItem->m_iInt;
      MonsterDropItem >> pDropItem->m_wLevel;
      MonsterDropItem >> pDropItem->m_wApUp;
      MonsterDropItem >> pDropItem->m_wDpUp;
      MonsterDropItem >> pDropItem->m_wHitUp;
      MonsterDropItem >> pDropItem->m_wDodgeUp;
      MonsterDropItem >> pDropItem->m_wIntUp;
      MonsterDropItem >> pDropItem->m_wApFixLevel;
      MonsterDropItem >> pDropItem->m_wDpFixLevel;
      MonsterDropItem >> pDropItem->m_wHitFixLevel;
      MonsterDropItem >> pDropItem->m_wDodgeFixLevel;
      MonsterDropItem >> pDropItem->m_wIntFixLevel;
      MonsterDropItem >> pDropItem->m_wApUpFix;
      MonsterDropItem >> pDropItem->m_wDpUpFix;
      MonsterDropItem >> pDropItem->m_wHitUpFix;
      MonsterDropItem >> pDropItem->m_wDodgeUpFix;
      MonsterDropItem >> pDropItem->m_wIntUpFix;
      MonsterDropItem >> pDropItem->m_wCriticalHit;
      MonsterDropItem >> pDropItem->m_wHoleNumber;
      MonsterDropItem >> pDropItem->m_wMaxDurability;
      MonsterDropItem >> pDropItem->m_wDurability;
      MonsterDropItem >> pDropItem->m_wDurabilityUp;
      MonsterDropItem >> pDropItem->m_wForging;
      MonsterDropItem >> pDropItem->m_wForgingDblUp;
      MonsterDropItem >> pDropItem->m_wHard;
      MonsterDropItem >> pDropItem->m_wStable;
      MonsterDropItem >> pDropItem->m_wFuncDbc;
      MonsterDropItem >> pDropItem->m_iEffectTime;
      MonsterDropItem >> pDropItem->m_wFuncEffect;
      MonsterDropItem >> pDropItem->m_iAlarmTime;
      MonsterDropItem >> pDropItem->m_wFuncAlarm;
      MonsterDropItem >> pDropItem->m_iTriggerTime;
      MonsterDropItem >> pDropItem->m_wFuncTrigger;
      MonsterDropItem >> pDropItem->m_wFuncEquip;
      MonsterDropItem >> pDropItem->m_wFuncTakeOff;
      MonsterDropItem >> pDropItem->m_wFuncTouch;
      MonsterDropItem >> szInitLog;
      //
      if( iNumber != pDropItem->m_wId )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Drop Item List Error, The Id(%d) Isn't Consecutive ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg( szInitLog );
#endif
        return -1;
      }
      // check
      if( m_mapBaseItem.find(pDropItem->m_wItemId ) == m_mapBaseItem.end() )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Drop Item List Error, The Item(%d) Isn't Exist ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg( szInitLog );
#endif
        return -1;
      }
      // Insert The Drop Item
      g_pDropItem->m_listDropItem.push_back( pDropItem );
      g_pDropItem->m_dwCount += 1;
    }
  }
  else
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Drop Item Error (Id=%d), Scan File Data Error ! *****", iNumber );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szInitLog );
#endif
    return 0;
  }
return 1;
#else
//
	FuncName("CSrvBaseData::LoadMonsterDropItem");

	CSrvDropItem		*pDropItem = NULL;
	int							iCount = 0, iNumber = 1;
	FILE						*pFile;
  int             dwDropItem[42];

	// Get File Full Path And Open File
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  pFile = fopen( szInitLog, "r" );
  if( !pFile )
  {
    return 0;
  }
	// Get Monster Drop Item Data
	fscanf( pFile,"%d ",&iCount );
	// Check Error
	if( iCount < 1 )
	{
		AddMemoErrMsg( "***** Load Monster Drop Item Error, The Count = Zero ! *****" );
		return 0;
	}
	// Check Error
	if( NULL == ( g_pDropItem->m_pDropItem = new CSrvDropItem[iCount] ) )
	{
		AddMemoErrMsg( "***** Load Monster Drop Item Error, Cannot New CSrvDropItem ! *****" );
		return 0;
	}
	g_pDropItem->m_dwCount = 0;

	// Load File Data
	for( iNumber = 1; iNumber <= iCount; iNumber++ )
	{
		pDropItem = &(g_pDropItem->m_pDropItem[iNumber-1]);
    ZeroMemory( dwDropItem, sizeof( dwDropItem ) );
    if( 43 == fscanf( pFile, "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %s",
                             &dwDropItem[0],  &dwDropItem[1],  &dwDropItem[2],  &dwDropItem[3],  &dwDropItem[4],
                             &dwDropItem[5],  &dwDropItem[6],  &dwDropItem[7],  &dwDropItem[8],  &dwDropItem[9],
                             &dwDropItem[10], &dwDropItem[11], &dwDropItem[12], &dwDropItem[13], &dwDropItem[14],
                             &dwDropItem[15], &dwDropItem[16], &dwDropItem[17], &dwDropItem[18], &dwDropItem[19],
                             &dwDropItem[20], &dwDropItem[21], &dwDropItem[22], &dwDropItem[23], &dwDropItem[24],
                             &dwDropItem[25], &dwDropItem[26], &dwDropItem[27], &dwDropItem[28], &dwDropItem[29],
                             &dwDropItem[30], &dwDropItem[31], &dwDropItem[32], &dwDropItem[33], &dwDropItem[34],
                             &dwDropItem[35], &dwDropItem[36], &dwDropItem[37], &dwDropItem[38], &dwDropItem[39],
                             &dwDropItem[40], &dwDropItem[41], szInitLog ) )
		{
			pDropItem->m_wId              = dwDropItem[0];
      pDropItem->m_wItemId          = dwDropItem[1];
      pDropItem->m_iAp              = dwDropItem[2];
      pDropItem->m_iDp              = dwDropItem[3];
      pDropItem->m_iHit             = dwDropItem[4];
			pDropItem->m_iDodge           = dwDropItem[5];
      pDropItem->m_iInt             = dwDropItem[6];
      pDropItem->m_wLevel           = dwDropItem[7];
      pDropItem->m_wApUp            = dwDropItem[8];
      pDropItem->m_wDpUp            = dwDropItem[9];
			pDropItem->m_wHitUp           = dwDropItem[10];
      pDropItem->m_wDodgeUp         = dwDropItem[11];
      pDropItem->m_wIntUp           = dwDropItem[12];
      pDropItem->m_wApFixLevel      = dwDropItem[13];
      pDropItem->m_wDpFixLevel      = dwDropItem[14];
			pDropItem->m_wHitFixLevel     = dwDropItem[15];
      pDropItem->m_wDodgeFixLevel   = dwDropItem[16];
      pDropItem->m_wIntFixLevel     = dwDropItem[17];
      pDropItem->m_wApUpFix         = dwDropItem[18];
      pDropItem->m_wDpUpFix         = dwDropItem[19];
			pDropItem->m_wHitUpFix        = dwDropItem[20];
      pDropItem->m_wDodgeUpFix      = dwDropItem[21];
      pDropItem->m_wIntUpFix        = dwDropItem[22];
      pDropItem->m_wCriticalHit     = dwDropItem[23];
      pDropItem->m_wHoleNumber      = dwDropItem[24];
			pDropItem->m_wMaxDurability   = dwDropItem[25];
      pDropItem->m_wDurability      = dwDropItem[26];
      pDropItem->m_wDurabilityUp    = dwDropItem[27];
      pDropItem->m_wForging         = dwDropItem[28];
      pDropItem->m_wForgingDblUp    = dwDropItem[29];
			pDropItem->m_wHard            = dwDropItem[30];
      pDropItem->m_wStable          = dwDropItem[31];
      pDropItem->m_wFuncDbc         = dwDropItem[32];
      pDropItem->m_iEffectTime      = dwDropItem[33];
      pDropItem->m_wFuncEffect      = dwDropItem[34];
			pDropItem->m_iAlarmTime       = dwDropItem[35];
      pDropItem->m_wFuncAlarm       = dwDropItem[36];
      pDropItem->m_iTriggerTime     = dwDropItem[37];
      pDropItem->m_wFuncTrigger     = dwDropItem[38];
      pDropItem->m_wFuncEquip       = dwDropItem[39];
			pDropItem->m_wFuncTakeOff     = dwDropItem[40];
      pDropItem->m_wFuncTouch       = dwDropItem[41];
      // Load Ok
			if( iNumber != pDropItem->m_wId )
			{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Drop Item List Error, The Id(%d) Isn't Consecutive ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg( szInitLog );
#endif
				return -1;
			}
			// check
			if( m_mapBaseItem.find(pDropItem->m_wItemId ) == m_mapBaseItem.end() )
			{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Drop Item List Error, The Item(%d) Isn't Exist ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg( szInitLog );
#endif
				return -1;
			}
			// Insert The Drop Item
			g_pDropItem->m_listDropItem.push_back( pDropItem );
			g_pDropItem->m_dwCount += 1;
		}
		else
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Drop Item Error (Id=%d), Scan File Data Error ! *****", iNumber );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return 0;
		}
	}
	fclose( pFile );
	return 1;
#endif //_DEBUG_JAPAN_DECRYPT_
}
//================================================================================================
//
//add by jack.ren for 4.0
#ifdef MARKITEM_40_MARKRULST
int CSrvBaseData::LoadBaseMarkItem(char *szFilePath)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseMarkItem");
  int               iMarkItem[2];
  int               iCount;
  string strFileName = m_szObjFile;
  strFileName += '/';
  strFileName += szFilePath;  
  CInStream MarkItem((char*)strFileName.c_str());
  if (!MarkItem.fail() && MarkItem.GetFileSize() != 0) 
  {
    MarkItem >> iCount >> iMarkItem[0] >> iMarkItem[1];
    g_iMarkItemId   = iMarkItem[0];
    g_iMarkItemCost = iMarkItem[1];
    if (iCount != 1)
    {
      AddMemoErrMsg( "***** Load Base MarkItem Error, Scan Count Error ! *****" );
      return 0;   
    }
    else
      return 1;    
  }
  else 
    return 0;
#else
  FuncName("CSrvBaseData::LoadBaseMarkItem");
  FILE              *pFile;
  int               iMarkItem[2];
  int               iCount;
  string strFileName = m_szObjFile;
  strFileName += '/';
  strFileName += szFilePath;
  pFile = fopen( strFileName.c_str(), "r" );
  if( !pFile )
  {
    return false;
  }
  if( 1 != fscanf( pFile, "%d ", &iCount ) )
  {
    AddMemoErrMsg( "***** Load Base MarkItem Error, Scan Count Error ! *****" );
    fclose( pFile );
    return 0;
  }
  if( 2 == fscanf( pFile, "%d %d", &iMarkItem[0], &iMarkItem[1] ) )
  {
    g_iMarkItemId   = iMarkItem[0];
    g_iMarkItemCost = iMarkItem[1];   
  }
  fclose( pFile );
  return 1;
#endif
}
#endif
//
//
#ifdef _AUTO_RUN_CITY_WAR_
int CSrvBaseData::LoadAutoCityWar(char *szFilePath)
{
  FuncName("CSrvBaseData::LoadAutoCityWar");
  FILE      *pFile;
  int       iCityWar[5];
  int       iCount;

  string strFileName = m_szObjFile;
  strFileName += '/';
  strFileName += szFilePath;
  //
  pFile = fopen(strFileName.c_str(), "r");
  if(!pFile)
  {
    return 0;
  }
  if(1 != fscanf(pFile, "%d ", &iCount))
  {
    AddMemoErrMsg( "***** Load Base Auto Run City War Error 1, Scan Count Error ! *****" );
    fclose(pFile);
    return 0;
  }
  //
  if(5 == fscanf(pFile, "%d %d %d %d %d", &iCityWar[0], &iCityWar[1], &iCityWar[2], &iCityWar[3], &iCityWar[4]))
  {
    g_iAutoWeek     = iCityWar[0];
    g_iAutoHour     = iCityWar[1];
    g_iAutoMinute   = iCityWar[2];
    g_iAutoStanding = iCityWar[3];
    g_iAutoMapId    = iCityWar[4];
    //
    fclose( pFile );
    return 1; 
  }
  return 0;      
}
#endif//_AUTO_RUN_CITY_WAR_

#ifdef _AUTO_ADD_WARP_POINT_
int CSrvBaseData::LoadAutoAddWarpPoint(char *szFilePath)
{
  FuncName("CSrvBaseData::LoadAutoAddWarpPoint");
  int           iCount;
  int           iTime[4];
  int           iTemp;
  WORD          wWarpPointPram[8];
  FILE          *pFile;
  _AddWarpPoint *pAddPoint = NULL;
  //
  string strFileName = m_szObjFile;
  strFileName += '/';
  strFileName += szFilePath;
  // 
  pFile = fopen(strFileName.c_str(), "r");
  if(!pFile) { return 0; }
  //
  if(1 != fscanf(pFile, "%d ", &iCount))
  {
    AddMemoErrMsg( "***** Load Add Warp Point Error 1, Scan Count Error ! *****" );
    fclose(pFile);
    return 0;
  }
  //
  for(int i=0; i<iCount; i++)
  {
    if( 13 == fscanf(pFile, "%d %d %d %d %d %hd %hd %hd %hd %hd %hd %hd %hd",
                            &iTemp, &iTime[0], &iTime[1], &iTime[2], &iTime[3],   // Run Time 
                            &wWarpPointPram[0], &wWarpPointPram[1],
                            &wWarpPointPram[2], &wWarpPointPram[3],
                            &wWarpPointPram[4], &wWarpPointPram[5], 
                            &wWarpPointPram[6], &wWarpPointPram[7]) )
    {
      if(iTime[0]>6 || iTime[1]>24 || iTime[2]>60) //Check Time
      { 
        fclose(pFile);
        return 0;
      }
      //
      pAddPoint = new(nothrow) _AddWarpPoint;  // new 失败不抛处异常返回NULL
      if(pAddPoint == NULL) 
      { 
        fclose(pFile);
        return 0 ;
      }
      //
      pAddPoint->iRunTime[0] = iTime[0];
      pAddPoint->iRunTime[1] = iTime[1];
      pAddPoint->iRunTime[2] = iTime[2];
      pAddPoint->iRunTime[3] = iTime[3];
      pAddPoint->wPointParameter[0] = wWarpPointPram[0];
      pAddPoint->wPointParameter[1] = wWarpPointPram[1];
      pAddPoint->wPointParameter[2] = wWarpPointPram[2];
      pAddPoint->wPointParameter[3] = wWarpPointPram[3];
      pAddPoint->wPointParameter[4] = wWarpPointPram[4];
      pAddPoint->wPointParameter[5] = wWarpPointPram[5];
      pAddPoint->wPointParameter[6] = wWarpPointPram[6];
      pAddPoint->wPointParameter[7] = wWarpPointPram[7];
      //
      g_mapAddWarpPiont.insert(map<int, _AddWarpPoint*>::value_type(iTemp, pAddPoint));
    }
  }
  fclose(pFile);
  return 1;
}
#endif //_AUTO_ADD_WARP_POINT_
//
//
#ifdef FUNCTION_MAKEHOLE_ITEM
int CSrvBaseData::LoadMixHoleItem(char *szFilePath)
{
  FuncName("CSrvBaseData::LoadMixHoleItem");
  FILE              *pFile;
  int               iMixHoleItem[2];
  int               iHoleItem[2];
  int                iCount;
  string strFileName = m_szObjFile;
  strFileName += '/';
  strFileName += szFilePath;
  pFile = fopen( strFileName.c_str(), "r" );

  if( !pFile )
  {
    return false;
  }
  if( 1 != fscanf( pFile, "%d ", &iCount ) )
  {
    AddMemoErrMsg( "***** Load Base MixHoleItem Error, Scan Count Error ! *****" );
    fclose( pFile );
    return 0;
  }
  //读入开第一孔和第二孔的几率
  if(2==fscanf(pFile, "%d %d", &iHoleItem[0], &iHoleItem[1])) 
  {
    g_iMakeFirstHole = iHoleItem[0];
    g_iMakeSeconHole = iHoleItem[1];
    //
    m_mapMixHoleItem.clear();
    for(int i=0; i<iCount-1; i++) 
    {
      if( 2 == fscanf( pFile, "%d %d", &iMixHoleItem[0], &iMixHoleItem[1] ) )
      {
        m_mapMixHoleItem.insert(map<int, int>::value_type(iMixHoleItem[0], iMixHoleItem[1]));    
      }
    }
  }
  fclose( pFile );
  return 1;
}
#endif
//===============================================================================================
//
//
int CSrvBaseData::LoadBaseMonster(char *szFilePath1, char *szFilePath2 )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
	FuncName("CSrvBaseData::LoadBaseMonster");
	CSrvTmpBaseMonster	*pBaseMonster = NULL, *pTemp;
	CSrvBaseMonster			*pMonster;
  DWORD								dwId = 0, dwBaseId = 0;
 	int									iCount1 = 0, iCount2 = 0, iNumber = 1;
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath1 );
  CInStream           BaseMonster( szInitLog );

  if ( !BaseMonster.fail() && BaseMonster.GetFileSize() != 0 ) 
  {
    BaseMonster >> iCount1;
    if ( iCount1 <=1 ) 
    {
      MessageBox( GetActiveWindow(), " The iCount in The BaseMonster.txt Error", "Warning...", MB_OK );
      return 0; 
    }
    pBaseMonster = new CSrvTmpBaseMonster[iCount1];
    if( pBaseMonster == NULL )
    {
      AddMemoErrMsg( "***** Load Basest Monster Error, Cannot New CSrvTmpBaseMonster ! *****" );
      return 0;
    }
    for ( iNumber = 1; iNumber<=iCount1; iNumber++ ) 
    {
      pTemp = &(pBaseMonster[iNumber-1]);
      //
      BaseMonster >> pTemp->m_wId;
      BaseMonster >> pTemp->m_szName;
      BaseMonster >> pTemp->m_wPicId;
      BaseMonster >> pTemp->m_wElement;
      BaseMonster >> pTemp->m_dwRaceAttri;
      BaseMonster >> pTemp->m_iHp;
      BaseMonster >> pTemp->m_iAp;
      BaseMonster >> pTemp->m_iDp;
      BaseMonster >> pTemp->m_iHit;
      BaseMonster >> pTemp->m_iDodge;
      BaseMonster >> pTemp->m_iInt;
      BaseMonster >> pTemp->m_wHard;
      BaseMonster >> pTemp->m_iHpUp;
      BaseMonster >> pTemp->m_iApUp;
      BaseMonster >> pTemp->m_iDpUp;
      BaseMonster >> pTemp->m_iHitUp;
      BaseMonster >> pTemp->m_iDodgeUp;
      BaseMonster >> pTemp->m_iIntUp;
      BaseMonster >> pTemp->m_iApBonuRand;
      BaseMonster >> pTemp->m_wSize;
      BaseMonster >> pTemp->m_iNpcProperty;
      BaseMonster >> pTemp->m_iAcceptTable;
      BaseMonster >> pTemp->m_wSpeed;
      BaseMonster >> pTemp->m_wSightRange;
      BaseMonster >> pTemp->m_wHelpRange;
      BaseMonster >> pTemp->m_wPursueRange;
      BaseMonster >> pTemp->m_wPursueSpeed;
      BaseMonster >> pTemp->m_wEscapeSpeed;
      BaseMonster >> pTemp->m_wHaltRange;
      BaseMonster >> szInitLog;
      pTemp->m_szName[MAX_NPC_NAME_LEN-1] = '\0';
      // Check Id
      if( pTemp->m_wId != iNumber )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Basest Monster Error, The Id=%d Isn't Consecutive ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg( szInitLog );
#endif
        return 0;
      }
      
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot Open The BaseMonster.txt", "Warning...", MB_OK );
    return 0;
  }
  //
  #ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath2 );
  CInStream     BaseMonster2( szInitLog );

  DWORD         dwTemp[4];
  
  if ( !BaseMonster2.fail() && BaseMonster2.GetFileSize() != 0 ) 
  {
    BaseMonster2 >> iCount2;
    if ( iCount2<=1 ) 
    {
      MessageBox( GetActiveWindow(), "The iCount in the Monster.txt File Error", "Warning", MB_OK);
      return 0;
    }
    for ( iNumber = 1; iNumber<=iCount2; iNumber++ ) 
    {
      BaseMonster2 >> dwId >> dwBaseId;
      if( dwBaseId <= iCount1 )
      {
        pMonster = new CSrvBaseMonster( &(pBaseMonster[dwBaseId-1]), dwId );
      }
      else
      {
        AddMemoErrMsg( "***** Load Base Monster Error, Monster Base Id > Basest Monster Id List ! *****" );
        return 0;
      }
      //
      BaseMonster2 >> pMonster->m_wLevel;
      BaseMonster2 >> pMonster->m_dwExp;
      BaseMonster2 >> pMonster->m_wExpLtLv;
      BaseMonster2 >> pMonster->m_iEquipAp;
      BaseMonster2 >> pMonster->m_iEquipHit;
      BaseMonster2 >> pMonster->m_iEquipDp;
      BaseMonster2 >> pMonster->m_iEquipDodge;
      BaseMonster2 >> pMonster->m_iEquipInt;
      BaseMonster2 >> pMonster->m_iBearPsb;
      BaseMonster2 >> dwTemp[0];
      BaseMonster2 >> dwTemp[1];
      pMonster->m_qwAntiStatus = MAKEQWORD(dwTemp[0],dwTemp[1]);
      BaseMonster2 >> pMonster->m_dwMoney;
      BaseMonster2 >> pMonster->m_wRace;
      BaseMonster2 >> pMonster->m_wSoul;
      BaseMonster2 >> pMonster->m_wCommand;
      BaseMonster2 >> pMonster->m_dwReviveTime;
      BaseMonster2 >> pMonster->m_dwBoneTime;
      BaseMonster2 >> pMonster->m_dwDelTime;
      BaseMonster2 >> pMonster->m_dwLifeTime;
      BaseMonster2 >> pMonster->m_wLevelLimit;
      BaseMonster2 >> pMonster->m_wFuncLbc;
      BaseMonster2 >> pMonster->m_wFuncEvent;
      BaseMonster2 >> pMonster->m_wFuncInit;
      BaseMonster2 >> pMonster->m_wFuncLvUp;
      BaseMonster2 >> pMonster->m_wFuncDef;
      BaseMonster2 >> pMonster->m_wFuncDied;
      BaseMonster2 >> pMonster->m_wSkillRate[0];
      BaseMonster2 >> pMonster->m_wSkill[0];
      BaseMonster2 >> pMonster->m_wSkillRate[1];
      BaseMonster2 >> pMonster->m_wSkill[1];
      BaseMonster2 >> pMonster->m_wSkillRate[2];
      BaseMonster2 >> pMonster->m_wSkill[2];
      BaseMonster2 >> pMonster->m_wSkillRate[3];
      BaseMonster2 >> pMonster->m_wSkill[3];
      BaseMonster2 >> pMonster->m_wSkillRate[4];
      BaseMonster2 >> pMonster->m_wSkill[4];
      BaseMonster2 >> pMonster->m_wDropRate[0];
      BaseMonster2 >> pMonster->m_wDropBegin[0];
      BaseMonster2 >> pMonster->m_wDropEnd[0];
      BaseMonster2 >> pMonster->m_wDropRate[1];
      BaseMonster2 >> pMonster->m_wDropBegin[1];
      BaseMonster2 >> pMonster->m_wDropEnd[1];
      BaseMonster2 >> pMonster->m_wDropRate[2];
      BaseMonster2 >> pMonster->m_wDropBegin[2];
      BaseMonster2 >> pMonster->m_wDropEnd[2];
      BaseMonster2 >> pMonster->m_wDropRate[3];
      BaseMonster2 >> pMonster->m_wDropBegin[3];
      BaseMonster2 >> pMonster->m_wDropEnd[3];
      BaseMonster2 >> pMonster->m_wDropRate[4];
      BaseMonster2 >> pMonster->m_wDropBegin[4];
      BaseMonster2 >> pMonster->m_wDropEnd[4];
      BaseMonster2 >> pMonster->m_wSoulProb;
      BaseMonster2 >> dwTemp[2];
      BaseMonster2 >> dwTemp[3];
      pMonster->m_dwDeadEvent = ( ( dwTemp[2] & 0x0000FFFF ) << 16 ) | ( dwTemp[3] & 0x0000FFFF );
      BaseMonster2 >> pMonster->m_wParam1;
      BaseMonster2 >> pMonster->m_iParam2;
      BaseMonster2 >> szInitLog;    
      //
      if( iNumber != pMonster->m_wBaseId )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Error, The Id=%d Isn't Consecutive ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg( szInitLog );
#endif
        return 0;
      }
      // Init Base Data, Include Skill, Drop Item List And All Functions
      if( pMonster->m_iNpcProperty & NPC_ATTRI_ESCAPE10 )
      {
        pMonster->m_wEscapeHp = pMonster->m_iHp / 10;
      }
      else
      {
        pMonster->m_wEscapeHp = 0;
      }
      if( !( pMonster->m_iNpcProperty & NPC_ATTRI_UNRESET ) )
      {
        pMonster->m_wDropRate[0] = pMonster->m_wDropRate[0];  
      }
      pMonster->m_wSkillRate[0] *= 100;
      pMonster->m_wSkillRate[1] *= 100;
      pMonster->m_wSkillRate[2] *= 100;
      pMonster->m_wSkillRate[3] *= 100;
      pMonster->m_wSkillRate[4] *= 100;
      //
      pMonster->m_wElement      = pBaseMonster[dwBaseId-1].m_wElement;
      pMonster->m_dwRaceAttri   = pBaseMonster[dwBaseId-1].m_dwRaceAttri;
      //
      pMonster->m_dwReviveTime *= 1000;
      pMonster->m_dwBoneTime   *= 1000;
      pMonster->m_dwDelTime		 *= 1000;
      pMonster->m_dwLifeTime	 *= 1000;
      
      pMonster->m_iHp			=  pBaseMonster[dwBaseId-1].m_iHp +
        ( pBaseMonster[dwBaseId-1].m_iHpUp * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iHpUp & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_iAp			=  pBaseMonster[dwBaseId-1].m_iAp +
        ( pBaseMonster[dwBaseId-1].m_iApUp * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iApUp & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_iDp			=  pBaseMonster[dwBaseId-1].m_iDp +
        ( pBaseMonster[dwBaseId-1].m_iDpUp * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iDpUp & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_iHit		=  pBaseMonster[dwBaseId-1].m_iHit +
        ( pBaseMonster[dwBaseId-1].m_iHitUp * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iHitUp & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_iDodge  =  pBaseMonster[dwBaseId-1].m_iDodge +
        ( pBaseMonster[dwBaseId-1].m_iDodgeUp * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iDodgeUp & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_iInt		=  pBaseMonster[dwBaseId-1].m_iInt +
        ( pBaseMonster[dwBaseId-1].m_iIntUp * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iIntUp & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_iApBonuRand = ( pBaseMonster[dwBaseId-1].m_iApBonuRand * pMonster->m_wLevel / 100 ) +
        ( ( pBaseMonster[dwBaseId-1].m_iApBonuRand & 0x63 ) * pMonster->m_wLevel / 100 );
      
      pMonster->m_wExpLtLvHalf = pMonster->m_wExpLtLv >> 1;
      //
      for( int i=0; i < MAX_MONSTER_SKILL; i++ )
      {
        if( pMonster->m_wSkill[i] && m_mapBaseSkill.find( pMonster->m_wSkill[i] ) == m_mapBaseSkill.end() )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Base Monster Error, Skill(Id=%d) is not Exist! *****", pMonster->m_wSkill[i] );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szInitLog );
#endif
          return 0;
        }
      }
      // Insert The Monster
      m_mapBaseMonster.insert( map<DWORD,CSrvBaseMonster*>::value_type( pMonster->m_wId, pMonster ) );      
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot OPen BaseMonster.txt", "Warning...", MB_OK );
    return 0;
  }
  delete[] pBaseMonster;
	return 1;
#else
  //
	FuncName("CSrvBaseData::LoadBaseMonster");

	CSrvTmpBaseMonster	*pBaseMonster = NULL, *pTemp;
	CSrvBaseMonster			*pMonster;
	int									iCount1 = 0, iCount2 = 0, iNumber = 1;
	DWORD								dwId = 0, dwBaseId = 0;
	FILE								*pFile;
  char                szMonsterName[1024];
  int                 dwMonsterData[56];
  DWORD               dwTemp[2];

	// Get File Full Path And Open File
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath1 );
  pFile = fopen( szInitLog, "r" );
  if( !pFile )
  {
    return false;
  }
	// 
	if( 1 != fscanf( pFile, "%d ", &iCount1 ) )
	{
		AddMemoErrMsg( "***** Load Basest Monster Error, Scan Count Error ! *****" );
		return 0;
	}
	// Check Error
	if( iCount1 < 1 )
	{
		AddMemoErrMsg( "***** Load Basest Monster Error, The Count < 1 ! *****" );
		return 0;
	}
	// Get Basest Monster Data Before Get Base Monster Data
	pBaseMonster = new CSrvTmpBaseMonster[iCount1];
	if( pBaseMonster == NULL )
	{
		AddMemoErrMsg( "***** Load Basest Monster Error, Cannot New CSrvTmpBaseMonster ! *****" );
		fclose( pFile );
		return 0;
	}
	//
	for( iNumber = 1; iNumber <= iCount1; iNumber++ )
	{
		pTemp = &(pBaseMonster[iNumber-1]);
    ZeroMemory( dwMonsterData, sizeof( dwMonsterData ) );
		//memset( pTemp, 0, sizeof( CSrvTmpBaseMonster ) );
    if( 30 == fscanf( pFile, "%d %s %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %s",
                             &dwMonsterData[0],  szMonsterName,      &dwMonsterData[1],  &dwMonsterData[2],  &dwMonsterData[3],
                             &dwMonsterData[4],  &dwMonsterData[5],  &dwMonsterData[6],  &dwMonsterData[7],  &dwMonsterData[8],
                             &dwMonsterData[9],  &dwMonsterData[10], &dwMonsterData[11], &dwMonsterData[12], &dwMonsterData[13],
                             &dwMonsterData[14], &dwMonsterData[15], &dwMonsterData[16], &dwMonsterData[17], &dwMonsterData[18],
                             &dwMonsterData[19], &dwMonsterData[20], &dwMonsterData[21], &dwMonsterData[22], &dwMonsterData[23],
                             &dwMonsterData[24], &dwMonsterData[25], &dwMonsterData[26], &dwMonsterData[27], szInitLog ) )
		{
      memcpy( pTemp->m_szName, szMonsterName, MAX_NPC_NAME_LEN );
      pTemp->m_szName[MAX_NPC_NAME_LEN-1] = '\0';
      //
			pTemp->m_wId            = dwMonsterData[0];
      pTemp->m_wPicId         = dwMonsterData[1];
      pTemp->m_wElement       = dwMonsterData[2];
      pTemp->m_dwRaceAttri    = dwMonsterData[3];
      pTemp->m_iHp            = dwMonsterData[4];
      pTemp->m_iAp            = dwMonsterData[5];
			pTemp->m_iDp            = dwMonsterData[6];
      pTemp->m_iHit           = dwMonsterData[7];
      pTemp->m_iDodge         = dwMonsterData[8];
      pTemp->m_iInt           = dwMonsterData[9];
      pTemp->m_wHard          = dwMonsterData[10];
      pTemp->m_iHpUp          = dwMonsterData[11];
			pTemp->m_iApUp          = dwMonsterData[12];
      pTemp->m_iDpUp          = dwMonsterData[13];
      pTemp->m_iHitUp         = dwMonsterData[14];
      pTemp->m_iDodgeUp       = dwMonsterData[15];
      pTemp->m_iIntUp         = dwMonsterData[16];
			pTemp->m_iApBonuRand    = dwMonsterData[17];
      pTemp->m_wSize          = dwMonsterData[18];
      pTemp->m_iNpcProperty   = dwMonsterData[19];
      pTemp->m_iAcceptTable   = dwMonsterData[20];
      pTemp->m_wSpeed         = dwMonsterData[21];
			pTemp->m_wSightRange    = dwMonsterData[22];
      pTemp->m_wHelpRange     = dwMonsterData[23];
      pTemp->m_wPursueRange   = dwMonsterData[24];
      pTemp->m_wPursueSpeed   = dwMonsterData[25];
      pTemp->m_wEscapeSpeed   = dwMonsterData[26];
			pTemp->m_wHaltRange     = dwMonsterData[27];
			// Check Id
			if( pTemp->m_wId != iNumber )
			{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Basest Monster Error, The Id=%d Isn't Consecutive ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg( szInitLog );
#endif
				fclose( pFile );
				return 0;
			}
      //pTemp->m_wSpeed				/= 5;
			//pTemp->m_wPursueRange /= 5;
			//pTemp->m_wPursueSpeed	/= 5;
			//pTemp->m_wEscapeSpeed /= 5;
			//if( pTemp->m_wId == 1 )			pTemp->m_wExpLtLv = 5;
			//else												pTemp->m_wExpLtLv = 40;
		}
		else
		{
			delete[] pBaseMonster;
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Basest Monster Error (Id=%d), Scan File Data Error ! *****", iNumber );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			fclose( pFile );
			return 0;
		}
	}
	fclose( pFile );
	// Get Base Monster Data After Get Basest Monster Data
	// Get File Full Path And Open File
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath2 );
  pFile = fopen( szInitLog, "r" );
  if( !pFile )
  {
    return false;
  }
	// Get Base Monster Count
	if( 1 != fscanf( pFile, "%d ", &iCount2 ) )
	{
		AddMemoErrMsg( "***** Load Base Monster Error, Scan Count Error ! *****" );
		fclose( pFile );
		return 0;
	}
	// Check Error
	if( iCount2 < 1 )
	{
		AddMemoErrMsg( "***** Load Base Monster Error, The Count < 1 ! *****" );
		fclose( pFile );
		return 0;
	}
	//
	for( iNumber = 1; iNumber <= iCount2; iNumber++ )
	{
		if( 2 == fscanf( pFile, "%d %d ", &dwId, &dwBaseId ) )
		{
			if( dwBaseId <= iCount1 )
			{
				pMonster = new CSrvBaseMonster( &(pBaseMonster[dwBaseId-1]), dwId );
				//memset( pMonster, 0, sizeof( CSrvBaseMonster ) );
			}
			else
			{
				AddMemoErrMsg( "***** Load Base Monster Error, Monster Base Id > Basest Monster Id List ! *****" );
				fclose( pFile );
				return 0;
			}
		}
		else
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Base Monster Error (Id=%d), Scan File Data Error(A) ! *****", iNumber );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			fclose( pFile );
			return 0;
		}
		// Continue Scsn File
    ZeroMemory( dwMonsterData, sizeof( dwMonsterData ) );
		if( 57 == fscanf( pFile, "%d %d %d %d %d"
                             "%d %d %d %d %d %d"
                             "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d"
														 "%d %d %d %d %d %s",
                             &dwMonsterData[0],  &dwMonsterData[1],  &dwMonsterData[2],  &dwMonsterData[3],  &dwMonsterData[4],
                             &dwMonsterData[5],  &dwMonsterData[6],  &dwMonsterData[7],  &dwMonsterData[8],  &dwTemp[1], &dwTemp[0],
                             &dwMonsterData[10], &dwMonsterData[11], &dwMonsterData[12], &dwMonsterData[13], &dwMonsterData[14],
                             &dwMonsterData[15], &dwMonsterData[16], &dwMonsterData[17], &dwMonsterData[18], &dwMonsterData[19],
                             &dwMonsterData[20], &dwMonsterData[21], &dwMonsterData[22], &dwMonsterData[23], &dwMonsterData[24],
                             &dwMonsterData[25], &dwMonsterData[26], &dwMonsterData[27], &dwMonsterData[28], &dwMonsterData[29],
                             &dwMonsterData[30], &dwMonsterData[31], &dwMonsterData[32], &dwMonsterData[33], &dwMonsterData[34],
                             &dwMonsterData[35], &dwMonsterData[36], &dwMonsterData[37], &dwMonsterData[38], &dwMonsterData[39],
                             &dwMonsterData[40], &dwMonsterData[41], &dwMonsterData[42], &dwMonsterData[43], &dwMonsterData[44],
                             &dwMonsterData[45], &dwMonsterData[46], &dwMonsterData[47], &dwMonsterData[48], &dwMonsterData[49],
                             &dwMonsterData[50], &dwMonsterData[51], &dwMonsterData[52], &dwMonsterData[53], &dwMonsterData[54],
                             szInitLog ) )
		{
			pMonster->m_wLevel          = dwMonsterData[0];
      pMonster->m_dwExp           = dwMonsterData[1];
      pMonster->m_wExpLtLv        = dwMonsterData[2];
      pMonster->m_iEquipAp        = dwMonsterData[3];
      pMonster->m_iEquipHit       = dwMonsterData[4];
      pMonster->m_iEquipDp        = dwMonsterData[5];
      pMonster->m_iEquipDodge     = dwMonsterData[6];
      pMonster->m_iEquipInt       = dwMonsterData[7];
      pMonster->m_iBearPsb        = dwMonsterData[8];
      pMonster->m_qwAntiStatus    = MAKEQWORD(dwTemp[0],dwTemp[1]);
      pMonster->m_dwMoney         = dwMonsterData[10];
      pMonster->m_wRace           = dwMonsterData[11];
      pMonster->m_wSoul           = dwMonsterData[12];
      pMonster->m_wCommand        = dwMonsterData[13];
			pMonster->m_dwReviveTime    = dwMonsterData[14];
      pMonster->m_dwBoneTime      = dwMonsterData[15];
      pMonster->m_dwDelTime       = dwMonsterData[16];
      pMonster->m_dwLifeTime      = dwMonsterData[17];
      pMonster->m_wLevelLimit     = dwMonsterData[18];
      pMonster->m_wFuncLbc        = dwMonsterData[19];
			pMonster->m_wFuncEvent      = dwMonsterData[20];
      pMonster->m_wFuncInit       = dwMonsterData[21];
      pMonster->m_wFuncLvUp       = dwMonsterData[22];
      pMonster->m_wFuncDef        = dwMonsterData[23];
      pMonster->m_wFuncDied       = dwMonsterData[24];
			pMonster->m_wSkillRate[0]   = dwMonsterData[25];
      pMonster->m_wSkill[0]       = dwMonsterData[26];
      pMonster->m_wSkillRate[1]   = dwMonsterData[27];
      pMonster->m_wSkill[1]       = dwMonsterData[28];
      pMonster->m_wSkillRate[2]   = dwMonsterData[29];
      pMonster->m_wSkill[2]       = dwMonsterData[30];
			pMonster->m_wSkillRate[3]   = dwMonsterData[31];
      pMonster->m_wSkill[3]       = dwMonsterData[32];
      pMonster->m_wSkillRate[4]   = dwMonsterData[33];
      pMonster->m_wSkill[4]       = dwMonsterData[34];
			pMonster->m_wDropRate[0]    = dwMonsterData[35];
      pMonster->m_wDropBegin[0]   = dwMonsterData[36];
      pMonster->m_wDropEnd[0]     = dwMonsterData[37];
			pMonster->m_wDropRate[1]    = dwMonsterData[38];
      pMonster->m_wDropBegin[1]   = dwMonsterData[39];
      pMonster->m_wDropEnd[1]     = dwMonsterData[40];
			pMonster->m_wDropRate[2]    = dwMonsterData[41];
      pMonster->m_wDropBegin[2]   = dwMonsterData[42];
      pMonster->m_wDropEnd[2]     = dwMonsterData[43];
			pMonster->m_wDropRate[3]    = dwMonsterData[44];
      pMonster->m_wDropBegin[3]   = dwMonsterData[45];
      pMonster->m_wDropEnd[3]     = dwMonsterData[46];
			pMonster->m_wDropRate[4]    = dwMonsterData[47];
      pMonster->m_wDropBegin[4]   = dwMonsterData[48];
      pMonster->m_wDropEnd[4]     = dwMonsterData[49];
      pMonster->m_wSoulProb       = dwMonsterData[50];
      pMonster->m_dwDeadEvent     = ( ( dwMonsterData[51] & 0x0000FFFF ) << 16 ) |
                                    ( dwMonsterData[52] & 0x0000FFFF );
      pMonster->m_wParam1         = dwMonsterData[53];
      pMonster->m_iParam2         = dwMonsterData[54];
      //
      if( iNumber != pMonster->m_wBaseId )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Monster Error, The Id=%d Isn't Consecutive ! *****", iNumber );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg( szInitLog );
#endif
				fclose( pFile );
				return 0;
      }
			// Init Base Data, Include Skill, Drop Item List And All Functions
			// ...
			if( pMonster->m_iNpcProperty & NPC_ATTRI_ESCAPE10 )
			{
				pMonster->m_wEscapeHp = pMonster->m_iHp / 10;
			}
			else
			{
				pMonster->m_wEscapeHp = 0;
			}
//#ifdef _DEBUG
//			pMonster->m_wSpeed        = 1;
//#endif
      if( !( pMonster->m_iNpcProperty & NPC_ATTRI_UNRESET ) )
      {
        pMonster->m_wDropRate[0] = pMonster->m_wDropRate[0];  
      }

      //pMonster->m_wDropRate[0] *= 3.2767f;
      //pMonster->m_wDropRate[1] *= 3.2767f;
      //pMonster->m_wDropRate[2] *= 3.2767f;
      //pMonster->m_wDropRate[3] *= 3.2767f;
      //pMonster->m_wDropRate[4] *= 3.2767f;

      pMonster->m_wSkillRate[0] *= 100;
      pMonster->m_wSkillRate[1] *= 100;
      pMonster->m_wSkillRate[2] *= 100;
      pMonster->m_wSkillRate[3] *= 100;
      pMonster->m_wSkillRate[4] *= 100;
      //
      pMonster->m_wElement      = pBaseMonster[dwBaseId-1].m_wElement;
      pMonster->m_dwRaceAttri   = pBaseMonster[dwBaseId-1].m_dwRaceAttri;
      //
			pMonster->m_dwReviveTime *= 1000;
			pMonster->m_dwBoneTime   *= 1000;
			pMonster->m_dwDelTime		 *= 1000;
			pMonster->m_dwLifeTime	 *= 1000;

			pMonster->m_iHp			=  pBaseMonster[dwBaseId-1].m_iHp +
                             ( pBaseMonster[dwBaseId-1].m_iHpUp * pMonster->m_wLevel / 100 ) +
                             ( ( pBaseMonster[dwBaseId-1].m_iHpUp & 0x63 ) * pMonster->m_wLevel / 100 );

			pMonster->m_iAp			=  pBaseMonster[dwBaseId-1].m_iAp +
                             ( pBaseMonster[dwBaseId-1].m_iApUp * pMonster->m_wLevel / 100 ) +
                             ( ( pBaseMonster[dwBaseId-1].m_iApUp & 0x63 ) * pMonster->m_wLevel / 100 );

			pMonster->m_iDp			=  pBaseMonster[dwBaseId-1].m_iDp +
                             ( pBaseMonster[dwBaseId-1].m_iDpUp * pMonster->m_wLevel / 100 ) +
                             ( ( pBaseMonster[dwBaseId-1].m_iDpUp & 0x63 ) * pMonster->m_wLevel / 100 );

			pMonster->m_iHit		=  pBaseMonster[dwBaseId-1].m_iHit +
                             ( pBaseMonster[dwBaseId-1].m_iHitUp * pMonster->m_wLevel / 100 ) +
                             ( ( pBaseMonster[dwBaseId-1].m_iHitUp & 0x63 ) * pMonster->m_wLevel / 100 );

			pMonster->m_iDodge  =  pBaseMonster[dwBaseId-1].m_iDodge +
                             ( pBaseMonster[dwBaseId-1].m_iDodgeUp * pMonster->m_wLevel / 100 ) +
                             ( ( pBaseMonster[dwBaseId-1].m_iDodgeUp & 0x63 ) * pMonster->m_wLevel / 100 );

			pMonster->m_iInt		=  pBaseMonster[dwBaseId-1].m_iInt +
                             ( pBaseMonster[dwBaseId-1].m_iIntUp * pMonster->m_wLevel / 100 ) +
                             ( ( pBaseMonster[dwBaseId-1].m_iIntUp & 0x63 ) * pMonster->m_wLevel / 100 );

      pMonster->m_iApBonuRand = ( pBaseMonster[dwBaseId-1].m_iApBonuRand * pMonster->m_wLevel / 100 ) +
                                ( ( pBaseMonster[dwBaseId-1].m_iApBonuRand & 0x63 ) * pMonster->m_wLevel / 100 );
			// Set Exp Limit Level
			//pMonster->m_wExpLtLv     = pBaseMonster[dwBaseId-1].m_wExpLtLv;
			pMonster->m_wExpLtLvHalf = pMonster->m_wExpLtLv >> 1;

			// check 
			for( int i=0; i < MAX_MONSTER_SKILL; i++ )
			{
				if( pMonster->m_wSkill[i] && m_mapBaseSkill.find( pMonster->m_wSkill[i] ) == m_mapBaseSkill.end() )
				{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
					_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Base Monster Error, Skill(Id=%d) is not Exist! *****", pMonster->m_wSkill[i] );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
					AddMemoErrMsg( szInitLog );
#endif
					return 0;
				}
			}

			// Insert The Monster
			m_mapBaseMonster.insert( map<DWORD,CSrvBaseMonster*>::value_type( pMonster->m_wId, pMonster ) );

		}
		else
		{
			SAFE_DELETE( pMonster );
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "***** Load Base Monster Error (Id=%d), Scan File Data Error(B) ! *****", iNumber );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			return 0;
		}
	}
	delete[] pBaseMonster;
	fclose( pFile );
	return 1;
#endif // _DEBUG_JAPAN_DECRYPT_
}


//===============================================================================================
//
//
int CSrvBaseData::LoadBaseSuitEquip(char * szFilePath)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CSrvBaseData::LoadBaseSuitEquip");
  CSuitEquipData      *pSuitEquip = NULL;
  CSrvBaseSkill       *pBaseSkill = NULL;
  DWORD               dwSuitData[12], dwCount = 0;
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  CInStream           SuitEquip( szInitLog );
//  CInStream           SuitEquip(".\\GameData\\Item\\SuitEquip.txt");
  if ( !SuitEquip.fail() && SuitEquip.GetFileSize() != 0 ) 
  {
    SuitEquip >> dwCount;
    if ( dwCount<=1 ) 
    {
      MessageBox( GetActiveWindow(), "Load SuitEquip dwCount Error", "Warning...", MB_OK);
      return FALSE;
    }
    for( int iLoop = 0; iLoop<dwCount; iLoop++ )
    {
      SuitEquip >> dwSuitData[0] >> dwSuitData[1];
      SuitEquip >> dwSuitData[2] >> dwSuitData[3];
      SuitEquip >> dwSuitData[4] >> dwSuitData[5];
      SuitEquip >> dwSuitData[6] >> dwSuitData[7];
      SuitEquip >> dwSuitData[8] >> dwSuitData[9];
      SuitEquip >> dwSuitData[10] >> dwSuitData[11];
      {
        // Check Data
        pSuitEquip = new CSuitEquipData;
        pSuitEquip->m_wId    = dwSuitData[0];
        pSuitEquip->m_wCount = dwSuitData[1];
        for( int iS = 0; iS < MAX_SUIT_EQUIP_COUNT; iS++ )
        {
          if( dwSuitData[iS+2] )
          {
            if( pBaseSkill = GetBaseSkill( dwSuitData[iS+2] ) )
            {
              pSuitEquip->m_pBaseSkill[iS] = pBaseSkill;
            }
            else
            {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
              _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "==>> Cannot Find The BaseSkill When Scan Suit Equip,"
                " SuitId=%d, SkillId=%d", pSuitEquip->m_wId, dwSuitData[iS+2] );
              szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg( szInitLog );
#endif
              SAFE_DELETE( pSuitEquip );
              return FALSE;
            }
          }
        }
        m_mapSuitSkill.insert( map<WORD,CSuitEquipData*>::value_type( pSuitEquip->GetId(), pSuitEquip ) );        
      }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Cannot Open the SuitEquip.txt", "Warning...", MB_OK);
    return FALSE;
  }
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Read Suit Equip File='%s' OK, Suit Equip Count=%d", szFilePath, dwCount );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
  return TRUE;
#else
//
  FuncName("CSrvBaseData::LoadBaseSuitEquip");
  FILE                *pFile = NULL;
  CSuitEquipData      *pSuitEquip = NULL;
  CSrvBaseSkill       *pBaseSkill = NULL;
  DWORD               dwSuitData[12], dwCount = 0;
  //
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  pFile = fopen( szInitLog, "r" );
  //
  if( pFile == NULL )
  {
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "==>> Cannot Open Suit Equip File=%s", szFilePath );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szInitLog );
    return 1;
  }
  fscanf( pFile, "%d", &dwCount );
  if( dwCount )
  {
    for( int iLoop = 0; iLoop < dwCount; iLoop++ )
    {
      memset( dwSuitData, 0, sizeof( DWORD ) * 12 );
      if( 12 == fscanf( pFile, "%d %d %d %d %d"
                               "%d %d %d %d %d"
                               "%d %d",
                               &dwSuitData[0], &dwSuitData[1], &dwSuitData[2], &dwSuitData[3], &dwSuitData[4],
                               &dwSuitData[5], &dwSuitData[6], &dwSuitData[7], &dwSuitData[8], &dwSuitData[9],
                               &dwSuitData[10], &dwSuitData[11] ) )
      {
        // Check Data
        pSuitEquip = new CSuitEquipData;
        pSuitEquip->m_wId    = dwSuitData[0];
        pSuitEquip->m_wCount = dwSuitData[1];
        for( int iS = 0; iS < MAX_SUIT_EQUIP_COUNT; iS++ )
        {
          if( dwSuitData[iS+2] )
          {
            if( pBaseSkill = GetBaseSkill( dwSuitData[iS+2] ) )
            {
              pSuitEquip->m_pBaseSkill[iS] = pBaseSkill;
            }
            else
            {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
              _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "==>> Cannot Find The BaseSkill When Scan Suit Equip,"
                                  " SuitId=%d, SkillId=%d", pSuitEquip->m_wId, dwSuitData[iS+2] );
              szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg( szInitLog );
#endif
              SAFE_DELETE( pSuitEquip );
              return FALSE;
            }
          }
        }
        m_mapSuitSkill.insert( map<WORD,CSuitEquipData*>::value_type( pSuitEquip->GetId(), pSuitEquip ) );
      }
      else
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "==>> Scan Suit Equip File Error, Id=%d", iLoop+1 );
        szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg( szInitLog );
#endif
        return FALSE;
      }
    }
  }
  _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "Read Suit Equip File='%s' OK, Suit Equip Count=%d", szFilePath, dwCount );
  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szInitLog );
	fclose( pFile );
  return TRUE;
#endif// _DEBUG_JAPAN_DECRYPT_
}
//===============================================================================================
//
//
void CSrvBaseData::ReleaseNouseMapInThisServer()
{
  CSrvBaseMap			                      *pBaseMap = NULL;
	map<DWORD,CSrvBaseMap*>::iterator     Iter_Bm;
  //
	for( Iter_Bm = m_mapBaseMap.begin(); Iter_Bm != m_mapBaseMap.end(); Iter_Bm++ )
  {
    pBaseMap = (CSrvBaseMap*)(Iter_Bm->second);
    //if( !pBaseMap->IsInThisServer() )
    {
		  pBaseMap->ReleaseMyCellInfo();
    }
  }
}
//===============================================================================================
//
//
void CSrvBaseData::ReleaseBaseMap()
{
  CSrvBaseMap			*pMap = NULL;
	
	for(map<DWORD, CSrvBaseMap*>::iterator iter = m_mapBaseMap.begin(); iter != m_mapBaseMap.end();)
  {
    pMap = (CSrvBaseMap*)iter->second;
		iter = m_mapBaseMap.erase(iter);
		SAFE_DELETE(pMap)
  }
  m_mapBaseMap.clear();
}
//===============================================================================================
//
//
void CSrvBaseData::ReleaseBaseSkill()
{
  CSrvBaseSkill			*pSkill = NULL;
	
	for(map<DWORD, CSrvBaseSkill*>::iterator iter = m_mapBaseSkill.begin(); iter != m_mapBaseSkill.end();)
  {
		pSkill = (CSrvBaseSkill*)iter->second;
		iter = m_mapBaseSkill.erase(iter);
    SAFE_DELETE(pSkill)
  }
  m_mapBaseSkill.clear();
}

#ifdef EVILWEAPON_3_6_VERSION
///////////////////////////////////////////////////////////
//Add by CECE 2004-04-05
void  CSrvBaseData::ReleaseBaseEvilWeapon()
{
  map<int, SBaseEvilWeapon*>::iterator iter;
  SBaseEvilWeapon			*pItem = NULL;
	
	for(iter = m_mapBaseEvilWeapon.begin(); iter != m_mapBaseEvilWeapon.end(); iter++ )
  {
    pItem = (SBaseEvilWeapon*)iter->second;
    SAFE_DELETE( pItem );
  }
  m_mapBaseEvilWeapon.clear();
}
#endif
///////////////////////////////////////////////////////////
//===============================================================================================
//
//
void CSrvBaseData::ReleaseBaseItem()
{
  CSrvBaseItem			*pItem = NULL;
	
	for(map<DWORD, CSrvBaseItem*>::iterator iter = m_mapBaseItem.begin(); iter != m_mapBaseItem.end();)
  {
    pItem = (CSrvBaseItem*)iter->second;
		iter  = m_mapBaseItem.erase(iter);
    SAFE_DELETE( pItem );
  }
  m_mapBaseItem.clear();
}
//===============================================================================================
//
//
void CSrvBaseData::ReleaseBaseEvent()
{
  // ...
	map<DWORD, CTask*>::iterator				tIte;
	map<DWORD, CEventPoint*>::iterator	pIte;
	CTask																*pTask = NULL;
	CEventPoint													*pPoint = NULL;
  m_mapEventPoint.clear();

	// Clear Task
	if( !m_mapTask.empty() )
	{
		for( tIte = m_mapTask.begin(); tIte != m_mapTask.end(); )
		{
			pTask = (CTask*)tIte->second;
			tIte  = m_mapTask.erase(tIte);
			SAFE_DELETE( pTask )
		}
		m_mapTask.clear();
	}
}
//===============================================================================================
//
//
void CSrvBaseData::ReleaseBaseMonster()
{
  CSrvBaseMonster					                *pMonster = NULL;
  map<DWORD, CSrvBaseMonster*>::iterator  Iter_Mt;
  //
	for( Iter_Mt = m_mapBaseMonster.begin(); Iter_Mt != m_mapBaseMonster.end(); )
  {
		pMonster = (CSrvBaseMonster*)(Iter_Mt->second);
		Iter_Mt  = m_mapBaseMonster.erase( Iter_Mt );
    //
		SAFE_DELETE( pMonster );
  }
  m_mapBaseMonster.clear();
}
//===============================================================================================
//
//
void CSrvBaseData::ReleaseSuitEquip()
{
  map<WORD,CSuitEquipData*>::iterator     Iter_Se;
  //
  for( Iter_Se = m_mapSuitSkill.begin(); Iter_Se != m_mapSuitSkill.end(); Iter_Se++ )
  {
    SAFE_DELETE( Iter_Se->second );
  }
  m_mapSuitSkill.clear();
}
//==========================================================================================
//
//Functions:
//
CSrvBaseMap* CSrvBaseData::GetBaseMap(DWORD dwId)
{
  static map<DWORD, CSrvBaseMap*>::iterator Iter_Map;

  Iter_Map = m_mapBaseMap.find( dwId );
  if( m_mapBaseMap.end() == Iter_Map )
  {
    return NULL;
  }
  return Iter_Map->second;
}
//===============================================================================================
//
//
CSrvBaseSkill* CSrvBaseData::GetBaseSkill(DWORD dwId)
{
  static map<DWORD, CSrvBaseSkill*>::iterator Iter_Skill;

  Iter_Skill = m_mapBaseSkill.find( dwId );
  if(m_mapBaseSkill.end() == Iter_Skill)
  {
    return NULL;
  }
  return Iter_Skill->second;
}
//===============================================================================================
//
//
CSrvBaseItem* CSrvBaseData::GetBaseItem(DWORD dwId)
{
  static map<DWORD, CSrvBaseItem*>::iterator Iter_Item;

  Iter_Item = m_mapBaseItem.find( dwId );
  if( m_mapBaseItem.end() == Iter_Item )
  {
    return NULL;
  }
  return (CSrvBaseItem*)(Iter_Item->second);
}
//===============================================================================================
//
//
CSrvBaseItem* CSrvBaseData::GetBaseItemFromName(const char *pName)
{
  static map<DWORD, CSrvBaseItem*>::iterator  Iter_Item1;
  CSrvBaseItem                                *pBaseItemByName = NULL;

  for( Iter_Item1 = m_mapBaseItem.begin(); Iter_Item1 != m_mapBaseItem.end(); Iter_Item1++ )
  {
    if( !strcmp( Iter_Item1->second->GetName(), pName ) )
    {
      pBaseItemByName = Iter_Item1->second;
      break;
    }
  }
  return pBaseItemByName;
}
//===============================================================================================
//
//
void CSrvBaseData::ChangeReviveTime( WORD wTime, BOOL bSub )
{
  map<DWORD, CSrvBaseMonster*>::iterator    Iter_M;
  CSrvBaseMonster                           *pBaseMonster = NULL;

  for( Iter_M = m_mapBaseMonster.begin(); Iter_M != m_mapBaseMonster.end(); Iter_M++ )
  {
    pBaseMonster = (Iter_M->second);
    pBaseMonster->ChangeReviveTime( wTime, bSub );
  }
}
//===============================================================================================
//
//
CSrvBaseMonster* CSrvBaseData::GetBaseMonster(DWORD dwId)
{
  map<DWORD, CSrvBaseMonster*>::iterator iter = m_mapBaseMonster.find(dwId);
  if(m_mapBaseMonster.end() == iter)
  {
    return NULL;
  }
  return (CSrvBaseMonster*)iter->second;
}
//===============================================================================================
//
//
CEventPoint* CSrvBaseData::GetEventPoint(DWORD dwEventId_PointIndex)  // DWORD: (Hi Word) Event ID, (Lo Word) the Index of EventPoint in the Event
{
  map<DWORD, CEventPoint*>::iterator iter = m_mapEventPoint.find(dwEventId_PointIndex);
  if(m_mapEventPoint.end() == iter)
  {
    return NULL;
  }
  return iter->second;
}
#ifdef _CHECK_SERVER_DATA_FILE_VERSION_
//===============================================================================================
//
//
CSrvBaseMonster **CSrvBaseData::GetMonsterByDropItem( const WORD & wItemId, WORD & wCount )
{
  map<DWORD, CSrvBaseMonster*>::iterator  Iter_Mt;
  CSrvBaseItem                            *pBaseItem;
  CSrvBaseMonster                         *pBaseMonster;
  CSrvDropItem							              *pItemFix;
  int                                     iListNo = 0, iCurrArray = 0, iNum = 0;
  static CSrvBaseMonster                  *pCheckBaseMonster[300];
  
  for( Iter_Mt = m_mapBaseMonster.begin(); Iter_Mt != m_mapBaseMonster.end(); Iter_Mt++ )
  {
    pBaseMonster = Iter_Mt->second;
    for( iListNo = 0; iListNo < 5; iListNo++ )
	  {
		  if( pBaseMonster->m_wDropRate[iListNo] == 0 )  continue;
		  // From Drop Begin Id To Drop End Id
		  for( iCurrArray = pBaseMonster->m_wDropBegin[iListNo]; iCurrArray <= pBaseMonster->m_wDropEnd[iListNo]; iCurrArray++ )
      {
        if( NULL != ( pItemFix = g_pDropItem->GetDropItem( iCurrArray ) ) )
        {
          if( pBaseItem = g_pBase->GetBaseItem( pItemFix->m_wItemId ) )
          {
            if( pBaseItem->GetId() == wItemId )
            {
              pCheckBaseMonster[iNum++]    = pBaseMonster;
              pBaseMonster->m_iAcceptTable = pBaseMonster->m_wDropRate[iListNo];
            }
          }
          else
          {
            _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "%s[%d]的第%d组掉落物品列表中有不存在的修正物品，DropFixId=%d", pBaseMonster->GetName(), pBaseMonster->GetId(), iListNo, pItemFix->m_wItemId );
            szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
            ::MessageBox( NULL, szInitLog, "Error", 0 );
            return NULL;
          }
        }
        else
        {
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "%s[%d]的第%d组掉落物品列表中有不存在的物品，Id=%d", pBaseMonster->GetName(), pBaseMonster->GetId(), iListNo, iCurrArray );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          ::MessageBox( NULL, szInitLog, "Error", 0 );
          return NULL;
        }
      }
    }
  }
  if( iNum )
  {
    wCount = iNum;
    return pCheckBaseMonster;
  }
  wCount = 0;
  return NULL;
}
//===============================================================================================
//
//
CSrvBaseItem ** CSrvBaseData::GetDropItemByMonsterId( const WORD & wMonsterId, WORD & wCount )
{
  map<DWORD, CSrvBaseMonster*>::iterator  Iter_Mt;
  CSrvBaseItem                            *pBaseItem;
  CSrvBaseMonster                         *pBaseMonster;
  CSrvDropItem							              *pItemFix;
  int                                     iListNo = 0, iCurrArray = 0, iNum = 0;
  static CSrvBaseItem                     *pCheckBaseItem[300];

  Iter_Mt = m_mapBaseMonster.find( wMonsterId );
  if( Iter_Mt != m_mapBaseMonster.end() )
  {
    pBaseMonster = Iter_Mt->second;
    for( iListNo = 0; iListNo < 5; iListNo++ )
	  {
		  if( pBaseMonster->m_wDropRate[iListNo] == 0 )  continue;
		  // From Drop Begin Id To Drop End Id
		  for( iCurrArray = pBaseMonster->m_wDropBegin[iListNo]; iCurrArray <= pBaseMonster->m_wDropEnd[iListNo]; iCurrArray++ )
		  {
        if( NULL != ( pItemFix = g_pDropItem->GetDropItem( iCurrArray ) ) )
        {
          if( pBaseItem = g_pBase->GetBaseItem( pItemFix->m_wItemId ) )
          {
            pCheckBaseItem[iNum++] = pBaseItem;
            pBaseItem->m_wIconId   = pBaseMonster->m_wDropRate[iListNo];
          }
          else
          {
            _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "%s[%d]的第%d组掉落物品列表中有不存在的修正物品，DropFixId=%d", pBaseMonster->GetName(), pBaseMonster->GetId(), iListNo, pItemFix->m_wItemId );
            szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
            ::MessageBox( NULL, szInitLog, "Error", 0 );
            return NULL;
          }
        }
        else
        {
          _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "%s[%d]的第%d组掉落物品列表中有不存在的物品，Id=%d", pBaseMonster->GetName(), pBaseMonster->GetId(), iListNo, iCurrArray );
          szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
          ::MessageBox( NULL, szInitLog, "Error", 0 );
          return NULL;
        }
      }
    }
    wCount = iNum;
    return pCheckBaseItem;
  }
  else
  {
    _snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, "此怪物不存在，Id=%d",  wMonsterId );
    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
    ::MessageBox( NULL, szInitLog, "Error", 0 );
    return NULL;
  }
}
#endif
//===============================================================================================
//
// member function of class CGsShowMessage
CGsShowMessage::CGsShowMessage()
{
  listMemoStrings.clear();
  listErrMemoStrings.clear();
  m_ListErrorMemoDlg.clear();
	
  InitializeCriticalSection(&cs_Message);
  InitializeCriticalSection(&cs_ErrMessage);
  InitializeCriticalSection(&cs_ErrMemoDlg);
  m_iPopErrorMsgDlg = 0;
}
//===============================================================================================
//
//
CGsShowMessage::~CGsShowMessage()
{
  DeleteCriticalSection(&cs_ErrMessage);
  DeleteCriticalSection(&cs_Message);
  DeleteCriticalSection(&cs_ErrMemoDlg);

  listErrMemoStrings.clear();
  listMemoStrings.clear();
}
//===============================================================================================
//
//
inline void CGsShowMessage::Show(char *szMsg)
{
  SYSTEMTIME  SysTime;
  char        szShowErrLog[MAX_MEMO_MSG_LEN+256];
	
  GetLocalTime( &SysTime );
  if( strlen( szMsg ) > (MAX_MEMO_MSG_LEN>>1) )
  { // cut the string which is too long
    szMsg[(MAX_MEMO_MSG_LEN>>1)-1] = '\0';
  }

  _snprintf( szShowErrLog, MAX_MEMO_MSG_LEN+256-1, "%04d %02d/%02d %02d:%02d:%02d %s\n", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond, szMsg);
  szShowErrLog[MAX_MEMO_MSG_LEN+256-1] = '\0';
	
  EnterCriticalSection( &cs_Message );
	
  // Show Message in the Message Memo
  if( listMemoStrings.size() >= 24 )
  {
    listMemoStrings.pop_front();
  }
  listMemoStrings.push_back( szShowErrLog );
	//
  m_iIsUpdated = 1;
  LeaveCriticalSection( &cs_Message );
}
//===============================================================================================
//
//
inline void CGsShowMessage::ShowError(char *szMsg)
{
  SYSTEMTIME  SysTime;
  char        szShowErrLog[MAX_MEMO_MSG_LEN+256];
	
  GetLocalTime( &SysTime );
  if( strlen( szMsg ) > (MAX_MEMO_MSG_LEN>>1) )
  { // cut the string which is too long
    szMsg[(MAX_MEMO_MSG_LEN>>1)-1] = '\0';
  }
  _snprintf( szShowErrLog, MAX_MEMO_MSG_LEN+256-1, "%04d %02d/%02d %02d:%02d:%02d %s\n", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond, szMsg);
  szShowErrLog[MAX_MEMO_MSG_LEN+256-1] = '\0';
	
  EnterCriticalSection( &cs_Message );

  // Show Message in the Message Memo
  if( listMemoStrings.size() >= 24 )
  {
    listMemoStrings.pop_front();
  }
  listMemoStrings.push_back( szShowErrLog );
	
  LeaveCriticalSection( &cs_Message );
	
  EnterCriticalSection( &cs_ErrMessage );
  // Show Message in the Error Message Memo
  if( listErrMemoStrings.size() >= 10 )
  {
    listErrMemoStrings.pop_front();
  }
  listErrMemoStrings.push_back( szShowErrLog );
	
  m_iIsUpdated = 1;
  LeaveCriticalSection( &cs_ErrMessage );
}
//===============================================================================================
//
//
void CGsShowMessage::GetGsAllState(char *szGsState, char *szMccState1, char *szMccState2, char *szClientCount, char *szPlayerCount)
{
  // Get Gs State
  strcpy(szGsState, "Gs: ");
  switch(g_pGs->GetState())
  {
	case GSSTATE_STOP:
		strcat(szGsState, "Stop");
		break;
	case GSSTATE_RUN:
		strcat(szGsState, "Running");
		break;
	case GSSTATE_INIT_BASEDATA:
		strcat(szGsState, "Init All Data");
		break;
	case GSSTATE_INIT_MCC_THREAD:
		strcat(szGsState, "Init Mcc Thread");
		break;
	case GSSTATE_WAIT_DBMCC_CONNECT:
		strcat(szGsState, "Connect DB Mcc");
		break;
	case GSSTATE_WAIT_CHATMCC_CONNECT:
		strcat(szGsState, "Connect Chat Mcc");
		break;
	case GSSTATE_AP_MAPINIT:
		strcat(szGsState, "Init All Map");
		break;
	case GSSTATE_AP_CHATROOMLIST:
		strcat(szGsState, "Init All Chatroom");
		break;
	case GSSTATE_AP_GMLIST:
		strcat(szGsState, "Init GM List");
		break;
	case GSSTATE_AP_GETTIME:
		strcat(szGsState, "Set Game Time");
		break;
	case GSSTATE_AP_CLEARLOCK:
		strcat(szGsState, "Clear Online Table");
		break;
		
	case GSSTATE_INIT_MAP_OBJ:
		strcat(szGsState, "Init All Map Object");
		break;
		
	case GSSTATE_INIT_NET_THREAD:
		strcat(szGsState, "Init Client Net Thread");
		break;
		
	case GSSTATE_INIT_COMPLETE:
		strcat(szGsState, "Init Complete");
		break;
	case GSSTATE_INIT_FAIL:
		strcat(szGsState, "Init Failed");
		break;
		
		// Game Server start
	case GSSTATE_GAME_START:
		strcat(szGsState, "Game Start");
		break;
	case GSSTATE_CLOSE:
		strcat(szGsState, "Close");
		break;
	case GSSTATE_ERROR_CLOSE:
		strcat(szGsState, "Error Colse");
		break;
  case GSSTATE_SAVE_ALL_CLIENT:
		strcat(szGsState, "Save All Client");
		break;
  case GSSTATE_WAIT_SAVE_ALL_CLIENT:
		strcat(szGsState, "Wait Save All Client");
		break;

	default:
		_snprintf( szGsState, 256-1, "????????==%d", g_pGs->GetState() );
    szGsState[256-1] = '\0';
		break;
  }
	
  // Get Mcc State
  strcpy( szMccState1, "DBMcc: ");
  switch( g_pMccDB->GetMccState() )
  {
	case MCCINFO_STATE_NOCONNECT:
		strcat(szMccState1, " No Connect");
		break;
	case MCCINFO_STATE_RETRY:
		strcat(szMccState1, " Retry");
		break;
	case MCCINFO_STATE_STOP:
		strcat(szMccState1, " Stop");
		break;
	case MCCINFO_STATE_RUN:
		strcat(szMccState1, " Run");
		break;
	case MCCINFO_STATE_EXIT:
		strcat(szMccState1, " Exit");
		break;
	default:
		strcat(szMccState1, " ?????");
		break;
  }
	strcpy(szMccState2, "ChatMcc: ");
  switch( g_pMccChat->GetMccState() )
  {
	case MCCINFO_STATE_NOCONNECT:
		strcat(szMccState2, " No Connect");
		break;
	case MCCINFO_STATE_RETRY:
		strcat(szMccState2, " Retry");
		break;
	case MCCINFO_STATE_STOP:
		strcat(szMccState2, " Stop");
		break;
	case MCCINFO_STATE_RUN:
		strcat(szMccState2, " Run");
		break;
	case MCCINFO_STATE_EXIT:
		strcat(szMccState2, " Exit");
		break;
	default:
		strcat(szMccState2, " ?????");
		break;
  }
	
  // Get Client Count
  // Note: please check "szClientCount"'s Len when it transfered by this function
  _snprintf( szClientCount, 256-1, "Client= (%d,%d)", g_pClientList->GetClientUsedCount(),g_pClientList->GetMaxUseClient() );
  szClientCount[256-1] = '\0';
	
  // Get Player Count
  // Note: please check "szPlayerCount"'s Len when it transfered by this function
  _snprintf( szPlayerCount, 256-1, "Player= %d", g_pGs->GetPlayerNow() );
  szPlayerCount[256-1] = '\0';
}
//===============================================================================================
//
//
string ContentGet(string& Content,long& Pointer,int& target,int& target_Count)
{
	static int Content_flag = 0;

	string temp;
	if(Content_flag==0)
	{
		while(Content[Pointer]!=StartF)
		{
			Pointer++;
			if(Pointer>=(long)Content.length())
				return "FileOver";
		}
		Content_flag++;
		Pointer++;
		temp=Content[Pointer];
	}
	if(Content[Pointer]==EndTgtF)
	{
		Pointer++;
		if(Content[Pointer]==EndF)
		{
			Content_flag--;
			return "EndTask";
		}
		else temp=Content[Pointer];
	}
  if(temp[0]==TaskF)
  {
		target=TASK;
		target_Count=0;
		Pointer++;
  }
  else if(temp[0]==PointF)
  {
		target=EVENTPOINT;
		target_Count=0;
		Pointer++;
  }
  else if(temp[0]==SubF)
  {
		target=SUBEVENT;
		target_Count=0;
		Pointer++;
  }
  else if(temp[0]==EveRF)
  {
		target=EVENTRUN;
		target_Count=0;
		Pointer++;
  }
  else if(temp[0]==EveLF)
  {
		target=EVENTLIMIT;
		target_Count=0;
		Pointer++;
  }
  temp="";
  while(Content[Pointer]!=CEF)
  {
		temp+=Content[Pointer];
		Pointer++;
		if(Pointer>=(long)Content.length())
			return "FileOver";
  }
  return temp;
}
//===============================================================================================
//
//
string Load(char*& pstr,char* Name,long& iFileLength)
{
	HANDLE hFile;
	hFile = CreateFile(Name,     // open ONE.TXT 
    GENERIC_READ,                 // open for reading 
    0,                            // do not share 
    NULL,                         // no security 
    OPEN_EXISTING,                // existing file only 
    FILE_ATTRIBUTE_NORMAL,        // normal file 
    NULL);                        // no attr. template 
	
  if (hFile == INVALID_HANDLE_VALUE) 
  { 
    return "No File";  // process error 
  }
  DWORD  dFileLength,dReadBytes;
  dFileLength = GetFileSize(hFile, &dFileLength);
  iFileLength  =(long)dFileLength;
  if(dFileLength==0)
  {
    CloseHandle(hFile);
    return "";
  }
  pstr=new char[dFileLength+1];
  ReadFile( hFile, pstr, dFileLength, &dReadBytes, NULL );
  CloseHandle(hFile);
  return pstr;
}
//===============================================================================================
//
//
bool Save(string& Content,char* Name)
{
  long  iFileLength;
  DWORD dFileWrite;
  iFileLength=Content.length();
  char *szBuffer=NULL;
  szBuffer=new char[iFileLength+1];
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szBuffer,Content.c_str(), iFileLength+1 );
#else
  strcpy( szBuffer,Content.c_str() );
#endif
  HANDLE hFile;
  hFile = CreateFile(Name,   // open TWO.TXT 
    GENERIC_WRITE,                // open for writing 
    0,                            // do not share 
    NULL,                         // no security 
    OPEN_ALWAYS,                  // open or create 
    FILE_ATTRIBUTE_NORMAL,        // normal file 
    NULL);                        // no attr. template 
  if (hFile == INVALID_HANDLE_VALUE) 
  { 
    return false;    // process error 
  } 
  WriteFile( hFile, szBuffer, iFileLength+1, &dFileWrite, NULL);
  delete []szBuffer;
  CloseHandle( hFile );
  return true;
}
//===============================================================================================
//
//
int LayerCount(string& temp)
{
	int Layers=0,flag=0;
	if(temp[0]!=LAYERS)
	{
		return -1;
	}
	for(int count=0;count<(int)temp.length();count++)
	{
		if(temp[count]==LAYERSPACE||temp[count]==LAYERE)
			if(Layers<3)
				Layers++;
			else if(flag) {
				Layers++;
				flag=(flag>0)?0:1;
			}
			else if(!flag)
				flag=(flag>0)?0:1;
	}
	return Layers;
}
//===============================================================================================
//
//
bool Put(list<CEventRun*>& EventRunList,CEventRun* pEventRun,string& Path,int& cur_layer,int Layer)
{
	int pointer,floor,flag=0;
	string temp="";
	//Path.Delete(1,1);
	list<CEventRun*>::iterator pt_EventRun;
	for(int count=0;count<(int)Path.length();count++)
	{
		if(Path[count]!=LAYERSPACE&&Path[count]!=LAYERE)
      temp+=Path[count];
		else if(cur_layer<3) {
      cur_layer++;
      temp="";
		}
		else if(!flag) {
      pointer=atoi(temp.c_str());
      flag=(flag>0)?0:1;
      temp="";
		}
		else if(flag)
		{
			floor=atoi(temp.c_str());
			cur_layer++;
			pt_EventRun=EventRunList.begin();
			while(pointer>1)
			{
				pt_EventRun++;
				pointer--;
			}
			CEventRunOption* pOption;
			pOption=(CEventRunOption*)(*pt_EventRun);
			if(cur_layer==Layer)
			{
				if(floor>=8) floor-=7;
        if(floor>=6) floor-=5;
				if(floor>=1&&floor<=4)
					pOption->listOptionRun[floor-1].push_back(pEventRun);
				else if(floor==5)
					pOption->listCancelRun.push_back(pEventRun);
				return true;
			}
			else if(cur_layer!=Layer)
			{
				if(floor>=8) floor-=7;
        if(floor>=6) floor-=5;
				if(floor>=1&&floor<=4)
				{
					if(Put(pOption->listOptionRun[floor-1],pEventRun,Path.substr(count+1,Path.length()-count-1),cur_layer,Layer))
						return true;
				}
				else if(floor==5)
				{
					if(Put(pOption->listCancelRun,pEventRun,Path.substr(count+1,Path.length()-count-1),cur_layer,Layer))
						return true;
				}
			}
			temp="";
		}
  }
  return true;
}
//==========================================================================================
//Parameters:
//          char		szCharInteger
//Returns:
//          Get the integer from char 
int gf_GetIntegerFromChar( const char * szInteger, int iPos )
{	
#ifndef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
	FuncName("gf_GetIntegerFromChar");
  char    szIntegerLog[MAX_MEMO_MSG_LEN];
#endif
	
	static int			g_iGetInteger[512];
	static char			szRetNum[1024];
	
	int			iLength = strlen( szInteger );
	BOOL		bGet = FALSE;
	char		szChar = '0';
	int			i = 0, j = 0, k = 0;
	
	// Check Error
	if( iLength > 1024 )
	{
    Fprintf("CrashWatchForSC.txt","iLength>1024 ");
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
		_snprintf( szIntegerLog, MAX_MEMO_MSG_LEN-1, "The string length is so long, Length=%d #", iLength );
    szIntegerLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoErrMsg( szIntegerLog );
#endif
	}
	// Check Error
	if( iPos > 512 || iPos < 1 )
	{
    Fprintf("CrashWatchForSC.txt","iPos>512 ");
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
		_snprintf( szIntegerLog, MAX_MEMO_MSG_LEN-1, "Cannot get the %dth Integer, Position is so big or small", iPos );
    szIntegerLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoErrMsg(szIntegerLog);
#endif
	}
	// Scan the string
	memset( szRetNum, 0, sizeof( char ) * 1024 );
	memset( g_iGetInteger, 0, sizeof( int ) * 512 ); 
	for( i = 0, j = 0, k = 0; i < iLength; i++ )
	{
		szChar = szInteger[i];
		
		if( szChar >= 48 && szChar <= 57 )
		{
			if( j == 0 && szChar == 48 )
				continue;
			szRetNum[j] = szChar;
			j++;
		}
		else
		{
			if( j > 0 )
			{
				g_iGetInteger[k] = atoi( szRetNum );
				k++;
				j = 0;
				memset( szRetNum, 0, sizeof( char ) * 1024 );
			}
		}
	}
	// Get The Last Integer
	if( j > 0 )
	{
		g_iGetInteger[k] = atoi( szRetNum );
	}
	
	return g_iGetInteger[iPos-1];
}
//==========================================================================================
//Parameters:
//          String  stFilePath
//Returns:
//          true    Load Success
//          false   Load Fail
int CSrvBaseData::LoadBaseEvent( char * szFilePath )
{
  FuncName("CSrvBaseData::LoadBaseEvent");
	//load the file into the buffer
	
	// Load Mix Item List File Data And Load Warp Point List File Data
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, BF_MIXITEMLIST_FILE);
	g_pMixItemList->LoadFile( szInitLog );
	//
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, BF_WARPPOINT_FILE);
	g_pWaprPointList->LoadFile( szInitLog );

#ifdef  _NEW_TRADITIONS_WEDDING_

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, BF_AWANEMAP_FILE);
  g_AwaneMapMgr->LoadFile(szInitLog);

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
	strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, BF_SPOUSEWARPLIMITMAP_FILE);
  g_GSWeddingMgr->LoadSpouseWarpLimitMap(szInitLog);

#endif

  // gary 2002/04/27 modify+1
  string Content=""; //string Content;

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else	
	strcpy(szInitLog, m_szObjFile);
#endif
  strcat(szInitLog, "/");
  strcat(szInitLog, szFilePath);
  char* szBuffer=NULL;
  long iFileLength;
  Content=Load(szBuffer,szInitLog,iFileLength);
  if(Content.compare("No File") == 0 ||Content.compare("") == 0)
  {
		if(szBuffer)
		{
			delete []szBuffer;
			szBuffer = NULL;
		}
    return false;
		/*
    if( Content.compare("") == 0 )
			if( !DeleteFile( szFilePath ) )
				return 0;
			Content="{&1^task1^0^#*1^1^1^1^1^point1^#$1^1^#}";
			Save(Content,szInitLog);
			Content=Load(szBuffer,szInitLog,iFileLength);
		*/
  }
  delete []szBuffer;
  //
  long          Pointer;
  int           target = 0;
  CTask         *Task_temp = NULL;
  CEventPoint   *EventPoint_temp = NULL;
  CSubEvent     *SubEvent_temp = NULL;
  CEventRun     *EventRun_temp = NULL;
  CEventLimit   *EventLimit_temp = NULL;
  string        temp, Path;
  int           target_Count = 0;
  int           value = 0;
  int           cur_layer, Layer;
  int           Task_Layer = 0, i;
  //add by cece
  int           prev_target =0,prev_target_count=0;
  char          szErrMsg[2048];
  //
  //
  for(Pointer=0;Pointer<iFileLength;Pointer++)
  {
		temp=ContentGet(Content,Pointer,target,target_Count);//Content: the value you want
		//Pointer: the file pointer
		//target: the object
		
		//target_Count: the object's task
		
		if( temp.compare("EndTask") == 0)
		{
			Task_temp=NULL;
			EventPoint_temp=NULL;
			SubEvent_temp=NULL;
			EventRun_temp=NULL;
			EventLimit_temp=NULL;
			target=0;
		}
		else if(temp.compare("FileOver") == 0 )
			break;
		switch(target)
		{
    case TASK:
      //add by cece
      prev_target=target;
      prev_target_count=target_Count;
      //
			switch(target_Count)
			{
			case 0: if(Task_temp==NULL)
              {
                try{
                  value=atoi(temp.c_str());
                }
                catch(...)
                {
                  Task_temp=NULL;
                  break;
                }
                Task_temp=new CTask;
                Task_temp->iTaskId=value;
                break;
              }
        else break;
			case 1: if(Task_temp!=NULL)
								Task_temp->strName=temp;
				break;
			case 2: try{
				value=atoi(temp.c_str());
							}
				catch(...)
				{
					break;
				}
				if(Task_temp!=NULL)
					Task_temp->iPublic=value;
				//add by cece
				m_mapTask.insert(map<DWORD, CTask*>::value_type(Task_temp->iTaskId, Task_temp ));
				
				break;
			}
			break;
			case EVENTPOINT:
        //add by cece
        if(prev_target==TASK && prev_target_count!=2)
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          _snprintf(szErrMsg, 2048-1, "Task %d'data is wrong!", Task_temp->iTaskId );
          szErrMsg[2048-1] = '\0';
          AddMemoErrMsg( szErrMsg );
#endif
          return 0;
        }
        prev_target=target;
        prev_target_count=target_Count;
        //
				switch(target_Count)
				{
				case 0: if(Task_temp!=NULL)
								{
                  try{
										value=atoi(temp.c_str());
                  }
                  catch(...)
                  {
										EventPoint_temp=NULL;
										break;
                  }
                  EventPoint_temp=new CEventPoint;
                  EventPoint_temp->iPointId=value;
                  if( EventPoint_temp->iPointId == 8 && Task_temp->iTaskId == 12 )
                  {
                    int iiiiiii = 0;
                    iiiiiii++;
                  }
                  break;
								}
					else break;
				case 1: try{
					value=atoi(temp.c_str());
								}
					catch(...)
					{
						break;
					}
					if(EventPoint_temp!=NULL)
						EventPoint_temp->iPointType=value;
					break;
				case 2: try{
					value=atoi(temp.c_str());
								}
					catch(...)
					{
						break;
					}
					if(EventPoint_temp!=NULL)
						EventPoint_temp->iPicId=value;
					break;
				case 3: try{
					value=atoi(temp.c_str());
								}
					catch(...)
					{
						break;
					}
					if(EventPoint_temp!=NULL)
						EventPoint_temp->iFaceId=value;
					break;
				case 4: try{
					value=atoi(temp.c_str());
								}
					catch(...)
					{
						break;
					}
					if(EventPoint_temp!=NULL)
						EventPoint_temp->iPointAction=value;
					break;
				case 5: if(EventPoint_temp!=NULL)
								{
                  EventPoint_temp->strNpcName=temp;
                  Task_temp->listPoint.push_back(EventPoint_temp);
                  //add by cece
                  DWORD		taskid = Task_temp->iTaskId;
                  DWORD		pointid = EventPoint_temp->iPointId;
                  taskid = MAKELONG(pointid,taskid);
                  m_mapEventPoint.insert(map<DWORD, CEventPoint*>::value_type(taskid,EventPoint_temp) );
								}
					break;
				}
				break;
				case SUBEVENT:
          //add by cece
          if(prev_target==EVENTPOINT && prev_target_count!=5)
          {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
            _snprintf(szErrMsg,2048-1,"Task %d EventPoint %d'data is wrong!", 
                    Task_temp->iTaskId,
                    EventPoint_temp->iPointId );
            szErrMsg[2048-1] = '\0';       
            AddMemoErrMsg( szErrMsg );
#endif
            return 0;
          }
          prev_target=target;
          prev_target_count=target_Count;
          //
          switch(target_Count)
          {
					case 0: if(EventPoint_temp!=NULL)
									{
										try{
											value=atoi(temp.c_str());
										}
										catch(...)
										{
											SubEvent_temp=NULL;
											break;
										}
										SubEvent_temp=new CSubEvent;
										if(SubEvent_temp!=NULL)
										{
											SubEvent_temp->iId=value;
										}
										break;
									}
						else break;
					case 1: try{
						value=atoi(temp.c_str());
									}
						catch(...)
						{
							break;
						}
						if(SubEvent_temp!=NULL)
						{
							SubEvent_temp->iStartType=value;
							EventPoint_temp->listSubEvent.push_back(SubEvent_temp);
						}
						break;
					}
					break;
					case EVENTRUN:
            //add by cece
            if( (prev_target==SUBEVENT && prev_target_count!=1) ||
                (prev_target==EVENTLIMIT && prev_target_count!=5) )
            {
              if(prev_target==SUBEVENT)
              {
                _snprintf(szErrMsg,2048-1,"Task %d EventPoint %d SubEvent %d'data is wrong!", 
                        Task_temp->iTaskId,
                        EventPoint_temp->iPointId,
                        SubEvent_temp->iId );
                szErrMsg[2048-1] = '\0';
              }
              else
              {
                _snprintf(szErrMsg,2048-1,"Task %d EventPoint %d SubEvent %d EventLimit %d'data is wrong!", 
                        Task_temp->iTaskId,
                        EventPoint_temp->iPointId,
                        SubEvent_temp->iId,
                        EventLimit_temp->iId );
                szErrMsg[2048-1] = '\0';
              }
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
              AddMemoErrMsg( szErrMsg );
#endif
              return 0;
            }
            prev_target=target;
            prev_target_count=target_Count;
            //
						switch(target_Count)
						{
            case 1: if(SubEvent_temp!=NULL)
                    {
											try{
												value=atoi(temp.c_str());
											}
											catch(...)
											{
												break;
											}
											if(EventRun_temp!=NULL)
											{
												EventRun_temp->iId=value;
											}
											break;
                    }
							else break;
            case 2: try{
							value=atoi(temp.c_str());
                    }
							catch(...)
							{
								break;
							}
							if(EventRun_temp!=NULL)
							{
								EventRun_temp->iTargetType=value;
								//SubEvent_temp->listRun.push_back(EventRun_temp);
							}
							break;
						case  3: Path=temp;
							cur_layer=0;
							if(EventRun_temp!=NULL){
								switch(EventRun_temp->iRunType)
								{
									//case RUNTYPE_NONE:
									//                     if(LayerCount(Path)==3&&SubEvent_temp!=NULL)
									//                     SubEvent_temp->listRun.push_back(EventRun_temp);
									//                     break;
									//RUNTYPE_2VALUE
									//case EVENT_RUN_CHANGE_MSG_BOX:
								case EVENT_RUN_SET_GLOBAL_FLAG:
								case EVENT_RUN_SET_LOCAL_FLAG:
								case EVENT_RUN_SET_SAVE_FLAG:
								case EVENT_RUN_MONEY_INCREASE:
								case EVENT_RUN_MONEY_ASSIGN:
								case EVENT_RUN_ITEM_INCREASE:
								case EVENT_RUN_ITEM_ASSIGN:
								case EVENT_RUN_ATTR_ASSIGN:
								case EVENT_RUN_ATTR_INCREASE:
								case EVENT_RUN_SKILL_INCREASE:
								case EVENT_RUN_SKILL_ASSIGN:
									//case EVENT_RUN_PLAY_SOUND:
								case EVENT_RUN_CANCEL_EVENT:
								case EVENT_RUN_GOTO_SUBEVENT:
								case EVENT_RUN_RESTORE_ALL:
									//case EVENT_RUN_CHANGE_SCREEN:
									//case EVENT_RUN_SET_WAIT_TIME:
								case EVENT_RUN_SET_LABEL:
								case EVENT_RUN_GOTO_LABEL:
								//case EVENT_RUN_SHOW_FACE:
								case EVENT_RUN_WRITE_LOG_BOOK:
								//add by cece
         				case EVENT_RUN_RECORDTIME:
								case EVENT_RUN_WARP:
								//add by cece
								case EVENT_RUN_SHOWINTERFACE:
								case EVENT_RUN_TIMER_START:
								case EVENT_RUN_TIMER_STOP:
								//
								case EVENT_RUN_SUCKSOUL:
								case EVENT_RUN_FORGE:
                case EVENT_RUN_DECLARECITYWAR:
                case EVENT_RUN_ADJUSTTAX:
                case EVENT_RUN_TRANSFERUNION:
                case EVENT_RUN_BINDMONSTER:
                case EVENT_RUN_NPCDISAPPEAR:
                case EVENT_RUN_SETCASTELLAN:
                case EVENT_RUN_NPCAPPEAR:
                case EVENT_RUN_GETTAX:
                case EVENT_RUN_GATECONTROL:
                case EVENT_RUN_FIXGATE:
                case EVENT_RUN_LIST:
                case EVENT_RUN_BUYHOUSE:
                case EVENT_RUN_MAINTENANCE:
                case EVENT_RUN_MESSAGE:
                case EVENT_RUN_FIELD_QUERY:
                case EVENT_RUN_DOOR_QUERY:
                case EVENT_RUN_REPAIR_ITEM:
												CEventRun2Value* p2Value;
												p2Value=(CEventRun2Value*)EventRun_temp;
												for(i=0;i<2;i++)
												{
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													p2Value->iValue[i]=atoi(temp.c_str());
												}
												Layer=LayerCount(Path);
												if(Layer==3&&SubEvent_temp!=NULL&&EventRun_temp!=NULL)
													SubEvent_temp->listRun.push_back(EventRun_temp);
												else if(Layer>3&&!SubEvent_temp->listRun.empty())
													Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												break;
												//RUNTYPE_4VALUE
												//case EVENT_RUN_SET_WEATHER:
												//                     CEventRun4Value* p4Value;
												//                     p4Value=(CEventRun4Value*)EventRun_temp;
												//                     for(i=0;i<4;i++)
												//                     {
												//                        Pointer++;
												//                        temp=ContentGet(Content,Pointer,target,target_Count);
												//                        p4Value->iValue[i]=atoi(temp.c_str());
												//                     }
												//                     Layer=LayerCount(Path);
												//                     if(Layer==3&&EventRun_temp!=NULL)
												//                     SubEvent_temp->listRun.push_back(EventRun_temp);
												//                     else if(Layer>3&&!SubEvent_temp->listRun.empty())
												//                     Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												//                     break;
												//RUNTYPE_6VALUE
												//case EVENT_RUN_CHANGE_COLOR:
												//                     CEventRun6Value* p6Value;
												//                     p6Value=(CEventRun6Value*)EventRun_temp;
												//                     for(i=0;i<6;i++)
												//                     {
												//                      Pointer++;
												//                      temp=ContentGet(Content,Pointer,target,target_Count);
												//                      p6Value->iValue[i]=atoi(temp.c_str());
												//                     }
												//                     Layer=LayerCount(Path);
												//                     if(Layer==3&&EventRun_temp!=NULL)
												//                     SubEvent_temp->listRun.push_back(EventRun_temp);
												//                     else if(Layer>3&&!SubEvent_temp->listRun.empty())
												//                     Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												//                     break;
												//RUNTYPE_MSG
											case EVENT_RUN_SHOW_MSG:
											case EVENT_RUN_CONTENT:
											case EVENT_RUN_SHOWADJUST:
												//add by cece
											case EVENT_RUN_CHANGE_TITLE:
											case EVENT_RUN_SYS_INFO:
												CEventRunMsg* pMsg;
												pMsg=(CEventRunMsg*)EventRun_temp;
												//add by cece
												//Pointer++;
												//temp=ContentGet(Content,Pointer,target,target_Count);
												//pMsg->iMessId = atoi(temp.c_str());
												//
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
												pMsg->strMessage=temp;
												Layer=LayerCount(Path);
												if(Layer==3&&EventRun_temp!=NULL)
													SubEvent_temp->listRun.push_back(EventRun_temp);
												else if(Layer>3&&!SubEvent_temp->listRun.empty())
													Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												break;
												//RUNTYPE_GIFT
											case EVENT_RUN_GIFT:
												CEventRunGift*	pGift;
												pGift = (CEventRunGift*)EventRun_temp;
												//add by cece
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
												//strcpy( pGift->szTitle, temp.c_str() );
												memcpy( pGift->szTitle, temp.c_str(), MAX_NPC_TALK_LEN-1 );
												pGift->szTitle[MAX_NPC_TALK_LEN-1] = '\0';
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
												//strcpy( pGift->szTalkMsg, temp.c_str() );
												memcpy( pGift->szTalkMsg, temp.c_str(), MAX_NPC_TALK_LEN-1 );
												pGift->szTalkMsg[MAX_NPC_TALK_LEN-1] = '\0';
												//
												for(i=0; i<RUN_GIFT_MAXCOUNT; i++)
												{
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													pGift->GiftList[i].dwGiveItemId = atoi(temp.c_str());
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													pGift->GiftList[i].dwGiveCount = atoi(temp.c_str());
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
#ifdef _REPAIR_SERVER_CRASH_NICK_
													SafeStrcpy( pGift->GiftList[i].szPromptMsg, temp.c_str(), MAX_NPC_TALK_LEN );
#else
													strcpy(pGift->GiftList[i].szPromptMsg, temp.c_str() );
#endif
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													pGift->GiftList[i].dwGetItemId = atoi(temp.c_str());
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													pGift->GiftList[i].dwGetCount = atoi(temp.c_str());
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
#ifdef _REPAIR_SERVER_CRASH_NICK_
													SafeStrcpy( pGift->GiftList[i].szEndingMsg, temp.c_str(), MAX_NPC_TALK_LEN);
#else
													strcpy( pGift->GiftList[i].szEndingMsg, temp.c_str() );
#endif
												}
												Layer=LayerCount(Path);
												if(Layer==3&&EventRun_temp!=NULL)
													SubEvent_temp->listRun.push_back(EventRun_temp);
												else if(Layer>3&&!SubEvent_temp->listRun.empty())
													Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												break;
												//RUNTYPE_SHOP
											case EVENT_RUN_SHOP:
                      case EVENT_RUN_SHOP_NEW:
												CEventRunShop* pShop;
												pShop=(CEventRunShop*)EventRun_temp;
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
												pShop->m_iShopPicId=atoi(temp.c_str());
												//add by cece
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
#ifdef _REPAIR_SERVER_CRASH_NICK_
												SafeStrcpy( pShop->m_szTitle, temp.c_str(), MAX_NPC_TALK_LEN );
#else
												strcpy( pShop->m_szTitle, temp.c_str() );
#endif
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);

#ifdef _REPAIR_SERVER_CRASH_NICK_
												SafeStrcpy( pShop->m_szTalkMsg, temp.c_str(), MAX_NPC_TALK_LEN );
#else
												strcpy( pShop->m_szTalkMsg, temp.c_str() );
#endif
												//
												for(i=0;i<MAX_TRADE_LIST;i++)
												{
													Pointer++;
													temp = ContentGet(Content,Pointer,target,target_Count);
													pShop->m_iTradeList[i][0] = atoi(temp.c_str());
													Pointer++;
													temp = ContentGet(Content,Pointer,target,target_Count);
													pShop->m_iTradeList[i][1] = atoi(temp.c_str());
												}
												Pointer++;
												temp = ContentGet(Content,Pointer,target,target_Count);
												pShop->m_iBuyTypeList[0] = atoi(temp.c_str());
												Pointer++;
												temp = ContentGet(Content,Pointer,target,target_Count);
												pShop->m_iBuyTypeList[1] = atoi(temp.c_str());
												Pointer++;
												temp = ContentGet(Content,Pointer,target,target_Count);
												pShop->m_iBuyTypeList[2] = atoi(temp.c_str());
												////
												Layer=LayerCount(Path);
												if(Layer==3&&EventRun_temp!=NULL)
													SubEvent_temp->listRun.push_back(EventRun_temp);
												else if(Layer>3&&!SubEvent_temp->listRun.empty())
													Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												break;
												//RUNTYPE_OPTION
											case EVENT_RUN_OPTION:
											case EVENT_RUN_INPUT_VALUE:
											//										case EVENT_RUN_PROBABILITY:
												CEventRunOption* pOption;
												pOption=(CEventRunOption*)EventRun_temp;
												for(i=0;i<4;i++)
												{
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													pOption->OptionName[i]=temp;
												}
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
												pOption->iCancelType=atoi(temp.c_str());
												Layer=LayerCount(Path);
												if(Layer==3&&EventRun_temp!=NULL)
													SubEvent_temp->listRun.push_back(EventRun_temp);
												else if(Layer>3&&!SubEvent_temp->listRun.empty())
													Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												break;
											case EVENT_RUN_PROBABILITY:
												// Modify By WildCat 2002-7-31
											
												CEventRunOption* pOption1;
												pOption1=(CEventRunOption*)EventRun_temp;
												for(i=0;i<4;i++)
												{
													Pointer++;
													temp=ContentGet(Content,Pointer,target,target_Count);
													pOption1->iOdds[i] = gf_GetIntegerFromChar( temp.c_str(), 1 );
													if( i > 0 )
													{
														pOption1->iOdds[i] += pOption1->iOdds[i-1];
													}
												}
												if( i > 0 && pOption1->iOdds[i-1] != 100 )
												{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
													AddMemoErrMsg( "*** The All Odds != 100% !!! ***" );
#endif
													return 0;
												}
												Pointer++;
												temp=ContentGet(Content,Pointer,target,target_Count);
												pOption1->iCancelType=atoi(temp.c_str());
												Layer=LayerCount(Path);
												if(Layer==3&&EventRun_temp!=NULL)
													SubEvent_temp->listRun.push_back(EventRun_temp);
												else if(Layer>3&&!SubEvent_temp->listRun.empty())
													Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
												break;
										 //add by cece
                     case EVENT_RUN_WARPSELECT:
                     case EVENT_RUN_MIXANDSALE:
										 case EVENT_RUN_BALANCE_WARP:
                     case EVENT_RUN_SETMERCENARY:
                     case EVENT_RUN_CITYWARSEL:
                     case EVENT_RUN_PUTMONSTER:
                     case EVENT_RUN_BACKMUSIC:
                     case EVENT_RUN_RAND_GAIN_ITEM:
                       {
                         int iCount, iValue;
                         CEventRunWarpSel* pSelWarp = (CEventRunWarpSel*)EventRun_temp;
                         //
                         Pointer++;
                         temp=ContentGet(Content,Pointer,target,target_Count);
                         iValue = atoi( temp.c_str() );
                         pSelWarp->dwParam1 = iValue;
                         Pointer++;
                         temp=ContentGet(Content,Pointer,target,target_Count);
                         iValue = atoi( temp.c_str() );
                         pSelWarp->dwParam2 = iValue;
                         //
                         Pointer++;
                         temp=ContentGet(Content,Pointer,target,target_Count);
                         iCount = atoi(temp.c_str());
                         for( int i =0; i< iCount; i++ )
                         {
                            Pointer++;
                            temp=ContentGet(Content,Pointer,target,target_Count);
                            iValue = atoi(temp.c_str());
                            pSelWarp->listWarpPoint.push_back( iValue );
                         }
                         Layer=LayerCount(Path);
                         if(Layer==3&&EventRun_temp!=NULL)
                           SubEvent_temp->listRun.push_back(EventRun_temp);
                         else if(Layer>3&&!SubEvent_temp->listRun.empty())
                           Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
                       }
                       break;
                       //
                       //add by cece
                     case EVENT_RUN_CHECKITEM:
                       {
                         CEventRunOption* pOption;
                         pOption=(CEventRunOption*)EventRun_temp;
                         for(int i =0; i<4;i++)
                         {
                           pOption->OptionName[i] = string("");
                         }
                         //CCheckItem* pItem;
                         //int icount,ivalue;
                         int icount = 0;
                         Pointer++;
                         temp=ContentGet(Content,Pointer,target,target_Count);
                         icount = atoi(temp.c_str());
                         if( icount > 0 )
                         {
                           if( icount >1 )
                           {
                             pOption->pCheckItem = new CCheckItem[icount];
                             pOption->iOddsIndex = icount;
                           }
                           else
                           {
                             pOption->pCheckItem = new CCheckItem;
                             pOption->iOddsIndex = 1;
                           }
                         }
                         else
                         {
                           pOption->pCheckItem = NULL;
                           pOption->iOddsIndex = 0;
                         }
                         CCheckItem checkTemp;
                         for( i=0; i<icount; i++)
                         {
                           Pointer++;
                           temp=ContentGet(Content,Pointer,target,target_Count);
                           checkTemp.wId = atoi(temp.c_str());
                           Pointer++;
                           temp=ContentGet(Content,Pointer,target,target_Count);
                           checkTemp.wCount = atoi(temp.c_str());
                           Pointer++;
                           temp=ContentGet(Content,Pointer,target,target_Count);
                           checkTemp.wPlace = atoi(temp.c_str());
                           pOption->pCheckItem[i] = checkTemp;
                         }
                         //
                         Layer=LayerCount(Path);
                         if(Layer==3&&EventRun_temp!=NULL)
                         {
                           SubEvent_temp->listRun.push_back(EventRun_temp);
                         }
                         else if(Layer>3&&!SubEvent_temp->listRun.empty())
                         {
                           Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
                         }
                         //
                       }
                       break;
                       //
                     case EVENT_RUN_CONDITION:
                       {
                         CEventRunOption* pOption;
                         pOption = (CEventRunOption*)EventRun_temp;
                         pOption->pLimit = NULL;
                         pOption->pLimit = new CEventLimit;
                         if (pOption->pLimit != NULL)
                         {
                           Pointer++;
                           temp = ContentGet(Content,Pointer,target,target_Count);
                           pOption->pLimit->iTargetType = atoi(temp.c_str());
                           Pointer++;
                           temp = ContentGet(Content,Pointer,target,target_Count);
                           pOption->pLimit->iLimitType  = atoi(temp.c_str());
                           Pointer++;
                           temp = ContentGet(Content,Pointer,target,target_Count);
                           pOption->pLimit->iObject     = atoi(temp.c_str());
                           Pointer++;
                           temp = ContentGet(Content,Pointer,target,target_Count);
                           pOption->pLimit->iValue      = atoi(temp.c_str());
                           Pointer++;
                           temp = ContentGet(Content,Pointer,target,target_Count);
                           pOption->pLimit->strName     = temp;
                           Layer=LayerCount(Path);
                           if(Layer==3&&EventRun_temp!=NULL)
                             SubEvent_temp->listRun.push_back(EventRun_temp);
                           else if(Layer>3&&!SubEvent_temp->listRun.empty())
                             Put(SubEvent_temp->listRun,EventRun_temp,Path.substr(1,Path.length()-1),cur_layer,Layer);
                         }
                       }//end case condition
                       break;
                     }//end switch
                   }
                   break;
           case  0: if(SubEvent_temp!=NULL){
						 try{
							 value=atoi(temp.c_str());
						 }
						 catch(...)
						 {
							 EventRun_temp=NULL;
							 break;
						 }
						 switch(value)
						 {
							 //case RUNTYPE_NONE:EventRun_temp=new CEventRun;break;
							 //RUNTYPE_2VALUE
						 case EVENT_RUN_CHANGE_MSG_BOX:
						 case EVENT_RUN_SET_GLOBAL_FLAG:
						 case EVENT_RUN_SET_LOCAL_FLAG:
						 case EVENT_RUN_SET_SAVE_FLAG:
						 case EVENT_RUN_MONEY_INCREASE:
						 case EVENT_RUN_MONEY_ASSIGN:
						 case EVENT_RUN_ITEM_INCREASE:
						 case EVENT_RUN_ITEM_ASSIGN:
						 case EVENT_RUN_ATTR_ASSIGN:
						 case EVENT_RUN_ATTR_INCREASE:
						 case EVENT_RUN_SKILL_INCREASE:
						 case EVENT_RUN_SKILL_ASSIGN:
							 //                     case EVENT_RUN_PLAY_SOUND:
						 case EVENT_RUN_CANCEL_EVENT:
						 case EVENT_RUN_GOTO_SUBEVENT:
						 case EVENT_RUN_RESTORE_ALL:
						 case EVENT_RUN_CHANGE_SCREEN:
						 case EVENT_RUN_SET_WAIT_TIME:
						 case EVENT_RUN_SET_LABEL:
						 case EVENT_RUN_GOTO_LABEL:
						 case EVENT_RUN_SHOW_FACE:
						 case EVENT_RUN_WRITE_LOG_BOOK:
						 //add by cece
						 case EVENT_RUN_RECORDTIME:
						 case EVENT_RUN_WARP:
						 //add by cece
             case EVENT_RUN_SHOWINTERFACE:
             case EVENT_RUN_TIMER_START:
             case EVENT_RUN_TIMER_STOP:
             //
             case EVENT_RUN_SUCKSOUL:
             case EVENT_RUN_FORGE:
             case EVENT_RUN_DECLARECITYWAR:
             case EVENT_RUN_ADJUSTTAX:
             case EVENT_RUN_TRANSFERUNION:
             case EVENT_RUN_BINDMONSTER:
             case EVENT_RUN_NPCDISAPPEAR:
             case EVENT_RUN_SETCASTELLAN:
             case EVENT_RUN_NPCAPPEAR:
             case EVENT_RUN_GETTAX:
             case EVENT_RUN_GATECONTROL:
             case EVENT_RUN_FIXGATE:
             case EVENT_RUN_LIST:
             case EVENT_RUN_BUYHOUSE:
             case EVENT_RUN_MAINTENANCE:
             case EVENT_RUN_MESSAGE:
             case EVENT_RUN_FIELD_QUERY:
             case EVENT_RUN_DOOR_QUERY:
             case EVENT_RUN_REPAIR_ITEM:
							    CEventRun2Value* p2Value;
							    p2Value=new CEventRun2Value;
							    EventRun_temp=(CEventRun*)p2Value;
							    break;
						 case EVENT_RUN_SHOW_MSG:
						 case EVENT_RUN_CONTENT:
						 case EVENT_RUN_SHOWADJUST:
							 //add by cece
						 case EVENT_RUN_CHANGE_TITLE:
						 case EVENT_RUN_SYS_INFO:
							 CEventRunMsg* pMsg;
							 pMsg=new CEventRunMsg;
							 EventRun_temp=(CEventRun*)pMsg;
							 break;
							 //RUNTYPE_SHOP
						 case EVENT_RUN_SHOP:
             case EVENT_RUN_SHOP_NEW:
							 CEventRunShop* pShop;
							 pShop=new CEventRunShop;
							 EventRun_temp=(CEventRun*)pShop;
							 break;
							 //RUNTYPE_OPTION
						 case EVENT_RUN_OPTION:
						 case EVENT_RUN_INPUT_VALUE:
						 case EVENT_RUN_PROBABILITY:
						 //
						 case EVENT_RUN_CHECKITEM:
             case EVENT_RUN_CONDITION:
						 //
							 CEventRunOption* pOption;
							 pOption=new CEventRunOption;
							 EventRun_temp=(CEventRun*)pOption;
							 break;
               
							 //RUNTYPE_GIFT
						 case EVENT_RUN_GIFT:
							 CEventRunGift*  pGift;
							 pGift = new CEventRunGift;
							 EventRun_temp = (CEventRun*)pGift;
							 break;
						 //
             case EVENT_RUN_WARPSELECT:
             case EVENT_RUN_MIXANDSALE:
             case EVENT_RUN_BALANCE_WARP:
             case EVENT_RUN_SETMERCENARY:
             case EVENT_RUN_CITYWARSEL:
             case EVENT_RUN_PUTMONSTER:
             case EVENT_RUN_BACKMUSIC:
             case EVENT_RUN_RAND_GAIN_ITEM:
               {
                 CEventRunWarpSel* pWarpSel;
                 pWarpSel = new CEventRunWarpSel;
                 EventRun_temp = (CEventRun*)pWarpSel;
               }
               break;
               //
             default: EventRun_temp=NULL;
               break;
             }
             if(EventRun_temp!=NULL)
             {
               //InitEventRun(*EventRun_temp);
               EventRun_temp->iRunType=value;
             }
             break;
                    }
                    else break;
           }
           break;
    case EVENTLIMIT:
      //add by cece
      if( (prev_target==SUBEVENT && prev_target_count!=1) ||
          (prev_target==EVENTRUN && prev_target_count!=3) )
      {
        if(prev_target==SUBEVENT)
        {
          _snprintf(szErrMsg,2048-1,"Task %d EventPoint %d SubEvent %d'data is wrong!", 
            Task_temp->iTaskId,
            EventPoint_temp->iPointId,
            SubEvent_temp->iId );
          szErrMsg[2048-1] = '\0';
        }
        else
        {
          _snprintf(szErrMsg,2048-1,"Task %d EventPoint %d SubEvent %d EventRun %d'data is wrong!", 
            Task_temp->iTaskId,
            EventPoint_temp->iPointId,
            SubEvent_temp->iId,
            EventRun_temp->iId );
          szErrMsg[2048-1] = '\0';
        }
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        AddMemoErrMsg( szErrMsg );
#endif
        return 0;
      }
      prev_target=target;
      prev_target_count=target_Count;
      //
			switch(target_Count)
			{
			case 0: if(SubEvent_temp!=NULL)
							{
								try{
									value=atoi(temp.c_str());
								}
								catch(...)
								{
									EventLimit_temp=NULL;
									break;
								}
								EventLimit_temp=new CEventLimit;
								if(EventLimit_temp!=NULL)
								{
									//InitEventLimit(*EventLimit_temp);
									EventLimit_temp->iId=value;
								}
								break;
							}
				else break;
			case 1: try{
				value=atoi(temp.c_str());
							}
				catch(...)
				{
					break;
				}
				if(EventLimit_temp!=NULL)
					EventLimit_temp->iTargetType=value;
				break;
			case 2: try{
				value=atoi(temp.c_str());
							}
				catch(...)
				{
					break;
				}
				if(EventLimit_temp!=NULL)
					EventLimit_temp->iLimitType=value;
				break;
			case 3:  try{
				value=atoi(temp.c_str());
							 }
				catch(...)
				{
					break;
				}
				if(EventLimit_temp!=NULL)
					EventLimit_temp->iObject=value;
				break;
			case 4:  try{
				value=atoi(temp.c_str());
							 }
				catch(...)
				{
					break;
				}
				if(EventLimit_temp!=NULL)
					EventLimit_temp->iValue=value;
				break;
			case 5: if(EventLimit_temp!=NULL)
							{
								EventLimit_temp->strName=temp;
								SubEvent_temp->listLimit.push_back(EventLimit_temp);
							}
				break;
			}
			break;
    }
    target_Count++;
  }
  return 1;
}

int CSrvBaseData::LoadNpcTranslucency( char * szFilePath )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
FuncName("CSrvBaseData::LoadNpcTranslucency");
  #ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  CInStream    NpcTranslucency( szInitLog );
  int iCount = 0;
  if( NpcTranslucency.fail() ||NpcTranslucency.GetFileSize() == 0 )
  {
    return false;
  }
 if( NpcTranslucency >> iCount )  
 {
   if( iCount > 0 )
   {
     DWORD dwTaskid  = 0;
     DWORD dwPointid = 0;
     for( int i = 0; i < iCount; i++ )
     {
       if( NpcTranslucency >> dwTaskid >> dwPointid )
       {
         dwTaskid = MAKELONG(dwPointid,dwTaskid);
         m_mapNpcTranslucency.push_back(dwTaskid);
       }
     }
   }
 }
 else
 {
#ifdef _DEBUG
   MessageBox( GetActiveWindow(), "Load NpcTranslucency.txt iCount Error", "Warning...", MB_OK );
#endif
 }
  return true;
#else
  FuncName("CSrvBaseData::LoadNpcTranslucency");

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, m_szObjFile, MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, m_szObjFile );
#endif
  strcat( szInitLog, "/" );
  strcat( szInitLog, szFilePath );
  FILE *pFile = fopen(szInitLog, "r"); 
  if( !pFile )
  {
    return false; 
  }

  int iCount = 0;
  // Get the total
	fscanf( pFile,"%d ", &iCount );
	if( iCount > 0 )
  {
    DWORD dwTaskid  = 0;
    DWORD dwPointid = 0;
    for( int i = 0; i < iCount; i++ )
    {
      if (2 == fscanf( pFile, "%d %d", &dwTaskid, &dwPointid))
      {
        //m_mapEventPoint;
        dwTaskid = MAKELONG(dwPointid,dwTaskid);
        m_mapNpcTranslucency.push_back(dwTaskid);
      }
    }
  }
  return true;
#endif //_DEBUG_JAPAN_DECRYPT_
}

//==========================================================================================
//Parameters:
//          String  stFilePath
//Returns:
//          true    Load Success
//          false   Load Fail
BOOL CopyMessage( SMsgData * pTheScrMsg, SMsgData * pTheDesMsg )
{
	// Check Error, Check Message Params
	if( pTheScrMsg->dwMsgLen > 2 || pTheScrMsg->dwMsgLen < 0 )
	{
		return FALSE;
	}
	pTheDesMsg->dwAID    = pTheScrMsg->dwAID;
	pTheDesMsg->dwMsgID  = pTheScrMsg->dwMsgID;
	pTheDesMsg->dwMsgLen = pTheScrMsg->dwMsgLen;
  pTheDesMsg->Msgs[0].Size = 0;
  pTheDesMsg->Msgs[1].Size = 0;
	if( pTheScrMsg->dwMsgLen == 0 )
	{
		pTheDesMsg->Msgs[0].Size = 0;
		pTheDesMsg->Msgs[1].Size = 0;
		return TRUE;
	}
	else if( pTheScrMsg->dwMsgLen == 1 )
	{
		if( pTheScrMsg->Msgs[0].Size > MAXMSGDATASIZE )
		{
			return FALSE;
		}
		pTheDesMsg->Msgs[0].Size = pTheScrMsg->Msgs[0].Size;
		memcpy( pTheDesMsg->Msgs[0].Data, pTheScrMsg->Msgs[0].Data, pTheScrMsg->Msgs[0].Size );
		pTheDesMsg->Msgs[1].Size = 0;
		return TRUE;
	}
	else if( pTheScrMsg->dwMsgLen == 2 )
	{
		if( pTheScrMsg->Msgs[0].Size > MAXMSGDATASIZE || pTheScrMsg->Msgs[1].Size > MAXMSGDATASIZE )
		{
			return FALSE;
		}
		pTheDesMsg->Msgs[0].Size = pTheScrMsg->Msgs[0].Size;
		memcpy( pTheDesMsg->Msgs[0].Data, pTheScrMsg->Msgs[0].Data, pTheScrMsg->Msgs[0].Size );
		pTheDesMsg->Msgs[1].Size = pTheScrMsg->Msgs[1].Size;
		memcpy( pTheDesMsg->Msgs[1].Data, pTheScrMsg->Msgs[1].Data, pTheScrMsg->Msgs[1].Size );
		return TRUE;
	}
	return FALSE;
}
//==========================================================================================
//Parameters:
//          String  stFilePath
//Returns:
//          true    Load Success
//          false   Load Fail
BOOL CopyMessage( const SMsgData & TheScrMsg, SMsgData * pTheDesMsg )
{
	// Check Error, Check Message Params
	if( TheScrMsg.dwMsgLen > 2 || TheScrMsg.dwMsgLen < 0 )
	{
		return FALSE;
	}
	pTheDesMsg->dwAID    = TheScrMsg.dwAID;
	pTheDesMsg->dwMsgID  = TheScrMsg.dwMsgID;
	pTheDesMsg->dwMsgLen = TheScrMsg.dwMsgLen;
  pTheDesMsg->Msgs[0].Size = 0;
  pTheDesMsg->Msgs[1].Size = 0;
	if( TheScrMsg.dwMsgLen == 0 )
	{
		pTheDesMsg->Msgs[0].Size = 0;
		pTheDesMsg->Msgs[1].Size = 0;
		return TRUE;
	}
	else if( TheScrMsg.dwMsgLen == 1 )
	{
		if( TheScrMsg.Msgs[0].Size > MAXMSGDATASIZE )
		{
			return FALSE;
		}
		pTheDesMsg->Msgs[0].Size = TheScrMsg.Msgs[0].Size;
		memcpy( pTheDesMsg->Msgs[0].Data, TheScrMsg.Msgs[0].Data, TheScrMsg.Msgs[0].Size );
		pTheDesMsg->Msgs[1].Size = 0;
		return TRUE;
	}
	else if( TheScrMsg.dwMsgLen == 2 )
	{
		if( TheScrMsg.Msgs[0].Size > MAXMSGDATASIZE || TheScrMsg.Msgs[1].Size > MAXMSGDATASIZE )
		{
			return FALSE;
		}
		pTheDesMsg->Msgs[0].Size = TheScrMsg.Msgs[0].Size;
		memcpy( pTheDesMsg->Msgs[0].Data, TheScrMsg.Msgs[0].Data, TheScrMsg.Msgs[0].Size );
		pTheDesMsg->Msgs[1].Size = TheScrMsg.Msgs[1].Size;
		memcpy( pTheDesMsg->Msgs[1].Data, TheScrMsg.Msgs[1].Data, TheScrMsg.Msgs[1].Size );
		return TRUE;
	}
	return FALSE;
}
////////////////////////////////////////////////////////////////////////
//  
//  复制消息TheScrMsg内容到pTheDesMsg
//
BOOL CopyMessage( const SMccMsgData &TheScrMsg, SMccMsgData *pTheDesMsg )
{
	// Check Error, Check Message Params
	if( TheScrMsg.dwMsgLen > 2 || TheScrMsg.dwMsgLen < 0 )
	{
		return FALSE;
	}
	pTheDesMsg->dwAID    = TheScrMsg.dwAID;
	pTheDesMsg->dwMsgLen = TheScrMsg.dwMsgLen;
  pTheDesMsg->Msgs[0].Size = 0;
  pTheDesMsg->Msgs[1].Size = 0;
	if( TheScrMsg.dwMsgLen == 0 )
	{
		pTheDesMsg->Msgs[0].Size = 0;
		pTheDesMsg->Msgs[1].Size = 0;
		return TRUE;
	}
	else if( TheScrMsg.dwMsgLen == 1 )
	{
		if( TheScrMsg.Msgs[0].Size > MAXMSGDATASIZE )
		{
			return FALSE;
		}
		pTheDesMsg->Msgs[0].Size = TheScrMsg.Msgs[0].Size;
		memcpy( pTheDesMsg->Msgs[0].Data, TheScrMsg.Msgs[0].Data, TheScrMsg.Msgs[0].Size );
		pTheDesMsg->Msgs[1].Size = 0;
		return TRUE;
	}
	else if( TheScrMsg.dwMsgLen == 2 )
	{
		if( TheScrMsg.Msgs[0].Size > MAXMSGDATASIZE || TheScrMsg.Msgs[1].Size > MAXMSGDATASIZE )
		{
			return FALSE;
		}
		pTheDesMsg->Msgs[0].Size = TheScrMsg.Msgs[0].Size;
		memcpy( pTheDesMsg->Msgs[0].Data, TheScrMsg.Msgs[0].Data, TheScrMsg.Msgs[0].Size );
		pTheDesMsg->Msgs[1].Size = TheScrMsg.Msgs[1].Size;
		memcpy( pTheDesMsg->Msgs[1].Data, TheScrMsg.Msgs[1].Data, TheScrMsg.Msgs[1].Size );
		return TRUE;
	}
	return FALSE;
}

//add by cece
//Implementation
//I/O operation
//===============================================================================================
//
//
/*
ofstream& operator<<( ofstream& out, WarpSel& item )
{
      out<<item.iID<<' '
         <<item.iMapID<<' '
         <<item.iCoorX<<' '
         <<item.iCoorY<<' '
         <<item.iCost<<' '
         <<item.str_warpname<<' '
         <<item.str_erromsg;
      return out;
}
//===============================================================================================
//
//
ifstream& operator>>( ifstream& in,  WarpSel& item )
{
     in>>item.iID
       >>item.iMapID
       >>item.iCoorX
       >>item.iCoorY
       >>item.iCost
       >>item.str_warpname
       >>item.str_erromsg;
     return in;
}
*/
//===============================================================================================
//
//
/*BOOL CWarpMap::SaveFile( char* szFileName )
{
     if(!szFileName) return FALSE;
     ofstream  outStream;
     outStream.open( szFileName );
     if( outStream.fail() ) return FALSE;
		 int itotalsize = warpMap.ListSize();
		 for( int i = 0; i<itotalsize; i++)
		 {
			    outStream<<warpMap[i];
					if( i!=itotalsize-1 )  outStream<<endl;
		 }
     outStream.close();
     warpMap.Resize(0);
     return TRUE;
}*/
//===============================================================================================
//
//
BOOL CWarpMap::LoadFile( char* szFileName )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
	  CInStream   WarpList( szFileName );
    WarpSel     item;
		char        str1[512], str2[512];
    if ( !WarpList.fail() && WarpList.GetFileSize() != 0) 
    {
      warpMap.Resize(0);
      while ( WarpList >> item.iID >> item.iMapID
                       >> item.iCoorX >> item.iCoorY
                       >> item.iCost >> str1 >>str2 ) 
      {
        item.str_warpname = str1;
        item.str_erromsg  = str2;
        int n = warpMap.ListSize();
        n++;
        warpMap.Resize(n);
        warpMap[n-1] = item;
        
      }
    }
    else
    {
      MessageBox( GetActiveWindow(), "Load The WarpList.txt Error", "Warning...", MB_OK );
      return false;
    }
    return TRUE;
#else
//
    /*if(!szFileName) return FALSE;
    ifstream    inStream;
    inStream.open( szFileName );
    if( inStream.fail() ) return FALSE;
    warpMap.Resize(0);
    WarpSel item;
    while( inStream>>item )
    {
			 int n = warpMap.ListSize();
			 n++;
			 warpMap.Resize(n);
       warpMap[n-1] = item;
    }
    inStream.close();
    return TRUE;*/
		
	  WarpSel     item;
	  FILE        *fp = fopen(szFileName,"r");
		char        str1[512], str2[512];
    //
	  if( fp == NULL )    return FALSE;
		warpMap.Resize(0);
		while( fscanf( fp,"%hd %hd %hd %hd %d %s %s",
                   &item.iID, &item.iMapID, &item.iCoorX, &item.iCoorY,
                   &item.iCost, str1,str2 ) == 7 )
		{
		  item.str_warpname = str1;
			item.str_erromsg  = str2;
		  //warpMap.insert( WARPMAP::value_type(item.iID,item) );
      int n = warpMap.ListSize();
			n++;
			warpMap.Resize(n);
      warpMap[n-1] = item;
		}
	  return TRUE;
#endif //_DEBUG_JAPAN_DECRYPT_
}
//===============================================================================================
//
//
WarpSel* CWarpMap::GetItemData( int iID )
{
   if( iID <= 0 || iID > warpMap.ListSize() ) return NULL;
   return &warpMap[iID-1];
}
//I/O operation
//===============================================================================================
//
//
/*ofstream& operator<<( ofstream& out, Material& item )
{
   out<<' '<<item.iItemIndex<<' '
      <<item.iCount;
   return out;
}
//===============================================================================================
//
//
ifstream& operator>>( ifstream& in,  Material& item )
{
   in>>item.iItemIndex
     >>item.iCount;
   return in;
}
//===============================================================================================
//
//
ofstream& operator<<( ofstream& out, MixItem& item )
{
   out<<item.iID<<' '
      <<item.iItemIndex<<' '
      <<item.iType<<' '
      <<item.iCost<<' ';
	 int isize = item.materialLst.ListSize();
   out<<isize;
	 for( int i =0; i<isize; i++)
	 {
		  out<<item.materialLst[i];
	 }
   return out;
}
//===============================================================================================
//
//
ifstream& operator>>( ifstream& in,  MixItem& item )
{
   in>>item.iID
     >>item.iItemIndex
     >>item.iType
     >>item.iCost;
	 item.materialLst.Resize(0);
   int isize;
   in>>isize;
	 item.materialLst.Resize(isize);
   Material material;
   for( int i=0; i< isize; i++ )
   {
        in>>material;
				item.materialLst[i] = material;
   }
   return in;
}
//===============================================================================================
//
//
ofstream& operator<<( ofstream& out, CMixItem& item )
{
	   int  isize = item.m_mixitemMap.ListSize();
	   for( int i = 0; i<isize; i++ )
		 {
			    out<<item.m_mixitemMap[i];
					if( i!=isize-1 ) out<<endl;
		 }
     return out;
}
//===============================================================================================
//
//
ifstream& operator>>( ifstream& in, CMixItem& item)
{
    //clear
	  item.m_mixitemMap.Resize(0);
    //
    MixItem mixitem;
    while( in>>mixitem )
    {
			int n = item.m_mixitemMap.ListSize();
			n++;
			item.m_mixitemMap.Resize(n);
			item.m_mixitemMap[n-1] = mixitem;
    }
    return in;
}
//===============================================================================================
//
//
BOOL CMixItem::SaveFile( char* szFileName )
{
     if(!szFileName) return FALSE;
     ofstream  outStream;
     outStream.open( szFileName );
     if( outStream.fail() ) return FALSE;
     outStream<<(*this);
		 m_mixitemMap.Resize(0);
     outStream.close();
     return TRUE;
}
*/
//===============================================================================================
//
//
BOOL CMixItem::LoadFile( char* szFileName )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  CInStream     LoadFile( szFileName );
  if ( !LoadFile.fail() && LoadFile.GetFileSize() != 0 ) 
  {
    m_mixitemMap.Resize(0);
    int itotal=0,isize;
    MixItem mixitem;
    Material material;
    
    while( LoadFile >> mixitem.iID
                    >> mixitem.iItemIndex
                    >> mixitem.iType
                    >> mixitem.iCost )
    {
      if( LoadFile >> isize )
      {
        mixitem.materialLst.Resize(isize);
        if ( isize > 0 ) 
        {
          for ( int i = 0; i<isize; i++ ) 
          {
            int iItemIndex,iCount;
            if(  LoadFile >> iItemIndex >> iCount )
            {
             	material.iItemIndex = iItemIndex;
              material.iCount = iCount;
              mixitem.materialLst[i]=material;
            }
          }
        }
        itotal++;
        m_mixitemMap.Resize(itotal);
        m_mixitemMap[itotal-1]=mixitem;
      }
    }    
  }
  else
  {
    MessageBox( GetActiveWindow(), "Load The MixITemList.txt Error", "Warning...", MB_OK );
    return FALSE;
  }
  return TRUE;
#else
//
    /*if(!szFileName) return FALSE;
    ifstream    inStream;
    inStream.open( szFileName );
    if( inStream.fail() ) return FALSE;
    inStream>>(*this);
    inStream.close();
    return TRUE;*/
		FILE *fp = fopen( szFileName, "r" );
		if( fp == NULL ) return FALSE;
		m_mixitemMap.Resize(0);
		int itotal=0,isize;
		MixItem mixitem;
		Material material;
    int ret;
		while( (ret=fscanf( fp, "%u %u %u %u",
                   &mixitem.iID, &mixitem.iItemIndex,
                   &mixitem.iType, &mixitem.iCost )) == 4 )
		{
			if(fscanf( fp, "%d", &isize )==1)
			{
				mixitem.materialLst.Resize(isize);
				if(isize>0)
				{	
					for(int i =0; i<isize;i++ )
					{
						int iItemIndex,iCount;
						if( fscanf( fp, "%d %d", &iItemIndex, &iCount ) == 2 )
						{
							//mixitem.materialLst.push_back(material);
							material.iItemIndex = iItemIndex;
							material.iCount = iCount;
							mixitem.materialLst[i]=material;
						}
					}
				}
        itotal++;
				m_mixitemMap.Resize(itotal);
				m_mixitemMap[itotal-1]=mixitem;
        //m_mixitemMap.insert( MIXITEMMAP::value_type(mixitem.iID,mixitem) );
			}
		}
		fclose(fp);
		fp=NULL;
		return TRUE;
#endif //_DEBUG_JAPAN_DECRYPT_
}
//===============================================================================================
//
//
const Material* CMixItem::GetItemData( MixItem* pitem, int iID )
{
   if(pitem == NULL) return NULL;
	 if( iID <= 0 || iID > pitem->materialLst.ListSize() ) return NULL;
	 return &pitem->materialLst[iID-1];
}
//===============================================================================================
//
//
const MixItem*  CMixItem::GetItemData( int iID )
{
	  if( iID <= 0 || iID > m_mixitemMap.ListSize() ) return NULL;
		return &m_mixitemMap[iID-1];
}
//===============================================================================================
//
//
int  CMixItem::GetSize( int iType , int iID )
{
  switch( iType )
  {
  case MATERIAL:
    {
      const MixItem* pitem = GetItemData( iID );
      if( pitem )
      {
        return pitem->materialLst.ListSize();
      }
      return 0;
    }
  case MIXITEM:  return m_mixitemMap.ListSize();
  default: return 0;
  }
}
bool CSrvBaseData::CheckNpcTranslucency(DWORD dwEvent)
{
  list<DWORD>::iterator itr_list = m_mapNpcTranslucency.begin();
  for( ; itr_list != m_mapNpcTranslucency.end(); itr_list++)
  {
    if (*itr_list == dwEvent)
    {
      return true;
    }
  }
  return false;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckEventAndNpc()
{
  return true;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckNpcAndItem()
{
  return true;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckNpcAndSkill()
{
  return true;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckNpcAndWarpPointList()
{
  return true;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckNpcAndMixItemList()
{
  return true;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckMonsterAndDropItem()
{
  return true;
}
//===============================================================================================
//
//
bool CSrvBaseData::CheckMonsterAndSkill()
{
  return true;
}
//===============================================================================================
//
//
double            TimeMagicNum = 0;
LARGE_INTEGER     ti;
void InitTimeGetTime()
{
  LARGE_INTEGER  tf;
  QueryPerformanceFrequency( &tf );
  TimeMagicNum = (double)tf.QuadPart/1000.0;
  //
  QueryPerformanceCounter( &ti );
}
//===============================================================================================
//
//
DWORD TimeGetTime()
{
  LARGE_INTEGER   tc;
  QueryPerformanceCounter( &tc );
  return (DWORD)((tc.QuadPart-ti.QuadPart)/TimeMagicNum);
}

/////////////////////////////////////////////////////////////////////////////
//
//class CWarTime define
BOOL  CWarTime::Delete( int iIndex )
{
  int i=0;
  for( list<SCityWarInfo>::iterator iter= m_WarTimeList.begin();
       iter != m_WarTimeList.end(); i++, iter++ )
  {
    if( i==iIndex )
    {
       m_WarTimeList.erase( iter );
       return TRUE;
    }
  }
  return FALSE;
}

SCityWarInfo* CWarTime::Get( int iIndex )
{
  int i=0;
  for( list<SCityWarInfo>::iterator iter= m_WarTimeList.begin();
       iter != m_WarTimeList.end(); i++, iter++ )
  {
    if( i==iIndex )
    {
      return &(*iter);
    }
  }
  return NULL;
}

void CWarTime::Load( const char* szFileName )
{
  ifstream ifile(szFileName);
  if( ifile.fail() ) return;
  Clear();
  SCityWarInfo sInfo;
  while( ifile>>sInfo.iMapID )
  {
    ifile>>sInfo.iTimeSlice>>sInfo.iValid;
    Insert( sInfo );
  }
  ifile.close();
}

void CWarTime::Save( const char* szFileName )
{
  ofstream ofile(szFileName);
  if( ofile.fail() ) return;
  SCityWarInfo* pInfo = NULL;
  int iCount = Size();
  for( int i=0; i<iCount; i++ )
  {
    pInfo = Get( i );
    if( pInfo )
    {
      ofile<<pInfo->iMapID<<'\t'
           <<pInfo->iTimeSlice<<'\t'
           <<pInfo->iValid<<'\n';
    }
  }
  ofile.close();
}
//=======================================================================================
//
//
BOOL CMercenaryManager::Load( char * szFileName )
{
#ifndef  __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
  FuncName("CMercenaryManager::Load");
  char                      szShowLog[1024];
#endif
  //
  if( szFileName == NULL )  return FALSE;
  m_mapMercenary.clear();
  //
#ifdef _DEBUG_JAPAN_DECRYPT_  //Add By Jack.Ren 05/09/16
  CInStream in( szFileName );
  if( in.fail() || in.GetFileSize() == 0 ) //修正CInStream的缺陷 
#else
  ifstream in( szFileName );
  if( in.fail() )
#endif//_DEBUG_JAPAN_DECRYPT_
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( szShowLog, 1024-1, "Cannot Open Mercenary File[%s]...", szFileName );
    szShowLog[1024-1] = '\0';
    AddMemoErrMsg( szShowLog );
#endif
    return FALSE;
  }
  //
  CMercenarySet             *pData = NULL;
  string                    strContent;
  int                       i = 0, iId = 0;
  //
  while( in >> strContent )
  {
    iId++;
    pData = new CMercenarySet( iId );
    memcpy( pData->m_szContent, strContent.c_str(), 13 );
		pData->m_szContent[12] = NULL;
    for( i = 0; i < 5; i++ )
    {
       in >> pData->m_wMonsterId[i]
          >> pData->m_dwPrice[i]
          >> pData->m_wPosX[i]
          >> pData->m_wPosY[i];
    }
    m_mapMercenary.insert( map<WORD,CMercenarySet*>::value_type( iId, pData ) );
  }
#ifdef _DEBUG_JAPAN_DECRYPT_
  return TRUE; 
#else
  in.close();
  return TRUE;
#endif //_DEBUG_JAPAN_DECRYPT_
}
//=======================================================================================
//
//
BOOL CMercenaryManager::InitBaseMonster()
{
  FuncName( "CMercenaryManager::InitBaseMonster" );
  //
  map<WORD,CMercenarySet*>::iterator    Iter_Ms;
  CMercenarySet                         *pMercenary = NULL;
  //
  for( Iter_Ms = m_mapMercenary.begin(); Iter_Ms != m_mapMercenary.end(); Iter_Ms++ )
  {
    pMercenary = Iter_Ms->second;
    for( int i = 0; i < 5; i++ )
    {
      if( pMercenary->m_wMonsterId[i] )
      {
        if( NULL == ( pMercenary->m_pBaseMonster[i] = g_pBase->GetBaseMonster( pMercenary->m_wMonsterId[i] ) ) )
        {  
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          char      szShowLog[MAX_MEMO_MSG_LEN];
					_snprintf( szShowLog, MAX_MEMO_MSG_LEN-1, "===>>> The Mercenary(%d) Monster(%d) Is Error, Cannnot Find Base Monster", Iter_Ms->first, pMercenary->m_wMonsterId[i] );
          szShowLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szShowLog );
#endif
          return FALSE;
        }
        if( !(pMercenary->m_pBaseMonster[i]->GetProperty() & NPC_ATTRI_NEVER_REVIVE) )
        {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          char      szShowLog[MAX_MEMO_MSG_LEN];
          _snprintf( szShowLog, MAX_MEMO_MSG_LEN-1, "===>>> The Mercenary(%d) Monster(%d) Is Error, AI(%d) Is Can Revive", Iter_Ms->first, pMercenary->m_wMonsterId[i], pMercenary->m_pBaseMonster[i]->GetProperty() );
          szShowLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szShowLog );
#endif
          return FALSE;
        }
      }
    }
  }
  return TRUE;
}
//=======================================================================================
//
//
void CCityWarTimeManager::ReloadCityWarTime( SMsgData * pTheMsg )
{
  SQueryWarTime                           *pQueryInfo = (SQueryWarTime*)(pTheMsg->Msgs[0].Data);
  CCityWarTimeCtrl                        *pCityWarTime = NULL;
  int                                     iCount = pTheMsg->Msgs[0].Size / sizeof( SQueryWarTime );
  //
  ClearAll();
  for( int iLoop = 0; iLoop < iCount; iLoop++ )
  {
    pCityWarTime = new CCityWarTimeCtrl;
    if( pCityWarTime )
    {
      pCityWarTime->m_wId             = pQueryInfo->wId;
      pCityWarTime->m_wRaceType       = pQueryInfo->wRaceType;
      pCityWarTime->m_wBeginHour      = pQueryInfo->wBeginHour;
      pCityWarTime->m_wBeginMinute    = pQueryInfo->wBeginMinute;
      pCityWarTime->m_wEndHour        = pQueryInfo->wEndHour;
      pCityWarTime->m_wEndMinute      = pQueryInfo->wEndMinute;
      pCityWarTime->m_wBaseCityMapId  = pQueryInfo->wBaseCityMapId;
      pCityWarTime->m_wCityWarMapId   = pQueryInfo->wCityWarMapId;
      pCityWarTime->m_dwGuildId       = pQueryInfo->dwGuildId;
      pCityWarTime->m_wSelTime        = pQueryInfo->wSelTime;
      //
      m_mapCityWarTime.insert( map<WORD,CCityWarTimeCtrl*>::value_type( pCityWarTime->m_wId, pCityWarTime ) );
    }
    pQueryInfo++;
  }
  g_pGs->ReleaseMsg( pTheMsg );
}
//=======================================================================================
//
//
/*BOOL CCityWarTimeManager::LoadCityWarTime()
{
  FuncName("CCityWarTimeManager::LoadCityWarTime");
  //
  FILE              *pFile = NULL;
  char              szFileName[256];
  char              szShowLog[MAX_MEMO_MSG_LEN];
  DWORD             dwData[10];
  int               iCount = 0;
  CCityWarTimeCtrl  *pCityWarTime = NULL;
  //
  sprintf( szFileName, "%s\\CityWarTime.txt", g_pBase->GetMapFilePath() );
  pFile = fopen( szFileName, "r" );
  //
  if( pFile == NULL )
  {
    sprintf( szShowLog, "===>>> Cannnot Load CityWarTime File, Please Check It[%s]...", szFileName );
    AddMemoErrMsg( szShowLog );
    return FALSE;
  }
  //
  for( iCount = 0; iCount < MAX_CITY_WAR_MAP_COUNT; iCount++ )
  {
    if( 10 == fscanf( pFile, "%d %d %d %d %d"
                             "%d %d %d %d %d",
                             &dwData[0], &dwData[1], &dwData[2], &dwData[3], &dwData[4],
                             &dwData[5], &dwData[6], &dwData[7], &dwData[8], &dwData[9] ) )
    {
      if( dwData[0] != ( iCount + 1 ) )
      {
        sprintf( szShowLog, "===>>> The CityWarTime File Id Is Error, Please Check It[%s]...", szFileName );
        AddMemoErrMsg( szShowLog );
        fclose( pFile );
        return FALSE;
      }
      pCityWarTime = new CCityWarTimeCtrl;
      pCityWarTime->m_wId             = dwData[0];
      pCityWarTime->m_wRaceType       = dwData[1];
      pCityWarTime->m_wBeginHour      = dwData[2];
      pCityWarTime->m_wBeginMinute    = dwData[3];
      pCityWarTime->m_wEndHour        = dwData[4];
      pCityWarTime->m_wEndMinute      = dwData[5];
      pCityWarTime->m_wBaseCityMapId  = dwData[6];
      pCityWarTime->m_wCityWarMapId   = dwData[7];
      pCityWarTime->m_dwGuildId       = dwData[8];
      pCityWarTime->m_wSelTime        = dwData[9];
      //
      m_mapCityWarTime.insert( map<WORD,CCityWarTimeCtrl*>::value_type( pCityWarTime->m_wId, pCityWarTime ) );
    }
    else
    {
      sprintf( szShowLog, "===>>> Scan CityWarTime File, Please Check It[%s]...", szFileName );
      AddMemoErrMsg( szShowLog );
      fclose( pFile );
      return FALSE;
    }
  }
  fclose( pFile );
  return TRUE;
}*/
//=======================================================================================
//
//
void CCityWarTimeManager::ClearSelTimeWhenWinCityWar( CGuild * pGuild, CGuild * pWinGuild, const WORD & wMapId )
{
  CCityWarTimeCtrl    *pCityWar_1 = NULL;
  CCityWarTimeCtrl    *pCityWar_2 = NULL;
  //
  if( pGuild )        pCityWar_1 = GetCityWarTimeByGuildId( pGuild->GetGuildId() );
                      pCityWar_2 = GetCityWarTimeByMapId( wMapId );
  //
  if( pCityWar_1 )
  {
    pCityWar_1->m_wSelTime  = 0;
    pCityWar_1->m_dwGuildId = pWinGuild->GetGuildId();
  }
  if( pCityWar_2 )
  {
    pCityWar_2->m_wSelTime  = 0;
    pCityWar_2->m_dwGuildId = pWinGuild->GetGuildId();
  }
}
//=======================================================================================
//
//
void CCityWarTimeManager::SaveCityWarTime()
{
  char       szFileName[256];
  //
  _snprintf( szFileName, 256-1, "%s\\CityWarTime.txt", g_pBase->GetMapFilePath() );
  szFileName[256-1] = '\0';
  //
  ofstream                               out( szFileName );
  CCityWarTimeCtrl                       *pCityWar = NULL;
  map<WORD,CCityWarTimeCtrl*>::iterator  Iter_Ct;
  //
  for( Iter_Ct = m_mapCityWarTime.begin(); Iter_Ct != m_mapCityWarTime.end(); Iter_Ct++ )
  {
	  if( pCityWar = (*Iter_Ct).second )
	  {
		  out << pCityWar->m_wId            << ' '
				  << pCityWar->m_wRaceType      << ' '
				  << pCityWar->m_wBeginHour     << ' '
				  << pCityWar->m_wBeginMinute   << ' '
				  << pCityWar->m_wEndHour       << ' '
				  << pCityWar->m_wEndMinute     << ' '
				  << pCityWar->m_wBaseCityMapId << ' '
				  << pCityWar->m_wCityWarMapId  << ' '
				  << pCityWar->m_dwGuildId      << ' '
				  << pCityWar->m_wSelTime       << '\n';
	  }
  }
  out.close();
}

#ifdef _DEBUG_MICHAEL_OPEN_FERRY
//=====================================================================================
// Function: Load文件并填充所有FerryData
// Note:     文件为GameData/event/ferry.txt
//
BOOL CFerryManager::Load(char* lpszFileName)
{
#ifndef _NEW_FERRY_SYSTEM_
#ifdef _DEBUG_JAPAN_DECRYPT_
  CInStream in(lpszFileName);
  if( in.fail() || in.GetFileSize() == 0 ) return FALSE;
#else
	ifstream in(lpszFileName);
	if( in.fail() ) return FALSE;
#endif
	Clear();
	CFerryData* pData=NULL;
	WORD wMapId = 0, wFirstTimeHour = 0, wFirstTimeMinute = 0, wTakeBoatTime = 0, wWaitTime = 0, wByWaterTime = 0;

	while( in>>wMapId )
	{
	 pData = new CFerryData;
	 pData->m_wFerryMapId = wMapId;
	 in>>pData->m_wDestinationMapId
		 >>pData->m_wDestinationDesX
		 >>pData->m_wDestinationDesY
		 >>pData->m_wBoatMapId
		 >>wFirstTimeHour
		 >>wFirstTimeMinute
		 >>wTakeBoatTime
 		 >>wWaitTime
		 >>wByWaterTime
		 >>pData->m_wMaxNumber
	   >>pData->m_wMonsterAttackOdss
		 >>pData->m_wMonsterNumber
		 >>pData->m_pMonsterID[0]
		 >>pData->m_pMonsterID[1]
		 >>pData->m_pMonsterID[2]
		 >>pData->m_pMonsterID[3]
		 >>pData->m_pMonsterID[4]
		 >>pData->m_pMonsterID[5]
		 >>pData->m_pMonsterID[6]
		 >>pData->m_pMonsterID[7]
		 >>pData->m_pMonsterID[8]
		 >>pData->m_pMonsterID[9];
		//
		pData->m_dwFirstBoatTime = MAKELONG( wFirstTimeMinute, wFirstTimeHour );
		// 企划填表填的是分钟，这里要转换为TickCount
		pData->m_dwTakeBoatTime  = wTakeBoatTime * 60000;
		pData->m_dwWaiteTime     = wWaitTime     * 60000;
		pData->m_dwByWaterTime   = wByWaterTime  * 60000;

		m_mapFerryData.insert( map<WORD,CFerryData*>::value_type(pData->m_wFerryMapId,pData ) );
	}
#else
	ifstream in(lpszFileName);
  if( in.fail() )   { return FALSE; }
	Clear();
  //
	CFerryData *pData = NULL;
	WORD wMapId = 0, wFirstTimeHour = 0, wFirstTimeMinute = 0, wTakeBoatTime = 0, wWaitTime = 0, wByWaterTime = 0;
  WORD wMonsterId[10], wMonstOdds[10];
	while( in>>wMapId )
  {
    pData = new CFerryData;
    pData->m_wFerryMapId = wMapId;
    in>>pData->m_wDestinationMapId
      >>pData->m_wDestinationDesX
      >>pData->m_wDestinationDesY
      >>pData->m_wBoatMapId
      >>wFirstTimeHour>>wFirstTimeMinute
      >>wTakeBoatTime>>wWaitTime>>wByWaterTime
      >>pData->m_wMaxNumber
      >>pData->m_wMonsterAttackOdss
      >>pData->m_wMonsterNumber
      >>wMonsterId[0]>>wMonstOdds[0]          // Monster ID && Create Monster Odds 
      >>wMonsterId[1]>>wMonstOdds[1]          // 0: Means Must Be Created (100%); 
      >>wMonsterId[2]>>wMonstOdds[2]          // >0:Means Create Odds
      >>wMonsterId[3]>>wMonstOdds[3]
      >>wMonsterId[4]>>wMonstOdds[4]
      >>wMonsterId[5]>>wMonstOdds[5]
      >>wMonsterId[6]>>wMonstOdds[6]
      >>wMonsterId[7]>>wMonstOdds[7]
      >>wMonsterId[8]>>wMonstOdds[8]
      >>wMonsterId[9]>>wMonstOdds[9];
    //
    for(int i=0; i<10; i++)
    {
      pData->m_pMonsterID[i] = MAKELONG(wMonsterId[i], wMonstOdds[i]);
    }
    pData->m_dwFirstBoatTime = MAKELONG( wFirstTimeMinute, wFirstTimeHour );
    // 企划填表填的是分钟，这里要转换为TickCount
    pData->m_dwTakeBoatTime  = wTakeBoatTime * 60000;
    pData->m_dwWaiteTime     = wWaitTime     * 60000;
    pData->m_dwByWaterTime   = wByWaterTime  * 60000;
    
    m_mapFerryData.insert( map<WORD,CFerryData*>::value_type(pData->m_wFerryMapId,pData ) );
  }
#endif
	return TRUE;
}
//=====================================================================================
// Function: 得到船上的总人数
// Note:     因为有偷渡的设定，故有可能大于m_wMaxNumber
//
WORD  CFerryData::GetBoatPlayerCount()
{ 
	return GetBoatMap()->GetPlayerCount();
}
//=====================================================================================
// Function: 发送消息给相关地图的所有人，包括码头地图、船地图、目的地地图
// Note:     不包括室内地图
//
void  CFerryData::SendTheMsgToAll( SMsgData* pTheMsg )
{
	GetFerryMap()->SendTheMsgToAll(pTheMsg);   
	GetBoatMap()->SendTheMsgToAll(pTheMsg);    	
	GetDesMap()->SendTheMsgToAll(pTheMsg);  
}
#endif


bool CSrvBaseData::CheckGameData()
{
	FuncName("CSrvBaseData::CheckGameData");

	int iErrCount=0;
	// Check Map
	SGameMapType* pType = NULL;
	map<int,SGameMapType*>::iterator iter_1;
  for( iter_1 = m_mapIdMapType.begin(); iter_1 != m_mapIdMapType.end(); iter_1++  )
	{
		pType = iter_1->second;
    if( m_mapIdMapType.find( pType->wReviveMap ) == m_mapIdMapType.end() )
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, " Scan Error The Map File(%d)  ReviveMap(%d) ", pType->iMapId, pType->wReviveMap );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			iErrCount++;
		}
    if( 0 != pType->wDescribe && m_mapIdMapType.find( pType->wDescribe )  == m_mapIdMapType.end() )
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf( szInitLog, MAX_MEMO_MSG_LEN-1, " >> Scan Error The Map File(%d)  Describe(%d)<< ", pType->iMapId, pType->wDescribe  );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg( szInitLog );
#endif
			iErrCount++;
		}
	}

	// Check BaseSkill
	CSrvBaseSkill* pBSkill = NULL;
	map<DWORD, CSrvBaseSkill*>::iterator iter_2;
  for( iter_2 = m_mapBaseSkill.begin(); iter_2 != m_mapBaseSkill.end(); iter_2++  )
	{
		pBSkill = iter_2->second;
		if( 0 != pBSkill->m_wAdvanceSkillId && m_mapBaseSkill.find(pBSkill->m_wAdvanceSkillId) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan Base Skill=%d , AdvanceSkillId=%d is NO EXIST *****", pBSkill->m_wId, pBSkill->m_wAdvanceSkillId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBSkill->m_wAlarmSkillId && m_mapBaseSkill.find(pBSkill->m_wAlarmSkillId) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog,MAX_MEMO_MSG_LEN-1, "***** Scan Base Skill=%d , AlarmSkillId=%d is NO EXIST *****", pBSkill->m_wId, pBSkill->m_wAlarmSkillId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBSkill->m_wTriggerSkillId && m_mapBaseSkill.find(pBSkill->m_wTriggerSkillId) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan Base Skill=%d , TriggerSkillId=%d is NO EXIST *****", pBSkill->m_wId, pBSkill->m_wTriggerSkillId );
      szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBSkill->m_wSkillLearnId && m_mapBaseSkill.find(pBSkill->m_wSkillLearnId) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base Skill=%d , SkillLearnId=%d is NO EXIST *****", pBSkill->m_wId, pBSkill->m_wSkillLearnId );
		  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBSkill->m_wFuncChangeId && m_mapBaseSkill.find(pBSkill->m_wFuncChangeId) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base Skill=%d , FuncChangeId=%d is NO EXIST *****", pBSkill->m_wId, pBSkill->m_wFuncChangeId );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBSkill->m_wChangeItemId && m_mapBaseItem.find(pBSkill->m_wChangeItemId) == m_mapBaseItem.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan Base Skill=%d , ChangeItemId=%d is NO EXIST *****", pBSkill->m_wId, pBSkill->m_wChangeItemId );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
	}

  // Check Item
	CSrvBaseItem * pBaseItem = NULL;
	map<DWORD, CSrvBaseItem*>::iterator iter_3;
  for( iter_3 = m_mapBaseItem.begin(); iter_3 != m_mapBaseItem.end(); iter_3++ )
	{
    pBaseItem = iter_3->second;
		if( 0 != pBaseItem->m_wFuncEffect && m_mapBaseSkill.find(pBaseItem->m_wFuncEffect) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base BaseItem=%d , FuncEffect=%d is NO EXIST *****", pBaseItem->m_wId, pBaseItem->m_wFuncEffect );
		  szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBaseItem->m_wFuncEquip && m_mapBaseSkill.find(pBaseItem->m_wFuncEquip) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base BaseItem=%d , FuncEquip=%d is NO EXIST *****", pBaseItem->m_wId, pBaseItem->m_wFuncEquip );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBaseItem->m_wFuncTouch && m_mapBaseSkill.find(pBaseItem->m_wFuncTouch) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base BaseItem=%d , FuncTouch=%d is NO EXIST *****", pBaseItem->m_wId, pBaseItem->m_wFuncTouch );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pBaseItem->m_wFuncTakeOff && m_mapBaseSkill.find(pBaseItem->m_wFuncTakeOff) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan Base BaseItem=%d , FuncTakeOff=%d is NO EXIST *****", pBaseItem->m_wId, pBaseItem->m_wFuncTakeOff );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}

		if( 0 != pBaseItem->m_wFuncDbc && m_mapBaseSkill.find(pBaseItem->m_wFuncDbc) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan Base BaseItem=%d , FuncDbc=%d is NO EXIST *****", pBaseItem->m_wId, pBaseItem->m_wFuncDbc );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}

	}
 
	// Check Drop Item 
	CSrvDropItem      * pDropItem = NULL;  
	list<CSrvDropItem*>::iterator iter_4;
	for(iter_4 = g_pDropItem->m_listDropItem.begin(); iter_4 != g_pDropItem->m_listDropItem.end(); iter_4++)
	{
		pDropItem = *iter_4;
		if( pDropItem->m_wHoleNumber > MAX_ITEM_SLOT )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan DropItem=%d , HoleNumber=%d is	ERROR*****", pDropItem->m_wId, pDropItem->m_wHoleNumber );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wItemId && m_mapBaseItem.find(pDropItem->m_wItemId) == m_mapBaseItem.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan DropItem=%d , ItemId=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wItemId );
	    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncDbc && m_mapBaseSkill.find(pDropItem->m_wFuncDbc) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan DropItem=%d , FuncDbc=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncDbc );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncAlarm && m_mapBaseSkill.find(pDropItem->m_wFuncAlarm) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan DropItem=%d , FuncAlarm=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncAlarm );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncTrigger && m_mapBaseSkill.find(pDropItem->m_wFuncTrigger) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan DropItem=%d , FuncTrigger=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncTrigger );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncEffect && m_mapBaseSkill.find(pDropItem->m_wFuncEffect) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan DropItem=%d , FuncEffect=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncEffect );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncEquip && m_mapBaseSkill.find(pDropItem->m_wFuncEquip) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan DropItem=%d , FuncEquip=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncEquip );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncTouch && m_mapBaseSkill.find(pDropItem->m_wFuncTouch) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1, "***** Scan DropItem=%d , FuncTouch=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncTouch );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
		if( 0 != pDropItem->m_wFuncTakeOff && m_mapBaseSkill.find(pDropItem->m_wFuncTakeOff) == m_mapBaseSkill.end() )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else 
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** Scan DropItem=%d , FuncTakeOff=%d is NO EXIST *****", pDropItem->m_wId, pDropItem->m_wFuncTakeOff );
	    szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}

	}
	

	// Check Monster
	int iLoop = 0, iSum = 0;
	CSrvBaseMonster * pBaseMonster = NULL;
	map<DWORD,CSrvBaseMonster*>::iterator iter_5;
  for( iter_5 = m_mapBaseMonster.begin(); iter_5 != m_mapBaseMonster.end(); iter_5++ )
	{
		iSum = 0;
		pBaseMonster = iter_5->second;
    for( iLoop = 0; iLoop < 5; iLoop++ )
		{
			iSum += pBaseMonster->m_wDropEnd[iLoop] - pBaseMonster->m_wDropBegin[iLoop];
		}
		if( iSum > 40 )
		{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
			_snprintf(szInitLog, MAX_MEMO_MSG_LEN-1,"***** BaseMonster=%d , Drop=%d > 40 *****", pBaseMonster->m_wBaseId, iSum );
			szInitLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szInitLog);
#endif
			iErrCount++;
		}
	}
	
  // Check End
	if( iErrCount > 0 ) return false;
	//
	return true;
}

#ifdef _NEW_TRADITIONS_WEDDING_
/////////////////////////
// class GSWeddingMgr
/////////////////////////
GSWeddingMgr::GSWeddingMgr()
{
  m_NeedQueryFromMCC = TRUE;
  m_SpouseWarpLimt.clear();
}

GSWeddingMgr::~GSWeddingMgr()
{
}

BOOL GSWeddingMgr::LoadSpouseWarpLimitMap( char* szSpouseWarpLimitMapFile)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  CInStream    SpouseWarp( szSpouseWarpLimitMapFile );
  if( !SpouseWarp.fail() && SpouseWarp.GetFileSize() != 0 )
  {
    int iCount = 0;
    int iMap;
    WarpLimit limit;
    if ( SpouseWarp >> iCount ) 
    {
      for ( int i = 0; i<iCount; i++ ) 
      {
        if( SpouseWarp >> iMap
          >> limit.OutLimit
          >> limit.IntoLimt )
        {
          m_SpouseWarpLimt.insert(map<int,WarpLimit>::value_type(iMap,limit));
        }
      }
    }
  }
  else
  {
   // MessageBox( GetActiveWindow(), "Cannot Open The LoadSpouseWarpLimitMap.txt", "Warning...", MB_OK );
    return false;
  }
  return TRUE;
#else
//
  m_SpouseWarpLimt.clear();

  FILE *fp = fopen( szSpouseWarpLimitMapFile, "r" );
  if( fp == NULL ) return FALSE;

  int iCount = 0;
 	fscanf( fp,"%d ",&iCount );

  int iMap;
  int OutLimit;
  float IntoLimit;
  WarpLimit limit;
  int ret = 0;
  for(int i=0; i<iCount; i++)
  {
    if(3 == fscanf( fp, "%u %u %f", 
                    &iMap,
                    &OutLimit,
                    &IntoLimit))
    {
      limit.IntoLimt = IntoLimit;
      limit.OutLimit = OutLimit;
      m_SpouseWarpLimt.insert(map<int,WarpLimit>::value_type(iMap,limit));
    }
  }
  fclose(fp);
  fp=NULL;
  return TRUE;
#endif// _DEBUG_JAPAN_DECRYPT_
}

BOOL GSWeddingMgr::SpouseWarpLimitInto(int iMapId)
{
  map<int,WarpLimit>::iterator itr;
  if ((itr=m_SpouseWarpLimt.find(iMapId)) != m_SpouseWarpLimt.end())
  {
    WarpLimit* plimit = &(itr->second);
    return (BOOL)(plimit->IntoLimt);
  }
  return FALSE;
}
BOOL GSWeddingMgr::SpouseWarpLimitOut(int iMapId)
{
  map<int,WarpLimit>::iterator itr;
  if ((itr=m_SpouseWarpLimt.find(iMapId)) != m_SpouseWarpLimt.end())
  {
    WarpLimit* plimit = &(itr->second);
    return (BOOL)(plimit->OutLimit);
  }
  return FALSE;
}


/////////////////////////
// class CAwaneMapMgr
/////////////////////////
BOOL CAwaneMapMgr::IsAwaneMap(int iMapId)
{
  list<CAwaneInfo>::iterator itr = m_AwaneMaplist.begin();
  for( ; itr != m_AwaneMaplist.end(); itr++ )
  {
    if (iMapId == itr->iMapId)
    {
      return TRUE;
    }
  }
  return FALSE;
}
BOOL CAwaneMapMgr::LoadFile(char* szFileName)
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  m_AwaneMaplist.clear();
  CInStream      AwaneMap( szFileName );
  if ( !AwaneMap.fail() && AwaneMap.GetFileSize() != 0) 
  {
    int iCount = 0;
    AwaneMap >> iCount;
    if ( iCount <= 0 ) 
    {
      MessageBox( GetActiveWindow(), "Load The iCount in AwaneMap.txt Error", "Warning", MB_OK );
      return false;
    }
    CAwaneInfo  AwaneInfo;
    for(int i=0; i<iCount; i++)
    {
      if( AwaneMap>>AwaneInfo.iMapId>>AwaneInfo.iAwaneInterval>>AwaneInfo.fAwaneRadio )
      {
        if (!IsAwaneMap(AwaneInfo.iMapId)) m_AwaneMaplist.push_back(AwaneInfo);
      }
    }
  }
  else
  {
    MessageBox( GetActiveWindow(), "Load The AwaneMap.txt Error", "Warning...", MB_OK );
    return false;
  }
  return TRUE;
#else
//
  m_AwaneMaplist.clear();

  FILE *fp = fopen( szFileName, "r" );
  if( fp == NULL ) return FALSE;

  int iCount = 0;
 	fscanf( fp,"%d ",&iCount );

  CAwaneInfo  AwaneInfo;
  int ret = 0;
  for(int i=0; i<iCount; i++)
  {
    if(3 == fscanf( fp, "%u %u %f", 
                    &AwaneInfo.iMapId,
                    &AwaneInfo.iAwaneInterval,
                    &AwaneInfo.fAwaneRadio ))
    {
      if (!IsAwaneMap(AwaneInfo.iMapId)) m_AwaneMaplist.push_back(AwaneInfo);
    }
  }
  fclose(fp);
  fp=NULL;
  return TRUE;
#endif //_DEBUG_JAPAN_DECRYPT_
}
BOOL CAwaneMapMgr::GetAwaneInfo(const int iMapId,int& iAwaneInterval,float& fAwaneRadio)
{
  list<CAwaneInfo>::iterator itr = m_AwaneMaplist.begin();
  for( ; itr != m_AwaneMaplist.end(); itr++ )
  {
    if (iMapId == itr->iMapId)
    {
      iAwaneInterval  = itr->iAwaneInterval;
      fAwaneRadio     = itr->fAwaneRadio;
      return TRUE;
    }
  }
  iAwaneInterval  = 0;
  fAwaneRadio     = 0;
  return FALSE;
}
#endif
//=======================================================================================
//Add By JackRen For Decrypt In Japan 3.6
//
//#ifdef _DEBUG_JAPAN_DECRYPT_
//注：函数缺陷，本类"对象.fail()"产生的并不是我们想要的结果.
const  char*   CInStream::m_szKey = "11111111";
char* CInStream::GetContent( char *szFileName )
{
   m_dwFileSize = 0;
   m_szFileBuf  = NULL;
   FILE *fp = fopen( szFileName, "rb" );
   if( fp==NULL ) 
   {
//#ifdef _DEBUG
//     MessageBox(GetActiveWindow(), "Open The File Failed  fp==NULL", "Warning...", MB_OK);
//#endif
     return NULL;
   }
   fseek( fp, 0, SEEK_END );
   DWORD dwFileSize = ftell( fp ), dwBufSize;
   m_dwFileSize = dwFileSize;
   fseek( fp, 0, SEEK_SET );
   dwBufSize=(dwFileSize+7)&~7;
   char*  szFileBuf=NULL;
   if( dwBufSize>0 )
   {
     szFileBuf = new char[dwBufSize+9];
     *(szFileBuf+dwBufSize)=0;
   }
   if( szFileBuf==NULL )
   {
     fclose( fp );
#ifdef _DEBUG
     MessageBox( GetActiveWindow(), "*******szFileBuf==NULL*******", "Warning...", MB_OK );
#endif
     return NULL;
   }
   fread( szFileBuf, dwFileSize, 1, fp );
   fclose( fp );
   for( int i=dwFileSize; i<dwBufSize; i++ )
   {
     szFileBuf[i] = ' ';
   }
   Decrypt( szFileBuf, dwBufSize );
   return szFileBuf;
} 

void CInStream::Decrypt( char * szDecryptchar,DWORD dwLen )
{
  BYTE szSecretKey[8] ;
  memcpy(szSecretKey,m_szKey,8);
  
  _asm
  {
    mov ecx,dwLen
    shr ecx,3
    jecxz End
    mov esi,szDecryptchar
    movq mm2,szSecretKey
Loop1:
    movq mm0,[esi]
    pxor mm0,mm2
    movq mm1,mm0
    psllq mm0,64-SHIFT_BITS
    psrlq mm1,SHIFT_BITS
    por mm0,mm1
    movq [esi],mm0
    add esi,8
    loop Loop1
End:
    emms
  }
}

//#endif //_DEBUG_JAPAN_DECRYPT_


#ifdef _MONSTER_ATTACK_CITY_

CMonsterAttackCity::CMonsterAttackCity() : m_bInitOk(false), m_dwLastLoopTime(0) 
{
  Destroy();
  ResetAttackMonsterInfo();
}
CMonsterAttackCity::~CMonsterAttackCity()
{
  Destroy();
  ResetAttackMonsterInfo();
}
bool CMonsterAttackCity::Init(char* szFileName)
{
  FuncName("CMonsterAttackCity::Init");
  bool bRet = false;
  if(NULL != szFileName)
  {
    strncpy(m_szFileName,szFileName,256);
    FILE* pFile = fopen(m_szFileName,"r");
    if(NULL != pFile)
    {
      char szBuff[1024];
      int iRet = 0;
      SYSTEMTIME  sDate;
      sDate.wDayOfWeek = 1;

      fscanf(pFile, "%s", m_szPrepareMsg);
      fscanf(pFile, "%s", m_szEndMsg);

      iRet = fscanf(pFile,"%s", szBuff);
      if(1 == iRet && !(strcmp(szBuff,"[time]")))
      {
        while(1)
        {
          fscanf(pFile,"%d %d %d",&sDate.wDayOfWeek, &sDate.wHour, &sDate.wMinute);
          if(0 == sDate.wDayOfWeek && 0 == sDate.wHour && 0 == sDate.wMinute)
          {
            break;
          }
          ACTIVEATTACKDATE * pDate = new ACTIVEATTACKDATE;
          if( NULL != pDate)
          {
            pDate->m_Date.wDayOfWeek = sDate.wDayOfWeek;
            pDate->m_Date.wHour = sDate.wHour;
            pDate->m_Date.wMinute = sDate.wMinute;
            pDate->m_Date.wSecond = 0;
            this->PushBackActiveAttackDate(pDate);
          }
        }
      }


      iRet = fscanf(pFile,"%s", szBuff);
      if(1 == iRet && !(strcmp(szBuff,"[map]")))
      {
        DWORD  dwMapId(0);
        int    iX(0);
        int    iY(0);
        while(1)
        {
          fscanf(pFile,"%d %d %d",&dwMapId, &iX, &iY);
          if(0 == dwMapId)
          {
            break;
          }
          CGameMap * pMap = g_pGs->GetGameMap( dwMapId );
          if(NULL == pMap)
          {
            continue;
          }
          MONSTERBORNPOINT * pPoint = new MONSTERBORNPOINT;
          if( NULL != pPoint)
          {
            pPoint->m_dwFloorId = dwMapId;
            pPoint->m_iX = iX;
            pPoint->m_iY = iY;
            this->PushBackMonsterBornPoint(pPoint);
          }
        }
      }

      iRet = fscanf(pFile,"%s", szBuff);
      if(1 == iRet && !(strcmp(szBuff,"[boss]")))
      {
        int iBossId(0);
        while(1)
        {
          fscanf(pFile, "%d", &iBossId);
          if(0 == iBossId)
          {
            break;
          }
          MONSTERSEED* pSeed = new MONSTERSEED;
          pSeed->m_iMonsterId = iBossId;
          pSeed->m_wCount = 1;
          this->PushBackBossSeed(pSeed);
        }
      }
      iRet = fscanf(pFile,"%s", szBuff);
      if(1 == iRet && !(strcmp(szBuff,"[monster]")))
      {
        int iMonster(0);
        while(1)
        {
          fscanf(pFile, "%d", &iMonster);
          if(0 == iMonster)
          {
            bRet = true;
            break;
          }
          MONSTERSEED* pSeed = new MONSTERSEED;
          pSeed->m_iMonsterId = iMonster;
          pSeed->m_wCount = 3;
          this->PushBackMonsterSeed(pSeed);
        }
      }
    }
  }
  if(bRet)
  {
    m_bInitOk = true;
  }
  return bRet;
}
void CMonsterAttackCity::Loop()
{
  FuncName("CMonsterAttackCity::Loop");
  if(m_bInitOk)
  {
    switch(m_dwState)
    {
    case ATTACK_WAITE:
      {
        if(m_dwLastLoopTime+10000 < TimeGetTime())
        {
          if(IsTimeBeginPrepare())
          {
            SetLoopState(ATTACK_PREPARE);
            ++m_dwSendMsgTime;
            SendMsgToAll(m_szPrepareMsg);
          }
          m_dwLastLoopTime = TimeGetTime();
        }
      }
      break;
    case ATTACK_PREPARE:
      {
        if(m_dwLastLoopTime + 1000 < TimeGetTime()) // 2分钟发一次公告
        {
          ++m_dwSendMsgTime;
          SendMsgToAll(m_szPrepareMsg);
          m_dwLastLoopTime = TimeGetTime();
          if( 0 < m_dwSendMsgTime) // 发送1 次
          {
            m_dwSendMsgTime = 0;
            m_dwAttackBeginTime = TimeGetTime();
            ResetBossPointer();
            SetLoopState(ATTACK_INITMONSTERINFO);
          }
        }
      }
      break;
    case ATTACK_INITMONSTERINFO:
      {
        bool bRet = IsBossListEnd();
        if(bRet)
        {
          SetLoopState(ATTACK_WAIT_REMOVE);
          m_dwLastLoopTime  = TimeGetTime();
        }
        else
        {
          int iRet = InitAttackMonsterInfo();
          if(3 != iRet)
          {
            SetLoopState(ATTACK_ERROR_END);
          }
          else
          {
            SetLoopState(ATTACK_CreateMonster);
          }
        }
      }
      break;
    case ATTACK_CreateMonster:
      {
        if(m_dwLastLoopTime + 2000 < TimeGetTime())
        {
          ++m_wCreateMonsterTime;
          this->CreateMonster(m_iMonsterId, 2, m_dwFloorId); // 放小怪

          char szLog[MAX_MEMO_MSG_LEN];
          _snprintf(szLog, MAX_MEMO_MSG_LEN-1, "Create 2 Monsters %d,floorId = %d ",
            m_iMonsterId, m_dwFloorId);
          szLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoMsg( szLog );

          if(MAX_CREATEMONSTERTIME_PERGROUP/2 == m_wCreateMonsterTime)
          {
            this->CreateMonster(m_iBossId, 1, m_dwFloorId); // 放boss

            _snprintf(szLog, MAX_MEMO_MSG_LEN-1, "Create 1 Boss %d,floorId = %d ",
              m_iBossId, m_dwFloorId);
            szLog[MAX_MEMO_MSG_LEN-1] = '\0';
            AddMemoMsg( szLog );
          }
          if(MAX_CREATEMONSTERTIME_PERGROUP < m_wCreateMonsterTime)
          {
            SetLoopState(ATTACK_WAITING_NEXT);
          }
          m_dwLastLoopTime = TimeGetTime();
        }
      }
      break;
    case ATTACK_WAITING_NEXT:
      {
        if(m_dwLastLoopTime + 300000 < TimeGetTime())
        {
          ResetAttackMonsterInfo();
          SetLoopState(ATTACK_INITMONSTERINFO);
          m_dwLastLoopTime  = TimeGetTime();
        }
      }
      break;
    case ATTACK_WAIT_REMOVE:
      {
        if(m_dwLastLoopTime + 1800000 < TimeGetTime())
        {
          SetLoopState(ATTACK_REMOVE_MONSTER);
          m_dwLastLoopTime  = TimeGetTime();
        }
      }
      break;
    case ATTACK_REMOVE_MONSTER:
      {
        RemoveMonster();

        ++m_dwSendMsgTime;
        SendMsgToAll(m_szEndMsg);


        char szLog[MAX_MEMO_MSG_LEN];
        _snprintf(szLog, MAX_MEMO_MSG_LEN-1, "Remove all monster");
        szLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoMsg( szLog )

        ResetAttackMonsterInfo();
        SetLoopState(ATTACK_END);
      }
      break;
    case ATTACK_END:
      {
        if(m_dwLastLoopTime + 120000 < TimeGetTime()) // 2分钟发一次公告
        {
          ++m_dwSendMsgTime;
          SendMsgToAll(m_szEndMsg);
          m_dwLastLoopTime = TimeGetTime();
          if(2 < m_dwSendMsgTime) // 发送3 次
          {
            m_dwSendMsgTime = 0;
            SetLoopState(ATTACK_WAITE);
          }
        }
      }
      break;
    case ATTACK_ERROR_END:
      {
        RemoveMonster();
        SetLoopState(ATTACK_WAITE);
      }
      break;
    default:
      break;
    }
  }
}
//---------------------------------------------------------------
void CMonsterAttackCity::PushBackActiveAttackDate(ACTIVEATTACKDATE* pDate)
{
  if(NULL != pDate)
  {
    m_ListAttackDate.push_back(pDate);
  }
}
/////
void CMonsterAttackCity::PushBackMonsterSeed(MONSTERSEED* pSeed)
{
  if(NULL != pSeed)
  {
    m_ListMonsterSeed.push_back(pSeed);
  }
}
/////
void CMonsterAttackCity::PushBackBossSeed(MONSTERSEED* pSeed)
{
  if(NULL != pSeed)
  {
    m_ListBossSeed.push_back(pSeed);
  }
}
/////
void CMonsterAttackCity::PushBackMonsterBornPoint(MONSTERBORNPOINT* pPoint)
{
  if(NULL != pPoint)
  {
    m_ListBornPoint.push_back(pPoint);
  }
}
//---------------------------------------------------------------
void CMonsterAttackCity::Destroy()
{
  *m_szFileName = 0;
  m_dwState = ATTACK_WAITE;
  m_dwSendMsgTime = 0;
  list<ACTIVEATTACKDATE*>::iterator   Iter_Date;
  list<MONSTERSEED*>::iterator        Iter_Seed;
  list<MONSTERBORNPOINT*>::iterator   Iter_BornPoint;

  ACTIVEATTACKDATE* pDate = NULL;
  MONSTERSEED* pSeed = NULL;
  MONSTERBORNPOINT* pBornPoint = NULL;
  for(Iter_Date = m_ListAttackDate.begin(); Iter_Date != m_ListAttackDate.end(); )
  {
    pDate = *Iter_Date;
    SAFE_DELETE(pDate);
    Iter_Date = m_ListAttackDate.erase(Iter_Date);
  }
  m_ListAttackDate.clear();
  ////////
  for(Iter_Seed = m_ListMonsterSeed.begin(); Iter_Seed != m_ListMonsterSeed.end(); )
  {
    pSeed = *Iter_Seed;
    SAFE_DELETE(pSeed);
    Iter_Seed = m_ListMonsterSeed.erase(Iter_Seed);
  }
  m_ListMonsterSeed.clear();
  /////////
  for(Iter_Seed = m_ListBossSeed.begin(); Iter_Seed != m_ListBossSeed.end(); )
  {
    pSeed = *Iter_Seed;
    SAFE_DELETE(pSeed);
    Iter_Seed = m_ListBossSeed.erase(Iter_Seed);
  }
  m_ListBossSeed.clear();
  /////////
  for(Iter_BornPoint= m_ListBornPoint.begin(); Iter_BornPoint != m_ListBornPoint.end(); )
  {
    pBornPoint = *Iter_BornPoint;
    SAFE_DELETE(pSeed);
    Iter_BornPoint = m_ListBornPoint.erase(Iter_BornPoint);
  }
  m_ListBornPoint.clear();
}
//---------------------------------------------------------------
MONSTERSEED* CMonsterAttackCity::GetRandBoss()
{
  const WORD wSize = m_ListBossSeed.size();
  MONSTERSEED* pSeed = NULL;
  if(wSize)
  {
    srand(TimeGetTime());
    const WORD wRand = rand()%(wSize);
    list<MONSTERSEED*>::iterator  Iter_Seed;
    Iter_Seed = m_ListBossSeed.begin();
    WORD wCounter = 0;
    while(wRand != wCounter)
    {
      ++Iter_Seed;
      ++wCounter;
      if(m_ListBossSeed.end() == Iter_Seed)
      {
        Iter_Seed = m_ListBossSeed.begin();
      }
    }
    pSeed = *Iter_Seed;
  }
  return pSeed;
}
//---------------------------------------------------------------
MONSTERSEED* CMonsterAttackCity::GetRandMonster()
{
  const WORD wSize = m_ListMonsterSeed.size();
  MONSTERSEED* pSeed = NULL;
  if(wSize)
  {
    srand(TimeGetTime());
    const WORD wRand = rand()%(wSize);
    list<MONSTERSEED*>::iterator  Iter_Seed;
    Iter_Seed = m_ListMonsterSeed.begin();
    WORD wCounter = 0;
    while(wRand != wCounter)
    {
      ++Iter_Seed;
      ++wCounter;
      if(m_ListMonsterSeed.end() == Iter_Seed)
      {
        Iter_Seed = m_ListMonsterSeed.begin();
      }
    }
    pSeed = *Iter_Seed;
  }
  return pSeed;
}
//---------------------------------------------------------------
MONSTERBORNPOINT* CMonsterAttackCity::GetRandBornPoint()
{
  const WORD wSize = m_ListBornPoint.size();
  MONSTERBORNPOINT* pPoint = NULL;
  if(wSize)
  {
    srand(TimeGetTime());
    const WORD wRand = rand()%(wSize);
    list<MONSTERBORNPOINT*>::iterator  Iter_Point;
    Iter_Point = m_ListBornPoint.begin();
    WORD wCounter = 0;
    while(wRand != wCounter)
    {
      ++Iter_Point;
      ++wCounter;
      if(m_ListBornPoint.end() == Iter_Point)
      {
        Iter_Point = m_ListBornPoint.begin();
      }
    }
    pPoint = *Iter_Point;
  }
  return pPoint;
}
ATTACKCITYSTATE CMonsterAttackCity::GetALoopState() const
{
  return m_dwState;
}
void CMonsterAttackCity::SetLoopState(const ATTACKCITYSTATE pState)
{
  m_dwState = pState;
}
bool CMonsterAttackCity::IsTimeBeginPrepare()
{
  FuncName("CMonsterAttackCity::IsTimeBeginPrepare");
  list<ACTIVEATTACKDATE*>::iterator     Iter_Date;
  ACTIVEATTACKDATE* pDate = NULL;
  SYSTEMTIME  sDate;
  GetLocalTime(&sDate);
  for(Iter_Date = m_ListAttackDate.begin(); Iter_Date != m_ListAttackDate.end(); ++Iter_Date)
  {
    pDate = *Iter_Date;
    if( pDate->m_Date.wDayOfWeek == sDate.wDayOfWeek &&
        pDate->m_Date.wHour == sDate.wHour &&
        pDate->m_Date.wMinute == sDate.wMinute&&
        sDate.wSecond < 60)
    {
      char szLog[MAX_MEMO_MSG_LEN];
      _snprintf( szLog, MAX_MEMO_MSG_LEN-1, "A Round Monster Attack City Begin %d, %d, %d", 
        sDate.wDayOfWeek, sDate.wHour, sDate.wMinute);
      szLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( szLog );

      return true;
    }
  }
  return false;
}

bool CMonsterAttackCity::IsTimeToEnd()
{
  FuncName("CMonsterAttackCity::IsTimeToEnd");
  list<ACTIVEATTACKDATE*>::iterator     Iter_Date;
  ACTIVEATTACKDATE* pDate = NULL;
  
  DWORD  dwNowTime = TimeGetTime();

  for(Iter_Date = m_ListAttackDate.begin(); Iter_Date != m_ListAttackDate.end(); ++Iter_Date)
  {
    pDate = *Iter_Date;
    //活动开始1个小时后，并且在1小时10分钟以内结束
    if((m_dwAttackBeginTime + 3600000 < dwNowTime) && (m_dwAttackBeginTime + 4200000 > dwNowTime))
    {
      SYSTEMTIME  sDate;
      GetLocalTime(&sDate);
      char szLog[MAX_MEMO_MSG_LEN];
      _snprintf( szLog, MAX_MEMO_MSG_LEN-1, "A Round Monster Attack City End %d, %d, %d", 
        sDate.wDayOfWeek, sDate.wHour, sDate.wMinute);
      szLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( szLog );
      
      m_dwAttackBeginTime = 0; 
      return true;
    }
  }
  return false;
}

void CMonsterAttackCity::SendMsgToAll( char* pszContent )
{  
  int iStrLen = strlen(pszContent);
  if( iStrLen <= 0 ) return;
  if( iStrLen > MAX_TALKMSG-1 ) iStrLen = MAX_TALKMSG-1;
  SMccMsgData     *pNewMccMsg;
  if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )
    return;
  //
  pNewMccMsg->Init( NULL );
  pNewMccMsg->dwAID        = AP_GMINSTR;
  pNewMccMsg->dwMsgLen     = 2;
  pNewMccMsg->Msgs[0].Size = sizeof( SNMGMInstruction );
  pNewMccMsg->Msgs[1].Size = iStrLen+1;

  SNMGMInstruction* pInstr = (SNMGMInstruction*)pNewMccMsg->Msgs[0].Data;
  memset( pInstr, 0, sizeof(SNMGMInstruction) );
  pInstr->wCode = 0;
  pInstr->wGMInstCode = GM_INST_TALKTOALL;
  memcpy( pInstr->szName, "ddd", MAX_ACCOUNT_LEN );
  pInstr->szName[MAX_ACCOUNT_LEN-1] = '\0';

  memcpy( pNewMccMsg->Msgs[1].Data, pszContent, pNewMccMsg->Msgs[1].Size );
  pNewMccMsg->Msgs[1].Data[iStrLen] = '\0';
  //
  g_pMccDB->AddSendMsg( pNewMccMsg );
};

//---------------------------------------------------------------

void CMonsterAttackCity::CreateMonster(const int iMonsterId , const WORD wCount, const DWORD dwFloorId)
{
  
  CGameMap* pMap = g_pGs->GetGameMap(dwFloorId);
  if( NULL != pMap)
  {
    SMsgData* pTheMsg = g_pGs->NewMsgBuffer();
    pTheMsg->Init();
    CMonster				*pNewMonster;
    WORD						wCreateCount = 0;
    SNMNpcInfo			*pTheNpcInfo = (SNMNpcInfo*)(pTheMsg->Msgs[1].Data);
    for( int i = 0; i < wCount; i++ )
    {
      if( NULL != ( pNewMonster = pMap->CreateMonster( iMonsterId, ( m_iX - 1 + gf_GetRandom( 2 ) ), ( m_iY - 1 + gf_GetRandom( 2 ) ), 0, 0 ) ) )
      {
        pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
        pNewMonster->SetReviveType( REVIVE_TYPE_GM_CREATE );
        pTheNpcInfo++;
        wCreateCount++;
      }
    }
    if( wCreateCount )
    {
      pTheMsg->dwAID        = A_PLAYERINFO;
      pTheMsg->dwMsgLen     = 2;
      pTheMsg->Msgs[0].Size = 1;
      pTheMsg->Msgs[1].Size = sizeof( SNMNpcInfo ) * wCreateCount;
      
      pMap->SendMsgNearPosition( *pTheMsg, m_iX, m_iY );
    }
    g_pGs->ReleaseMsg( pTheMsg );
  }
}


void CMonsterAttackCity::ResetAttackMonsterInfo()
{ 
  m_dwFloorId = 0;
  m_iX = 0;
  m_iY = 0;
  m_iMonsterId = 0;
  m_iBossId = 0;
  m_wCreateMonsterTime = 0;
}
int CMonsterAttackCity::InitAttackMonsterInfo()
{
  MONSTERSEED* pSeed =  GetNextBossSeed();
  int iRet = 0;
  if(NULL != pSeed)
  {
    iRet = 1;
    m_iBossId = pSeed->m_iMonsterId;
    pSeed = GetRandMonster();
    if(NULL != pSeed)
    {
      iRet = 2;
      m_iMonsterId = pSeed->m_iMonsterId;
      MONSTERBORNPOINT* pPoint =  GetRandBornPoint();
      if(NULL != pPoint)
      {
        m_dwFloorId = pPoint->m_dwFloorId;
        m_iX = pPoint->m_iX;
        m_iY = pPoint->m_iY;
        iRet = 3;
      }
      else
      {
        iRet = 4;
      }
    }
  }
  return iRet;
}


void CMonsterAttackCity::RemoveMonster()
{
  g_pGs->DisableAllMonster(0,true);
}

MONSTERSEED* CMonsterAttackCity::GetNextBossSeed()
{
  MONSTERSEED* pSeed = NULL;
  if(m_ListMonsterPointer != m_ListBossSeed.end())
  {
    pSeed = *m_ListMonsterPointer;
    ++m_ListMonsterPointer;
  }
  return pSeed;
}

bool CMonsterAttackCity::IsBossListEnd()
{
  return (m_ListBossSeed.end() == m_ListMonsterPointer);
}
void CMonsterAttackCity::ResetBossPointer()
{
  m_ListMonsterPointer = m_ListBossSeed.begin();
}

#endif

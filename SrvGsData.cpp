 #include "stdafx.h"
#include "SrvGsData.h"

#include <Winsock2.h>

#include "../SrvClass/Map/SrvMap.h"
#include "../SrvClass/Life/SrvPlayer.h"
#include "../SrvClass/Life/SrvNpc.h"
#include "../SrvClass/Life/SrvMonster.h"
#include "../SrvClass/Item/SrvItem.h"
#include "../SrvClass/Time/GameTime.h"
#include "../Network/SrvClientData.h"
#include "../../_common/iniFile.h"
#include "../Mcc/SrvMccInfo.h"
#include "../Resource.h"

// Add By WildCat 2002-5-22
#ifdef _DEBUG_WILDCAT_
extern DWORD			g_dwNetFunc[5], g_dwMonster[MAX_MAP_NUM][2], g_dwNpc[MAX_MAP_NUM][2], g_dwPlayer[MAX_MAP_NUM][2], g_dwClientNet, g_dwMccNet, g_dwMap, g_dwLoop, g_dwNetLoop, g_dwAcceptLoop;
#endif
extern CMccInfo		 *g_pMccDB, *g_pMccChat;
extern HWND        g_hwnd;
#ifdef _DEBUG_CHECK_ACTION_STATE_
extern char        szActionCode[MAX_ACCOUNT_LEN];
#endif

WORD      g_wItemUnipueIdCounter = 0;
BOOL      g_bHandleSelRet0 = FALSE;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// base data table
//
//=========================================================================================================
//
//
CGsData::CGsData(HWND hTheWnd)
{
  //FuncName("CGsData::CGsData");
  //
	m_maxPlayerLevel = 0 ; ///add by fhc

  m_bWinsockOk = true;
	m_iMaxPlayers       = 0;
	m_iPlayerMessNum    = 0;
	m_iPlayerMccMessNum = 0;
  m_iMccRecvMessNum   = 0;
  m_iEndGameTime      = 0;
  m_iMaxLoginReady    = 0;
  m_iNowLoginReady    = 0;
  //
  m_iGsState          = GSSTATE_STOP;
	m_iMapTotal         = 0;
  m_iSelfMapCount     = 0;
  m_wServerGroup      = 0;

  InitializeCriticalSection(&m_cs_listCheater);
  InitializeCriticalSection(&m_cs_GsState);

	// Zero Message Buffer
	m_MsgBuffer    = NULL;
	m_MccMsgBuffer = NULL;
	m_TimeOutMsg   = NULL;

	// Zero Message Buffer
	m_cs_PlayerMsgBuffer			= NULL;
	m_ListPlayerMsgBuffer			= NULL;
	m_cs_PlayerMccMsgBuffer		= NULL;
	m_ListPlayerMccMsgBuffer	= NULL;
  // for System
  m_hWnd = hTheWnd;
  m_pGameTime = new CGameTime();
#ifdef _DEBUG_WILDCAT_
	dwCheckBuffer = 0;
#endif
	m_listCheater.clear();
  // Clear And Fill Data
  listAllMapData.clear();

	m_dwLoadItemInterval = 1000;
  m_bCheckCheater      = FALSE;
  m_bOpenLogDB         = TRUE;
  m_iMaxHistoryPlayer  = 0;
  m_dwGameStartTime    = TimeGetTime();
	m_listGMIP.clear();
  //
  m_iPlayerItemCount = m_iPlayerSkillCount = m_iPlayerMagicCount = 0;
  //
  m_dwMaxItem   = 0;
  m_dwUsingItem = 0;
  m_pItemMem    = NULL;
  m_ListItem.clear();
  //
  m_dwMaxSkill   = 0;
  m_dwUsingSkill = 0;
  m_pSkillMem    = NULL;
  m_ListSkill.clear();
  //
  m_dwMaxMagic   = 0;
  m_dwUsingMagic = 0;
  m_pMagicMem    = NULL;
  m_ListMagic.clear();

  InitializeCriticalSection( &m_cs_ReadFlux );
  InitializeCriticalSection( &m_cs_WriteFlux );
  m_dwMaxFluxRead      = 0;
  m_dwAverageFluxRead  = 0;
  m_dwMaxFluxWrite     = 0;
  m_dwAverageFluxWrite = 0;
  //
  InitializeCriticalSection( &m_cs_ReadFluxMcc );
  InitializeCriticalSection( &m_cs_WriteFluxMcc );
  m_dwMaxFluxReadMcc      = 0;
  m_dwAverageFluxReadMcc  = 0;
  m_dwMaxFluxWriteMcc     = 0;
  m_dwAverageFluxWriteMcc = 0;
  m_bRecordPlayerTalk     = 0;
  //
  m_mapAccountPlayer.clear();
  m_mapRandomMonster.clear();
}
//=========================================================================================================
//
//
CGsData::~CGsData()
{
  FuncName("CGsData::~CGsData");

  //保存item unique ID 计数的基数
  //CIni itemIni(BF_INI_FILE); 
  //itemIni.Write( "Common", "ItemUniqueIdCounterBase", CItem::m_64UniqueIdCounter );
	//itemIni.Save(BF_INI_FILE);
  // clean Winsock (Winsock startup in CGsData::CGsData())
  if(0 == WSACleanup())
  {
    _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "WSACleanup ok");
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg(g_szGsDataLog);
  }
  else
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "WSACleanup error=%d", WSAGetLastError());
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(g_szGsDataLog);
#endif
  }
  
  // clear all global data
  m_mapCodePlayer.clear();
  m_mapMailPlayer.clear();

  // clear GameMaps in the server
  CGameMap			*pMap = NULL;
	for(map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++)
  {
    pMap = iter->second;
		if( pMap )
    {
      SAFE_DELETE( pMap );
    }
  }
  m_mapGameMap.clear();

  if(m_pGameTime)
  {
    SAFE_DELETE( m_pGameTime );
  }

  listAllMapData.clear();

  DeleteCriticalSection(&m_cs_listCheater);
  DeleteCriticalSection(&m_cs_GsState);

	if( m_iGsState == GSSTATE_GAME_START )
	{
		DeleteCriticalSection(&m_cs_SysMsgBuffer);
		DeleteCriticalSection(&m_cs_SysMccMsgBuffer);
		DeleteCriticalSection(&m_cs_RecvMccMsgBuffer);
		DeleteCriticalSection(&m_cs_TimeoutBuffer);
    DeleteCriticalSection(&m_cs_MccAshcan);
    DeleteCriticalSection(&m_cs_SysAshcan);
		for( int i = 0; i < GetMaxPlayer(); i++ )
		{
			DeleteCriticalSection(&m_cs_PlayerMsgBuffer[i]);
			DeleteCriticalSection(&m_cs_PlayerMccMsgBuffer[i]);
		}
	}
	if( m_cs_PlayerMsgBuffer )
  {
    delete[] m_cs_PlayerMsgBuffer;
    m_cs_PlayerMsgBuffer = NULL;
  }
	if( m_cs_PlayerMccMsgBuffer )
  {
    delete[] m_cs_PlayerMccMsgBuffer;
    m_cs_PlayerMccMsgBuffer = NULL;
  }

	ReleaseAllMsgBuffer();

	m_listCheater.clear();
	for( list<CGMIpData*>::iterator i = m_listGMIP.begin(); i != m_listGMIP.end(); i++ )
	{
    SAFE_DELETE( (*i) );
  }
	m_listGMIP.clear();

	if( GetFileAttributes( BFSERVER_GM_LOGPATH ) == 0xFFFFFFFF )
	{
		CreateDirectory( BFSERVER_GM_LOGPATH, NULL );
	}
  DeleteCriticalSection(&m_cs_ReadFlux);
  DeleteCriticalSection(&m_cs_WriteFlux);
  DeleteCriticalSection(&m_cs_ReadFluxMcc);
  DeleteCriticalSection(&m_cs_WriteFluxMcc);
}
//=========================================================================================================
//
//
bool CGsData::Init()
{
  FuncName("CGsData::Init");
  BOOL          bHaveSichuanGM = FALSE;

  CIniFile BfIni(BF_INI_FILE);

  // Does Winsock Startup ok in CGsData::CGsData() ?
  if(!m_bWinsockOk)
  {
    return false;
  }
  //读入item unique ID的计数基数    
  //CItem::m_64UniqueIdCounter = BfIni.ReadInteger("Common", "ItemUniqueIdCounterBase", 0);

  // Handle select() Return 0
  g_bHandleSelRet0 = BfIni.ReadInteger( "Common", "HandleSelRet0", 0 );
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Handle Select() Return 0 : %d", g_bHandleSelRet0);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);

  // Read Ini File
  BfIni.ReadString("Common", "ServerName", m_szServerName, MAX_GAME_SERVER_NAME);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Game Server Name: %s", m_szServerName);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  // Get Server Id
  //m_iServerId = BfIni.ReadInteger("Common", "ServerId", 1);
	// Get Max Player
  m_iMaxPlayers = BfIni.ReadInteger("Common", "MaxPlayer", 50);
	if( m_iMaxPlayers > 2000 )
		m_iMaxPlayers = 2000;
  m_iMaxPlayers += 50;
//Luou Marked for more memory
//	else if( m_iMaxPlayers < 50 )
//		m_iMaxPlayers = 50;
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Max Players: %d", m_iMaxPlayers);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);

  // About Accept Time
  m_dwAcceptInterval = BfIni.ReadInteger("Common", "AcceptCount", 2);
  if( m_dwAcceptInterval > 5 )    m_dwAcceptInterval = 5;
  m_dwAcceptInterval = 1000 / m_dwAcceptInterval;
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Accept Interval=%d, Accept Count Per Second=%d", m_dwAcceptInterval, 1000/m_dwAcceptInterval);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  
  // system message buffer num for each player
  m_iSystemMessNum = BfIni.ReadInteger( "Common", "SystemMessNum", 8 );
  if( m_iSystemMessNum > 8 )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "System Mess Num Modify: %d==>>8", m_iSystemMessNum );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    m_iSystemMessNum = 8;
  }
  else if( m_iSystemMessNum < 4 )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "System Mess Num Modify: %d==>>4", m_iSystemMessNum );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    m_iSystemMessNum = 4;
  }
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "System Mess Num: %d", m_iSystemMessNum );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
	// Get Player Message Number; 优化内存后 Recv(4) + Send(12) = 16
	m_iPlayerMessNum = BfIni.ReadInteger("Common", "EveryPlayerMessNum", 16 );
	if( m_iPlayerMessNum > 16 )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Very Player Mess Num Modify: %d==>>16", m_iPlayerMessNum );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    m_iPlayerMessNum = 16;
  }
	else if( m_iPlayerMessNum < 8 )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Very Player Mess Num Modify: %d==>>8", m_iPlayerMessNum );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    m_iPlayerMessNum = 8;
  }
  //
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Player Mess Num: %d", m_iPlayerMessNum );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
  // mcc recv message buffer num for each player
  m_iMccRecvMessNum = BfIni.ReadInteger("Common", "MccRecvMessNum", 4 );
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Mcc Recv Mess Num: %d", m_iMccRecvMessNum );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
	// Get Player Mcc Message Num
	m_iPlayerMccMessNum  = BfIni.ReadInteger( "Common", "PlayerMccMessNum", 4 );
  m_iPlayerMccMessNum += 2;
	//if( m_iPlayerMccMessNum > 16 )
	//	m_iPlayerMccMessNum = 16;
	//else if( m_iPlayerMccMessNum < 4 )
	//	m_iPlayerMccMessNum = 4;
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Player Mcc Mess Num: %d", m_iPlayerMccMessNum );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
  // Sys Mcc Msg
	m_iSystemMccMessNum  = BfIni.ReadInteger( "Common", "SystemMccMessNum", 1 );
  m_iSystemMccMessNum += 1;
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "System Mcc Mess Num: %d", m_iSystemMccMessNum );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
	// Get Gs Port
  m_iGsPort = BfIni.ReadInteger("Common", "GsPort", 5999);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Game Server Port: %d", m_iGsPort);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  m_iClient_Gs_Encrypt = BfIni.ReadInteger("Common", "Filter_Gs_Encrypt", 0);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Encrypt Between Filter And Gs: %d", m_iClient_Gs_Encrypt);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  // Load Server Group
  m_wServerGroup = BfIni.ReadInteger("Common", "ServerGroup", 0xFFFF);
  if( m_wServerGroup == 0xFFFF )
  {
    ::MessageBox( g_hwnd, "Server Group Is NULL, Please Set It And Be Sure Unique", "Warning", 0 );
    return FALSE;
  }
  if( m_wServerGroup > 60000 )
  {
    ::MessageBox( g_hwnd, "Server Group Is Overflow, Please < 60000", "Warning", 0 );
    return FALSE;
  }
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Server Group: %d", m_wServerGroup);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  //
  m_bOpenLogDB = BfIni.ReadInteger( "Common", "OpenLogDB", 1 );
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Open Log DB: %d", m_bOpenLogDB);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	// Load Client Login Load Character Info Interval
	m_dwLoadItemInterval = BfIni.ReadInteger("Common", "LoadInterval", 1000);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Player Load Item And Character Data Interval (ms): %d", m_dwLoadItemInterval);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	// About Ground Item Active Interval
  m_iItemVanish = BfIni.ReadInteger("Common", "ItemVanish", 0);
  //
	if( m_iItemVanish < 180000 )          m_iItemVanish = 180000;
	else if( m_iItemVanish > 600000 )     m_iItemVanish = 600000;
  //
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Item Vanish interval (ms): %d", m_iItemVanish);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] ='\0';
  AddMemoMsg(g_szGsDataLog);
	// About Npc Cure Cost
	m_wNpcCureHp = BfIni.ReadInteger("Common", "NpcCureHpPer", 6);
	m_wNpcCureMp = BfIni.ReadInteger("Common", "NpcCureMpPer", 2);
	_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Npc Cure Cost 1$ / Hp=%d, 1$ / Mp=%d", m_wNpcCureHp, m_wNpcCureMp);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	// About Npc Dye Cost
	m_wNpcDyeCost = BfIni.ReadInteger("Common", "NpcDyeCost", 200);
	_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Npc Dye Cost = %d", m_wNpcDyeCost );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	// About Default Weapon
	m_wDefaultSword = BfIni.ReadInteger("Common", "DefaultSword", 101);
	m_wDefaultPike  = BfIni.ReadInteger("Common", "DefaultPike", 103);
	// About Save Player Data Interval
	m_dwSaveItemInterval = BfIni.ReadInteger("Common", "SaveItemInterval", 1000);
	m_dwSaveCharInterval = BfIni.ReadInteger("Common", "SaveCharInterval", 1000);
	_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Save Item Interval (Seconds):%d, Save Character Interval=%d(ms) When End Game #", m_dwSaveItemInterval/1000, m_dwSaveCharInterval/1000 );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	// About Save Player Data In Game Start Interval
  m_iSaveInterval = BfIni.ReadInteger("Common", "SaveInterval", 3600000);
  if( m_iSaveInterval > 10800000 )        m_iSaveInterval = 10800000;
  else if( m_iSaveInterval < 900000 )    m_iSaveInterval = 900000;
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Auto Save Interval (Minutes): %d", m_iSaveInterval/1000/60 );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	//
  m_iSaveRetryInterval = BfIni.ReadInteger("Common", "SaveRetryInterval", 180000);
  if( m_iSaveRetryInterval > 600000 )       m_iSaveRetryInterval = 600000;
  else if( m_iSaveRetryInterval < 180000 )  m_iSaveRetryInterval = 600000;
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Save Retry Interval (Minutes): %d", m_iSaveRetryInterval/1000/60);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
	// About Player Wait Time When Player Logout
  m_iLogoutWaitInterval = BfIni.ReadInteger("Common", "LogoutWaitInterval", 30000);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Logout Wait Interval (Seconds): %d", m_iLogoutWaitInterval/1000);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  
  // About Whisper Account
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( m_szWhisperAccount, "WaeiWhisper", MAX_ACCOUNT_LEN );
	SafeStrcpy( m_szWhisperPassword, "112233", MAX_PASSWORD_LEN );
#else	
  strcpy( m_szWhisperAccount, "WaeiWhisper" );
  strcpy( m_szWhisperPassword, "112233" );
#endif
  BfIni.ReadString( "Common", "WhisperAccount", m_szWhisperAccount, MAX_ACCOUNT_LEN );
  BfIni.ReadString( "Common", "WhisperPassword", m_szWhisperPassword, MAX_PASSWORD_LEN );
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Whisper Account: %s", m_szWhisperAccount);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Whisper Password: %s", m_szWhisperPassword);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  // About Login Ready
  m_iMaxLoginReady = BfIni.ReadInteger( "Common", "MaxLoginReady", 100 );
  // About DB
  //BfIni.ReadString( "DBData", "DBAccount", m_szDBAccount, 128 );
  //BfIni.ReadString( "DBData", "DBPassword", m_szDBPassword, 128 );
  BfIni.ReadString( "DBData", "DBIp", m_szDBIp, 16 );
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "DB Account: %s", m_szDBAccount );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "DB Password: *********" );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "DBIp: %s", m_szDBIp);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  
  // About Accept Interval
  m_iAcceptInterval = BfIni.ReadInteger("Common", "AcceptInterval", 250);
  _snprintf(g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Accept Interval: %d", m_iAcceptInterval);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);
  //
  m_bCheckCheater = BfIni.ReadInteger("Common", "CheckCheater", 0);
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "CheckCheater = %d", m_bCheckCheater);
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
  //
  m_bRecordPlayerTalk = BfIni.ReadInteger("Common", "RecordPlayerTalk", 0);
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Record Player Talk = %d", m_bRecordPlayerTalk );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( g_szGsDataLog );
  // Init code
  m_codePlayer.Init( CODE_MIN_PLAYER, CODE_MAX_PLAYER );


	if( !g_pClientList->InitMaxList( GetMaxPlayer() ) )
  {
    AddMemoMsg( "g_pClientList->InitMaxList(g_pGs->GetMaxPlayer()) Fail !#" );
    return false;
  }

  // About Item And Skill Memory Block
  // Item Package = 150;    Item Storage = 120;   OtherItem/Player  > 400 - 270 = 130;
  // Learn Skill = 30;      Learn Magic  = 30;    OtherMagic/Player > 200 - 60  = 140;
  m_iPlayerItemCount  = BfIni.ReadInteger("Common", "PlayerItemCount", 400 );
  if( m_iPlayerItemCount > 1000 )       m_iPlayerItemCount = 1000;
  else if( m_iPlayerItemCount < 400 )   m_iPlayerItemCount = 400;
  m_iPlayerSkillCount = BfIni.ReadInteger("Common", "PlayerSkillCount", 200 );
  if( m_iPlayerSkillCount > 500 )       m_iPlayerSkillCount = 300;
  else if( m_iPlayerSkillCount < 200 )  m_iPlayerSkillCount = 200;
  m_iPlayerMagicCount = BfIni.ReadInteger("Common", "PlayerMagicCount", 200 );
  if( m_iPlayerMagicCount > 30 )        m_iPlayerMagicCount = 30;
  else if( m_iPlayerMagicCount < 20 )   m_iPlayerMagicCount = 20;

	// Add By WildCat 2002-5-20 For static message buffer
	InitMsgBuffer();
  InitCItemMem();
  InitCSkillMem();
  InitCMagicMem();
  // About No Mcc Mode
  m_iMccMode = BfIni.ReadInteger("Mcc", "MccMode", 1);
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Mcc Mode: Mcc Count=%d", m_iMccMode );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(g_szGsDataLog);

  // About Whisper Count
  g_pWhispers->SetMaxWhisper( BfIni.ReadInteger( "Common", "MaxWhisper", 1 ) );

  // Clear Data
	m_listGmData.clear();
  m_mapCodePlayer.clear();
  m_mapMailPlayer.clear();
  m_mapNamePlayer.clear();
  m_mapGameMap.clear();
  // Init GameTime
  m_pGameTime->ResetTime();

	// Get Base GM
	if( m_iMccMode )
	{
		SGmData			GM;

#ifdef _REPAIR_SERVER_CRASH_NICK_
		SafeStrcpy( GM.szAccount, "WILDCAT", MAX_ACCOUNT_LEN );
#else
		strcpy( GM.szAccount, "WILDCAT" );
#endif
		GM.iAuthority = 3;
		m_listGmData.push_back( GM );
	}

	// Load GM Ip Addr
  CIniFile		GMIni(BF_GM_IP_FILE);
	int					iGMIPCount = 0;
	char				szGMIp[16];
	CGMIpData		*pGMIPData;

  iGMIPCount = GMIni.ReadInteger( "GMIP", "GMIPCount", 1 );
  
	for( int m = 0; m < iGMIPCount; m++ )
	{
		pGMIPData = new CGMIpData;
		_snprintf( szGMIp, 16-1, "IpStart%d", m + 1 );
    szGMIp[16-1] = '\0';
		GMIni.ReadString( "GMIP", szGMIp, pGMIPData->m_szIpStart, 16 );
    _snprintf( szGMIp, 16-1, "IpGate%d", m + 1 );
    szGMIp[16-1] = '\0';
    GMIni.ReadString( "GMIP", szGMIp, pGMIPData->m_szIpGate, 16 );
    _snprintf( szGMIp, 16-1, "Privilege%d", m + 1 );
    szGMIp[16-1] = '\0';
    pGMIPData->m_wPrivilege = GMIni.ReadInteger( "GMIP", szGMIp, 1 );
#ifdef DEBUG_VERSION_FOR_BEIJING_
    if( (0 == strcmp(pGMIPData->m_szIpStart," ")) || ( 0 == strcmp(pGMIPData->m_szIpGate," ")))
      pGMIPData->m_wPrivilege = 0xffff;
#endif
    //
    pGMIPData->m_iIpStart[0] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 1 );
    pGMIPData->m_iIpStart[1] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 2 );
    pGMIPData->m_iIpStart[2] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 3 );
    pGMIPData->m_iIpStart[3] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 4 );
    // Check Sichuan GM Ip
    if( pGMIPData->m_iIpStart[0] == 211 && pGMIPData->m_iIpStart[1] == 95 &&
        pGMIPData->m_iIpStart[2] == 163 && pGMIPData->m_iIpStart[3] == 100 )
    {
      bHaveSichuanGM = TRUE;
    }
    //
    pGMIPData->m_iIpGate[0] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 1 );
    pGMIPData->m_iIpGate[1] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 2 );
    pGMIPData->m_iIpGate[2] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 3 );
    pGMIPData->m_iIpGate[3] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 4 );
    //
    if( pGMIPData->m_iIpStart[0] < 0 || pGMIPData->m_iIpStart[1] < 0 ||
        pGMIPData->m_iIpStart[2] < 0 || pGMIPData->m_iIpStart[3] < 0 ||
        pGMIPData->m_iIpGate[0]  < 0 || pGMIPData->m_iIpGate[1]  < 0 ||
        pGMIPData->m_iIpGate[2]  < 0 || pGMIPData->m_iIpGate[3]  < 0 )
    {
      AddMemoErrMsg( "***** Read GM.ini Error, Ip Is Error ! *****" );
      return false;
    }
		m_listGMIP.push_back( pGMIPData );
	}
  //
  if( bHaveSichuanGM == FALSE )
  {
    pGMIPData = new CGMIpData;
    pGMIPData->m_iIpStart[0] = 211;
    pGMIPData->m_iIpStart[1] = 95;
    pGMIPData->m_iIpStart[2] = 163;
    pGMIPData->m_iIpStart[3] = 100;
    //
    pGMIPData->m_iIpGate[0] = 255;
    pGMIPData->m_iIpGate[1] = 255;
    pGMIPData->m_iIpGate[2] = 255;
    pGMIPData->m_iIpGate[3] = 255;
    m_listGMIP.push_back( pGMIPData );
  }
  // Get Whisper Ip Gate
  iGMIPCount = GMIni.ReadInteger( "WhisperIP", "WhisperIPCount", 1 );
	for( m = 0; m < iGMIPCount; m++ )
  {
		pGMIPData = new CGMIpData;
		_snprintf( szGMIp, 16-1, "WIpStart%d", m + 1 );
    szGMIp[16-1] = '\0';
		GMIni.ReadString( "WhisperIP", szGMIp, pGMIPData->m_szIpStart, 16 );
    _snprintf( szGMIp, 16-1, "WIpGate%d", m + 1 );
    szGMIp[16-1] = '\0';
    GMIni.ReadString( "WhisperIP", szGMIp, pGMIPData->m_szIpGate, 16 );
    pGMIPData->m_wPrivilege = 0;
    //
    pGMIPData->m_iIpStart[0] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 1 );
    pGMIPData->m_iIpStart[1] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 2 );
    pGMIPData->m_iIpStart[2] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 3 );
    pGMIPData->m_iIpStart[3] = gf_GetIntegerFromChar( pGMIPData->m_szIpStart, 4 );
    //
    pGMIPData->m_iIpGate[0] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 1 );
    pGMIPData->m_iIpGate[1] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 2 );
    pGMIPData->m_iIpGate[2] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 3 );
    pGMIPData->m_iIpGate[3] = gf_GetIntegerFromChar( pGMIPData->m_szIpGate, 4 );
    //
    if( pGMIPData->m_iIpStart[0] < 0 || pGMIPData->m_iIpStart[1] < 0 ||
        pGMIPData->m_iIpStart[2] < 0 || pGMIPData->m_iIpStart[3] < 0 ||
        pGMIPData->m_iIpGate[0]  < 0 || pGMIPData->m_iIpGate[1]  < 0 ||
        pGMIPData->m_iIpGate[2]  < 0 || pGMIPData->m_iIpGate[3]  < 0 )
    {
      AddMemoErrMsg( "***** Read GM.ini Error, Ip Is Error ! *****" );
      return false;
    }
		m_listWhisperIP.push_back( pGMIPData );
	}
  // Set Check Cheater
  HMENU   hMenu         = GetMenu( g_hwnd );
  if( g_pGs->GetCheckCheater() )  ::CheckMenuItem( hMenu, IDM_CHECKCHEATER, MF_CHECKED );
  else                            ::CheckMenuItem( hMenu, IDM_CHECKCHEATER, MF_UNCHECKED );
  //
  if( g_pGs->RecordPlayerTalk() ) ::CheckMenuItem( hMenu, IDM_RECORDPLAYERTALK, MF_CHECKED );
  else                            ::CheckMenuItem( hMenu, IDM_RECORDPLAYERTALK, MF_UNCHECKED );
  return true;
}
//=========================================================================================================
//
// Get Information of the Game Server
CGameMap* CGsData::GetGameMap(const DWORD & dwMapId)
{
  static map<DWORD, CGameMap*>::iterator  Iter_Map;

  Iter_Map = m_mapGameMap.find( dwMapId );
  if( Iter_Map != m_mapGameMap.end() )
  {
    return Iter_Map->second;
  }
  return NULL;
}
//=========================================================================================================
//
//
bool CGsData::AddPlayerByAccount( CPlayer * pPlayer )
{
  string      szCheckAct = pPlayer->GetAccount();
  //
  if( m_mapAccountPlayer.find( szCheckAct ) != m_mapAccountPlayer.end() )      return false;
  m_mapAccountPlayer.insert( map<string,CPlayer*>::value_type( szCheckAct, pPlayer ) );
  return true;
}
//=========================================================================================================
//
//
void CGsData::DelPlayerByAccount( CPlayer * pPlayer )
{
  string      szCheckAct = pPlayer->GetAccount();
  //
  m_mapAccountPlayer.erase( szCheckAct );
}
#ifdef _SRV_CLIENT_ARRAY_
//=========================================================================================================
//
//
CPlayer * CGsData::GetInServerPlayerByAccount( char * szAccount )
{
  map<string,CPlayer*>::iterator    Iter_Pl = m_mapAccountPlayer.find( string(szAccount) );
  if( Iter_Pl == m_mapAccountPlayer.end() )     return NULL;
  return Iter_Pl->second;
}
//=========================================================================================================
//
//
CPlayer* CGsData::GetPlayerFromCode(const WORD & wCode)
{
  static CPlayer    *pFoundPlayer = NULL;

  if( wCode >= CODE_MIN_PLAYER && wCode < ( CODE_MIN_PLAYER + m_iMaxPlayers ) )
  {
    pFoundPlayer = g_pClientList->m_pClientArray[wCode-CODE_MIN_PLAYER].GetPlayer();
    //if( pFoundPlayer->IsInGame() )
    if( pFoundPlayer->GetStatus() != STATUS_PLAYER_WAIT )
    {
      return pFoundPlayer;
    }
  }
  return NULL;
}
//=========================================================================================================
//
//
CPlayer* CGsData::FindPlayerFromMessVerify( CMessVerify *pMessVerify )
{
  static CPlayer    *pFoundPlayer = NULL;
  WORD              wMyCode = pMessVerify->wGsCode;

  if( wMyCode >= CODE_MIN_PLAYER && wMyCode < ( CODE_MIN_PLAYER + m_iMaxPlayers ) )
  {
    pFoundPlayer = g_pClientList->m_pClientArray[wMyCode-CODE_MIN_PLAYER].GetPlayer();
    if( pFoundPlayer->GetStatus() != STATUS_PLAYER_WAIT )
    {
      if( !strcmp( pFoundPlayer->GetAccount(), pMessVerify->szAccount ) )   return pFoundPlayer;
    }
  }
  return NULL;
}
//=========================================================================================================
//
//
CPlayer* CGsData::FindPlayerFromVerifyMailId( const WORD & wCode, const DWORD & dwMailId )
{
  static CPlayer    *pFoundPlayer = NULL;

  if( wCode >= CODE_MIN_PLAYER && wCode < ( CODE_MIN_PLAYER + m_iMaxPlayers ) )
  {
    pFoundPlayer = g_pClientList->m_pClientArray[wCode-CODE_MIN_PLAYER].GetPlayer();
    if( pFoundPlayer->GetStatus() != STATUS_PLAYER_WAIT )
    {
      if( pFoundPlayer->GetMailId() == dwMailId )   return pFoundPlayer;
    }
  }
  return NULL;
}
//=========================================================================================================
//
//
CPlayer* CGsData::GetPlayerFromCodeAll(const WORD & wCode)
{
  static CPlayer    *pFoundPlayer = NULL;

  if( wCode >= CODE_MIN_PLAYER && wCode < ( CODE_MIN_PLAYER + m_iMaxPlayers ) )
  {
    if( g_pClientList->m_pClientArray[wCode-CODE_MIN_PLAYER].GetState() > CLIENTSTATE_LOGOUT_OK )
    {
      return g_pClientList->m_pClientArray[wCode-CODE_MIN_PLAYER].GetPlayer();
    }
  }
  return NULL;
}
#else
//=========================================================================================================
//
//
CPlayer* CGsData::GetPlayerFromCode(const WORD & dwTheCode)
{
  CPlayer *pPlayer = NULL;
  
  map<DWORD, CPlayer*>::iterator iterPlayer = m_mapCodePlayer.find(dwTheCode);
  if(m_mapCodePlayer.end() != iterPlayer)
  {
    pPlayer = iterPlayer->second;
  }

  if(pPlayer)
  {
		if(pPlayer->IsInGame())
    {
      return pPlayer;
    }
  }
  return NULL;
}
#endif
//=========================================================================================================
//
//
CPlayer * CGsData::GetPlayerByAccount(char * szAccount)
{
	CPlayer * pThePlayer;
	for( map<DWORD, CPlayer*>::iterator iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
	{
		if( NULL != ( pThePlayer = iter->second ) )
		{
			if( !strcmp( szAccount,pThePlayer->GetPlayerAccount() ) )
			{
        if( pThePlayer->IsInGame() )
				  return pThePlayer;
        else
          return NULL;
			}
		}
	}
	return NULL;
}
//=========================================================================================================
//
//
CPlayer* CGsData::GetPlayerFromMailId( const DWORD & iTheMailId )
{
  static CPlayer													*pPlayer;
	static map<DWORD, CPlayer*>::iterator		Iter_MailId;

  pPlayer = NULL;
  Iter_MailId = m_mapMailPlayer.find( iTheMailId );
  if( m_mapMailPlayer.end() != Iter_MailId )
  {
     pPlayer = Iter_MailId->second;
     if( pPlayer->IsInGame() )  return pPlayer;
  }
  return NULL;
}
//=========================================================================================================
//
//
CPlayer * CGsData::GetPlayerFromMailIdAnyway(const  DWORD & dwMailId )
{
	static map<DWORD, CPlayer*>::iterator		Iter_MailId;

  Iter_MailId = m_mapMailPlayer.find( dwMailId );
  if( m_mapMailPlayer.end() != Iter_MailId )
  {
    return Iter_MailId->second;
  }
  return NULL;
}
//==========================================================================================
//
//
void CGsData::GetGameTime( SGameTime & GameTime )
{
	m_pGameTime->GetTime( GameTime );
}
//==========================================================================================
//
//
void CGsData::SetGameTime(SMsgData * pTheMsg)
{
	CGameMap		      *pMap				 = NULL;
	SGameTime		      *SetTime		 = (SGameTime*)(pTheMsg->Msgs[0].Data);
	sGTChange					*pTimeChange = (sGTChange*)(pTheMsg->Msgs[0].Data);
  SNMMapAddInfo     *pMapAddInfo;

	SetTime->wHour = pTimeChange->sGTime.wHour;
  m_pGameTime->SetTime( SetTime );

	FuncName("CGsData::SetGameTime");
#ifdef __DEBUG_DONT_SHOW_ANY_RUNTIME_LOG__
#else
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "==>> Game Time Change To %d", SetTime->wHour );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
	AddMemoMsg( g_szGsDataLog );
#endif

  pTheMsg->Init();
	pTheMsg->dwMsgLen			= 2;
  pTheMsg->dwAID				= A_GAMETIMECHANGE;
	pTheMsg->Msgs[0].Size = sizeof( SGameTime );
  pTheMsg->Msgs[1].Size = sizeof( SNMMapAddInfo );

  pMapAddInfo = (SNMMapAddInfo*)(pTheMsg->Msgs[1].Data);

	if( SetTime->wHour == 19 )
	{ // Night
		for( map<DWORD,CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pMap = (CGameMap*)(iter->second) ) )
			{
				//if( pMap->DayAndNight() )
				{
					// Appear All Night Npc
					pMap->NpcAppearWhenNight();
					// Disappear All Day Npc
					pMap->NpcDisappearWhenNight();
				}
        // Check Map Weather
        pMap->CheckMapAddInfo();

        // Get Map Add Info
        pMap->GetMapAddInfo( *pMapAddInfo );
        // Send Message To All
        pMap->SendTheMsgToAll( *pTheMsg );
			}
		}
	}
	else if( SetTime->wHour == 6 )
	{ // Day
		for( map<DWORD,CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pMap = (CGameMap*)(iter->second) ) )
			{
				//if( pMap->DayAndNight() )
				{
					// Appear All Day Npc
					pMap->NpcAppearWhenDay();
					// Disappear All Day Npc
					pMap->NpcDisappearWhenDay();
				}
        // Check Map Weather
        pMap->CheckMapAddInfo();

        // Get Map Add Info
        pMap->GetMapAddInfo( *pMapAddInfo );
        // Send Message To All
        pMap->SendTheMsgToAll( *pTheMsg );
			}
		}
	}
  else
  {
		for( map<DWORD,CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pMap = (CGameMap*)(iter->second) ) )
			{
        // Check Map Weather
        pMap->CheckMapAddInfo();

        // Get Map Add Info
        pMap->GetMapAddInfo( *pMapAddInfo );
        // Send Message To All
        pMap->SendTheMsgToAll( *pTheMsg );
			}
		}
  }
  g_pGs->ReleaseMsg( pTheMsg );
}
//=========================================================================================================
//
//
int CGsData::SendCommonChannelMsgToAll( SMsgData * pNewMsg )
{
  if( pNewMsg == NULL )		return 0;
  //
  int iNum = m_mapCodePlayer.size() ;
  if( iNum == 0 )
  {
    g_pGs->ReleaseMsg( pNewMsg );
    return 0;
  }
  //
  iNum = 0;
	for( map<DWORD,CPlayer*>::iterator iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
	{
    if( (!((iter->second)->GetSwitch() & PSWITCH_CLOSE_PUB_CHN )) )
    {
      g_pMultiSendPlayer[iNum] = (iter->second);
      //(iter->second)->AddSendMsg( pNewMsg );
      iNum++;
    }
	}
  if( iNum == 0 )       ReleaseMsg( pNewMsg );
  else
  {
    AddRef( pNewMsg, iNum-1 );
    for( int sss = 0; sss < iNum; sss++ )   
      g_pMultiSendPlayer[sss]->AddSendMsg( pNewMsg );
  }
  //
	return iNum;
}
//=========================================================================================================
//
//
int CGsData::SendTheMsgToAll( SMsgData * pNewMsg )
{
  if( pNewMsg == NULL )		return 0;
  //
  int iNum = m_mapCodePlayer.size() ;
  if( iNum == 0 )
  {
    g_pGs->ReleaseMsg( pNewMsg );
    return 0;
  }
  //
  iNum = 0;
	for( map<DWORD,CPlayer*>::iterator iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
	{
    g_pMultiSendPlayer[iNum] = (iter->second);
    //(iter->second)->AddSendMsg( pNewMsg );
    iNum++;
	}
  if( iNum == 0 )       ReleaseMsg( pNewMsg );
  else
  {
    AddRef( pNewMsg, iNum-1 );
    for( int sss = 0; sss < iNum; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pNewMsg );
  }
  //
	return iNum;
}
//=========================================================================================================
//
//
int CGsData::SendTheMsgToAll( const SMsgData & NewMsg )
{
  if( m_mapCodePlayer.empty() )   return 0;
  //
  SMsgData    *pPubMsg = g_pGs->NewMsgBuffer();
  if( pPubMsg == NULL )           return 0;
  ::CopyMessage( NewMsg, pPubMsg);
  //
  map<DWORD,CPlayer*>::iterator   Iter_Pl;
  int                             iNum = 0;
  //
	for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
	{
    g_pMultiSendPlayer[iNum] = (Iter_Pl->second);
    //(Iter_Pl->second)->AddSendMsg( pPubMsg );
    iNum++;
	}
  //
  if( iNum == 0 )       ReleaseMsg( pPubMsg );
  else
  {
    AddRef( pPubMsg, iNum-1 );
    for( int sss = 0; sss < iNum; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pPubMsg );
  }
  //
	return iNum;
}
//=========================================================================================================
//
//
int CGsData::SendOccupChannelMsg( SMsgData *pTheMsg, const WORD & wOccup )
{
  int       iFirst = 0;
  WORD      wTempOccup = wOccup;
  //
  switch( wTempOccup )
  {
  case OCCU_SWORDMAN:
  case OCCU_SWORDMANF:
    wTempOccup = OCCU_SWORDMAN|OCCU_SWORDMANF;
    break;
  case OCCU_BLADEMAN:
  case OCCU_BLADEMANF:
    wTempOccup = OCCU_BLADEMAN|OCCU_BLADEMANF;
    break;
  case OCCU_PIKEMAN:
  case OCCU_PIKEMANF:
    wTempOccup = OCCU_PIKEMAN|OCCU_PIKEMANF;
    break;
  case OCCU_WIZARD:
  case OCCU_WIZARDF:
    wTempOccup = OCCU_WIZARD|OCCU_WIZARDF;
    break;
  default:
    g_pGs->ReleaseMsg( pTheMsg );
    return 0;
  }
  //
	for( map<DWORD,CPlayer*>::iterator iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
	{
    if( (iter->second)->GetOccupation() & wTempOccup &&
        (!((iter->second)->GetSwitch()&PSWITCH_CLOSE_OCCU_CHN)) )
    {
      g_pMultiSendPlayer[iFirst] = (iter->second);
      iFirst++;
			
			if( MAX_MULTISEND_PLAYER == iFirst )		break;
			//
    }
	}
  if( iFirst == 0 )       ReleaseMsg( pTheMsg );
  else
  {
    AddRef( pTheMsg, iFirst-1 );
    for( int sss = 0; sss < iFirst; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheMsg );
  }
  //
	return iFirst;
}
//=========================================================================================================
//
//
bool CGsData::SetGmList(SMsgData *pTheMsg)
{
  FuncName("CGsData::SetGmList");

  // Check Data Size
  int iGmCount = pTheMsg->Msgs[0].Size / sizeof(SGmData);
  if( (sizeof(SGmData) * iGmCount) != pTheMsg->Msgs[0].Size )
  {
    AddMemoErrMsg("Data Size Error !#");
    g_pGs->ReleaseMsg(pTheMsg);
    pTheMsg = NULL;
    return false;
  }

  // Get GM information for initialization
  SGmData* pTheGmList = (SGmData*)(pTheMsg->Msgs[0].Data);
  for( int i = 0; i < iGmCount; i++ )
  {
    m_listGmData.push_back( *(pTheGmList + i ) );
  }
	SGmData		GM;
	GM.iAuthority = GM_ADMIN;
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( GM.szAccount, "WILDCAT", MAX_ACCOUNT_LEN );
#else
	strcpy( GM.szAccount, "WILDCAT" );
#endif
	m_listGmData.push_back( GM );
  g_pGs->ReleaseMsg( pTheMsg );
  pTheMsg = NULL;
  return true;
}
//=========================================================================================================
//
//
char * CGsData::ShowGMList()
{
  static char               szGMList[10000];
  char                      szTemp[256];
  list<SGmData>::iterator   iter;
  BOOL                      bHave = FALSE;

  _snprintf( szGMList, 10000-1, "GM List :\n    " );
  szGMList[10000-1] = '\0';
  for( iter = m_listGmData.begin(); iter != m_listGmData.end(); iter++ )
  {
    _snprintf( szTemp, 256-1, "Account : %s;  ", (*iter).szAccount );
    szTemp[256-1] = '\0';
    if( (*iter).iAuthority == 1 )
    {
      strcat( szTemp, "Online GM\n    " );
    }
    else if( (*iter).iAuthority == 2 )
    {
      strcat( szTemp, "GM Leader\n    " );
    }
    else if( (*iter).iAuthority > 2 )
    {
      strcat( szTemp, "GM Admin\n    " );
    }
    strcat( szGMList, szTemp );
    bHave = TRUE;
  }
  if( bHave )
  {
    return szGMList;
  }
  return NULL;
}
//=========================================================================================================
//
//
BOOL CGsData::ParseGMInstruction( char * szInstr, HWND hDlg, int iErrorId, int iItemId )
{
  char    szFirstWord[256], szTemp[1024], szLog[2048];
  int     iValue[6];

  if( 1 != sscanf( szInstr, "%s", szFirstWord ) )
	{
    SetDlgItemText( hDlg, iErrorId, "Instruction Fromat Is Error, The Frist Word must Be A Keyword #" );
		return false;
	}
  // Parse GM Instructions
 	if( !strcmp( szFirstWord, "SetGMAuthority" ) )
	{
		if( 3 != sscanf( szInstr, "%s %s %d", szFirstWord, szTemp, &iValue[0] ) )
		{
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
			return false;
		}
		if( iValue[0] > 3 )
		{
      SetDlgItemText( hDlg, iErrorId, "Input Authority Is Error #" );
			return false;
		}
    //SGmData     *pGM;
    // Get GM Data
    //if( NULL != ( pGM = GetGMByAccount( szTemp ) ) )
    //{
    //  pGM->iAuthority = iValue[0];
    //  SetDlgItemText( hDlg, iItemId, "Set GM Authority Succeed #" );
		//  return true;
    //}
    //SetDlgItemText( hDlg, iErrorId, "Cannot Find The GM #" );
    CPlayer   *pPlayer = g_pGs->GetPlayerByAccount( szTemp );
    if( pPlayer )
    {
      SMsgData    *pNewMsg;
      if( NULL == ( pNewMsg = g_pGs->NewMsgBuffer( pPlayer->GetSelfCode() ) ) )
      {
        return false;
      }
      pPlayer->SetPrivilege( (WORD)iValue[0] );
      //
      pNewMsg->Init();
      pNewMsg->dwAID        = A_INSTRUCTIONS;
      pNewMsg->dwMsgLen     = 1;
      pNewMsg->Msgs[0].Size = sizeof( SNMGMInstruction );
      //
      SNMGMInstruction      *pInstr = (SNMGMInstruction*)(pNewMsg->Msgs[0].Data);

      pInstr->wGMInstCode = GM_INST_CHGAUTHORITY;
      pInstr->wValue[0]   = iValue[0];
#ifdef _REPAIR_SERVER_CRASH_NICK_
#ifdef _JAPAN_VERSION_
      SafeStrcpy( pInstr->szName, szTemp, MAX_ACCOUNT_LEN );
#else
      SafeStrcpy( pInstr->szName, szTemp, MAX_PLAYER_NAME_LEN );
#endif
			
#else
      strcpy( pInstr->szName, szTemp );
#endif
      //
      pPlayer->AddSendMsg( pNewMsg );
      SetDlgItemText( hDlg, iItemId, "Set GM Authority OK #" );
    }
    else
    {
      SMccMsgData			*pNewMccMsg = g_pGs->NewMccMsgBuffer();
      if( pNewMccMsg == NULL )		return false;
      
      SChangeGMPri			*pChangeGM = (SChangeGMPri*)(pNewMccMsg->Msgs[0].Data);
      
      pNewMccMsg->Init( NULL );
      pNewMccMsg->dwAID    = AP_CHANGEGMPRI;
      pNewMccMsg->dwMsgLen = 1;
      pNewMccMsg->Msgs[0].Size = sizeof( SChangeGMPri );
      
      pChangeGM->dwMailId  = 0;
      pChangeGM->wPri      = iValue[0];
#ifdef _REPAIR_SERVER_CRASH_NICK_
			SafeStrcpy( pChangeGM->szAccount, szTemp, MAX_ACCOUNT_LEN );
#else
      strcpy( pChangeGM->szAccount, szTemp );
#endif

      g_pMccDB->AddSendMsg( pNewMccMsg );
      SetDlgItemText( hDlg, iItemId, "Set GM Authority To Mcc OK #" );
    }
    return true;
	}
  // Warp Some Player To Speciafy Map X Y
  else if( !strcmp( szFirstWord, "Warp" ) )
  {
		if( 5 != sscanf( szInstr, "%s %s %d %d %d", szFirstWord, szTemp, &iValue[0], &iValue[1], &iValue[2] ) )
		{
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
			return false;
		}
		if( iValue[0] < 1 || iValue[1] < 1 || iValue[2] < 1 )
		{
      SetDlgItemText( hDlg, iErrorId, "Input Value Is Overflow #" );
			return false;
		}
		CGameMap	*pTheMap = NULL;
		if( NULL == ( pTheMap = g_pGs->GetGameMap( iValue[0] ) ) )
		{
      SetDlgItemText( hDlg, iErrorId, "Cannot Find The Input Map In This Server #" );
			return false;
		}
		if( iValue[1] >= pTheMap->GetSizeX() || iValue[1] <= 0 )
		{
      SetDlgItemText( hDlg, iErrorId, "Input X Coordinate Is Overflow #" );
			return false;	
		}
		if( iValue[2] >= pTheMap->GetSizeY() || iValue[2] <= 0 )
		{
      SetDlgItemText( hDlg, iErrorId, "Input Y Coordinate Is Overflow #" );
			return false;
		}
		// Send the Player Warp message to GM
		CPlayer  *pThePlayer = NULL;
    if( ( pThePlayer = g_pGs->FindPlayerFromName( szTemp ) ) == NULL )
		{
      SetDlgItemText( hDlg, iErrorId, "Cannot Find The Input Player In This Server #" );
			return false;
		}
		
	  SWarpPoint			TheWarp;

	  TheWarp.wTargetMapId = iValue[0];
		TheWarp.wTargetMapX   = iValue[1];
		TheWarp.wTargetMapY   = iValue[2];
		pThePlayer->Send_A_WARP( TheWarp, PLAYER_WARP_TYPE_MAP );

    _snprintf( szLog, 2048-1, "Warp %s To Map(Id=%d) Tile(%d,%d) #", szTemp, iValue[0], iValue[1], iValue[2] ); 
    szLog[2048-1] = '\0';
    SetDlgItemText( hDlg, iItemId, szLog );
    return true;
  }
  else if( !strcmp( szFirstWord, "KickGM" ) )
  {
    if( 2 != sscanf( szInstr, "%s %s", szFirstWord, szTemp ) )
		{
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
			return false;	
		}
    if( KickGM( szTemp ) )
    {
      SetDlgItemText( hDlg, iItemId, "Kick The GM Succeed #" );
      return TRUE;
    }
    else
    {
      SetDlgItemText( hDlg, iErrorId, "The GM Is Not On This Server #" );
      return false;
    }
    return true;
  }
  else if( !strcmp( szFirstWord, "SetMapRunType" ) )
  {
    if( 3 != sscanf( szInstr, "%s %d %d", szFirstWord, &iValue[0], &iValue[1] ) )
    {
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
      return false;
    }
    if( iValue[1] < 1 || iValue[1] >= MAP_RUNTYPE_MAX )
    {
      SetDlgItemText( hDlg, iErrorId, "The Map Run Type Is Overflow #" );
			return false;
    }
    CGameMap	*pTheMap = NULL;
		if( NULL == ( pTheMap = g_pGs->GetGameMap( iValue[0] ) ) )
		{
      SetDlgItemText( hDlg, iErrorId, "Cannot Find The Input Map In This Server #" );
			return false;
		}
    pTheMap->SetRunType( iValue[1] );
    if( iValue[1] == MAP_RUNTYPE_NORMAL )
    {
      _snprintf( szLog, 2048-1, "Map[%s](%d) Run Type = Normal #", pTheMap->GetName(), iValue[0] );
      szLog[2048-1] = '\0';
    }
    else if( iValue[1] == MAP_RUNTYPE_MOD2 )
    {
      _snprintf( szLog, 2048-1, "Map[%s](%d) Run Type = Mod2 #", pTheMap->GetName(), iValue[0] );
      szLog[2048-1] = '\0';
    }
    else if( iValue[1] == MAP_RUNTYPE_MOD3 )
    {
      _snprintf( szLog, 2048-1, "Map[%s](%d) Run Type = Mod3 #", pTheMap->GetName(), iValue[0] );
      szLog[2048-1] = '\0';
    }
    else if( iValue[1] == MAP_RUNTYPE_MOD4 )
    {
      _snprintf( szLog, 2048-1, "Map[%s](%d) Run Type = Mod4 #", pTheMap->GetName(), iValue[0] );
      szLog[2048-1] = '\0';
    }
    SetDlgItemText( hDlg, iItemId, szLog );
    return true;
  }
  else if( !strcmp( szFirstWord, "SetTime" ) )
  {
    if( 2 != sscanf( szInstr, "%s %d", szFirstWord, &iValue[0] ) )
    {
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
      return false;
    }
    if( iValue[0] < 0 || iValue[0] > 23 )
    {
      SetDlgItemText( hDlg, iErrorId, "The Server Time Overflow #" );
			return false;
    }
    SMsgData    *pNewMsg = NewMsgBuffer();
    pNewMsg->Init();
    pNewMsg->dwAID = AP_GETTIME;
    pNewMsg->dwMsgLen = 1;
    pNewMsg->Msgs[0].Size = sizeof( sGTChange );

    sGTChange			*pTimeChange = (sGTChange*)(pNewMsg->Msgs[0].Data);
    pTimeChange->sGTime.wHour = iValue[0];

    SetGameTime( pNewMsg );
    //g_pGs->SendAllPlayerServerTime();
    _snprintf( szLog, 2048-1, "Server Time Change OK, Now Time = %d #", iValue[0] );
    szLog[2048-1] = '\0';
    SetDlgItemText( hDlg, iItemId, szLog );
    return true;
  }
  else if( !strcmp( szFirstWord, "TalkToAll" ) )
  {
    if( 2 != sscanf( szInstr, "%s %s", szFirstWord, szTemp ) )
    {
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
      return false;
    }

    SMsgData    *pNewMsg = NewMsgBuffer();
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
			pNewMsg = NULL;
    }
  }
//#ifdef _DEBUG_CHECK_ACTION_STATE_
//  else if( !strcmp( szFirstWord, "SAA" ) )
//  {
//    if( 2 != sscanf( szInstr, "%s %s", szFirstWord, szTemp ) )
//    {
//      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
//      return false;
//    }
//    if( strlen( szTemp ) >= MAX_ACCOUNT_LEN )
//    {
//      SetDlgItemText( hDlg, iItemId, "The Account Is Overflow #" );
//      return false;
//    }
//    strcpy( szActionCode, szTemp );
//    sprintf( szLog, "Set Action Account OK, Now Account = %s #", szTemp );
//    SetDlgItemText( hDlg, iItemId, szLog );
//    return true;
//  }
//  else if( !strcmp( szFirstWord, "CAA" ) )
//  {
//    strcpy( szActionCode, "" );
//    SetDlgItemText( hDlg, iItemId, "Clear Action Account OK #" );
//    return true;
//  }
//#endif
  else if( !strcmp( szFirstWord, "CheckCltState" ) )
  {
    if( 2 != sscanf( szInstr, "%s %s", szFirstWord, szTemp ) )
    {
      SetDlgItemText( hDlg, iErrorId, "Scan The Instruction Error #" );
      return false;
    }
    if( strlen( szTemp ) >= MAX_ACCOUNT_LEN )
    {
      SetDlgItemText( hDlg, iItemId, "The Account Is Overflow #" );
      return false;
    }
    CPlayer     *pThePlayer = NULL;
    if( NULL != ( pThePlayer = g_pGs->GetPlayerByAccount( szTemp ) ) )
    {
      _snprintf( szTemp, 1024-1, "Player Name=%s;\nClient State=%d;\nPlayer State=%d;\nRecv Mess Num=%d;\nSend Mess Num=%d;",
               pThePlayer->GetPlayerName(), pThePlayer->GetClientState(),
               pThePlayer->GetStatus(), (pThePlayer->GetClient())->GetRecvMessCount(),
               (pThePlayer->GetClient())->GetSendMessCount() );
      szTemp[1024-1] = '\0';
      SetDlgItemText( hDlg, iItemId, szTemp );
    }
    else
    {
      SetDlgItemText( hDlg, iErrorId, "Cannot Find The Player" );
    }
    return true;
  }
  //// Reinitialize All Monster In Some Map
  //else if( !strcmp( szFirstWord, "ReinitAllMonster" ) )
  //{
  //  return true;
  //}
  //// Reintialize Monster That Specialfy Id In Some Map
  //else if( !strcmp( szFirstWord, "ReinitMonster" ) )
  //{
  //  return true;
  //}
  //// Close All Monster In Some Map
  //else if( !strcmp( szFirstWord, "CloseAllMonster" ) )
  //{
  //  return true;
  //}
  //// Close Monster That Speciaffy Id In Some Map
  //else if( !strcmp( szFirstWord, "CloseMonster" ) )
  //{
  //  return true;
  //}
  //// Open All Monster In Some Map
  //else if( !strcmp( szFirstWord, "OpenAllMonster" ) )
  //{
  //  return true;
  //}
  //// Open Monster That Speciaffy Id In Some Map
  //else if( !strcmp( szFirstWord, "OpenMonster" ) )
  //{
  //  return true;
  //}
  // Kick GM From GM List

  SetDlgItemText( hDlg, iErrorId, "The Instruction Is Error, There Is Not The Instruction #" );
  return false;
}
//=========================================================================================================
//
//
char * CGsData::ShowAllGMInstructions()
{
  static char       szAllInstr[10000];

#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szAllInstr, "  SetGMAuthority  'GM_Account'  'GM_Authority'\n  ", 10000 );
#else
  strcpy( szAllInstr, "  SetGMAuthority  'GM_Account'  'GM_Authority'\n  " );
#endif
  strcat( szAllInstr, "KickGM  'GM_Account'\n  " );
  strcat( szAllInstr, "ReinitMessage\n  " );
  strcat( szAllInstr, "SetMapRunType  'MapId'  'RunType'\n  " );
  strcat( szAllInstr, "SetTime 'GameTime'\n  " );
  //strcat( szAllInstr, "ReinitAllMonster  'MapId'\n  " );
  //strcat( szAllInstr, "ReinitMonster  'MapId'  'MonsterId'\n  " );
  //strcat( szAllInstr, "CloseAllMonster  'MapId'\n  " );
  //strcat( szAllInstr, "OpenAllMonster  'MapId'\n  " );
  //strcat( szAllInstr, "CloseMonster  'MapId'  'MonsterId'\n  " );
  //strcat( szAllInstr, "OpenMonster  'MapId'  'MonsterId'\n  " );
  strcat( szAllInstr, "Warp  'PlayerName'  'MapId'  'X'  'Y'\n  " );
  strcat( szAllInstr, "SAA  'PlayerAccount'\n  " );
  strcat( szAllInstr, "CAA  'PlayerAccount'\n  " );

  return szAllInstr;
}
//=========================================================================================================
//
// Map Obj Operation
int CGsData::InitGameMap(SMsgData* pTheMapMsg)
{
  FuncName("CGsData::InitGameMap");

  SGsInitMapData    *pMapData    = NULL;
  CGameMap          *pTheGameMap = NULL;
  int               iMapNum = 0, iRecvCount = 0;
  int               iMapTotalFromMcc = pTheMapMsg->dwMsgID;

  // Clear And Fill Data
  //listAllMapData.clear();

  iRecvCount   = pTheMapMsg->Msgs[0].Size / sizeof(SGsInitMapData);
  m_iMapTotal += iRecvCount;
  if( (sizeof(SGsInitMapData) * iRecvCount) != pTheMapMsg->Msgs[0].Size )
  {
    AddMemoErrMsg("Data size error !#");
    g_pGs->ReleaseMsg( pTheMapMsg );
    return 0;
  }
  pMapData = (SGsInitMapData*)(pTheMapMsg->Msgs[0].Data);
  // Get All Gs-Map Data
  for( int i = 0; i < iRecvCount; i++ )
  {
    pMapData->szGsName[MAX_GAME_SERVER_NAME-1] = '\0';
    pMapData->szServerIp[15] = '\0';
    listAllMapData.push_back( *pMapData );
    pMapData++;
  }
  // Check Data Size
  if( pTheMapMsg->dwMsgLen == 2 )
  {
    iRecvCount   = pTheMapMsg->Msgs[1].Size / sizeof(SGsInitMapData);
    m_iMapTotal += iRecvCount;
    if( (sizeof(SGsInitMapData) * iRecvCount) != pTheMapMsg->Msgs[1].Size )
    {
      AddMemoErrMsg("Data size error !#");
      g_pGs->ReleaseMsg( pTheMapMsg );
      return 0;
    }
    pMapData = (SGsInitMapData*)(pTheMapMsg->Msgs[1].Data);
    // Get All Gs-Map Data
    for( int i = 0; i < iRecvCount; i++ )
    {
      pMapData->szGsName[MAX_GAME_SERVER_NAME-1] = '\0';
      pMapData->szServerIp[15] = '\0';
      listAllMapData.push_back( *pMapData );
      pMapData++;
    }
  }
  if( m_iMapTotal < iMapTotalFromMcc )
  {
    g_pGs->ReleaseMsg( pTheMapMsg );
    return -1;
  }

  //
  // Init the Map in the Game Server
  // 做每张地图相关数据的初始化
  for( list<SGsInitMapData>::iterator iter = listAllMapData.begin(); iter != listAllMapData.end(); iter++ )
  {
//	  SetProcessWorkingSetSize(GetCurrentProcess(),-1,-1) ;
    if( strcmp( iter->szGsName, m_szServerName ) == 0 )
    {
      //if( m_mapGameMap.end() != m_mapGameMap.find( DWORD(iter->iMapId) ) )
      //{
      //  continue;
      //}
      //
      m_iServerId = iter->wServerId;
      pTheGameMap = new CGameMap();
			if( NULL == pTheGameMap )				 return -1;
			//

			_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Server Initialize Map %d From Mcc Message ! *****", iter->iMapId );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
			AddMemoMsg( g_szGsDataLog );
///////////////////////////////////////////////////////////////////////////////////
// 		char* _s_ =g_szGsDataLog ;
// 	  if( g_bLogChecked )
// 	  { 
// 		  EnterCriticalSection(&g_cs_MsgLock); 
// 		  dwThreadID=::GetCurrentThreadId();
// 		  if(dwThreadID==g_dwMCCThreadID)
// 			  szMsg=_szMsgMCC_;
// 		  else if(dwThreadID==g_dwClientThreadID)
// 			  szMsg=_szMsgClient_;
// 		  else 
// 			  szMsg=_szMsgMain_;
// 		  if(strlen(_s_) >= MAX_MEMO_MSG_LEN) 
// 		  {
// 			  sprintf(szMsg, "**[%s] Too long msg length=%d in Line=%d of %s", _szFName_, strlen(_s_), __LINE__, __FILE__); 
// 			  if (g_bLogChecked) 
// 				  g_pShowMessage->ShowError(szMsg);
// 			  SrvErrLog.Write(szMsg); 
// 		  }
//           else
// 		  { 
// 			  sprintf(szMsg, "[%s] %s", _szFName_, _s_); 
// 			  if (g_bLogChecked) 
// 				  g_pShowMessage->Show(szMsg); 
// 			  SrvLog.Write(szMsg); 
// 		  } 
// 		  LeaveCriticalSection(&g_cs_MsgLock);
// 	  }
//////////////////////////////////////////////////////////////////////
      //
      if( !pTheGameMap->Init( iter->iMapId ) )
      {
        delete pTheGameMap;
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
				_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Initialize Map %d *****", iter->iMapId );
        g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
				AddMemoErrMsg(g_szGsDataLog);
#endif
        // Release g_mapGameMap
        // ...
        //return 0;
      }
      else if( m_mapGameMap.end() != m_mapGameMap.find( DWORD(iter->iMapId) ) )
      {
        delete pTheGameMap;
        // Release g_mapGameMap
        // ...
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Initialize Map %d, Because Already Init This Map *****", iter->iMapId );
        g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(g_szGsDataLog);
#endif
        g_pGs->ReleaseMsg( pTheMapMsg );
        return 0;
      }
      else
      {    
        m_mapGameMap.insert( map<DWORD, CGameMap*>::value_type( DWORD(iter->iMapId), pTheGameMap ) );
        m_iSelfMapCount++;
        iMapNum++;
      }
    }
  } 
// Erase The Map, Where It's Not On This Game Server
#ifdef _AUTO_ADD_WARP_POINT_
  map<int, _AddWarpPoint*>::iterator itero;
  _AddWarpPoint *pWarpPoint = NULL;
  for(itero=g_mapAddWarpPiont.begin(); itero!=g_mapAddWarpPiont.end();)
  {
    pWarpPoint = itero->second;
    if(g_pGs->GetGameMap(pWarpPoint->wPointParameter[0]) == NULL)
    {
      g_mapAddWarpPiont.erase(itero++);
      SAFE_DELETE(pWarpPoint);
    }
    else
    {
      ++itero;
    }
  }
#endif // _AUTO_ADD_WARP_POINT_
	_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Server Get %d Maps From Mcc Message ! *****", iMapNum );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
	AddMemoMsg(g_szGsDataLog);
  g_pGs->ReleaseMsg( pTheMapMsg );
  //
  if( m_iMapTotal >= iMapTotalFromMcc )       return iMapNum;
  else                                        return -1;
}
//=====================================================================================
//
//
bool CGsData::ResetRandomMonster()
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CGameMap::ResetMonsterInfo");
  char														szFileName[256];
	CRandomMonster                  *pRandomMst = NULL;
  int                             iCount = 0;
  DWORD                           dwMonsterId = 0, dwMapCount = 0, dwMapId[10];
  WORD                            wAddMap[10];
  CSrvBaseMonster                 *pBaseMonster = NULL;
  
  _snprintf( szFileName, 256-1, "%s//Monster//RandomMonster.txt", g_pBase->GetObjectFilePath() );
  szFileName[256-1] = '\0';
  CInStream                       RandomMonster( szFileName );
	if( RandomMonster.fail() || RandomMonster.GetFileSize() == 0)
	{
		_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** The File[RandomMonster.txt] Is NULL *****" );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoMsg( g_szGsDataLog );
		return true;
	}
  m_mapRandomMonster.clear();
  if ( !( RandomMonster >> iCount ) ) 
  {
    MessageBox( GetActiveWindow(), "Loading The RandomMonster.txt OF iCount Error", "Warning...", MB_OK );
    return false;
  }
  for ( int iLoop = 0; iLoop<iCount; iLoop++) 
  {
    ZeroMemory( dwMapId, sizeof( DWORD ) * 10  );
    if( !(RandomMonster >> dwMonsterId
                        >> dwMapCount
                        >> dwMapId[0]
                        >> dwMapId[1]
                        >> dwMapId[2]
                        >> dwMapId[3]
                        >> dwMapId[4]
                        >> dwMapId[5]
                        >> dwMapId[6]
                        >> dwMapId[7]
                        >> dwMapId[8]
                        >> dwMapId[9]
       ) )
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Scan Random Monster File Error 1 *****" );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szGsDataLog );
      return false;
    }
    for( int i = 0; i < 10; i++ )
    {
      wAddMap[i] = dwMapId[i];
    }
    pRandomMst = new CRandomMonster( dwMapCount, dwMonsterId, wAddMap );
    if( pRandomMst == NULL )
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot New RandomMonster Buffer *****" );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szGsDataLog );
      return false;
    }
    m_mapRandomMonster.insert( map<WORD,CRandomMonster*>::value_type( dwMonsterId, pRandomMst ) );
    // Set Base Monster Property
    if( NULL == ( pBaseMonster = g_pBase->GetBaseMonster( dwMonsterId ) ) )
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Find RandomMonster BaseData *****" );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg( g_szGsDataLog );
      return false;
    }
    pBaseMonster->m_iNpcProperty |= NPC_ATTRI_RANDOM_MAP;    
  }
  return true;
#else
//
  FuncName("CGameMap::ResetMonsterInfo");
	FILE														*File = NULL;
	char														szFileName[256];
	CRandomMonster                  *pRandomMst = NULL;
  int                             iCount = 0;
  DWORD                           dwMonsterId = 0, dwMapCount = 0, dwMapId[10];
  WORD                            wAddMap[10];
  CSrvBaseMonster                 *pBaseMonster = NULL;
  //
  _snprintf( szFileName, 256-1, "%s//Monster//RandomMonster.txt", g_pBase->GetObjectFilePath() );
  szFileName[256-1] = '\0';
	if( ( File = fopen( szFileName, "r" ) ) == NULL )
	{
		_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** The File[RandomMonster.txt] Is NULL *****" );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoMsg( g_szGsDataLog );
		return true;
	}
  //
  m_mapRandomMonster.clear();
  //
  if( 1 != fscanf( File, "%d", &iCount ) )
  {
    fclose( File );
    return true;
  }
  //
  for( int iLoop = 0; iLoop < iCount; iLoop++ )
  {
    ZeroMemory( dwMapId, sizeof( DWORD ) * 10 );
    if( 12 != fscanf( File, "%d %d %d %d %d"
                            "%d %d %d %d %d"
                            "%d %d",
                            &dwMonsterId, &dwMapCount,
                            &dwMapId[0], &dwMapId[1], &dwMapId[2], &dwMapId[3], &dwMapId[4], 
                            &dwMapId[5], &dwMapId[6], &dwMapId[7], &dwMapId[8], &dwMapId[9] ) )
    {
		  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Scan Random Monster File Error 1 *****" );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		  AddMemoMsg( g_szGsDataLog );
      fclose( File );
      return false;
    }
    //
    for( int i = 0; i < 10; i++ )
    {
      wAddMap[i] = dwMapId[i];
    }
    //
    pRandomMst = new CRandomMonster( dwMapCount, dwMonsterId, wAddMap );
    if( pRandomMst == NULL )
    {
		  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot New RandomMonster Buffer *****" );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		  AddMemoMsg( g_szGsDataLog );
      fclose( File );
      return false;
    }
    //
    m_mapRandomMonster.insert( map<WORD,CRandomMonster*>::value_type( dwMonsterId, pRandomMst ) );
    // Set Base Monster Property
    if( NULL == ( pBaseMonster = g_pBase->GetBaseMonster( dwMonsterId ) ) )
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Find RandomMonster BaseData *****" );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		  AddMemoMsg( g_szGsDataLog );
      fclose( File );
      return false;
    }
    pBaseMonster->m_iNpcProperty |= NPC_ATTRI_RANDOM_MAP;
  }
  fclose( File );
  return true;
#endif//_DEBUG_JAPAN_DECRYPT_
}
//=========================================================================================================
//
//
bool CGsData::ResetMapObj()
{
  CGameMap  *pTheMap = NULL;
  
  if( !ResetRandomMonster() )                 
  {
    MessageBox(GetActiveWindow(),"ResetRandomMonster() Error","Error",MB_OK);
    return false;
  }
  for(map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++)
  {
    pTheMap = iter->second;
    if( pTheMap )
    {
      if( !pTheMap->ResetNpcInfo() )
      {
        MessageBox(GetActiveWindow(),"ResetNpcInfo() Error","Error",MB_OK);
        return false;
      }
      if( !pTheMap->ResetMonsterInfo() )
      { 
        MessageBox(GetActiveWindow(),"ResetMonsterInfo() Error","Error",MB_OK);
        return false;
      }
      if( !pTheMap->RebindNpcAndMonster() )   
      {
        MessageBox(GetActiveWindow(),"RebindNpcAndMonster() Error","Error",MB_OK);
        return false;
      }
#ifdef _NEW_TRADITIONS_WEDDING_
      if( !pTheMap->ResetAwaneInfo() )   
      {
        MessageBox(GetActiveWindow(),"ResetAwaneInfo() Error","Error",MB_OK);
        return false;
      }
      if( !pTheMap->ResetMarryFateInfo() )
      {
        MessageBox(GetActiveWindow(),"ResetMarryFateInfo() Error","Error",MB_OK);
        return false;
      }
#endif
    }
  }
  return true;
}
//=========================================================================================================
//
//
void CGsData::DoMapAction()
{
#ifdef _DEBUG
  FuncName("CGsData::DoMapAction");
#endif
  CGameMap        *pTheMap = NULL;
  static DWORD    g_dwMapMod = 0;

#ifdef _DEBUG_WILDCAT_
	int			iLdd1 = 0, iLdd2 = 0, iLdd3 = 0;
#endif

  g_dwMapMod++;
  for(map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++)
  {
    pTheMap = iter->second;
    if( pTheMap )
    {
#ifdef _NEW_CITY_WAR_2005_
      pTheMap->DoIncreaseGiftItem();
#endif
      if( pTheMap->GetRunType() == MAP_RUNTYPE_NORMAL )
      {
			  // Check Weather 
			  //pTheMap->CheckMapAddInfo();
			  // Do Npc Action
#ifndef SRVD_NO_NPC_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  ENTERTIME(80);
					  g_dwNpc[iLdd1][0] = pTheMap->GetMapId();
				  }
#endif
#ifdef _AUTO_ADD_WARP_POINT_
          pTheMap->CheckWarpPiontTime();
#endif // _AUTO_ADD_WARP_POINT_
				  pTheMap->NpcDoAction();
				  pTheMap->GroundItemDoAction();
//////////////////////////////////////////
//Add by CECE 2004-04-08
#ifdef  EVILWEAPON_3_6_VERSION
          pTheMap->DoWarPointpAction();
#endif
//////////////////////////////////////////
#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  LEAVETIME(80);
					  g_dwNpc[iLdd1][1] = GETRECORD(80);
					  iLdd1++;
				  }
#endif
			  }
#endif
			// Do Monster Action
#ifndef SRVD_NO_MONSTER_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  g_dwMonster[iLdd2][0] = pTheMap->GetMapId();
					  ENTERTIME(81);
				  }
#endif
				  pTheMap->MonsterDoAction();
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  LEAVETIME(81);
					  g_dwMonster[iLdd2][1] = GETRECORD(81);	
					  iLdd2++;
				  }
#endif
			  }
#endif

#ifdef _DEBUG_WILDCAT_
				  if( iLdd3 < MAX_MAP_NUM )
				  {
					  g_dwPlayer[iLdd3][0] = pTheMap->GetMapId();
					  ENTERTIME(82);
				  }
#endif
			  // Do Magic Action
			  pTheMap->MagicDoAction();
			  // ...
        pTheMap->CityWarDoAction();
				//
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
				pTheMap->FerryDoAction();
#endif

#ifdef _NEW_TRADITIONS_WEDDING_
				pTheMap->AwaneDoAction();
#endif

        // Player Get Information
#ifdef _DEBUG
        pTheMap->PlayerGetNeighterhoodInfo();
#else
			  if( pTheMap->m_dwGetInfoTime < TimeGetTime() )
        {
				  pTheMap->PlayerGetNeighterhoodInfo();
				  pTheMap->m_dwGetInfoTime = TimeGetTime() + 900;
			  }
#endif
        //
#ifdef _DEBUG_WILDCAT_
			  if( iLdd3 < MAX_MAP_NUM )
			  {
				  LEAVETIME(82);
				  g_dwPlayer[iLdd3][1] = GETRECORD(82);
				  iLdd3++;
			  }
#endif
      }
      else if( pTheMap->GetRunType() == MAP_RUNTYPE_MOD2 && ( g_dwMapMod % 2 == 0 ) )
      {
        // Check Weather 
			  //pTheMap->CheckMapAddInfo();
			  // Do Npc Action
#ifndef SRVD_NO_NPC_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  ENTERTIME(80);
					  g_dwNpc[iLdd1][0] = pTheMap->GetMapId();
				  }
#endif

				  pTheMap->NpcDoAction();
				  pTheMap->GroundItemDoAction();

#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  LEAVETIME(80);
					  g_dwNpc[iLdd1][1] = GETRECORD(80);
					  iLdd1++;
				  }
#endif
			  }
#endif
			// Do Monster Action
#ifndef SRVD_NO_MONSTER_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  g_dwMonster[iLdd2][0] = pTheMap->GetMapId();
					  ENTERTIME(81);
				  }
#endif
				  pTheMap->MonsterDoAction();
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  LEAVETIME(81);
					  g_dwMonster[iLdd2][1] = GETRECORD(81);	
					  iLdd2++;
				  }
#endif
			  }
#endif

#ifdef _DEBUG_WILDCAT_
				  if( iLdd3 < MAX_MAP_NUM )
				  {
					  g_dwPlayer[iLdd3][0] = pTheMap->GetMapId();
					  ENTERTIME(82);
				  }
#endif
			  // Do Magic Action
			  pTheMap->MagicDoAction();
			  // ...
        pTheMap->CityWarDoAction();
				//
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
				pTheMap->FerryDoAction();
#endif
				// Player Get Information
#ifdef _DEBUG
        pTheMap->PlayerGetNeighterhoodInfo();
#else
			  if( pTheMap->m_dwGetInfoTime < TimeGetTime() )
        {
				  pTheMap->PlayerGetNeighterhoodInfo();
				  pTheMap->m_dwGetInfoTime = TimeGetTime() + 900;
			  }
#endif
#ifdef _DEBUG_WILDCAT_
			  if( iLdd3 < MAX_MAP_NUM )
			  {
				  LEAVETIME(82);
				  g_dwPlayer[iLdd3][1] = GETRECORD(82);
				  iLdd3++;
			  }
#endif
      }
      else if( pTheMap->GetRunType() == MAP_RUNTYPE_MOD3 && ( g_dwMapMod % 3 == 0 ) )
      {
        // Check Weather 
			  //pTheMap->CheckMapAddInfo();
			  // Do Npc Action
#ifndef SRVD_NO_NPC_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  ENTERTIME(80);
					  g_dwNpc[iLdd1][0] = pTheMap->GetMapId();
				  }
#endif

				  pTheMap->NpcDoAction();
				  pTheMap->GroundItemDoAction();

#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  LEAVETIME(80);
					  g_dwNpc[iLdd1][1] = GETRECORD(80);
					  iLdd1++;
				  }
#endif
			  }
#endif
			// Do Monster Action
#ifndef SRVD_NO_MONSTER_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  g_dwMonster[iLdd2][0] = pTheMap->GetMapId();
					  ENTERTIME(81);
				  }
#endif
				  pTheMap->MonsterDoAction();
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  LEAVETIME(81);
					  g_dwMonster[iLdd2][1] = GETRECORD(81);	
					  iLdd2++;
				  }
#endif
			  }
#endif

#ifdef _DEBUG_WILDCAT_
				  if( iLdd3 < MAX_MAP_NUM )
				  {
					  g_dwPlayer[iLdd3][0] = pTheMap->GetMapId();
					  ENTERTIME(82);
				  }
#endif
			  // Do Magic Action
			  pTheMap->MagicDoAction();
			  // ...
        pTheMap->CityWarDoAction();
				//
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
				pTheMap->FerryDoAction();
#endif
			  // Player Get Information
#ifdef _DEBUG
        pTheMap->PlayerGetNeighterhoodInfo();
#else
			  if( pTheMap->m_dwGetInfoTime < TimeGetTime() )
        {
				  pTheMap->PlayerGetNeighterhoodInfo();
				  pTheMap->m_dwGetInfoTime = TimeGetTime() + 900;
			  }
#endif
#ifdef _DEBUG_WILDCAT_
			  if( iLdd3 < MAX_MAP_NUM )
			  {
				  LEAVETIME(82);
				  g_dwPlayer[iLdd3][1] = GETRECORD(82);
				  iLdd3++;
			  }
#endif
      }
      else if( pTheMap->GetRunType() == MAP_RUNTYPE_MOD4 && ( g_dwMapMod % 4 == 0 ) )
      {
        // Check Weather 
			  //pTheMap->CheckMapAddInfo();
			  // Do Npc Action
#ifndef SRVD_NO_NPC_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  ENTERTIME(80);
					  g_dwNpc[iLdd1][0] = pTheMap->GetMapId();
				  }
#endif

				  pTheMap->NpcDoAction();
				  pTheMap->GroundItemDoAction();

#ifdef _DEBUG_WILDCAT_
				  if( iLdd1 < MAX_MAP_NUM )
				  {
					  LEAVETIME(80);
					  g_dwNpc[iLdd1][1] = GETRECORD(80);
					  iLdd1++;
				  }
#endif
			  }
#endif
			// Do Monster Action
#ifndef SRVD_NO_MONSTER_ACTION
			  {
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  g_dwMonster[iLdd2][0] = pTheMap->GetMapId();
					  ENTERTIME(81);
				  }
#endif
				  pTheMap->MonsterDoAction();
#ifdef _DEBUG_WILDCAT_
				  if( iLdd2 < MAX_MAP_NUM )
				  {
					  LEAVETIME(81);
					  g_dwMonster[iLdd2][1] = GETRECORD(81);	
					  iLdd2++;
				  }
#endif
			  }
#endif

#ifdef _DEBUG_WILDCAT_
				  if( iLdd3 < MAX_MAP_NUM )
				  {
					  g_dwPlayer[iLdd3][0] = pTheMap->GetMapId();
					  ENTERTIME(82);
				  }
#endif
			  // Do Magic Action
			  pTheMap->MagicDoAction();
			  // ...
        pTheMap->CityWarDoAction();
				//
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
				pTheMap->FerryDoAction();
#endif
			  // Player Get Information
#ifdef _DEBUG
        pTheMap->PlayerGetNeighterhoodInfo();
#else
			  if( pTheMap->m_dwGetInfoTime < TimeGetTime() )
        {
				  pTheMap->PlayerGetNeighterhoodInfo();
				  pTheMap->m_dwGetInfoTime = TimeGetTime() + 900;
			  }
#endif
#ifdef _DEBUG_WILDCAT_
			  if( iLdd3 < MAX_MAP_NUM )
			  {
				  LEAVETIME(82);
				  g_dwPlayer[iLdd3][1] = GETRECORD(82);
				  iLdd3++;
			  }
#endif
      }

#ifdef _DEBUG_RECORD_MAP_PLAYER_COUNT_
      static DWORD s_dwTimeInvalid = ClientTickCount + 60 * 60 * 1000; // 1 hour
      if( s_dwTimeInvalid < ClientTickCount )
      {
        s_dwTimeInvalid = ClientTickCount + 60 * 60 * 1000;
        _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, " Map(%d) Count=%d ", pTheMap->GetMapId(), pTheMap->GetPlayerCount() );
        g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
        g_MapPlayerCountLog.Write( g_szGsDataLog );
      }
#endif

    }
    else
    {
#ifdef _DEBUG
      AddMemoErrMsg("pTheMap = NULL !#");
#endif
    }
  }
	return;
}
//=========================================================================================================
//
//
void CGsData::CheckAllMapCodePlayer()
{
  
}
//=========================================================================================================
//
//
bool CGsData::AddPlayer(CPlayer* pThePlayer)
{
  DWORD			  dwNewCode = 0, dwNewMailId = 0;
  SMsgData		*pTheMsg  = NULL;
  string      szInsertName = pThePlayer->GetName();
  map<DWORD, CPlayer*>::iterator iter;

  // Add New Player Into Game By Code And Mail Id
  dwNewCode   = pThePlayer->GetSelfCode();
  dwNewMailId = pThePlayer->GetMailId();
  {
    // Check Game Code List And Mail Id List
    if( m_mapCodePlayer.end() != m_mapCodePlayer.find( dwNewCode ) )
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Cannot Add Player %s With The Code = %d, Because Already Active(%d) #", pThePlayer->GetPlayerName(), dwNewCode, m_mapCodePlayer.size() );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddErrLogOnly( g_szGsDataLog );
      return false;
    }
    else if( m_mapMailPlayer.end() != ( iter = m_mapMailPlayer.find( dwNewMailId )  ) )
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "Cannot Add Player %s With The Mail Id = %d, Because Already Active(%d) #", pThePlayer->GetPlayerName(), dwNewMailId, m_mapMailPlayer.size() );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddErrLogOnly( g_szGsDataLog );
      m_mapMailPlayer.erase( iter );
      return false;
    }
    // Add Player Into Code List And Mail Id List
    m_mapCodePlayer.insert( map<DWORD, CPlayer*>::value_type( dwNewCode,   pThePlayer ) );
    m_mapMailPlayer.insert( map<DWORD, CPlayer*>::value_type( dwNewMailId, pThePlayer ) );
    m_mapNamePlayer.insert( map<string, CPlayer*>::value_type( szInsertName, pThePlayer ) );
    //
    pThePlayer->SetLoadChar();
    pThePlayer->SetState( STATUS_PLAYER_STAND );
    g_PlayerOnlineCount++;
    if( m_mapCodePlayer.size() > m_iMaxHistoryPlayer )      m_iMaxHistoryPlayer = m_mapCodePlayer.size();
    pThePlayer->PlayerLoginGuild();
    return true;
  }
}
//#define _DEBUG_SHOW_PLAYER_LOGOUT_FROM_GS_MAP_
//=========================================================================================================
//
//
bool CGsData::DelPlayer(CPlayer * pThePlayer)
{
	static map<DWORD, CPlayer*>::iterator		  Iter_C;
	static map<DWORD, CPlayer*>::iterator		  Iter_M;
  static map<string,CPlayer*>::iterator     Iter_N;
	static CTeam                              *pLogoutTeam;
  // Release The Player From The mapCodePlayer
  Iter_C = m_mapCodePlayer.find( pThePlayer->GetSelfCode() );
  if( Iter_C != m_mapCodePlayer.end() )
  {
    m_mapCodePlayer.erase( Iter_C );
  }
  else
  {
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "===>>> Cannot Find The Code[%d] Player[%s][%d] When Del From Gs...",
             pThePlayer->GetSelfCode(), pThePlayer->GetName(), pThePlayer->GetMailId() );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    g_PlayerLog.Write( g_szGsDataLog );
  }
  // Del Whisper
  g_pWhispers->DelWhisper( pThePlayer );

	// Release Player Code
	pThePlayer->ClearLoadChar();
  pThePlayer->ClearListenWhisper();
  // Clear Player Team, If Teamer == 0 After This Player Logout
  if( pLogoutTeam = ::GetTeam( pThePlayer->GetTeamId() ) )
  {
    if( pLogoutTeam->ClearTeamerForLogoutTheGs( pThePlayer->GetMailId() ) == -1 )
    {
      g_pGs->DeleteATeam( pLogoutTeam->GetTeamID() );
    }
  }
  // About Player Guild
  pThePlayer->PlayerLogoutFromGuild();
  // Release The Player From The mapMailPlayer
  Iter_M = m_mapMailPlayer.find( pThePlayer->GetMailId() );
  if( Iter_M != m_mapMailPlayer.end() )
  {
    m_mapMailPlayer.erase( Iter_M );
  }
  // Release The Player From The mapNamePlayer
  string      szEraseName = pThePlayer->GetName();
  //
  Iter_N = m_mapNamePlayer.find( szEraseName );
  if( Iter_N != m_mapNamePlayer.end() )
  {
    m_mapNamePlayer.erase( Iter_N );
  }
	// Delete Player From Map Tile
	if( pThePlayer->GetJoinMap() )
	{
    if( pThePlayer->GetInMap() && pThePlayer->GetAddInMap() == 1 )
    {
      pThePlayer->GetInMap()->DelPlayer( pThePlayer->GetSelfCode() );
    }
    else
    {
		  map<DWORD, CGameMap*>::iterator       Iter_Map;
      CGameMap                              *pTheMap = NULL;
      //
		  for( Iter_Map = m_mapGameMap.begin(); Iter_Map != m_mapGameMap.end(); Iter_Map++ )
		  {
        pTheMap = Iter_Map->second;
        if( pTheMap->GetPlayer( pThePlayer->GetSelfCode() ) )
        {
          pTheMap->DelPlayer( pThePlayer->GetSelfCode() );
        }
		  }
    }
	}
  g_PlayerOnlineCount--;
  return true;
}
//=========================================================================================================
//
//
CPlayer * CGsData::FindPlayerFromName( char * szPlayerName )
{
  map<string,CPlayer*>::iterator      Iter_N;

  szPlayerName[MAX_PLAYER_NAME_LEN-1] = '\0';
  Iter_N = m_mapNamePlayer.find( string(szPlayerName) );
  if( Iter_N != m_mapNamePlayer.end() )
  {
    if( (Iter_N->second)->IsInGame() )  return (CPlayer*)(Iter_N->second);
    else                                return NULL;
  }
  return NULL;
	//if( strlen( szPlayerName ) >= MAX_PLAYER_NAME_LEN )
	//{
	//	return NULL;
	//}
	//map<DWORD,CPlayer*>::iterator		iter;
	//for( iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
	//{
	//	if( !strcmp( (iter->second)->GetPlayerName(), szPlayerName ) )
  //  {
  //    if( (iter->second)->IsInGame() )    return (CPlayer*)(iter->second);
  //    else                                return NULL;
  //  }
	//}
	//return NULL;
}
//=========================================================================================================
//
//
CPlayer * CGsData::FindPlayerFromNameAll( char * szPlayerName )
{
  map<string,CPlayer*>::iterator      Iter_N;

  szPlayerName[MAX_PLAYER_NAME_LEN-1] = '\0';
  Iter_N = m_mapNamePlayer.find( string(szPlayerName) );
  if( Iter_N != m_mapNamePlayer.end() )
  {
    return (CPlayer*)(Iter_N->second);
  }
  return NULL;

	//if( strlen( szPlayerName ) >= MAX_PLAYER_NAME_LEN )
	//{
	//	return NULL;
	//}
	//map<DWORD,CPlayer*>::iterator		iter;
	//for( iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
	//{
	//	if( !strcmp( (iter->second)->GetPlayerName(), szPlayerName ) )
  //  {
  //    return (CPlayer*)(iter->second);
  //  }
	//}
	//return NULL;
}
//=========================================================================================================
//
//
char* CGsData::GetServerNameByMapId(DWORD dwTheMapId)
{
  static list<SGsInitMapData>::iterator Iter_GSN;
  for( Iter_GSN = listAllMapData.begin(); Iter_GSN != listAllMapData.end(); Iter_GSN++ )
  {
    if( dwTheMapId == Iter_GSN->iMapId )
    {
      return Iter_GSN->szGsName;
    }
  }
  return NULL;
}
//=========================================================================================================
//
//
BOOL CGsData::KickGM( char * szAccount )
{
  static list<SGmData>::iterator  Iter_GM;
  for( Iter_GM = m_listGmData.begin(); Iter_GM != m_listGmData.end(); Iter_GM++ )
	{
		if( !strcmp( (*Iter_GM).szAccount, szAccount ) )
    {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGsData::KickGM ===>> 111" );
#endif
      m_listGmData.erase( Iter_GM );
      return TRUE;
    }
	}
	return FALSE;
}
//=========================================================================================================
//
//
int	CGsData::GetGMAuthority( char * szAccount )
{
  static list<SGmData>::iterator   Iter_GM;
	for( Iter_GM = m_listGmData.begin(); Iter_GM != m_listGmData.end(); Iter_GM++ )
	{
		if( !strcmp( (*Iter_GM).szAccount, szAccount ) )
			return (*Iter_GM).iAuthority;
	}
	return -1;
}
//=========================================================================================================
//
//
SGmData * CGsData::GetGMByAccount( char * szAccount )
{
  list<SGmData>::iterator Iter_GM;
	for( Iter_GM = m_listGmData.begin(); Iter_GM != m_listGmData.end(); Iter_GM++ )
	{
		if( !strcmp( (*Iter_GM).szAccount, szAccount ) )
		{
			return (SGmData*)( &(*Iter_GM) );
		}
	}
	return NULL;
}
//=========================================================================================================
//
//Note:
SGameTime * CGsData::GetGameTime()
{
	return m_pGameTime->GetTime();
}
//=========================================================================================================
//
//
SNMMapInfo2Filter * CGsData::GetAllMapInfo( int & iSize )
{
	SNMMapInfo2Filter			*pMapInfo = NULL;
	CGameMap							*pTheMap  = NULL;
	int										iCount    = 0;

	// Get Map Number
	iSize = m_mapGameMap.size();
	// Get All Map Information
	pMapInfo = new SNMMapInfo2Filter[iSize];
	if( pMapInfo == NULL )
	{
		iSize = 0;
		return NULL;
	}
	for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
	{
		if( NULL != ( pTheMap = (CGameMap*)(iter->second) ) )
		{
			(pMapInfo+iCount)->dwMapId = pTheMap->GetMapId();
			(pMapInfo+iCount)->dwX_Y   = MAKELONG( pTheMap->GetClientSizeY(), pTheMap->GetClientSizeX() );
			iCount++;
		}
	}
	iSize = iCount;
	return pMapInfo;
}
//=========================================================================================================
//
//Note: Release one player's MCC buffer,CLIENT buffer
void CGsData::ReleaseOnePlayerBuffer(const WORD & wCode)
{
	static list<SMsgData*>::iterator      iter_Clt;
	static list<SMccMsgData*>::iterator   iter_Mcc;
  SMsgData                              *pMsg;
  SMccMsgData                           *pMccMsg;

  if( wCode >= CODE_MIN_PLAYER && wCode <= CODE_MIN_PLAYER + GetMaxPlayer() )
  {
	  EnterCriticalSection( &m_cs_PlayerMsgBuffer[wCode - CODE_MIN_PLAYER] );
	  for( iter_Clt  = m_ListPlayerMsgBuffer[wCode - CODE_MIN_PLAYER].begin();
         iter_Clt != m_ListPlayerMsgBuffer[wCode - CODE_MIN_PLAYER].end(); ++iter_Clt )
    {
      pMsg = (*iter_Clt);
      if( pMsg->bRelease > 0 )
      {
        pMsg->bRelease = 0;
        g_SysMsgBufferUsed--;
      }
    }
	  LeaveCriticalSection( &m_cs_PlayerMsgBuffer[wCode - CODE_MIN_PLAYER] );
	  EnterCriticalSection( &m_cs_PlayerMccMsgBuffer[wCode - CODE_MIN_PLAYER] );
	  for( iter_Mcc  = m_ListPlayerMccMsgBuffer[wCode - CODE_MIN_PLAYER].begin();
         iter_Mcc != m_ListPlayerMccMsgBuffer[wCode - CODE_MIN_PLAYER].end(); ++iter_Mcc )
    {
      pMccMsg = *iter_Mcc;
      if( pMccMsg->bRelease > 0 )
      {
        pMccMsg->bRelease = 0;
        g_MccMsgBufferUsed--;
      }
    }
	  LeaveCriticalSection( &m_cs_PlayerMccMsgBuffer[wCode - CODE_MIN_PLAYER] );
  }
}
//=========================================================================================================
//
// Add one
void CGsData::AddRef( SMsgData *pSysMsg, const WORD & wCount)
{
  if( pSysMsg == NULL )         return;

  if( pSysMsg->IsReleased() )   return;
  //
  if( pSysMsg->GetCode() != SYS_MSG_BUFFER_CODE &&
      pSysMsg->GetCode() != MCC_MSG_BUFFER_CODE )
  {
    return;
  }
  pSysMsg->bRelease += wCount;
}
//=========================================================================================================
//
//Note: Please do not call 'delete' for release it
inline SMsgData * CGsData::NewMsgBuffer( const WORD & Code )
{
	static const WORD		wSysMsgNum  = m_iSystemMessNum * GetMaxPlayer();
  static const WORD   wMccRecvMsgNum = m_iMccRecvMessNum * GetMaxPlayer();
#ifdef _DEBUG
  FuncName("CGsData::NewMsgBuffer");
#endif

	if( ( Code < CODE_MIN_PLAYER || Code > CODE_MIN_PLAYER + GetMaxPlayer() ) && Code != SYS_MSG_BUFFER_CODE && Code != MCC_MSG_BUFFER_CODE )
	{
#ifdef _DEBUG
		_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Code(%d) Want To New Buffer, But Overflow ! *****", Code );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoErrMsg( g_szGsDataLog );
#endif
		return NULL;
	}

	if( Code == SYS_MSG_BUFFER_CODE )
	{
  	SMsgData	*pNewMsg = NULL;
		EnterCriticalSection( &m_cs_SysMsgBuffer );
		for( int i = 0; i < wSysMsgNum; i++ )
		{
			pNewMsg = (SMsgData*)( *m_ListSysMsgBuffer.begin() );
			m_ListSysMsgBuffer.pop_front();
			m_ListSysMsgBuffer.push_back( pNewMsg );
			if( pNewMsg->bRelease == 0 )
			{
        if( pNewMsg->wPCode != SYS_MSG_BUFFER_CODE )
        {
          LeaveCriticalSection( &m_cs_SysMsgBuffer );
#ifdef _DEBUG
          _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Sys Mess Buffer!(AID=%d)******",pNewMsg->dwAID);
          g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
          g_PlayerLog.Write( g_szGsDataLog );
#endif
          return NULL;
        }
				pNewMsg->bRelease++;
        g_SysMsgBufferUsed++;
				LeaveCriticalSection( &m_cs_SysMsgBuffer );
				return pNewMsg;
			}
		}
#ifdef _DEBUG_HAVE_MESS_ASHCAN_
		LeaveCriticalSection( &m_cs_SysMsgBuffer );
    pNewMsg = new SMsgData;
    if( pNewMsg )
    {
      pNewMsg->bRelease = 1;
      pNewMsg->wPCode   = SYS_MSG_BUFFER_DECODE;
    }
    return pNewMsg;
#else
		LeaveCriticalSection( &m_cs_SysMsgBuffer );
#endif
	}
	else if( Code == MCC_MSG_BUFFER_CODE )
  {
  	SMsgData	  *pNewMsg = NULL;
		EnterCriticalSection( &m_cs_RecvMccMsgBuffer );
		for( int i = 0; i < wMccRecvMsgNum; i++ )
		{
			pNewMsg = (SMsgData*)( *m_ListRecvMccBuffer.begin() );
			m_ListRecvMccBuffer.pop_front();
			m_ListRecvMccBuffer.push_back( pNewMsg );
			if( pNewMsg->bRelease == 0 )
			{
        if( pNewMsg->wPCode != MCC_MSG_BUFFER_CODE )
        {
#ifdef _DEBUG
          _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Mcc Mess Buffer!(AID=%d)******",pNewMsg->dwAID);
          g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
          g_PlayerLog.Write( g_szGsDataLog );
#endif
		      LeaveCriticalSection( &m_cs_RecvMccMsgBuffer );
          return NULL;
        }
				pNewMsg->bRelease++;
        g_SysMsgBufferUsed++;
		    LeaveCriticalSection( &m_cs_RecvMccMsgBuffer );
				return pNewMsg;
			}
		}
#ifdef _DEBUG_HAVE_MESS_ASHCAN_
		LeaveCriticalSection( &m_cs_RecvMccMsgBuffer );
    pNewMsg = new SMsgData;
    if( pNewMsg )
    {
      pNewMsg->bRelease = 1;
      pNewMsg->wPCode   = SYS_MSG_BUFFER_DECODE;
    }
    return pNewMsg;
#else
    LeaveCriticalSection( &m_cs_RecvMccMsgBuffer );
#endif
  }
  else
	{
  	SMsgData	*pNewMsg = NULL;
		EnterCriticalSection( &m_cs_PlayerMsgBuffer[Code - CODE_MIN_PLAYER] );
		for( int i = 0; i < m_iPlayerMessNum; i++ )
		{
			pNewMsg = (SMsgData*)( *m_ListPlayerMsgBuffer[Code - CODE_MIN_PLAYER].begin() );
			m_ListPlayerMsgBuffer[Code - CODE_MIN_PLAYER].pop_front();
			m_ListPlayerMsgBuffer[Code - CODE_MIN_PLAYER].push_back( pNewMsg );
			if( pNewMsg->bRelease == 0 )
			{
        if( pNewMsg->wPCode != Code )
        {
				  LeaveCriticalSection( &m_cs_PlayerMsgBuffer[Code - CODE_MIN_PLAYER] );
#ifdef _DEBUG
          _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Player Mess Buffer!(AID=%d)******",pNewMsg->dwAID);
          g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
          g_PlayerLog.Write( g_szGsDataLog );
#endif
          return NULL;
        }
				pNewMsg->bRelease++;
        g_SysMsgBufferUsed++;
  		  LeaveCriticalSection( &m_cs_PlayerMsgBuffer[Code - CODE_MIN_PLAYER] );
				return pNewMsg;
			}
		}
		LeaveCriticalSection( &m_cs_PlayerMsgBuffer[Code - CODE_MIN_PLAYER] );
    //
		EnterCriticalSection( &m_cs_SysMsgBuffer );
		for( i = 0; i < wSysMsgNum; i++ )
		{
			pNewMsg = (SMsgData*)( *m_ListSysMsgBuffer.begin() );
			m_ListSysMsgBuffer.pop_front();
			m_ListSysMsgBuffer.push_back( pNewMsg );
			if( pNewMsg->bRelease == 0 )
			{
        if( pNewMsg->wPCode != SYS_MSG_BUFFER_CODE )
        {
          LeaveCriticalSection( &m_cs_SysMsgBuffer );
#ifdef _DEBUG
          _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Sys Mess Buffer!(AID=%d)******",pNewMsg->dwAID);
          g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
          g_PlayerLog.Write( g_szGsDataLog );
#endif
          return NULL;
        }
				pNewMsg->bRelease++;
        g_SysMsgBufferUsed++;
				LeaveCriticalSection( &m_cs_SysMsgBuffer );
				return pNewMsg;
			}
		}
    LeaveCriticalSection( &m_cs_SysMsgBuffer );
    //
#ifdef _DEBUG_HAVE_MESS_ASHCAN_
    pNewMsg = new SMsgData;
    if( pNewMsg )
    {
      pNewMsg->bRelease = 1;
      pNewMsg->wPCode   = SYS_MSG_BUFFER_DECODE;
    }
    return pNewMsg;
#endif
	}
  //
#ifdef _DEBUG
  if( Code >= CODE_MIN_PLAYER && Code <= CODE_MAX_PLAYER )
  {
    SMsgData	  *pNewMsg = NULL;
    CPlayer     *pPlayer = GetPlayerFromCode(Code);
    if( pPlayer )
    {
		  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** %s : Lose Buffer -- ******", pPlayer->GetPlayerAccount() );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      char      szTemp[32];
      EnterCriticalSection( &m_cs_PlayerMsgBuffer[Code - CODE_MIN_PLAYER] );
    
      for( list<SMsgData*>::iterator iter = m_ListPlayerMsgBuffer[Code - CODE_MIN_PLAYER].begin();
           iter != m_ListPlayerMsgBuffer[Code - CODE_MIN_PLAYER].end(); iter++ )
      {
				if( NULL != (*iter) )
				{
					_snprintf( szTemp, 32-1, "%d; ", (*iter)->dwAID );
          szTemp[32-1] = '\0';
					strcat( g_szGsDataLog, szTemp );
				}
      }
      LeaveCriticalSection( &m_cs_PlayerMsgBuffer[Code - CODE_MIN_PLAYER] );
    }
    g_PlayerLog.Write( g_szGsDataLog );
  }
#endif
  return NULL;
}
//=========================================================================================================
//
//Note: Before you release the SMsgData, do not call ::Init() or zero it
inline void CGsData::ReleaseMsg( SMsgData * pTheMsg )
{
  static const SMsgData      *g_MyBufEnd = m_MsgBuffer + GetMaxMsgBuffer();

  if( pTheMsg == NULL )                                 return;
  if( pTheMsg < m_MsgBuffer || pTheMsg > g_MyBufEnd )
  {
#ifdef _DEBUG_RELEASE_MESS_ASHCAN_
    if( pTheMsg->wPCode == SYS_MSG_BUFFER_DECODE )
    {
      EnterCriticalSection( &m_cs_SysAshcan );
      m_ListSysMsgAshcan.push_back( pTheMsg );
      LeaveCriticalSection( &m_cs_SysAshcan );
      return;
    }
    SAFE_DELETE( pTheMsg );
#endif
    return;
  }

  if( pTheMsg->wPCode == SYS_MSG_BUFFER_CODE )
  {
		EnterCriticalSection( &m_cs_SysMsgBuffer );
    if( pTheMsg->bRelease > 0 )
    {
      pTheMsg->bRelease--;
      if( pTheMsg->bRelease == 0 )    g_SysMsgBufferUsed--;
    }
    LeaveCriticalSection( &m_cs_SysMsgBuffer );
  }
  else if( pTheMsg->wPCode == MCC_MSG_BUFFER_CODE )
  {
		EnterCriticalSection( &m_cs_RecvMccMsgBuffer );
    if( pTheMsg->bRelease > 0 )
    {
      pTheMsg->bRelease--;
      if( pTheMsg->bRelease == 0 )    g_SysMsgBufferUsed--;
    }
    LeaveCriticalSection( &m_cs_RecvMccMsgBuffer );
  }
  else
  {
		EnterCriticalSection( &m_cs_PlayerMsgBuffer[pTheMsg->wPCode - CODE_MIN_PLAYER] );
    if( pTheMsg->bRelease > 0 )
    {
      pTheMsg->bRelease--;
      if( pTheMsg->bRelease == 0 )    g_SysMsgBufferUsed--;
    }
    LeaveCriticalSection( &m_cs_PlayerMsgBuffer[pTheMsg->wPCode - CODE_MIN_PLAYER] );
  }
}
//=========================================================================================================
//
//
SMccMsgData * CGsData::NewMccMsgBuffer( const WORD & Code )
{
	static const WORD		wSysMccMsgNum  = m_iSystemMccMessNum * GetMaxPlayer();

#ifdef _DEBUG
	FuncName("CGsData::NewMccMsgBuffer");
#endif

	if( ( Code < CODE_MIN_PLAYER || Code > CODE_MIN_PLAYER + GetMaxPlayer() ) && Code != 0 )
	{
#ifdef _DEBUG
		_snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Code(%d) Want To New Mcc Buffer, But Overflow ! *****", Code );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
		AddMemoErrMsg( g_szGsDataLog );
#endif
		return NULL;
	}

	if( Code == 0 )
	{
	  SMccMsgData     *pNewMccMsg;
		EnterCriticalSection( &m_cs_SysMccMsgBuffer );
		for( int i = 0; i < wSysMccMsgNum; i++ )
		{
			pNewMccMsg = (SMccMsgData*)( *m_ListSysMccMsgBuffer.begin() );
			m_ListSysMccMsgBuffer.pop_front();
			m_ListSysMccMsgBuffer.push_back( pNewMccMsg );
			if( pNewMccMsg->bRelease == 0 )
			{
        if( pNewMccMsg->wPCode != 0 )
        {
#ifdef _DEBUG
          _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Sys Mcc Mess Buffer!(AID=%d)******",pNewMccMsg->dwAID);
          g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
          g_PlayerLog.Write( g_szGsDataLog );
#endif
				  LeaveCriticalSection( &m_cs_SysMccMsgBuffer );
				  return NULL;
        }
				g_MccMsgBufferUsed++;
				LeaveCriticalSection( &m_cs_SysMccMsgBuffer );
				pNewMccMsg->bRelease++;
				return pNewMccMsg;
			}
		}
#ifdef _DEBUG_HAVE_MCC_MESS_ASHCAN_
		LeaveCriticalSection( &m_cs_SysMccMsgBuffer );
    pNewMccMsg = new SMccMsgData;
    if( pNewMccMsg )
    {
      pNewMccMsg->wPCode   = MCC_MSG_BUFFER_DECODE;
      pNewMccMsg->bRelease = 1;
    }
    return pNewMccMsg;
#else
    LeaveCriticalSection( &m_cs_SysMccMsgBuffer );
#endif
	}
	else
	{
	  SMccMsgData   *pNewMccMsg;
		EnterCriticalSection( &m_cs_PlayerMccMsgBuffer[Code - CODE_MIN_PLAYER] );
		for( int i = 0; i < m_iPlayerMccMessNum; i++ )
		{
			pNewMccMsg = (SMccMsgData*)( *m_ListPlayerMccMsgBuffer[Code - CODE_MIN_PLAYER].begin() );
			m_ListPlayerMccMsgBuffer[Code - CODE_MIN_PLAYER].pop_front();
			m_ListPlayerMccMsgBuffer[Code - CODE_MIN_PLAYER].push_back( pNewMccMsg );
			if( pNewMccMsg->bRelease == 0 )
			{
        if( pNewMccMsg->wPCode != Code )
        {
#ifdef _DEBUG
          _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Player Mcc Mess Buffer!(AID=%d)******",pNewMccMsg->dwAID);
          g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
          g_PlayerLog.Write( g_szGsDataLog );
#endif
				  LeaveCriticalSection( &m_cs_PlayerMccMsgBuffer[Code - CODE_MIN_PLAYER] );
          return NULL;
        }
				pNewMccMsg->bRelease++;
				g_MccMsgBufferUsed++;
				LeaveCriticalSection( &m_cs_PlayerMccMsgBuffer[Code - CODE_MIN_PLAYER] );
				return pNewMccMsg;
			}
		}
#ifdef _DEBUG_HAVE_MCC_MESS_ASHCAN_
		LeaveCriticalSection( &m_cs_PlayerMccMsgBuffer[Code - CODE_MIN_PLAYER] );
    pNewMccMsg = new SMccMsgData;
    if( pNewMccMsg )
    {
      pNewMccMsg->wPCode   = MCC_MSG_BUFFER_DECODE;
      pNewMccMsg->bRelease = 1;
    }
    return pNewMccMsg;
#else
		LeaveCriticalSection( &m_cs_PlayerMccMsgBuffer[Code - CODE_MIN_PLAYER] );
#endif
	}
	return NULL;
}
//=========================================================================================================
//
//
void CGsData::ReleaseMccMsg( SMccMsgData * pTheMccMsg )
{
  static const SMccMsgData    *g_MyMccBufEnd = m_MccMsgBuffer + GetMaxMccMsgBuffer();

  if( pTheMccMsg == NULL )                      return;
  if( pTheMccMsg < m_MccMsgBuffer || pTheMccMsg > g_MyMccBufEnd )
  {
#ifdef _DEBUG_RELEASE_MESS_ASHCAN_
    pTheMccMsg->bRelease--;
    if( pTheMccMsg->wPCode == MCC_MSG_BUFFER_DECODE && pTheMccMsg->bRelease == 0 )
    {
      EnterCriticalSection( &m_cs_MccAshcan );
      m_ListMccMsgAshcan.push_back( pTheMccMsg );
      LeaveCriticalSection( &m_cs_MccAshcan );
      return;
    }
    SAFE_DELETE( pTheMccMsg );
#endif
    return;
  }
  //
  if( pTheMccMsg->wPCode == 0 )
  {
		EnterCriticalSection( &m_cs_SysMccMsgBuffer );
    if( pTheMccMsg->bRelease > 0 )
    {
		  pTheMccMsg->bRelease--;
      if( pTheMccMsg->bRelease == 0 )    g_MccMsgBufferUsed--;
    }
#ifdef _DEBUG
    else
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Sys Mcc Msg Is Already Released!(AID=%d)******",pTheMccMsg->dwAID );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      g_PlayerLog.Write( g_szGsDataLog );
    }
#endif
		LeaveCriticalSection( &m_cs_SysMccMsgBuffer );
  }
  else
  {
		EnterCriticalSection( &m_cs_PlayerMccMsgBuffer[pTheMccMsg->wPCode - CODE_MIN_PLAYER] );
    if( pTheMccMsg->bRelease > 0 )
    {
		  pTheMccMsg->bRelease--;
      if( pTheMccMsg->bRelease == 0 )    g_MccMsgBufferUsed--;
    }
#ifdef _DEBUG
    else
    {
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Player Mcc Msg Is Already Released!(AID=%d)******",pTheMccMsg->dwAID );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      g_PlayerLog.Write( g_szGsDataLog );
    }
#endif
		LeaveCriticalSection( &m_cs_PlayerMccMsgBuffer[pTheMccMsg->wPCode - CODE_MIN_PLAYER] );
  }
}
//=========================================================================================================
//
//
STimeOutMsg * CGsData::NewTimeoutMsg()
{
	STimeOutMsg * pNewTimeoutMsg;

	EnterCriticalSection( &m_cs_TimeoutBuffer );
	for( int i = 0; i < GetMaxMccMsgBuffer(); i++ )
	{
		pNewTimeoutMsg = (STimeOutMsg*)( *m_ListTimeoutMsgBuffer.begin() );
		m_ListTimeoutMsgBuffer.pop_front();
		m_ListTimeoutMsgBuffer.push_back( pNewTimeoutMsg );
		if( pNewTimeoutMsg->bRelease == TRUE )
		{
			pNewTimeoutMsg->bRelease = FALSE;
			g_TimeoutMsgBufferUsed++;
			LeaveCriticalSection( &m_cs_TimeoutBuffer );
			return pNewTimeoutMsg;
		}
	}
	LeaveCriticalSection( &m_cs_TimeoutBuffer );
	return NULL;
}
//=========================================================================================================
//
//
void CGsData::ReleaseTimeoutMsg( STimeOutMsg * pTheMsg )
{
#ifdef _DEBUG
  FuncName("CGsData::ReleaseMccMsg");
#endif

	EnterCriticalSection( &m_cs_TimeoutBuffer );
	if( pTheMsg->bRelease == FALSE )
	{
		g_TimeoutMsgBufferUsed--;
		pTheMsg->bRelease = TRUE;
	}
#ifdef _DEBUG
  else
  {
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "****** ERROR: Error Release Timeout Msg (AID=%d) !******",pTheMsg->m_dwAID );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(g_szGsDataLog);
  }
#endif
	LeaveCriticalSection( &m_cs_TimeoutBuffer );
}
//=========================================================================================================
//
//
int * CGsData::CheckLoseMsgAID( int & iSize)
{
	static int		g_iLoseMsgAID[80];
  BOOL          bAddLose = FALSE;
  DWORD         dwErrCode = 0;
  int           iAddPos = 0;

  memset( g_iLoseMsgAID, 0, sizeof( int ) * 80 );
	iSize = 0;
  //
	if( m_MsgBuffer )
	{
		for( int i = 0; i < g_pGs->GetMaxMsgBuffer(); i++ )
		{
			if( m_MsgBuffer[i].bRelease > 0 )
			{
        iAddPos  = -1;
        for( int j = 0; j < 20; j++ )
        {
				  if( g_iLoseMsgAID[(j*3)] == m_MsgBuffer[i].dwAID )
          {
						g_iLoseMsgAID[(j*3)+1] += 1;
            if( g_iLoseMsgAID[(j*3)] == A_ERROR )
            {
              dwErrCode = LOWORD( (*(DWORD*)m_MsgBuffer[i].Msgs[0].Data) );
              if( dwErrCode != g_iLoseMsgAID[(j*3)+2] )   g_iLoseMsgAID[(j*3)+2] |= ( dwErrCode << 16 );
            }
            else
            {
						  if( g_iLoseMsgAID[(j*3)+2] != m_MsgBuffer[i].wPCode )	g_iLoseMsgAID[(j*3)+2] = m_MsgBuffer[i].wPCode;
            }
            break;
          }
          else if( iAddPos == -1 && g_iLoseMsgAID[(j*3)] == 0 )
          {
            iAddPos = j;
          }
        }
        //
        if( iAddPos != -1 )
        {
          g_iLoseMsgAID[iAddPos*3]		 = m_MsgBuffer[i].dwAID;
          g_iLoseMsgAID[(iAddPos*3)+1] = 1;
          g_iLoseMsgAID[(iAddPos*3)+2] = m_MsgBuffer[i].wPCode;
          //
          if( g_iLoseMsgAID[(j*3)] == A_ERROR )
          {
            dwErrCode = LOWORD( (*(DWORD*)m_MsgBuffer[i].Msgs[0].Data) );
            g_iLoseMsgAID[(j*3)+2] = dwErrCode;
          }
					iSize++;
        }
        if( iSize == 20 ) break;
			}
		}
	}
	return g_iLoseMsgAID;
}
//=========================================================================================================
//
//
int * CGsData::CheckMccLoseMsgAID( int & iSize)
{
	static int		g_iLoseMccMsgAID[10000];
  DWORD         dwAllAID[5];

	iSize = 0;
	if( m_MsgBuffer )
	{
		for( int i = 0; i < g_pGs->GetMaxMccMsgBuffer(); i++ )
		{
			if( m_MccMsgBuffer[i].bRelease > 0  )
			{
        for( int j = 0; j < 20; j++ )
        {
				  if( dwAllAID[j] == m_MccMsgBuffer[i].dwAID )
          {
            break;
          }
          else if( dwAllAID[j] == 0 )
          {
            dwAllAID[j] = m_MccMsgBuffer[i].dwAID;
            g_iLoseMccMsgAID[iSize++] = dwAllAID[j];
          }
        }
        if( iSize == 5 ) break;
			}
		}
	}
	return g_iLoseMccMsgAID;
}
//=========================================================================================================
//
//
void CGsData::InitMsgBuffer()
{
#ifdef _DEBUG_WILDCAT_
	//FuncName("CGsData::InitMsgBuffer");
	//char	szTemp[MAX_LOGMSG_LEN];
#endif

	WORD			wCode = CODE_MIN_PLAYER, wPlayerCount = 0, wMsgCount = 0, i;

	//////////////////////////////////////////////////////////////////////////////
	//
	//
	// Initialize Every Player Message Buffer And Message Buffer's Criticla Section

	m_cs_PlayerMsgBuffer		= new CRITICAL_SECTION[GetMaxPlayer()];
	m_cs_PlayerMccMsgBuffer = new CRITICAL_SECTION[GetMaxPlayer()];
	m_ListPlayerMsgBuffer		= new list<SMsgData*>[GetMaxPlayer()];
	m_ListPlayerMccMsgBuffer= new list<SMccMsgData*>[GetMaxPlayer()];
  m_ListMccMsgAshcan.clear();
  m_ListSysMsgAshcan.clear();
	// Zero Both Message Buffer
	for( i = 0; i < GetMaxPlayer(); i++ )
	{
		InitializeCriticalSection( &m_cs_PlayerMsgBuffer[i] );
		InitializeCriticalSection( &m_cs_PlayerMccMsgBuffer[i] );
		m_ListPlayerMsgBuffer[i].clear();
		m_ListPlayerMccMsgBuffer[i].clear();
	}
	InitializeCriticalSection( &m_cs_SysMsgBuffer );
	InitializeCriticalSection( &m_cs_SysMccMsgBuffer );
	InitializeCriticalSection( &m_cs_RecvMccMsgBuffer );
	InitializeCriticalSection( &m_cs_TimeoutBuffer );
  InitializeCriticalSection( &m_cs_MccAshcan );
  InitializeCriticalSection( &m_cs_SysAshcan );
	//////////////////////////////////////////////////////////////////////////////
	//
	//
	// 初始化每个玩家用于与client通信的buffer
	m_MsgBuffer = (SMsgData*)new SMsgData[GetMaxMsgBuffer()];
	for( i = 0; i < m_iPlayerMessNum * GetMaxPlayer(); i++ )
	{
		m_MsgBuffer[i].Init();
		m_MsgBuffer[i].bRelease = 0;
    m_MsgBuffer[i].wPCode = wCode;
		m_ListPlayerMsgBuffer[wCode-CODE_MIN_PLAYER].push_back( &m_MsgBuffer[i] );
		if( ( ( i + 1 ) % GetPlayerMessNum() ) == 0 )		wCode++;
	}

	// 初始化系统用于与client通信的buffer
	m_ListSysMsgBuffer.clear();
	for( ; i < GetMaxPlayer() * (m_iPlayerMessNum + m_iSystemMessNum); i++ )
	{
		m_MsgBuffer[i].Init();
		m_MsgBuffer[i].bRelease = 0;
    m_MsgBuffer[i].wPCode   = SYS_MSG_BUFFER_CODE;
		m_ListSysMsgBuffer.push_back( &m_MsgBuffer[i] );
	}
  // 初始化 用于接收MCC消息的buffer
  m_ListRecvMccBuffer.clear();
	for( ; i < GetMaxMsgBuffer(); i++ )
	{
		m_MsgBuffer[i].Init();
		m_MsgBuffer[i].bRelease = 0;
    m_MsgBuffer[i].wPCode = MCC_MSG_BUFFER_CODE;
		m_ListRecvMccBuffer.push_back( &m_MsgBuffer[i] );
	}
	//////////////////////////////////////////////////////////////////////////////
	//
	//
	// 初始化每个玩家用于与mcc通信的buffer
	wCode = CODE_MIN_PLAYER;
	m_MccMsgBuffer = (SMccMsgData*)new SMccMsgData[GetMaxMccMsgBuffer()];
	for( i = 0; i < m_iPlayerMccMessNum * GetMaxPlayer(); i++ )
	{
		m_MccMsgBuffer[i].Init( g_pClientList->m_pClientArray[wCode-CODE_MIN_PLAYER].m_pPlayer );
    m_MccMsgBuffer[i].bRelease = 0;
    m_MccMsgBuffer[i].wPCode = wCode;
		m_ListPlayerMccMsgBuffer[wCode-CODE_MIN_PLAYER].push_back( &m_MccMsgBuffer[i] );
		if( ( ( i + 1 ) % GetPlayerMccMessNum() ) == 0 )		wCode++;
	}
	// 初始化系统用于与mcc通信的buffer
	m_ListSysMccMsgBuffer.clear();
	for( ; i < GetMaxMccMsgBuffer(); i++ )
	{
		m_MccMsgBuffer[i].Init( NULL );
    m_MccMsgBuffer[i].bRelease = 0;
    m_MccMsgBuffer[i].wPCode = 0;
		m_ListSysMccMsgBuffer.push_back( &m_MccMsgBuffer[i] );
	}

	//////////////////////////////////////////////////////////////////////////////
	//
	//
	// 初始化公用的 Timeout Message Buffer List
	m_TimeOutMsg = (STimeOutMsg*)new STimeOutMsg[ GetMaxMccMsgBuffer() ];
	m_ListTimeoutMsgBuffer.clear();
	for( i = 0; i < GetMaxMccMsgBuffer(); i++ )
	{
		m_ListTimeoutMsgBuffer.push_back( &m_TimeOutMsg[i] );
	}
}
#ifdef _DEBUG_RELEASE_MESS_ASHCAN_
//=========================================================================================================
//
//
void CGsData::ReleaseMsgAshcan()
{
  // Release Sys Ashcan Message
#ifdef _DEBUG_HAVE_MESS_ASHCAN_
  EnterCriticalSection( &m_cs_SysAshcan );
  if( !m_ListSysMsgAshcan.empty() )
  {
    list<SMsgData*>::iterator   Iter_Sys;
    SMsgData                    *pTheMsg;

    for( Iter_Sys = m_ListSysMsgAshcan.begin(); Iter_Sys != m_ListSysMsgAshcan.end(); Iter_Sys++ )
    {
      pTheMsg = (*Iter_Sys);
      SAFE_DELETE( pTheMsg );
    }
    m_ListSysMsgAshcan.clear();
  }
  LeaveCriticalSection( &m_cs_SysAshcan );
#endif
  // Release Mcc Ashcan Message
#ifdef _DEBUG_HAVE_MCC_MESS_ASHCAN_
  EnterCriticalSection( &m_cs_MccAshcan );
  if( !m_ListMccMsgAshcan.empty() )
  {
    list<SMccMsgData*>::iterator  Iter_Mcc;
    SMccMsgData                   *pTheMccMsg;
    for( Iter_Mcc = m_ListMccMsgAshcan.begin(); Iter_Mcc != m_ListMccMsgAshcan.end(); Iter_Mcc++ )
    {
      pTheMccMsg = (*Iter_Mcc);
      SAFE_DELETE( pTheMccMsg );
    }
    m_ListMccMsgAshcan.clear();
  }
  LeaveCriticalSection( &m_cs_MccAshcan );
#endif
}
#endif
//=========================================================================================================
//
//
void CGsData::ReleaseAllMsgBuffer()
{
	//for( int i = 0; i < g_pGs->GetMaxMsgBuffer(); i++ )
	//{
	//	free( &m_MsgBuffer[i] );
	//}
	//for( int j = 0; j < g_pGs->GetMaxMccMsgBuffer(); j++ )
	//{
	//	free( &m_MccMsgBuffer[j] );	
	//}
	if( m_ListPlayerMsgBuffer )
  {
    delete[] m_ListPlayerMsgBuffer;
    m_ListPlayerMsgBuffer = NULL;
  }
	if( m_ListPlayerMccMsgBuffer )
  {
    delete[] m_ListPlayerMccMsgBuffer;
    m_ListPlayerMccMsgBuffer = NULL;
  }
	if( m_MsgBuffer )
  {
    delete[] m_MsgBuffer;
    m_MsgBuffer = NULL;
  }
	if( m_MccMsgBuffer )
  {
    delete[] m_MccMsgBuffer;
    m_MccMsgBuffer = NULL;
  }
	if( m_TimeOutMsg )
	{
		delete[] m_TimeOutMsg;
	}
}
//=========================================================================================================
//
//
void CGsData::ReturnAllMessageBuffer()
{
}
//=========================================================================================================
//
//
void CGsData::ReInitMsgBuffer()
{
}
//=========================================================================================================
//
//
void CGsData::ReInitMccMsgBuffer()
{
}
//=========================================================================================================
//
//
void CGsData::ReleasePlayerCode(CPlayer* pThePlayer)
{
	if( pThePlayer->GetSelfCode() != 0 )		
	{
		m_codePlayer.ReleaseCode( pThePlayer->GetSelfCode() );
		//pThePlayer->SetCode(0);
	}
}
//=========================================================================================================
//
//
CTeam* CGsData::CreateATeam( WORD const & wTeamID )
{
  static map<WORD,CTeam*>::iterator Iter_Tm;

  Iter_Tm = m_mapAllTeam.find( wTeamID );
  if( Iter_Tm != m_mapAllTeam.end() )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateATeam1" );
    AddMemoErrMsg( "**** CreateATeam Error, The Team Is Already Exist ! ****" );
#endif
    return Iter_Tm->second;
  }
  CTeam   *pTeam = new CTeam;
  if( pTeam == NULL )
  { 
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateATeam1" );
    AddMemoErrMsg( "**** CreateATeam Error ****" );
#endif
    return NULL;
  }
  pTeam->SetTeamID( wTeamID );
  m_mapAllTeam[wTeamID] = pTeam;
  return pTeam;
}
//=========================================================================================================
//
//
CTeam* CGsData::CreateATeam( const DWORD & dwHolderMailID, const DWORD & dwJoinerMailID, const WORD & wTeamID)
{
  static map<WORD,CTeam*>::iterator Iter_Tm;

  Iter_Tm = m_mapAllTeam.find( wTeamID );
  if( Iter_Tm != m_mapAllTeam.end() )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateATeam2" );
    AddMemoErrMsg( "**** CreateATeam Error, The Team Is Already Exist ! ****" );
#endif
    return Iter_Tm->second;
  }
  CTeam   *pTeam = new CTeam( dwHolderMailID, dwJoinerMailID, wTeamID );
  if( pTeam == NULL )
  { 
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateATeam2" );
    AddMemoErrMsg( "**** CreateATeam Error, Memory Leave ! ****" );
#endif
    return NULL;
  }
  pTeam->SetTeamID( wTeamID );

  m_mapAllTeam[wTeamID] = pTeam;
  return pTeam;
}
//=========================================================================================================
//
//
void CGsData::DeleteAllTeam()
{
  static map<WORD,CTeam*>::iterator     Iter_Da;
  SMsgData                              *pNewMsg = NULL;
  CTeam                                 *pTeam;
  
  if( m_mapAllTeam.empty() )      return;

  for( Iter_Da = m_mapAllTeam.begin(); Iter_Da != m_mapAllTeam.end(); )
  {
    pTeam = Iter_Da->second;

    if( pNewMsg = NewMsgBuffer() )
    {
      pNewMsg->Init();
      pNewMsg->dwAID        = A_DELETETEAM;
      pNewMsg->dwMsgLen     = 1;
      pNewMsg->Msgs[0].Size = sizeof(WORD);

      *(WORD*)(pNewMsg->Msgs[0].Data) = pTeam->GetTeamID();
      //
      pTeam->SendTheMsgToMember( pNewMsg, NULL );
    }
    pTeam->ClearAllMemberData();
    //
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
    g_CrashLog.Write( "CGsData::DeleteAllTeam ===>> 111" );
#endif
    //
    Iter_Da = m_mapAllTeam.erase( Iter_Da );
    SAFE_DELETE( pTeam );
  }
}
//=========================================================================================================
//
//
void CGsData::DeleteATeam(const WORD & wTeamID)
{
  static map<WORD,CTeam*>::iterator     Iter_Dl;

  Iter_Dl = m_mapAllTeam.find( wTeamID );
  if( Iter_Dl != m_mapAllTeam.end() )
  {
    CTeam   *pTeam = Iter_Dl->second;
    pTeam->ClearAllMemberData();
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
    g_CrashLog.Write( "CGsData::DeleteATeam ===>> 111" );
#endif
    m_mapAllTeam.erase( Iter_Dl );
    SAFE_DELETE( pTeam );
  }
}
//=========================================================================================================
//
//
BOOL CGsData::CheckCheater( CPlayer * pPlayer, int iIntegral )
{
  // return TRUE;
#ifdef _DEBUG
  pPlayer->AddSendErrorMsg( ERROR_CODE_ADD_CHEATER_VALUE, 2 );
#endif
  //
	if( m_bCheckCheater == TRUE )
  {
    if( iIntegral == 0 )    iIntegral = 50;
	  pPlayer->AddCheaterIntegral( iIntegral );
    //
    if( pPlayer->GetCheaterIntegral() > MAX_PLAYER_CHEATER_INTEGRAL )
	  {
      // Send Message To Mcc For Add Cheater
      SMccMsgData     *pNewMccMsg = g_pGs->NewMccMsgBuffer();
      if( pNewMccMsg )
      {
        pNewMccMsg->Init( NULL );
        pNewMccMsg->dwAID        = AP_ADDCHEATER;
        pNewMccMsg->dwMsgLen     = 1;
        pNewMccMsg->Msgs[0].Size = sizeof( CCheaterData );
        //
        CCheaterData    *pCheater = (CCheaterData*)(pNewMccMsg->Msgs[0].Data);
        //
        pCheater->m_dwTime   = 14;
        pCheater->m_dwMailId = pPlayer->GetMailId();
#ifdef _REPAIR_SERVER_CRASH_NICK_
				SafeStrcpy( pCheater->m_szIP, (pPlayer->GetClient())->GetIp(), 16 );
				SafeStrcpy( pCheater->m_szAccount, pPlayer->GetAccount(), MAX_ACCOUNT_LEN );
#else
        strcpy( pCheater->m_szIP, (pPlayer->GetClient())->GetIp() );
        strcpy( pCheater->m_szAccount, pPlayer->GetAccount() );
#endif

        g_pMccDB->AddSendMsg( pNewMccMsg );
      }
      //
      pPlayer->AddSendErrorMsg( ERROR_CODE_CHEATER, 2 );
      pPlayer->SetClientState( CLIENTSTATE_LOGOUT );
      return TRUE;
    }
		return FALSE;
	}
  else if( iIntegral == 0 )
  {
    //
    pPlayer->AddCheaterIntegral( 50 );
    //
    if( pPlayer->GetCheaterIntegral() > MAX_PLAYER_CHEATER_INTEGRAL )
	  {
      // Send Message To Mcc For Add Cheater
      SMccMsgData     *pNewMccMsg = g_pGs->NewMccMsgBuffer();
      if( pNewMccMsg )
      {
        pNewMccMsg->Init( NULL );
        pNewMccMsg->dwAID        = AP_ADDCHEATER;
        pNewMccMsg->dwMsgLen     = 1;
        pNewMccMsg->Msgs[0].Size = sizeof( CCheaterData );
        //
        CCheaterData    *pCheater = (CCheaterData*)(pNewMccMsg->Msgs[0].Data);
        //
        pCheater->m_dwTime   = 14;
        pCheater->m_dwMailId = pPlayer->GetMailId();
        memcpy( pCheater->m_szIP, (pPlayer->GetClient())->GetIp(), 16 );
        memcpy( pCheater->m_szAccount, pPlayer->GetAccount(), MAX_ACCOUNT_LEN );
        pCheater->m_szIP[15]                     = '\0';
        pCheater->m_szAccount[MAX_ACCOUNT_LEN-1] = '\0';

        g_pMccDB->AddSendMsg( pNewMccMsg );
      }
      //
      pPlayer->AddSendErrorMsg( ERROR_CODE_CHEATER );
      pPlayer->SetClientState( CLIENTSTATE_LOGOUT );
      return TRUE;
    }
		return FALSE;
  }
	return FALSE;
}
//=========================================================================================================
//
//
void CGsData::AddCheaterByAll( const CCheaterData & TheCheater )
{
  //
#ifdef _DEBUG
  FuncName( "CGsData::AddCheater" );
  _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Set %s Is Cheater, Punish Time=%d(h) *****", TheCheater.m_szAccount, TheCheater.m_dwTime*24 );
  g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
  //
  if( g_pGs->GetState() == GSSTATE_GAME_START )
  {
    AddMemoErrMsg( g_szGsDataLog );
  }
  else
  {
    AddMemoMsg( g_szGsDataLog );
  }
#endif
  //
  CCheaterData		*pCheater = NULL;
  EnterCriticalSection( &m_cs_listCheater );
  for( list<CCheaterData*>::iterator iter = m_listCheater.begin(); iter != m_listCheater.end(); iter++ )
  {
    pCheater = (CCheaterData*)(*iter);
		if( !strcmp( pCheater->m_szAccount, TheCheater.m_szAccount ) )
		{
			pCheater->m_dwTime += TheCheater.m_dwTime;
      LeaveCriticalSection( &m_cs_listCheater );
      return;
		}
  }
	pCheater = new CCheaterData;
	if( NULL == pCheater )			return;
  //
  memcpy( pCheater, &TheCheater, sizeof( CCheaterData ) );

	//
	if( m_listCheater.size() > 500 )		m_listCheater.pop_back();
	m_listCheater.push_front( pCheater );
  LeaveCriticalSection( &m_cs_listCheater );
}
//=========================================================================================================
//
//
void CGsData::DelCheater( const char * szAcocunt )
{	
	CCheaterData		*pCheater = NULL;

  EnterCriticalSection( &m_cs_listCheater );
	for( list<CCheaterData*>::iterator iter = m_listCheater.begin(); iter != m_listCheater.end(); )
	{
		pCheater = (CCheaterData*)(*iter);
		if( !strcmp( pCheater->m_szAccount, szAcocunt ) )
		{
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGsData::DelCheater ===>> 111" );
#endif
			iter = m_listCheater.erase( iter );
      SAFE_DELETE( pCheater );
		}
    else
    {
      iter++;
    }
	}
  LeaveCriticalSection(&m_cs_listCheater);
}
//=========================================================================================================
//
//
CCheaterData * CGsData::GetCheaterFromIP( const char * szIp )
{
	CCheaterData		*pCheater = NULL;

  EnterCriticalSection(&m_cs_listCheater);
	for( list<CCheaterData*>::iterator iter = m_listCheater.begin(); iter != m_listCheater.end(); iter++ )
	{
		pCheater = (CCheaterData*)(*iter);
		if( !strcmp( szIp, pCheater->m_szIP ) )
		{
      LeaveCriticalSection(&m_cs_listCheater);
			return pCheater;
		}
	}
  LeaveCriticalSection(&m_cs_listCheater);
	return NULL;
}
//=========================================================================================================
//
//
CCheaterData * CGsData::GetCheaterFromAccount( char * szAccount )
{
	CCheaterData		*pCheater = NULL;

  EnterCriticalSection(&m_cs_listCheater);
	for( list<CCheaterData*>::iterator iter = m_listCheater.begin(); iter != m_listCheater.end(); iter++ )
	{
		pCheater = (CCheaterData*)(*iter);
		if( !strcmp( pCheater->m_szAccount, szAccount ) )
		{
      LeaveCriticalSection(&m_cs_listCheater);
			return pCheater;
		}
	}
  LeaveCriticalSection(&m_cs_listCheater);
	return NULL;
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::SendMcc_AP_CHEATERLIST()
{
  SMccMsgData     *pNewMccMsg = NewMccMsgBuffer();

  if( pNewMccMsg )
  {
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID    = AP_CHEATERLIST;
    pNewMccMsg->dwMsgLen = 0;
    g_pMccDB->AddSendMsg( pNewMccMsg );
  }
}
//------------------------------------------------------------------------------------------
// 
//
BOOL CGsData::RecvMcc_AP_CHEATERLIST( SMsgData *pTheMsg )
{
  CCheaterHeader        *pHeader   = (CCheaterHeader*)(pTheMsg->Msgs[0].Data);
  CCheaterData          *pCheater  = (CCheaterData*)(pTheMsg->Msgs[0].Data+sizeof(CCheaterHeader));
  WORD                  wMessCount = MAXMSGDATASIZE / sizeof( CCheaterData );

  if( pHeader->wCount != ( pTheMsg->Msgs[0].Size - sizeof( CCheaterHeader ) ) / sizeof( CCheaterData ) )
  {
    pHeader->wCount = ( pTheMsg->Msgs[0].Size - sizeof( CCheaterHeader ) ) / sizeof( CCheaterData );
  }
  //
  for( int i = 0; i < pHeader->wCount; i++ )
  {
    if( i == wMessCount )    pCheater = (CCheaterData*)(pTheMsg->Msgs[1].Data);
    AddCheaterByAll( *pCheater );
    pCheater++;
  }
  return pHeader->wStateFlag;
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::DisableAllMonster(const WORD & wId, BOOL bGMCreate)
{
	CGameMap				*pTheMap = NULL;

  for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
	{
		if( NULL != ( pTheMap = iter->second ) )
		{
			pTheMap->DisableAllMonster( wId, bGMCreate );
		}
	}
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::ReInitAllMonster()
{
	CGameMap				*pTheMap = NULL;

  for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
	{
		if( NULL != ( pTheMap = iter->second ) )
		{
			pTheMap->ReInitAllMonster();
		}
	}	
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::DisableMonsterById( int iId, int iMapId )
{
	CGameMap		*pTheMap = NULL;

	if( iMapId )
	{
		map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.find( (DWORD)iMapId );
		if( iter != m_mapGameMap.end() )
		{
			if( NULL != ( pTheMap = ( iter->second ) ) )
			{
				pTheMap->DisableMonsterById( iId );
			}
		}
	}
	else
	{
		for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pTheMap = iter->second ) )
			{
				pTheMap->DisableMonsterById( iId );
			}
		}			
	}
}
//add by zetorchen 
#ifdef _NEW_CITY_WAR_2005_
//------------------------------------------------------------------------------------------
// 
//
void CGsData::EnableAllCityWarMon( int iMapId )
{
  CGameMap *pTheMap = NULL;
  if( iMapId )
	{
		map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.find( (DWORD)iMapId );
		if( iter != m_mapGameMap.end() )
		{
			if( NULL != ( pTheMap = ( iter->second ) ) )
			{
				pTheMap->EnableCityWarMon();
			}
		}
	}
	else
	{
		for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pTheMap = iter->second ) )
			{
				pTheMap->EnableCityWarMon();
			}
		}			
	}
}
#endif // #endif _NEW_CITY_WAR_2005_
//------------------------------------------------------------------------------------------
// 
//
void CGsData::ReInitMonsterById(int iId, int iMapId)
{
	CGameMap		*pTheMap = NULL;

	if( iMapId )
	{
		map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.find( (DWORD)iMapId );
		if( iter != m_mapGameMap.end() )
		{
			if( NULL != ( pTheMap = ( iter->second ) ) )
			{
				pTheMap->ReInitMonsterById( iId );
			}
		}
	}
	else
	{
		for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pTheMap = iter->second ) )
			{
				pTheMap->ReInitMonsterById( iId );
			}
		}			
	}
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::DisableAllNpc()
{
	CGameMap				*pTheMap = NULL;

  for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
	{
		if( NULL != ( pTheMap = iter->second ) )
		{
			pTheMap->DisableAllNpc();
		}
	}	
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::ReInitAllNpc()
{
	CGameMap				*pTheMap = NULL;

  for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
	{
		if( NULL != ( pTheMap = iter->second ) )
		{
			pTheMap->ReInitAllNpc();
		}
	}	
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::DisableNpcById( int iId, int iMapId )
{
	CGameMap		*pTheMap = NULL;

	if( iMapId )
	{
		map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.find( (DWORD)iMapId );
		if( iter != m_mapGameMap.end() )
		{
			if( NULL != ( pTheMap = ( iter->second ) ) )
			{
				pTheMap->DisableNpcById( iId );
			}
		}
	}
	else
	{
		for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pTheMap = iter->second ) )
			{
				pTheMap->DisableNpcById( iId );
			}
		}			
	}
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::ReInitNpcById( int iId, int iMapId )
{
	CGameMap		*pTheMap = NULL;

	if( iMapId )
	{
		map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.find( (DWORD)iMapId );
		if( iter != m_mapGameMap.end() )
		{
			if( NULL != ( pTheMap = ( iter->second ) ) )
			{
				pTheMap->ReInitNpcById( iId );
			}
		}
	}
	else
	{
		for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
		{
			if( NULL != ( pTheMap = iter->second ) )
			{
				pTheMap->ReInitNpcById( iId );
			}
		}	
	}
}
//------------------------------------------------------------------------------------------
// 
//
int CGsData::BeParty( const SNMPartyData & PartyData, BOOL bGroundParty )
{
  CGameMap      *pTheMap;
  SMsgData      *pNewMsg;
  int           iTotal = 0;

  if( NULL != ( pNewMsg = g_pGs->NewMsgBuffer() ) )
  {
    pNewMsg->dwAID				= A_TALKTOALL;
    pNewMsg->dwMsgLen			= 1;
	  pNewMsg->Msgs[0].Size = strlen( PartyData.szBeginCon );
#ifdef _REPAIR_SERVER_CRASH_NICK_
		SafeStrcpy( pNewMsg->Msgs[0].Data, PartyData.szBeginCon, MAXMSGDATASIZE );
#else
	  strcpy( pNewMsg->Msgs[0].Data, PartyData.szBeginCon );
#endif

    g_pGs->SendTheMsgToAll( pNewMsg );
  }

  for( map<DWORD, CGameMap*>::iterator iter = m_mapGameMap.begin(); iter != m_mapGameMap.end(); iter++ )
  {
    if( NULL != ( pTheMap = (CGameMap*)(iter->second) ) ) 
    {
      iTotal += pTheMap->BeParty( PartyData, bGroundParty );
    }
  }

  if( NULL != ( pNewMsg = g_pGs->NewMsgBuffer() ) )
  {
    pNewMsg->dwAID				= A_TALKTOALL;
    pNewMsg->dwMsgLen			= 1;
	  pNewMsg->Msgs[0].Size = strlen( PartyData.szEndCon );
#ifdef _REPAIR_SERVER_CRASH_NICK_
		SafeStrcpy( pNewMsg->Msgs[0].Data, PartyData.szEndCon, MAXMSGDATASIZE );
#else
	  strcpy( pNewMsg->Msgs[0].Data, PartyData.szEndCon );
#endif

    g_pGs->SendTheMsgToAll( pNewMsg );
  }
  return iTotal;
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::SendAllPlayerServerTime()
{
  SGameTime						GameTime;
  SMsgData            *pNewMsg;

  m_pGameTime->GetTime( GameTime );
  if( NULL == ( pNewMsg = NewMsgBuffer() ) )
  {
    return;
  }
  pNewMsg->Init();
  SGameTime		      *SetTime  = (SGameTime*)(pNewMsg->Msgs[0].Data);
  SetTime->wHour = GameTime.wHour;
  SetGameTime( pNewMsg );
}
//------------------------------------------------------------------------------------------
// 
//
//	WORD		              m_wPrivilege;
//  int                   m_iIpStart[4];
//  int                   m_iIpGate[4];
//	char		              m_szIpStart[16];
//  char                  m_szIpGate[16];
//	list<CGMBaseData*>    m_ListBaseData;
bool CGsData::CheckGMIp( CPlayer * pPlayer )
{
	static list<CGMIpData*>::iterator			Iter_GMIp;
	static CGMIpData*											pGMIpData;
  static int                            iIpCmp[4];

  iIpCmp[0] = pPlayer->GetIp(0);
  iIpCmp[1] = pPlayer->GetIp(1);
  iIpCmp[2] = pPlayer->GetIp(2);
  iIpCmp[3] = pPlayer->GetIp(3);

	for( Iter_GMIp = m_listGMIP.begin(); Iter_GMIp != m_listGMIP.end(); Iter_GMIp++ )
	{
		pGMIpData = (*Iter_GMIp);
		if( ( pGMIpData->m_iIpStart[0] & pGMIpData->m_iIpGate[0] ) == ( iIpCmp[0] & pGMIpData->m_iIpGate[0] ) &&
        ( pGMIpData->m_iIpStart[1] & pGMIpData->m_iIpGate[1] ) == ( iIpCmp[1] & pGMIpData->m_iIpGate[1] ) &&
        ( pGMIpData->m_iIpStart[2] & pGMIpData->m_iIpGate[2] ) == ( iIpCmp[2] & pGMIpData->m_iIpGate[2] ) &&
        ( pGMIpData->m_iIpStart[3] & pGMIpData->m_iIpGate[3] ) == ( iIpCmp[3] & pGMIpData->m_iIpGate[3] ) )
		{
			if( pGMIpData->m_wPrivilege <= pPlayer->GetGMPrivilege() )		return true;
		}
	}
	return false;
}
//------------------------------------------------------------------------------------------
// 
//
bool CGsData::CheckWhisperIp( CPlayer * pPlayer )
{
	static list<CGMIpData*>::iterator			Iter_GMIp;
	static CGMIpData*											pGMIpData;
  static int                            iIpCmp[4];

  iIpCmp[0] = pPlayer->GetIp(0);
  iIpCmp[1] = pPlayer->GetIp(1);
  iIpCmp[2] = pPlayer->GetIp(2);
  iIpCmp[3] = pPlayer->GetIp(3);

	for( Iter_GMIp = m_listWhisperIP.begin(); Iter_GMIp != m_listWhisperIP.end(); Iter_GMIp++ )
	{
		pGMIpData = (*Iter_GMIp);
		if( ( pGMIpData->m_iIpStart[0] & pGMIpData->m_iIpGate[0] ) == ( iIpCmp[0] & pGMIpData->m_iIpGate[0] ) &&
        ( pGMIpData->m_iIpStart[1] & pGMIpData->m_iIpGate[1] ) == ( iIpCmp[1] & pGMIpData->m_iIpGate[1] ) &&
        ( pGMIpData->m_iIpStart[2] & pGMIpData->m_iIpGate[2] ) == ( iIpCmp[2] & pGMIpData->m_iIpGate[2] ) &&
        ( pGMIpData->m_iIpStart[3] & pGMIpData->m_iIpGate[3] ) == ( iIpCmp[3] & pGMIpData->m_iIpGate[3] ) )
		{
			return true;
		}
	}
	return false;
}
//------------------------------------------------------------------------------------------
// 
//
bool CGsData::GetAllPlayerMailIdList( DWORD *pMailIdList )
{
  map<DWORD,CPlayer*>::iterator   Iter_M;

  for( Iter_M = m_mapMailPlayer.begin(); Iter_M != m_mapMailPlayer.end(); Iter_M++ )
  {
    *pMailIdList = Iter_M->first;
    pMailIdList++;
  }
  return true;
}
//------------------------------------------------------------------------------------------
// 
//
void CGsData::ClearGMIp( CPlayer * pPlayer )
{
	static list<CGMIpData*>::iterator			Iter_GMIp;
	static CGMIpData*											pGMIpData;
  static int                            iIpCmp[4];

  iIpCmp[0] = gf_GetIntegerFromChar( pPlayer->GetPlayerIP(), 1 );
  iIpCmp[1] = gf_GetIntegerFromChar( pPlayer->GetPlayerIP(), 2 );
  iIpCmp[2] = gf_GetIntegerFromChar( pPlayer->GetPlayerIP(), 3 );
  iIpCmp[3] = gf_GetIntegerFromChar( pPlayer->GetPlayerIP(), 4 );

	for( Iter_GMIp = m_listGMIP.begin(); Iter_GMIp != m_listGMIP.end(); Iter_GMIp++ )
	{
		pGMIpData = (*Iter_GMIp);
		if( ( pGMIpData->m_iIpStart[0] & pGMIpData->m_iIpGate[0] ) == ( iIpCmp[0] & pGMIpData->m_iIpGate[0] ) &&
        ( pGMIpData->m_iIpStart[1] & pGMIpData->m_iIpGate[1] ) == ( iIpCmp[1] & pGMIpData->m_iIpGate[1] ) &&
        ( pGMIpData->m_iIpStart[2] & pGMIpData->m_iIpGate[2] ) == ( iIpCmp[2] & pGMIpData->m_iIpGate[2] ) &&
        ( pGMIpData->m_iIpStart[3] & pGMIpData->m_iIpGate[3] ) == ( iIpCmp[3] & pGMIpData->m_iIpGate[3] ) )
		{
      // Clear All Account
      // ...
		}
	}
}
//------------------------------------------------------------------------------------------
// 
//
bool CGsData::CheckDataRelation()
{
  // Check Event And Npc
  // ...
  //g_pBase->CheckEventAndNpc
  // Check Npc And Item
  // ...
  // Check Npc And Skill
  // ...
  // Check Npc And Warp Point List
  // ...
  // Check Npc And Mix Item List
  // ...
  // Check Monster And Drop Item
  // ...
  // Check Monster And Skill
  // ...
  // Check ...
  return true;
}
//------------------------------------------------------------------------------------------
// 
//
void CGsWhisper::AddGM( CPlayer *pPlayer )
{
  if( pPlayer->GetGMPrivilege() > 1 )   m_ListGM.push_front( pPlayer );
  else                                  m_ListGM.push_back( pPlayer );
}
//------------------------------------------------------------------------------------------
// 
//
WORD CGsWhisper::GetOnlineGM( SNMOnlineGM *pOnlineGM )
{
  static list<CPlayer*>::iterator     Iter_On;
  WORD                                wCount = 0;

  if( m_ListGM.empty() )            return 0;
  for( Iter_On = m_ListGM.begin(); Iter_On != m_ListGM.end(); Iter_On++ )
  {
    memcpy( pOnlineGM->szName, (*Iter_On)->GetPlayerName(), MAX_PLAYER_NAME_LEN );
    pOnlineGM->szName[MAX_PLAYER_NAME_LEN-1] = '\0';
    pOnlineGM->dwMailId   = (*Iter_On)->GetMailId();
    pOnlineGM->wPrivilege = (*Iter_On)->GetGMPrivilege();
    pOnlineGM++;
    wCount++;
    if( wCount > 19 )               return wCount;
  }
  return wCount;
}
//------------------------------------------------------------------------------------------
// 
//
void CGsWhisper::SendWhisper( SMsgData *pTheMsg )
{
  static list<CPlayer*>::iterator     Iter_Send;
  CPlayer                             *pRecver  = NULL;

  if( m_ListWhisper.empty() )
  {
    SendOnlineGM( pTheMsg );
    return;
  }
  //
  int       iNum = 0;
  for( Iter_Send = m_ListWhisper.begin(); Iter_Send != m_ListWhisper.end(); Iter_Send++ )
  {
    if( NULL != ( pRecver = (*Iter_Send) ) )
    {
      g_pMultiSendPlayer[iNum] = pRecver;
      //pRecver->AddSendMsg( pTheMsg );
      iNum++;
    }
  }
  //
  if( iNum == 0 )       g_pGs->ReleaseMsg( pTheMsg );
  else
  {
    g_pGs->AddRef( pTheMsg, iNum-1 );
    for( int sss = 0; sss < iNum; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheMsg );
  }
}
//----------------------------------------------------------------------------------------
// 
//
inline void  CGsWhisper::SendOnlineGM( SMsgData *pTheMsg )
{
  static list<CPlayer*>::iterator     Iter_Send;
  CPlayer                             *pRecver  = NULL;
  int                                 iNum = 0;

  if( m_ListGM.empty() )
  {
    g_pGs->ReleaseMsg( pTheMsg );
    return;
  }
  //
  for( Iter_Send = m_ListGM.begin(); Iter_Send != m_ListGM.end(); Iter_Send++ )
  {
    if( NULL != ( pRecver = (*Iter_Send) ) )
    {
      g_pMultiSendPlayer[iNum] = pRecver;
      //pRecver->AddSendMsg( pTheMsg );
      iNum++;
    }
  }
  //
  if( iNum == 0 )       g_pGs->ReleaseMsg( pTheMsg );
  else
  {
    g_pGs->AddRef( pTheMsg, iNum-1 );
    for( int sss = 0; sss < iNum; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheMsg );
  }
}
//=========================================================================================================
//
//
void CGsData::InitCItemMem()
{
  m_dwUsingItem = 0;
  m_dwMaxItem = m_iMaxPlayers * m_iPlayerItemCount;
  m_pItemMem  = new CItem[m_dwMaxItem];
  m_pItemAshcan = (CItem**)new DWORD[m_dwMaxItem];

  for( int i = 0; i < m_dwMaxItem; i++ )
  {
    m_pItemMem[i].m_dwSrvCode  = i + 1;
    m_pItemMem[i].m_wMemState = MEMSTATE_STATIC_FREE;
    m_ListItem.push_back( &m_pItemMem[i] );
    m_pItemAshcan[i] = NULL;
  }
}
//=========================================================================================================
//
//
void CGsData::InitCSkillMem()
{
  m_dwUsingSkill = 0;
  ////////////////////////////////////////////////////
  //Add by CECE
  //因为每个妖器附带最多4个技能 ( 3个召唤技能 1个附加技能)
//#ifdef  EVILWEAPON_3_6_VERSION
//  m_dwMaxSkill = m_iMaxPlayers * (m_iPlayerSkillCount+EVILWEAPON_SKILL_COUNT+1);
//#else
  m_dwMaxSkill = m_iMaxPlayers * m_iPlayerSkillCount;
//#endif
  //
  m_pSkillMem  = new CSkill[m_dwMaxSkill];
  m_pSkillAshcan = (CSkill**)new DWORD[m_dwMaxSkill];

  for( int i = 0; i < m_dwMaxSkill; i++ )
  {
    m_pSkillMem[i].m_dwSrvCode  = i + 1;
    m_pSkillMem[i].m_wMemState = MEMSTATE_STATIC_FREE;
    m_ListSkill.push_back( &m_pSkillMem[i] );
    m_pSkillAshcan[i] = NULL;
  }
}
//=========================================================================================================
//
//
void CGsData::InitCMagicMem()
{
  m_dwUsingMagic = 0;
  m_dwMaxMagic = m_iMaxPlayers * m_iPlayerMagicCount;
  if( m_dwMaxMagic > CODE_MAX_MAGIC - CODE_MIN_MAGIC )  m_dwMaxMagic = CODE_MAX_MAGIC - CODE_MIN_MAGIC;
  m_pMagicMem  = new CMagic[m_dwMaxMagic];
  m_pMagicAshcan = (CMagic**)new DWORD[m_dwMaxMagic];

  for( int i = 0; i < m_dwMaxMagic; i++ )
  {
    m_pMagicMem[i].m_dwSrvCode  = i + 1;
    m_pMagicMem[i].m_wMemState = MEMSTATE_STATIC_FREE;
    m_ListMagic.push_back( &m_pMagicMem[i] );
    m_pMagicAshcan[i] = NULL;
  }
}
//=========================================================================================================
//
//
void CGsData::DeleteCItem( CItem * pItem, const WORD & wType )
{
  if( !pItem )        return;
  if( pItem < m_pItemMem || pItem > m_pItemMem + m_dwMaxItem )// * sizeof( DWORD ) )
  {
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCItem" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "*****[%d] Some CItem Memory Addr Error, Addr=%d *****", wType, pItem );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return;
  }
  //
  if( pItem->m_dwSrvCode <= m_dwMaxItem &&
      pItem->m_dwSrvCode > 0 &&
      pItem == m_pItemAshcan[pItem->m_dwSrvCode-1] &&
      pItem->m_wMemState == MEMSTATE_STATIC_USING )
  {
    pItem->m_wMemState = MEMSTATE_STATIC_FREE;
    m_dwUsingItem--;
    m_pItemAshcan[pItem->m_dwSrvCode-1] = NULL;
    ///////////////////////////////////////////
    //Add by CECE 2004-04-05
#ifdef  EVILWEAPON_3_6_VERSION
    if( (pItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_EVILWEAPON) 
        && pItem->m_pBase->m_wEvilWeapon )
    {
      SAFE_DELETE( pItem->m_pEvilWeapon );
    }
#endif
    ///////////////////////////////////////////
    m_ListItem.push_back( pItem );
  }
  else if( pItem->m_wMemState == MEMSTATE_DYNAMIC_USING )
  {
    //SAFE_DELETE( pItem );
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCItem" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "*****[%d] Some CItem Memory State = Dynmic_Using, CItem(%d)[%s], ItemAddr=%08x *****",
             wType, pItem->m_dwSrvCode, pItem->GetName(), pItem );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
  }
  else
  {
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCItem" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "*****[%d] CItem(%d)[%s] Cannot Delete, Pos=%d, Memory State=%d, ItemAddr=(%08x,%08x) *****",
             wType, pItem->m_dwSrvCode, pItem->GetName(),
             pItem->GetPackagePos(), pItem->m_wMemState, pItem,
             m_pItemAshcan[pItem->m_dwSrvCode-1] );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
  }
}
//=========================================================================================================
//
//
void CGsData::DeleteCSkill( CSkill *pSkill )
{
  if( !pSkill )      return;
  if( pSkill < m_pSkillMem || pSkill > m_pSkillMem + m_dwMaxSkill )// * sizeof( DWORD ) )
  {
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCSkill" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Some CSkill Memory Addr Error, Addr=%d *****", pSkill );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return;
  }
  if( pSkill->m_dwSrvCode <= m_dwMaxSkill &&
      pSkill->m_dwSrvCode > 0 &&
      pSkill == m_pSkillAshcan[pSkill->m_dwSrvCode-1] )
  {
    pSkill->m_wMemState = MEMSTATE_STATIC_FREE;
    m_dwUsingSkill--;
    m_pSkillAshcan[pSkill->m_dwSrvCode-1] = NULL;
    m_ListSkill.push_back( pSkill );
  }
  else if( pSkill->m_wMemState == MEMSTATE_DYNAMIC_USING )
  {
    SAFE_DELETE( pSkill );
  }
  else
  {
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCSkill" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** CSkill(%d)[%s] Cannot Delete, Memory State=%d, Ashcan Addr %d *****", pSkill->m_dwSrvCode, pSkill->GetBaseName(), pSkill->m_wMemState, m_pSkillAshcan[pSkill->m_dwSrvCode-1] );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
  }
}
//=========================================================================================================
//
//
void CGsData::DeleteCMagic( CMagic *pMagic )
{
  if( !pMagic )      return;
  if( !pMagic )      return;
  if( pMagic < m_pMagicMem || pMagic > m_pMagicMem + m_dwMaxMagic )// * sizeof( DWORD ) )
  {
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCMagic" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Some CMagic Memory Addr Error, Addr=%d *****", pMagic );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return;
  }
  if( pMagic->m_dwSrvCode <= m_dwMaxMagic &&
      pMagic->m_dwSrvCode > 0 &&
      pMagic == m_pMagicAshcan[pMagic->m_dwSrvCode-1] )
  {
    if( pMagic->m_pPlayer )    pMagic->m_pPlayer->DelTrap( pMagic );
    else                       pMagic->m_pMonster->DelTrap( pMagic );
    pMagic->m_pBase->ClearAllTarget();
    pMagic->ReleaseMyMapSign();
    pMagic->m_pInMap->DelMagic( pMagic );
    pMagic->m_wMemState = MEMSTATE_STATIC_FREE;
    m_dwUsingMagic--;
    m_pMagicAshcan[pMagic->m_dwSrvCode-1] = NULL;
    m_ListMagic.push_back( pMagic );
  }
  else if( pMagic->m_wMemState == MEMSTATE_DYNAMIC_USING )
  {
    if( pMagic->m_pPlayer )    pMagic->m_pPlayer->DelTrap( pMagic );
    else                       pMagic->m_pMonster->DelTrap( pMagic );
    pMagic->m_pBase->ClearAllTarget();
    pMagic->ReleaseMyMapSign();
    pMagic->m_pInMap->DelMagic( pMagic );
    SAFE_DELETE( pMagic );
  }
  else
  {
#ifdef _DEBUG
    FuncName( "CGsData::DeleteCMagic" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** CMagic(%d)[%s] Cannot Delete, Memory State=%d, Ashcan Addr %d *****", pMagic->m_dwSrvCode, pMagic->m_pBase->GetBaseName(), pMagic->m_wMemState, m_pMagicAshcan[pMagic->m_dwSrvCode-1] );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
  }
}
//------------------------------------------------------------------------------------------
// CMagic Construct 1
//
CMagic * CGsData::CreateCMagic( CSkill* pTheBase, BOOL bPK )
{
  CMagic      *pNewMagic = NULL;

  if( m_ListMagic.empty() )
  {
    // Must New One CMagic
  }
  else
  {
    pNewMagic = (*m_ListMagic.begin());
    m_ListMagic.pop_front();
    if( pNewMagic->m_dwSrvCode <= m_dwMaxMagic && pNewMagic->m_dwSrvCode > 0 )
    {
      m_pMagicAshcan[pNewMagic->m_dwSrvCode-1] = pNewMagic;
    }
    else
    {
#ifdef _DEBUG_WILDCAT_
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCMagic" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Create CMagic Error, The CMagic Srv Code=(%d,%d) ! *****", pNewMagic->m_dwSrvCode, m_dwMaxMagic );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
#endif
    }
    m_dwUsingMagic++;
    pNewMagic->m_wMemState = MEMSTATE_STATIC_USING;

    // Init The Magic Data
	  pNewMagic->m_pBase         = pTheBase;
    //if( pTheBase->GetPlayerUser() )
    //{
    //  pNewMagic->m_pMyUser     = pTheBase->GetPlayerUser();
    //}
    //else
    //{
    //  pNewMagic->m_pMyUser     = pTheBase->GetMonsterUser();
    //}
	  pNewMagic->m_pPlayer       = pTheBase->GetPlayerUser();
    pNewMagic->m_pMonster      = pTheBase->GetMonsterUser();
    pNewMagic->m_bCanPK        = bPK;
    if( pNewMagic->m_pPlayer )
    {
      pNewMagic->m_iX     = pNewMagic->m_iEndX  = pNewMagic->m_pPlayer->GetPosX();
      pNewMagic->m_iY     = pNewMagic->m_iEndY  = pNewMagic->m_pPlayer->GetPosY();
      pNewMagic->m_iDir   = pNewMagic->m_pPlayer->GetMoveDir();
      pNewMagic->m_pInMap = pNewMagic->m_pPlayer->GetInMap();
    }
    else// if( pNewMagic->m_pMonster )
    {
      pNewMagic->m_iX     = pNewMagic->m_iEndX  = pNewMagic->m_pMonster->GetPosX();
      pNewMagic->m_iY     = pNewMagic->m_iEndY  = pNewMagic->m_pMonster->GetPosY();
      pNewMagic->m_iDir   = pNewMagic->m_pMonster->GetMoveDir();
      pNewMagic->m_pInMap = pNewMagic->m_pMonster->GetInMap();
    }
	  pNewMagic->m_dwLifeTime    = ClientTickCount + pTheBase->m_pBase->GetAlarmTime();
    pNewMagic->m_dwActionTime  = ClientTickCount + pTheBase->m_pBase->GetTriggerTime();
    pNewMagic->m_iStatus       = 0;
    pNewMagic->m_wUpdateTurn   = 0;
    pNewMagic->m_pFirstTarget  = NULL;
    pNewMagic->m_wCode         = pNewMagic->m_pInMap->GetNewMagicCode();
	  pNewMagic->m_pBase->ClearAllTarget();
    // 设置地图标记
    pNewMagic->SetMyMapSign();
    pNewMagic->m_pInMap->AddMagic( pNewMagic );
    //
    if( pNewMagic->m_pPlayer )    pNewMagic->m_pPlayer->AddTrap( pNewMagic );
    else                          pNewMagic->m_pMonster->AddTrap( pNewMagic );
  }
  return pNewMagic;
}
//------------------------------------------------------------------------------------------
// CSkill Construct 1
//
CSkill * CGsData::CreateCSkill( CSrvBaseSkill* pTheBase, const int & iTheExp, CLife * pUser, const int & iType )
{
  CSkill      *pNewSkill = NULL;

  if( m_ListSkill.empty() )
  {
    // Must New One CSkill
  }
  else
  {
    pNewSkill = (*m_ListSkill.begin());
    m_ListSkill.pop_front();
    if( pNewSkill->m_dwSrvCode <= m_dwMaxSkill && pNewSkill->m_dwSrvCode > 0 )
    {
      m_pSkillAshcan[pNewSkill->m_dwSrvCode-1] = pNewSkill;
    }
    else
    {
#ifdef _DEBUG_WILDCAT_
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCSkill" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Create CSkill Error, The Skill Srv Code=(%d,%d) ! *****", pNewSkill->m_dwSrvCode, m_dwMaxSkill );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
#endif
    }
    m_dwUsingSkill++;
    pNewSkill->m_wMemState = MEMSTATE_STATIC_USING;

	  pNewSkill->m_pBase         = pTheBase;
	  pNewSkill->m_iExp          = iTheExp;
    pNewSkill->m_wElement      = pTheBase->m_wElement;
    pNewSkill->m_wElementCount = 0;
    if( pTheBase->GetType() > SKILL_TYPE_WIZARD_SKILL )
    {
      memset( pNewSkill->m_wSimElement, 0, sizeof( WORD ) * 16 );
      for( int i = 0; i < 16; i++ )
      {
        if( ( pNewSkill->m_wElement & ( 1 << i ) ) )
        {
          pNewSkill->m_wSimElement[pNewSkill->m_wElementCount] = ( 1 << i );
          pNewSkill->m_wElementCount++;
        }
      }
    }
	  // 0 Means Monster
	  if( iType == 0 || pUser->IsMonster() )
	  {
	  	pNewSkill->m_pPlayer  = NULL;
		  pNewSkill->m_pMonster = (CMonster*)(pUser);
	  }
	  // 1 Means Player
	  else if( iType == 1 || pUser->IsPlayer() )
	  {
		  pNewSkill->m_pMonster = NULL;
		  pNewSkill->m_pPlayer  = (CPlayer*)(pUser);
	  }
	  pNewSkill->m_listTargets.clear();
	  pNewSkill->m_listMiss.clear();
  }
  return pNewSkill;
}
//------------------------------------------------------------------------------------------
// CSkill Construct 1
//
CSkill * CGsData::CreateCSkill( CSrvBaseSkill* pTheBase, const WORD & wElement, const int & iTheExp, CLife * pUser, const int & iType )
{
  CSkill      *pNewSkill = NULL;

  if( m_ListSkill.empty() )
  {
    // Must New One CSkill
  }
  else
  {
    pNewSkill = (*m_ListSkill.begin());
    m_ListSkill.pop_front();
    if( pNewSkill->m_dwSrvCode <= m_dwMaxSkill && pNewSkill->m_dwSrvCode > 0 )
    {
      m_pSkillAshcan[pNewSkill->m_dwSrvCode-1] = pNewSkill;
    }
    else
    {
#ifdef _DEBUG_WILDCAT_
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCSkill" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Create CSkill Error, The Skill Srv Code=(%d,%d) ! *****", pNewSkill->m_dwSrvCode, m_dwMaxSkill );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
#endif
    }
    m_dwUsingSkill++;
    pNewSkill->m_wMemState = MEMSTATE_STATIC_USING;

	  pNewSkill->m_pBase         = pTheBase;
	  pNewSkill->m_iExp          = iTheExp;
    if( pTheBase->GetType() > SKILL_TYPE_WIZARD_SKILL )
    {
	    pNewSkill->m_wElement      = pTheBase->m_wElement;
      pNewSkill->m_wElementCount = 0;
    }
    else
    {
      pNewSkill->m_wElement      = wElement;
      pNewSkill->m_wElementCount = 0;
      memset( pNewSkill->m_wSimElement, 0, sizeof( WORD ) * 16 );
      for( int i = 0; i < 16; i++ )
      {
        if( ( pNewSkill->m_wElement & ( 1 << i ) ) )
        {
          pNewSkill->m_wSimElement[pNewSkill->m_wElementCount] = ( 1 << i );
          pNewSkill->m_wElementCount++;
        }
      }
    }
	  // 0 Means Monster
	  if( iType == 0 || pUser->IsMonster() )
	  {
	  	pNewSkill->m_pPlayer  = NULL;
		  pNewSkill->m_pMonster = (CMonster*)(pUser);
	  }
	  // 1 Means Player
	  else if( iType == 1 || pUser->IsPlayer() )
	  {
		  pNewSkill->m_pMonster = NULL;
		  pNewSkill->m_pPlayer  = (CPlayer*)(pUser);
	  }
	  pNewSkill->m_listTargets.clear();
	  pNewSkill->m_listMiss.clear();
  }
  return pNewSkill;
}
//------------------------------------------------------------------------------------------
// Item Construct 1
//
CItem * CGsData::CreateCItem( CSrvBaseItem * pBaseItem, CPlayer * pOwner, const WORD & wCount )
{
  CItem               *pNewItem = NULL;
  static BYTE			    bLoop01;

  if( m_ListItem.empty() )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateCItem1" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Cannot Create CItem, List Empty ! *****", pOwner->GetPlayerAccount() );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return NULL;
    // Must Tack Back The Ground Item Memory
    // ...
    //pNewItem = new CItem();// CSrvBaseItem * pBaseItem, CPlayer * pOwner, const WORD & wCount );
  }
  else
  {
    m_dwUsingItem++;
    pNewItem = (*m_ListItem.begin());
    if( pNewItem->m_dwSrvCode <= m_dwMaxItem && pNewItem->m_dwSrvCode > 0 )
    {
      m_pItemAshcan[pNewItem->m_dwSrvCode-1] = pNewItem;
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCItem1" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Create CItem Error, The Item Srv Code=(%d,%d) ! *****", pOwner->GetPlayerAccount(), pNewItem->m_dwSrvCode, m_dwMaxItem );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
      return NULL;
    }
    m_ListItem.pop_front();
    pNewItem->m_wMemState = MEMSTATE_STATIC_USING;
    // Record Mail Id
    pNewItem->m_dwMailId = pOwner->GetMailId();
    // Init The Item
	  for( bLoop01 = 0; bLoop01 < MAX_ITEM_SLOT; bLoop01++ )
	  {
		  pNewItem->m_pSklTessera[bLoop01]    = NULL;
		  pNewItem->m_wTesseraRndNum[bLoop01] = 0;
		  pNewItem->m_wTesseraItemID[bLoop01] = 0;
	  }
	  pNewItem->m_pBase       = pBaseItem;
#ifdef _DEBUG_ITEM_CREASH_
    pNewItem->m_dwCrashData[0] = 0;
    pNewItem->m_dwCrashData[1] = 0;
#endif
    pNewItem->m_dwUniqueId[0] = ( pOwner->GetMailId() << 8 ) |
										            ( ( g_SysTime.wYear & 0x000F ) << 4 ) | g_SysTime.wMonth;
	  pNewItem->m_dwUniqueId[1] = ( g_SysTime.wDay << 27 ) |
										            ( g_SysTime.wHour << 22 ) |
										            ( g_SysTime.wMinute << 16 ) |
										            ( g_SysTime.wSecond << 10 ) |
										            pOwner->GetItemLoopCounter();
    //
#ifdef _REPAIR_SERVER_CRASH_NICK_
		memset( pNewItem->m_szNickName, 0, MAX_ITEM_NAME_LEN );  // nick name
#else
	  strcpy( pNewItem->m_szNickName, "" );		// nick name
#endif
	  pNewItem->m_dwMailId = 0;								// Owner Mail Id

	  if( pBaseItem->GetType() > ITEM_TYPE_UNEQUIP )
	  {
		  pNewItem->m_wCount = 1;									// Current Wrap Count
	  }
	  else
	  {
		  pNewItem->m_wCount = wCount;
	  }
    if( pNewItem->m_wCount > pBaseItem->GetMaxWrap() )  pNewItem->m_wCount = pBaseItem->GetMaxWrap();
	  //m_wPosX	 = pOwner->GetPosX();
	  //m_wPosY	 = pOwner->GetPosY();
	  //m_wMapId = pOwner->GetMapId();

    // 克制怪物的附加属性--只有在此道具上添加镶嵌物时才会增加属性, 否则和CSrvBaseItem一样
    // 计算克制怪物相形，体形，种族，Boss 加成因数
    pNewItem->m_wElement      = pBaseItem->m_wElement;
    pNewItem->m_dwRaceAttri   = pBaseItem->m_dwRaceAttri;
    pNewItem->m_dwBossCode    = pBaseItem->m_dwBossCode;
    pNewItem->m_wRaceBonuRate = pBaseItem->m_wRaceBonuRate;
    pNewItem->m_wBossBonuRate = pBaseItem->m_wBossBonuRate;

	  pNewItem->m_iAp      = pBaseItem->m_iAp;				// Current Add Ap
	  pNewItem->m_iDp      = pBaseItem->m_iDp;				// Current Add Dp
	  pNewItem->m_iHit     = pBaseItem->m_iHit;				// Current Add Hit
	  pNewItem->m_iDodge   = pBaseItem->m_iDodge;			// Current Add Dodge
	  pNewItem->m_iInt     = pBaseItem->m_iInt;				// Current Add Intelligence
	  pNewItem->m_iBearPsb = pBaseItem->m_iBearPsb;   //

	  pNewItem->m_wLevel = pBaseItem->m_wLevel;			// Current Level Of Item
	  pNewItem->m_wApUp  = pBaseItem->m_wApUp;			// Current Ap Up By Level
	  pNewItem->m_wDpUp  = pBaseItem->m_wDpUp;			// Current Dp Up By Level
	  pNewItem->m_wDgeUp = pBaseItem->m_wDgeUp;			// Current Hit Up By Level
	  pNewItem->m_wHitUp = pBaseItem->m_wHitUp;			// Current Dodge Up By Level
	  pNewItem->m_wIntUp = pBaseItem->m_wIntUp;			// Current Intelligence Up By Level

	  pNewItem->m_wApFixLevel    = pBaseItem->m_wApFixLevel;		// How Level Of Item Change The Ap Up Data
	  pNewItem->m_wDpFixLevel    = pBaseItem->m_wDpFixLevel;		// How Level Of Item Change The Dp Up Data
	  pNewItem->m_wHitFixLevel   = pBaseItem->m_wHitFixLevel;		// How Level Of Item Change The Hit Up Data
	  pNewItem->m_wDodgeFixLevel = pBaseItem->m_wDodgeFixLevel;	// How Level Of Item Change The Dodge Up Data
	  pNewItem->m_wIntFixLevel   = pBaseItem->m_wIntFixLevel;		// How Level Of Item Change The Intelligence Up Data

	  pNewItem->m_wApFix	  = pBaseItem->m_wApFix;							// The Base Data Of Change Ap Up
	  pNewItem->m_wDpFix		= pBaseItem->m_wDpFix;							// The Base Data Of Change Dp Up
	  pNewItem->m_wHitFix		= pBaseItem->m_wHitFix;							// The Base Data Of Change Hit Up
	  pNewItem->m_wDodgeFix	= pBaseItem->m_wDodgeFix;						// The Base Data Of Change Dodge Up
	  pNewItem->m_wIntFix		= pBaseItem->m_wIntFix;							// The Base Data Of Change Intelligence Up

	  pNewItem->m_wCriticalHit = pBaseItem->m_wCriticalHit;			// The Critical Hit Percent

	  pNewItem->m_wMaxDurability = pBaseItem->m_wMaxDurability;	// The Max Durabiblity
	  pNewItem->m_wDurability    = pBaseItem->m_wDurability;		// Current Durability
	  pNewItem->m_wDurabilityUp  = pBaseItem->m_wDurabilityUp;	// Durability Up Data

	  pNewItem->m_wForging      = pBaseItem->m_wForging;				// Forging Times
	  pNewItem->m_wForgingDblUp = pBaseItem->m_wForgingDblUp;		// The Durability Up Data When Player Forging The Item

	  pNewItem->m_wHard    = pBaseItem->m_wHard;								// The Hard Value Decide The Item Mangle
	  pNewItem->m_wStable  = pBaseItem->m_wStable;							// The Stable Value Mean The Item Disappear Rate When Compose Item 

	  //=============道具SKILL函数相关===============
	  pNewItem->m_wFuncDbc     = pBaseItem->m_wFuncDbc;							// Double Click Functoin
	  pNewItem->m_iEffectTime  = pBaseItem->m_iEffectTime;				// Use Degree ( 使用次数 )
	  pNewItem->m_wFuncEffect  = pBaseItem->m_wFuncEffect;				// The Skill Function
	  pNewItem->m_wFuncEquip   = pBaseItem->m_wFuncEquip;				// Equip, Put On Equipment Function
	  pNewItem->m_wFuncTakeOff = pBaseItem->m_wFuncTakeOff;			// Take Off Equipment Function
	  pNewItem->m_wFuncTouch   = pBaseItem->m_wFuncTouch;				// Touch Item Function
	  pNewItem->m_wHoleNumber	 = pBaseItem->m_wHoleNumber;			// hole number
	  pNewItem->m_iAlarmTime   = -1;					// The Duration Time
	  pNewItem->m_wFuncAlarm   = 0;						// The Duration Function
	  pNewItem->m_iTriggerTime = -1;					// Trigger Interval, Period
	  pNewItem->m_wFuncTrigger = 0;						// Period Function
	  memset( pNewItem->m_iVar, 0, sizeof( int ) * 4 );	// The Variables That Called By All Functions

	  //m_wTesseraItemID[0] = pPlayerItem->wHoleItem[0];
    //m_wTesseraItemID[1] = pPlayerItem->wHoleItem[1];
    //m_wTesseraItemID[2] = pPlayerItem->wHoleItem[2];
	  pNewItem->m_wTesseraItemID[0] = 0;
    pNewItem->m_wTesseraItemID[1] = 0;
    pNewItem->m_wTesseraItemID[2] = 0;
	  pNewItem->m_dwColorId = 0;

	  pNewItem->InitFuncSkill();
    // Set Ground Item Color
    //pNewItem->SetGroundItemColor();

    //////////////////////////////////////////////////////////////////////////////////
    //Add by CECE 2004-04-05
#ifdef  EVILWEAPON_3_6_VERSION
    if( (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_EVILWEAPON) 
        && pNewItem->m_pBase->m_wEvilWeapon )
        pNewItem->m_pEvilWeapon = new CEvilWeapon( pNewItem->m_pBase->m_wEvilWeapon, pOwner, pNewItem->m_iVar );
    else
        pNewItem->m_pEvilWeapon = NULL;
#endif
#ifdef ALL_SPECIAL_ITEM_FUNC
    if( pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_SAVEPOINT)
    {
      pNewItem->m_iVar[0] = g_pBase->GetSavePoinItemUseCount(pNewItem->m_pBase->m_wId);
      pNewItem->m_iVar[1] = g_pBase->GetSavePoinItemUseOdds(pNewItem->m_pBase->m_wId);
    }
#ifdef _MUSIC_BOX_
    if( pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_MUSICBOX )
    {
      pNewItem->m_iVar[0] = g_pBase->GetMusicBoxSongNum(pNewItem->m_pBase->m_wId);
    }
#endif    
#endif

#ifdef _NEW_TRADITIONS_WEDDING_
    if (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_RESTOREHP)
    {
      g_pBase->GetRestoreItemInfo(pNewItem->m_pBase->m_wId,
                                  pNewItem->m_iVar[0],
                                  pNewItem->m_iVar[1],
                                  pNewItem->m_iVar[2],
                                  pNewItem->m_iVar[3]);
    }
#endif    
    ///////////////////////////////////////////////////////////////////////////////////
  }
  return pNewItem;
}
//------------------------------------------------------------------------------------------
// Construct 2
//
CItem * CGsData::CreateCItem( CSrvBaseItem * pBaseItem, CPlayer * pOwner, LPSPlayerItemEx pPlayerItem, BOOL bIsStorage )
{
#ifdef FOR_TAIBEI_COPYBUG
  static int iHouseMapId[10] = 
  {
    4001,4012,4018,4014,4003,4020,4010,4016,4022,4004
  };
#endif
  CItem               *pNewItem = NULL;
  static int          bLoop02;

  if( m_ListItem.empty() )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateCItem2" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Cannot Create CItem, List Empty ! *****", pOwner->GetPlayerAccount() );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return NULL;
    // Must Take Back The Ground Item Memory
    // ...
    //pNewItem = new CItem();// CSrvBaseItem * pBaseItem, CPlayer * pOwner, const WORD & wCount );
  }
  else
  {
    m_dwUsingItem++;
    pNewItem = (*m_ListItem.begin());
    if( !pNewItem )
      return NULL;
    if( pNewItem->m_dwSrvCode <= m_dwMaxItem && pNewItem->m_dwSrvCode > 0 )
    {
      m_pItemAshcan[pNewItem->m_dwSrvCode-1] = pNewItem;
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCItem2" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Create CItem Error, The Item Srv Code=(%d,%d) ! *****", pOwner->GetPlayerAccount(), pNewItem->m_dwSrvCode, m_dwMaxItem );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
      return NULL;
    }
    m_ListItem.pop_front();
    pNewItem->m_wMemState = MEMSTATE_STATIC_USING;
    // Record Mail Id
    pNewItem->m_dwMailId = pOwner->GetMailId();
    // Init The Item
    for( bLoop02 = 0; bLoop02 < MAX_ITEM_SLOT; bLoop02++ )
    {
      pNewItem->m_pSklTessera[bLoop02] = NULL;
      pNewItem->m_wTesseraRndNum[bLoop02] = 0;
      pNewItem->m_wTesseraItemID[bLoop02] = 0;
    }
#ifdef _DEBUG_ITEM_CREASH_
    pNewItem->m_dwCrashData[0] = 0;
    pNewItem->m_dwCrashData[1] = 0;
#endif
    pNewItem->m_pBase = pBaseItem;
    //======================= Save Data ==================
    pNewItem->m_dwUniqueId[0] = pPlayerItem->dwUniqueId[0];
    pNewItem->m_dwUniqueId[1] = pPlayerItem->dwUniqueId[1];
    //
    pNewItem->m_wPackagePos = pPlayerItem->wPos;
    //
    memcpy( pNewItem->m_szNickName, pPlayerItem->szNickName, MAX_ITEM_NAME_LEN );
    pNewItem->m_szNickName[MAX_ITEM_NAME_LEN-1] = '\0';
    pNewItem->m_dwMailId = pPlayerItem->dwMailId; // Nickname Owner Mail Id
    
    //if( pBaseItem->GetType() > ITEM_TYPE_UNEQUIP )
    //{
    //	m_wCount = 1;									// Current Wrap Count
    //}
    //else
    //{
    //	m_wCount = (WORD)LOBYTE(pPlayerItem->wHoleNumber_Count);
    //}
    
    // 克制怪物的附加属性--只有在此道具上添加镶嵌物时才会增加属性, 否则和CSrvBaseItem一样
    pNewItem->m_wElement      = pBaseItem->m_wElement;
    pNewItem->m_dwRaceAttri   = pBaseItem->m_dwRaceAttri;
    pNewItem->m_dwBossCode    = pBaseItem->m_dwBossCode;
    pNewItem->m_wRaceBonuRate = pBaseItem->m_wRaceBonuRate;
    pNewItem->m_wBossBonuRate = pBaseItem->m_wBossBonuRate;
    //
    pNewItem->m_wCount = (WORD)LOBYTE(pPlayerItem->wHoleNumber_Count);// Current Wrap Count
    if( bIsStorage == FALSE )
    {
      if( pNewItem->m_wCount > pBaseItem->GetMaxWrap() )  pNewItem->m_wCount = pBaseItem->GetMaxWrap();
    }
    else
    {
      if( pNewItem->m_wCount > MAX_STORAGE_ITEM_WRAP )    pNewItem->m_wCount = MAX_STORAGE_ITEM_WRAP;
    }
    
    pNewItem->m_wPosX	 = pOwner->GetPosX();
    pNewItem->m_wPosY	 = pOwner->GetPosY();
    pNewItem->m_wMapId = pOwner->GetMapId();
    
    pNewItem->m_iAp    = pPlayerItem->iAp;				// Current Add Ap
    pNewItem->m_iDp    = pPlayerItem->iDp;				// Current Add Dp
    pNewItem->m_iHit   = pPlayerItem->iHit;				// Current Add Hit
    pNewItem->m_iDodge = pPlayerItem->iDodge;			// Current Add Dodge
    pNewItem->m_iInt   = pPlayerItem->iInt;				// Current Add Intelligence
    pNewItem->m_iBearPsb = pBaseItem->m_iBearPsb; //
    
    pNewItem->m_wLevel = pPlayerItem->wLevel_IntFix >> 8;			            // Current Level Of Item
    pNewItem->m_wApUp  = (WORD)HIBYTE(pPlayerItem->wApUp_ApFixLevel);			// Current Ap Up By Level
    pNewItem->m_wDpUp  = (WORD)HIBYTE(pPlayerItem->wDpUp_DpFixLevel);			// Current Dp Up By Level
    pNewItem->m_wHitUp = (WORD)HIBYTE(pPlayerItem->wHitUp_HitFixLevel);		// Current Dodge Up By Level
    pNewItem->m_wDgeUp = (WORD)HIBYTE(pPlayerItem->wDgeUp_DodgeFixLevel);	// Current Hit Up By Level
    pNewItem->m_wIntUp = (WORD)HIBYTE(pPlayerItem->wIntUp_IntFixLevel);		// Current Intelligence Up By Level
    
    pNewItem->m_wApFixLevel    = (WORD)LOBYTE(pPlayerItem->wApUp_ApFixLevel);			// How Level Of Item Change The Ap Up Data
    pNewItem->m_wDpFixLevel    = (WORD)LOBYTE(pPlayerItem->wDpUp_DpFixLevel);			// How Level Of Item Change The Dp Up Data
    pNewItem->m_wHitFixLevel   = (WORD)LOBYTE(pPlayerItem->wHitUp_HitFixLevel);		// How Level Of Item Change The Hit Up Data
    pNewItem->m_wDodgeFixLevel = (WORD)LOBYTE(pPlayerItem->wDgeUp_DodgeFixLevel);	// How Level Of Item Change The Dodge Up Data
    pNewItem->m_wIntFixLevel   = (WORD)LOBYTE(pPlayerItem->wIntUp_IntFixLevel);		// How Level Of Item Change The Intelligence Up Data
    
    pNewItem->m_wApFix	  = (WORD)HIBYTE(pPlayerItem->wApFix_DpFix);							// The Base Data Of Change Ap Up
    pNewItem->m_wDpFix		= (WORD)LOBYTE(pPlayerItem->wApFix_DpFix);							// The Base Data Of Change Dp Up
    pNewItem->m_wHitFix		= (WORD)HIBYTE(pPlayerItem->wHitFix_DodgeFix);					// The Base Data Of Change Hit Up
    pNewItem->m_wDodgeFix	= (WORD)LOBYTE(pPlayerItem->wHitFix_DodgeFix);					// The Base Data Of Change Dodge Up
    pNewItem->m_wIntFix		= (WORD)LOBYTE(pPlayerItem->wLevel_IntFix);				// The Base Data Of Change Intelligence Up
    
    pNewItem->m_wCriticalHit = (WORD)LOBYTE(pPlayerItem->wColorId_CriticalHit);		// The Critical Hit Percent
    
    pNewItem->m_wMaxDurability = pPlayerItem->wMaxDurability;	// The Max Durabiblity
    pNewItem->m_wDurability    = pPlayerItem->wDurability;		// Current Durability
    pNewItem->m_wDurabilityUp  = pPlayerItem->wDurabilityUp;	// Durability Up Data
    
    pNewItem->m_wForging      = (WORD)HIBYTE(pPlayerItem->wForging_Hard); // Forging Times
    pNewItem->m_wForgingDblUp = (WORD)HIBYTE(pPlayerItem->wForgingDblUp_Stable);		// The Durability Up Data When Player Forging The Item
    
    pNewItem->m_wHard    = (WORD)LOBYTE(pPlayerItem->wForging_Hard);		// The Hard Value Decide The Item Mangle
    pNewItem->m_wStable  = (WORD)LOBYTE(pPlayerItem->wForgingDblUp_Stable); // The Stable Value Mean The Item Disappear Rate When Compose Item 

    //=============道具SKILL函数相关===============
    pNewItem->m_wFuncDbc     = pPlayerItem->wFuncDbc;					// Double Click Functoin
    pNewItem->m_iEffectTime  = pPlayerItem->iEffectTime;			// Use Degree ( 使用次数 )
    pNewItem->m_wFuncEffect  = pPlayerItem->wFuncEffect;			// The Skill Function
    pNewItem->m_wFuncEquip   = pPlayerItem->wFuncEquip;				// Equip, Put On Equipment Function
    pNewItem->m_wFuncTakeOff = pPlayerItem->wFuncTakeOff;			// Take Off Equipment Function
    pNewItem->m_wFuncTouch   = pPlayerItem->wFuncTouch;				// Touch Item Function
    pNewItem->m_iAlarmTime   = pPlayerItem->iAlarmTime;				// The Duration Time
    pNewItem->m_wFuncAlarm   = pPlayerItem->wFuncAlarm;				// The Duration Function
    pNewItem->m_iTriggerTime = pPlayerItem->iTriggerTime;			// Trigger Interval, Period
    pNewItem->m_wFuncTrigger = pPlayerItem->wFuncTrigger;			// Period Function
    pNewItem->m_wHoleNumber	 = HIBYTE(pPlayerItem->wHoleNumber_Count);// hole number
    memcpy( pNewItem->m_iVar, pPlayerItem->iVar, sizeof( int ) * 4 );	// The Variables That Called By All Functions
#ifdef FOR_TAIBEI_COPYBUG
    if( pBaseItem->m_wId == 1719 )
    {
      int i = 0;
      for(; i<10; i++)
      {
        if(pNewItem->m_iVar[1] == iHouseMapId[i])
          break;
      }
      if(i == 10)
        pNewItem->m_iVar[1] = 0;
      pNewItem->m_iVar[2] = 0;
      pNewItem->m_iVar[3] = 0;
    }
#endif
    pNewItem->m_wTesseraItemID[0] = pPlayerItem->wHoleItem[0];
    pNewItem->m_wTesseraItemID[1] = pPlayerItem->wHoleItem[1];
    pNewItem->m_wTesseraItemID[2] = pPlayerItem->wHoleItem[2];
    
    pNewItem->m_dwColorId = 0;
    
    pNewItem->InitFuncSkill();
    // Set Ground Item Color
    //pNewItem->SetGroundItemColor();

    //////////////////////////////////////////////////////////////////////////////////
    //Add by CECE 2004-04-05
#ifdef  EVILWEAPON_3_6_VERSION
    if( (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_EVILWEAPON) 
        && pNewItem->m_pBase->m_wEvilWeapon )
        pNewItem->m_pEvilWeapon = new CEvilWeapon( pNewItem->m_pBase->m_wEvilWeapon, pOwner, pNewItem->m_iVar );
    else
        pNewItem->m_pEvilWeapon = NULL;
#endif
#ifdef _NEW_TRADITIONS_WEDDING_
    if (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_RESTOREHP)
    {
      g_pBase->GetRestoreItemInfo(pNewItem->m_pBase->m_wId,
                                  pNewItem->m_iVar[0],
                                  pNewItem->m_iVar[1],
                                  pNewItem->m_iVar[2],
                                  pNewItem->m_iVar[3]);
    }
#endif    
    ///////////////////////////////////////////////////////////////////////////////////
  }
  return pNewItem;
}
//------------------------------------------------------------------------------------------
// Construct 3
//
CItem * CGsData::CreateCItem( CItem * pItem, CPlayer * pOwner, const WORD & wCount )
{
  CItem         *pNewItem = NULL;
  CSrvBaseItem  *pBaseItem = NULL;
  static int    bLoop03;

  if( NULL == pItem )
	{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateCItem3" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Create CItem, pItem is NULL ! *****");
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return NULL;
	}
  if( NULL == pOwner )
	{
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateCItem3" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Create CItem, pOwner is NULL ! *****");
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return NULL;
	}
  if( m_ListItem.empty() )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateCItem3" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Cannot Create CItem, List Empty ! *****", pOwner->GetPlayerAccount() );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return NULL;
    // Must Tack Back The Ground Item Memory
    // ...
    //pNewItem = new CItem();// CSrvBaseItem * pBaseItem, CPlayer * pOwner, const WORD & wCount );
  }
  else
  {
    m_dwUsingItem++;
    pNewItem = (*m_ListItem.begin());
		//
    if( pNewItem->m_dwSrvCode <= m_dwMaxItem && pNewItem->m_dwSrvCode > 0 )
    {
      m_pItemAshcan[pNewItem->m_dwSrvCode-1] = pNewItem;
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCItem3" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Create CItem Error, The Item Srv Code=(%d,%d) ! *****", pOwner->GetPlayerAccount(), pNewItem->m_dwSrvCode, m_dwMaxItem );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
      return NULL;
    }
    m_ListItem.pop_front();
    pNewItem->m_wMemState = MEMSTATE_STATIC_USING;
    // Record Mail Id
    pNewItem->m_dwMailId = pOwner->GetMailId();
    // Init The Item
    for( bLoop03 = 0; bLoop03 < MAX_ITEM_SLOT; bLoop03++ )
	  {
		  pNewItem->m_pSklTessera[bLoop03] = NULL;
		  pNewItem->m_wTesseraRndNum[bLoop03] = 0;
		  pNewItem->m_wTesseraItemID[bLoop03] = 0;
	  }
#ifdef _DEBUG_ITEM_CREASH_
    pNewItem->m_dwCrashData[0] = 0;
    pNewItem->m_dwCrashData[1] = 0;
#endif
	  pBaseItem = pNewItem->m_pBase = pItem->m_pBase;
	  //=============== Save Data=========================
    pNewItem->m_dwUniqueId[0] = ( pOwner->GetMailId() << 8 ) |
										            ( ( g_SysTime.wYear & 0x000F ) << 4 ) |
										            g_SysTime.wMonth;
	  pNewItem->m_dwUniqueId[1] = ( g_SysTime.wDay << 27 ) |
										            ( g_SysTime.wHour << 22 ) |
										            ( g_SysTime.wMinute << 16 ) |
										            ( g_SysTime.wSecond << 10 ) |
										            pOwner->GetItemLoopCounter();
    //
	  memcpy( pNewItem->m_szNickName, pItem->GetNickName(), MAX_ITEM_NAME_LEN );	// The Item Nick Name
    pNewItem->m_szNickName[MAX_ITEM_NAME_LEN-1] = '\0';
	  pNewItem->m_dwMailId = pItem->m_dwMailId;								// Owner Mail Id

	  pNewItem->m_wCount = wCount;							// Current Wrap Count
    if( pNewItem->m_wCount > pBaseItem->GetMaxWrap() )  pNewItem->m_wCount = pBaseItem->GetMaxWrap();

    // 克制怪物的附加属性--只有在此道具上添加镶嵌物时才会增加属性, 否则和CSrvBaseItem一样
    pNewItem->m_wElement      = pBaseItem->m_wElement;
    pNewItem->m_dwRaceAttri   = pBaseItem->m_dwRaceAttri;
    pNewItem->m_dwBossCode    = pBaseItem->m_dwBossCode;
    pNewItem->m_wRaceBonuRate = pBaseItem->m_wRaceBonuRate;
    pNewItem->m_wBossBonuRate = pBaseItem->m_wBossBonuRate;
    //

	  pNewItem->m_wPosX	 = pOwner->GetPosX();
	  pNewItem->m_wPosY	 = pOwner->GetPosY();
	  pNewItem->m_wMapId = pOwner->GetMapId();
	  pNewItem->m_iAp    = pItem->m_iAp;				// Current Add Ap
	  pNewItem->m_iDp    = pItem->m_iDp;				// Current Add Dp
	  pNewItem->m_iHit   = pItem->m_iHit;				// Current Add Hit
	  pNewItem->m_iDodge = pItem->m_iDodge;			// Current Add Dodge
	  pNewItem->m_iInt   = pItem->m_iInt;				// Current Add Intelligence
	  pNewItem->m_iBearPsb = pBaseItem->m_iBearPsb; //

	  pNewItem->m_wLevel = pItem->m_wLevel;			// Current Level Of Item
	  pNewItem->m_wApUp  = pItem->m_wApUp;			// Current Ap Up By Level
	  pNewItem->m_wDpUp  = pItem->m_wDpUp;			// Current Dp Up By Level
	  pNewItem->m_wDgeUp = pItem->m_wDgeUp;			// Current Hit Up By Level
	  pNewItem->m_wHitUp = pItem->m_wHitUp;			// Current Dodge Up By Level
	  pNewItem->m_wIntUp = pItem->m_wIntUp;			// Current Intelligence Up By Level

	  pNewItem->m_wApFixLevel    = pItem->m_wApFixLevel;		// How Level Of Item Change The Ap Up Data
	  pNewItem->m_wDpFixLevel    = pItem->m_wDpFixLevel;		// How Level Of Item Change The Dp Up Data
	  pNewItem->m_wHitFixLevel   = pItem->m_wHitFixLevel;		// How Level Of Item Change The Hit Up Data
	  pNewItem->m_wDodgeFixLevel = pItem->m_wDodgeFixLevel;	// How Level Of Item Change The Dodge Up Data
	  pNewItem->m_wIntFixLevel   = pItem->m_wIntFixLevel;		// How Level Of Item Change The Intelligence Up Data

	  pNewItem->m_wApFix	  = pItem->m_wApFix;							// The Base Data Of Change Ap Up
	  pNewItem->m_wDpFix		= pItem->m_wDpFix;							// The Base Data Of Change Dp Up
	  pNewItem->m_wHitFix		= pItem->m_wHitFix;							// The Base Data Of Change Hit Up
	  pNewItem->m_wDodgeFix	= pItem->m_wDodgeFix;						// The Base Data Of Change Dodge Up
	  pNewItem->m_wIntFix		= pItem->m_wIntFix;							// The Base Data Of Change Intelligence Up

	  pNewItem->m_wCriticalHit = pItem->m_wCriticalHit;			// The Critical Hit Percent

	  pNewItem->m_wMaxDurability = pItem->m_wMaxDurability;	// The Max Durabiblity
	  pNewItem->m_wDurability    = pItem->m_wDurability;		// Current Durability
	  pNewItem->m_wDurabilityUp  = pItem->m_wDurabilityUp;	// Durability Up Data

	  pNewItem->m_wForging      = pItem->m_wForging;				// Forging Times
	  pNewItem->m_wForgingDblUp = pItem->m_wForgingDblUp;		// The Durability Up Data When Player Forging The Item

	  pNewItem->m_wHard    = pItem->m_wHard;								// The Hard Value Decide The Item Mangle
	  pNewItem->m_wStable  = pItem->m_wStable;							// The Stable Value Mean The Item Disappear Rate When Compose Item 

	  //=============道具SKILL函数相关===============
	  pNewItem->m_wFuncDbc			= pItem->m_wFuncDbc;					// Double Click Functoin
	  pNewItem->m_iEffectTime		= pItem->m_iEffectTime;				// Use Degree ( 使用次数 )
	  pNewItem->m_wFuncEffect		= pItem->m_wFuncEffect;				// The Skill Function
	  pNewItem->m_wFuncEquip		= pItem->m_wFuncEquip;				// Equip, Put On Equipment Function
	  pNewItem->m_wFuncTakeOff	= pItem->m_wFuncTakeOff;			// Take Off Equipment Function
	  pNewItem->m_wFuncTouch		= pItem->m_wFuncTouch;				// Touch Item Function
	  pNewItem->m_iAlarmTime		= pItem->m_iAlarmTime;				// The Duration Time
	  pNewItem->m_wFuncAlarm		= pItem->m_wFuncAlarm;				// The Duration Function
	  pNewItem->m_iTriggerTime	= pItem->m_iTriggerTime;			// Trigger Interval, Period
	  pNewItem->m_wFuncTrigger	= pItem->m_wFuncTrigger;			// Period Function
	  pNewItem->m_wHoleNumber		= pItem->m_wHoleNumber;				// hole number

	  memcpy( pNewItem->m_iVar,pItem->m_iVar,sizeof(int) * 4 );// The Variables That Called By All Functions
	  pNewItem->m_wTesseraItemID[0] = pItem->m_wTesseraItemID[0];
    pNewItem->m_wTesseraItemID[1] = pItem->m_wTesseraItemID[1];
    pNewItem->m_wTesseraItemID[2] = pItem->m_wTesseraItemID[2];
	  pNewItem->m_dwColorId = 0;

	  pNewItem->InitFuncSkill();
    // Set Ground Item Color
    //pNewItem->SetGroundItemColor();

    //////////////////////////////////////////////////////////////////////////////////
    //Add by CECE 2004-04-05
#ifdef  EVILWEAPON_3_6_VERSION
    if( (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_EVILWEAPON) 
        && pNewItem->m_pBase->m_wEvilWeapon )
        pNewItem->m_pEvilWeapon = new CEvilWeapon( pNewItem->m_pBase->m_wEvilWeapon, pOwner, pNewItem->m_iVar );
    else
        pNewItem->m_pEvilWeapon = NULL;
#endif
#ifdef _NEW_TRADITIONS_WEDDING_
    if (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_RESTOREHP)
    {
      g_pBase->GetRestoreItemInfo(pNewItem->m_pBase->m_wId,
                                  pNewItem->m_iVar[0],
                                  pNewItem->m_iVar[1],
                                  pNewItem->m_iVar[2],
                                  pNewItem->m_iVar[3]);
    }
#endif    
    ///////////////////////////////////////////////////////////////////////////////////
  }
  return pNewItem;
}
//------------------------------------------------------------------------------------------
// Construct 4
//
CItem * CGsData::CreateCItem( CSrvDropItem * pItem, CMonster * pMonster, const WORD & wCount )
{
  CItem               *pNewItem = NULL;
  CSrvBaseItem        *pBaseItem = NULL;
	static int					iCurrLevel;
	static BYTE					bLoop04;

  if( m_ListItem.empty() )
  {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
    FuncName( "CGsData::CreateCItem4" );
    _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Cannot Create CItem, List Empty ! *****", pMonster->GetName() );
    g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( g_szGsDataLog );
#endif
    return NULL;
    // Must Tack Back The Ground Item Memory
    // ...
    //pNewItem = new CItem();// CSrvBaseItem * pBaseItem, CPlayer * pOwner, const WORD & wCount );
  }
  else
  {
    g_wItemUnipueIdCounter = (++g_wItemUnipueIdCounter)%0xFFF;
    m_dwUsingItem++;
    pNewItem = (*m_ListItem.begin());
    if( pNewItem->m_dwSrvCode <= m_dwMaxItem && pNewItem->m_dwSrvCode > 0 )
    {
      m_pItemAshcan[pNewItem->m_dwSrvCode-1] = pNewItem;
    }
    else
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CGsData::CreateCItem4" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s: Monster Create CItem Error, The Item Srv Code=(%d,%d) ! *****", pMonster->GetName(), pNewItem->m_dwSrvCode, m_dwMaxItem );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
      return NULL;
    }
    m_ListItem.pop_front();
    pNewItem->m_wMemState = MEMSTATE_STATIC_USING;
    // Record Mail Id
    //pNewItem->m_dwMailId = pMonster->GetBaseId();
    // Init The Item
#ifdef _DEBUG_ITEM_CREASH_
    pNewItem->m_dwCrashData[0] = 0;
    pNewItem->m_dwCrashData[1] = 0;
#endif
    pNewItem->m_dwUniqueId[0] = ( pMonster->GetBaseId() << 16 ) |
                                ( ( g_SysTime.wYear & 0x000F ) << 12 ) |
										            ( g_wItemUnipueIdCounter & 0xFFF );
	  pNewItem->m_dwUniqueId[1] = ( g_SysTime.wMonth << 28 ) |
                                ( g_SysTime.wDay << 23 ) |
										            ( g_SysTime.wHour << 18 ) |
										            ( g_SysTime.wMinute << 13 ) |
										            ( g_SysTime.wSecond << 6 );
	  pNewItem->m_dwColorId = 0;

	  pBaseItem = pNewItem->m_pBase  = g_pBase->GetBaseItem( pItem->m_wItemId );
    if( pNewItem->m_pBase == NULL )
    {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      FuncName( "CItem::CItem_4" );
      _snprintf( g_szGsDataLog, MAX_MEMO_MSG_LEN-1, "***** %s(%d) Drop Item Data Error, The Fixed Item Id=%d *****", pMonster->GetName(), pMonster->GetBaseId(), pItem->m_wItemId );
      g_szGsDataLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( g_szGsDataLog );
#endif
      return NULL;
    }
	  for( bLoop04 = 0; bLoop04 < MAX_ITEM_SLOT; bLoop04++ )
	  {
		  pNewItem->m_pSklTessera[bLoop04] = NULL;
		  pNewItem->m_wTesseraRndNum[bLoop04] = 0;
		  pNewItem->m_wTesseraItemID[bLoop04] = 0;
	  }
    // 克制怪物的附加属性--只有在此道具上添加镶嵌物时才会增加属性, 否则和CSrvBaseItem一样
    pNewItem->m_wElement      = pBaseItem->m_wElement;
    pNewItem->m_dwRaceAttri   = pBaseItem->m_dwRaceAttri;
    pNewItem->m_dwBossCode    = pBaseItem->m_dwBossCode;
    pNewItem->m_wRaceBonuRate = pBaseItem->m_wRaceBonuRate;
    pNewItem->m_wBossBonuRate = pBaseItem->m_wBossBonuRate;

		//================= Save Data==========================
		pNewItem->m_wPosX	 = pMonster->GetPosX();
		pNewItem->m_wPosY	 = pMonster->GetPosY();
		pNewItem->m_wMapId = pMonster->GetMapId();
		pNewItem->m_wPackagePos   = 0;
#ifdef _REPAIR_SERVER_CRASH_NICK_
		memset( pNewItem->m_szNickName, 0, MAX_ITEM_NAME_LEN );
#else
		strcpy( pNewItem->m_szNickName, "" );
#endif
		pNewItem->m_dwMailId  = 0;
		pNewItem->m_dwColorId = 0;
		if( pNewItem->m_pBase->GetType() > ITEM_TYPE_UNEQUIP )
		{
			pNewItem->m_wCount    = 1;
		}
		else
		{
			pNewItem->m_wCount    = wCount;
		}
    if( pNewItem->m_wCount > pBaseItem->GetMaxWrap() )  pNewItem->m_wCount = pBaseItem->GetMaxWrap();

		pNewItem->m_iAp			       = pBaseItem->m_iAp      + pItem->m_iAp;
		pNewItem->m_iDp			       = pBaseItem->m_iDp      + pItem->m_iDp;
		pNewItem->m_iHit		       = pBaseItem->m_iHit     + pItem->m_iHit;
		pNewItem->m_iDodge	       = pBaseItem->m_iDodge   + pItem->m_iDodge;
		pNewItem->m_iInt		       = pBaseItem->m_iInt     + pItem->m_iInt;
	  pNewItem->m_iBearPsb       = pBaseItem->m_iBearPsb;// + pItem->m_iBearPsb; //

		pNewItem->m_wMaxDurability = pBaseItem->m_wMaxDurability + pItem->m_wMaxDurability;
		pNewItem->m_wHoleNumber    = pItem->m_wHoleNumber;
		pNewItem->m_wTesseraNum    = 0;
		// Ap(Item_Lv) = Ap(Item_Lv-1) + Ap_Up +［ (Item_Lv-1) / Ap_FixLv ］× Ap_Fix
		// Hm(Item_Lv) = Hm(Item_Lv-1) + Hm_Up
		// Compute Item Data Change Within Level Up
		if( pItem->m_wLevel )
		{
			for( iCurrLevel = 0; iCurrLevel < pItem->m_wLevel; iCurrLevel++ )
			{
				pNewItem->m_iAp += pItem->m_wApUp + ( pBaseItem->m_wLevel / pItem->m_wApFixLevel ) * pItem->m_wApUpFix;
				pNewItem->m_wMaxDurability += pItem->m_wMaxDurability;
			}
		}
		pNewItem->m_wLevel	      = pBaseItem->m_wLevel + pItem->m_wLevel;
		pNewItem->m_wDurability   = pNewItem->m_wMaxDurability;
		pNewItem->m_wDurabilityUp = pBaseItem->m_wDurabilityUp + pItem->m_wDurability;

		pNewItem->m_wApUp		= pBaseItem->m_wApUp + pItem->m_wApUp;
		pNewItem->m_wDpUp		= pBaseItem->m_wDpUp + pItem->m_wDpUp;
		pNewItem->m_wDgeUp	= pBaseItem->m_wDgeUp + pItem->m_wDodgeUp;
		pNewItem->m_wHitUp	= pBaseItem->m_wHitUp + pItem->m_wHitUp;
		pNewItem->m_wIntUp	= pBaseItem->m_wIntUp + pItem->m_wIntUp;

		pNewItem->m_wApFixLevel			= pBaseItem->m_wApFixLevel + pItem->m_wApFixLevel;
		pNewItem->m_wDpFixLevel			= pBaseItem->m_wDpFixLevel + pItem->m_wDpFixLevel;
		pNewItem->m_wHitFixLevel		= pBaseItem->m_wHitFixLevel + pItem->m_wHitFixLevel;
		pNewItem->m_wDodgeFixLevel	= pBaseItem->m_wDodgeFixLevel + pItem->m_wDodgeFixLevel;
		pNewItem->m_wIntFixLevel		= pBaseItem->m_wIntFixLevel + pItem->m_wIntFixLevel;

		pNewItem->m_wApFix		= pBaseItem->m_wApFix + pItem->m_wApUpFix;
		pNewItem->m_wDpFix		= pBaseItem->m_wDpFix + pItem->m_wDpUpFix;
		pNewItem->m_wHitFix		= pBaseItem->m_wHitFix + pItem->m_wHitUpFix;
		pNewItem->m_wDodgeFix	= pBaseItem->m_wDodgeFix + pItem->m_wDodgeUpFix;
		pNewItem->m_wIntFix		= pBaseItem->m_wIntFix + pItem->m_wIntUpFix;

		pNewItem->m_wCriticalHit = pBaseItem->m_wCriticalHit + pItem->m_wCriticalHit;

		pNewItem->m_wForging			= pBaseItem->m_wForging + pItem->m_wForging;
		pNewItem->m_wForgingDblUp	= pBaseItem->m_wForgingDblUp + pItem->m_wForgingDblUp;

		pNewItem->m_wStable	= pBaseItem->m_wStable + pItem->m_wStable;
		pNewItem->m_wHard	  = pBaseItem->m_wHard + pItem->m_wHard;

		memset( pNewItem->m_iVar, 0, sizeof( int ) * 4 );

		pNewItem->m_iAlarmTime		= pItem->m_iAlarmTime;
		pNewItem->m_wFuncAlarm		= pItem->m_wFuncAlarm;
		pNewItem->m_iTriggerTime	= pItem->m_iTriggerTime;
		pNewItem->m_wFuncTrigger	= pItem->m_wFuncTrigger;

		if( pItem->m_iEffectTime )	pNewItem->m_iEffectTime		= pItem->m_iEffectTime;
		else												pNewItem->m_iEffectTime		= pBaseItem->m_iEffectTime;
		if( pItem->m_wFuncEffect )	pNewItem->m_wFuncEffect		= pItem->m_wFuncEffect;
		else												pNewItem->m_wFuncEffect		= pBaseItem->m_wFuncEffect;
		if( pItem->m_wFuncDbc )			pNewItem->m_wFuncDbc			= pItem->m_wFuncDbc;
		else												pNewItem->m_wFuncDbc			= pBaseItem->m_wFuncDbc;
		if( pItem->m_wFuncEquip )		pNewItem->m_wFuncEquip		= pItem->m_wFuncEquip;
		else												pNewItem->m_wFuncEquip		= pBaseItem->m_wFuncEquip;
		if( pItem->m_wFuncTakeOff )	pNewItem->m_wFuncTakeOff	= pItem->m_wFuncTakeOff;
		else												pNewItem->m_wFuncTakeOff	= pBaseItem->m_wFuncTakeOff;
		if( pItem->m_wFuncTouch )		pNewItem->m_wFuncTouch		= pItem->m_wFuncTouch;
		else												pNewItem->m_wFuncTouch		= pBaseItem->m_wFuncTouch;
		pNewItem->InitFuncSkill();
    // Set Ground Item Color
    pNewItem->SetGroundItemColor();
    
    //////////////////////////////////////////////////////////////////////////////////
    //Add by CECE 2004-04-05
#ifdef  EVILWEAPON_3_6_VERSION
    if( (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_EVILWEAPON) 
        && pNewItem->m_pBase->m_wEvilWeapon )
        pNewItem->m_pEvilWeapon = new CEvilWeapon( pNewItem->m_pBase->m_wEvilWeapon, pMonster, pNewItem->m_iVar );
    else
        pNewItem->m_pEvilWeapon = NULL;
#endif
#ifdef ALL_SPECIAL_ITEM_FUNC
    if( pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_SAVEPOINT)
    {
      pNewItem->m_iVar[0] = g_pBase->GetSavePoinItemUseCount(pNewItem->m_pBase->m_wId);
      pNewItem->m_iVar[1] = g_pBase->GetSavePoinItemUseOdds(pNewItem->m_pBase->m_wId);
    }
#ifdef _MUSIC_BOX_
    if( pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_MUSICBOX )
    {
      pNewItem->m_iVar[0] = g_pBase->GetMusicBoxSongNum(pNewItem->m_pBase->m_wId);
    }
#endif
#endif

#ifdef _NEW_TRADITIONS_WEDDING_
    if (pNewItem->m_pBase->m_dwSpecialAttr & ITEMX_PROPERTY_RESTOREHP)
    {
      g_pBase->GetRestoreItemInfo(pNewItem->m_pBase->m_wId,
                                  pNewItem->m_iVar[0],
                                  pNewItem->m_iVar[1],
                                  pNewItem->m_iVar[2],
                                  pNewItem->m_iVar[3]);
    }
#endif
    ///////////////////////////////////////////////////////////////////////////////////
	}
  return pNewItem;
}
//===========================================================================================================
//
//
void CGsData::SendMcc_AC_LOG_MONSTERDROP()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( g_wMstDropNum && g_wMstDropNum < 30 )
  {
    SMccMsgData     *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )      return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_MONSTERDROP;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SMonsterDropItem) * g_wMstDropNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_MonsterDropMsg.Msgs[0].Data,
            sizeof( SMonsterDropItem ) * g_wMstDropNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    //
    g_wMstDropNum = 0;
    g_pMstDropLog = (SMonsterDropItem*)(g_MonsterDropMsg.Msgs[0].Data);
  }
}
//===========================================================================================================
//
//
void CGsData::SendMcc_AC_LOG_ITEMMIX()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( g_wMixItemNum && g_wMixItemNum < 30 )
  {
    SMccMsgData   *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )    return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_ITEMMIX;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SItemMix) * g_wMixItemNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_MixItemMsg.Msgs[0].Data, sizeof( SItemMix ) * g_wMixItemNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit
    g_wMixItemNum = 0;
    g_pMixItemLog = (SItemMix*)(g_MonsterDropMsg.Msgs[0].Data);
  }
}
//===========================================================================================================
//
//
void CGsData::SendMcc_AC_LOG_ITEMTESSERA()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( g_wTesseraItemNum && g_wTesseraItemNum < 20 )
  {
    SMccMsgData *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )  return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_ITEMTESSERA;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SItemTessera) * g_wTesseraItemNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_TesseraItemMsg.Msgs[0].Data,
            sizeof(SItemTessera) * g_wTesseraItemNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit
    g_wTesseraItemNum = 0;
    g_pTesseraItemLog = (SItemTessera*)(g_TesseraItemMsg.Msgs[0].Data);
  }
}
//===========================================================================================================
//
//
void CGsData::SendMcc_AC_LOG_ITEMBLESS()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( g_wBlessItemNum && g_wBlessItemNum < 20 )
  {
    SMccMsgData   *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )          return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_ITEMBLESS;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SItemBless) * g_wBlessItemNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_BlessItemMsg.Msgs[0].Data,
            sizeof(SItemBless) * g_wBlessItemNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit
    g_wBlessItemNum = 0;
    g_pBlessItemLog = (SItemBless*)(g_BlessItemMsg.Msgs[0].Data);
  }
}
//===========================================================================================================
//
//
void CGsData::SendMcc_AC_LOG_PLAYERALTER()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( g_wAlterItemNum && g_wAlterItemNum < 30 )
  {
    SMccMsgData   *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )    return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_PLAYERALTER;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof( SPlayerAlterLog ) * g_wAlterItemNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_AlterItemMsg.Msgs[0].Data,
            sizeof( SPlayerAlterLog ) * g_wAlterItemNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit
    g_wAlterItemNum = 0;
    g_pAlterItemLog = (SPlayerAlterLog*)(g_AlterItemMsg.Msgs[0].Data);
  }
}

//===========================================================================================================
//  MICHAEL CREATED
//
void CGsData::SendMcc_AC_LOG_GUILDINFO()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( 0 != g_wGuildInfoLogNum && g_wGuildInfoLogNum < 20 )
  {
    SMccMsgData   *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )          return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_GUILD;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SGuildInfoLog) * g_wGuildInfoLogNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_GuildInfoLogMsg.Msgs[0].Data,
            sizeof(SGuildInfoLog) * g_wGuildInfoLogNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit
    g_wGuildInfoLogNum = 0;
    g_pGuildInfoLog = (SGuildInfoLog*)(g_GuildInfoLogMsg.Msgs[0].Data);
  }
}

//===========================================================================================================
//  MICHAEL CREATED
//
void CGsData::SendMcc_AC_LOG_PICKDROPMONEY()
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( GetMccMode() != 2 )      return;
  //
  if( 0 != g_wPickDropMoneyNum && g_wPickDropMoneyNum < 20 )
  {
    SMccMsgData   *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if( pNewMccMsg == NULL )          return;
    //
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_PICKDROP_MONEY;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SPickDropMoney) * g_wPickDropMoneyNum;
    //
    memcpy( pNewMccMsg->Msgs[0].Data, g_PickDropMoneyMsg.Msgs[0].Data,
            sizeof(SPickDropMoney) * g_wPickDropMoneyNum );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit
    g_wPickDropMoneyNum = 0;
    g_pPickDropMoneyLog = (SPickDropMoney*)(g_PickDropMoneyMsg.Msgs[0].Data);
  }
}

void CGsData::AddMaxLoginReady(int nNumber)
{
	m_iMaxLoginReady+=nNumber ;
}

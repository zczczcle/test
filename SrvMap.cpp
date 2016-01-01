//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Copyright( C ) 2001 Waei Corporation.  All Rights Reserved.
//
//	Author	: DingDong Lee 
//	Desc	  : Handle Map Info, Data And All Monster, Npc Initialize
//	Date	  : 2001-4-19
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SrvMap.h"

#include "../Life/SrvLife.h"
#include "../Life/SrvMonster.h"
#include "../Life/SrvPlayer.h"
#include "../Life/SrvNpc.h"
#include "../Item/SrvItem.h"
#include "../Skill/SrvSkill.h"
#include "../../Network/SrvClientData.h"
#include "../Guild/SrvGuild.h"
#include "../../Mcc/SrvMccInfo.h"
#pragma warning(disable:4800)
#ifdef _DEBUG_CHECK_ACTION_STATE_
char              szActionCode[MAX_ACCOUNT_LEN] = "WILDCAT\0";
static WORD       ThisCodeCount = 0;
extern CLogFile   g_Action;
#endif

extern CMccInfo     *g_pMccDB,*g_pMccChat;
static char szMapLog[MAX_MEMO_MSG_LEN];
// 0: 左上方;   1: 右上方
static int  g_iCityGateObstacleX[2][14] =
{
  {  3,  2,  1,  0, -1, -2, -3,  3,  2,  1,  0, -1, -2, -3 },
  { -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0 },
};
static int  g_iCityGateObstacleY[2][14] =
{
  { -1, -1, -1, -1, -1, -1, -1,  0,  0,  0,  0,  0,  0,  0 },
  {  3,  2,  1,  0, -1, -2, -3,  3,  2,  1,  0, -1, -2, -3 },
};

//======
#ifdef _NEW_CITY_WAR_2005_
#define PER_UNIT_TIME_ADD 237
#endif
#ifdef VERSION_40_HOUSE_FUNCTION
CHouseMgr *g_pHouseMgr;
#endif
////////////////////////////////////////////////////////////////////////////////////////
//
// Define For New Function
#define _WILDCAT_ARRAY_MONSTER_NPC_LIST_
DWORD		g_dwMonsterTotal = 0;
////////////////////////////////////////////////////////////////////////////////////////
// Base Map
CSrvBaseMap::CSrvBaseMap(): m_dwMapId(0), m_dwAreaId(0), m_iMapState(0), m_iX(0), m_iY(0), m_pOrg(NULL)
{
#ifdef _REPAIR_SERVER_CRASH_NICK_
  memset( m_szMapName, 0, MAX_MAP_NAME_LEN );
  memset( m_szContent, 0, MAX_MAP_CONTENT_LEN );
#else	
  strcpy( m_szMapName, "" );
  strcpy( m_szContent, "" );
#endif
  memset( m_listAdjustPoint, NULL, sizeof( m_listAdjustPoint ) );
  ////////////////////////////////////////////////////////////////
  //Add by CECE 2004-08-05
#ifdef ELYSIUM_3_7_VERSION
  SetViewMatch( FALSE );
#endif
  ////////////////////////////////////////////////////////////////
}
//=====================================================================================
//
//
CSrvBaseMap::~CSrvBaseMap()
{
  if( m_pOrg )
  {
    delete[] m_pOrg;
  }
  m_mapWarpPoint.clear();
}
//=====================================================================================
//
//
bool CSrvBaseMap::Init(const char *szMapPath, const DWORD & dwMapID)
{
  FuncName("CSrvBaseMap::Init");
  
  // Read the Mps file of the map ---------------------------------
  ifstream  FileIn(szMapPath, ios::in|ios::binary);
  char      cTemp = '\0';
  char      szBuffer[MAX_READFILE_BUFFER];
  int       iArea;
  //int				iFlagCount = 0;
  
  memset(szBuffer, 0, MAX_READFILE_BUFFER);
  
  if( !FileIn.is_open() )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_		
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Find The Mps File: %s", szMapPath );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szMapLog );
#endif
    return false;
  }
  
  // Read header
  SMpsFileHeader TheHeader;
  
  if( !FileIn.read( (char*)(&TheHeader), sizeof(SMpsFileHeader) ) )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Invalid Mps File: %s", szMapPath);
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szMapLog );
#endif
    return false;
  }
  if( strcmp( (char*)(&(TheHeader.Marker)), "MAP" ) != 0 )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Invalid Header Of The Mps File [%s]", szMapPath );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szMapLog );
#endif
    return false;
  }
  memcpy( m_szMapName, TheHeader.Filename, MAX_MAP_NAME_LEN );
  m_szMapName[MAX_MAP_NAME_LEN-1] = '\0';
  memcpy( m_szContent, TheHeader.Filecomment, MAX_MAP_CONTENT_LEN );
  m_szContent[MAX_MAP_CONTENT_LEN-1] = '\0';
  
  // Get map information
  //m_dwMapId = TheHeader.Id;
  m_dwMapId  = dwMapID;
  m_dwAreaId = TheHeader.AREAID;
  //dwVersion = ??? ;
  m_pOrg = NULL;
  m_iX = TheHeader.Width + TheHeader.Height/2;
  m_iY = m_iX;
  iArea = m_iX * m_iY;
  
  // for temp +2
  m_iCliWidth  = TheHeader.Width;
  m_iCliHeight = TheHeader.Height;
  
  m_pOrg = new char[iArea];
  for( int j = 0; j < iArea; j++ )
  {
    if( !FileIn.eof() )
    {
      FileIn.get( cTemp );
      m_pOrg[j] = cTemp;
    }
    else
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Invalid size of mps file: %s", szMapPath );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( szMapLog );
#endif
      return false;
    }
  }
  //
  for( int kkk = 0; kkk < m_iX; kkk++ )
  {
    for( int hhh = 0; hhh < m_iY; hhh++ )
    {
      if( kkk == 0 || hhh == 0 )  m_pOrg[hhh*m_iX+kkk] |= 0x80;
    }
  }
  //
  //if( iFlagCount )
  //{
  //	sprintf( szMapLog, "***** The Map[Id=%d] Have %d Errors ! *****", m_dwMapId, iFlagCount );
  //  AddMemoErrMsg( szMapLog );
  //}
  
  FileIn.close();
  _snprintf(szMapLog, MAX_MEMO_MSG_LEN-1, "Load Map Ok (%s, ID=%d, %dx%d) [%s]", m_szMapName, m_dwMapId, m_iX, m_iY, szMapPath);
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(szMapLog);
  
  // Read the Opf file of the map ---------------------------------
#ifdef _REPAIR_SERVER_CRASH_NICK_
  SafeStrcpy( szBuffer, szMapPath, MAX_READFILE_BUFFER );
#else
  strcpy(szBuffer, szMapPath);
#endif
  int iLen = strlen(szBuffer);
  szBuffer[iLen-3] = 'o';
  szBuffer[iLen-2] = 'p';
  szBuffer[iLen-1] = 'f';
  
  ifstream  OpfFileIn(szBuffer, ios::in|ios::binary);
  if(!OpfFileIn.is_open())
  { // cannot find the Opf file of the map --> the map does not have any warp points
    return true;
  }
  
  // Read header
  SOpfFiIeHeader TheOpfHeader;
  
  if(!OpfFileIn.read((char*)(&TheOpfHeader), sizeof(SOpfFiIeHeader)))
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Invalid Opf file: %s", szBuffer );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    return false;
  }
  if(strcmp((char*)(&(TheOpfHeader.Marker)), "OPF") != 0)
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Invalid header of the Opf file [%s]", szBuffer );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    return false;
  }
  
  // Read Data
  SWarpPos      TheWarpPos;
  SWarpPoint    *pWarpPoint = NULL;
  DWORD dwTheX_Y = 0;
  int iCount = 0;
  
  // For Test
  AddMemoMsg("**********************************************************************");
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Map ID = %d, Warp Point : *****", m_dwMapId );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg(szMapLog);
  
  int iLdd = 1;
  while(OpfFileIn.read((char*)(&TheWarpPos), sizeof(SWarpPos)))
  {
    //
    // For Temp Begin WildCat
    int iX = TheWarpPos.x, iY =TheWarpPos.y, iToX = TheWarpPos.tox, iToY = TheWarpPos.toy;
    int iTmpMapId = TheWarpPos.MapId;
    // For Temp End
    //
    dwTheX_Y = (TheWarpPos.x << 16) | (TheWarpPos.y & 0x0000FFFF);
    // For Test
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Warp Point No.%d : From (%d,%d) To Map(%d) Tile(%d,%d) ***", iLdd++, TheWarpPos.x, TheWarpPos.y, TheWarpPos.MapId, TheWarpPos.tox, TheWarpPos.toy );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg(szMapLog);
    //
    if( m_mapWarpPoint.find( dwTheX_Y ) != m_mapWarpPoint.end() )
    {
      m_mapWarpPoint.clear();
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "******The position(%d,%d,Map=%d) has two warp points in the Opf file [%s] !#", TheWarpPos.x, TheWarpPos.y, TheWarpPos.MapId, szBuffer );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( szMapLog );
#endif
    }
    if( NULL == ( pWarpPoint = new SWarpPoint ) )
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      AddMemoErrMsg( "Cannot New SWarpPoint Struct..." );
#endif
      return false;
    }
    pWarpPoint->wCode         = 0;
    pWarpPoint->wMapId        = iTmpMapId;
    pWarpPoint->wPosX         = TheWarpPos.x;
    pWarpPoint->wPosY         = TheWarpPos.y;
    pWarpPoint->wTargetMapId  = TheWarpPos.MapId;
    pWarpPoint->wTargetMapX   = TheWarpPos.tox;
    pWarpPoint->wTargetMapY   = TheWarpPos.toy;
    pWarpPoint->wType         = WARPPOINT_TYPE_MIN;
    pWarpPoint->wState        = WARPPOINT_STATE_ON;
    pWarpPoint->dwLifeTime    = 0;
    //
    m_mapWarpPoint.insert( map<DWORD, SWarpPoint*>::value_type( dwTheX_Y, pWarpPoint ) );
    iCount++;
  }
  if( iCount != TheOpfHeader.wListCount )
  {
    // release the list of warp points
    m_mapWarpPoint.clear();
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Count Of The Warp Points(Map=%d) Is Incorrect In The Opf File [%s] !!! *****", pWarpPoint->wTargetMapId, szBuffer );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szMapLog );
    AddMemoErrMsg( szMapLog );
#endif
    return false;
  }
  
  OpfFileIn.close();
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Load Opf File [%s] OK ! *****", szBuffer );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoMsg( szMapLog );
  
  InitAdjustPoint();
  return true;
}
//=====================================================================================
//
//Note:
BOOL CSrvBaseMap::AddWarpPoint( SWarpPoint * pTheWarp, const DWORD & dwX_Y )
{
  /////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-20
  //if( pTheWarp->wTargetMapId == m_dwMapId )
  /////////////////////////////////////////////////////////////////////////
  {
    if( m_mapWarpPoint.find( dwX_Y ) == m_mapWarpPoint.end() )
    {
      m_mapWarpPoint.insert( map<DWORD, SWarpPoint*>::value_type( dwX_Y, pTheWarp ) );
      return TRUE;
    }
  }
  return FALSE;
}
//=====================================================================================
//
//Note:
BOOL CSrvBaseMap::DeleteWarpPoint( const DWORD & dwX_Y )
{
  m_Iter_Wp = m_mapWarpPoint.find( dwX_Y );
  if( m_Iter_Wp != m_mapWarpPoint.end() )
  {
    SAFE_DELETE( m_Iter_Wp->second );
    m_mapWarpPoint.erase( m_Iter_Wp );
    return TRUE;
  }
  return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Game Map
CGameMap::CGameMap()
{
  //////////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-15
#ifdef EVILWEAPON_3_6_VERSION
  m_dwWarpTickCount = 0;
#endif
  //////////////////////////////////////////////////////////////////////////////
  
  m_mapCodePlayer.clear();
  m_mapCodeMonster.clear();
  //
  m_mapDisableMonster.clear();
  m_mapDefenceMonster.clear();
  m_mapGuildMonster.clear();
  m_listCityGate.clear();
#ifdef _NEW_CITY_WAR_2005_
  m_listCityWarMon.clear();
  m_mapGiftItemCount.clear();
#endif
#ifdef VERSION_38_FUNCTION
  m_listTrapMagic.clear();
#endif
  m_mapRandomMonster.clear();
  //
  m_mapCodeNpc.clear();
  m_mapCodeGroundItem.clear();
  m_mapCodeMagic.clear();
  memset( m_listAdjustPoint, NULL, sizeof( int ) * MAX_ADJUST_POINT );
  // About Monster Team Info
  m_pMonsterTeamList = NULL;
  m_pMapOrg = NULL;
  // Add By WildCat
  m_CodeMonster.Init(CODE_MIN_MONSTER,CODE_MAX_MONSTER);
  
  m_CodeNpc.Init(CODE_MIN_NPC,CODE_MAX_NPC);
  
  m_CodeGroundItem.Init(CODE_MIN_GROUNDITEM,CODE_MAX_GROUNDITEM);
  
  m_CodeMagic.Init(CODE_MIN_MAGIC,CODE_MAX_MAGIC);
  
  m_CodeWarp.Init(CODE_MIN_WARP,CODE_MAX_WARP);
  // Weather
  m_dwWeatherTrigger = TimeGetTime() + MAP_WEATHER_CHANGE_TIME;
  m_iWeatherState		 = MAP_WEATHER_NONE;
  m_dwMapType				 = MAP_TYPE_NONE;
  //memset( &m_stMapDate, 0, sizeof(SYSTEMTIME));
  m_dwViveMonster    = 0;
  m_dwGetInfoTime		 = 0;
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
  m_dwNextFerryStateTime  = 0x7FFFFFFF;
  m_iFerryState      = BOAT_STATE_WAIT;
  m_dwMonsterAttackFerryTime = 0x7FFFFFFF;
#endif
  m_bNewPathFind     = FALSE;
  m_dwRunType        = MAP_RUNTYPE_NORMAL;
  m_dwCityTpe        = 0;
  // About Npc
  m_listDisappearNpc.clear();
  
  // About New Version 1.0025 Life List And Ground Item Lsit
  m_PlayerCodeList  = NULL;
  m_MonsterCodeList = NULL;
  m_dwMonsterCount  = 0;
  m_NpcCodeList			= NULL;
  m_dwNpcCount			= 0;
  m_pTileGroundItem = NULL;
  m_wMaxTeamId			= 0;
  // About City War And City Manager
  m_dwTaxIncome     = 0;
  m_wTaxRate        = 0;
  m_pOwnerGuild     = NULL;
  m_pOccupyMonster  = NULL;
  m_wMaxMercenary   = 0;
  m_wNowMercenary   = 0;
#ifdef _NEW_CITY_WAR_2005_
  m_dwSaveGiftItemTrigger[0] = 0;
  m_dwSaveGiftItemTrigger[1] = 0;
#endif
  
#ifdef _NEW_TRADITIONS_WEDDING_
  m_bAwaneMap = FALSE;
  m_bMarryFateMap = FALSE;
  m_dwAwaneLastTick = TimeGetTime();
#endif
  m_wLastViewMonCode = 0;
}
//=====================================================================================
//
//
CGameMap::~CGameMap()
{
  if( m_pMapOrg ) 
  {
    delete[] m_pMapOrg;
    m_pMapOrg = NULL;
  }
  if( m_pMapTilePlayer )
  {
    delete[] m_pMapTilePlayer;
    m_pMapTilePlayer = NULL;	
  }
  //if( m_MonsterCodeList )
  //{
  //  delete[] m_MonsterCodeList;
  //	m_MonsterCodeList = NULL;
  //}
  if( m_NpcCodeList )
  {
    delete[] m_NpcCodeList;
    m_NpcCodeList = NULL;
  }
  if( m_pMonsterTeamList )
  {
    delete[] m_pMonsterTeamList;
    m_pMonsterTeamList = NULL;
  }
  
  ClearTileLife();
  ClearPlayer();
  ClearMonster();
  ClearNpc();
  ClearGroundItem();
  
  m_mapCodePlayer.clear();
  m_mapCodeMonster.clear();
#ifdef _NEW_CITY_WAR_2005_
  m_listCityWarMon.clear();
  m_mapGiftItemCount.clear();
#endif
#ifdef VERSION_38_FUNCTION
  m_listTrapMagic.clear();
#endif
  m_mapCodeNpc.clear();
  m_mapCodeGroundItem.clear();
  m_mapCodeMagic.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////
//
//---------- Add By WildCat Code 2002-4-25 begin for clear all data
//
void CGameMap::ClearPlayer()
{
  m_mapCodePlayer.clear();	
}
//=====================================================================================
//
//
void CGameMap::ReviveOccupyMonster()
{
  if( m_pOccupyMonster )
  {
    m_pOccupyMonster->Revive();
    if( m_pOccupyMonster->GetEventNpc() )   NpcDisappear( (m_pOccupyMonster->GetEventNpc())->GetCode() );
  }
}
//=====================================================================================
//
//
#define _DEBUG_SHOW_WARP_ENEMY_COUNT_
void CGameMap::WarpAllEnemy( const WORD & wDesMap, const DWORD & dwSelfGuildId )
{
  map<DWORD, CPlayer*>::iterator    Iter_Pl;
  SWarpPoint                        WarpPoint;
  CPlayer                           *pPlayer   = NULL;
  CSrvBaseMap                       *pGameMap  = NULL;
  SAdjustPoint                      *pWarpCoor = NULL;
  CGuild                            *pTheGuild = NULL;
  WORD                              wLoop = 0, wCount = 0, wCount_3 = 0;
  //
  if( NULL == ( pGameMap = g_pBase->GetBaseMap( wDesMap ) ) )     return;
  //
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pPlayer   = Iter_Pl->second;
    pTheGuild = pPlayer->GetGuild();
    if( pPlayer->GetStatus() == STATUS_PLAYER_DEAD )
    {
      g_pMultiSendPlayer_3[wCount_3] = pPlayer;
      wCount_3++;
    }
    // Warp The Leagueless Guild Player
    else if( dwSelfGuildId == 0 ||
      pTheGuild == NULL ||
      (pTheGuild->GetGuildId() != dwSelfGuildId &&
      (!pTheGuild->IsLeague( dwSelfGuildId ))) )
    {
      if( pWarpCoor = pGameMap->GetAdjustPoint() )
      {
        WarpPoint.wTargetMapId = wDesMap;
        WarpPoint.wTargetMapX	 = pWarpCoor->wAdjustX;
        WarpPoint.wTargetMapY	 = pWarpCoor->wAdjustY;
        //
        pPlayer->SetWarpPoint( &WarpPoint );
        g_pMultiSendPlayer_2[wCount] = pPlayer;
        wCount++;
      }
    }
  }
  //
  for( wLoop = 0; wLoop < wCount; wLoop++ )
  {
    if( g_pMultiSendPlayer_2[wLoop] )
    {
      g_pMultiSendPlayer_2[wLoop]->Send_A_WARP( *(g_pMultiSendPlayer_2[wLoop]->GetSelfWarpPoint()), PLAYER_WARP_TYPE_NPC );
    }
  }
  //
  for( wLoop = 0; wLoop < wCount_3; wLoop++ )
  {
    if( g_pMultiSendPlayer_3[wLoop] )
    {
      g_pMultiSendPlayer_3[wLoop]->Revive( REVIVE_GO_BACK_TOWM, 0 );
    }
  }
}
//=====================================================================================
//
//add by zetorche 
void CGameMap::WarpAll( const WORD & wDesMap)
{
  map<DWORD, CPlayer*>::iterator    Iter_Pl;
  SWarpPoint                        WarpPoint;
  CPlayer                           *pPlayer   = NULL;
  CSrvBaseMap                       *pGameMap  = NULL;
  SAdjustPoint                      *pWarpCoor = NULL;
  WORD                              wLoop = 0, wCount = 0, wCount_3 = 0;
  //
  if( NULL == ( pGameMap = g_pBase->GetBaseMap( wDesMap ) ) )     return;
  //
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pPlayer   = Iter_Pl->second;
    if( pPlayer->GetStatus() == STATUS_PLAYER_DEAD )
    {
      g_pMultiSendPlayer_3[wCount_3] = pPlayer;
      wCount_3++;
    }
    // Warp The Leagueless Guild Player
    else
    {
      if( pWarpCoor = pGameMap->GetAdjustPoint() )
      {
        WarpPoint.wTargetMapId = wDesMap;
        WarpPoint.wTargetMapX	 = pWarpCoor->wAdjustX;
        WarpPoint.wTargetMapY	 = pWarpCoor->wAdjustY;
        pPlayer->SetWarpPoint( &WarpPoint );
        g_pMultiSendPlayer_2[wCount] = pPlayer;
        wCount++;
      }
    }
  }
  //
  for( wLoop = 0; wLoop < wCount; wLoop++ )
  {
    if( g_pMultiSendPlayer_2[wLoop] )
    {
      g_pMultiSendPlayer_2[wLoop]->Send_A_WARP( *(g_pMultiSendPlayer_2[wLoop]->GetSelfWarpPoint()), PLAYER_WARP_TYPE_NPC );
    }
  }
  //
  for( wLoop = 0; wLoop < wCount_3; wLoop++ )
  {
    if( g_pMultiSendPlayer_3[wLoop] )
    {
      g_pMultiSendPlayer_3[wLoop]->Revive( REVIVE_GO_BACK_TOWM, 0 );
    }
  }
}
//
void CGameMap::Send_A_ADJUST_TAXRATE()
{
  SMsgData            *pNewMsg = g_pGs->NewMsgBuffer();
  //
  if( pNewMsg )
  {
    pNewMsg->Init();
    pNewMsg->dwAID        = A_ADJUST_TAXRATE;
    pNewMsg->dwMsgLen     = 1;
    pNewMsg->Msgs[0].Size = sizeof( SNMAdjustTaxRate );
    SNMAdjustTaxRate      *pAdjustTaxRate = (SNMAdjustTaxRate*)(pNewMsg->Msgs[0].Data);
    //
    pAdjustTaxRate->wCityMapId = GetMapId();
    pAdjustTaxRate->wNpcCode   = 0;
    pAdjustTaxRate->wTaxRate   = GetTaxRate();
    //
    SendTheMsgToAll( pNewMsg );
  }
}
//=====================================================================================
//
//
BOOL CGameMap::FindGuildPlayer( CGuild * pGuild )
{
  if( pGuild == NULL )      return FALSE;
  //
  CGuild                          *pTempGuild = NULL;
  map<DWORD,CPlayer*>::iterator   Iter_Pl;
  //
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pTempGuild = (Iter_Pl->second)->GetGuild();
    if( pTempGuild &&
      pTempGuild->GetGuildId() == pGuild->GetGuildId() )
    {
      return TRUE;
    }
  }
  return FALSE;
}
//=====================================================================================
//
//
void CGameMap::NotifyGuildDefenceMonster( CPlayer * pAttacker )
{
  static map<DWORD, CMonster*>::iterator	  Iter_Gdm;
  //
  if( m_mapGuildMonster.empty() )           return;
  //
  CMonster                                  *pMonster = NULL;
  for( Iter_Gdm = m_mapGuildMonster.begin(); Iter_Gdm != m_mapGuildMonster.end(); Iter_Gdm++ )
  {
    pMonster = (Iter_Gdm->second);
    if( pMonster->InAttackRange( pAttacker ) )
    {
      pMonster->WillAttackTarget( pAttacker );
    }
  }
}
//=====================================================================================
//
//
void CGameMap::NotifyDefenceMonster( CPlayer * pAttacker )
{
  static map<DWORD, CMonster*>::iterator	  Iter_Dfm;
  CGuild                                    *pGuild = pAttacker->GetGuild();
  //
  if( m_mapDefenceMonster.empty() )     return;
  //
  CMonster                                  *pMonster = NULL;
  for( Iter_Dfm = m_mapDefenceMonster.begin(); Iter_Dfm != m_mapDefenceMonster.end(); Iter_Dfm++ )
  {
    pMonster = (Iter_Dfm->second);
    if( pGuild && pMonster->GetType() & NPC_ATTRI_ATTACK_CITY )
    {
      WORD          wBaseCity = pGuild->GetLeagueBelongCity();
      if( wBaseCity && wBaseCity == g_pGuildMng->GetBaseCityWarMap() && pMonster->InAttackRange( pAttacker ) )
      {
        pMonster->WillAttackTarget( pAttacker );
      }
    }
    else if( pMonster->GetType() & NPC_ATTRI_PROTECT_PLAYER )
    {
      if( pMonster->InAttackRange( pAttacker ) )
      {
        pMonster->WillAttackTarget( pAttacker );
      }
    }
  }
}
//=====================================================================================
//
//
void CGameMap::ClearTileLife()
{
  map<DWORD, LPLifeTileList>::iterator				lIte;
  LifeTileList::iterator											tIte;
  LPLifeTileList															LifeList = NULL;
  
  for( lIte = m_mapTilePlayer.begin(); lIte != m_mapTilePlayer.end(); )
  {
    LifeList = lIte->second;
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
    g_CrashLog.Write( "CGameMap::ClearTileLife ===>> 111" );
#endif
    lIte = m_mapTilePlayer.erase( lIte );
    if( LifeList )
    {
      LifeList->clear();
      SAFE_DELETE( LifeList );
    }
  }
  m_mapTilePlayer.clear();
}
//=====================================================================================
//
//
void CGameMap::ClearMonster()
{
  map<DWORD,CMonster*>::iterator			mIte;
  CMonster*														pMonster = NULL;
  //
  for( mIte = m_mapCodeMonster.begin(); mIte != m_mapCodeMonster.end(); )
  {
    pMonster = (CMonster*)mIte->second;
    mIte = m_mapCodeMonster.erase( mIte );
    m_mapDefenceMonster.erase( pMonster->GetCode() );
    //
    SAFE_DELETE( pMonster );
  }	
  m_mapCodeMonster.clear();
  m_mapDefenceMonster.clear();
}
//=====================================================================================
//
//
void CGameMap::ClearNpc()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::ClearNpc()");
#endif
  
  map<DWORD,CNpc*>::iterator	nIte;
  CNpc*												pNpc = NULL;
  
  for(nIte=m_mapCodeNpc.begin();nIte!=m_mapCodeNpc.end();)
  {
    pNpc = (CNpc*)nIte->second;
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
    g_CrashLog.Write( "CGameMap::ClearNpc ===>> 111" );
#endif
    nIte = m_mapCodeNpc.erase(nIte);
    SAFE_DELETE( pNpc );
  }	
  m_mapCodeNpc.clear();
}
//=====================================================================================
//
//
void CGameMap::ClearGroundItem()
{
  
}
//=====================================================================================
//
//
void CGameMap::ClearMagic()
{
  
}
//=====================================================================================
//
//
bool CGameMap::Init(int iTheMapId)
{
  FuncName("CGameMap::Init");
  int i, j;
  int iFlagCount = 0;
  
#ifdef _DEBUG
  if( iTheMapId == 1400 )
  {
    int iFuckCrash = 0;
    iFuckCrash = 1;
  }
#endif
  m_pBase = g_pBase->GetBaseMap( iTheMapId );

  if( !m_pBase )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Find The Map ID=%d, When Init CGameMap #", iTheMapId );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    return false;
  }
  //
  m_pBase->SetMapState( 1 );
  // Add By WildCat 2002-5-29 For Load the map type
  SGameMapType		*pMapType = g_pBase->GetMapType( iTheMapId );
  if( pMapType == NULL )
  {
    m_dwMapType       = 0;
    m_dwWeatherRandom = 50;
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Get The Map Type Of ID=%d When Init CGameMap #", iTheMapId );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    return false;
  }
  else
  {
    m_dwMapType         = pMapType->dwType;
    m_dwWeatherRandom   = pMapType->dwWeatherRandom;
    m_wAttribute        = pMapType->wAttribute;
    m_wReviveMap        = pMapType->wReviveMap;
    m_wReviveX          = pMapType->wReviveX;
    m_wReviveY          = pMapType->wReviveY;
    //
    m_wDefenceReviveMap = pMapType->wDefenceReviveMap;
    m_wDefenceReviveX   = pMapType->wDefenceReviveX;
    m_wDefenceReviveY   = pMapType->wDefenceReviveY;
    m_wMaxMercenary     = pMapType->wMaxMercenary;
    //
    m_wDescribe         = pMapType->wDescribe;
  }
  // Load Map Data
  m_iSizeX = m_pBase->GetSizeX();   // Width
  m_iSizeY = m_pBase->GetSizeY();   // Heigh
  
  // Init The Map Tile Of Player Postion
  m_pMapTilePlayer = new LifeTileList[m_iSizeX*m_iSizeY];

  for( i = 0; i < DWORD(m_iSizeX*m_iSizeY); i++ )
  {
    m_pMapTilePlayer[i].clear();
  }
  // Init Ground Item Tile Info
	m_pTileGroundItem = new GroundItemTileLsit[m_iSizeX*m_iSizeY];

  if (m_pTileGroundItem==NULL)
  {
	  MessageBox(NULL,"分配内存错误，已经超出内存限制","Error",MB_OK) ;
	  return false ;
  }
  for( i = 0; i < DWORD(m_iSizeX*m_iSizeY); i++ )
  {
	  try
	  {
		  m_pTileGroundItem[i].clear();
	  }
		catch (...) 
		{
			int nret = GetLastError () ;
		}
  }
  
  int iArea = m_iSizeX * m_iSizeY;
  m_pMapOrg = new DWORD[iArea];
  
  for( i = 0; i < (int)m_iSizeX; i++ )
  {
    for( j = 0; j < (int)m_iSizeY; j++ )
    {
      m_pMapOrg[m_iSizeX*j+i] = ((DWORD)(m_pBase->GetOneCell(i, j))) << 24;
      if( m_pMapOrg[m_iSizeX*j+i] & 0x01000000 )	iFlagCount++;
      m_pMapOrg[m_iSizeX*j+i] &= 0xC0000000;
      if( i == 0 || j == 0 )
      {
        m_pMapOrg[m_iSizeX*j+i] |= 0x80000000;
      }
    }
  }
  if( iFlagCount )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Map[Id=%d] Have %d Errors ! *****", iTheMapId, iFlagCount );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szMapLog );
#endif
  }
  // Init Adjust Point
  InitAdjustPoint();
  //==========================================================================
#ifdef _NEW_CITY_WAR_2005_
  //如果有CityMasterGiftItem.txt则先读取系统存储的CityMasterGift%d.txt
  //对server的存档，不传输用，所以没用数据库存储
  FILE *pSave;
  DWORD dwCount, dwSave1, dwSave2;
  char szFileName[256];
  _snprintf( szFileName, 256-1, "%s/CityMasterGift%d.txt", g_pBase->GetMapFilePath(), m_pBase->m_dwMapId );
  szFileName[256-1] = '\0';
  if( ( pSave = fopen( szFileName, "r" ) ) != NULL )
  {
    fscanf( pSave, "%d", &dwCount );
    for( int ii = 0; ii < dwCount; ii++ )
    {
      if( 2 == fscanf( pSave, "%d %d", &dwSave1, &dwSave2 ) )//dwSave1: itemId   dwSave2:high:addcount low:
      {
        InitMasterGift( dwSave1, dwSave2 );
      }
      else
      {
        fclose( pSave );
        return false;
      }
    }
  }
  if(g_pGuildMng->IsMasterGiftMap( m_pBase->m_dwMapId ))
  {
    g_pGuildMng->InitMasterGift( this );
  }
#endif
  return true;
}
//=====================================================================================
//
//
bool CGameMap::ResetMonsterInfo()
{
#ifdef _DEBUG
  int aaaa = 1;
  if(this->GetMapId() == 4001)
  {
    aaaa = 2;
  }
#endif
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CGameMap::ResetMonsterInfo");
  map<DWORD,CMonster*>::iterator	mIte;
  char														szFileName[256];
  int															dwRandX = 0, dwRandY = 0, iMonsterTotal = 0, iColNum = 0, iCount = 0, iMap = 0, iScan = 0, i, j;
  DWORD														dwX_Y = 0, dwX = 0, dwY = 0, dwMonsterId = 0, dwMonsterCode = 0, dwTeamId = 0, dwCount = 0;
  CSrvBaseMonster									*pBaseMonster;
  CMonsterTeamers									*m_pMonsterTeamList;// About Monster Team Info
  WORD                            wNowTeamId = 0;

  _snprintf( szFileName, 256-1, "%s//Monster//MonsterList",g_pBase->GetObjectFilePath() );
  szFileName[256-1] = '\0';
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "_%d.txt", (int)m_pBase->GetMapId() ); 
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  strcat( szFileName, szMapLog );
  CInStream                       MonsterInfo( szFileName );   
  if ( MonsterInfo.fail() || MonsterInfo.GetFileSize() == 0 ) 
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Map(%d) Have No Monsters ! *****", GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg(szMapLog);
    return true;
  }
  MonsterInfo >> iColNum;
  if ( iColNum < 1 ) 
  {
    return true;
  }
  MonsterInfo >> iCount;
  if ( iCount < 1 ) 
  {
    return true;
  }
  MonsterInfo >> m_wMaxTeamId;
  if( m_wMaxTeamId > 0 )
  {
    m_pMonsterTeamList = new CMonsterTeamers[m_wMaxTeamId];
  }
  for( int h = 0; h < m_wMaxTeamId; h++ )
  {
    m_pMonsterTeamList[h].SetMap( this );
    m_pMonsterTeamList[h].SetTeamId( h + 1 );
  }
  m_dwMonsterCount  = iCount;
  m_MonsterCodeList = new LPCMonster[iCount];
  for( i = 0; i < iCount; i++ )
  {
    m_MonsterCodeList[i] = NULL;
  }
  m_mapCodeMonster.clear();
  m_mapDefenceMonster.clear();
  //
  for( i = 0; i < iColNum; i++ )
  {
    MonsterInfo >> dwTeamId 
                >> dwMonsterId 
                >> dwX 
                >> dwY 
                >> dwCount;
    if( wNowTeamId < dwTeamId )
    {
      wNowTeamId = dwTeamId;
    }
    {
      // Find The Base Monster
      if( NULL == ( pBaseMonster = g_pBase->GetBaseMonster( dwMonsterId ) ) )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Found The Base Monster(%d) When Reset Monster In Map(%d) ! *****", dwMonsterId, GetMapId() );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        return false;
      }
      // Set Random X, Y
      for( j = 0; j < dwCount; j++ )
      {
        if( pBaseMonster->GetSpeed() )
        {
          for( int z = 0; z < 10; z++ )
          {
            dwRandX = gf_GetRandom( 14 ) - gf_GetRandom( 14 );
            dwRandY = gf_GetRandom( 14 ) - gf_GetRandom( 14 );
            if( !( GetTileFlag( dwX + dwRandX, dwY + dwRandY ) & TILE_ALLOCCULDE ) )	break;
            else if( z == 9 )
            {
              dwRandX = -1 + gf_GetRandom( 2 );
              dwRandY = -1 + gf_GetRandom( 2 );
            }
          }
        }
        else
        {
          dwRandX = dwRandY = 0;
        }
        if( ( dwMonsterCode = GetNewMonsterCode() ) > CODE_MAX_MONSTER || iMonsterTotal >= iCount )
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Total Is Error 1, Real=%d, KeyIn=%d In Map(%d), Code(%d)",
            iMonsterTotal, iCount, m_pBase->GetMapId(), dwMonsterCode );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg(szMapLog);
#endif
          return false;
        }
        CMonster * pNewMonster = new CMonster( pBaseMonster, dwMonsterCode );
        
        if( pNewMonster )
        {
          g_dwMonsterTotal++;
          if( !pNewMonster->InitSkill() )
          {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
            _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Monster(%d) Skill Is Error In Map(%d) ! *****", dwMonsterId, m_pBase->GetMapId() );
            szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
            AddMemoErrMsg(szMapLog);
#endif
            SAFE_DELETE( pNewMonster );
            return false;
          }
          // 初始化城门怪物的数据
          if( pNewMonster->IsCityGate() )
          {
            if( !AddCityGate( pNewMonster ) )
            {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
              _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The City Gate Monster(%d) Id Must Be Unique In Map(%d)*****", dwMonsterId, m_pBase->GetMapId() );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg( szMapLog );
#endif
              return false;
            }
          }
          // 初始化据点标志物怪物的数据
          pNewMonster->InitDeadEvent();
          // 初始化城门
          if( pNewMonster->IsCityGate() )    pNewMonster->SetState( STATUS_MONSTER_DISABLE );
          //
#ifdef _NEW_CITY_WAR_2005_
          if( pNewMonster->IsCityWarMon() )
          {
            AddCityWarMon( pNewMonster, dwX + dwRandX, dwY + dwRandY );//攻城战怪物在攻城战中才显示
          }
          else
          {
            AddMonster( pNewMonster, dwX + dwRandX, dwY + dwRandY );
          }
#else
          AddMonster( pNewMonster, dwX + dwRandX, dwY + dwRandY );
#endif
          pNewMonster->SetGameMap( this );
          pNewMonster->InitPos( dwX, dwY, dwX + dwRandX, dwY + dwRandY );
          pNewMonster->SetWanderRange();
          m_MonsterCodeList[iMonsterTotal] = pNewMonster;
          iMonsterTotal++;
          // Set Monster Team
          if( dwTeamId && dwTeamId <= m_wMaxTeamId )
          {
            m_pMonsterTeamList[dwTeamId-1].AddMonster( pNewMonster );
            pNewMonster->SetTeam( &( m_pMonsterTeamList[dwTeamId-1] ) );
            pNewMonster->SetTeamId( dwTeamId );
            m_pMonsterTeamList[dwTeamId-1].InitBaseBirthplace( dwX, dwY );
          }
          else
          {
            if( dwTeamId )
            {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
              _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Monster(%d) Team Id(%d) > Max Team Id(%d) In Map(%d) ! *****", dwMonsterId, dwTeamId, m_wMaxTeamId, m_pBase->GetMapId() );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg(szMapLog);
#endif
              return false;
            }
          }
        }
        else
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Fail new the monster(%d) when Reset map(%d) monster #", dwMonsterId, m_pBase->GetMapId() );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg(szMapLog);
#endif
          return false;
        }
      }
    }
  }
  if( iMonsterTotal != iCount )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Total Is Error 2, Real=%d, KeyIn=%d In Map(%d)",
      iMonsterTotal, iCount, m_pBase->GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szMapLog );
#endif
    return false;
  }
  //
  if( wNowTeamId != m_wMaxTeamId )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Team Id Is Error, Real=%d, KeyIn=%d In Map(%d)",
      wNowTeamId, m_wMaxTeamId, m_pBase->GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    return false;
  }
  // Init All Monster's Team Birthplace
  for( int g = 0; g < m_wMaxTeamId; g++ )
  {
    m_pMonsterTeamList[g].InitTeamBirthPlace();
  }
  return true;
#else
//
  FuncName("CGameMap::ResetMonsterInfo");
  FILE														*File;
  map<DWORD,CMonster*>::iterator	mIte;
  char														szFileName[256];
  int															dwRandX = 0, dwRandY = 0, iMonsterTotal = 0, iColNum = 0, iCount = 0, iMap = 0, iScan = 0, i, j;
  DWORD														dwX_Y = 0, dwX = 0, dwY = 0, dwMonsterId = 0, dwMonsterCode = 0, dwTeamId = 0, dwCount = 0;
  CSrvBaseMonster									*pBaseMonster;
  CMonsterTeamers									*m_pMonsterTeamList;// About Monster Team Info
  WORD                            wNowTeamId = 0;
  
  //#ifdef _DEBUG
  //  return true;
  //#endif
  //
  _snprintf( szFileName, 256-1, "%s//Monster//MonsterList",g_pBase->GetObjectFilePath() );
  szFileName[256-1] = '\0';
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "_%d.txt", (int)m_pBase->GetMapId() ); 
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
#ifdef REPLACE_STRCAT
  SafeStrcat( szFileName, szMapLog, 256);
#else
  strcat( szFileName, szMapLog );
#endif
  if( ( File = fopen( szFileName, "r" ) ) == NULL )
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Map(%d) Have No Monsters ! *****", GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg(szMapLog);
    return true;
  }
  // Get File Head
  // ===== This Txt File's Column Count
  fscanf( File, "%d ", &iColNum );
  if( iColNum < 1 )
  {
    return true;
  }
  // ===== This Map Monster's Total
  fscanf( File, "%d ", &iCount );
  if( iCount < 1 )
  {
    fclose( File );
    return true;
  }
  // ===== The Monster's Team Count Of This Map
  fscanf( File, "%hd ", &m_wMaxTeamId );
  if( m_wMaxTeamId > 0 )
  {
    m_pMonsterTeamList = new CMonsterTeamers[m_wMaxTeamId];
  }
  for( int h = 0; h < m_wMaxTeamId; h++ )
  {
    m_pMonsterTeamList[h].SetMap( this );
    m_pMonsterTeamList[h].SetTeamId( h + 1 );
  }
  // Init Max Monster Code List
  m_dwMonsterCount  = iCount;
  m_MonsterCodeList = new LPCMonster[iCount];
  for( i = 0; i < iCount; i++ )
  {
    m_MonsterCodeList[i] = NULL;
  }
  m_mapCodeMonster.clear();
  m_mapDefenceMonster.clear();
  //
  for( i = 0; i < iColNum; i++ )
  {
    iScan = fscanf( File, "%d %d %d %d %d ", &dwTeamId, &dwMonsterId, &dwX, &dwY, &dwCount );
    // Convert the client coordinate to server coordinate
    //ConvertCli2Srv( (int*)(&dwX), (int*)(&dwY) );
    if( wNowTeamId < dwTeamId )     wNowTeamId = dwTeamId;
    if( 5 == iScan )
    {
      // Find The Base Monster
      if( NULL == ( pBaseMonster = g_pBase->GetBaseMonster( dwMonsterId ) ) )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Cannot Found The Base Monster(%d) When Reset Monster In Map(%d) ! *****", dwMonsterId, GetMapId() );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        fclose( File );
        return false;
      }
      // Set Random X, Y
      for( j = 0; j < dwCount; j++ )
      {
        if( pBaseMonster->GetSpeed() )
        {
          for( int z = 0; z < 10; z++ )
          {
            dwRandX = gf_GetRandom( 14 ) - gf_GetRandom( 14 );
            dwRandY = gf_GetRandom( 14 ) - gf_GetRandom( 14 );
            if( !( GetTileFlag( dwX + dwRandX, dwY + dwRandY ) & TILE_ALLOCCULDE ) )	break;
            else if( z == 9 )
            {
              dwRandX = -1 + gf_GetRandom( 2 );
              dwRandY = -1 + gf_GetRandom( 2 );
            }
          }
        }
        else
        {
          dwRandX = dwRandY = 0;
        }
        if( ( dwMonsterCode = GetNewMonsterCode() ) > CODE_MAX_MONSTER || iMonsterTotal >= iCount )
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Total Is Error 1, Real=%d, KeyIn=%d In Map(%d), Code(%d)",
            iMonsterTotal, iCount, m_pBase->GetMapId(), dwMonsterCode );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg(szMapLog);
#endif
          fclose( File );
          return false;
        }
        CMonster * pNewMonster = new CMonster( pBaseMonster, dwMonsterCode );
        
        if( pNewMonster )
        {
          g_dwMonsterTotal++;
          if( !pNewMonster->InitSkill() )
          {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
            _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Monster(%d) Skill Is Error In Map(%d) ! *****", dwMonsterId, m_pBase->GetMapId() );
            szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
            AddMemoErrMsg(szMapLog);
#endif
            SAFE_DELETE( pNewMonster );
            fclose( File );
            return false;
          }
          // 初始化城门怪物的数据
          if( pNewMonster->IsCityGate() )
          {
            if( !AddCityGate( pNewMonster ) )
            {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
              _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The City Gate Monster(%d) Id Must Be Unique In Map(%d)*****", dwMonsterId, m_pBase->GetMapId() );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg( szMapLog );
#endif
              fclose( File );
              return false;
            }
          }
          // 初始化据点标志物怪物的数据
          pNewMonster->InitDeadEvent();
          // 初始化城门
          if( pNewMonster->IsCityGate() )    pNewMonster->SetState( STATUS_MONSTER_DISABLE );
          //
#ifdef _NEW_CITY_WAR_2005_
          if( pNewMonster->IsCityWarMon() )
          {
            AddCityWarMon( pNewMonster, dwX + dwRandX, dwY + dwRandY );//攻城战怪物在攻城战中才显示
          }
          else
          {
            AddMonster( pNewMonster, dwX + dwRandX, dwY + dwRandY );
          }
#else
          AddMonster( pNewMonster, dwX + dwRandX, dwY + dwRandY );
#endif
          pNewMonster->SetGameMap( this );
          pNewMonster->InitPos( dwX, dwY, dwX + dwRandX, dwY + dwRandY );
          pNewMonster->SetWanderRange();
          m_MonsterCodeList[iMonsterTotal] = pNewMonster;
          iMonsterTotal++;
          // Set Monster Team
          if( dwTeamId && dwTeamId <= m_wMaxTeamId )
          {
            m_pMonsterTeamList[dwTeamId-1].AddMonster( pNewMonster );
            pNewMonster->SetTeam( &( m_pMonsterTeamList[dwTeamId-1] ) );
            pNewMonster->SetTeamId( dwTeamId );
            m_pMonsterTeamList[dwTeamId-1].InitBaseBirthplace( dwX, dwY );
          }
          else
          {
            if( dwTeamId )
            {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
              _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Monster(%d) Team Id(%d) > Max Team Id(%d) In Map(%d) ! *****", dwMonsterId, dwTeamId, m_wMaxTeamId, m_pBase->GetMapId() );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg(szMapLog);
#endif
              fclose( File );
              return false;
            }
          }
        }
        else
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Fail new the monster(%d) when Reset map(%d) monster #", dwMonsterId, m_pBase->GetMapId() );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg(szMapLog);
#endif
          fclose( File );
          return false;
        }
      }
    }
    else
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Scan Monster Error In Map(%d)", m_pBase->GetMapId() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szMapLog);
#endif
      fclose( File );
      return false;
    }
  }
  //
  if( iMonsterTotal != iCount )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Total Is Error 2, Real=%d, KeyIn=%d In Map(%d)",
      iMonsterTotal, iCount, m_pBase->GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg( szMapLog );
#endif
    fclose( File );
    return false;
  }
  //
  if( wNowTeamId != m_wMaxTeamId )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Team Id Is Error, Real=%d, KeyIn=%d In Map(%d)",
      wNowTeamId, m_wMaxTeamId, m_pBase->GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    fclose( File );
    return false;
  }
  // Init All Monster's Team Birthplace
  for( int g = 0; g < m_wMaxTeamId; g++ )
  {
    m_pMonsterTeamList[g].InitTeamBirthPlace();
  }
  fclose( File );
  return true;
#endif//_DEBUG_JAPAN_DECRYPT_
}


//=====================================================================================
//
//
CMonster * CGameMap::CreateMonster(int iId, int iX, int iY, BOOL bDisrevive)
{
  CSrvBaseMonster									*pBaseMonster;
  DWORD														dwX_Y = 0, dwMonsterCode = 0;
  CMonster												*pMonster;
  
  if( NULL != ( pBaseMonster = g_pBase->GetBaseMonster( iId ) ) )
  {
    if( ( dwMonsterCode = GetNewMonsterCode() ) <= CODE_MAX_MONSTER )
    {
      pMonster = new CMonster( pBaseMonster, dwMonsterCode );
      if( pMonster )
      {
        if( !pMonster->InitSkill() )
        {
          SAFE_DELETE( pMonster );
          return NULL;
        }
        if( bDisrevive )      pMonster->SetReviveType( REVIVE_TYPE_DELETE );
        AddMonster( pMonster, iX, iY );
        pMonster->SetGameMap(this);
        pMonster->InitPos( iX, iY, iX + gf_GetRandom( 8 ), iY + gf_GetRandom( 8 ) );
        pMonster->SetWanderRange();
        g_dwMonsterTotal++;
        return pMonster;
      }
    }
  }
  return NULL;
}
//=====================================================================================
//
//
bool CGameMap::RebindNpcAndMonster()
{
  //#ifdef _DEBUG
  //  return true;
  //#endif
  
  if( m_mapCodeNpc.empty() )      return true;
  //
  map<DWORD, CNpc*>::iterator     Iter_Npc;
  CMonster                        *pTheMonster = NULL;
  CNpc                            *pTheNpc = NULL;
  CEventPoint                     *pTheEvent = NULL;
  int                             iMonsterDead = 0, iCount = 0, iLoop = 0;
  CNpc                            *pDisappearNpc[10];
  //
  for( Iter_Npc = m_mapCodeNpc.begin(); Iter_Npc != m_mapCodeNpc.end(); Iter_Npc++ )
  {
    pTheNpc = Iter_Npc->second;
    if( pTheEvent = pTheNpc->GetEventPoint() )
    {
      switch( pTheEvent->GetPointType() )
      {
      case EVENT_POINTTYPE_MONSTERDIE:
        iMonsterDead++;
        pDisappearNpc[iCount++] = pTheNpc;
        if( m_pOccupyMonster )
        {
          m_pOccupyMonster->SetEventNpc( pTheNpc );
        }
        else      return false;
        break;
      default:
        break;
      }
    }
  }
  //
  for( iLoop = 0; iLoop < iCount; iLoop++ )
  {
    if( pDisappearNpc[iLoop] )    NpcDisappear( pDisappearNpc[iLoop]->GetCode() );
  }
  //
  if( iMonsterDead > 1 )    return false;
  return true;
}
//=====================================================================================
//
//
bool CGameMap::ResetNpcInfo()
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  FuncName("CGameMap::ResetNpcInfo");
  map<DWORD,CMonster*>::iterator	mIte;
  char													szFileName[256];
  int															iCount = 0, iMap = 0, iScan = 0, iX = 0, iY = 0, i, j = 0;
  DWORD														dwX_Y = 0, dwEventId = 0, dwNpcCode = CODE_MIN_NPC-1;
  CEventPoint											*pEvent = NULL;
  SGameTime												*pGameTime;
  _snprintf( szFileName, 256-1, "%s//Event//PointList",g_pBase->GetObjectFilePath());
  szFileName[256-1] = '\0';
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "_%d.txt", (int)m_pBase->GetMapId() ); 
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  strcat( szFileName, szMapLog );
  CInStream            ResetNpc( szFileName );
  if ( !ResetNpc.fail() && ResetNpc.GetFileSize() != 0 ) 
  { 
    ResetNpc >> iMap;
    if ( iMap != (int)m_pBase->GetMapId() ) 
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Map ID is error, when load file '%s' #", szFileName );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg(szMapLog);
#endif
      return false;
    }

    ResetNpc >> iCount;
    m_dwNpcCount  = iCount;
    m_NpcCodeList = new LPCNpc[iCount];
    for( i = 0; i < iCount; i++ )
    {
      m_NpcCodeList[i] = NULL;
    }
    m_mapCodeNpc.clear();
    for( i = 0, j = 0; i < iCount; i++ )
    {
      if ( !( ResetNpc >> dwEventId >> dwX_Y ) ) 
      {
#ifdef _DEBUG
        MessageBox( GetActiveWindow(), "Event dwEventId & dwX_Y Error", "Warning...", MB_OK );
#endif
        return false;
      }
      // Convert the client coordinate to server coordinate
      iX = HIWORD( dwX_Y );
      iY = LOWORD( dwX_Y );
      ConvertCli2Srv( &iX, &iY );
      dwX_Y = MAKELONG( iY, iX );
      char szTemp[10];
      pEvent = g_pBase->GetEventPoint( dwEventId );
      if( pEvent == NULL )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The EventPoint(%d,%d) is not existing in map(%d) #", HIWORD(dwEventId), LOWORD(dwEventId), iMap );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        MessageBox(NULL,itoa(iMap,szTemp,10),"Erro",MB_OK);
        MessageBox(NULL,itoa(dwEventId,szTemp,10),"Err",MB_OK);
        continue;
      }
      if( m_mapIDEventPoint.find( dwEventId ) != m_mapIDEventPoint.end() )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The EventPoint(%d,%d) Is Already Existing In map(%d) #", HIWORD(dwEventId), LOWORD(dwEventId), iMap );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        continue;
      }
      if( TheTileHaveNpc( HIWORD(dwX_Y), LOWORD(dwX_Y) ) )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The Npc Position(%d,%d) Have Other In map(%d) #", HIWORD(dwEventId), LOWORD(dwEventId), iMap );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        continue;
      }
      if( dwNpcCode++ > CODE_MAX_NPC )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        AddMemoErrMsg("Npc Code Overload #");
#endif
        return false;
      }
      pEvent->GetPicId();
      
      CNpc * pNewNpc = new CNpc( dwEventId, dwNpcCode );
      if( pNewNpc )
      {
        pNewNpc->SetEventPoint( pEvent );
        pNewNpc->SetGameMap(this);
#ifdef _NPC_TRANSLUCENCY_
        pNewNpc->SetNpcTranslucency(g_pBase->CheckNpcTranslucency(dwEventId));
#endif
        AddNpc( pNewNpc, (int)HIWORD(dwX_Y), (int)LOWORD(dwX_Y) );
#ifdef VERSION_40_HOUSE_FUNCTION
        if(pNewNpc->GetNpcType()==EVENT_POINTTYPE_HOUSE)
        {
#ifdef _DEBUG
          int i1 = pNewNpc->GetNpcFaceId();
          int i2 = GetMapId();
#endif
          DWORD dwTemp = MAKELONG(pNewNpc->GetNpcFaceId(),GetMapId());
          CHouse *pHouse = g_pHouseMgr->GetHouseById(dwTemp);
          if(pHouse != NULL)
          {
            pHouse->SetHouseNpcCode(pNewNpc->GetSelfCode());
          }
        }
#endif
        pNewNpc->SetWanderRange( 14, 14 );
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
        if( !(pNewNpc->SetFerryData()) )
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "FerryData Error >>> npc(%d) when Reset map(%d) npc #", dwEventId, m_pBase->GetMapId() );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg(szMapLog);
#endif
          return false;
        }
#endif
        m_NpcCodeList[j++] = pNewNpc;
        m_mapIDEventPoint.insert( map<DWORD, CEventPoint*>::value_type( dwEventId, pEvent ) );
      }
      else
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Fail new the npc(%d) when Reset map(%d) npc #", dwEventId, m_pBase->GetMapId());
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
      }		
    }      
    if( NULL != ( pGameTime = g_pGs->GetGameTime() ) )         // Set Disppear Npc
    {   
      if( pGameTime->wHour >= 6 && pGameTime->wHour <= 18 )    // Day
      {
        NpcAppearWhenDay();
        NpcDisappearWhenDay();
      }   
      else                                                     // Night
      {
        NpcAppearWhenNight();
        NpcDisappearWhenNight();
      }
    }
  }
  else
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Map(%d) Have No Npc ! *****", GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szMapLog );
    return TRUE;
  }
  return TRUE;
#else
//
  FuncName("CGameMap::ResetNpcInfo");
  FILE														*File;
  map<DWORD,CMonster*>::iterator	mIte;
  char														szFileName[256];
  int															iCount = 0, iMap = 0, iScan = 0, iX = 0, iY = 0, i, j = 0;
  DWORD														dwX_Y = 0, dwEventId = 0, dwNpcCode = CODE_MIN_NPC-1;
  CEventPoint											*pEvent = NULL;
  SGameTime												*pGameTime;
  // Init listNpc
  // ...
  _snprintf( szFileName, 256-1, "%s//Event//PointList",g_pBase->GetObjectFilePath());
  szFileName[256-1] = '\0';
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "_%d.txt", (int)m_pBase->GetMapId() ); 
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
#ifdef REPLACE_STRCAT 
  SafeStrcat( szFileName, szMapLog, 256 );
#else
  strcat( szFileName, szMapLog );
#endif
  if( ( File = fopen( szFileName, "r" ) ) == NULL )
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** The Map(%d) Have No Npc ! *****", GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szMapLog );
    return TRUE;
  }
  //  Get file data
  fscanf( File, "%d ", &iMap);
  if( iMap != (int)m_pBase->GetMapId() )
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Map ID is error, when load file '%s' #", szFileName );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsg(szMapLog);
#endif
    fclose( File );
    return false;
  }
  fscanf( File, "%d ", &iCount);
  m_dwNpcCount  = iCount;
  m_NpcCodeList = new LPCNpc[iCount];
  for( i = 0; i < iCount; i++ )
  {
    m_NpcCodeList[i] = NULL;
  }
  m_mapCodeNpc.clear();
  for( i = 0, j = 0; i < iCount; i++ )
  {
    iScan = fscanf( File, "%d %d ", &dwEventId, &dwX_Y );
    // Convert the client coordinate to server coordinate
    iX = HIWORD( dwX_Y );
    iY = LOWORD( dwX_Y );
    ConvertCli2Srv( &iX, &iY );
    dwX_Y = MAKELONG( iY, iX );
    if( 2 == iScan )
    {
      char szTemp[10];
      pEvent = g_pBase->GetEventPoint( dwEventId );
      if( pEvent == NULL )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The EventPoint(%d,%d) is not existing in map(%d) #", HIWORD(dwEventId), LOWORD(dwEventId), iMap );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        MessageBox(NULL,itoa(iMap,szTemp,10),"Erro",MB_OK);
        MessageBox(NULL,itoa(dwEventId,szTemp,10),"Err",MB_OK);
        continue;
      }
      if( m_mapIDEventPoint.find( dwEventId ) != m_mapIDEventPoint.end() )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The EventPoint(%d,%d) Is Already Existing In map(%d) #", HIWORD(dwEventId), LOWORD(dwEventId), iMap );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        continue;
      }
      if( TheTileHaveNpc( HIWORD(dwX_Y), LOWORD(dwX_Y) ) )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The Npc Position(%d,%d) Have Other In map(%d) #", HIWORD(dwEventId), LOWORD(dwEventId), iMap );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
        continue;
      }
      if( dwNpcCode++ > CODE_MAX_NPC )
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        AddMemoErrMsg("Npc Code Overload #");
#endif
        fclose( File );
        return false;
      }
      pEvent->GetPicId();

      CNpc * pNewNpc = new CNpc( dwEventId, dwNpcCode );
      if( pNewNpc )
      {
        pNewNpc->SetEventPoint( pEvent );
        pNewNpc->SetGameMap(this);
#ifdef _NPC_TRANSLUCENCY_
        pNewNpc->SetNpcTranslucency(g_pBase->CheckNpcTranslucency(dwEventId));
#endif
        AddNpc( pNewNpc, (int)HIWORD(dwX_Y), (int)LOWORD(dwX_Y) );
#ifdef VERSION_40_HOUSE_FUNCTION
        if(pNewNpc->GetNpcType()==EVENT_POINTTYPE_HOUSE)
        {
#ifdef _DEBUG
          int i1 = pNewNpc->GetNpcFaceId();
          int i2 = GetMapId();
#endif
          DWORD dwTemp = MAKELONG(pNewNpc->GetNpcFaceId(),GetMapId());
          CHouse *pHouse = g_pHouseMgr->GetHouseById(dwTemp);
          if(pHouse != NULL)
          {
            pHouse->SetHouseNpcCode(pNewNpc->GetSelfCode());
          }
        }
#endif
        //
        pNewNpc->SetWanderRange( 14, 14 );
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
        if( !(pNewNpc->SetFerryData()) )
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "FerryData Error >>> npc(%d) when Reset map(%d) npc #", dwEventId, m_pBase->GetMapId() );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg(szMapLog);
#endif
          return false;
        }
#endif
        m_NpcCodeList[j++] = pNewNpc;
        m_mapIDEventPoint.insert( map<DWORD, CEventPoint*>::value_type( dwEventId, pEvent ) );
      }
      else
      {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Fail new the npc(%d) when Reset map(%d) npc #", dwEventId, m_pBase->GetMapId());
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoErrMsg(szMapLog);
#endif
      }		
    }
  }
  // Set Disppear Npc
  if( NULL != ( pGameTime = g_pGs->GetGameTime() ) )
  {
    // Day
    if( pGameTime->wHour >= 6 && pGameTime->wHour <= 18 )
    {
      NpcAppearWhenDay();
      NpcDisappearWhenDay();
    }
    // Night
    else
    {
      NpcAppearWhenNight();
      NpcDisappearWhenNight();
    }
  }
  fclose( File );
  return true;
#endif //_DEBUG_JAPAN_DECRYPT_
}

#ifdef _NEW_TRADITIONS_WEDDING_
extern CAwaneMapMgr    *g_AwaneMapMgr;
bool CGameMap::ResetAwaneInfo()
{
  if (g_AwaneMapMgr)
  {
    m_bAwaneMap = g_AwaneMapMgr->GetAwaneInfo(GetMapId(),m_dwAwaneInterval,m_fAwaneRadio);
  }
  return true;
}

bool CGameMap::ResetMarryFateInfo()
{
  //姻缘之路地图
  const int iMarryFateMapNum = 7;
  const int iMarryFataMap[iMarryFateMapNum] = { 4005, 4006, 4007, 4301, 4302, 4303, 4304 };

  for (int i=0; i< iMarryFateMapNum; i++)
  {
    if (GetMapId() == iMarryFataMap[i])
    {
      m_bMarryFateMap = TRUE;
      break;
    }
  }
  return true;
}

bool CGameMap::IsAwaneMap()
{
  return m_bAwaneMap;
}
bool CGameMap::IsMarryFateMap()
{
  return m_bMarryFateMap;
}
#endif

//=====================================================================================
//
//
CPlayer * CGameMap::GetPlayerByName( const char * szName )
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::GetPlayerByName()");
#endif
  
  static map<DWORD, CPlayer*>::iterator			iter_Name;
  static CPlayer														*pNamePlayer;
  
  for( iter_Name = m_mapCodePlayer.begin(); iter_Name != m_mapCodePlayer.end(); iter_Name++ )
  {
    if( NULL != ( pNamePlayer = iter_Name->second ) )
    {
      if( !strcmp( pNamePlayer->GetPlayerName(), szName ) )
      {
        return pNamePlayer;
      }
    }
  }
  return NULL;
}
//=====================================================================================
//
//
CPlayer* CGameMap::GetPlayer(const WORD &dwCode)
{
  static map<DWORD,CPlayer*>::iterator      Iter_Pl;
  
  Iter_Pl = m_mapCodePlayer.find( dwCode );
  if( Iter_Pl != m_mapCodePlayer.end() )
  {
    if( (Iter_Pl->second)->IsInGame() )     return (CPlayer*)(Iter_Pl->second);
  }
  return NULL;
}
//=====================================================================================
//
//
CMonster * CGameMap::GetCityGate(const WORD & wId )
{
  list<CMonster*>::iterator     Iter_Ms;
  //
  for( Iter_Ms = m_listCityGate.begin(); Iter_Ms != m_listCityGate.end(); Iter_Ms++ )
  {
    if( (*Iter_Ms)->GetBaseId() == wId )      return (*Iter_Ms);
  }
  return NULL;
}
//=====================================================================================
//
//
CMonster* CGameMap::GetMonster(const WORD &dwCode)
{
#ifdef _WILDCAT_ARRAY_MONSTER_NPC_LIST_
  if( dwCode >= CODE_MIN_MONSTER && dwCode < CODE_MIN_MONSTER + m_dwMonsterCount )
  {
    return m_MonsterCodeList[dwCode-CODE_MIN_MONSTER];
  }
  else
  {
    map<DWORD,CMonster*>::iterator		mIte;
    
    mIte = m_mapCodeMonster.find( dwCode );
    if( mIte != m_mapCodeMonster.end() )
    {
      return (CMonster*)(mIte->second);
    }
    return NULL;
  }
#else
  map<DWORD,CMonster*>::iterator		mIte;
  
  mIte = m_mapCodeMonster.find( dwCode );
  if( mIte != m_mapCodeMonster.end() )
  {
    return (CMonster*)(mIte->second);
  }
  return NULL;
#endif
}
//=====================================================================================
//
//
CNpc* CGameMap::GetNpc(const WORD &dwCode)
{
#ifdef _WILDCAT_ARRAY_MONSTER_NPC_LIST_
  if( dwCode >= CODE_MIN_NPC && dwCode < CODE_MIN_NPC + m_dwNpcCount )
  {
    return m_NpcCodeList[dwCode-CODE_MIN_NPC];
  }
  return NULL;
#else
  map<DWORD,CNpc*>::iterator iter;
  
  iter = m_mapCodeNpc.find(dwCode);
  if( iter != m_mapCodeNpc.end() )
  {
    return (CNpc*)(iter->second);
  }
  return NULL;
#endif
}
//=====================================================================================
//
//
BOOL CGameMap::GetAllNearNpc(int xPos,int yPos,void* pNpcData,int& iCount)
{
  SNMNpcInfo *pTheNpcInfo = (SNMNpcInfo*)pNpcData;
  iCount = 0;

  map<DWORD,CNpc*>::iterator iter = m_mapCodeNpc.begin() ;
  while ( iter != m_mapCodeNpc.end() )
  {
    CNpc* pTheLife = (CNpc*)(iter->second);
 		if( pTheLife )
		{
			if( pTheLife->GetDistance(xPos, yPos) <= MAX_GET_INFO_DISTANCE )
			{
				( (CNpc*)pTheLife )->Get_SNMNpcInfo( ( pTheNpcInfo + iCount ) );
				iCount++;
        if( (char*)(pTheNpcInfo + iCount) > (char*)((char*)pNpcData + 4088) )
          return FALSE;
			}
		}
    //
    iter++;
  }
  return (iCount>0);
}
//=====================================================================================
//
//
CNpc * CGameMap::GetNpcByName(char * szName)
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::GetNpcByName()");
#endif
  
  CNpc		*pNpc;
  
  for( map<DWORD,CNpc*>::iterator iter = m_mapCodeNpc.begin(); iter != m_mapCodeNpc.end(); iter++ )
  {
    if( NULL != ( pNpc = ( iter->second ) ) )
    {
      if( !strcmp( szName, pNpc->GetNpcName() ) )
      {
        return pNpc;
      }
    }
  }
  return NULL;
}
//=====================================================================================
//
//
CGroundItem* CGameMap::GetGroundItem(const WORD & dwCode)
{
  static map<DWORD,CGroundItem*>::iterator       Iter_GI;
  
  Iter_GI = m_mapCodeGroundItem.find(dwCode);
  if( Iter_GI != m_mapCodeGroundItem.end() )
  {
    return (CGroundItem*)(Iter_GI->second);
  }
  return NULL;
}
//=====================================================================================
//
//
CMagic * CGameMap::GetMagic(const WORD & dwCode)
{
  static map<DWORD, CMagic*>::iterator    Iter_Mg;
  
  Iter_Mg = m_mapCodeMagic.find(dwCode);
  if( Iter_Mg != m_mapCodeMagic.end() )
  {
    return (CMagic*)(Iter_Mg->second);
  }
  return NULL;
}
//=====================================================================================
//
//Function:
CLife * CGameMap::GetFirstLifeInTile( const WORD & dwX, const WORD & dwY )
{
  CLife     *pLife = NULL;
  
  if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 )
  {
    if( m_pMapTilePlayer[dwY*m_iSizeX+dwX].empty() )	return NULL;
    pLife = (*m_pMapTilePlayer[dwY*m_iSizeX+dwX].begin());
    if( pLife->GetInMap() != this )
    {
      m_pMapTilePlayer[dwY*m_iSizeX+dwX].pop_front();
      return NULL;
    }
    return pLife;
  }
  return NULL;
}
//=====================================================================================
//
//Function:	----------------------------------------- Add by WildCat 2002-4-2 begin
//					Get the player list of current coordinate
LPLifeTileList CGameMap::GetTileLifeList( const WORD & dwX, const WORD & dwY )
{	
 	if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 )
    return &(m_pMapTilePlayer[dwY*m_iSizeX+dwX]);
  else
    return NULL;
}
//=====================================================================================
//
//Function:
//
BOOL CGameMap::HaveLifeInTile( const WORD & dwX, const WORD & dwY )
{
  if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 )
    return ( !m_pMapTilePlayer[dwY*m_iSizeX+dwX].empty() );
  else
    return FALSE;
}
//=====================================================================================
//
//Function:
//					Insert the player into the current coordinate when player position changed
BOOL CGameMap::InsertLifeToTile(const WORD & dwX, const WORD & dwY, CLife * pTheLife)
{
  //#ifdef _DEBUG_WILDCAT_
  //	FuncName("CGameMap::InsertLifeToTile");
  //#endif
  
  if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 && pTheLife )
  {
    if( pTheLife->IsPlayer() )
    {
      m_pMapTilePlayer[dwY*m_iSizeX+dwX].push_back( pTheLife );
    }
    else// if( pTheLife->IsPlayer() )
    {
      m_pMapTilePlayer[dwY*m_iSizeX+dwX].push_front( pTheLife );
    }
    return TRUE;
  }
#ifdef _DEBUG_WILDCAT_
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Can not insert life(%d) to tile(%d,%d) because slop over [MapID=%d] #",pTheLife->GetCode(),dwX,dwY,GetMapId() );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddErrLogOnly(szMapLog);
#endif
  return FALSE;
}
//=====================================================================================
//
//Function:
BOOL CGameMap::DelLifeFromTile(CLife * pTheLife)
{
  static DWORD										dwPostion;
  LifeTileList::iterator		Iter_Life;
  static WORD                     dwX, dwY;
  
  dwX = pTheLife->GetPosX();
  dwY = pTheLife->GetPosY();
  //
  if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 )
  {
    dwPostion = dwY*m_iSizeX+dwX;
    if( m_pMapTilePlayer[dwPostion].empty() ) 			  return FALSE;
    //
    for( Iter_Life = m_pMapTilePlayer[dwPostion].begin(); Iter_Life != m_pMapTilePlayer[dwPostion].end(); Iter_Life++ )
    {
      if( (*Iter_Life) == pTheLife )
      {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "CGameMap::DelLifeFromTile ===>>%d,%d,(%d,%d),(%d,%d)", pTheLife->GetCode(), GetMapId(), m_iSizeX, m_iSizeY, dwX, dwY );
        szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
        g_CrashLog.Write( szMapLog );
#endif
        m_pMapTilePlayer[dwPostion].erase( Iter_Life );
        return TRUE;
      }
    }
    return FALSE;
  }
  return FALSE;
}
//=====================================================================================
//
//
DWORD CGameMap::GetCodeFromXY( const WORD & dwX, const WORD & dwY )
{
  static DWORD dwMyPos;
  
  dwMyPos = dwY * m_iSizeX + dwX;
  if( dwMyPos < ( m_iSizeX * m_iSizeY ) )
  {
    if( !m_pMapTilePlayer[dwMyPos].empty() )
    {
      return (*m_pMapTilePlayer[dwMyPos].begin())->GetCode();
    }
  }
  return 0;
}
//=====================================================================================
//
//
LPGroundItemTileLsit CGameMap::GetTileItemList(const WORD & dwX, const WORD & dwY)
{
 	if( dwX < m_iSizeX && dwY < m_iSizeY )
    return &(m_pTileGroundItem[dwY*m_iSizeX+dwX]);
  else
    return NULL;
}
//=====================================================================================
//
//
CGroundItem * CGameMap::GetTileFirstItem(const WORD & dwX, const WORD & dwY)
{
 	if( dwX < m_iSizeX && dwY < m_iSizeY )
  {
    if( !(m_pTileGroundItem[dwY*m_iSizeX+dwX].empty()) )
    {
      return (*(m_pTileGroundItem[dwY*m_iSizeX+dwX].begin()));
    }
  }
  return NULL;
}
//=====================================================================================
//
//
BOOL CGameMap::InsertItemToTile(const WORD & dwX, const WORD & dwY, CGroundItem * pItem)
{
  
  if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 )
  {
    m_pTileGroundItem[dwY*m_iSizeX+dwX].push_front( pItem );
    return TRUE;
  }
#ifdef _DEBUG_WILDCAT_
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Can not insert item(%d) to tile(%d,%d) because slop over [MapID=%d] #",pItem->GetCode(),dwX,dwY,GetMapId());
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddErrLogOnly(szMapLog);
#endif
  return FALSE;
}
//=====================================================================================
//
//
BOOL CGameMap::DelItemFromTile(const WORD & dwX, const WORD & dwY, CGroundItem * pItem)
{
  static DWORD													dwPostion;
  static GroundItemTileLsit::iterator		Iter_GItem;
  
  if( dwX < m_iSizeX && dwY < m_iSizeY && dwX > 0 && dwY > 0 )
  {
    dwPostion = dwY*m_iSizeX+dwX;
    if( m_pTileGroundItem[dwPostion].empty() )
    {
#ifdef _DEBUG_WILDCAT
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "*** This Item(%d) Is Not In The Tile(%d,%d) Because Empty [MapID=%d] ! ***",pItem->GetCode(),dwX,dwY,GetMapId());
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddErrLogOnly(szMapLog);
#endif
      return FALSE;
    }
    for( Iter_GItem = m_pTileGroundItem[dwPostion].begin(); Iter_GItem != m_pTileGroundItem[dwPostion].end(); Iter_GItem++ )
    {
      if( (*Iter_GItem) == pItem )
      {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        g_CrashLog.Write( "CGameMap::DelItemFromTile ===>> 111" );
#endif
        m_pTileGroundItem[dwPostion].erase( Iter_GItem );
        return TRUE;
      }
    }
#ifdef _DEBUG_WILDCAT_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "*** This Item(%d) Is Not In The Tile(%d,%d) [MapID=%d] ! ***",pItem->GetCode(),dwX,dwY,GetMapId());
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddErrLogOnly(szMapLog);
#endif
    return FALSE;
  }
#ifdef _DEBUG_WILDCAT_
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Can Not Delete Item(%d) From Tile(%d,%d) Because Slop Over [MapID=%d] ! *****",pItem->GetCode(),dwX,dwY,GetMapId());
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddErrLogOnly(szMapLog);
#endif
  return FALSE;
}
//=====================================================================================
//
//
bool CGameMap::PlayerTryWalk(CPlayer *pThePlayer)  // return true when find the warp point
{
  static WORD     wTempX, wTempY, wMagicCode;
  static CMagic   *pTheMagic = NULL;
  wTempX  = (WORD)pThePlayer->m_PassTile[pThePlayer->m_iMoveStep].x;
  wTempY  = (WORD)pThePlayer->m_PassTile[pThePlayer->m_iMoveStep++].y;
  
  // If This Map Cannot PK, Then Cannot Pass B-Tile
  //if( m_dwMapType & ( MAP_TYPE_NO_PK | MAP_TYPE_NO_ATTACK ) )
  //{
  //	if( m_pMapOrg[dwTempY*m_iSizeX+dwTempX] & TILE_LIFE_FLAG )
  //	{
  //		// Stop Here
  //		pThePlayer->SetState( STATUS_PLAYER_STOP );
  //		pThePlayer->m_iUpdateTurn = 0;
  //		// Send Message To Client ??? -- Set Player Status To STATUS_PLAYER_STOP
  //		// And Update Status Is TRUE, The Next Status Is STATUS_PLAYER_STAND
  //	}
  //}
  // Else Can Pass B-Tile MAP_TYPE_NO_PK
  
  if( ( 0 < wTempX ) && ( wTempX < m_iSizeX ) && ( 0 < wTempY ) && ( wTempY < m_iSizeY ) )
  {
    //DWORD dwTheTile = m_pMapOrg[dwTempY*m_iSizeX + dwTempX];
    
    // Set Player Dir
    pThePlayer->SetMoveDir( wTempX, wTempY );
    // Check Magic On Map
    if( wMagicCode = GetMagicCode( wTempX, wTempY ) )
    {
      if( pTheMagic = GetMagic( wMagicCode ) )
      {
        pTheMagic->BeActivation( pThePlayer );
      }
    }
    // Clear Old Tile Code
    //m_pMapOrg[iTileY*m_iSizeX + iTileX] &= TILE_CLEARCODE;
    
    // Set The New Tile Code
    //m_pMapOrg[iTempY*m_iSizeX + iTempX] &= ( TILE_HAVECODE & pThePlayer->GetCode() );
    
    //  Delete The Player From This Tile
    //  Set The Player's New Position And Insert The Player Into This Tile
    pThePlayer->SetX_Y( wTempX, wTempY );
#ifndef _DEBUG_NO_ANY_LOG_
#ifdef _DEBUG
#ifdef _SHOW_PLAYER_MOVE_LOG_
    FuncName("CGameMap::PlayerTryWalk");
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "===>>> %s Walk To Tile(%d,%d) <<<===", pThePlayer->GetPlayerName(), wTempX, wTempY );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoMsg( szMapLog );
#endif
#endif
#endif
    return true;
  }
  return false;
}
//=====================================================================================
//
//
bool CGameMap::AddPlayer(CPlayer* pNewPlayer, const WORD & iTheX, const WORD & iTheY)
{
  if( m_mapCodePlayer.find( pNewPlayer->GetSelfCode() ) == m_mapCodePlayer.end() )
  {
    m_mapCodePlayer.insert( map<DWORD,CPlayer*>::value_type( pNewPlayer->GetSelfCode(), pNewPlayer ) );
    pNewPlayer->SetJoinMap( GetMapId() );
    pNewPlayer->SetMapInfo( this, iTheX, iTheY );
    pNewPlayer->CheckSavePoint();
    pNewPlayer->AddInMapTime();
    return true;
  }
  else
  {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** %s Is Already In The Map(%d) ! *****", pNewPlayer->GetPlayerAccount(),m_pBase->GetMapId());
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddMemoErrMsgOnly(szMapLog);
#endif
  }
  return false;
}
//==========================================================================================
//
//
bool CGameMap::DelPlayer(DWORD dwCode)
{
  static CPlayer													*pThePlayer = NULL;
  static map<DWORD,CPlayer*>::iterator		Iter_DP;
  
  Iter_DP = m_mapCodePlayer.find( dwCode );
  if( Iter_DP != m_mapCodePlayer.end() )
  {
    if( NULL != ( pThePlayer = ( CPlayer* )( Iter_DP->second ) ) )
    {
      ///////////////////////////////////////////////
      //Add by Cece 2004-07-18
#ifdef ELYSIUM_3_7_VERSION
      LPCFighter pFightInfo = pThePlayer->GetFightInfo();
      //如果从竞技场跳出的话收回FightInfo
      if( pFightInfo && pFightInfo->GetMapId() == GetMapId() )
      {
        pFightInfo->DelNum();
        if( pFightInfo->GetNum() == 0 ) pFightInfo->SetInMap( FALSE );
        pThePlayer->SetFightInfo( NULL );
        pThePlayer->ClearFightSwitch();
        pThePlayer->SetBeSent( FALSE );
      }
#endif
      ///////////////////////////////////////////////
      pThePlayer->ClearMyCodeInMap();
      // Delete This Player From The Map
      m_mapCodePlayer.erase( Iter_DP );
      //
      pThePlayer->SetJoinMap( 0 );
      pThePlayer->DelInMapTime();
      DelTileCode( pThePlayer->GetPosX(), pThePlayer->GetPosY() );
      if( pThePlayer->GetStatus() != STATUS_PLAYER_DEAD )
      {
        if( !DelLifeFromTile( pThePlayer ) )
        {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          FuncName("CGameMap::DelPlayer");
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Delete Life[%s](%d) From Map(%s)'s Tile When Delete Player From This Map %d #",
            pThePlayer->GetPlayerAccount(), pThePlayer->GetSelfCode(),
            m_pBase->GetMapName(), dwCode );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szMapLog );
#endif
          return false;
        }
      }
      return true;
    }
    else
    {
      m_mapCodePlayer.erase( Iter_DP );
      return false;
    }
  }
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
  FuncName("CGameMap::DelPlayer");
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Find Player Code=%d In GameMap %s(%d)",
    dwCode, m_pBase->GetMapName(), GetMapId() );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddMemoErrMsg( szMapLog );
#endif
  return false;
}
//==========================================================================================
//
//
void CGameMap::FixCityGate( const WORD & Id )
{
  CMonster      *pMonster = GetCityGate( Id );
  if( pMonster == NULL )    return;
  //
  if( pMonster->GetStatus() == STATUS_MONSTER_DEAD )
  {
    pMonster->Revive();
  }
  else
  {
    pMonster->SetHp( pMonster->GetMaxHp() );
    // Send The Msg To All Client
    MsgUpdateMstHp.Msgs[0].Size = sizeof( SNMUpdateMonsterHp );
    SNMUpdateMonsterHp    *pUpdate = (SNMUpdateMonsterHp*)(MsgUpdateMstHp.Msgs[0].Data);
    //
    pUpdate->wCode  = pMonster->GetCode();
    pUpdate->iMapHp = pMonster->GetMaxHp();
    pUpdate->iHp    = pMonster->GetHp();
    //
    SendMsgNearPosition_Far( MsgUpdateMstHp, pMonster->GetPosX(), pMonster->GetPosY() );
  }
}
//==========================================================================================
//
//
void CGameMap::CityGateIsCrash( const WORD & Id )
{
  CMonster      *pMonster = GetCityGate( Id );
  if( pMonster == NULL || pMonster->GetStatus() == STATUS_MONSTER_DISABLE ||
    !pMonster->IsDead() ) return;
  DWORD         dwDir = pMonster->GetParam1();
  if( dwDir > 1 )           return ;
  //
  WORD          wX = pMonster->GetPosX(), wY = pMonster->GetPosY();
  int           iIndex = 0;
  //
  for( int i = 0; i < 14; i++ )
  {
    iIndex = (wY+g_iCityGateObstacleY[dwDir][i])*m_iSizeX+(wX+g_iCityGateObstacleX[dwDir][i]);
    m_pMapOrg[iIndex] &= ~TILE_FULLOCCULDE;
  }
  //
  SMsgData       *pNewMsg = g_pGs->NewMsgBuffer();
  if( pNewMsg )
  {
    pNewMsg->Init();
    pNewMsg->dwAID        = A_OPENCITYGATE;
    pNewMsg->dwMsgLen     = 1;
    pNewMsg->Msgs[0].Size = sizeof( WORD );
    *(WORD*)(pNewMsg->Msgs[0].Data) = pMonster->GetCode();
    //
    SendTheMsgToAll( pNewMsg );
    //SendMsgNearPosition_Far( *pNewMsg, pMonster->GetPosX(), pMonster->GetPosY() );
    //g_pGs->ReleaseMsg( pNewMsg );
  }
}
//==========================================================================================
//
//
void CGameMap::OpenCityGate( const WORD & Id )
{
  CMonster      *pMonster = GetCityGate( Id );
  if( pMonster == NULL || pMonster->GetStatus() == STATUS_MONSTER_DISABLE ||
    pMonster->IsDead() )  return;
  DWORD         dwDir = pMonster->GetParam1();
  if( dwDir > 1 )           return ;
  //
  WORD          wX = pMonster->GetPosX(), wY = pMonster->GetPosY();
  int           iIndex = 0;
  //
  for( int i = 0; i < 14; i++ )
  {
    iIndex = (wY+g_iCityGateObstacleY[dwDir][i])*m_iSizeX+(wX+g_iCityGateObstacleX[dwDir][i]);
    m_pMapOrg[iIndex] &= ~TILE_FULLOCCULDE;
  }
  pMonster->SetState( STATUS_MONSTER_DISABLE );
  // Send The Msg To All Player In This Map
  if( m_mapCodePlayer.empty() )  return;
  //
  SMsgData       *pNewMsg = g_pGs->NewMsgBuffer();
  if( pNewMsg )
  {
    pNewMsg->Init();
    pNewMsg->dwAID        = A_OPENCITYGATE;
    pNewMsg->dwMsgLen     = 1;
    pNewMsg->Msgs[0].Size = sizeof( WORD );
    *(WORD*)(pNewMsg->Msgs[0].Data) = pMonster->GetCode();
    //
    SendTheMsgToAll( pNewMsg );
    //SendMsgNearPosition_Far( *pNewMsg, pMonster->GetPosX(), pMonster->GetPosY() );
    //g_pGs->ReleaseMsg( pNewMsg );
  }
}
//==========================================================================================
//
//
void CGameMap::CloseCityGate( const WORD & Id )
{
  CMonster      *pMonster = GetCityGate( Id );
  if( pMonster == NULL || pMonster->GetStatus() == STATUS_MONSTER_STAND ||
    pMonster->IsDead() )  return;
  DWORD         dwDir = pMonster->GetParam1();
  if( dwDir > 1 )           return;
  //
  WORD          wX = pMonster->GetPosX(), wY = pMonster->GetPosY();
  int           iIndex = 0;
  //
  for( int i = 0; i < 14; i++ )
  {
    iIndex = (wY+g_iCityGateObstacleY[dwDir][i])*m_iSizeX+(wX+g_iCityGateObstacleX[dwDir][i]);
    m_pMapOrg[iIndex] |= TILE_FULLOCCULDE;
  }
  pMonster->SetState( STATUS_MONSTER_STAND );
  // Send The Msg To All Player In This Map
  if( m_mapCodePlayer.empty() )   return;
  //
  SMsgData       *pNewMsg = g_pGs->NewMsgBuffer();
  if( pNewMsg )
  {
    pNewMsg->Init();
    pNewMsg->dwAID        = A_CLOSECITYGATE;
    pNewMsg->dwMsgLen     = 1;
    pNewMsg->Msgs[0].Size = sizeof( WORD );
    *(WORD*)(pNewMsg->Msgs[0].Data) = pMonster->GetCode();
    //
    SendTheMsgToAll( pNewMsg );
    //SendMsgNearPosition_Far( *pNewMsg, pMonster->GetPosX(), pMonster->GetPosY() );
    //g_pGs->ReleaseMsg( pNewMsg );
  }
}
//==========================================================================================
//
//
void CGameMap::CloseAllCityGate()
{
  list<CMonster*>::iterator     Iter_Ms;
  //
  for( Iter_Ms = m_listCityGate.begin(); Iter_Ms != m_listCityGate.end(); Iter_Ms++ )
  {
    CloseCityGate( (*Iter_Ms)->GetBaseId() );
  }
}
//==========================================================================================
//
//
void CGameMap::OpenAllCityGate()
{
  list<CMonster*>::iterator     Iter_Ms;
  //
  for( Iter_Ms = m_listCityGate.begin(); Iter_Ms != m_listCityGate.end(); Iter_Ms++ )
  {
    OpenCityGate( (*Iter_Ms)->GetBaseId() );
  }
}
//==========================================================================================
//
//
void CGameMap::FixAllCityGate()
{
  list<CMonster*>::iterator     Iter_Ms;
  //
  for( Iter_Ms = m_listCityGate.begin(); Iter_Ms != m_listCityGate.end(); Iter_Ms++ )
  {
    FixCityGate( (*Iter_Ms)->GetBaseId() );
  }
}
//==========================================================================================
//
//
bool CGameMap::AddCityGate( CMonster * pNewMonster )
{
  list<CMonster*>::iterator     Iter_Ms;
  //
  for( Iter_Ms = m_listCityGate.begin(); Iter_Ms != m_listCityGate.end(); Iter_Ms++ )
  {
    if( (*Iter_Ms)->GetBaseId() == pNewMonster->GetBaseId() )   return false;
  }
  m_listCityGate.push_back( pNewMonster );
  return true;
}
//==========================================================================================
//
//
bool CGameMap::AddMercenaryForCityWar( CSrvBaseMonster * pBaseMonster, const WORD & wPosX, const WORD & wPosY )
{
  if( !(pBaseMonster->GetProperty() & NPC_ATTRI_NEVER_REVIVE) )   return false;
  //
  WORD          wNewCode = GetNewMonsterCode();
  if( wNewCode >= CODE_MAX_MONSTER )                              return false;
  //
  CMonster            *pNewMonster = new CMonster( pBaseMonster, wNewCode );
  if( !pNewMonster->InitSkill() )
  {
    SAFE_DELETE( pNewMonster );
    return false;
  }
  //
  // 初始化据点标志物怪物的数据
  pNewMonster->InitDeadEvent();
  //
  pNewMonster->SetGameMap( this );
  pNewMonster->InitPos( wPosX, wPosY, wPosX, wPosY );
  pNewMonster->SetWanderRange();
  //
  CMonster            *pOtherMonster = NULL;
  if( pNewMonster == NULL )
  {
    return false;
  }
  // Set The Monster Into The Tile By Monster AI
  if( pNewMonster->IsDefenceGuild() )          // 协防城主同盟
  {
    if( !IsCityWarMap() )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    //
    if( !AddMonster( pNewMonster, wPosX, wPosY ) )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    pNewMonster->SetTeam( NULL );
    pNewMonster->SetTeamId( 0 );
  }
  else if( pNewMonster->IsDefenceOccupy() )    // 协防据点标志物
  {
    pOtherMonster = GetOccupyMonster();
    if( pOtherMonster == NULL )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    CMonsterTeamers   *pMTeam = pOtherMonster->GetTeam();
    if( pMTeam == NULL )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    if( !AddMonster( pNewMonster, wPosX, wPosY ) )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    pMTeam->AddMonster( pNewMonster );
    pNewMonster->SetTeam( pMTeam );
    pNewMonster->SetTeamId( pMTeam->GetTeamId() );
  }
  else if( pNewMonster->IsDefenceCityGate() )  // 协防城门
  {
    pOtherMonster = GetCityGate( pNewMonster->GetParam1() );
    if( pOtherMonster == NULL )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    CMonsterTeamers   *pMTeam = pOtherMonster->GetTeam();
    if( pMTeam == NULL )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    if( !AddMonster( pNewMonster, wPosX, wPosY ) )
    {
      SAFE_DELETE( pNewMonster );
      return false;
    }
    //
    pMTeam->AddMonster( pNewMonster );
    pNewMonster->SetTeam( pMTeam );
    pNewMonster->SetTeamId( pMTeam->GetTeamId() );
  }
  else
  {
    SAFE_DELETE( pNewMonster );
    return false;
  }
  AddMercenaryCount();
  pNewMonster->SetMercenary( 1 );
  InsertLifeToTile( wPosX, wPosY, pNewMonster );
  return TRUE;
  // NPC_ATTRI_NEVER_REVIVE
  // NPC_ATTRI_DEFENCE_CITYGATE
  // NPC_ATTRI_DEFENCE_LEAGUE
  // NPC_ATTRI_DEFENCE_OCCUPY
}
//==========================================================================================
//
//
bool CGameMap::AddPutMonster( CSrvBaseMonster* pBaseMonster, const WORD & wPosX, const WORD & wPosY, BOOL bView )
{
  WORD          wNewCode = GetNewMonsterCode();
  if( wNewCode >= CODE_MAX_MONSTER )                              return false;
  //
  if(bView)
  {
    m_wLastViewMonCode = wNewCode;
  }
  CMonster            *pNewMonster = new CMonster( pBaseMonster, wNewCode );
  if( pNewMonster == NULL )
  {
    return false;
  }
  if( !pNewMonster->InitSkill() )
  {
    SAFE_DELETE( pNewMonster );
    return false;
  }
  pNewMonster->InitDeadEvent();
  pNewMonster->SetGameMap( this );
  pNewMonster->InitPos( wPosX, wPosY, wPosX, wPosY );
  pNewMonster->SetWanderRange();
  //
//  CMonster            *pOtherMonster = NULL;

  if( !AddMonster( pNewMonster, wPosX, wPosY ) )
  {
    SAFE_DELETE( pNewMonster );
    return false;
  }
  InsertLifeToTile( wPosX, wPosY, pNewMonster );
  return true;
}
//==========================================================================================
//
//
bool CGameMap::AddRandomMonster( CMonster* pNewMonster )
{
  if( m_mapRandomMonster.find( pNewMonster->GetCode() ) == m_mapRandomMonster.end() )
  {
    m_mapRandomMonster.insert( map<DWORD,CMonster*>::value_type( pNewMonster->GetCode(), pNewMonster ) );
    return true;
  }
  return false;
}
#ifdef _NEW_CITY_WAR_2005_ //add by zetorchen 20050110
//==========================================================================================
//
//
bool CGameMap::AddCityWarMon(CMonster* pNewMonster, const WORD & iTheX, const WORD & iTheY)
{
 	pNewMonster->SetMapPos( this->m_pBase->GetMapId(), iTheX, iTheY );
  m_listCityWarMon.push_back(pNewMonster);
  return true;
}
//==========================================================================================
//
//
void CGameMap::InitMasterGift( DWORD dwItemId, DWORD dwCount )
{
  map<DWORD,DWORD>::iterator it;
  it = m_mapGiftItemCount.find( dwItemId );
  if( it == m_mapGiftItemCount.end() )
  {
    m_mapGiftItemCount[dwItemId] = dwCount;
  }
}
//==========================================================================================
//
//
void CGameMap::UnableCityWarMon()
{
  CMonster                          *pTheMonster = NULL;
  list<CMonster*>::iterator         listIT;
  map<DWORD, CMonster*>::iterator   mapIT;
  DWORD dwOutCount=0;
  SMsgData *pTheMsg = g_pGs->NewMsgBuffer();
  pTheMsg->Init();
  pTheMsg->dwAID             = A_CITYWARMONOUT;
  pTheMsg->dwMsgLen          = 1;
  DWORD *dwMonCode = ( DWORD * )( pTheMsg->Msgs[0].Data );
  dwMonCode++; //step over Count
  if( !m_listCityWarMon.empty() )
  {
    for( listIT = m_listCityWarMon.begin(); listIT != m_listCityWarMon.end(); listIT++ )
    {
      if( NULL != ( pTheMonster = *listIT ) )
      {
        if( m_mapCodeMonster.find( pTheMonster->GetSelfCode() ) != m_mapCodeMonster.end() )
        {
          *dwMonCode = pTheMonster->GetCode();
          DelTileCode(pTheMonster->GetPosX(), pTheMonster->GetPosY());
          pTheMonster->GoOutTile();
          m_mapCodeMonster.erase( pTheMonster->GetSelfCode() );
          m_dwViveMonster--;
          dwMonCode++;
          dwOutCount++;
          if( m_mapDefenceMonster.find( pTheMonster->GetSelfCode() ) != m_mapDefenceMonster.end() )
          {
            m_mapDefenceMonster.erase( pTheMonster->GetSelfCode() );
          }
        }
      }
    }
    dwMonCode = ( DWORD * )( pTheMsg->Msgs[0].Data );
    *dwMonCode = dwOutCount;
    pTheMsg->Msgs[0].Size = sizeof( DWORD ) + sizeof( DWORD ) * dwOutCount;   
    SendTheMsgToAll( pTheMsg );
  }
}
//==========================================================================================
//
//
void CGameMap::EnableCityWarMon()
{
  CMonster                       *pTheMonster = NULL;
  list<CMonster*>::iterator      listIT;
  if ( !m_listCityWarMon.empty() )
  {
    for( listIT = m_listCityWarMon.begin(); listIT != m_listCityWarMon.end(); listIT++ )
    {
      if( NULL != ( pTheMonster = (*listIT) ) )
      {
        if( m_mapCodeMonster.find( pTheMonster->GetSelfCode() ) == m_mapCodeMonster.end() )
        {
          m_mapCodeMonster.insert( map<DWORD, CMonster*>::value_type( pTheMonster->GetCode(), pTheMonster ) );
          m_dwViveMonster++;
          
          if( pTheMonster->GetType() & NPC_ATTRI_PROTECT_PLAYER ||
            pTheMonster->GetType() & NPC_ATTRI_ATTACK_CITY )
          {
            if( m_mapDefenceMonster.find( pTheMonster->GetSelfCode() ) == m_mapDefenceMonster.end() )
            {
              m_mapDefenceMonster.insert( map<DWORD,CMonster*>::value_type( pTheMonster->GetSelfCode(),pTheMonster ) );
            }
          }
        }
        pTheMonster->ReGoInCityWar();
      }
    }
  }
}
#endif // #endif For _NEW_CITY_WAR_2005_ 
//==========================================================================================
//
//
bool CGameMap::AddMonster(CMonster* pNewMonster, const WORD & iTheX, const WORD & iTheY)
{
  map<DWORD,CMonster*>::iterator		mIte;
  
  mIte = m_mapCodeMonster.find( pNewMonster->GetSelfCode() );
  if( mIte == m_mapCodeMonster.end() )
  {
    // Add new Monster into the map
    pNewMonster->SetMapPos( this->m_pBase->GetMapId(), iTheX, iTheY );
    m_mapCodeMonster.insert( map<DWORD,CMonster*>::value_type( pNewMonster->GetSelfCode(),
      pNewMonster ) );
    m_dwViveMonster++;
    // Add Defence Monster
    if( pNewMonster->GetType() & NPC_ATTRI_PROTECT_PLAYER ||
      pNewMonster->GetType() & NPC_ATTRI_ATTACK_CITY )
    {
      if( m_mapDefenceMonster.find( pNewMonster->GetSelfCode() ) ==
        m_mapDefenceMonster.end() )
      {
        m_mapDefenceMonster.insert( map<DWORD,CMonster*>::value_type( pNewMonster->GetSelfCode(),
          pNewMonster ) );
      }
    }
    else if( (pNewMonster->GetType()&NPC_ATTRI_OCCUPY_MONSTER) && m_pOccupyMonster == NULL )
    {
      m_pOccupyMonster = pNewMonster;
    }
    //else if( (pNewMonster->GetType() & NPC_ATTRI_DEFENCE_LEAGUE) && IsCityWarMap() )
    //{
    //  m_mapGuildMonster.insert( map<DWORD,CMonster*>::value_type( pNewMonster->GetSelfCode(),
    //                                                              pNewMonster ) );
    //}
    //
    return true;
  }
  else
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Code=%d Is Already In GameMap %s[%d]",
      pNewMonster->GetSelfCode(), m_pBase->GetMapName(), GetMapId() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddErrLogOnly( szMapLog );
  }
  return false;
}
//==========================================================================================
//
// No Release Monster Memory
bool CGameMap::DelMonster(const DWORD & dwCode,map<DWORD,CMonster*>::iterator* pIter)
{
  CMonster                          *pTheMonster = NULL;
  map<DWORD,CMonster*>::iterator		mIte;
  
  mIte = m_mapCodeMonster.find( dwCode );
  if( mIte != m_mapCodeMonster.end() )
  {
    pTheMonster = (CMonster*)(mIte->second);
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
    g_CrashLog.Write( "CGameMap::DelMonster ===>> 111" );
#endif
    //
    if(pIter)
    {
      (*pIter) = m_mapCodeMonster.erase( mIte );
    }
    else
    {
      m_mapCodeMonster.erase( mIte );
    }
    //
    if( !m_mapDisableMonster.empty() )   m_mapDisableMonster.erase( dwCode );
    if( !m_mapDefenceMonster.empty() )   m_mapDefenceMonster.erase( dwCode );
    //if( !m_mapGuildMonster.empty() )     m_mapGuildMonster.erase( dwCode );
    //
    if( pTheMonster->GetType() & NPC_ATTRI_GATE )
    {
      m_listCityGate.remove( pTheMonster );
    }
    //if( m_pOccupyMonster == pTheMonster )
    //{
    //  m_pOccupyMonster = NULL;
    //}
    //
    ReleaseMonsterCode( pTheMonster );
    //SAFE_DELETE( pTheMonster );
    m_dwViveMonster--;
    return true;
  }
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Find Monster Code=%d In GameMap %d", dwCode, m_pBase->GetMapId() );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddErrLogOnly( szMapLog );
  return false;
}
//==========================================================================================
//
// No Release Monster Memory
bool CGameMap::DelLastViewMonster()
{
  if(m_wLastViewMonCode)
  {
    DelMonster(m_wLastViewMonCode);
    m_wLastViewMonCode = 0;
    return true;
  }
  return false;
}
//==========================================================================================
//
//
bool CGameMap::AddNpc(CNpc* pNewNpc, const WORD & iTheX, const WORD & iTheY)
{
  
  map<DWORD,CNpc*>::iterator		nIte;
  
  nIte=m_mapCodeNpc.find(pNewNpc->GetSelfCode());
  if(nIte==m_mapCodeNpc.end())
  {
    // Add new Monster into the map
    pNewNpc->InitPos( iTheX, iTheY );
    pNewNpc->SetMapPos(this->m_pBase->GetMapId(), iTheX, iTheY);
    m_mapCodeNpc.insert(map<DWORD,CNpc*>::value_type(pNewNpc->GetSelfCode(),pNewNpc));
    //InsertLifeToTile(pNewNpc->GetPosX(),pNewNpc->GetPosY(),pNewNpc);
    return true;
  }
  else
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Npc Code=%d Is Already In GameMap %s", pNewNpc->GetSelfCode(), m_pBase->GetMapName() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddErrLogOnly(szMapLog);	
  }
  return false; 
}
//==========================================================================================
//
//
bool CGameMap::DelNpc(const DWORD & dwCode)
{
  CNpc                          *pTheNpc = NULL;
  map<DWORD,CNpc*>::iterator		mIte;
  
  mIte = m_mapCodeNpc.find( dwCode );
  if( mIte != m_mapCodeNpc.end() )
  {
    pTheNpc = (CNpc*)(mIte->second);
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
    g_CrashLog.Write( "CGameMap::DelNpc ===>> 111" );
#endif
    m_mapCodeNpc.erase( mIte );
    // 据点标志物Npc
    if( pTheNpc->GetNpcType() == EVENT_POINTTYPE_MONSTERDIE )
    {
      if( m_pOccupyMonster )      m_pOccupyMonster->SetEventNpc( NULL );
    }
    //
    ReleaseNpcCode(pTheNpc);
    SAFE_DELETE( pTheNpc );
    return true;
  }
  return false;
}
//==========================================================================================
//
//
bool CGameMap::AddGroundItem(CGroundItem* pNewItem)
{
  //FuncName("CGameMap::AddGroundItem");
  
  static map<DWORD,CGroundItem*>::iterator		nIte;
  static WORD																	wGX, wGY;
  
  nIte = m_mapCodeGroundItem.find(pNewItem->GetCode());
  if( nIte == m_mapCodeGroundItem.end() )
  {
    // Add new Monster into the map
    //pNewItem->InitPos( iTheX, iTheY );
    //pNewItem->SetMapPos(this->m_pBase->GetMapId(), iTheX, iTheY);
    wGX = pNewItem->GetPosX();
    wGY = pNewItem->GetPosY();
    //if( wGX < m_iSizeX && wGY < m_iSizeY )
    if( InsertItemToTile( wGX, wGY, pNewItem ) )
    {
      // Insert Ground Item To Tile
      //m_pTileGroundItem[wGY*m_iSizeX+wGX].push_front(pNewItem);
      // Insert Ground Item To List
      m_mapCodeGroundItem.insert(map<DWORD,CGroundItem*>::value_type(pNewItem->GetCode(),pNewItem));
      return true;
    }
  }
  else
  {
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "*** GroundItem Code=%d Is Already In GameMap %s ***", pNewItem->GetCode(), m_pBase->GetMapName() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddErrLogOnly(szMapLog);	
  }
  return false;
}
//==========================================================================================
//
//
bool CGameMap::DelGroundItem(const DWORD & dwCode, BOOL bRelease)
{
  //#ifdef _DEBUG_WILDCAT_
  //  FuncName("CGameMap::DelGroundItem");
  //#endif
  
  static map<DWORD,CGroundItem*>::iterator		mIte;
  static GroundItemTileLsit::iterator					gIte;
  static CGroundItem													*pDelItem = NULL;
  static WORD																	wGX, wGY;
  CItem                                *pDelCItem = NULL;
  
  mIte = m_mapCodeGroundItem.find(dwCode);
  if( mIte != m_mapCodeGroundItem.end() )
  {
    pDelItem = (CGroundItem*)( mIte->second );
    wGX = pDelItem->GetPosX();
    wGY = pDelItem->GetPosY();
    //if( wGX < m_iSizeX && wGY < m_iSizeY )
    if( DelItemFromTile( wGX, wGY, pDelItem ) )
    {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::DelGroundItem ===>> 111" );
#endif
      m_mapCodeGroundItem.erase( mIte );
      //ReleaseGroundItemCode( pItem );
      if( bRelease )
      {
        pDelCItem = pDelItem->GetBaseItem();
#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
        if( pDelCItem->GetPackagePos() != 0 )
        {
          FuncName( "CGameMap::DelGroundItem" );
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** 31 Delete Error Item=%d, Package Pos=%d ! *****", pDelCItem->GetId(), pDelCItem->GetPackagePos() );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szMapLog );
        }
#endif
        g_pGs->DeleteCItem( pDelCItem, 70 );
        pDelCItem = NULL;
        SAFE_DELETE( pDelItem );
      }
      return true;
    }
  }
#ifdef _DEBUG_WILDCAT_
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Cannot Find Ground Item Code=%d In GameMap %s", dwCode, m_pBase->GetMapName() );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  AddErrLogOnly( szMapLog );
#endif
  return false;
}
//==========================================================================================
//
//
bool CGameMap::DelLimitTimeItem(DWORD dwMapId, WORD wX, WORD wY, DWORD dwItemId)
{  
  static GroundItemTileLsit::iterator    iter;
  static map<DWORD,CGroundItem*>::iterator		mIte;
  static CGroundItem* pGroudItem  = NULL;
  static LPGroundItemTileLsit GroudItemList = NULL;
  static dwCode = 0;
  if(dwMapId != GetMapId())
    return false;
  GroudItemList = GetTileItemList(wX, wY);
  if( NULL == GroudItemList)
    return false;
  if(GroudItemList->empty())
    return false;
  iter = GroudItemList->begin();
  for(; iter != GroudItemList->end();)
  {
    pGroudItem = *iter;
    if(pGroudItem)
    {
      if(pGroudItem->GetId() == dwItemId)
      {
        dwCode = pGroudItem->GetCode();
        mIte = m_mapCodeGroundItem.find(dwCode);
        if( mIte != m_mapCodeGroundItem.end() )
        {
          m_mapCodeGroundItem.erase(mIte);
          *(DWORD*)(MsgClearCode.Msgs[0].Data)			  = pGroudItem->GetCode();
          SendMsgNearPosition(MsgClearCode, pGroudItem->GetPosX(), pGroudItem->GetPosY());
          iter = GroudItemList->erase(iter);
          SAFE_DELETE(pGroudItem);
        }
        else
          iter ++;
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }
  return true;
}
//==========================================================================================
//
//
bool CGameMap::AddMagic( CMagic * pNewMagic )
{
  map<DWORD,CMagic*>::iterator		iter;
  
  iter = m_mapCodeMagic.find( pNewMagic->GetCode() );
  
  if( iter != m_mapCodeMagic.end() )
  {
#ifdef _DEBUG_WILDCAT_
    _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "The Magic(%d) Is Existed In Map[%s] #", pNewMagic->GetCode(), GetName() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    AddErrLogOnly( szMapLog );
#endif
    return false;
  }
  m_mapCodeMagic.insert( map<DWORD,CMagic*>::value_type( pNewMagic->GetCode(), pNewMagic ) );
  
  //#ifdef _DEBUG_WILDCAT_
  //	sprintf( szMapLog, "Add The Magic(%d) Into Map(%s) #", pNewMagic->GetCode(), GetName() );
  //	AddErrLogOnly(szMapLog);
  //#endif
  
  return true;
}
//==========================================================================================
//
//
bool CGameMap::DelMagic(CMagic* pTheMagic)
{
  //	map<DWORD, CMagic*>::iterator		iter;
  //
  //	iter = m_mapCodeMagic.find( pTheMagic->GetCode() );
  //	if( iter != m_mapCodeMagic.end() )
  //	{
  //		ReleaseMagicCode( pTheMagic );
  //#ifdef _DEBUG_WILDCAT_CRASH_INFO_
  //    g_CrashLog.Write( "CGameMap::DelMagic ===>> 111" );
  //#endif
  //		m_mapCodeMagic.erase( iter );
  //		return true;
  //	}
  
  //#ifdef _DEBUG_WILDCAT_
  //  sprintf(szMapLog, "Cannot Find Magic Code=%d In GameMap %d", pTheMagic->GetCode(), GetMapId() );
  //	AddErrLogOnly( szMapLog );
  //#endif
  
  return false;
}
//==========================================================================================
//
//
void CGameMap::MonsterDoAction()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::MonsterDoAction()");
#endif
  
  //CPlayer		*pThePlayer2 = NULL;
  CMonster	                        *pTheMonster = NULL;
  map<DWORD,CMonster*>::iterator		mIte;
  
  // For Test -- WildCat 2002-7-31
  //return;
  //DWORD dwX = 60, dwY = 40;
  //DWORD dwFlag = *(m_pMapOrg + dwX + dwY * m_iSizeX);
  
  // Monster Action and Add A_Action to the Players in the Neighterhood
  if( 0 == m_mapCodeMonster.size() ) 
    return;
  for( mIte = m_mapCodeMonster.begin(); mIte!= m_mapCodeMonster.end(); mIte++ )
  {
    pTheMonster = mIte->second;
    if( NULL == pTheMonster )
      continue;
    pTheMonster->DoActionEX();
  }
  //
  if( !m_mapRandomMonster.empty() )
  {
    for( mIte = m_mapRandomMonster.begin(); mIte != m_mapRandomMonster.end(); )
    {
      pTheMonster = mIte->second;
      pTheMonster->RandomMapRevive();
      //
      mIte = m_mapRandomMonster.erase( mIte );
    }
  }
  // 关掉下面的功能，为避免造成怪物重生的错误发生
#ifdef _DISABLE_THIS_FUNCTION_
  if( m_dwViveMonster < ( m_mapCodeMonster.size() / 5 + 1 ) &&
    m_mapCodeMonster.size() != 1 )
  {
    for( mIte = m_mapCodeMonster.begin(); mIte!= m_mapCodeMonster.end(); mIte++ )
    {
      pTheMonster = (CMonster*)(mIte->second);
      if( pTheMonster->GetStatus() == STATUS_MONSTER_DEAD )
      {
        pTheMonster->Revive();
      }
      if( m_dwViveMonster > ( m_mapCodeMonster.size() / 2 ) )
      {
        break;
      }
    }
  }
#endif
}
//==========================================================================================
//
// Add by zetorchen
#ifdef _NEW_CITY_WAR_2005_ 
void CGameMap::DoIncreaseGiftItem()
{
  DWORD dwGiftByTaxRate;
  if( ( dwGiftByTaxRate = g_pGuildMng->GetGiftAddByTax( (WORD) m_pBase->m_dwMapId ) ) != 0 )
  {  
    DWORD dwNowTime = TimeGetTime();
    if ( dwNowTime > m_dwSaveGiftItemTrigger[0] )
    {
      // Add Item Num
      AddAllMasterGift( dwGiftByTaxRate );
      m_dwSaveGiftItemTrigger[0] = dwNowTime + 300000;/*300000*/ // 5Min
      if ( dwNowTime > m_dwSaveGiftItemTrigger[1] )
      {
        // Save file
        SaveAllMasterGift();
        m_dwSaveGiftItemTrigger[1] = dwNowTime + 1800000/*1800000*/; // 30Min 
      }
    }
  }
}
#endif
//==========================================================================================
//
//
void CGameMap::NpcDoAction()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::NpcDoAction()");
#endif
  
  CPlayer *pThePlayer2 = NULL;
  
  // Npc Action and Add A_Action to the Players in the Neighterhood
  for(map<DWORD,CNpc*>::iterator iter_Npc = m_mapCodeNpc.begin(); iter_Npc != m_mapCodeNpc.end(); iter_Npc++)
  {
    iter_Npc->second->DoAction();
  }
}
//==========================================================================================
//
//
void CGameMap::GroundItemDoAction()
{
  static CGroundItem												*pItem = NULL;
  static CItem															*pBaseItem;
  static map<DWORD, CGroundItem*>::iterator	iter;
  static DWORD															dwNow = 0;
  
  for( iter = m_mapCodeGroundItem.begin(); iter != m_mapCodeGroundItem.end(); )
  {
    if( NULL != ( pItem = (iter->second) ) )
    {
      if( ClientTickCount > pItem->GetVanishTime() )
      {
        // Send Clear Code
        // Delete The Ground Item From Map And Release Memory
        if( DelItemFromTile( pItem->GetPosX(), pItem->GetPosY(), pItem ) )
        {
          *(DWORD*)MsgClearCode.Msgs[0].Data = pItem->GetCode();
          SendMsgNearPosition( MsgClearCode, pItem->GetPosX(), pItem->GetPosY() );
          
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
          g_CrashLog.Write( "CGameMap::GroundItemDoAction ===>> 111" );
#endif
          iter = m_mapCodeGroundItem.erase( iter );
          if( NULL != ( pBaseItem = pItem->GetBaseItem() ) )
          {
#ifdef _DEBUG_I_WANT_FIND_ITEM_CRASH_
            if( pBaseItem->GetPackagePos() != 0 )
            {
              FuncName( "CGameMap::GroundItemDoAction" );
              _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** 32 Delete Error Item=%d, Package Pos=%d ! *****", pBaseItem->GetId(), pBaseItem->GetPackagePos() );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoErrMsg( szMapLog );
            }
#endif
            g_pGs->DeleteCItem( pBaseItem, 71 );
            pBaseItem = NULL;
          }
          SAFE_DELETE( pItem );
        }
        else
        {
          iter++;
        }
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }
}
//==========================================================================================
//
//
void CGameMap::MagicDoAction()
{
  CMagic					              *pTheMagic = NULL;
  map<DWORD, CMagic*>::iterator Iter_Magic;
  
  // Magic Action and Add A_Action to the Players in the Neighterhood
  for( Iter_Magic = m_mapCodeMagic.begin(); Iter_Magic != m_mapCodeMagic.end(); )
  {
    if( ( pTheMagic = (CMagic*)(Iter_Magic->second) ) == NULL )
    {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::MagicDoAction ===>> 111" );
#endif
      Iter_Magic = m_mapCodeMagic.erase( Iter_Magic );
    }
    else
    {
      switch( pTheMagic->GetStatus() )
      {
      case MAGICSTATE_DEAD:
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        g_CrashLog.Write( "CGameMap::MagicDoAction ===>> 222" );
#endif
        pTheMagic->SetState( MAGICSTATE_NONE );
        //ReleaseMagicCode( pTheMagic );
        Iter_Magic = m_mapCodeMagic.erase( Iter_Magic );
#ifdef VERSION_38_FUNCTION
        PopOutTrapMagic( pTheMagic );
        SendMsgSpecialStatus( NULL, SPE_STATUS_EAGLE_EYE );
#endif
        g_pGs->DeleteCMagic( pTheMagic );
        break;
      default:
        pTheMagic->DoAction();
        Iter_Magic++;
        break;
      }
    }
  }
}
//=====================================================================================
//
//
void CGameMap::CityWarDoAction()
{
  if( g_pGuildMng->GetBaseCityWarMap() == m_pBase->GetMapId() )
  {
#ifdef _DEBUG
    DWORD dwTime = g_pGuildMng->GetCityWarTime();
#endif
    if( ClientTickCount > g_pGuildMng->GetCityWarTime() )
    {
      SMccMsgData          *pNewMccMsg = g_pGs->NewMccMsgBuffer();
      if( pNewMccMsg )
      {
        pNewMccMsg->Init( NULL );
        pNewMccMsg->dwAID        = AP_CITYWAR_END;
        pNewMccMsg->dwMsgLen     = 1;
        pNewMccMsg->Msgs[0].Size = sizeof( SCityWarEnd );
        SCityWarEnd       *pCityWarEnd = (SCityWarEnd*)(pNewMccMsg->Msgs[0].Data);
        //
#ifdef _NEW_CITY_WAR_2005_
        pCityWarEnd->dwGuildId = 0;
#else
        CGuild          *pGuild = g_pGuildMng->FindGuildByMap( GetMapId() );
        if( pGuild )    pCityWarEnd->dwGuildId = pGuild->GetGuildId();
        else            pCityWarEnd->dwGuildId = 0; // Npc Occupy The City
#endif
        pCityWarEnd->wMapId = m_pBase->GetMapId();
        //
        g_pMccDB->AddSendMsg( pNewMccMsg );
        g_pGuildMng->SetCityWarTime( ClientTickCount + 300000 );    //  自动延时5分钟
      }
    }
  }
}
#ifdef _NEW_CITY_WAR_2005_
//=====================================================================================
//
//
DWORD CGameMap::GetGiftItemCount(DWORD dwItemId)
{
  if(m_mapGiftItemCount.find(dwItemId)!=m_mapGiftItemCount.end())
  {
    DWORD dwTemp = (DWORD)HIWORD(m_mapGiftItemCount[dwItemId]);
    m_mapGiftItemCount[dwItemId] = (DWORD)LOWORD(m_mapGiftItemCount[dwItemId]);
    return dwTemp;
  }
  return 0;
}
//=====================================================================================
//
//
void CGameMap::SaveAllMasterGift()
{
  if( m_mapGiftItemCount.size() )
  {
    FILE *pSave;
    char szFileName[256];
    _snprintf( szFileName, 256-1, "%s/CityMasterGift%d.txt", g_pBase->GetMapFilePath(), m_pBase->m_dwMapId );
    szFileName[256-1] = '\0';
    pSave = fopen( szFileName, "w" );
    if( pSave )
    {
      fprintf( pSave, "%d ", m_mapGiftItemCount.size() );
      map<DWORD,DWORD>::iterator itSave;
      for( itSave = m_mapGiftItemCount.begin(); itSave != m_mapGiftItemCount.end(); itSave++ )
      {
        fprintf( pSave, "%d %d\n", itSave->first, itSave->second );
      }
    }
    fclose( pSave );
  }
}
//=====================================================================================
//
//
void CGameMap::AddAllMasterGift( DWORD dwRate )
{
  if(dwRate<100)return;
  if( m_mapGiftItemCount.size() )
  {
    map<DWORD,DWORD>::iterator itMap;
    map<DWORD,DWORD>::iterator itAll;
    for( itMap = m_mapGiftItemCount.begin(); itMap != m_mapGiftItemCount.end(); )
    {
      DWORD dwMin_Max = g_pGuildMng->GetGiftMin_Max( MAKELONG(itMap->first,m_pBase->m_dwMapId ) );
      if(dwMin_Max!=0)
      {
        itMap->second += (PER_UNIT_TIME_ADD * HIWORD(dwMin_Max)) + //一城五村TAXRATE为满时的基数
          ((300-dwRate)/200.0)*(LOWORD(dwMin_Max)-HIWORD(dwMin_Max))*PER_UNIT_TIME_ADD;
#ifdef _DEBUG
        DWORD dwiii = itMap->second;
#endif
        itMap++;
      }
      else
      {
        itMap = m_mapGiftItemCount.erase(itMap);
      }
    }
  }
}
//=====================================================================================
//
//
void CGameMap::CityWarOverToMcc(SMccMsgData* pNewMccMsg)
{
  if( pNewMccMsg )
  {
    g_pMccDB->AddSendMsg( pNewMccMsg );
    g_pGuildMng->SetCityWarTime( ClientTickCount + 300000 );    //  自动延时5分钟
  }
}
#endif
//=====================================================================================
//
//Add By Zetorchen For New Function In 3.8
#ifdef VERSION_38_FUNCTION
typedef struct _TrapBuffer
{
  WORD wX;
  WORD wY;
  DWORD dwTime;
}TrapBuffer, *LPTrapBuffer;
BOOL CGameMap::FillTrapBuffer( SMsgData *pTheMsg )
{
  if(m_listTrapMagic.size())
  {
    DWORD dwNowTime = TimeGetTime();
    WORD wTrapNum = m_listTrapMagic.size()>500?500:m_listTrapMagic.size();
    list<CMagic*>::iterator Iter;
    pTheMsg->Msgs[0].Size = sizeof(WORD);
    *(WORD*)pTheMsg->Msgs[0].Data = wTrapNum;
    pTheMsg->Msgs[1].Size = sizeof(TrapBuffer)*wTrapNum;
    LPTrapBuffer pTrapBuffer = (LPTrapBuffer)pTheMsg->Msgs[1].Data;
    //
    int i = 0;
    for( Iter = m_listTrapMagic.begin(); ( i<wTrapNum && Iter != m_listTrapMagic.end()); i++,Iter++ )
    {
      CMagic* pTrap = *Iter;
      if( pTrap )
      {
        DWORD dwLifeLeft = pTrap->GetLifeTime()-dwNowTime;
        if(dwLifeLeft>3000)
        {
          pTrapBuffer->wX = pTrap->GetPosX();
          pTrapBuffer->wY = pTrap->GetPosY();
          pTrapBuffer->dwTime = dwLifeLeft;
          pTrapBuffer++;
        }
      }
    }
    return TRUE;
  }
  else
  {
    pTheMsg->Msgs[0].Size = sizeof(WORD);
    *(WORD*)pTheMsg->Msgs[0].Data = 0;
    pTheMsg->Msgs[1].Size = 0;
    return TRUE;
  }
}
#define MAX_SPECIALSTATUSPLAYER_FOR_SEND 100
void CGameMap::SendMsgSpecialStatus( SMsgData * pTheMsgData, QWORD qwSpe )
{
  if( 0 == qwSpe ) return;
  static CPlayer		                    *pThePlayer;
  static SMsgData		                    *pTheNewMsg;
  static BOOL				                    bSendMessage;
  static map<DWORD,CPlayer*>::iterator  iter_Player;
  static WORD                           g_wSpePlayer = 0;
  
  bSendMessage  = FALSE;
  g_wSpePlayer = 0;
  pTheNewMsg    = NULL;
  
  for( iter_Player = m_mapCodePlayer.begin(); iter_Player != m_mapCodePlayer.end(); )
  {
    if( ( pThePlayer = ( CPlayer* )(iter_Player->second) ) == NULL )
    { 
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::SendMsgSpecialStatus ===>> 444" );
#endif
      iter_Player = m_mapCodePlayer.erase(iter_Player);
    }
    else if( pThePlayer->GetSpecialStatus() & qwSpe )
    {          
      if(pTheMsgData == NULL) //All spe player get all trap info
      {
        pThePlayer->Send_A_ALLTRAP();
      }
      else if( !bSendMessage )
      {
        pTheNewMsg = g_pGs->NewMsgBuffer();
        if( pTheNewMsg == NULL )
        {
          return;
        }
        if ( qwSpe == SPE_STATUS_EAGLE_EYE )
        {
          pTheNewMsg->Init();
          pTheNewMsg->dwAID = A_EAGLEEYE;
          pTheNewMsg->dwMsgID = pTheMsgData->dwMsgID;
          SNMUseTrap  *lpInfo   =(SNMUseTrap*)(pTheMsgData->Msgs[0].Data);
          pTheNewMsg->dwMsgLen = 2;
          pTheNewMsg->Msgs[0].Size = sizeof(WORD);
          pTheNewMsg->Msgs[1].Size = sizeof(DWORD);
          *(WORD*)pTheNewMsg->Msgs[0].Data = lpInfo->wSkillId;
          *(DWORD*)pTheNewMsg->Msgs[1].Data = MAKELONG(lpInfo->wTargetY,lpInfo->wTargetX);
          bSendMessage = TRUE;
        }
        else
        {
          ::CopyMessage( *pTheMsgData, pTheNewMsg );
          bSendMessage = TRUE;
        }
      }
      g_pMultiSendPlayer[g_wSpePlayer] = pThePlayer;
      if( ++g_wSpePlayer >= MAX_SPECIALSTATUSPLAYER_FOR_SEND ) break;
      iter_Player++;
    }
    else
    {
      iter_Player++;
    }
  }
  //
  if( g_wSpePlayer == 0 || pTheMsgData == NULL )  g_pGs->ReleaseMsg( pTheNewMsg );
  else
  {
    g_pGs->AddRef( pTheNewMsg, g_wSpePlayer-1 );
    for( int sss = 0; sss < g_wSpePlayer; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheNewMsg );
  }
}
#endif
//=====================================================================================
//
//
#define MAX_NEAR_PLAYER_FOR_SEND    100
void CGameMap::SendMsgNearPosition( const SMsgData & TheMsgData, const WORD & iTheX, const WORD & iTheY)
{
  static CPlayer		                    *pThePlayer;
  static SMsgData		                    *pTheNewMsg;
  static BOOL				                    bSendMessage;
  static map<DWORD,CPlayer*>::iterator  iter_Player;
  static WORD                           g_wNearPlayer = 0;
  
#ifdef _DEBUG_WILDCAT_SEND_NEAR_BY_TILE_
  // 
#else
  bSendMessage  = FALSE;
  g_wNearPlayer = 0;
  pTheNewMsg    = NULL;
  // Find Players in the Neighterhood
  for( iter_Player = m_mapCodePlayer.begin(); iter_Player != m_mapCodePlayer.end(); )
  {
    if( ( pThePlayer = ( CPlayer* )(iter_Player->second) ) == NULL )
    { 
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::SendMsgNearPosition ===>> 111" );
#endif
      iter_Player = m_mapCodePlayer.erase(iter_Player);
    }
    else if( pThePlayer->GetCltDis( iTheX, iTheY ) <= MAX_NEAR_DISTANCE )
    {
      if( bSendMessage == FALSE )
      {
        // Init Message Buffer
        pTheNewMsg = g_pGs->NewMsgBuffer();
        if( pTheNewMsg == NULL )
        {
#ifdef _DEBUG_WILDCAT_
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
          FuncName("CGameMap::SendMsgNearPosition");
          _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "===>>> Cannot New Msg, AID=%d", TheMsgData.dwAID );
          szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
          AddMemoErrMsg( szMapLog );
#endif
#endif
          return;
        }
        ::CopyMessage( TheMsgData, pTheNewMsg );
        bSendMessage = TRUE;
      }
      g_pMultiSendPlayer[g_wNearPlayer] = pThePlayer;
      //pThePlayer->AddSendMsg( pTheNewMsg );
      if( ++g_wNearPlayer >= MAX_NEAR_PLAYER_FOR_SEND ) break;
      iter_Player++;
    }
    else
    {
      iter_Player++;
    }
  }
  //
  if( g_wNearPlayer == 0 )  g_pGs->ReleaseMsg( pTheNewMsg );
  else
  {
    g_pGs->AddRef( pTheNewMsg, g_wNearPlayer-1 );
    for( int sss = 0; sss < g_wNearPlayer; sss++ )
    {
      if( g_pMultiSendPlayer[sss] )
        g_pMultiSendPlayer[sss]->AddSendMsg( pTheNewMsg );
    }
  }
#endif
}
//=====================================================================================
//
//
void CGameMap::SendMsgNearPosition_Close( const SMsgData & TheMsgData, const WORD & iTheX, const WORD & iTheY)
{
  //FuncName("CGameMap::SendMsgNearPosition");
  
  static CPlayer		                    *pThePlayer;
  static SMsgData		                    *pTheNewMsg;
  static BOOL				                    bSendMessage;
  static map<DWORD,CPlayer*>::iterator  iter_Player;
  static WORD                           g_wNearPlayer = 0;
  
#ifdef _DEBUG_WILDCAT_SEND_NEAR_BY_TILE_
  // 
#else
  bSendMessage  = FALSE;
  g_wNearPlayer = 0;
  pTheNewMsg    = NULL;
  //
  //int       iWarpPoint = m_mapCodeWarpPoint.size();
  // Find Players in the Neighterhood
  for( iter_Player = m_mapCodePlayer.begin(); iter_Player != m_mapCodePlayer.end(); )
  {
    if( ( pThePlayer = ( CPlayer* )(iter_Player->second) ) == NULL )
    { 
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::SendMsgNearPosition_Close ===>> 111" );
#endif
      iter_Player = m_mapCodePlayer.erase(iter_Player);
    }
    else if( pThePlayer->GetCltDis( iTheX, iTheY ) <= MAX_NEAR_DSITANCE_CLOSE )
    {
      if( bSendMessage == FALSE )
      {
        // Init Message Buffer
        pTheNewMsg = g_pGs->NewMsgBuffer();
        if( pTheNewMsg == NULL )					return;
        ::CopyMessage( TheMsgData, pTheNewMsg );
        bSendMessage = TRUE;
      }
      g_pMultiSendPlayer[g_wNearPlayer] = pThePlayer;
      //pThePlayer->AddSendMsg( pTheNewMsg );
      if( ++g_wNearPlayer >= MAX_NEAR_PLAYER_FOR_SEND ) break;
      iter_Player++;
    }
    else
    {
      iter_Player++;
    }
  }
  //
  if( g_wNearPlayer == 0 )  g_pGs->ReleaseMsg( pTheNewMsg );
  else
  {
    g_pGs->AddRef( pTheNewMsg, g_wNearPlayer-1 );
    for( int sss = 0; sss < g_wNearPlayer; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheNewMsg );
  }
#endif
}
//=====================================================================================
//
//
void CGameMap::SendMsgNearPosition_Far( const SMsgData & TheMsgData, const WORD & iTheX, const WORD & iTheY)
{
  //FuncName("CGameMap::SendMsgNearPosition");
  
  static CPlayer		                    *pThePlayer;
  static SMsgData		                    *pTheNewMsg;
  static BOOL				                    bSendMessage;
  static map<DWORD,CPlayer*>::iterator  iter_Player;
  static WORD                           g_wNearPlayer = 0;
  
#ifdef _DEBUG_WILDCAT_SEND_NEAR_BY_TILE_
  //
#else
  bSendMessage  = FALSE;
  g_wNearPlayer = 0;
  // Find Players in the Neighterhood
  for( iter_Player = m_mapCodePlayer.begin(); iter_Player != m_mapCodePlayer.end(); )
  {
    if( ( pThePlayer = ( CPlayer* )(iter_Player->second) ) == NULL )
    { 
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::SendMsgNearPosition_Far ===>> 111" );
#endif
      iter_Player = m_mapCodePlayer.erase(iter_Player);
    }
    else if( pThePlayer->GetCltDis( iTheX, iTheY ) <= MAX_NEAR_DISTANCE_FAR )
    {
      if( bSendMessage == FALSE )
      {
        // Init Message Buffer
        pTheNewMsg = g_pGs->NewMsgBuffer();
        if( pTheNewMsg == NULL )			  return;
        ::CopyMessage( TheMsgData, pTheNewMsg );
        bSendMessage = TRUE;
      }
      g_pMultiSendPlayer[g_wNearPlayer] = pThePlayer;
      //pThePlayer->AddSendMsg( pTheNewMsg );
      if( ++g_wNearPlayer >= MAX_NEAR_PLAYER_FOR_SEND )   break;
      iter_Player++;
    }
    else
    {
      iter_Player++;
    }
  }
  //
  if( g_wNearPlayer == 0 )  g_pGs->ReleaseMsg( pTheNewMsg );
  else
  {
    g_pGs->AddRef( pTheNewMsg, g_wNearPlayer-1 );
    for( int sss = 0; sss < g_wNearPlayer; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheNewMsg );
  }
#endif
}
//=====================================================================================
//
//
void CGameMap::PlayerGetNeighterhoodInfo()
{
  static CPlayer				*pThePlayer  = NULL;
  static CPlayer				*pThePlayer2 = NULL;
  static CNpc						*pTheNpc     = NULL;
  static CMonster				*pTheMonster = NULL;
  static CMagic					*pTheMagic   = NULL;
  static CGroundItem		*pTheItem    = NULL;
  
  static map<DWORD,CPlayer*>::iterator			  iter_Player, iter_Player2;
  static map<DWORD,CMonster*>::iterator			  iter_Monster;
  static map<DWORD,CNpc*>::iterator				    iter_Npc;
  static map<DWORD,CGroundItem*>::iterator	  iter_Item;
  
  int           iCount, iSizeAction = ( MAXMSGDATASIZE / sizeof( SNMAction ) ) - 10;
  SMsgData	    *pTheMsg     = NULL;
  SNMAction     *pAllAction  = NULL;
  SNMSetHpMpSp  *pHpMpSp     = NULL;
  
  for( iter_Player = m_mapCodePlayer.begin(); iter_Player != m_mapCodePlayer.end(); )
  {
    pThePlayer = (CPlayer*)(iter_Player->second);
    if( pThePlayer == NULL )
    {
      iter_Player = m_mapCodePlayer.erase( iter_Player );
      continue;
    }
    else if( pThePlayer->GetInMap() == NULL || pThePlayer->GetInMap() != this )
    {
      //pThePlayer->AddSendErrorMsg( ERROR_CODE_VALUEINVALID, 99 );
      //pThePlayer->SetClientState( CLIENTSTATE_LOGOUT );
      iter_Player++;
      continue;
    }
    // Find Players in the Neighterhood
    if( pThePlayer->BeUpdated() && pThePlayer->GetActionStatus() != ACTION_STATUS_WARP )
    {
      iCount = 0;
      if( NULL == ( pTheMsg = g_pGs->NewMsgBuffer( pThePlayer->GetCode() ) ) )
      {
        if( NULL == ( pTheMsg = g_pGs->NewMsgBuffer() ) ) return;
      }
      
      pTheMsg->Init();
      pTheMsg->dwMsgLen     = 2;
      pTheMsg->Msgs[1].Size = sizeof(SNMSetHpMpSp);
      //
      pAllAction    = (SNMAction*)(pTheMsg->Msgs[0].Data);
      pHpMpSp       = (SNMSetHpMpSp*)(pTheMsg->Msgs[1].Data);
      pHpMpSp->dwHp = pThePlayer->GetHp();
      pHpMpSp->wMp  = pThePlayer->GetMp();
      pHpMpSp->wSp  = pThePlayer->GetSp();
      pHpMpSp->dwSoul = pThePlayer->GetSoul();
      
      for( iter_Player2 = m_mapCodePlayer.begin(); iter_Player2 != m_mapCodePlayer.end(); iter_Player2++)
      {
        pThePlayer2 = (CPlayer*)(iter_Player2->second);
        //
        if( pThePlayer2 == NULL )         continue;
        //
        if( pThePlayer->GetCltDis( pThePlayer2->GetPosX(), pThePlayer2->GetPosY() ) <= MAX_NEAR_DISTANCE && !(pThePlayer2->GetGMStatus() & GM_STATUS_ANCHORET) )
        {
          pThePlayer2->GetSNMAction( pAllAction );
          pAllAction++;
          if( ++iCount >= iSizeAction )
          {
            iCount--;
            pTheMsg->dwAID        = A_ACTION;
            pTheMsg->Msgs[0].Size = sizeof(SNMAction) * iCount;
            pThePlayer->AddSendMsg(pTheMsg);
            return;
          }
        }
      }
      // Find Monsters in the Neighberhood
      for( iter_Monster = m_mapCodeMonster.begin(); iter_Monster != m_mapCodeMonster.end(); )
      {
        pTheMonster = (CMonster*)(iter_Monster->second);
        if( pTheMonster == NULL )
        {
          iter_Monster = m_mapCodeMonster.erase( iter_Monster );
          continue;
        }
        if( pTheMonster->GetStatus() != STATUS_MONSTER_DEAD &&
          pTheMonster->GetCltDis( pThePlayer->GetPosX(), pThePlayer->GetPosY() ) <= MAX_NEAR_DISTANCE )
        {
          pTheMonster->GetSNMAction( pAllAction );
          pAllAction++;
          if( ++iCount >= iSizeAction )
          {
            iCount--;
            pTheMsg->dwAID				= A_ACTION;
            pTheMsg->Msgs[0].Size = sizeof(SNMAction) * iCount;
            pThePlayer->AddSendMsg( pTheMsg );
            return;
          }
        }
        iter_Monster++;
      }
      ///////////////////////////////////////////////////////////////////////////////////////
      //Add by CECE 2004-04-15
#ifdef EVILWEAPON_3_6_VERSION
      SWarpPoint     *pWarpPoint = NULL;
      map<DWORD, SWarpPoint*>::iterator Iter_Wp;
      //
      for( Iter_Wp = m_mapCodeWarpPoint.begin(); Iter_Wp != m_mapCodeWarpPoint.end(); Iter_Wp++ )
      {
        pWarpPoint = (*Iter_Wp).second;
        if( pWarpPoint && pWarpPoint->wType == WARPPOINT_TYPE_EVIL &&
          ( pWarpPoint->GetCltDis( pThePlayer->GetPosX(), pThePlayer->GetPosY() ) <= MAX_NEAR_DISTANCE ) )
        {
          pWarpPoint->GetSNMAction( pAllAction );
          pAllAction++;
          //
          if( ++iCount >= iSizeAction )
          {
            iCount--;
            pTheMsg->dwAID        = A_ACTION;
            pTheMsg->Msgs[0].Size = sizeof(SNMAction) * iCount;
            pThePlayer->AddSendMsg( pTheMsg );
            return;
          }
        }
      }
#endif
      ///////////////////////////////////////////////////////////////////////////////////////
      
      // Find NPC in the Neighterhood
      for( iter_Npc = m_mapCodeNpc.begin(); iter_Npc != m_mapCodeNpc.end(); iter_Npc++)
      {
        pTheNpc = (CNpc*)(iter_Npc->second);
        if( pTheNpc->GetCltDis( pThePlayer->GetPosX(), pThePlayer->GetPosY() ) <= MAX_NEAR_DISTANCE )
        {
          pTheNpc->GetSNMAction( pAllAction );
          pAllAction++;
          if( ++iCount >= iSizeAction )
          {
            iCount--;
            pTheMsg->dwAID = A_ACTION;
            pTheMsg->Msgs[0].Size = sizeof(SNMAction)*iCount;
            pThePlayer->AddSendMsg(pTheMsg);
            return;
          }
        }
      }
      // Find Magic in the Neighterhood
      //for(map<DWORD,CMagic*>::iterator iter_Magic = m_mapCodeMagic.begin(); iter_Magic != m_mapCodeMagic.end(); iter_Magic++)
      //{
      //	pTheMagic=(CMagic*)(iter_Magic->second);
      //	assert(pTheMagic);
      //if( (pTheMagic=(CMagic*)(iter_Magic->second)) == NULL )
      //{ //sprintf(szMapLog, "GameMap %s Npc list node = NULL", szMapName); //AddMemoErrMsg(szMapLog);
      //  iter_Magic = (m_mapCodeMagic.erase(iter_Magic));
      //	if( iter_Magic == m_mapCodeMagic.end() )
      //		break;
      //	continue;
      //}
      //else if
      //	if( pTheMagic->BeUpdated() && pTheMagic->GetDistance(pThePlayer->GetPosX(), pThePlayer->GetPosY()) <= MAX_NEAR_DISTANCE)
      //	{
      //		pTheMagic->GetSNMAction((pAllAction+iCount));
      //		if( ++iCount >= MAXMSGDATASIZE / iSizeAction )
      //		{
      //			pTheMsg->dwAID = A_ACTION;
      //			pTheMsg->dwMsgLen = 1;
      //			pTheMsg->DataSize[0] = sizeof(SNMAction)*iCount;
      //			pThePlayer->AddSendMsg(pTheMsg);
      //			return;
      //		}
      //	}
      //}
      // Find Obstale in the Neighterhood
      
      
      
      // Find Ground Item in the Neighterhood
      for( iter_Item = m_mapCodeGroundItem.begin(); iter_Item != m_mapCodeGroundItem.end(); )
      {
        pTheItem = (CGroundItem*)(iter_Item->second);
        if( pTheItem == NULL )
        {
          iter_Item = m_mapCodeGroundItem.erase( iter_Item );
          continue;
        }
        if( pThePlayer->GetCltDis( pTheItem->GetPosX(), pTheItem->GetPosY() ) <= MAX_NEAR_DISTANCE)
        {
          pTheItem->GetSNMAction( *pAllAction );
          pAllAction++;
          if( ++iCount >= iSizeAction )
          {
            iCount--;
            pTheMsg->dwAID = A_ACTION;
            pTheMsg->Msgs[0].Size = sizeof(SNMAction)*iCount;
            pThePlayer->AddSendMsg(pTheMsg);
            return;
          }
        }
        iter_Item++;
      }
      // Wait Cece Coding
      // ...
      // Get The Dynamic Warp Point A_ACTION
      //for(  )
      //{
      //  
      //}
      //
#ifdef _DEBUG_CHECK_ACTION_STATE_
      if( ThisCodeCount )
      {
        g_Action.Write( szMapLog );
      }
#endif
      // Send A_ACTION
      if( iCount > 0 )
      {
        pTheMsg->dwAID        = A_ACTION;
        pTheMsg->Msgs[0].Size = sizeof(SNMAction) * iCount;
        pThePlayer->AddSendMsg( pTheMsg );
      }
      else
      {
        g_pGs->ReleaseMsg( pTheMsg );
        pTheMsg = NULL;
      }
    }
    //
    iter_Player++;
    pThePlayer->UpdateTurnCheck();
  }
  // Set Player Update State
  //for( iter_Player = m_mapCodePlayer.begin(); iter_Player != m_mapCodePlayer.end(); iter_Player++ )
  //{
  //  pThePlayer = (CPlayer*)(iter_Player->second);
  //  pThePlayer->UpdateTurnCheck();
  //	pThePlayer = NULL;
  //}
  // Set Monster Update state
  //pTheMonster = NULL;
  //for(map<DWORD,CMonster*>::iterator iter2 = m_mapCodeMonster.begin(); iter2 != m_mapCodeMonster.end(); iter2++)
  //{
  //	pTheMonster=(CMonster*)(iter2->second);
  //	assert(pTheMonster);
  //  pTheMonster->UpdateTurnCheck();
  //	pTheMonster = NULL;
  //}
  // Set Npc Updata state
  //pTheNpc = NULL;
  //for(map<DWORD,CNpc*>::iterator iter3 = m_mapCodeNpc.begin(); iter3 != m_mapCodeNpc.end(); iter3++)
  //{
  //	pTheNpc=(CNpc*)(iter3->second);
  //	assert(pTheNpc);
  //  pTheNpc->UpdateTurnCheck();
  //	pTheNpc = NULL;
  //}
  
  // Set Magic Update State
  //pTheMagic = NULL;
  //for(map<DWORD,CMagic*>::iterator iter4 = m_mapCodeMagic.begin(); iter4 != m_mapCodeMagic.end(); iter4++)
  //{
  //	pTheMagic=(CMagic*)(iter4->second);
  //	assert(pTheMagic);
  //  pTheMagic->UpdateTurnCheck();
  //	pTheMagic = NULL;
  //}
  // Set GroundItem Update State ( This is unnecessary )
  //pTheItem = NULL;
  //for(map<DWORD,CGroundItem*>::iterator iter5 = m_mapCodeGroundItem.begin(); iter5 != m_mapCodeGroundItem.end(); iter5++)
  //{
  //	pTheItem=(CGroundItem*)(iter5->second);
  //	assert(pTheItem);
  //  pTheItem->UpdateTurnCheck();
  //}
}
//=====================================================================================
//
//Note: Please use it carefully because this function is too slow
void CGameMap::SendTheMsgToAll( SMsgData * pTheMsg )
{
  CPlayer		*pPlayer = NULL;
  int       iNum     = 0;
  
  if( m_mapCodePlayer.empty() )
  {
    g_pGs->ReleaseMsg( pTheMsg );
    return;
  }
  //
  map<DWORD, CPlayer*>::iterator        Iter_Pl;
  //
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pPlayer = Iter_Pl->second;
    if( pPlayer )
    {
      g_pMultiSendPlayer[iNum] = pPlayer;
      //pPlayer->AddSendMsg( pTheMsg );
      iNum++;
    }
  }
  if( iNum == 0 )	      g_pGs->ReleaseMsg( pTheMsg );
  else
  {
    g_pGs->AddRef( pTheMsg, iNum-1 );
    for( int sss = 0; sss < iNum; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pTheMsg );
  }
}
//=====================================================================================
//
// Note: Please use it carefully because this function is too slow
void CGameMap::SendTheMsgToAll( const SMsgData & NewMsg )
{
  SMsgData                          *pNewMsg = NULL;
  CPlayer                           *pPlayer = NULL;
  int                               iNum     = 0;
  map<DWORD, CPlayer*>::iterator    Iter_Ply;
  //
  if( !m_mapCodePlayer.empty() )
  {
    if( NULL == ( pNewMsg = g_pGs->NewMsgBuffer() ) )   return;
    ::CopyMessage( NewMsg, pNewMsg );
    //
    for( Iter_Ply = m_mapCodePlayer.begin(); Iter_Ply != m_mapCodePlayer.end(); Iter_Ply++ )
    {
      g_pMultiSendPlayer[iNum] = (Iter_Ply->second);
      iNum++;
    }
    //
    if( iNum == 0 )       g_pGs->ReleaseMsg( pNewMsg );
    else
    {
      g_pGs->AddRef( pNewMsg, iNum-1 );
      for( int sss = 0; sss < iNum; sss++ )   g_pMultiSendPlayer[sss]->AddSendMsg( pNewMsg );
    }
  }
}
//=====================================================================================
//
//Note: Release Code In Map
void CGameMap::ReleaseMonsterCode(CMonster* pTheMonster)
{
  if( pTheMonster->GetCode() != 0 )
    m_CodeMonster.ReleaseCode(pTheMonster->GetCode());
}
//=====================================================================================
//
//Note: Release Code In Map
void CGameMap::ReleaseNpcCode(CNpc* pTheNpc)
{
  if( pTheNpc->GetCode() != 0 )
    m_CodeNpc.ReleaseCode(pTheNpc->GetCode());	
}
//=====================================================================================
//
//Note: Release Code In Map
void CGameMap::ReleaseGroundItemCode(CGroundItem* pTheItem)
{
  if( pTheItem->GetCode() != 0 )
    m_CodeGroundItem.ReleaseCode(pTheItem->GetCode());	
}
//=====================================================================================
//
//Note: Do Weather Now
void CGameMap::DoWeather( SMsgData *pTheMsg, BOOL bAction )
{
  if( bAction )  
  {
    if( m_dwMapType & MAP_TYPE_RAIN )					// 下雨
    {
      if( m_dwMapType & MAP_TYPE_LEVEL_MORE ) 
      {
        m_iWeatherState = ( MAP_WEATHER_RAIN << 16 ) | ( MAP_WEATHER_LEVEL_MORE & 0x0000FFFF );
      }
      else if( m_dwMapType & MAP_TYPE_LEVEL_MIDDLE )
      {
        m_iWeatherState = ( MAP_WEATHER_RAIN << 16 ) | ( MAP_WEATHER_LEVEL_MIDDLE & 0x0000FFFF );
      }
      else if( m_dwMapType & MAP_TYPE_LEVEL_LITTLE )
      {
        m_iWeatherState = ( MAP_WEATHER_RAIN << 16 ) | ( MAP_WEATHER_LEVEL_LITTLE & 0x0000FFFF );
      }
    }
    else if( m_dwMapType & MAP_TYPE_SNOW )		// 下雪
    {
      if( m_dwMapType & MAP_TYPE_LEVEL_MORE ) 
      {
        m_iWeatherState = ( MAP_WEATHER_SNOW << 16 ) | ( MAP_WEATHER_LEVEL_MORE & 0x0000FFFF );
      }
      else if( m_dwMapType & MAP_TYPE_LEVEL_MIDDLE )
      {
        m_iWeatherState = ( MAP_WEATHER_SNOW << 16 ) | ( MAP_WEATHER_LEVEL_MIDDLE & 0x0000FFFF );
      }
      else if( m_dwMapType & MAP_TYPE_LEVEL_LITTLE )
      {
        m_iWeatherState = ( MAP_WEATHER_SNOW << 16 ) | ( MAP_WEATHER_LEVEL_LITTLE &0x0000FFFF );
      }
      else
        return;
    }
    else if( m_dwMapType & MAP_TYPE_FLOWER )	// 下桃花瓣
    {
      if( m_dwMapType & MAP_TYPE_LEVEL_MORE ) 
      {
        m_iWeatherState = ( MAP_WEATHER_FLOWER << 16 ) | ( MAP_WEATHER_LEVEL_MORE & 0x0000FFFF );
      }
      else if( m_dwMapType & MAP_TYPE_LEVEL_MIDDLE )
      {
        m_iWeatherState = ( MAP_WEATHER_FLOWER << 16 ) | ( MAP_WEATHER_LEVEL_MIDDLE & 0x0000FFFF );
      }
      else if( m_dwMapType & MAP_TYPE_LEVEL_LITTLE )
      {
        m_iWeatherState = ( MAP_WEATHER_FLOWER << 16 ) | ( MAP_WEATHER_LEVEL_LITTLE & 0x0000FFFF );
      }
    }
  }
  else
  {
    m_iWeatherState = 0;
  }
  pTheMsg->Init();
  pTheMsg->dwAID        = A_MAPADDINFO;
  pTheMsg->dwMsgLen     = 1;
  pTheMsg->Msgs[0].Size = sizeof( SNMMapAddInfo );
  //
  GetMapAddInfo( *((SNMMapAddInfo*)(pTheMsg->Msgs[0].Data)) );
  SendTheMsgToAll( *pTheMsg );
  g_pGs->ReleaseMsg( pTheMsg );
}
//=====================================================================================
//
//Note: Check Weather
void CGameMap::CheckMapAddInfo()
{
  // 天气要变
  if( gf_GetRandom( 100 ) < m_dwWeatherRandom )
  {
    // 如果没有在下雨
    if( m_iWeatherState == 0 )  
    {
      if( m_dwMapType & MAP_TYPE_RAIN )					// 下雨
      {
        if( m_dwMapType & MAP_TYPE_LEVEL_MORE ) 
        {
          m_iWeatherState = ( MAP_WEATHER_RAIN << 16 ) | ( MAP_WEATHER_LEVEL_MORE & 0x0000FFFF );
        }
        else if( m_dwMapType & MAP_TYPE_LEVEL_MIDDLE )
        {
          m_iWeatherState = ( MAP_WEATHER_RAIN << 16 ) | ( MAP_WEATHER_LEVEL_MIDDLE & 0x0000FFFF );
        }
        else if( m_dwMapType & MAP_TYPE_LEVEL_LITTLE )
        {
          m_iWeatherState = ( MAP_WEATHER_RAIN << 16 ) | ( MAP_WEATHER_LEVEL_LITTLE & 0x0000FFFF );
        }
      }
      else if( m_dwMapType & MAP_TYPE_SNOW )		// 下雪
      {
        if( m_dwMapType & MAP_TYPE_LEVEL_MORE ) 
        {
          m_iWeatherState = ( MAP_WEATHER_SNOW << 16 ) | ( MAP_WEATHER_LEVEL_MORE & 0x0000FFFF );
        }
        else if( m_dwMapType & MAP_TYPE_LEVEL_MIDDLE )
        {
          m_iWeatherState = ( MAP_WEATHER_SNOW << 16 ) | ( MAP_WEATHER_LEVEL_MIDDLE & 0x0000FFFF );
        }
        else if( m_dwMapType & MAP_TYPE_LEVEL_LITTLE )
        {
          m_iWeatherState = ( MAP_WEATHER_SNOW << 16 ) | ( MAP_WEATHER_LEVEL_LITTLE &0x0000FFFF );
        }
        else
          return;
      }
      else if( m_dwMapType & MAP_TYPE_FLOWER )	// 下桃花瓣
      {
        if( m_dwMapType & MAP_TYPE_LEVEL_MORE ) 
        {
          m_iWeatherState = ( MAP_WEATHER_FLOWER << 16 ) | ( MAP_WEATHER_LEVEL_MORE & 0x0000FFFF );
        }
        else if( m_dwMapType & MAP_TYPE_LEVEL_MIDDLE )
        {
          m_iWeatherState = ( MAP_WEATHER_FLOWER << 16 ) | ( MAP_WEATHER_LEVEL_MIDDLE & 0x0000FFFF );
        }
        else if( m_dwMapType & MAP_TYPE_LEVEL_LITTLE )
        {
          m_iWeatherState = ( MAP_WEATHER_FLOWER << 16 ) | ( MAP_WEATHER_LEVEL_LITTLE & 0x0000FFFF );
        }
      }
    }
  }
  else
  {
    m_iWeatherState = 0;
  }
}
//=====================================================================================
//
//Note: Release Code In Map
void CGameMap::ReleaseMagicCode(CMagic* pTheMagic)
{
  if( pTheMagic->GetCode() != 0 )
    m_CodeMagic.ReleaseCode( pTheMagic->GetCode() );
}
//=====================================================================================
//
void CGameMap::ReleaseWarpCode(SWarpPoint* pTheMagic)
{
  if( pTheMagic->wCode != 0 )   m_CodeWarp.ReleaseCode( pTheMagic->wCode );
}
//------------------------------------------------------------------------------------------
//
//Note: Check the Position weather in client map
BOOL CGameMap::IsInClientMap( DWORD X, DWORD Y )
{
  static iIsInX, iIsInY;
  
  iIsInX = X; iIsInY = Y;
  ConvertSrv2Cli( &iIsInX, &iIsInY );
  if( iIsInX > 0 && iIsInY > 0 )
    return TRUE;
  return FALSE;
}
//=====================================================================================
//
//Note: Check the Position weather in client map
BOOL CGameMap::IsInServerMap( DWORD X, DWORD Y )
{
  if( X < 0 || X > m_iSizeX || Y < 0 || Y > m_iSizeY )
  {
    return FALSE;
  }
  return TRUE;
}
//=====================================================================================
//
//
void CGameMap::NpcDisappear( const WORD & wNpcCode )
{
  CNpc							*pTheNpc = GetNpc( wNpcCode );
  //
  if( pTheNpc )
  {
    // Erase From Map
    m_mapCodeNpc.erase( pTheNpc->GetCode() );
    // Insert The Npc To Disappear List
    m_listDisappearNpc.push_back( pTheNpc );
    // Clear His Code
    *(DWORD*)(MsgClearCode.Msgs[0].Data) = ( PLAYER_WARP_TYPE_LOGOUT << 16 ) | pTheNpc->GetCode();
    SendMsgNearPosition( MsgClearCode, pTheNpc->GetPosX(), pTheNpc->GetPosY() );
  }
}
//=====================================================================================
//
//
void CGameMap::NpcAppear( const WORD & wNpcCode )
{
  CNpc							        *pTheNpc = NULL;
  list<CNpc*>::iterator     Iter_Np;
  //
  for( Iter_Np = m_listDisappearNpc.begin(); Iter_Np != m_listDisappearNpc.end(); Iter_Np++ )
  {
    if( (*Iter_Np)->GetCode() == wNpcCode )
    {
      pTheNpc = (*Iter_Np);
      m_listDisappearNpc.erase( Iter_Np );
      break;
    }
  }
  //
  if( pTheNpc )
  {
    map<DWORD,CNpc*>::iterator    Iter_Npc = m_mapCodeNpc.find( wNpcCode );
    if( Iter_Npc == m_mapCodeNpc.end() )
    {
      // Insert The Npc Into Map, Player Query His Info
      m_mapCodeNpc.insert( map<DWORD,CNpc*>::value_type( pTheNpc->GetCode(), pTheNpc ) );
    }
  }
}
//=====================================================================================
//
//Note:
void CGameMap::NpcDisappearWhenNight()
{
  CNpc							*pTheNpc = NULL;
  
  for( map<DWORD,CNpc*>::iterator iter = m_mapCodeNpc.begin(); iter != m_mapCodeNpc.end(); )
  {
    if( NULL != ( pTheNpc = (CNpc*)(iter->second) ) )
    {
      if( pTheNpc->GetNpcActionType() == EVENT_POINTACTION_DAY )
      {
        // Insert the npc to disappear list and clear it's code
        *(DWORD*)(MsgClearCode.Msgs[0].Data) = ( PLAYER_WARP_TYPE_LOGOUT << 16 ) | pTheNpc->GetCode();
        SendMsgNearPosition( MsgClearCode, pTheNpc->GetPosX(), pTheNpc->GetPosY() );
        
        m_listDisappearNpc.push_back(pTheNpc);
        // Delete the npc when night
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        g_CrashLog.Write( "CGameMap::NpcDisappearWhenNight ===>> 111" );
#endif
        iter = m_mapCodeNpc.erase(iter);
//        iter = m_mapCodeNpc.begin();
//        if( iter == m_mapCodeNpc.end() )    break;
//        pTheNpc = NULL;
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }
}
//=====================================================================================
//
//Note:
void CGameMap::NpcAppearWhenNight()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::NpcAppearWhenNight()");
#endif
  
  CNpc			*pTheNpc = NULL;
  
  for( list<CNpc*>::iterator iter = m_listDisappearNpc.begin(); iter != m_listDisappearNpc.end(); )
  {
    if( NULL != ( pTheNpc = (CNpc*)(*iter) ) )
    {
      if( pTheNpc->GetNpcActionType() == EVENT_POINTACTION_NIGHT )
      {
        // Insert the npc when night
        m_mapCodeNpc.insert( map<DWORD,CNpc*>::value_type(pTheNpc->GetCode(), pTheNpc) );
        // Delete the npc when night
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        g_CrashLog.Write( "CGameMap::NpcAppearWhenNight ===>> 111" );
#endif
        iter = m_listDisappearNpc.erase( iter );
//        iter = m_listDisappearNpc.begin();
//        if( iter == m_listDisappearNpc.end() )    break;
//        pTheNpc = NULL;
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }
}
//=====================================================================================
//
//Note:
void CGameMap::NpcDisappearWhenDay()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::NpcDisappearWhenDay()");
#endif
  
  CNpc							*pTheNpc = NULL;
  
  for( map<DWORD,CNpc*>::iterator iter = m_mapCodeNpc.begin(); iter != m_mapCodeNpc.end(); )
  {
    if( NULL != ( pTheNpc = (CNpc*)(iter->second) ) )
    {
      if( pTheNpc->GetNpcActionType() == EVENT_POINTACTION_NIGHT )
      {
        // Insert the npc to disappear list and clear it's code
        *(DWORD*)(MsgClearCode.Msgs[0].Data) = ( PLAYER_WARP_TYPE_LOGOUT << 16 ) | pTheNpc->GetCode();
        SendMsgNearPosition( MsgClearCode, pTheNpc->GetPosX(), pTheNpc->GetPosY() );
        
        m_listDisappearNpc.push_back(pTheNpc);
        // Delete the npc when day
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        g_CrashLog.Write( "CGameMap::NpcDisappearWhenDay ===>> 111" );
#endif
        iter = m_mapCodeNpc.erase(iter);
//        iter = m_mapCodeNpc.begin();
//        if(iter == m_mapCodeNpc.end() )
//          break;
//        pTheNpc = NULL;
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }
}
//=====================================================================================
//
//Note:	
void CGameMap::NpcAppearWhenDay()
{
  CNpc			*pTheNpc = NULL;
  
  for( list<CNpc*>::iterator iter = m_listDisappearNpc.begin(); iter != m_listDisappearNpc.end(); )
  {
    if( NULL != ( pTheNpc = (CNpc*)(*iter) ) )
    {
      if( pTheNpc->GetNpcActionType() == EVENT_POINTACTION_DAY )
      {
        // Insert the npc when day
        m_mapCodeNpc.insert( map<DWORD,CNpc*>::value_type(pTheNpc->GetCode(), pTheNpc) );
        // Delete the npc when day
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
        g_CrashLog.Write( "CGameMap::NpcAppearWhenDay ===>> 111" );
#endif
        iter = m_listDisappearNpc.erase(iter);
//        iter = m_listDisappearNpc.begin();
//        if(iter == m_listDisappearNpc.end() )
//          break;
//        pTheNpc = NULL;
      }
      else
      {
        iter++;
      }
    }
    else
    {
      iter++;
    }
  }	
}
//=====================================================================================
//
// Note:
BOOL CGameMap::AddWarpPoint( SWarpPoint * pTheWarp, const DWORD & dwX_Y )
{
  m_Iter_Wp = m_mapCodeWarpPoint.find( pTheWarp->wCode );
  if( m_Iter_Wp != m_mapCodeWarpPoint.end() )   return FALSE;
  //
  DWORD         dwTempXY = 0;
  // Check The Warp Point In 9 Tiles
  for( int iLoop = 0; iLoop < 9; iLoop++ )
  {
    dwTempXY = MAKELONG( LOWORD( dwX_Y ) + DirOffsetY[iLoop],
      HIWORD( dwX_Y ) + DirOffsetX[iLoop] );
    if( m_pBase->GetWarpPointByPos( dwTempXY ) )    return FALSE;
  }
  //
  SWarpPoint      *pCopyWarp = pTheWarp;
  // Add Warp Point Into 9 Tiles
  for( iLoop = 0; iLoop < 9; iLoop++ )
  {
    if( iLoop )
    {
      dwTempXY = MAKELONG( LOWORD( dwX_Y ) + DirOffsetY[iLoop],
        HIWORD( dwX_Y ) + DirOffsetX[iLoop] );
      if( pTheWarp = new SWarpPoint( GetNewWarpCode() ) )
      {
        pTheWarp->wMapId       = GetMapId();
        pTheWarp->wPosX        = HIWORD( dwX_Y ) + DirOffsetX[iLoop];
        pTheWarp->wPosY        = LOWORD( dwX_Y ) + DirOffsetY[iLoop];
        pTheWarp->wTargetMapId = pCopyWarp->wTargetMapId;
        pTheWarp->wTargetMapX  = pCopyWarp->wTargetMapX;
        pTheWarp->wTargetMapY  = pCopyWarp->wTargetMapY;
        pTheWarp->dwLifeTime   = pCopyWarp->dwLifeTime;
        pTheWarp->wState       = WARPPOINT_STATE_ON;
        pTheWarp->wType        = WARPPOINT_TYPE_EVIL_ADDI;
      }
    }
    else    dwTempXY = dwX_Y;
    //
    if( pTheWarp )
    {
      m_mapCodeWarpPoint.insert( map<DWORD,SWarpPoint*>::value_type( pTheWarp->wCode, pTheWarp ) );
      m_pBase->AddWarpPoint( pTheWarp, dwTempXY );
    }
  }
  return TRUE;
}
//=====================================================================================
//
//Note:
//BOOL CGameMap::DeleteWarpPoint( const DWORD & dwX_Y )
//{
//  SWarpPoint        *pPoint = m_pBase->GetWarpPointByPos( dwX_Y );
//  //
//  if( pPoint && ( m_Iter_Wp = m_mapCodeWarpPoint.find( pPoint->wCode ) ) != m_mapCodeWarpPoint.end() )
//  {
//    m_mapCodeWarpPoint.erase( m_Iter_Wp );
//    m_pBase->DeleteWarpPoint( dwX_Y );
//    return TRUE;
//  }
//	return FALSE;
//}
static char			g_szAllShowInfo[1000000];
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllPlayerTileInfo()
{
  FuncName("CGameMap::ShowAllPlayerTileInfo");
  
  LPLifeTileList	pLifeList = NULL;
  CLife						*pLife = NULL;
  CPlayer					*pPlayer = NULL;
  int							iCount = 0, iTotal = 0;
  
  iTotal += _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d] Player Tile Info -- ", GetMapId() );
  g_szAllShowInfo[1000000-1] = '\0';
  
  for( int i = 0; i < (int)m_iSizeY; i++ )
  {
    for( int j = 0; j < (int)m_iSizeX; j++ )
    {
      pLifeList = &m_pMapTilePlayer[i*m_iSizeX+j];
      if( !pLifeList->empty() )
      {
        for( LifeTileList::iterator iter = pLifeList->begin(); iter != pLifeList->end(); iter++ )
        {
          if( NULL != ( pLife = (*iter) ) )
          {
            if( pLife->IsPlayer() && iTotal < 20000 )
            {
              pPlayer = (CPlayer*)(pLife);
              iTotal += _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "%s[%d](%d,%d), ", pPlayer->GetPlayerName(), pPlayer->GetCode(), j, i );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              if( iTotal >= 1000000 )  return g_szAllShowInfo;
              strcat( g_szAllShowInfo, szMapLog );
              iCount++;
            }
          }
          else
          {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
            _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Find Empty Pointer, Map(%d) Postion(%d,%d) ! *****", GetMapId(), j, i );
            szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
            AddMemoErrMsg( szMapLog );
#endif
          }
        }
      }
    }
  }
  if( iCount > 0 )
  {
    _snprintf( g_szAllShowInfo + iTotal, 1000000-1-iTotal, "Player Count = %d #", iCount );
    g_szAllShowInfo[1000000-1] = '\0';
    return g_szAllShowInfo;
  }
  return NULL;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllPlayerInfo()
{
  FuncName("CGameMap::ShowAllPlayerInfo");
  
  CPlayer					                  *pPlayer = NULL;
  int							                  iCount = 0, iTotal = 0;
  map<DWORD, CPlayer*>::iterator    iter;
  
  iTotal += _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d] Players Info -- MailId Count=%d, Name Count=%d", GetMapId(), g_pGs->GetMailIdMapSize(), g_pGs->GetNameMapSize() );
  g_szAllShowInfo[1000000-1] = '\0';
  //
  for( iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
  {
    if( iter->second && iTotal < 20000 )
    {
      pPlayer = (CPlayer*)(iter->second);
      if( iTotal >= 1000000 )  return g_szAllShowInfo;
      iTotal += _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "%s[%d](%d,%d), ", pPlayer->GetPlayerName(), pPlayer->GetCode(), pPlayer->GetPosX(), pPlayer->GetPosY() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      strcat( g_szAllShowInfo, szMapLog );
      iCount++;
    }
    else
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Find Empty Pointer Or Count OverFlow, In Map(%d) When Get Player Info! *****", GetMapId() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( szMapLog );
#endif
    }
  }
  //
  if( iCount > 0 )
  {
    _snprintf( g_szAllShowInfo + iTotal, 1000000-1-iTotal, "Player Count = %d #", iCount );   
    g_szAllShowInfo[1000000-1] = '\0';
    return g_szAllShowInfo;
  }
  return g_szAllShowInfo;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllMstTileInfo()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::ShowAllGroundItemInfo()");
#endif
  
  LifeTileList::iterator	            Iter_M;
  int							                    iTotal    = 0, XXX = 0, YYY = 0, iMonsterCount = 0;
  LPLifeTileList                      pLifeList = NULL;
  CLife                               *pTarget  = NULL;
  CMonster                            *pMonster = NULL;
  char			                          g_szMstTemp[1000000];
  
#ifdef _REPAIR_SERVER_CRASH_NICK_
  memset( g_szMstTemp, 0, 1000000 );
#else 
  strcpy( g_szMstTemp, "" );
#endif
  //iTotal += sprintf( g_szAllShowInfo, "The Map[%d] Monster Tile Info -- Monster Count=%d; ", GetMapId(), m_mapCodeGroundItem.size() );
  for( XXX = 0; XXX < m_iSizeX; XXX++ )
  {
    for( YYY = 0; YYY < m_iSizeY; YYY++ )
    {
      if( pLifeList = GetTileLifeList( XXX, YYY ) )
      {
        if( !pLifeList->empty() )
        {
          for( Iter_M = pLifeList->begin(); Iter_M != pLifeList->end(); Iter_M++ )
          {
            pTarget = (*Iter_M);
            if( pTarget->IsMonster() )
            {
              pMonster = (CMonster*)(pTarget);
              if( iTotal >= 999000 )
              {
                _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d] Monster Tile Info -- Monster Count=(%d,%d); %s", GetMapId(), m_mapCodeMonster.size(), iMonsterCount, g_szMstTemp );
                g_szAllShowInfo[1000000-1] = '\0';
                return g_szAllShowInfo;
              }
              iTotal += _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "%s[%d](%d,%d), ", pMonster->GetName(), pMonster->GetCode(), pMonster->GetPosX(), pMonster->GetPosY() );
              szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
              strcat( g_szMstTemp, szMapLog );
              iMonsterCount++;
            }
          }
        }
      }
    }
  }
  iTotal += _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d] Monster Tile Info -- Monster Count=(%d,%d); %s", GetMapId(), m_mapCodeMonster.size(), iMonsterCount, g_szMstTemp );
  g_szAllShowInfo[1000000-1] = '\0';
  return g_szAllShowInfo;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllGroundItemInfo()
{
  map<DWORD, CGroundItem*>::iterator	  iter;
  int							                      iTotal = 0;
  CGroundItem                           *pItem;
  
  iTotal += _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d] Ground Item Info -- Ground Item Count=%d", GetMapId(), m_mapCodeGroundItem.size() );
  g_szAllShowInfo[1000000-1] = '\0';
  for( iter = m_mapCodeGroundItem.begin(); iter != m_mapCodeGroundItem.end(); iter++ )
  {
    pItem = (iter->second);
    if( iTotal >= 1000000 )  return g_szAllShowInfo;
    iTotal += _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "%s[%d](%d,%d), ", pItem->GetName(), pItem->GetCode(), pItem->GetPosX(), pItem->GetPosY() );
    szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
    strcat( g_szAllShowInfo, szMapLog );
  }
  return g_szAllShowInfo;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllFlagInfo( HWND hText )
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::ShowAllFlagInfo()");
#endif
  
  int							x = 0, y = 0, X, Y, iCount = 0, iNum = 0;
  
  /*for( y = 0; y < m_pBase->GetCltSizeY(); y++ )
  {
		g_szAllShowInfo[y*m_iSizeX] = '\n';
    for( x = 0; x < m_pBase->GetCltSizeX(); x++ )
    {
    X = x;	Y = y;
    ConvertCli2Srv( &X, &Y );
    if( !( m_pMapOrg[Y*m_iSizeX+X] & 0xC1000000 ) )	g_szAllShowInfo[y*m_iSizeX+x] = 'O';
    else if( m_pMapOrg[Y*m_iSizeX+X] & 0xC0000000 )	g_szAllShowInfo[y*m_iSizeX+x] = 'X';
    else if( m_pMapOrg[Y*m_iSizeX+X] & 0x01000000 )	g_szAllShowInfo[y*m_iSizeX+x] = '#';
    else																						g_szAllShowInfo[y*m_iSizeX+x] = '*';
    }
    }
  g_szAllShowInfo[y*m_iSizeX] = '\0';*/
  for( y = 0; y < m_iSizeY; y++ )
  {
    for( x = 0; x < m_iSizeX; x++ )
    {
      X = x;	Y = y;
      //ConvertCli2Srv( &X, &Y );
      if( m_pMapOrg[Y*m_iSizeX+X] & 0x01000000 )	iNum++;
    }
  }
  _snprintf( g_szAllShowInfo, 1000000-1, "Map Life Flag Count=%d\n", iNum );
  g_szAllShowInfo[1000000-1] = '\0';
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "Monster Count=%d", m_dwMonsterCount );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
  strcat( g_szAllShowInfo, szMapLog );
  return g_szAllShowInfo;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllMonsterInfo()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::ShowAllMonsterInfo()");
#endif
  
  FuncName("CGameMap::ShowAllMonsterInfo");
  
  CMonster				*pMonster = NULL;
  int							iCount = 0, iTotal = 0;
  
  iTotal += _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d][%s] Monsters(%d) Info -- ", GetMapId(), GetName(), m_mapCodeMonster.size() );
  g_szAllShowInfo[1000000-1] = '\0';
  
  for( map<DWORD, CMonster*>::iterator iter = m_mapCodeMonster.begin(); iter != m_mapCodeMonster.end(); iter++ )
  {
    if( iter->second && iTotal < 20000 )
    {
      pMonster = (CMonster*)(iter->second);
      if( iTotal >= 1000000 )  return g_szAllShowInfo;
      iTotal += _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "%s[%d](%d,%d), ", pMonster->GetName(), pMonster->GetCode(), pMonster->GetPosX(), pMonster->GetPosY() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      strcat( g_szAllShowInfo, szMapLog );
      iCount++;
    }
    else
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Find Empty Pointer Or Count OverFlow, In Map(%d) When Get Player Info! *****", GetMapId() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( szMapLog );
#endif
    }
  }
  
  if( iCount > 0 )
  {
    _snprintf( g_szAllShowInfo + iTotal, 1000000-1-iTotal, "Monster Count = %d #", iCount );   
    g_szAllShowInfo[1000000-1] = '\0';
    return g_szAllShowInfo;
  }
  return NULL;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllNpcInfo()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::ShowAllNpcInfo()");
#endif
  
  FuncName("CGameMap::ShowAllNpcInfo");
  
  CNpc						*pNpc = NULL;
  int							iCount = 0, iTotal = 0;
  
  iTotal += _snprintf( g_szAllShowInfo, 1000000-1, "The Map[%d] Npcs Info -- ", GetMapId() );
  g_szAllShowInfo[1000000-1] = '\0';
  
  for( map<DWORD, CNpc*>::iterator iter = m_mapCodeNpc.begin(); iter != m_mapCodeNpc.end(); iter++ )
  {
    if( iter->second && iTotal < 20000 )
    {
      pNpc = (CNpc*)(iter->second);
      if( iTotal >= 1000000 )  return g_szAllShowInfo;
      iTotal += _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "%s[%d](%d,%d), ", pNpc->GetNpcName(), pNpc->GetCode(), pNpc->GetPosX(), pNpc->GetPosY() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      strcat( g_szAllShowInfo, szMapLog );
      iCount++;
    }
    else
    {
#ifndef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
      _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "***** Find Empty Pointer Or Count OverFlow, In Map(%d) When Get Player Info! *****", GetMapId() );
      szMapLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoErrMsg( szMapLog );
#endif
    }
  }
  //
  if( iCount > 0 )
  {
    _snprintf( g_szAllShowInfo + iTotal, 1000000-1-iTotal, "Npc Count = %d #", iCount );   
    g_szAllShowInfo[1000000-1] = '\0';
    return g_szAllShowInfo;
  }
  return NULL;
}
//=====================================================================================
//
//Note:
char * CGameMap::ShowAllMapAddInfo()
{
  //	FuncName("CGameMap::ShowAllMapAddInfo");
  
  _snprintf( szMapLog, MAX_MEMO_MSG_LEN-1, "This Map Add Info : \n          " );
  szMapLog[MAX_MEMO_MSG_LEN-1] = '\0'; 
  if( m_dwMapType & MAP_TYPE_NIGHT || m_dwMapType & MAP_TYPE_ESTOP_NIGHT )
  {
    strcat( szMapLog, "Map Is All Night !\n          " );
  }
  else if( m_dwMapType & MAP_TYPE_DAY )	
  {
    strcat( szMapLog, "Map Is All Day !\n          " );
  }
  else
  {
    strcat( szMapLog, "Map Is Day And Night !\n          " );
  }
  
  if( m_dwMapType & MAP_TYPE_NO_ATTACK )
  {
    strcat( szMapLog, "Can Not Attack In The Map !\n          " );
  }
  else
  {
    strcat( szMapLog, "Can Attack In The Map !\n          " );
  }
  if( m_dwMapType & MAP_TYPE_NO_PK )
  {
    strcat( szMapLog, "Can Not PK In The Map !\n          " );
  }
  else
  {
    strcat( szMapLog, "Can PK In The Map !\n          " );
  }
  
  if( m_dwMapType & MAP_TYPE_RAIN )
  {
    strcat( szMapLog, "It Will Rain In The Map !\n          " );
  }
  else if( m_dwMapType & MAP_TYPE_SNOW )
  {
    strcat( szMapLog, "It Will Snow In The Map !\n          " );
  }
  else if( m_dwMapType & MAP_TYPE_FLOWER )
  {
    strcat( szMapLog, "It Will Fall Flower In The Map !\n          " );
  }
  else
  {
    strcat( szMapLog, "It Have No Other Weather In The Map !\n          " );
  }
  
  if( m_bNewPathFind )
  {
    strcat( szMapLog, "Path Find Is New Version !\n          " );
  }
  else
  {
    strcat( szMapLog, "Path Find Is Old Version !\n          " );
  }
  
  if( m_dwRunType == MAP_RUNTYPE_NORMAL )
  {
    strcat( szMapLog, "The Map Run Type Is Normal !\n          " );
  }
  else if( m_dwRunType == MAP_RUNTYPE_MOD2 )
  {
    strcat( szMapLog, "The Map Run Type Is Mod2 !\n          " );
  }
  else if( m_dwRunType == MAP_RUNTYPE_MOD3 )
  {
    strcat( szMapLog, "The Map Run Type Is Mod3 !\n          " );
  }
  else if( m_dwRunType == MAP_RUNTYPE_MOD4 )
  {
    strcat( szMapLog, "The Map Run Type Is Mod4 !\n          " );
  }
  
  return szMapLog;
}
//=====================================================================================
//
//Note:
void CGameMap::DisableAllMonster(const WORD & wId, BOOL bGMCreate)
{
  CMonster				*pTheMonster;
  SMsgData				*pNewMsg;
  
  pNewMsg = g_pGs->NewMsgBuffer();
  if( pNewMsg == NULL )           return;
  //
  for( map<DWORD, CMonster*>::iterator  Iter_Mt = m_mapCodeMonster.begin();
  Iter_Mt != m_mapCodeMonster.end(); )
  {
    if( NULL != ( pTheMonster = Iter_Mt->second ) )
    {
      if( ( wId == 0 || pTheMonster->GetBaseId() == wId ) &&
        ( ( bGMCreate == FALSE ) || ( bGMCreate && pTheMonster->GetReviveType() == REVIVE_TYPE_GM_CREATE ) ) )
      {
        Iter_Mt = m_mapCodeMonster.erase( Iter_Mt );
        m_mapDefenceMonster.erase( pTheMonster->GetCode() );
        DelLifeFromTile( (CLife*)pTheMonster );
        m_mapDisableMonster.insert( map<DWORD, CMonster*>::value_type( pTheMonster->GetCode(), pTheMonster ) );
      }
      else
      {
        Iter_Mt++;
      }
    }
    else
    {
      m_mapDefenceMonster.erase( Iter_Mt->first );
      Iter_Mt = m_mapCodeMonster.erase( Iter_Mt );
    }
  }
  if( ( !m_mapDisableMonster.empty() ) || ( !m_mapCodePlayer.empty() ) )
  {
    pNewMsg->Init();
    pNewMsg->dwAID		    = A_CLEARALLMONSTER;
    pNewMsg->dwMsgLen     = 1;
    pNewMsg->Msgs[0].Size = sizeof( WORD );
    *(WORD*)(pNewMsg->Msgs[0].Data) = wId;
    SendTheMsgToAll( pNewMsg );
  }
  else
  {
    g_pGs->ReleaseMsg( pNewMsg );
  }
}
//=====================================================================================
//
//Note:
void CGameMap::ReInitAllMonster()
{
  CMonster		                      *pTheMonster;
  map<DWORD, CMonster*>::iterator   iter;
  
  for( iter = m_mapDisableMonster.begin(); iter != m_mapDisableMonster.end(); )
  {
    if( NULL != ( pTheMonster = iter->second ) )
    {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::ReInitAllMonster ===>> 111" );
#endif
      iter = m_mapDisableMonster.erase( iter );
      m_mapCodeMonster.insert( map<DWORD, CMonster*>::value_type( pTheMonster->GetCode(), pTheMonster ) );
      // Add Defence Monster
      if( pTheMonster->GetType() & NPC_ATTRI_PROTECT_PLAYER ||
        pTheMonster->GetType() & NPC_ATTRI_ATTACK_CITY )
      {
        if( m_mapDefenceMonster.find( pTheMonster->GetSelfCode() ) ==
          m_mapDefenceMonster.end() )
        {
          m_mapDefenceMonster.insert( map<DWORD,CMonster*>::value_type( pTheMonster->GetSelfCode(),
            pTheMonster ) );
        }
      }
      //
      pTheMonster->ReinitAll();
      continue;
    }
    iter++;
  }
}
//=====================================================================================
//
//Note:
void CGameMap::DisableMonsterById(int iId)
{
  CMonster				*pTheMonster;
  SMsgData				*pNewMsg;
  int							iCount = 0;
  DWORD						*pCode;
  
  pNewMsg = g_pGs->NewMsgBuffer();
  if( pNewMsg == NULL )     return;
  //
  pCode = (DWORD*)pNewMsg->Msgs[0].Data;
  
  for( map<DWORD, CMonster*>::iterator iter = m_mapCodeMonster.begin(); iter != m_mapCodeMonster.end(); )
  {
    if( NULL != ( pTheMonster = iter->second ) && pTheMonster->GetBaseId() == iId )
    {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::DisableMonsterById ===>> 111" );
#endif
      //
      m_mapDefenceMonster.erase( pTheMonster->GetCode() );
      //
      iter = m_mapCodeMonster.erase( iter );
      //
      m_mapDisableMonster.insert( map<DWORD, CMonster*>::value_type( pTheMonster->GetCode(), pTheMonster ) );
      DelLifeFromTile( (CLife*)pTheMonster );
      if( iCount >= MAXMSGDATASIZE / sizeof( WORD ) ) break;
      *( pCode + iCount ) = pTheMonster->GetCode();
      iCount++;
    }
    else
    {
      iter++;
    }
  }
  if( iCount && ( !m_mapCodePlayer.empty() ) )
  {
    pNewMsg->Init();
    pNewMsg->dwAID				= A_CLEARCODE;
    pNewMsg->dwMsgLen			= 1;
    pNewMsg->Msgs[0].Size = sizeof( DWORD ) * iCount;
    SendTheMsgToAll( pNewMsg );
  }
  else
  {
    g_pGs->ReleaseMsg( pNewMsg );
  }
}
//=====================================================================================
//
//Note:
void CGameMap::ReInitMonsterById(int iId)
{
  CMonster		                      *pTheMonster;
  map<DWORD, CMonster*>::iterator   iter;
  
  for( iter = m_mapDisableMonster.begin(); iter != m_mapDisableMonster.end(); )
  {
    if( NULL != ( pTheMonster = iter->second ) && pTheMonster->GetBaseId() == iId )
    {
#ifdef _DEBUG_WILDCAT_CRASH_INFO_
      g_CrashLog.Write( "CGameMap::ReInitMonsterById ===>> 111" );
#endif
      iter = m_mapDisableMonster.erase( iter );
      m_mapCodeMonster.insert( map<DWORD, CMonster*>::value_type( pTheMonster->GetCode(), pTheMonster ) );
      // Add Defence Monster
      if( pTheMonster->GetType() & NPC_ATTRI_PROTECT_PLAYER ||
        pTheMonster->GetType() & NPC_ATTRI_ATTACK_CITY )
      {
        if( m_mapDefenceMonster.find( pTheMonster->GetSelfCode() ) ==
          m_mapDefenceMonster.end() )
        {
          m_mapDefenceMonster.insert( map<DWORD,CMonster*>::value_type( pTheMonster->GetSelfCode(),
            pTheMonster ) );
        }
      }
      pTheMonster->ReinitAll();
      continue;
    }
    iter++;
  }
}
//=====================================================================================
//
//Note:
void CGameMap::DisableAllNpc()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::DisableAllNpc()");
#endif
  
  CNpc			*pTheNpc = NULL;
  
  for( map<DWORD, CNpc*>::iterator iter = m_mapCodeNpc.begin(); iter != m_mapCodeNpc.end(); iter++ )
  {
    if( NULL != ( pTheNpc = ( iter->second ) ) )
    {
      pTheNpc->SetState( STATUS_NPC_DISABLE );
    }
  }
}
//=====================================================================================
//
//Note:
void CGameMap::ReInitAllNpc()
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::ReInitAllNpc()");
#endif
  
  CNpc			*pTheNpc = NULL;
  
  for( map<DWORD, CNpc*>::iterator iter = m_mapCodeNpc.begin(); iter != m_mapCodeNpc.end(); iter++ )
  {
    if( NULL != ( pTheNpc = ( iter->second ) ) )
    {
      pTheNpc->SetState( STATUS_NPC_STAND );
    }
  }
}
//=====================================================================================
//
//Note:
void CGameMap::DisableNpcById(int iId)
{
  CNpc			*pTheNpc;
  
  map<DWORD, CNpc*>::iterator iter = m_mapCodeNpc.find( iId );
  if( iter != m_mapCodeNpc.end() )
  {
    if( NULL != ( pTheNpc = ( iter->second ) ) )
    {
      pTheNpc->SetState( STATUS_NPC_DISABLE );
    }
  }
}
//=====================================================================================
//
//Note:
void CGameMap::ReInitNpcById(int iId)
{
  CNpc			*pTheNpc;
  
  map<DWORD, CNpc*>::iterator iter = m_mapCodeNpc.find( iId );
  if( iter != m_mapCodeNpc.end() )
  {
    if( NULL != ( pTheNpc = ( iter->second ) ) )
    {
      pTheNpc->SetState( STATUS_NPC_STAND );
    }
  }
}
//=====================================================================================
//
//Note:
int CGameMap::BeParty( const SNMPartyData & PartyData, BOOL bGroundParty )
{
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CGameMap::BeParty()");
#endif
  
  CPlayer               *pPlayer;
  int                   iTotal = 0;
  
  // Start Party
  for( map<DWORD, CPlayer*>::iterator iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
  {
    if( NULL != ( pPlayer = (CPlayer*)(iter->second) ) && pPlayer->GetLevel() > (int)PartyData.dwLevelLimit )
    {
      iTotal += pPlayer->BeParty( PartyData, bGroundParty );
    }
  }
  return iTotal;
}
//=====================================================================================
//
//
void CGameMap::kickAllPlayer( SMsgData * pTheMsg )
{
  map<DWORD, CPlayer*>::iterator	iter;
  //
  for( iter = m_mapCodePlayer.begin(); iter != m_mapCodePlayer.end(); iter++ )
  {
    (iter->second)->AddSendErrorMsg( ERROR_CODE_GM_KICK );
    (iter->second)->SetClientState( CLIENTSTATE_LOGOUT );
  }
}

#ifdef _AUTO_ADD_WARP_POINT_
//=====================================================================================
// Function:CheckWarpPiontTime()
// 具体执行时间(分)+随机数 <= 60
// 精度为5分钟,如果当天不是产生warp-point 则检查精度为两个小时
//=====================================================================================
BOOL CGameMap::CheckWarpPiontTime()
{

  time_t     ti;
  struct  tm *tm;
  int        iCount = 1;
  int        iTick  = 0;
  map<int, _AddWarpPoint*>::iterator iter;
  struct _AddWarpPoint *pWarpPiont = NULL;
  static DWORD dwCheckFrequent = TimeGetTime() + 30000;
  
  if(ClientTickCount > dwCheckFrequent)
  {
    time(&ti);
    tm = localtime(&ti);
    for(iter = g_mapAddWarpPiont.begin(); iter!=g_mapAddWarpPiont.end(); iter++)
    {
      pWarpPiont = iter->second;
      // Run It On The Set-Day Every week;
      iTick = (iTick > iter->first) ? iTick : iter->first;
      if(pWarpPiont->iRunTime[0] != tm->tm_wday)   
      { 
        iCount++;
        continue; 
      } 
      //
      int iTemMinute = pWarpPiont->iRunTime[2] + gf_GetRandom(pWarpPiont->iRunTime[3]);
      int iTemHour   = pWarpPiont->iRunTime[1];
      if(iTemMinute > 60)
      {
        iTemMinute = 59; //
      }
      if((iTemHour == tm->tm_hour) && (tm->tm_min >= iTemMinute))
      {
        DWORD dwX_Y = MAKELONG(pWarpPiont->wPointParameter[2], pWarpPiont->wPointParameter[1]);
        if(!m_pBase->HaveEvilWarpPoint(dwX_Y)) 
        {
          SetTempWarpPiont(pWarpPiont);
        }
      }  
    }
    // Set Check Frequency
    if(iCount == iTick)
    {
      dwCheckFrequent = ClientTickCount + 7200000; // 2 * 60 * 60 * 10000 ms
    }
    else
    {
      dwCheckFrequent = ClientTickCount + 30000;   // 5 * 60 * 1000 ms
    }
  }
  return TRUE;
}

// Parameter Format: 1111 12 49 1 4309 2 17 60  | Source Map ID X Y: 1111 12 49 
//                                              | Warp Point Type: 1
//                                              | Des Map ID X Y: 4309 2 17
//                                              | Spacing Time: 60

BOOL CGameMap::SetTempWarpPiont(const _AddWarpPoint *pWarpPiontPram)
{
  WORD          wSourMapId = pWarpPiontPram->wPointParameter[0];
  int           iSourMapX  = pWarpPiontPram->wPointParameter[1];
  int           iSourMapY  = pWarpPiontPram->wPointParameter[2]; 
  WORD					wDesMapId  = pWarpPiontPram->wPointParameter[4];
  int					  iDesMapX   = pWarpPiontPram->wPointParameter[5];
  int					  iDesMapY	 = pWarpPiontPram->wPointParameter[6];
  DWORD         dwLifeTime = pWarpPiontPram->wPointParameter[7];
  SWarpPoint    *pNewWarp  = NULL;
  CGameMap      *pTheCallMap = NULL;
  
  // Get Des Map Id, X, Y
  // Maybe Des-Warp-Point It Is Not On This Game Server
  if( NULL != (pTheCallMap = g_pGs->GetGameMap(wDesMapId)) )
  {
    pTheCallMap->ConvertCli2Srv(&iDesMapX, &iDesMapY);
    if(iDesMapX >= pTheCallMap->GetSizeX() || iDesMapY >= pTheCallMap->GetSizeY() ||
      iDesMapX < 1 || iDesMapY < 1)
    {
      return FALSE;
    }
  } 
  // Get Source Map Id, X, Y
  // Source-Warp-Point Must Be On This Game Server
  if( NULL == (pTheCallMap = g_pGs->GetGameMap(wSourMapId)) )
  {
    return FALSE;
  }
  pTheCallMap->ConvertCli2Srv(&iSourMapX, &iSourMapY);
  if(iSourMapX >= pTheCallMap->GetSizeX() || iSourMapY >= pTheCallMap->GetSizeY() ||
    iSourMapX < 1 || iSourMapY < 1)
  {
    return FALSE;
  }
  if( NULL == (pNewWarp = new SWarpPoint(pTheCallMap->GetNewWarpCode())) )
  {
    return FALSE;
  }
  
  pNewWarp->wTargetMapId = wDesMapId;
  pNewWarp->wTargetMapX  = iDesMapX;
  pNewWarp->wTargetMapY  = iDesMapY;
  pNewWarp->wType        = WARPPOINT_TYPE_EVIL;
  pNewWarp->wState       = WARPPOINT_STATE_ON;
  pNewWarp->dwLifeTime   = dwLifeTime * 60 * 1000 + ClientTickCount;
  pNewWarp->wMapId       = wSourMapId;
  pNewWarp->wPosX        = iSourMapX;
  pNewWarp->wPosY        = iSourMapY;
  // Function AddWarpPoint While Check This Point Whether Had A Warp Point Or Not, 
  // In This Map
  if(!pTheCallMap->AddWarpPoint(pNewWarp, MAKELONG(iSourMapY, iSourMapX)))
  {
    SAFE_DELETE(pNewWarp);
    return FALSE;
  }
  // Announce All The Player Open The Door~
  AnnounceAddPoint(wSourMapId);
  return TRUE;
}

BOOL CGameMap::AnnounceAddPoint(WORD wMapId)
{
  CGameMap *pTheMainMap = g_pGs->GetGameMap(wMapId/100*100); //十位和个位置零 查找主城ID:1100  2100 3100 
  if(pTheMainMap != NULL)
  { 
    SMccMsgData *pNewMccMsg = g_pGs->NewMccMsgBuffer();
    if(NULL == pNewMccMsg)  { return FALSE; }
    
    string szTalkToAll("Ωじぇ竒瞷琘返");
 //   szTalkToAll += pTheMainMap->GetName();
 //   szTalkToAll += "返.";

    pNewMccMsg->Init(NULL);
    pNewMccMsg->dwAID        = AP_GMINSTR;
    pNewMccMsg->dwMsgLen     = 2;
    pNewMccMsg->Msgs[0].Size = sizeof(SNMGMInstruction);
    pNewMccMsg->Msgs[1].Size = szTalkToAll.length() +1;
    
    SNMGMInstruction *pInstr = (SNMGMInstruction*)pNewMccMsg->Msgs[0].Data;
    memset(pInstr, 0, sizeof(SNMGMInstruction));
    pInstr->wCode = 110;
    pInstr->wGMInstCode = GM_INST_TALKTOALL;
    memcpy(pInstr->szName, "RCH", MAX_ACCOUNT_LEN);  // szName & wCode Only Fill In
    pInstr->szName[MAX_ACCOUNT_LEN-1] = '\0';
    memcpy(pNewMccMsg->Msgs[1].Data, szTalkToAll.c_str(), pNewMccMsg->Msgs[1].Size);
    pNewMccMsg->Msgs[1].Data[szTalkToAll.length()] = '\0';
    
    g_pMccDB->AddSendMsg( pNewMccMsg );
  }
  return TRUE;
}
#endif //_AUTO_ADD_WARP_POINT_
//=====================================================================================
//
//
void CMonsterTeamers::NotifyMyTeamers( const WORD & wCode, CPlayer * pTarget, CMonster * pNptifyer )
{
  static int		iNotifyNow1;
#ifdef _DEBUG_CHECK_FUNC_STATE_
  CCheckFuncState callStackCheck("CMonsterTeamers::NotifyMyTeamers()");
#endif
  
  for( iNotifyNow1 = 0; iNotifyNow1 < m_wCount; iNotifyNow1++ )
  {
    if( m_pMonster[iNotifyNow1] )
    {
      if( m_pMonster[iNotifyNow1] != pNptifyer )
      {
        m_pMonster[iNotifyNow1]->SetNotifyCode( wCode, pTarget, pNptifyer );
      }
    }
  }
}
//=====================================================================================
//
//
void CMonsterTeamers::TeamerRevive( CMonster *pReviver )
{
  static int		iNotifyNow2 = 0;
  
  for( iNotifyNow2 = 0; iNotifyNow2 < m_wCount; iNotifyNow2++ )
  {
    if( m_pMonster[iNotifyNow2] )
    {
      if( m_pMonster[iNotifyNow2] != pReviver &&
        m_pMonster[iNotifyNow2]->GetDistance( pReviver->GetPosX(), pReviver->GetPosY() ) < m_pMonster[iNotifyNow2]->GetHelpRange() )
      {
        m_pMonster[iNotifyNow2]->m_wNotifyTimes = 0;
      }
    }
  }
}
//=====================================================================================
//
//
void CMonsterTeamers::InitTeamBirthPlace()
{
  WORD															wRandX, wRandY, wDisX, wDisY;
  map<DWORD, SWarpPoint*>::iterator	iter;
  int i = 0;
  
GOON1:
  for( ; i < 10; i++ )
  {
    for( int j = 0; j < 20; j++ )
    {
GOON2:
    if( j == 19 )
    {
      wRandX = m_wInitX -1 + gf_GetRandom( 2 );
      wRandY = m_wInitY -1 + gf_GetRandom( 2 );
      m_wBirthPlaceX[i] = wRandX;
      m_wBirthPlaceY[i] = wRandY;
      i++;
      goto GOON1;
    }
    else
    {
      wRandX = gf_GetRandom( m_pMap->GetSizeX() );
      wRandY = gf_GetRandom( m_pMap->GetSizeY() );
    }
    if( !( m_pMap->GetTileFlag( wRandX, wRandY ) & TILE_ALLOCCULDE ) )
    {
      for( iter = m_pMap->m_pBase->m_mapWarpPoint.begin(); iter != m_pMap->m_pBase->m_mapWarpPoint.end(); iter++ )
      {
        wDisX = abs( wRandX - HIWORD(iter->first) );
        wDisY = abs( wRandY - LOWORD(iter->first) );
        if( wDisX + wDisY < 24 )
        {
          j++;
          goto GOON2;
        }
      }
      m_wBirthPlaceX[i] = wRandX;
      m_wBirthPlaceY[i] = wRandY;
      break;
    }
    }
  }
}
//=====================================================================================
//
//
CNpc *CGameMap::TheTileHaveNpc( WORD X, WORD Y )
{
  static map<DWORD, CNpc*>::iterator      Iter_Npc;
  CNpc                                    *pNpc;
  
  for( Iter_Npc = m_mapCodeNpc.begin(); Iter_Npc != m_mapCodeNpc.end(); Iter_Npc++ )
  {
    pNpc = Iter_Npc->second;
    if( pNpc->GetPosX() == X && pNpc->GetPosY() == Y )      return pNpc;
  }
  return NULL;
}
#ifdef _DEBUG_MICHAEL_OPEN_FERRY
//=====================================================================================
// Function: 检查渡船状态
// Note:     GameMap只需检查是否到站，触发开船是由NPC来完成的
//
void CGameMap::FerryDoAction()
{  
  if( !IsBoatMap() || GetFerryState() == BOAT_STATE_WAIT )
  {
    return;
  }
  
  // 怪物袭船
  if( ClientTickCount > GetMonsterAttackFerryTime() && GetFerryState() == BOAT_STATE_RUN )
  {
#ifdef _NEW_FERRY_SYSTEM_
    DoNewMonsterAttack();
#else
    DoMonsterAttack();
#endif
    return;
  }
  
  // 到站了，大家下船
  if( ClientTickCount > GetNextFerryStateTime() )
  {    
    WarpAllFerryer();
    SetFerryState( BOAT_STATE_WAIT );
    SetNextFerryStateTime( 0x7FFFFFFF );
    return;
  }	
}

#ifdef _NEW_TRADITIONS_WEDDING_
void CGameMap::AwaneDoAction()
{
  if (m_bAwaneMap)
  {
    if (TimeGetTime() > m_dwAwaneLastTick)
    {
      m_dwAwaneLastTick += m_dwAwaneInterval;

      CPlayer				*pThePlayer  = NULL;
      map<DWORD,CPlayer*>::iterator	iter_Player = m_mapCodePlayer.begin();
      for( ; iter_Player != m_mapCodePlayer.end(); iter_Player++)
      {
        pThePlayer = (CPlayer*)(iter_Player->second);
        if (pThePlayer) pThePlayer->Awane(m_fAwaneRadio);
      }
    }
  }
}
#endif
//=====================================================================================
// Function: 到站了，船上所以人都给我扔出去
// Note :    Modify from CGameMap::WarpAllEnemy
//
void CGameMap::WarpAllFerryer()
{
  if( !IsBoatMap() )
  {
    return;
  }
  //
  map<DWORD, CPlayer*>::iterator    Iter_Pl;
  SWarpPoint                        WarpPoint;
  CPlayer                           *pPlayer   = NULL;
  CSrvBaseMap                       *pGameMap  = NULL;
  WORD                              wLoop = 0, wWarp_Count = 0, wRevive_Count = 0;
  WORD															wDesMap = 0, wDesX = 0, wDesY = 0;
  CFerryData										    *pFerryData = NULL;
  //
  if( NULL == ( pFerryData = g_pFerryMng->GetDataByBoatMapId( GetMapId() ) ) )    return;
  //
  wDesMap = pFerryData->GetDesMapId();
  wDesX		= pFerryData->GetDesX();
  wDesY		= pFerryData->GetDesY();
  // 
  if( NULL == ( pGameMap = g_pBase->GetBaseMap( wDesMap ) ) )                     return;
  //
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pPlayer = Iter_Pl->second;
    //
    if( pPlayer->GetStatus() == STATUS_PLAYER_DEAD )
    {
      g_pMultiSendPlayer_3[wRevive_Count] = pPlayer;
      wRevive_Count++;
    }
    else
    {
      WarpPoint.wTargetMapId  = wDesMap;
      WarpPoint.wTargetMapX		= wDesX;
      WarpPoint.wTargetMapY		= wDesY;
      //
      pPlayer->SetWarpPoint( &WarpPoint );
      g_pMultiSendPlayer_2[wWarp_Count] = pPlayer;
      wWarp_Count++;
    }
  }
  // 到站下船
  for( wLoop = 0; wLoop < wWarp_Count; wLoop++ )
  {
    if( g_pMultiSendPlayer_2[wLoop] )
    {
      g_pMultiSendPlayer_2[wLoop]->Send_A_WARP( *(g_pMultiSendPlayer_2[wLoop]->GetSelfWarpPoint()), PLAYER_WARP_TYPE_NPC );
    }
  }
  // 出师未捷身先死，免费送回城
  for( wLoop = 0; wLoop < wRevive_Count; wLoop++ )
  {
    if( g_pMultiSendPlayer_3[wLoop] )
    {
      g_pMultiSendPlayer_3[wLoop]->Revive( REVIVE_GO_BACK_TOWM, 0 );
    }
  }
  // 收怪物
  map<DWORD,CMonster*>::iterator			mIte;
  for( mIte = m_mapCodeMonster.begin(); mIte != m_mapCodeMonster.end(); mIte++ )
  {
    (mIte->second)->DiedForFerry();
  }
}
//=====================================================================================
// Function: 设置怪物袭船的相关状态
// Note :    只有船的地图才可以被呼叫到这个Function
//
void CGameMap::SetMonsterAttackState()
{
  CFerryData										    *pFerryData = NULL;
  //
  if( !IsBoatMap()  ||
		  !(pFerryData = g_pFerryMng->GetDataByBoatMapId(GetMapId())) ||
      pFerryData->GetMonsterNumber() == 0	|| 
      pFerryData->GetAttackOdss() < gf_GetRandom( 100 )	)
  {
    return;
  }
  DWORD dwAttackTime = gf_GetRandom( FERRY_MON_ATTACK_BEFORE - FERRY_MON_ATTACK_AFTER ) + FERRY_MON_ATTACK_AFTER;
  dwAttackTime = dwAttackTime * 60 * 1000;
  SetMonsterAttackTime( dwAttackTime + ClientTickCount );
#ifdef _DEBUG
  FuncName("CGameMap::SetMonsterAttackState");
  char szStr[1024];
  
  _snprintf( szStr, 1024-1, " Monster Attact After %d Minute # ", dwAttackTime/60000 );
  szStr[1024-1] = '\0';
  AddMemoMsg( szStr );
#endif
}

//=====================================================================================
// Function: 怪物袭船
// Note :    只有船的地图才可以被呼叫到这个Function
//
inline void CGameMap::DoMonsterAttack()
{	
  CMonster				 *pNewMonster    = NULL;
  WORD							pMonsterCount[ FERRY_MON_NUM + FERRY_BOSS_NUM ];
  WORD							wAMonsterCount = 0;
  WORD              wCreateCount   = 0;
  DWORD             dwWarpMapFlag  = 0;
  //
  SetFerryState( BOAT_STATE_MON );
  SetMonsterAttackTime( 0x7FFFFFFF );
  
  memset( pMonsterCount, 0, (FERRY_MON_NUM + FERRY_BOSS_NUM) * sizeof(WORD) );
  
  CFerryData        *pFerryData = NULL;
  if( NULL == ( pFerryData = g_pFerryMng->GetDataByBoatMapId( GetMapId() ) ) )  
  {
    return;
  }
  //
  wAMonsterCount = 2 * 2 * pFerryData->GetMonsterNumber() / FERRY_MON_NUM;
  for( int iLoop = 0; iLoop < FERRY_MON_NUM; iLoop++ )
  {
    if( 1 == gf_GetRandom( 2 ) )
    {
      pMonsterCount[iLoop]  = gf_GetRandom( wAMonsterCount ) + 1;
    }
    else
    {
      pMonsterCount[iLoop]  = 0;
    }
  }
  //
  if( FERRY_BOSS_NUM == 2 )
  {
    pMonsterCount[FERRY_MON_NUM+0] = gf_GetRandom( FERRY_BOSS_NUM + 1 );
    pMonsterCount[FERRY_MON_NUM+1] = FERRY_BOSS_NUM - pMonsterCount[FERRY_MON_NUM+0];
  }
  
  //
  SMsgData       *pNewMsg = g_pGs->NewMsgBuffer();
  if( NULL == pNewMsg )     return;
  //
  pNewMsg->dwAID					= A_PLAYERINFO;
  pNewMsg->dwMsgLen				= 2;
  pNewMsg->Msgs[0].Size		= 1;
  //
  SNMNpcInfo    *pTheNpcInfo = (SNMNpcInfo*)(pNewMsg->Msgs[1].Data);
  SAdjustPoint  *pWarpCoor   = NULL;
  
  for( WORD wLoop1 = 0; wLoop1 < FERRY_MON_NUM + FERRY_BOSS_NUM; wLoop1++ )
  {
    for( WORD wLoop2 = 0; wLoop2 < pMonsterCount[wLoop1]; wLoop2++ )
    {
      if( pWarpCoor = GetAdjustPoint() )
      {
        if( NULL != ( pNewMonster = CreateMonster( pFerryData->GetMonsterID(wLoop1), pWarpCoor->wAdjustX, pWarpCoor->wAdjustY, 0 , 0, TRUE ) ) )
        {
          pNewMonster->SetReviveType( REVIVE_TYPE_DELETE );
          pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
          pTheNpcInfo++;
          wCreateCount++;
        }
      }
    }
  }
  pNewMsg->Msgs[1].Size = sizeof( SNMNpcInfo ) * wCreateCount;
  SendTheMsgToAll( pNewMsg );
}
#ifdef _NEW_FERRY_SYSTEM_
void CGameMap::DoNewMonsterAttack()
{
  CMonster				 *pNewMonster    = NULL;
  WORD							wAMonsterCount = 0;
  WORD              wCreateCount   = 0;
  WORD							pMonsterCount[FERRY_MON_NUM]; // 只产生3种小怪，1种BOSS
  //
  SetFerryState( BOAT_STATE_MON );
  SetMonsterAttackTime( 0x7FFFFFFF );
  
  memset(pMonsterCount, 0, (FERRY_MON_NUM) * sizeof(WORD));
  
  CFerryData        *pFerryData = NULL;
  if( NULL == ( pFerryData = g_pFerryMng->GetDataByBoatMapId( GetMapId() ) ) )  
  {
    return;
  }
  //
  wAMonsterCount = 2 * 2 * pFerryData->GetMonsterNumber() / FERRY_MON_NUM;
  for( int iLoop = 0; iLoop < FERRY_MON_NUM; iLoop++ )
  {
    pMonsterCount[iLoop]  = gf_GetRandom( wAMonsterCount ) + 1;
  }
  //
  int          iOdds     = 100 - gf_GetRandom(100);
  unsigned int iAbsOdds  = 0;
  WORD         iBossId   = 0;
  unsigned int iBossOdds = 100;
  DWORD        dwMonster;

  for(WORD wCount=FERRY_MON_NUM; wCount<FERRY_BOSS_NUM; wCount++)
  {
    dwMonster = pFerryData->GetMonsterInfor(wCount);   // 高16位表示MonsterId 低16位表示产生几率 
    iAbsOdds  = abs(iOdds - HIWORD(dwMonster));
    if(iAbsOdds<iBossOdds) 
    {
      iBossId   = LOWORD(dwMonster);
      iBossOdds = iAbsOdds;
    }
  }
  //
  SMsgData  *pNewMsg = g_pGs->NewMsgBuffer();
  if( NULL == pNewMsg )     return;
  //
  pNewMsg->dwAID					= A_PLAYERINFO;
  pNewMsg->dwMsgLen				= 2;
  pNewMsg->Msgs[0].Size		= 1;
  // 产生3种小怪
  SNMNpcInfo    *pTheNpcInfo = (SNMNpcInfo*)(pNewMsg->Msgs[1].Data);
  SAdjustPoint  *pWarpCoor   = NULL;
  
  for(WORD wLoop1 = 0; wLoop1 < FERRY_MON_NUM; wLoop1++)  
  {
    for(WORD wLoop2 = 0; wLoop2 < pMonsterCount[wLoop1]; wLoop2++)
    {
      if(pWarpCoor=GetAdjustPoint())
      {
        dwMonster = pFerryData->GetMonsterInfor(wLoop1);
        if(HIWORD(dwMonster)!=0)   { continue; }    // 
        if( NULL != ( pNewMonster = CreateMonster(LOWORD(dwMonster), pWarpCoor->wAdjustX, pWarpCoor->wAdjustY, 0 , 0, TRUE) ) )
        {
          pNewMonster->SetReviveType( REVIVE_TYPE_DELETE );
          pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
          pTheNpcInfo++;
          wCreateCount++;
        }
      }
    }
  }
  // 放1种BOSS
  if((iBossId!=0) && (pWarpCoor = GetAdjustPoint()))
  {
    pNewMonster = CreateMonster(iBossId, pWarpCoor->wAdjustX, pWarpCoor->wAdjustY, 0 , 0, TRUE);
    if(pNewMonster!=NULL)
    {
      pNewMonster->SetReviveType( REVIVE_TYPE_DELETE );
      pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
      pTheNpcInfo++;
      wCreateCount++;
    }
  }
  pNewMsg->Msgs[1].Size = sizeof( SNMNpcInfo ) * wCreateCount;
  SendTheMsgToAll( pNewMsg );
}
#endif //_NEW_FERRY_SYSTEM_
#endif

//=====================================================================================
// Function: 清除怪物状态
// Note :    
//
void CGameMap::ClearMonsterStatus()
{
  map<DWORD,CMonster*>::iterator			mIte;
  CMonster*														pMonster = NULL;
  
  for( mIte = m_mapCodeMonster.begin(); mIte != m_mapCodeMonster.end(); mIte++ )
  {
    pMonster = (CMonster*)mIte->second;
    if( NULL != pMonster )		        	pMonster->ClearStatus();
    //
  }	
}


//=====================================================================================
//  Function:  定点造怪物
//  Param : iId ( MonsterId )                   iX iY ( BasePos )
//          iRandomX iRandomY ( Random Rate )   bDisrevive ( revive flag default FALSE )
CMonster * CGameMap::CreateMonster(int iId, int iX, int iY, int iRandomX, int iRandomY, BOOL bDisrevive)
{
  CSrvBaseMonster									*pBaseMonster;
  DWORD														dwX_Y = 0, dwMonsterCode = 0;
  CMonster												*pMonster;
  
  if( NULL != ( pBaseMonster = g_pBase->GetBaseMonster( iId ) ) )
  {
    if( ( dwMonsterCode = GetNewMonsterCode() ) <= CODE_MAX_MONSTER )
    {
      pMonster = new CMonster( pBaseMonster, dwMonsterCode );
      if( pMonster )
      {
        if( !pMonster->InitSkill() )
        {
          SAFE_DELETE( pMonster );
          return NULL;
        }
        if( bDisrevive )      pMonster->SetReviveType( REVIVE_TYPE_DELETE );
        AddMonster( pMonster, iX, iY );
        pMonster->SetGameMap(this);
        pMonster->InitPos( iX, iY, iX + gf_GetRandom( iRandomX ), iY + gf_GetRandom( iRandomY ) );
        pMonster->SetWanderRange();
        g_dwMonsterTotal++;
        return pMonster;
      }
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////
//Add by CECE 2004-04-08
#ifdef  EVILWEAPON_3_6_VERSION
void CGameMap::DoWarPointpAction()
{
  if( m_dwWarpTickCount >= ClientTickCount )        return;
  m_dwWarpTickCount = ClientTickCount + 2000;
  //
  SWarpPoint                          *pWarp = NULL;
  map<DWORD, SWarpPoint*>::iterator   Iter_Wp;
  //
  for( Iter_Wp = m_mapCodeWarpPoint.begin(); Iter_Wp != m_mapCodeWarpPoint.end(); )
  {
    pWarp = Iter_Wp->second;
    if( pWarp && pWarp->dwLifeTime < ClientTickCount )
    {
      // Send Clear Code
      *(DWORD*)MsgClearCode.Msgs[0].Data = pWarp->wCode;
      SendMsgNearPosition( MsgClearCode, pWarp->wPosX, pWarp->wPosY );
      //
      if( !m_pBase->DeleteWarpPoint( MAKELONG( pWarp->wPosY, pWarp->wPosX ) ) )
      {
        // Show Log, Memry Leak
#ifdef _DEBUG
        FuncName("CGameMap::DoWarPointpAction");
        AddMemoErrMsg( "Release SWarpPoint Memory Failed..." );
#endif
      }
      Iter_Wp = m_mapCodeWarpPoint.erase( Iter_Wp );
    }
    else
    {
      Iter_Wp++;
    }
  }
}
#endif
////////////////////////////////////////////////////////////////
//add by cece 2004-07-14
#ifdef ELYSIUM_3_7_VERSION

//检查选手允许带的道具
CCheckItemTable g_CheckItemTable;

void CCheckItemTable::Load( /*const*/ char* szFileName )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  CInStream in(szFileName);
  if(in.fail()||in.GetFileSize() == 0)
    return;
#else
  ifstream in( szFileName );
  if( in.fail() ) return;
#endif//_DEBUG_JAPAN_DECRYPT_
  m_Table.clear();
  SCheckItem sItem;
  //
  int iConId, iValue, iCount;
  while( in>>iConId )
  {
    sItem.Clear();
    in>>iValue; //iCheckType;
    sItem.iCheckType = iValue;
    in>>iValue; //iAllTotal
    sItem.iAllTotal  = iValue;
    in>>iCount;
    for( int i = 0; i<iCount; i++ )
    {
      in>>iValue; //iItemId
      sItem.listId.push_back( iValue );
    }
    //
    m_Table.push_back( sItem );
  }
} 

LPSCheckItem CCheckItemTable::GetCon( int iConId )
{
  if( iConId < m_Table.size() )
  {
    return &m_Table[iConId];
  }
  return NULL;
}

CFightFieldMgr g_FightFieldMgr;

CFightFieldMgr::CFightFieldMgr()
{
  //must be modify
  m_pFieldList   = NULL;
  m_iFieldCount  = 0;
  m_pMatchResult = NULL;
  //
  SetFieldTime( 60 );
  SetLastTickCount( 0 );
  SetMatchId( 0 );
  SetMatchCount( 0 );
  SetFighterCount( 0 );
  SetAction( MATCH_ACTION_NONE );
  SetMatchMode( MATCH_MODE_NONE );
  SetMatchType( -1 );
  //
  SetMatchState( MATCH_STATE_NONE );
  SetRestTime( 0 );
  //must be modify
  m_iLobbyMapId = 0;
  SetLobby( NULL );
  //
  InitFight();
  //为初赛
  InitLayer();//SetLayer( 1 );
  m_iAreaMatchCount = SIGNUP_NOTLIVE_NUM_MAX/TOTAL_MATCH_TIMES;
  //Alloc Challenger table
  for( int i = 0; i< MATCH_TYPE_MAX; i++ )
  {
    m_pChallengerTable[i] = new LPSChallenger*[m_iAreaMatchCount];  //一个区包括16人
    for( int m = 0; m < m_iAreaMatchCount; m++ )
    {
      m_pChallengerTable[i][m] = new LPSChallenger[TOTAL_MATCH_TIMES];
    }
  }
  //
  SetUpdateTime( 1 );
  SetSignupTimeFlag( FALSE );
  SetQueryTimeFlag( FALSE );
  SetFightTimeFlag( FALSE );
  SetAfterFight( FALSE );
  SetNoMatch( FALSE );
  //
  MATCH_MONEY_COST = 1000000;
  LIVEMODE_PLAYER_NUM_MIN = 8;
  //
  //生存赛所需基本数据
  m_iStepByCount = 0;        //生存赛下降一阶的人数
  m_iStepCount = 0;          //生存赛下降的总阶数
  SetLastCount( 0 );         //生存赛下降时的总人数     
  m_pLiveMatchData = 0;      //生存赛的放怪数据
  m_iMonsterLevelStep = 0;   //不同等级怪物的层数
  m_plistMonster = NULL;     //不同等级的怪物ID
#ifdef FF_EXTEND_ZETOR
  m_iSignUpType = 0;
#endif
}


void CFightFieldMgr::InitFight()
{
  //对应好参赛选手在比赛过程记录中的位置
  for( int i=0; i< TOTAL_MATCH_TIMES; i++ )
  {
    m_Records[i].Init();
  } 
  memset( m_pResultTable, 0, sizeof(m_pResultTable) );
  for( i=0; i< MATCH_AREA_WINNER_MAX; i++ )
  {
    m_MatchWinner[i].Init();
  }
  //memset( m_pAreaWinner, 0, sizeof(m_pAreaWinner) );
  //Init Challenger table
  /*
  for( i = 0; i< MATCH_TYPE_MAX; i++ )
  {
  for( int m = 0; m < m_iAreaMatchCount; m++ )
  {
  for( int n = 0; n< TOTAL_MATCH_TIMES; n++ )
  {
  m_pChallengerTable[i][m][n] = NULL;
  }
  }
  }
  */
  SetSendCount( 0 );
  SetMatchId( 0 );
}

BOOL CFightFieldMgr::BeginSignUp( SMsgData * pTheMsg,CPlayer* pPlayer )
{
  int iMatchMode,iTime;
#ifdef FF_EXTEND_ZETOR
  int iSignUpType;
#endif
  SNMGMInstruction* pInstr = (SNMGMInstruction*)pTheMsg->Msgs[0].Data;
  iMatchMode = pInstr->wValue[0];
#ifdef FF_EXTEND_ZETOR
  iSignUpType = pInstr->wValue[1];
  iTime      = pInstr->wValue[2];
#else
  iTime      = pInstr->wValue[1];
#endif
  char* pszContent = (char*)pTheMsg->Msgs[1].Data;
  //如果已有指令在执行,则无效
  if( GetRestTime() > 0 )
  {
    return FALSE;
  }
  //如果不是报名模式的范围或时间<=0,则无效
  if( iMatchMode <= MATCH_MODE_NONE ||
    iMatchMode >= MATCH_MODE_MAX  ||
    iTime <= 0 )
  {
    return FALSE;
  }
#ifdef FF_EXTEND_ZETOR
  if( iSignUpType < 0 || iSignUpType > 4 )
  {
    return FALSE;
  }
  SetSignUpType(iSignUpType);
#endif
  SetAction( MATCH_ACTION_SIGNUP );
  SetMatchMode( iMatchMode );
  SetRestTime( iTime*60 );
  SetLastTickCount( GetTickCount() );
  //清除上次的报名信息
  ClearNameList();
  InitResutlInfo();
  InitAllLayerResult();
  //为初赛
  InitLayer();//SetLayer( 1 );
  SetSignupTimeFlag( TRUE );
  SetQueryTimeFlag( FALSE );
  SetFightTimeFlag( FALSE );
  SetAfterFight( FALSE );
  SetUpdateTime( 1 );
  //
  SetWinnerMatch( FALSE );
  SetLastCount( 0 );
  //send to all
  pPlayer->SendTalkToAll( pszContent );
  return TRUE;
}

//检查人数是否已满
BOOL CFightFieldMgr::CheckSignup( CPlayer* pPlayer, SMsgData * pTheMsg )
{
  LPSSignup pSignup = (LPSSignup)pTheMsg->Msgs[0].Data;
  int iMatchType = pSignup->wMatchType;
  //check signup time
  if( GetAction() != MATCH_ACTION_SIGNUP )
  {
    pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
      MATCH_ERROR_NOTIN_SIGNUP_TIME );
    return FALSE;
  }
  //check mode
  if( pSignup->wMatchMode != GetMatchMode() )
  {
    pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, MATCH_ERROR_MODE );
    return FALSE;
  }
  //check time
  if( GetRestTime() == 0 )
  {
    pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, MATCH_ERROR_TIMEOUT );
    return FALSE;
  }
#ifdef _MODIFY_GUID_TEAMMACTH_
	//check name
	if (strlen(pSignup->szName) >= MAX_PLAYER_NAME_LEN -1)
	{
		g_pGs->ReleaseMsg( pTheMsg );
		return FALSE;
	}
#endif
  //
  int iSignupNum = 0; //当前报名人数
  switch( pSignup->wMatchMode )
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
    {  
      //check money
      if( pPlayer->GetMoney() < MATCH_MONEY_COST )
      {
        pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
          MATCH_ERROR_MONEY );
        
        return FALSE;
      }
      //1. 检查职业是否符合
      //2. 检查人数要求
      int iOccu = pPlayer->GetOccupation();
      switch( iOccu )
      {
      case OCCU_SWORDMAN:	 // 人类男剑宗
      case OCCU_SWORDMANF: // 人类女剑宗
#ifdef FF_EXTEND_ZETOR
        if(m_iSignUpType != 1) return FALSE;
#endif     
        if( iMatchType != MATCH_TYPE_SWORD )
        {
          pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
            MATCH_ERROR_SIGNUP_OCCU );
          return FALSE;
        }
        else
        {
          //检查是否报过名
          LPSChallenger pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_SWORD );
          if( pFighter )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_BE_SIGNUP );
            return FALSE;
          }
          iSignupNum = GetSignupNum( MATCH_TYPE_SWORD );
          //如果人数已满，则抱错
          if( iSignupNum >= SIGNUP_OCCU_NUM_MAX )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_NAMELIST_FUUL );
            return FALSE;
          }
        }
        break;
      case OCCU_BLADEMAN:	 // 人类男诡道
      case OCCU_BLADEMANF: // 人类女诡道
#ifdef FF_EXTEND_ZETOR
        if(m_iSignUpType != 3) return FALSE;
#endif   
        if( iMatchType != MATCH_TYPE_BLADE )
        {
          pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
            MATCH_ERROR_SIGNUP_OCCU );
          return FALSE;
        }
        else
        {
          //检查是否报过名
          LPSChallenger pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_BLADE );
          if( pFighter )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_BE_SIGNUP );
            return FALSE;
          }
          iSignupNum = GetSignupNum( MATCH_TYPE_BLADE );
          //如果人数已满，则抱错
          if( iSignupNum >= SIGNUP_OCCU_NUM_MAX )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_NAMELIST_FUUL );
            return FALSE;
          }
        }
        break;
      case OCCU_PIKEMAN:	// 人类男戟门
      case OCCU_PIKEMANF:	// 人类女戟门
#ifdef FF_EXTEND_ZETOR
        if(m_iSignUpType != 2) return FALSE;
#endif   
        if( iMatchType != MATCH_TYPE_PIKE )
        {
          pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
            MATCH_ERROR_SIGNUP_OCCU );
          return FALSE;
          
        }
        else
        {
          //检查是否报过名
          LPSChallenger pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_PIKE );
          if( pFighter )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_BE_SIGNUP );
            return FALSE;
          }
          iSignupNum = GetSignupNum( MATCH_TYPE_PIKE );
          //如果人数已满，则抱错
          if( iSignupNum >= SIGNUP_OCCU_NUM_MAX )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_NAMELIST_FUUL );
            return FALSE;
          }
        }
        break;
      case OCCU_WIZARD:	  // 人类男幻道
      case OCCU_WIZARDF:  // 人类女幻道
#ifdef FF_EXTEND_ZETOR
        if(m_iSignUpType != 4) return FALSE;
#endif   
        if( iMatchType != MATCH_TYPE_WIZARD )
        {
          pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
            MATCH_ERROR_SIGNUP_OCCU );
          return FALSE;
        }
        else
        {
          //检查是否报过名
          LPSChallenger pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_WIZARD );
          if( pFighter )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_BE_SIGNUP );
            return FALSE;
          }
          iSignupNum = GetSignupNum( MATCH_TYPE_WIZARD );
          //如果人数已满，则抱错
          if( iSignupNum >= SIGNUP_OCCU_NUM_MAX )
          {
            pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
              MATCH_ERROR_NAMELIST_FUUL );
            return FALSE;
          }
        }
        break;
        }
      }
      break;
   case MATCH_MODE_WEIGHT:        //量级赛
     {
       //check money
       if( pPlayer->GetMoney() < MATCH_MONEY_COST )
       {
         pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
           MATCH_ERROR_MONEY );
         
         return FALSE;
       }
       int iLevel = pPlayer->GetLevel();
       switch( iMatchType )
       {
       case MATCH_TYPE_HEAVYWEIGHT: //重量级
         {
#ifdef FF_EXTEND_ZETOR
           if( m_iSignUpType != 3 ) return FALSE;
#endif  
           //检查是否报过名
           LPSChallenger pFighter = NULL;
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_LIGHTWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_MIDDLEWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           //
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_HEAVYWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           iSignupNum = GetSignupNum( MATCH_TYPE_HEAVYWEIGHT );
           if( iSignupNum >= SIGNUP_WEIGHT_MUN_MAX )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAMELIST_FUUL );
             return FALSE;
           }
         }  
         break;
       case MATCH_TYPE_LIGHTWEIGHT: //初量级
         {
#ifdef FF_EXTEND_ZETOR
           if( m_iSignUpType != 1 ) return FALSE;
#endif  
           if( iLevel > 50 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_SIGNUP_LEVEL );
             return FALSE;
           }
           //检查是否报过名
           LPSChallenger pFighter = NULL;
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_MIDDLEWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_HEAVYWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           //
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_LIGHTWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           iSignupNum = GetSignupNum( MATCH_TYPE_LIGHTWEIGHT );
           if( iSignupNum >= SIGNUP_WEIGHT_MUN_MAX )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAMELIST_FUUL );
             return FALSE;
           }
         }
         break;
       case MATCH_TYPE_MIDDLEWEIGHT: //中量级
         {
#ifdef FF_EXTEND_ZETOR
           if( m_iSignUpType != 2 ) return FALSE;
#endif  
           if( iLevel > 80 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_SIGNUP_LEVEL );
             return FALSE;
           }
           //检查是否报过名
           LPSChallenger pFighter = NULL;
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_LIGHTWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_HEAVYWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           //
           pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_MIDDLEWEIGHT );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           iSignupNum = GetSignupNum( MATCH_TYPE_MIDDLEWEIGHT );
           if( iSignupNum >= SIGNUP_WEIGHT_MUN_MAX )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAMELIST_FUUL );
             return FALSE;
           }
         }
         break;
      }//switch( iMatchType )
    }
    break;
   case MATCH_MODE_TEAM:          //团体赛
     {
       CTeam* pTeam = pPlayer->GetTeam();
       //不是组队状态
       if( pTeam == NULL )
       {
         pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
           MATCH_ERROR_NO_TEAM );
         return FALSE;
       }
       //不是队长
       if( pTeam->GetLeaderMailId() != pPlayer->GetMailId() )
       {
         pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
           MATCH_ERROR_NOT_TEAM_LEADER );
         return FALSE;
       }
       //
       switch( iMatchType ) 
       {
       case MATCH_TYPE_2VS2: //2对2
         {
#ifdef FF_EXTEND_ZETOR
           if( m_iSignUpType != 1 ) return FALSE;
#endif  
           //check money
           if( pPlayer->GetMoney() < MATCH_MONEY_COST*2 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_MONEY );
             
             return FALSE;
           }           
           if( pTeam->GetMembersNum() != 2 )
           {         
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_TEAM_NUM );
             return FALSE;
           }
           //必须是夫妻队
           WORD wCount;
           DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
           if( pMailIdList==NULL )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NO_TEAM );
             return FALSE;
           }
           if( pMailIdList[1] == 0 || pPlayer->GetSpouseMailId() != pMailIdList[1] )
             //if( pPlayer->GetSpouseMailId() != pTeam->GetMember( (int)1 )->GetMailId() )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NOT_SPOUSE );
             return FALSE;
           }
           //检查队伍数量
           if( GetSignupNum( MATCH_TYPE_2VS2 ) >= SIGNUP_MODE_TEAM )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAMELIST_FUUL );
             return FALSE;
           }
           //检查是否报过名
           LPSChallenger pFighter = FindTeamMem( pPlayer->GetMailId() );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           //是否重名
           pFighter = FindTeamByName( pSignup->szName );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAME_EXIST );
             return FALSE;
           }
         }
         break;
       case MATCH_TYPE_4VS4: //4对4
         {
#ifdef FF_EXTEND_ZETOR
           if( m_iSignUpType != 2 ) return FALSE;
#endif  
           //check money
           if( pPlayer->GetMoney() < MATCH_MONEY_COST*4 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_MONEY );
             
             return FALSE;
           }           
           if( pTeam->GetMembersNum() != 4 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_TEAM_NUM );
             return FALSE;
           }
           //检查队伍数量
           if( GetSignupNum( MATCH_TYPE_4VS4 ) >= SIGNUP_MODE_TEAM )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAMELIST_FUUL );
             return FALSE;
           }
           //检查是否报过名
           LPSChallenger pFighter = FindTeamMem( pPlayer->GetMailId() );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           //是否重名
           pFighter = FindTeamByName( pSignup->szName );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAME_EXIST );
             return FALSE;
           }
         }
         break;
       case MATCH_TYPE_6VS6: //6对6
         {
       #ifdef FF_EXTEND_ZETOR
           if( m_iSignUpType != 3 )     return FALSE;
       #endif
       #ifdef _MODIFY_GUID_TEAMMACTH_
					 const WORD wGuildLevel = 90;
					 CGuild *pGuild = g_pGuildMng->FindOneGuild( pPlayer->GetUnionId() );
					 if (NULL == pGuild)
					 {
						 pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_GUILD_EXIST );
             return FALSE;
					 }
					 if ( 2 != (pGuild->IsMaster(pPlayer->GetMailId())) )
					 {
						 pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NOT_GUILDMASTER );
             return FALSE;
					 }
					 if (pPlayer->GetLevel() < wGuildLevel)
					 {
						 pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_GUILD_LEVEL );
						 return FALSE;
					 }
					 //
					 CPlayer *pUser;
					 WORD wCount, wIter;
           DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
           if( pMailIdList==NULL )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NO_TEAM );
             return FALSE;
           }
					 for ( wIter=0; wIter < wCount; ++wIter)
					 {
						 pUser = g_pGs->GetPlayerFromMailId(pMailIdList[wIter]);
						 if (NULL == pUser)   continue;
						 if (pUser->GetMailId() == pPlayer->GetMailId()) continue;
						 if (pUser->GetLevel() < wGuildLevel)
						 {
							 pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH,
								 MATCH_ERROR_TEAM_LEVEL );
							 return FALSE;
						 }
					 }
       #endif //_MODIFY_GUID_TEAMMACTH_
           //check money
           if( pPlayer->GetMoney() < MATCH_MONEY_COST*6 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_MONEY );
             
             return FALSE;
           }
           if( pTeam->GetMembersNum() != 6 )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_TEAM_NUM );
             return FALSE;
           }
           //检查队伍数量
           if( GetSignupNum( MATCH_TYPE_6VS6 ) >= SIGNUP_MODE_TEAM )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAMELIST_FUUL );
             return FALSE;
           }
           //检查是否报过名
           LPSChallenger pFighter = FindTeamMem( pPlayer->GetMailId() );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_BE_SIGNUP );
             return FALSE;
           }
           //是否重名
           pFighter = FindTeamByName( pSignup->szName );
           if( pFighter )
           {
             pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
               MATCH_ERROR_NAME_EXIST );
             return FALSE;
           }
         }
         break;
       default:
         {
           pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
             MATCH_ERROR_SIGNUP_ILLEGAL );
           return FALSE;
         }
         break;
       }
     }
     break;
   case MATCH_MODE_LIVEMODE:      //生存赛
     {
       //check money
       if( pPlayer->GetMoney() < MATCH_MONEY_COST )
       {
         pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
           MATCH_ERROR_MONEY );
         
         return FALSE;
       }
       //检查是否报过名
       LPSChallenger pFighter = FindbyID( pPlayer->GetMailId(), MATCH_TYPE_LIVEMODE );
       if( pFighter )
       {
         pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
           MATCH_ERROR_BE_SIGNUP );
         return FALSE;
       }
       if( GetSignupNum( MATCH_TYPE_LIVEMODE ) >= SIGNUP_LIVEMODE_NUM_MAX )
       {
         pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
           MATCH_ERROR_NAMELIST_FUUL );
         return FALSE;
       }
     }
     break;
  }
  return TRUE;
}

BOOL CFightFieldMgr::AddToNameList( CPlayer* pPlayer, SMsgData * pTheMsg )
{
  LPSSignup pSignup = (LPSSignup)pTheMsg->Msgs[0].Data;
  int iMatchType = pSignup->wMatchType;
  
  SChallenger* psFighter = new SChallenger;
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
    {
      pPlayer->SetMoney( pPlayer->GetMoney() - MATCH_MONEY_COST );
      //
      psFighter->sNameTip.iId = pPlayer->GetMailId();
      memcpy( psFighter->sNameTip.szName, pPlayer->GetName(), MAX_PLAYER_NAME_LEN ); //人名
      psFighter->sNameTip.szName[MAX_PLAYER_NAME_LEN-1] = '\0';
      //m_Fighters[pSignup->wMatchType].push_back( psFighter );
      GetSignupList( pSignup->wMatchType ).push_back( psFighter );
      SetUpdateTime( GetUpdateTime() + 1 );
      pPlayer->SendOccuMatchSignUP();
    }
    break;
  case MATCH_MODE_WEIGHT:        //量级赛
  case MATCH_MODE_LIVEMODE:      //生存赛
    {
      pPlayer->SetMoney( pPlayer->GetMoney() - MATCH_MONEY_COST );
      //
      psFighter->sNameTip.iId = pPlayer->GetMailId();
      memcpy( psFighter->sNameTip.szName, pPlayer->GetName(), MAX_PLAYER_NAME_LEN ); //人名
      psFighter->sNameTip.szName[MAX_PLAYER_NAME_LEN-1] = '\0';
      //m_Fighters[pSignup->wMatchType].push_back( psFighter );
      GetSignupList( pSignup->wMatchType ).push_back( psFighter );
      SetUpdateTime( GetUpdateTime() + 1 );
    }
    break;
  case MATCH_MODE_TEAM:          //团体赛
    {
      //
      CTeam* pTeam = pPlayer->GetTeam();
      psFighter->sNameTip.iId = pTeam->GetTeamID();
      memcpy( psFighter->sNameTip.szName, pSignup->szName, MAX_PLAYER_NAME_LEN ); //队伍名
      psFighter->sNameTip.szName[MAX_PLAYER_NAME_LEN-1] = '\0';
      int i = 0;
      WORD wCount = 0;
      DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
      switch( pSignup->wMatchType )
      {
      case MATCH_TYPE_2VS2:
        pPlayer->SetMoney( pPlayer->GetMoney() - MATCH_MONEY_COST*2 );
        for(i=0; i<2; i++)
        {
          psFighter->iTeamMailId[i] = pMailIdList[i];
        }
        break;
      case MATCH_TYPE_4VS4:
        pPlayer->SetMoney( pPlayer->GetMoney() - MATCH_MONEY_COST*4 );
        for(i=0; i<4; i++)
        {
          psFighter->iTeamMailId[i] = pMailIdList[i];
        }
        break;
      case MATCH_TYPE_6VS6:
        pPlayer->SetMoney( pPlayer->GetMoney() - MATCH_MONEY_COST*6 );
        for(i=0; i<6; i++)
        {
          psFighter->iTeamMailId[i] = pMailIdList[i];
        }
        break;
      }
#ifdef ELYSIUM_3_7_VERSION
      pPlayer->SendTeamMatchSignUp( psFighter, pSignup->wMatchType );
#endif
      //m_Fighters[pSignup->wMatchType].push_back( psFighter );
      GetSignupList( pSignup->wMatchType ).push_back( psFighter );
      SetUpdateTime( GetUpdateTime() + 1 );
    }
    break;
  default:
    {
      pPlayer->AddSendErrorMsg( pTheMsg, ERROR_CODE_MATCH, 
        MATCH_ERROR_SIGNUP_ILLEGAL );
      return FALSE;
    }
    break;
  }
  //success
  pSignup->dwUpdateCount = GetUpdateTime();
  pPlayer->AddSendMsg( pTheMsg );
  pSignup->iMoney = pPlayer->GetMoney();
  return TRUE;
}

void CFightFieldMgr::ClearNameList()
{
  for( int i = 0; i< 4; i++ )
  {
    for( NAME_ITER iter = m_Fighters[i].begin(); 
    iter != m_Fighters[i].end(); iter++ )
    {
      SAFE_DELETE((*iter));
    }
    m_Fighters[i].clear();
  }
  //
  InitFight();
}

LPSChallenger CFightFieldMgr::FindbyID( int iId,int iType )
{
  if( iId == 0 ) return NULL;
  int iMatchType = GetMatchType();
  if( iType != -1 ) iMatchType = iType;
  for( NAME_ITER iter = m_Fighters[iMatchType].begin(); 
  iter != m_Fighters[iMatchType].end(); iter++ )
  {
    if( (*iter)->sNameTip.iId == iId ) return (*iter);
  }
  return NULL;
}

LPSChallenger CFightFieldMgr::FindTeamMem( int iMailId )
{
  for( int iMatchType = MATCH_TYPE_2VS2; 
  iMatchType < MATCH_TYPE_XVSX_MAX;
  iMatchType++ )
  {
    for( NAME_ITER iter = m_Fighters[iMatchType].begin(); 
    iter != m_Fighters[iMatchType].end(); iter++ )
    {
      switch( iMatchType )
      {
      case MATCH_TYPE_2VS2:
        {
          for( int i = 0; i<2; i++ )
          {
            if( (*iter)->iTeamMailId[i] == iMailId )
            {
              return (*iter);
            }
          }
        }
        break;
      case MATCH_TYPE_4VS4:
        {
          for( int i = 0; i<4; i++ )
          {
            if( (*iter)->iTeamMailId[i] == iMailId )
            {
              return (*iter);
            }
          }
        }
        break;
      case MATCH_TYPE_6VS6:
        {
          for( int i = 0; i<6; i++ )
          {
            if( (*iter)->iTeamMailId[i] == iMailId )
            {
              return (*iter);
            }
          }
        }
        break;
      }//switch( iMatchType )
    } 
  }
  return NULL;
}

LPSChallenger CFightFieldMgr::FindTeamByName( const char* szName )
{
  for( int iMatchType = MATCH_TYPE_2VS2; 
  iMatchType < MATCH_TYPE_XVSX_MAX;
  iMatchType++ )
  {
    for( NAME_ITER iter = m_Fighters[iMatchType].begin(); 
    iter != m_Fighters[iMatchType].end(); iter++ )
    {
      if( strcmp( (*iter)->sNameTip.szName, szName ) == 0 )
      {
        return (*iter);
      }
    }
  }
  return NULL;
}


//GM输入比赛开始的指令
BOOL CFightFieldMgr::DeclareFight( SMsgData * pTheMsg, CPlayer* pPlayer )                                  
{ 
  SNMGMInstruction* pInstr = (SNMGMInstruction*)pTheMsg->Msgs[0].Data;
  int iMatchType  = pInstr->wValue[0];
  int iBeforeTime = pInstr->wValue[1];
  //如果已有指令在执行,则无效
  if( GetRestTime() > 0 || iBeforeTime <= 0 )
  {
    return FALSE;
  }
  //如果不是报名模式的范围,则无效
  int iMatchMode = GetMatchMode();
  if( iMatchMode <= MATCH_MODE_NONE ||
    iMatchMode >= MATCH_MODE_MAX )
  {
    return FALSE;
  }
  //检查比赛类型
  switch( GetMatchMode() ) {
  case MATCH_MODE_OCCUPATION:
    if( iMatchType >= MATCH_TYPE_OCCU_MAX ) return FALSE;
    break;
  case MATCH_MODE_WEIGHT:
    if( iMatchType >= MATCH_TYPE_WEIGHT_MAX ) return FALSE;
    break;
  case MATCH_MODE_TEAM:
    if( iMatchType >= MATCH_TYPE_XVSX_MAX ) return FALSE;
    break;
  case MATCH_MODE_LIVEMODE:
    if( iMatchType >= MATCH_TYPE_LIVEMODE_MAX ) return FALSE;
    break;
  default:
    return FALSE;
  }
  //
  //如果没有人报名或者只有一人报名
  int iCount = GetSignupNum( iMatchType );
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION:
  case MATCH_MODE_WEIGHT:
  case MATCH_MODE_TEAM:
    {     
      if( iCount == 0 || iCount == 1 ) return FALSE;
      int iMatchCount = iCount/TOTAL_MATCH_TIMES;
      int iRestFighterCount = iCount%TOTAL_MATCH_TIMES;
      if( iRestFighterCount > 0 ) iMatchCount += 1; //如果人数不满一场的完整人数则加一场
      //设置每场比赛的人数
      SetFighterCount( (iCount+1)/iMatchCount );
      //设置一次比赛的总局数
      SetMatchId( 0 );
      SetMatchCount( iMatchCount );
    }
    break;
  case MATCH_MODE_LIVEMODE:
    {
      if( iCount < LIVEMODE_PLAYER_NUM_MIN ) return FALSE;
      //设置每场比赛的人数
      SetFighterCount( iCount );
      //设置一次比赛的总局数
      SetMatchId( 0 );
      SetMatchCount( 1 );
    }
    break;
  }
  //
  InitLayer();
  InitFight();
  //
  SetAction( MATCH_ACTION_BEGIN );
  //SetMatchMode( iMatchMode );
  SetMatchType( iMatchType );
  //
  SetMatchState( MATCH_STATE_BEFORE );  //比武大会开始之前
  SetRestTime( iBeforeTime*60 );
  //
  SetFightTimeFlag( TRUE );
  SetAfterFight( FALSE );
  //
  SetLastTickCount( GetTickCount() );
  SetUpdateTime( 1 );
  //比武大会预备时间通知MCC
  SendMcc_AP_BEFORE_ALL_MATCH( GetRestTime() );
  return TRUE;
}

//做整个流程控制
void CFightFieldMgr::DoAction()
{
  //获取剩余时间
  BOOL bEnd = FALSE;
  int iRestTime = GetRestTime();
  if( iRestTime <= 0 ) return;
  
  //获得起始时间
  DWORD dwLastTime = GetLastTickCount(), dwCurTime = GetTickCount();
  if( (dwCurTime - dwLastTime) >= 1000 )
  {
    SetRestTime( iRestTime-1 );
    SetLastTickCount( dwCurTime );
    if( iRestTime == 1 ) bEnd = TRUE;
  }
  
  switch( GetAction() )
  {
  case MATCH_ACTION_NONE:
    break;
  case MATCH_ACTION_SIGNUP: //报名
    {
      //check time
      if( bEnd )
      {        
        //通知报名时间结束
        SetSignupTimeFlag( FALSE );
        SendMcc_AP_END_SIGNUP();
        //产生竞赛表
        if( MakeMatchTable() )
        {
          SetQueryTimeFlag( TRUE );
          //send signup end to mcc
          //SendMcc_AP_END_SIGNUP();
        }
        else
        {
          //清除报名名单
          ClearNameList();
          //发送给MCC整个比赛结束
        }
      }
    }
    break;
  case MATCH_ACTION_BEGIN:
    {
      //如果比赛时间到
      if( bEnd )
      {
        switch( GetMatchState() )
        {
          //等待比赛时间结束
        case MATCH_STATE_NONE:
          break;
        case MATCH_STATE_BEFORE: //如果是比赛开始前
          {
            SetMatchState( MATCH_STATE_START );  //比武大会开始之前
            SetLastTickCount( dwCurTime );
          }
          break;
        case MATCH_STATE_RUNNING: //比赛中的状态
          {
            //做胜负判断
            JudgeResult( TRUE );
            //结束本次比赛
            SetMatchState( MATCH_STATE_END );
          }
          break;
        }
      }
      //else
      {
        switch( GetMatchState() )
        {
          //整个比赛到这里就结束了
        case MATCH_STATE_NONE:
          break;
          //比赛正式开始前的准备期
        case MATCH_STATE_BEFORE:
          break;
          //比赛开始的状态
        case MATCH_STATE_START:
          //送选手到赛场
          SendFighterToField();
          //开始比赛计时间
          SetRestTime( GetFieldTime() );
          //发送参赛名单
          switch( GetMatchState() )
          {
          case MATCH_STATE_RUNNING:
            SendMcc_AP_MATCH_BEGIN();
            break;
          case MATCH_STATE_START:
            //整理每局的比赛的结果
            EndMatch();
            //发送结果名单
            SendMcc_AP_MATCH_END();
            break;
          }
          break;
          //比赛中的状态
          case MATCH_STATE_RUNNING:
            //做胜负判断
            JudgeResult( FALSE );
            break;
            //每局的比赛结束的状态
          case MATCH_STATE_END:
            //整理每局的比赛的结果
            EndMatch();
            //发送结果名单
            SendMcc_AP_MATCH_END();
            break;
        }//switch( GetMatchState() )
      }
    }
    break;
  }
}

//产生竞赛表
BOOL CFightFieldMgr::MakeMatchTable()
{
  BOOL bNoMatch = FALSE;
  WORD wSignupNotice[MATCH_TYPE_MAX];
  memset( wSignupNotice, 0, sizeof(WORD)*MATCH_TYPE_MAX );
  //
  if( GetLayer() == 1 )
  {
    //初始化竞赛表格
    //InitFight();
    //计算出本次比赛的一个比赛类型的总场数和人数
    int iMatchTypeCount = 0;
    switch( GetMatchMode() )
    {
    case MATCH_MODE_OCCUPATION:    //职业赛
      iMatchTypeCount = MATCH_TYPE_OCCU_MAX;
      break;
    case MATCH_MODE_WEIGHT:        //量级赛
      iMatchTypeCount = MATCH_TYPE_WEIGHT_MAX;
      break;
    case MATCH_MODE_TEAM:          //团体赛
      iMatchTypeCount = MATCH_TYPE_XVSX_MAX;
      break;
    case MATCH_MODE_LIVEMODE:
      iMatchTypeCount = MATCH_TYPE_LIVEMODE_MAX;
      break;
    }
    //
    //如果没有人报名或者只有一人报名
    /*
    int iNumJoin = 0;
    for( int iType = 0; iType < iMatchTypeCount; iType++ )
    {
    iNumJoin += GetSignupNum( iType );
    }
    //
    if( iNumJoin <= 1 )
    {
    //退回该玩家的系统佣金
    return FALSE;
  }*/
    
    for( int iType = 0; iType < iMatchTypeCount; iType++ )
    {
      SetMatchType( iType );
      //
      switch( GetMatchMode() )
      {
      case MATCH_MODE_OCCUPATION:    //职业赛
      case MATCH_MODE_WEIGHT:        //量级赛
      case MATCH_MODE_TEAM:          //团体赛
        {      
          int iCount = GetSignupNum( iType );
          if( iCount == 0 || iCount == 1 )
          {
            if( iCount == 1 )
            {
              //退回该玩家的系统佣金
              HandleBackMoney( iType );
              bNoMatch = TRUE;
              wSignupNotice[iType] = 1;
            }
            continue;
          }
          int iMatchCount = iCount/TOTAL_MATCH_TIMES;
          int iRestFighterCount = iCount%TOTAL_MATCH_TIMES;
          if( iRestFighterCount > 0 ) iMatchCount += 1; //如果人数不满一场的完整人数则加一场
          //设置每场比赛的人数
          SetFighterCount( (iCount+1)/iMatchCount );
          //设置一次比赛的总局数
          SetMatchCount( iMatchCount );
          //填充竞赛表单
          NAME_LIST NameList = /*GetSignupList*/GetRandomList( iType );
          int iCurMatchId = 0;
          //按8个人的阵形排列
          for( NAME_ITER iter = NameList.begin(); 
          iter != NameList.end(); /*iter++*/ )
          {
            //如果不是最后一场
            if( iCurMatchId < iMatchCount-1 )
            {
              if( GetFighterCount() <= TOTAL_MATCH_TIMES/2 )
              {
                int iFighter1 = GetFighterCount();
                if( iFighter1 > 0 ) SetFightArray( iter, iCurMatchId, 0, iFighter1 );
              }
              else
              {
                int iFighter1 = GetFighterCount()/2;
                if( iFighter1 > 0 ) SetFightArray( iter, iCurMatchId, 0, iFighter1 );
                int iFighter2 = GetFighterCount() - iFighter1;
                if( iFighter2 > 0 ) SetFightArray( iter, iCurMatchId, 1, iFighter2 );
              }
              iCurMatchId++;
            }
            else
            {
              //最后一场的人数
              int iFighters = iCount - ( iMatchCount - 1 ) * GetFighterCount();
              if( iFighters <= TOTAL_MATCH_TIMES/2 )
              {
                int iFighter1 = iFighters;
                if( iFighter1 > 0 ) SetFightArray( iter, iCurMatchId, 0, iFighter1 );
              }
              else
              {
                int iFighter1 = iFighters/2;
                if( iFighter1 > 0 ) SetFightArray( iter, iCurMatchId, 0, iFighter1 );
                int iFighter2 = iFighters - iFighter1;
                if( iFighter2 > 0 ) SetFightArray( iter, iCurMatchId, 1, iFighter2 );
              }
            }
          }
          //
        }
        break;
      case MATCH_MODE_LIVEMODE:
        {
          int iCount = GetSignupNum( iType );
          if( iCount < LIVEMODE_PLAYER_NUM_MIN+1 )
          {
            //退回该玩家的系统佣金
            HandleBackMoney( iType );
            bNoMatch = TRUE;
            wSignupNotice[iType] = 1;
            continue;
          }
          SetFighterCount( iCount );
          SetMatchCount( 1 );
        }
        break;
      }
    }
    //
    SetMatchType( -1 );
    //
    if( bNoMatch )
    {
    /*
    SMsgData  *pNewMsg = g_pGs->NewMsgBuffer();
    if( pNewMsg )
    {
    pNewMsg->Init();
    pNewMsg->dwAID        = A_NOMATCH_NOTICE;
    pNewMsg->dwMsgLen     = 1;
    pNewMsg->Msgs[0].Size = sizeof(WORD) + sizeof(wSignupNotice);
    WORD *pData = (WORD*)pNewMsg->Msgs[0].Data;
    pData[0] = GetMatchMode();
    memcpy( &pData[1], wSignupNotice, sizeof(wSignupNotice) );
    g_pGs->SendTheMsgToAll( pNewMsg );
    }
      */
      SMccMsgData     *pNewMccMsg = NULL;
      if( ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )
      {
        pNewMccMsg->Init( NULL );
        pNewMccMsg->dwAID        = AP_NOMATCH_NOTICE;
        pNewMccMsg->dwMsgLen     = 1;
        pNewMccMsg->Msgs[0].Size = sizeof(WORD) + sizeof(wSignupNotice);
        WORD *pData = (WORD*)pNewMccMsg->Msgs[0].Data;
        pData[0] = GetMatchMode();
        memcpy( &pData[1], wSignupNotice, sizeof(wSignupNotice) );
        //
        g_pMccDB->AddSendMsg( pNewMccMsg );
      }
    }
  }
  return TRUE;
}

//按8个人的阵形排列
void CFightFieldMgr::SetFightArray( LPCFighter* pWinner, int iFighterCount )
{
  int icount = 0;
  switch( iFighterCount )
  {
  case 2:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 2 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 3 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 4 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 5 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 6 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 7 );
    break;
  case 3:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    SetMatchWaitPos( pWinner[icount+2], (LPCFighter)NULL, 2 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 3 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 4 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 5 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 6 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 7 );
    break;
  case 4:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    
    SetMatchWaitPos( pWinner[icount+2], pWinner[icount+3], 2 );
    SetMatchWaitPos( pWinner[icount+3], pWinner[icount+2], 3 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 4 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 5 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 6 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 7 );
    break;
  case 5:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    SetMatchWaitPos( pWinner[icount+2], (LPCFighter)NULL, 2 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 3 );
    
    SetMatchWaitPos( pWinner[icount+3], pWinner[icount+4], 4 );
    SetMatchWaitPos( pWinner[icount+4], pWinner[icount+3], 5 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 6 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 7 );
    break;
  case 6:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    SetMatchWaitPos( pWinner[icount+2], (LPCFighter)NULL, 2 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 3 );
    
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 4 );
    SetMatchWaitPos( pWinner[icount+3], (LPCFighter)NULL, 5 );
    
    SetMatchWaitPos( pWinner[icount+4], pWinner[icount+5], 6 );
    SetMatchWaitPos( pWinner[icount+5], pWinner[icount+4], 7 );
    break;
  case 7:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    SetMatchWaitPos( pWinner[icount+2], pWinner[icount+3], 2 );
    SetMatchWaitPos( pWinner[icount+3], pWinner[icount+2], 3 );
    
    SetMatchWaitPos( pWinner[icount+4], pWinner[icount+5], 4 );
    SetMatchWaitPos( pWinner[icount+5], pWinner[icount+4], 5 );
    
    SetMatchWaitPos( pWinner[icount+6], (LPCFighter)NULL, 6 );
    SetMatchWaitPos( (LPCFighter)NULL, (LPCFighter)NULL, 7 );
    break;
  case 8:
    SetMatchWaitPos( pWinner[icount+0], pWinner[icount+1], 0 );
    SetMatchWaitPos( pWinner[icount+1], pWinner[icount+0], 1 );
    
    SetMatchWaitPos( pWinner[icount+2], pWinner[icount+3], 2 );
    SetMatchWaitPos( pWinner[icount+3], pWinner[icount+2], 3 );
    
    SetMatchWaitPos( pWinner[icount+4], pWinner[icount+5], 4 );
    SetMatchWaitPos( pWinner[icount+5], pWinner[icount+4], 5 );
    
    SetMatchWaitPos( pWinner[icount+6], pWinner[icount+7], 6 );
    SetMatchWaitPos( pWinner[icount+7], pWinner[icount+6], 7 );
    break;
  }
}

void CFightFieldMgr::SetFightArray( NAME_ITER& iter,     
                                   int iMatchId,       //局数
                                   int iSide,          //阵列数
                                   int iFighterCount ) //当前阵列的总玩家数
{
  int iSizeCount = TOTAL_MATCH_TIMES/2;
  switch( iFighterCount )
  {
  case 2:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 2 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 3 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 4 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 5 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 6 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 7 );
    break;
  case 3:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 2 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 3 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 4 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 5 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 6 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 7 );
    break;
  case 4:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 2 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 3 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 4 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 5 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 6 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 7 );
    break;
  case 5:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 2 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 3 );
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 4 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 5 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 6 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 7 );
    break;
  case 6:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 2 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 3 );
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 4 );
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 5 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 6 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 7 );
    iter++;
    break;
  case 7:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 2 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 3 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 4 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 5 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 6 );
    iter++;
    SetMatchWaitPos( (LPSChallenger)NULL, iMatchId, iSide*iSizeCount + 7 );
    break;
  case 8:
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 0 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 1 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 2 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 3 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 4 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 5 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 6 );
    iter++;
    SetMatchWaitPos( (*iter), iMatchId, iSide*iSizeCount + 7 );
    iter++;
    break;
  }
}

//送选手到赛场时才会产生选手的资料
void CFightFieldMgr::SendFighterToField()
{ 
  //向CLIENT发送比赛情况(谁VS谁)
  LPCFighter pFightInfo1 = NULL, pFightInfo2 = NULL;
  BOOL bNoMatch = TRUE;
  int  iFieldCount = GetFieldCount();  //可以派送去的竞技场个数
  int  iSendCount  = GetSendCount();   //已经送出的选手数
  SetPrevSendCount( iSendCount );
  int  iFighter1 = 0, iFighter2 = 0; //保存玩家的Id号
  BOOL bWaiting1 = FALSE, bWaiting2 = FALSE; 
  SWarpPoint *pWarp1 = NULL, *pWarp2 = NULL;
  LPSMapWarp pFieldInfo = NULL;
  //初始化比赛结果
  InitMatchResult();
  //如果为初赛
  if( GetLayer() == 1 )
  {
    switch( GetMatchMode() )
    {
    case MATCH_MODE_OCCUPATION:    //职业赛
    case MATCH_MODE_WEIGHT:        //量级赛
    case MATCH_MODE_TEAM:          //团体赛
      {
        LPSNameTip pFighterTip1 = NULL, pFighterTip2 = NULL;
        int  iCurMatchId = GetMatchId();     //当前的每局
        if( IsAllSend( iSendCount ) == FALSE/*iSendCount < TOTAL_MATCH_TIMES*/ )
        {
          for( int i = 1; i< iFieldCount; i++ )
          {
            //get map info
            pFieldInfo = GetFieldInfo( i );
            //随机获取Warp点
            pWarp1  = pFieldInfo->GetSide( 0 );
            pWarp2  = pFieldInfo->GetSide( 1 );
            //查看是否在休息室
            pFighterTip1 = GetFighterTip( iCurMatchId, iSendCount + 0 );
            iFighter1 = 0;
            if( pFighterTip1 ) iFighter1 = pFighterTip1->iId;
            bWaiting1 = CheckWaitting( iFighter1 );
            //查看是否在休息室
            pFighterTip2 = GetFighterTip( iCurMatchId, iSendCount + 1 );
            iFighter2 = 0;
            if( pFighterTip2 ) iFighter2 = pFighterTip2->iId;
            bWaiting2 = CheckWaitting( iFighter2 );
            //指定对手
            if( iFighter1 )
            {
              if( iFighter2 ) SetFightInfo( iSendCount +0 , GetFighter(iSendCount +1) );
              else            SetFightInfo( iSendCount +0, NULL );
              //
              GetFightResult( iSendCount+0 )->SetInMap( bWaiting1 );
            }
            if( iFighter2 )
            {
              if( iFighter1 )  SetFightInfo( iSendCount +1 , GetFighter(iSendCount +0) );
              else             SetFightInfo( iSendCount +1, NULL );
              GetFightResult( iSendCount+1 )->SetInMap( bWaiting2 );
            }
            //做比赛判定规则
            //如果都在则有一场比赛
            if( bWaiting1 && bWaiting2 )
            {
              bNoMatch = FALSE;
              //将1,2号选手传入到赛场
              Warp( iFighter1, pWarp1, iSendCount+0, PSWITCH_CITYWAR_ATTACKER );
              Warp( iFighter2, pWarp2, iSendCount+1, PSWITCH_CITYWAR_DEFENCER );
              //有一场比赛
              //i++;
            }
            //
            iSendCount += 2;
            SetSendCount( iSendCount );
            
            //输出比赛结果比赛结果
            MakeMatchResult( i, 
              pFighterTip1, pFighterTip2,
              bWaiting1, bWaiting2 );
            
            //如果所有的选手都传过了
            if( IsAllSend( iSendCount ) == TRUE /*iSendCount >= TOTAL_MATCH_TIMES*/ ) break;
          }//for( int i = 0; i< iFieldCount; i++ )
          //先检查一次结果
          JudgeResult( FALSE );
          //只要有比赛
          if( bNoMatch == FALSE )
          {
            //切到比赛中的状态
            SetMatchState( MATCH_STATE_RUNNING );
          }
          else
          {
            SetNoMatch( TRUE );
            if( IsAllSend( iSendCount ) == FALSE/*iSendCount < TOTAL_MATCH_TIMES*/ )
            {
              //切到开始比赛的状态
              SetMatchState( MATCH_STATE_START );
            }
            else
            {
              SetMatchState( MATCH_STATE_END );  
            }
          }//if( bNoMatch == FALSE )
        }
        //如果送完选手
        else
        {
          SetMatchState( MATCH_STATE_END );
        }
      }
      break;
    case MATCH_MODE_LIVEMODE:      //生存赛
      {
        //SWarpPoint* pWarp1 = NULL;
        //BOOL bNoMatch = TRUE;
        int  iFighter = 0;
        BOOL bWaiting = FALSE;
        int  iCount = GetSignupNum( MATCH_TYPE_LIVEMODE );
        //
        if( iCount < LIVEMODE_PLAYER_NUM_MIN+1 )
        {
          //抱人数不够的错误
          SetMatchState( MATCH_STATE_NONE );
          SetRestTime( 0 );
          return;
        }
        //
        m_pResultTable[0] = &m_Records[0];
        m_pResultTable[0]->SetJoin( TRUE );
        //
        pFieldInfo = GetFieldInfo( 0 );
        NAME_LIST& nameList = GetSignupList( MATCH_TYPE_LIVEMODE );
        for( NAME_ITER iter = nameList.begin(); 
        iter != nameList.end(); iter++ )
        {             
          //随机获取Warp点
          pWarp1  = pFieldInfo->GetPoint();
          iFighter = (*iter)->sNameTip.iId;
          bWaiting = CheckWaitting( iFighter );
          if( bWaiting )
          {
            //get map info
            //获取生存赛的地图Id
            Warp( iFighter, pWarp1, 0 );
          }
        }
        //
        LPSMatchResult pMatchResult = GetMatchResult( 0 );
        pMatchResult->Clear();
        pMatchResult->iResult = RESULT_ALLJOIN_VS;
        //切到比赛中的状态
        SetMatchState( MATCH_STATE_RUNNING );
        //
        SetLastCount( iCount );
        CreateMonster();
      }
      break;
   }
  }
  //如果不为初赛
  //并且只能是职业赛,量级赛,团体赛
  else
  {
    if( IsAllSend( iSendCount ) == FALSE/*iSendCount < GetBoutsByLayer()*/ )
    {
      LPSNameTip pFighterTip1 = NULL, pFighterTip2 = NULL;
      for( int i = 1; i< iFieldCount; i++ )
      {
        //get map info
        pFieldInfo = GetFieldInfo( i );
        pWarp1  = pFieldInfo->GetSide( 0 );
        pWarp2  = pFieldInfo->GetSide( 1 );
        //
        iFighter1 = 0;
        pFightInfo1 = GetFightResult( iSendCount+0 );
        pFightInfo2 = GetFightResult( iSendCount+1 );
        if( pFightInfo1 )
        {
          pFighterTip1 = &pFightInfo1->GetSelfInfo()->sNameTip;
          iFighter1 = pFighterTip1->iId;
        }
        else
        {
          pFighterTip1 = NULL;
          iFighter1 = 0;
        }
        //查看是否在休息室
        bWaiting1 = CheckWaitting( iFighter1 );
        iFighter2 = 0;
        if( pFightInfo2 )
        {
          pFighterTip2 = &pFightInfo2->GetSelfInfo()->sNameTip;
          iFighter2 = pFighterTip2->iId;
        }
        else
        {
          pFighterTip2 = NULL;
          iFighter2 = 0;
        }
        //查看是否在休息室
        bWaiting2 = CheckWaitting( iFighter2 );
        //指定对手
        if( iFighter1 )
        {
          //加入了比赛
          pFightInfo1->SetJoin( TRUE );
          pFightInfo1->SetNum( 0 );
          if( iFighter2 ) pFightInfo1->SetEnemy( pFightInfo2 );
          else            pFightInfo1->SetEnemy( NULL );
          pFightInfo1->SetInMap( bWaiting1 );
        }
        if( iFighter2 )
        {
          //加入了比赛
          pFightInfo2->SetJoin( TRUE );
          pFightInfo2->SetNum( 0 );
          if( iFighter1 )   pFightInfo2->SetEnemy( pFightInfo1 );
          else              pFightInfo2->SetEnemy( NULL );
          pFightInfo2->SetInMap( bWaiting2 );
        }
        //做比赛判定规则
        //如果都在则有一场比赛
        if( bWaiting1 && bWaiting2 )
        {
          bNoMatch = FALSE;
          //将1,2号选手传入到赛场
          Warp( iFighter1, pWarp1, iSendCount+0, PSWITCH_CITYWAR_ATTACKER );
          Warp( iFighter2, pWarp2, iSendCount+1, PSWITCH_CITYWAR_DEFENCER );
          //有一场比赛
          //i++;
        }
        //
        iSendCount += 2;
        SetSendCount( iSendCount );
        
        //输出比赛结果比赛结果
        MakeMatchResult( i, 
          pFighterTip1, pFighterTip2,
          bWaiting1, bWaiting2 );
        
        //
        //如果所有的选手都传过了
        if( IsAllSend( iSendCount ) == TRUE/*iSendCount >= GetBoutsByLayer()*/ ) break;
      }//for( int i = 0; i< iFieldCount; i++ )
      //先检查一次结果
      JudgeResult( FALSE );
      //只要有比赛
      if( bNoMatch == FALSE )
      {
        //切到比赛中的状态
        SetMatchState( MATCH_STATE_RUNNING );
      }
      else
      {
        SetNoMatch( TRUE );
        if( IsAllSend( iSendCount ) == FALSE/*iSendCount < GetBoutsByLayer()*/ )
        {
          //切到开始比赛的状态
          SetMatchState( MATCH_STATE_START );
        }
        else
        {
          SetMatchState( MATCH_STATE_END );
        }
      }//if( bNoMatch == FALSE )
    }
    //如果送完选手
    else
    {
      SetMatchState( MATCH_STATE_END );
    }
  }
}

//查看选手是否在休息室等待
BOOL CFightFieldMgr::CheckWaitting( int& iId ) 
{
  if( iId == 0 ) return FALSE;
  //休息室的地图ID
  int iMapId = GetLobbyMapId();
  //
  CPlayer *pPlayer = NULL;
  CTeam   *pTeam   = NULL;
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
  case MATCH_MODE_WEIGHT:        //量级赛
  case MATCH_MODE_LIVEMODE:      //生存赛
    if( pPlayer = g_pGs->GetPlayerFromMailId( iId ) )
    {
      if( pPlayer->GetMapId() != iMapId )
      {
        //人物不在休息室，抱错
        return FALSE;
      }
      return TRUE;
    }
    return FALSE;
  case MATCH_MODE_TEAM:         //团体赛
    {  
      //允许部分选手参赛      
      LPSChallenger pFighter = FindbyID( iId );
      if( pFighter == NULL ) return FALSE;
      //
      if( NULL == ( pTeam = ::GetTeam( iId ) ) )
      {
        //获得队长的MailId
        int iLeaderMailId = pFighter->iTeamMailId[0];
        CPlayer* pTeamLeader = g_pGs->GetPlayerFromMailId( iLeaderMailId );
        if( NULL == pTeamLeader ) 
        {
          //队长不在线的错误
          return FALSE;
        }
        pTeam = pTeamLeader->GetTeam();
        if( NULL == pTeam )
        {
          //队长没有组队的错误
          return FALSE;
        }
        //获得新的队伍ID
        iId = pTeam->GetTeamID();
        //check team member count
        int iMemCount = pTeam->GetMembersNum();
        switch( GetMatchType() )
        {
        case MATCH_TYPE_2VS2: //2对2
          if( iMemCount > 2 )
          {
            //抱队伍人数过多的错误
            return FALSE;
          }
          break;
        case MATCH_TYPE_4VS4: //4对4
          if( iMemCount > 4 )
          {
            //抱队伍人数过多的错误
            return FALSE;
          }
          break;
        case MATCH_TYPE_6VS6: //6对6
          if( iMemCount > 6 )
          {
            //抱队伍人数过多的错误
            return FALSE;
          }
          break;
        }
      }
      //
      BOOL bHaveMem = FALSE, bFind = FALSE;
      int iOldMemCount = 0, iNewMemCount = 0;
      int iOldMemMailId = 0, iNewMemMailId = 0;
      //计算原队伍人数
      for( int i=0; i< TEAM_MEMBER_COUNT_MAX; i++ )
      {
        if( pFighter->iTeamMailId[i] ) iOldMemCount++;
      }
      //检查队伍状态
      iNewMemCount = pTeam->GetMembersNum();
      WORD wCount;
      DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
      for( int n = 0; n< iNewMemCount; n++ )
      {
        bFind = FALSE;
        iNewMemMailId = pMailIdList[n];
        //查看玩家是否在线
        pPlayer = g_pGs->GetPlayerFromMailId( iNewMemMailId );
        if( pPlayer && pPlayer->GetMapId() == iMapId ) bHaveMem = TRUE;
        //
        for( int m =0; m<iOldMemCount; m++ )
        {
          if( iNewMemMailId == pFighter->iTeamMailId[m] )
          {
            bFind = TRUE;
            break;
          }
        }
        //抱队伍状态于开始不符的错误
        if( bFind == FALSE ) return FALSE;
      }
      //
      if( bHaveMem == FALSE )
      {
        //报没有队员在休息室的错误
        return FALSE;
      }
      //队伍ID改为新的ID
      pFighter->sNameTip.iId = iId;
    }
    break;
  }
  return TRUE;
}

//NPC用来做单个的报名检查
int CFightFieldMgr::CheckFighter( CPlayer* pPlayer )
{
  if( pPlayer == NULL ) return 0;
  int  iMailId = pPlayer->GetMailId();
  BOOL bFind =  FALSE;
  for( int iType = 0; iType < MATCH_TYPE_MAX; iType++ )
  {
    NAME_LIST& nameList = GetSignupList( iType );
    for( NAME_ITER iter = nameList.begin(); 
    iter != nameList.end(); iter++ )
    {
      if( (*iter)->sNameTip.iId == iMailId )
      {
        bFind = TRUE;
        break;
      }
    }
    if( bFind ) break;
  }
  //
  if( bFind ) return 1;
  else        return 0;
}

//NPC用来做队伍的报名检查
int  CFightFieldMgr::CheckTeam( CPlayer* pPlayer )
{  
  if( pPlayer == NULL ) return 0;
  int iTeamId = 0, iMailId = pPlayer->GetMailId();
  //允许部分选手参赛
  CTeam* pTeam = pPlayer->GetTeam();
  if( pTeam == NULL )  return 0;
  iTeamId = pTeam->GetTeamID();
  //查找是否是已经报名的队伍
  LPSChallenger pFighter = NULL;
  BOOL bFind = FALSE;
  for( int iType = 0; iType < MATCH_TYPE_XVSX_MAX; iType++ )
  {
    NAME_LIST& nameList = GetSignupList( iType );
    for( NAME_ITER iter = nameList.begin(); 
    iter != nameList.end(); iter++ )
    {
      for( int iMemCount = 0; iMemCount < TEAM_MEMBER_COUNT_MAX; iMemCount++ )
      {
        if( (*iter)->iTeamMailId[iMemCount] )
        {
          if( (*iter)->iTeamMailId[iMemCount] == iMailId )
          {
            bFind = TRUE;
            pFighter = (*iter);
            break;
          }
        }
        else break;
      }//for( int iMemCount = 0;
      if( bFind ) break;
    }
    if( bFind ) break;
  }
  //是没有抱过名的队伍
  if( bFind == FALSE ) return 0;
  //获得队长的MailId
  int iLeaderMailId = pFighter->iTeamMailId[0];
  CPlayer* pTeamLeader = g_pGs->GetPlayerFromMailId( iLeaderMailId );
  if( NULL == pTeamLeader ) 
  {
    //队长不在线的错误
    return 0;
  }
  pTeam = pTeamLeader->GetTeam();
  if( NULL == pTeam )
  {
    //队长没有组队的错误
    return 0;
  }
  //check team member count
  int iMemCount = pTeam->GetMembersNum();
  switch( GetMatchType() )
  {
  case MATCH_TYPE_2VS2: //2对2
    if( iMemCount > 2 )
    {
      //抱队伍人数过多的错误
      return 0;
    }
    break;
  case MATCH_TYPE_4VS4: //4对4
    if( iMemCount > 4 )
    {
      //抱队伍人数过多的错误
      return 0;
    }
    break;
  case MATCH_TYPE_6VS6: //6对6
    if( iMemCount > 6 )
    {
      //抱队伍人数过多的错误
      return 0;
    }
    break;
  }
  //
  BOOL bHaveMem = FALSE;
  int iOldMemCount = 0, iNewMemCount = 0;
  int iOldMemMailId = 0, iNewMemMailId = 0;
  //计算原队伍人数
  for( int i=0; i< TEAM_MEMBER_COUNT_MAX; i++ )
  {
    if( pFighter->iTeamMailId[i] ) iOldMemCount++;
  }
  //检查队伍状态
  iNewMemCount = pTeam->GetMembersNum();
  WORD wCount;
  DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
  for( int n = 0; n< iNewMemCount; n++ )
  {
    bHaveMem = FALSE;
    iNewMemMailId = pMailIdList[n];
    //
    for( int m =0; m<iOldMemCount; m++ )
    {
      if( iNewMemMailId == pFighter->iTeamMailId[m] )
      {
        bHaveMem = TRUE;
        break;
      }
    }
    //抱队伍状态于开始不符的错误
    if( bHaveMem == FALSE ) return 0;
  }
  //获得新的队伍ID
  pFighter->sNameTip.iId = iTeamId;
  return 1;
}

void CFightFieldMgr::Warp( int iId, SWarpPoint* pWarp, int iSendCount, DWORD dwSwitch )
{  
  //休息室的地图ID
  int iMapId = GetLobbyMapId();
  //
  CPlayer* pPlayer = NULL;
  CTeam*   pTeam = NULL;
  LPCFighter pFightInfo = NULL;
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
  case MATCH_MODE_WEIGHT:        //量级赛
    {
      if( ( pPlayer = g_pGs->GetPlayerFromMailId( iId ) ) &&
        ( pPlayer->GetMapId() == iMapId ) )
      {
        pFightInfo = GetFightResult( iSendCount );
        pFightInfo->SetNum( pFightInfo->GetNum() + 1 );
        pFightInfo->SetMapId( pWarp->wTargetMapId );
        pPlayer->SetFightInfo( pFightInfo );
        //给玩家分配竞赛信息
        if( dwSwitch )
        {
          pPlayer->m_dwSwitch |= dwSwitch;
          pPlayer->Send_A_SWITCH();
        }
        pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
        Send_A_SHOWTIMER( pPlayer );
      }
    }
    break;
  case MATCH_MODE_LIVEMODE:      //生存赛
    {
      if( ( pPlayer = g_pGs->GetPlayerFromMailId( iId ) ) &&
        ( pPlayer->GetMapId() == iMapId ) )
      {
        pFightInfo = GetFightResult( iSendCount );
        pFightInfo->SetNum( pFightInfo->GetNum() + 1 );
        pFightInfo->SetMapId( pWarp->wTargetMapId );
        pPlayer->SetFightInfo( pFightInfo );
        //给玩家分配竞赛信息
        pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
        Send_A_SHOWTIMER( pPlayer );
      }
    }
    break;
  case MATCH_MODE_TEAM:          //团体赛
    {
      if( pTeam = ::GetTeam( iId ) )
      {
        int iMemCount = pTeam->GetMembersNum();
        WORD wCount;
        DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
        pFightInfo = GetFightResult( iSendCount );
        pFightInfo->SetMapId( pWarp->wTargetMapId );
        for( int i=0; i<iMemCount; i++ )
        {
          pPlayer = pTeam->GetMember( pMailIdList[i] );
          if( pPlayer && pPlayer->GetMapId() == iMapId )
          {
            pFightInfo->SetNum( pFightInfo->GetNum() + 1 );
            pPlayer->SetFightInfo( pFightInfo );
            if( dwSwitch )
            {
              pPlayer->m_dwSwitch |= dwSwitch;
              pPlayer->Send_A_SWITCH();
            }
            //给玩家分配竞赛信息
            pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
            Send_A_SHOWTIMER( pPlayer );
          }
        }//for( int i=0; i<iMemCount; i++ )
      }//if( pTeam = ::GetTeam( iId ) )
    }
    break;
  }
}

//生成每局的比赛的结果作为上一层的对阵列表
//当做结果排序时只能出现一个胜者或没有胜者
int CFightFieldMgr::EndMatch()
{  
  int icount = 0;
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
  case MATCH_MODE_WEIGHT:        //量级赛
  case MATCH_MODE_TEAM:          //团体赛
    {
      //检查是否还有玩家参加这局比赛
      BOOL bAlSend = IsAllSend( GetSendCount() );
      //如果所有的选手都传过了
      if( bAlSend )
      {
        SetSendCount( 0 );
        //
        LPCFighter pWinner = NULL;
        for( int i = 0; i< GetBoutsByLayer(); i+=2 )
        {
          if( m_pResultTable[i+0] )
          {
            icount++;
            pWinner = m_pResultTable[i+0];
          }
          else if( m_pResultTable[i+1] )
          {
            icount++;
            pWinner = m_pResultTable[i+0] = m_pResultTable[i+1];
            m_pResultTable[i+1] = NULL;
          }
        }
        //
        int ipos = 1;
        for( i = 2; i< GetBoutsByLayer(); i+=2 )
        {
          if( m_pResultTable[i] )
          {
            m_pResultTable[ipos] = m_pResultTable[i];
            m_pResultTable[i] = NULL;
            ipos++;
          }
          else
          {
            m_pResultTable[ipos] = NULL;
            ipos++;
          }
        }
        //如果有冠军产生
        if( icount == 1 || icount == 0 )
        {
          if( GetLayer() < 5 )
          {
            SetLayer( 5 );
            if( icount == 1 )
            {              
              //如果没有到总冠军,直接升到总冠军
              if( pWinner->GetWinTime() < 5 )
              {
                pWinner->SetWinTime( 5 );
              }
              m_MatchWinner[GetMatchId()] = *pWinner;
            }
            else
            {
              m_MatchWinner[GetMatchId()].Init();
            }
          }
          //还有比赛则开始另一局
          if( GetMatchId() < GetMatchCount()-1 )
          {          
            SetMatchId( GetMatchId() + 1 );
            //为初赛
            SetLayer( 1 );
            //比赛重新开始
            //SetMatchState( MATCH_STATE_START );
            SetMatchState( MATCH_STATE_BEFORE );
            SetRestTime( GetIntervalTime() );
          }
          //所有局数都结束,则让每局的胜出者再来最后一场比赛
          else if( GetMatchId() == GetMatchCount()-1 )
          {
            //
            int iwincount = 0; //胜出者人数
            if( IsWinnerMatch() == FALSE )
            {
              LPSChallenger pFighter = NULL;
              //LPCFighter pFighterArray[MATCH_AREA_WINNER_MAX];
              memset( m_pResultTable, 0, sizeof(m_pResultTable) );
              //memset( pFighterArray, 0, sizeof(pFighterArray) );
              for( int i = 0; i< MATCH_AREA_WINNER_MAX; i++ )
              {
                pFighter = m_MatchWinner[i].GetSelfInfo();
                if( pFighter && pFighter->sNameTip.iId )
                {
                  m_pResultTable[i] = &m_MatchWinner[i];
                  //pFighterArray[iwincount] = &m_MatchWinner[i];
                  iwincount++;
                }
              }
              //
            }
            //第2次总复赛
            else
            {
              LPCFighter pWinner = NULL;
              for( int i = 0; i< GetBoutsByLayer(); i+=2 )
              {
                if( m_pResultTable[i+0] )
                {
                  iwincount++;
                  pWinner = m_pResultTable[i+0];
                }
                else if( m_pResultTable[i+1] )
                {
                  iwincount++;
                  pWinner = m_pResultTable[i+0] = m_pResultTable[i+1];
                  m_pResultTable[i+1] = NULL;
                }
              }
              //
              int ipos = 1;
              for( i = 2; i< GetBoutsByLayer(); i+=2 )
              {
                if( m_pResultTable[i] )
                {
                  m_pResultTable[ipos] = m_pResultTable[i];
                  m_pResultTable[i] = NULL;
                  ipos++;
                }
                else
                {
                  m_pResultTable[ipos] = NULL;
                  ipos++;
                }
              }
            }
            //比赛上升一层
            SetLayer( GetLayer() + 1 );
            //
            SetWinnerMatch( TRUE );
            //如果有冠军产生
            if( iwincount == 0 || iwincount == 1 )
            {
              //如果没有到总冠军,直接升到总冠军
              if( GetLayer() < 8 )
              {
                SetLayer( 8 );
                //
                if( iwincount == 1 )
                {
                  if( pWinner->GetWinTime() < 7 )
                  {
                    pWinner->SetWinTime( 7 );
                  }
                }
              }
              //整个比赛到这里就结束了
              SetMatchState( MATCH_STATE_NONE );
              SetRestTime( 0 );
              //END
            #ifdef _MODIFY_GUID_TEAMMACTH_
							//整理帮会团体赛(6VS6)的比赛结果
							HandleMatchResult();
            #endif
              SetFightTimeFlag( FALSE );
              SetAfterFight( TRUE );
              //
              SetWinnerMatch( FALSE );
#ifdef _ADD_ITEM_TO_WINNER_
              CPlayer *pPlayer = NULL;
              if(NULL != pWinner && NULL != pWinner->m_pSelfInfo)
              {
                if(MATCH_MODE_OCCUPATION == GetMatchId())
                {
                  DWORD dwMailId = pWinner->m_pSelfInfo->sNameTip.iId;
                  if(NULL != (pPlayer=g_pGs->GetPlayerFromMailId(dwMailId)))
                  {
                    pPlayer->SetSaveFlag(135,EVENTSTATUS_ON);
                    SMsgData *pNewMsgTalk = g_pGs->NewMsgBuffer();
                    char  szTemp[1024];
                    sprintf(szTemp, "セΩゑ辽玜瓁%s, 盢莉眔10遏步ホ, 叫睦ぱ窾ㄆ硄矪烩.", pPlayer->GetPlayerName());
                    if( NULL != pNewMsgTalk )
                    {
                      pNewMsgTalk->dwAID        = A_TALKTOALL;
                      pNewMsgTalk->dwMsgLen	    = 1;
                      pNewMsgTalk->Msgs[0].Size = strlen( szTemp );
                      SafeStrcpy( pNewMsgTalk->Msgs[0].Data, szTemp, MAXMSGDATASIZE );
                      g_pGs->SendTheMsgToAll( *pNewMsgTalk );
                      g_pGs->ReleaseMsg( pNewMsgTalk );
                    }
                  }                  
                }
              }
#endif             
              //发送给MCC整个比赛结束
              //SendMcc_AP_END_ALL_MATCH();
            }
            else
            {            
              //第6层为4位选手一起比赛
              //SetFightArray( pFighterArray, iwincount );
              //比赛重新开始
              //SetMatchState( MATCH_STATE_START );
              SetMatchState( MATCH_STATE_BEFORE );
              SetRestTime( GetIntervalTime() );
            }
          }
        }
        else// if( icount >= 2 )
        {
          //比赛上升一层
          SetLayer( GetLayer() + 1 );
          //比赛重新开始
          //SetMatchState( MATCH_STATE_START );
          SetMatchState( MATCH_STATE_BEFORE );
          SetRestTime( GetIntervalTime() );
        }
      }
      else
      {
        //进入下一局比赛,比赛重新开始
        //SetMatchState( MATCH_STATE_START );
        SetMatchState( MATCH_STATE_BEFORE );
        SetRestTime( GetIntervalTime() );
      }
      //////////////////////////////////////////////////////////////////
      //Add by CECE 2004-09-20
      CopyLayer();
      //////////////////////////////////////////////////////////////////
    }
    break;
  case  MATCH_MODE_LIVEMODE:      //生存赛
    {
      icount = 0;
      SetMatchState( MATCH_STATE_NONE );
      SetRestTime( 0 );
      SetFightTimeFlag( FALSE );
      //发送给MCC整个比赛结束
      //SendMcc_AP_END_ALL_MATCH();
    }
    break;
 }
 //
 //SetUpdateTime( GetUpdateTime()+1 );
 return icount;
}

//判断每次时间段内的结果,
//比赛结果一出,就把选手Warp回
void CFightFieldMgr::JudgeResult( BOOL bGameOver )
{
  LPCFighter     pFighter1 = NULL, pFighter2 = NULL;
  LPSMatchResult pMatchResult = NULL;
  int iDraw1 = -1, iDraw2 = -1;
#ifdef _MODIFY_GUID_TEAMMACTH_
	bool bTeamResult=false;//此变量用于接收帮会团体比赛的结果
#endif
  //
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
  case MATCH_MODE_WEIGHT:        //量级赛
  case MATCH_MODE_TEAM:          //团体赛
    {
      for( int i = GetPrevSendCount();
      i < GetSendCount(); i+= 2 )
      {
        iDraw1 = -1; iDraw2 = -1;
        //
        pFighter1 = GetFightResult( i+0 );
        pFighter2 = GetFightResult( i+1 );
        //这次没有比赛
        if( pFighter1 == NULL && pFighter2 == NULL ) continue;
        //如果已经判断输赢
        if( ( pFighter1 && pFighter1->GetJoin() == FALSE )  ||
          ( pFighter2 && pFighter2->GetJoin() == FALSE )  )
          continue;
        //获取场地比赛结果的结构
        //pMatchResult = GetMatchResult( ( i-GetPrevSendCount() ) / 2 );
        pMatchResult = GetMatchResult( ( ( i-GetPrevSendCount() ) / 2)+1 );
        //add by Jack.Ren 修正在比赛结束最后刹那使用召唤 不能正常Warp的bug——等到召唤动作完成后再Warp
        //没有考虑团体赛
        CPlayer * pPlayer1 = NULL;
        CPlayer * pPlayer2 = NULL;
        BOOL      bSummon = TRUE;
#ifdef FIX_BUG_FIGHT_EVIL
				//非团体比赛
        if (GetMatchMode() != MATCH_MODE_TEAM) 
        {
					// Check Player
          if (pFighter1 != NULL)
            pPlayer1 = g_pGs->GetPlayerFromMailId( pFighter1->GetSelfInfo()->sNameTip.iId );
          if (pFighter2 != NULL)
            pPlayer2 = g_pGs->GetPlayerFromMailId( pFighter2->GetSelfInfo()->sNameTip.iId );
          // Check Time
          if (pPlayer1 != NULL) 
          {
            if (ClientTickCount < pPlayer1->GetSummonTime()+5000)
              bSummon = FALSE;
          }
          if (pPlayer2 != NULL)
          {
            if (ClientTickCount < pPlayer2->GetSummonTime()+5000)
              bSummon = FALSE;
          }
        }
				//modify by lihuijie 1/27/2008
				//修正在团体比赛结束最后刹那使用召唤 不能正常Warp的bug——等到召唤动作完成后再Warp
				//只要团体比赛中有人使用了召唤，那么所有队Warp也要被延时，这样是为了和以前的修证保持统一
				if (GetMatchMode() == MATCH_MODE_TEAM)
				{
					int iCnt = 0, k;
					const int iMaxCnt=2;
					CPlayer *pUser;
					LPCFighter pFihgter;
					for (pFihgter=pFighter1; iCnt < iMaxCnt; ++iCnt)
					{
						if (NULL==pFihgter && pFihgter!=pFighter2)  pFihgter=pFighter2;
						if (NULL==pFihgter && pFihgter==pFighter2)  break;
						for( k = 0; k < TEAM_MEMBER_COUNT_MAX; ++k )
						{
							if(0 == pFihgter->GetSelfInfo()->iTeamMailId[k])  continue;
							pUser = g_pGs->GetPlayerFromMailId( pFihgter->GetSelfInfo()->iTeamMailId[k] );
							if (NULL == pUser)    continue;
							if (ClientTickCount < pUser->GetSummonTime()+5000)
							{
								bSummon = FALSE;
								break;
							}
						} // end for iCnt
						if (pFihgter==pFighter2)	iCnt = iMaxCnt - 1;
						else        							pFihgter=pFighter2;
					}// end for pFihgter=pFighter1
				}
#endif
        //如果比赛结束
        if( bGameOver && bSummon)
        {
          //当选手都报名了
          if( pFighter1 && pFighter2)
          {
            //如果都在场
            if( pFighter1->IsInMap() && pFighter2->IsInMap() )
            {
              //如果双方都没有死亡,则判断积分
              if( pFighter1->GetNum() > 0 && pFighter2->GetNum() > 0 )
              {
#ifdef _MODIFY_GUID_TEAMMACTH_
								//针对帮会团体比赛作下面操作,若已经有了比赛结果则退出
								bTeamResult = JudeGuidMatchResult(pFighter1,pFighter2,pMatchResult,i,&iDraw1,&iDraw2);
								if ( !bTeamResult )
								{
#endif//_MODIFY_GUID_TEAMMACTH_
                //Win
                if( pFighter1->GetValue() > pFighter2->GetValue() )
                {
#ifdef _FIGHT_RESULT_LOG_
                  SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
#endif
                  pFighter1->Win();
                  DrawMatch( i+1 ); iDraw2 = i+1;
                  //都参加,都在场,1P赢
                  pMatchResult->iResult = RESULT_ALLJOIN_1P_WON;
                }
                //Draw
                else if( pFighter1->GetValue() == pFighter2->GetValue() )
                {
#ifdef _FIGHT_RESULT_LOG_
                  SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 0 );
#endif
                  DrawMatch( i+0 ); iDraw1 = i+0;
                  DrawMatch( i+1 ); iDraw2 = i+1;
                  //都参加,都在场,打平
                  pMatchResult->iResult = RESULT_ALLJOIN_BOTH_DRAW;
                }
                //Fail
                else//( pFighter1->GetValue() < pFighter2->GetValue() )
                {
#ifdef _FIGHT_RESULT_LOG_
                  SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
#endif
                  DrawMatch( i+0 ); iDraw1 = i+0;
                  pFighter2->Win();
                  //都参加,都在场,2P赢
                  pMatchResult->iResult = RESULT_ALLJOIN_2P_WON;
                }
#ifdef _MODIFY_GUID_TEAMMACTH_
								}//end if(!bTeamResult)
#endif
              }
              //选手1获胜            
              else if( pFighter1->GetNum() > 0 && pFighter2->GetNum() == 0 )
              {
#ifdef _FIGHT_RESULT_LOG_
                SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
#endif
                pFighter1->Win();
                DrawMatch( i+1 ); iDraw2 = i+1;
                //都参加,都在场,1P赢
                pMatchResult->iResult = RESULT_ALLJOIN_1P_WON;
              }
              //选手2获胜
              else if( pFighter1->GetNum() == 0 && pFighter2->GetNum() > 0 )
              {
#ifdef _FIGHT_RESULT_LOG_
                SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
#endif                
                DrawMatch( i+0 ); iDraw1 = i+0;
                pFighter2->Win();
                //都参加,都在场,2P赢
                pMatchResult->iResult = RESULT_ALLJOIN_2P_WON;
                
              }
              //
              WarpToLobby( pFighter1, true );
              WarpToLobby( pFighter2 );
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            //如果1P在场
            else if( pFighter1->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
#endif
              pFighter1->Win();
              DrawMatch( i+1 ); iDraw2 = i+1;
              //都参加,2P不在场,1P赢
              pMatchResult->iResult = RESULT_ALLJOIN_2PNOT_1P_WON;
              //
              WarpToLobby( pFighter1, true );
              //WarpToLobby( pFighter2 );
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            //如果2P在场
            else if( pFighter2->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              pFighter2->Win(); 
              //都参加,1P不在场,2P赢
              pMatchResult->iResult = RESULT_ALLJOIN_1PNOT_2P_WON;
              //
              //WarpToLobby( pFighter1 );
              WarpToLobby( pFighter2, true );
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            //都不在场
            else
            {
              DrawMatch( i+0 ); iDraw1 = i+0;
              DrawMatch( i+1 ); iDraw2 = i+1;
              //都参加,都在场,打平
              pMatchResult->iResult = RESULT_ALLJOIN_ALLNOT_DRAW;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
          }
          //1号选手在场
          else if( pFighter1 )
          {
            if( pFighter1->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, NULL, m_iMatchMode, 1 );
#endif
              pFighter1->Win();
              DrawMatch( i+1 ); iDraw2 = i+1;
              //只有1P参加,1号在场,1P赢
              pMatchResult->iResult = MATCH_1PJOIN_1P_WON;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            else
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, NULL, m_iMatchMode, 0 );
#endif              
              DrawMatch( i+0 ); iDraw1 = i+0;
              DrawMatch( i+1 ); iDraw2 = i+1;
              //只有1P参加,1号不在场,1P输
              pMatchResult->iResult = MATCH_1PJOIN_1PNOT_1P_DRAW;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
          }
          //2号选手在场
          else if( pFighter2 )
          {
            if( pFighter2->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( NULL, pFighter2, m_iMatchMode, 2 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              pFighter2->Win();
              //只有2P参加,2号在场,2P赢
              pMatchResult->iResult = MATCH_2PJOIN_2P_WON;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            else
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 0 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              DrawMatch( i+1 ); iDraw2 = i+1;
              //只有2P参加,2号不在场,2P输
              pMatchResult->iResult = MATCH_2PJOIN_2PNOT_2P_DRAW;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
          }
        }
        //如果比赛还未结束
        else if( bSummon )
        {
          //当选手都报名了
          if( pFighter1 && pFighter2 )
          {
            //如果都在场
            if( pFighter1->IsInMap() && pFighter2->IsInMap() )
            {
              if( pFighter1->GetNum() > 0 && pFighter2->GetNum() == 0 )
              {
#ifdef _FIGHT_RESULT_LOG_
                SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
#endif
                pFighter1->Win();
                DrawMatch( i+1 ); iDraw2 = i+1;
                //都参加,都在场,1P赢
                pMatchResult->iResult = RESULT_ALLJOIN_1P_WON;
                //
                //#ifndef FF_EXTEND_ZETOR
                WarpToLobby( pFighter1, true );
                //#endif
                WarpToLobby( pFighter2 );
                //所有局数都结束,则让每局的输掉者退出比赛
                if( IsWinnerMatch() )
                {
                  if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                  if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
                }
                //
                SetUpdateTime( GetUpdateTime()+1 );
              }
              else if( pFighter1->GetNum() == 0 && pFighter2->GetNum() == 0 )
              {
#ifdef _FIGHT_RESULT_LOG_
                SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 0 );
#endif                
                DrawMatch( i+0 ); iDraw1 = i+0;
                DrawMatch( i+1 ); iDraw2 = i+1;
                //都参加,都在场,打平
                pMatchResult->iResult = RESULT_ALLJOIN_BOTH_DRAW;
                //
                WarpToLobby( pFighter1, true );
                WarpToLobby( pFighter2 );
                //所有局数都结束,则让每局的输掉者退出比赛
                if( IsWinnerMatch() )
                {
                  if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                  if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
                }
                //
                SetUpdateTime( GetUpdateTime()+1 );
              }
              else if( pFighter1->GetNum() == 0 && pFighter2->GetNum() > 0 )
              {
#ifdef _FIGHT_RESULT_LOG_
                SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
#endif
                DrawMatch( i+0 ); iDraw1 = i+0;
                pFighter2->Win();
                //都参加,都在场,2P赢
                pMatchResult->iResult = RESULT_ALLJOIN_2P_WON;
                //
                WarpToLobby( pFighter1, true );
                //#ifndef FF_EXTEND_ZETOR
                WarpToLobby( pFighter2 );
                //#endif
                //所有局数都结束,则让每局的输掉者退出比赛
                if( IsWinnerMatch() )
                {
                  if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                  if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
                }
                //
                SetUpdateTime( GetUpdateTime()+1 );
              }
            }//if( pFighter1->IsInMap() && pFighter2->IsInMap() )
            //只有1P在场
            else if( pFighter1->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
#endif
              pFighter1->Win();
              DrawMatch( i+1 ); iDraw2 = i+1;
              //都参加,2P不在场,1P赢
              pMatchResult->iResult = RESULT_ALLJOIN_2PNOT_1P_WON;
              //
              //#ifndef FF_EXTEND_ZETOR
              WarpToLobby( pFighter1, true );
              //#endif
              //WarpToLobby( pFighter2 );
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            //只有2P在场
            else if( pFighter2->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              pFighter2->Win();
              //都参加,1P不在场,2P赢
              pMatchResult->iResult = RESULT_ALLJOIN_1PNOT_2P_WON;
              //
              //WarpToLobby( pFighter1 );
              //#ifndef FF_EXTEND_ZETOR
              WarpToLobby( pFighter2, true );
              //#endif
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            //都不在场
            else
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 0 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              DrawMatch( i+1 ); iDraw2 = i+1;
              //都参加,都在场,打平
              pMatchResult->iResult = RESULT_ALLJOIN_ALLNOT_DRAW;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }//if( pFighter1->IsInMap() && pFighter2->IsInMap() )
          }
          //1号选手在场
          else if( pFighter1 )
          {
            //如果1P在场
            if( pFighter1->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, NULL, m_iMatchMode, 1 );
#endif              
              pFighter1->Win();
              DrawMatch( i+1 ); iDraw2 = i+1;
              //只有1P参加,1号在场,1P赢
              pMatchResult->iResult = MATCH_1PJOIN_1P_WON;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            else
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( pFighter1, NULL, m_iMatchMode, 0 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              DrawMatch( i+1 ); iDraw2 = i+1;
              //只有1P参加,1号不在场,1P输
              pMatchResult->iResult = MATCH_1PJOIN_1PNOT_1P_DRAW;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
          }
          //2号选手在场
          else if( pFighter2 )
          {
            //如果2P在场
            if( pFighter2->IsInMap() )
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( NULL, pFighter2, m_iMatchMode, 2 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              pFighter2->Win(); 
              //只有2P参加,2号在场,2P赢
              pMatchResult->iResult = MATCH_2PJOIN_2P_WON;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            else
            {
#ifdef _FIGHT_RESULT_LOG_
              SendFightResultlog( NULL, pFighter2, m_iMatchMode, 0 );
#endif
              DrawMatch( i+0 ); iDraw1 = i+0;
              DrawMatch( i+1 ); iDraw2 = i+1;
              //只有2P参加,2号不在场,2P输
              pMatchResult->iResult = MATCH_2PJOIN_2PNOT_2P_DRAW;
              //所有局数都结束,则让每局的输掉者退出比赛
              if( IsWinnerMatch() )
              {
                if( (iDraw1 == i+0 ) && pFighter1 ) pFighter1->Init();
                if( (iDraw2 == i+1 ) && pFighter2 ) pFighter2->Init();
              }
              //
              SetUpdateTime( GetUpdateTime()+1 );
            }
            
          }//if( pFighter1 && pFighter2 )
        }//if( bGameOver )
      }//for(
    }
    break;
  case MATCH_MODE_LIVEMODE:      //生存赛
    {
      //
      pFighter1 = GetFightResult( 0 );
      if( pFighter1->GetJoin() == FALSE ) break;
      
      if( bGameOver )
      {
        //
        CreateMonster( FALSE );
        //计算剩下来的人的分数
        WarpToLobby( pFighter1, true );
      }
      else
      {
        if( pFighter1->GetNum() <= LIVEMODE_PLAYER_NUM_MIN )
        {
          //
          CreateMonster( FALSE );
          //计算剩下来的人的分数
          WarpToLobby( pFighter1, true );
          //
          SetRestTime( 1 );
        }
        else
        {
          //根据人数变化来放怪
          int iNum = pFighter1->GetNum();
          if( GetLastCount() - iNum >= GetStepByCount() )
          {
            CreateMonster( TRUE );
            SetLastCount( iNum );
          }
        }
      }
      //获取场地比赛结果的结构
      pMatchResult = GetMatchResult( 0 );
      pMatchResult->iResult = RESULT_ALLJOIN_BOTH_WON;
    }
    break;
  }
}

//比赛结束后把选手Warp到休息室
void CFightFieldMgr::WarpToLobby( LPCFighter pFihgter, bool bIsShow )
{
  if( pFihgter == NULL ) return;
  if( GetMatchMode()!= MATCH_MODE_LIVEMODE && pFihgter->GetSelfInfo() == NULL) return;
  int iMapId = pFihgter->GetMapId();
  if( iMapId == 0 ) return;
  //clear info        
  pFihgter->SetMapId( 0 );
  pFihgter->SetJoin( FALSE );
  pFihgter->SetEnemy( NULL );
  //
  CPlayer* pPlayer  = NULL;
  CTeam*   pTeam    = NULL;
  int      iFighter = 0;
  SWarpPoint* pWarp = NULL;
  //
  LPSMapWarp pLobby = GetLobby();
  if( pLobby )
  {
    switch( GetMatchMode() )
    {
    case MATCH_MODE_OCCUPATION:    //职业赛
    case MATCH_MODE_WEIGHT:        //量级赛
      if( ( pPlayer = g_pGs->GetPlayerFromMailId( pFihgter->GetSelfInfo()->sNameTip.iId ) ) &&
        pPlayer->GetBeSent() &&
        ( pPlayer->GetMapId() == iMapId ) )
      {
        pWarp = pLobby->GetPoint();
        pWarp->wMapId = pPlayer->GetMapId();
        //给玩家分配竞赛信息
        pPlayer->SetBeSent( FALSE );
        pPlayer->ClearFightSwitch();
        pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
        ReviveFighter( pPlayer );
      }
      break;
    case MATCH_MODE_LIVEMODE:      //生存赛
      {
        //获取获胜的选手
        NAME_LIST& nameList = GetSignupList( MATCH_TYPE_LIVEMODE );
        for( NAME_ITER iter = nameList.begin(); 
        iter != nameList.end(); iter++ )
        {
          iFighter = (*iter)->sNameTip.iId;
          if( ( pPlayer = g_pGs->GetPlayerFromMailId( iFighter ) ) &&
            pPlayer->GetBeSent() &&
            ( pPlayer->GetMapId() == iMapId ) )
          {
            pWarp = pLobby->GetPoint();
            pWarp->wMapId = pPlayer->GetMapId();
            //加入到赢的列表里面
            //给玩家分配竞赛信息
            pPlayer->SetBeSent( FALSE );
            pPlayer->ClearFightSwitch();
            pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
            ReviveFighter( pPlayer );
          }
        }
      }
      break;
    case MATCH_MODE_TEAM:          //团体赛
      {
        for( int i = 0; i<TEAM_MEMBER_COUNT_MAX; i++ )
        {
          if( pFihgter->GetSelfInfo()->iTeamMailId[i] == 0 ) continue;
          pPlayer = g_pGs->GetPlayerFromMailId( pFihgter->GetSelfInfo()->iTeamMailId[i] );
          if( pPlayer &&
            pPlayer->GetBeSent() &&
            pPlayer->GetMapId() == iMapId )
          {
            pWarp = pLobby->GetPoint();
            pWarp->wMapId = pPlayer->GetMapId();
            //给玩家分配竞赛信息
            pPlayer->SetBeSent( FALSE );
            pPlayer->ClearFightSwitch();
            pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
            ReviveFighter( pPlayer );
          }
        }
      }
      /*if( pTeam = ::GetTeam( pFihgter->GetSelfInfo()->sNameTip.iId ) )
      {
      int iMemCount = pTeam->GetMembersNum();
      WORD wCount;
      DWORD* pMailIdList = pTeam->GetAllTeamerMailIDList( wCount );
      for( int i=0; i<iMemCount; i++ )
      {
      pPlayer = pTeam->GetMember( pMailIdList[i] );
      if( pPlayer &&
      pPlayer->GetBeSent() &&
      pPlayer->GetMapId() == iMapId )
      {
      pWarp = pLobby->GetPoint();
      pWarp->wMapId = pPlayer->GetMapId();
      //给玩家分配竞赛信息
      pPlayer->SetBeSent( FALSE );
      pPlayer->Send_A_WARP( *pWarp, PLAYER_WARP_TYPE_MAP );
      ReviveFighter( pPlayer );
      }
      }//for( int i=0; i<iMemCount; i++ )
    }*/
      break;
    }  
#ifdef _MODIFY_GUID_TEAMMACTH_
		if (pPlayer && bIsShow)   pPlayer->SendTalkToAll("セ近ゑ辽挡");
#endif
	}
}

//接受报名
BOOL CFightFieldMgr::Recv_A_SIGNUP( CPlayer* pPlayer, SMsgData * pTheMsg )
{
  if( pTheMsg == NULL ) return FALSE;
  //检查
  if( FALSE == CheckSignup( pPlayer, pTheMsg ) ) return FALSE;
  //添加到名单中
  if( FALSE == AddToNameList( pPlayer, pTheMsg ) ) return FALSE;
  return TRUE;
}

//查询报名名单
void CFightFieldMgr::Recv_A_QRYSIGNUP( CPlayer* pPlayer, SMsgData* pTheMsg )
{
  //pTheMsg->Init();
  pTheMsg->dwMsgLen = 2;
  pTheMsg->Msgs[0].Size = sizeof(SQrySignup);
  pTheMsg->Msgs[1].Size = SIGNUP_NUM_MAX*MAX_PLAYER_NAME_LEN;
  //
  LPSQrySignup pNameList = (LPSQrySignup)pTheMsg->Msgs[0].Data;
  pNameList->wMatchMode = GetMatchMode();
  //fill name list
  int   icount = 0;
  char* pNameBuf = (char*)pTheMsg->Msgs[1].Data;
  memset( pNameBuf, 0, SIGNUP_NUM_MAX*MAX_PLAYER_NAME_LEN );
  for( int iMatchType = 0; iMatchType < MATCH_TYPE_MAX; iMatchType++ )
  {
    icount = iMatchType*SIGNUP_NOTLIVE_NUM_MAX;
    pNameList->wCount[iMatchType] = GetSignupNum( iMatchType );
    if( pNameList->wCount[iMatchType] == 0 ) continue;
    NAME_LIST& nameList = GetSignupList( iMatchType );
    for( NAME_ITER iter = nameList.begin(); iter != nameList.end(); iter++ )
    {
#ifdef _REPAIR_SERVER_CRASH_NICK_
      SafeStrcpy( pNameBuf+icount*MAX_PLAYER_NAME_LEN, (*iter)->sNameTip.szName, MAX_PLAYER_NAME_LEN );
#else
      strcpy( pNameBuf+icount*MAX_PLAYER_NAME_LEN, (*iter)->sNameTip.szName );
#endif
      icount++;
    }
  }
  pPlayer->AddSendMsg( pTheMsg );
}

//查询比赛结果
void CFightFieldMgr::Recv_A_QRYRESULT( CPlayer* pPlayer, SMsgData* pTheMsg )
{
  int wType = *((int*)pTheMsg->Msgs[0].Data);
  //
  switch( GetMatchMode() )
  {
  case MATCH_MODE_OCCUPATION: //职业赛
    if( wType >= MATCH_TYPE_OCCU_MAX )
    {
      g_pGs->ReleaseMsg( pTheMsg );
      return;
    }
    break;
  case MATCH_MODE_WEIGHT:     //量级赛
    if( wType >= MATCH_TYPE_WEIGHT_MAX )
    {
      g_pGs->ReleaseMsg( pTheMsg );
      return;
    }
    break;
  case MATCH_MODE_TEAM:       //团体赛
    if( wType >= MATCH_TYPE_XVSX_MAX )
    {
      g_pGs->ReleaseMsg( pTheMsg );
      return;
    }
    break;
  case MATCH_MODE_LIVEMODE:   //生存赛
    g_pGs->ReleaseMsg( pTheMsg );
    return;
    /*
    if( wType >= MATCH_TYPE_LIVEMODE_MAX )
    {
    g_pGs->ReleaseMsg( pTheMsg );
    return;
    }
    break;
    */
  }
  WORD wPresent, wJoin;
  int  icount   = 0;
  WORD*   pwLayer  = (WORD*)pTheMsg->Msgs[0].Data;
  for( int itype = 0; itype < MATCH_TYPE_MAX; itype++ )
  {
    for( int i = 0; i< 4; i++ )
    {
      pwLayer[itype*4+i] = m_iAllLayer[itype][i]/*m_iLayer[i]*/;
    }
  }
  //pwLayer[0]               = GetLayer();
  //pwLayer[1]               = GetMatchId();
  WORD*           pwResult = pwLayer+16;//(WORD*)(pTheMsg->Msgs[0].Data+sizeof(WORD));
  LPSResultRecord pResultRecord = (LPSResultRecord)pTheMsg->Msgs[1].Data;
  LPSChallenger   pInfo    = NULL;
  //
  for( int iarea = 0; iarea< GetMaxAreaCount(); iarea++ )
  {
    wPresent = 0;
    wJoin    = 0x8000;
    for( int i = 0; i< TOTAL_MATCH_TIMES; i++ )
    {
      //如果有选手报名
      if( pInfo = m_pChallengerTable[wType][iarea][i] )
      {
        wPresent |= wJoin;
        pResultRecord[icount].sVsInfo  = pInfo->sNameTip;
        pResultRecord[icount].iWinTime = pInfo->iWinTime;
        icount++;
      }
      wJoin >>= 1;
    }
    //
    pwResult[iarea] = wPresent;
  }
  pTheMsg->dwMsgLen = 1;
  pTheMsg->Msgs[0].Size = (16+iarea)*sizeof(WORD);
  if( icount > 0 )
  {
    pTheMsg->dwMsgLen = 2;
    pTheMsg->Msgs[1].Size = icount*sizeof(SResultRecord);
  }
  pPlayer->AddSendMsg( pTheMsg );
}

//读取比赛的信息
BOOL CFightFieldMgr::LoadMatchInfo( /*const*/ char* szFileName )
{
#ifdef _DEBUG_JAPAN_DECRYPT_
  CInStream in(szFileName);
  if (in.fail()||in.GetFileSize() == 0) 
  {
    return FALSE;
  }
#else
  ifstream in(szFileName);
  if( in.fail() ) return FALSE;
#endif//_DEBUG_JAPAN_DECRYPT_
  //
  CSrvBaseMap     *pTheWarpMap = NULL;
  //load signup money
  in>>MATCH_MONEY_COST;         //报名费用
  in>>LIVEMODE_PLAYER_NUM_MIN;  //生存赛的胜利人数
  
  //load field time
  //比赛时间
  //TYPE1的比赛时间 TYPE2的比赛时间 TYPE3的比赛时间 TYPE4的比赛时间 休息时间
  for( int i = 0; i< MATCH_MODE_MAX; i++ )
  {
    for( int m =0; m < 5; m++ )
    {
      in>>m_iBaseRefTime[i][m];
    }
  }
  //load map warp point
  int iPosX, iPosY;
  int iMapCount = 0;
  //地图总张数
  //地图WARP点说明格式
  //ID WARP点个数  X1 Y1 X2 Y2 ... Xn Yn 
  //第1张必须为休息室地图
  //第2张必须为生存赛地图
  //剩下为一般的比赛场地
  in>>iMapCount;
  iMapCount -= 1;
  
  m_pLobby = new SMapWarp[1];
  in>>m_pLobby->iMapId;
  m_iLobbyMapId = m_pLobby->iMapId;
  //
  pTheWarpMap = g_pBase->GetBaseMap( m_iLobbyMapId );
  pTheWarpMap->SetViewMatch( TRUE );
  //
  in>>m_pLobby->iCount;
  m_pLobby->warpPoint = new SWarpPoint[m_pLobby->iCount];
  memset( m_pLobby->warpPoint, 0, sizeof(SWarpPoint)*m_pLobby->iCount );
  for( i = 0; i<m_pLobby->iCount; i++ )
  {
    m_pLobby->warpPoint[i].wMapId = 0;
    //
    m_pLobby->warpPoint[i].wTargetMapId  = m_pLobby->iMapId;
    in>>iPosX;
    in>>iPosY;
    pTheWarpMap->ConvertCli2Srv( &iPosX, &iPosY );
    m_pLobby->warpPoint[i].wTargetMapX = iPosX;
    m_pLobby->warpPoint[i].wTargetMapY = iPosY;
  }
  //
  m_pFieldList  = NULL;
  m_iFieldCount = 0;
  if( iMapCount > 0 )
  {    
    m_iFieldCount = iMapCount;
    //
    m_pMatchResult = new SMatchResult[iMapCount];
    m_pFieldList   = new SMapWarp[iMapCount];
    for( int i = 0; i< iMapCount; i++ )
    {
      m_pMatchResult[i].SetId( i );
      //
      in>>m_pFieldList[i].iMapId;
      //
      pTheWarpMap = g_pBase->GetBaseMap( m_pFieldList[i].iMapId );
      pTheWarpMap->SetViewMatch( TRUE );
      //
      in>>m_pFieldList[i].iCount;
      m_pFieldList[i].warpPoint = new SWarpPoint[m_pFieldList[i].iCount];
      memset( m_pFieldList[i].warpPoint, 0, sizeof(SWarpPoint)*m_pFieldList[i].iCount );
      for( int m = 0; m<m_pFieldList[i].iCount; m++ )
      {
        m_pFieldList[i].warpPoint[m].wMapId = m_pLobby->iMapId;
        //
        m_pFieldList[i].warpPoint[m].wTargetMapId = m_pFieldList[i].iMapId;
        in>>iPosX;
        in>>iPosY;
        pTheWarpMap->ConvertCli2Srv( &iPosX, &iPosY );
        m_pFieldList[i].warpPoint[m].wTargetMapX = iPosX;
        m_pFieldList[i].warpPoint[m].wTargetMapY = iPosY;
      }
    }
  }
  //禁止选手使用的道具
  //格式
  //个数 ID1 ID2 .. IDn
  m_listNotUseItemF.clear();
  int iCount = 0, iValue;
  in>>iCount;
  for( i = 0; i< iCount; i++ )
  {
    in>>iValue;
    m_listNotUseItemF.push_back(iValue);
  }
  //禁止观战者使用的道具
  //格式
  //个数 ID1 ID2 .. IDn
  m_listNotUseItemV.clear();
  in>>iCount;
  for( i = 0; i< iCount; i++ )
  {
    in>>iValue;
    m_listNotUseItemV.push_back(iValue);
  }
  //load 生存赛人数放怪规则
  in>>m_iStepByCount;        //生存赛下降一阶的人数
  in>>m_iStepCount;          //生存赛下降的总阶数
  m_pLiveMatchData = new WORD*[m_iStepCount];
  for( i = 0; i< m_iStepCount; i++ )
  {
    m_pLiveMatchData[i] = new WORD[LIVEMATCH_INFO_MAX];
    for( int m = 0; m < LIVEMATCH_INFO_MAX; m++ )
    {
      in>>m_pLiveMatchData[i][m];
    }
  }
  //load 怪物信息 
  in>>m_iMonsterLevelStep;
  m_plistMonster = new list<int>[m_iMonsterLevelStep];
  int iIndex;
  for( i = 0; i<m_iMonsterLevelStep; i++ )
  {
    in>>iIndex;
    list<int>& pList = m_plistMonster[iIndex-1];
    in>>iCount;
    for( int m = 0; m<iCount; m++ )
    {
      in>>iValue;
      pList.push_back(iValue);
    }
  }
#ifndef _DEBUG_JAPAN_DECRYPT_
  in.close();
#endif
  return TRUE;
}

void CFightFieldMgr::RecvMcc_AP_MATCHX(SMsgData* pMccMsg)
{
  if( NULL == pMccMsg ) return;
  SMsgData    *pNewMsg = g_pGs->NewMsgBuffer();
  if( pNewMsg == NULL )
  {
    g_pGs->ReleaseMsg( pMccMsg );
    pMccMsg = NULL;
    return;
  }
  ::CopyMessage( pMccMsg, pNewMsg );
  g_pGs->ReleaseMsg( pMccMsg );
  pMccMsg = NULL;
  //
  switch( pNewMsg->dwAID )
  {
  case AP_END_SIGNUP:   //报名结束
    pNewMsg->dwAID = A_END_SIGNUP;
    break;
  case AP_BEFORE_ALL_MATCH: //比武大会预备时间
    pNewMsg->dwAID = A_BEFORE_ALL_MATCH;
    break;
  case AP_MATCH_BEGIN:  //一场比赛开始
    pNewMsg->dwAID = A_MATCH_BEGIN;
    break;
  case AP_MATCH_END:    //一场比赛结束,并做中场通知
    pNewMsg->dwAID = A_MATCH_END;
    break;
  case AP_END_ALL_MATCH: //比武大会结束
    pNewMsg->dwAID = A_END_ALL_MATCH;
    break;
  case AP_NOMATCH_NOTICE: //比赛开局通知
    pNewMsg->dwAID = A_NOMATCH_NOTICE;
    break;
  }
  g_pGs->SendTheMsgToAll( pNewMsg );
}


void CFightFieldMgr::SendMcc_AP_END_SIGNUP()     //报名结束
{
  SMccMsgData     *pNewMccMsg = NULL;
  if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )  return;
  pNewMccMsg->Init( NULL );
  pNewMccMsg->dwAID        = AP_END_SIGNUP;
  pNewMccMsg->dwMsgLen     = 0;
  pNewMccMsg->Msgs[0].Size = 0;
  pNewMccMsg->Msgs[1].Size = 0;
  //
  g_pMccDB->AddSendMsg( pNewMccMsg );
}

void CFightFieldMgr::SendMcc_AP_END_ALL_MATCH()    //比武大会结束
{
  SMccMsgData     *pNewMccMsg = NULL;
  if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )  return;
  int icount = 0;
  pNewMccMsg->Init( NULL );
  pNewMccMsg->dwAID        = AP_END_ALL_MATCH;
  LPSMatchResult pResult = (LPSMatchResult)pNewMccMsg->Msgs[1].Data;
  for( int i = 0; i< GetFieldCount(); i++ )
  {
  /*
  //只要不是没有人参加
  if( m_pMatchResult[i].iResult == RESULT_ALLJOIN_BOTH_DRAW ||
  m_pMatchResult[i].iResult == RESULT_ALLJOIN_1P_WON    ||
  m_pMatchResult[i].iResult == RESULT_ALLJOIN_2P_WON    ||
  m_pMatchResult[i].iResult == RESULT_ALLJOIN_BOTH_WON )
  {
  pResult[icount++] = m_pMatchResult[i];
  }
    */
    if( ( m_pMatchResult[i].iResult != RESULT_NOJOIN_MOMATCH ) &&
      ( m_pMatchResult[i].bSend == FALSE ) )
    {
      pResult[icount++] = m_pMatchResult[i];
    }
  }
  if( icount > 0 )
  {
    pNewMccMsg->dwMsgLen = 2;
    pNewMccMsg->Msgs[1].Size = icount*sizeof(SMatchResult);
  }
  else  pNewMccMsg->dwMsgLen = 1;
  pNewMccMsg->Msgs[0].Size = sizeof(SMatchNotice);
  LPSMatchNotice pNotice = (LPSMatchNotice)pNewMccMsg->Msgs[0].Data;
  pNotice->iMatchMode  = GetMatchMode();
  pNotice->iMatchType  = GetMatchType();
  pNotice->iBeforeTime = 0;
  //
  g_pMccDB->AddSendMsg( pNewMccMsg );
}

void CFightFieldMgr::SendMcc_AP_BEFORE_ALL_MATCH( int iBeforeTime )
{
  SMccMsgData     *pNewMccMsg = NULL;
  if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )  return;
  pNewMccMsg->Init( NULL );
  pNewMccMsg->dwAID        = AP_BEFORE_ALL_MATCH;
  pNewMccMsg->dwMsgLen     = 1;
  pNewMccMsg->Msgs[0].Size = sizeof(SMatchNotice);
  LPSMatchNotice pResult = (LPSMatchNotice)pNewMccMsg->Msgs[0].Data;
  pResult->iMatchMode  = GetMatchMode();
  pResult->iMatchType  = GetMatchType();
  pResult->iBeforeTime = iBeforeTime/60;
  //
  g_pMccDB->AddSendMsg( pNewMccMsg );
}

void CFightFieldMgr::SendMcc_AP_MATCH_BEGIN()
{
  SMccMsgData     *pNewMccMsg = NULL;
  if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )  return;
  int icount = 0;
  pNewMccMsg->Init( NULL );
  pNewMccMsg->dwAID        = AP_MATCH_BEGIN;
  LPSMatchResult pResult = (LPSMatchResult)pNewMccMsg->Msgs[0].Data;
  for( int i = 0; i< GetFieldCount(); i++ )
  {
    //只要不是没有人参加
    if( m_pMatchResult[i].iResult != RESULT_NOJOIN_MOMATCH )
    {
      pResult[icount++] = m_pMatchResult[i];
      if( m_pMatchResult[i].iResult != RESULT_ALLJOIN_1PVS2P &&
        m_pMatchResult[i].iResult != RESULT_ALLJOIN_VS )
      {
        m_pMatchResult[i].bSend = TRUE;
      }
    }
  }
  if( icount > 0 ) pNewMccMsg->dwMsgLen = 1;
  else             pNewMccMsg->dwMsgLen = 0;
  pNewMccMsg->Msgs[0].Size = icount*sizeof(SMatchResult);
  //
  g_pMccDB->AddSendMsg( pNewMccMsg );
}

void CFightFieldMgr::SendMcc_AP_MATCH_END()
{
  SMccMsgData     *pNewMccMsg = NULL;
  if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )  return;
  int icount = 0;
  pNewMccMsg->Init( NULL );
  pNewMccMsg->dwAID        = AP_MATCH_END;
  LPSMatchResult pResult = (LPSMatchResult)pNewMccMsg->Msgs[1].Data;
  for( int i = 0; i< GetFieldCount(); i++ )
  {
  /*
  if( IsNoMatch() )
  {
  if( m_pMatchResult[i].iResult != RESULT_NOJOIN_MOMATCH )
  {
  pResult[icount++] = m_pMatchResult[i];
  }
  }
  //只要不是没有人参加
  else
  {
  if( m_pMatchResult[i].iResult == RESULT_ALLJOIN_BOTH_DRAW ||
  m_pMatchResult[i].iResult == RESULT_ALLJOIN_1P_WON    ||
  m_pMatchResult[i].iResult == RESULT_ALLJOIN_2P_WON    ||
  m_pMatchResult[i].iResult == RESULT_ALLJOIN_BOTH_WON )
  {
  pResult[icount++] = m_pMatchResult[i];
  }
  }*/
    if( ( m_pMatchResult[i].iResult != RESULT_NOJOIN_MOMATCH ) &&
      ( m_pMatchResult[i].bSend == FALSE ) )
    {
      pResult[icount++] = m_pMatchResult[i];
      m_pMatchResult[i].bSend = TRUE;
    }
  }
  if( icount > 0 )
  {
    pNewMccMsg->dwMsgLen = 2;
    pNewMccMsg->Msgs[1].Size = icount*sizeof(SMatchResult);
  }
  else  pNewMccMsg->dwMsgLen = 1;
  pNewMccMsg->Msgs[0].Size = sizeof(DWORD)+sizeof(SMatchNotice);
  *(DWORD*)pNewMccMsg->Msgs[0].Data = GetRestTime()/60;
  LPSMatchNotice pNotice = (LPSMatchNotice)(pNewMccMsg->Msgs[0].Data+sizeof(DWORD));
  pNotice->iMatchMode  = GetMatchMode();
  pNotice->iMatchType  = GetMatchType();
  pNotice->iBeforeTime = 0;
  g_pMccDB->AddSendMsg( pNewMccMsg );
  //
  SetNoMatch( FALSE );
}

//判断是否全部传送完毕
BOOL CFightFieldMgr::IsAllSend( int iSendCount )
{
  //检查是否还有玩家参加这局比赛
  BOOL bAlSend = TRUE;
  if( GetLayer() == 1 )
  {
    for( int i = iSendCount; i< GetBoutsByLayer(); i++ )
    {
      if( m_pChallengerTable[GetMatchType()][GetMatchId()][i] ) 
      {
        bAlSend = FALSE;
        break;
      }
    }
  }
  else
  {
    for( int i = iSendCount; i< GetBoutsByLayer(); i++ )
    {
      if( m_pResultTable[i] )
      {
        bAlSend = FALSE;
        break;
      }//if( m_pResultTable[i] )
    }
  }
  return bAlSend;
}


//输出比赛结果比赛结果
void CFightFieldMgr::MakeMatchResult( int  iField, 
                                     LPSNameTip pFighter1,
                                     LPSNameTip pFighter2,
                                     BOOL bWaitting1, 
                                     BOOL bWaitting2 )
{
  LPSMatchResult pMatchResult = &m_pMatchResult[iField];
  //有2个选手参加比赛
  if( pFighter1 && pFighter2 )
  {
    pMatchResult->sVSInfo[0] = *pFighter1;
    pMatchResult->sVSInfo[1] = *pFighter2;
    //都参加,1P不在场,2P赢
    if( bWaitting1 == FALSE && bWaitting2 == TRUE )  pMatchResult->iResult = RESULT_ALLJOIN_1PNOT_2P_WON;
    //都参加,2P不在场,1P赢
    else if( bWaitting1 == TRUE  && bWaitting2 == FALSE ) pMatchResult->iResult = RESULT_ALLJOIN_2PNOT_1P_WON;
    //都参加,都不在场,打平
    else if( bWaitting1 == FALSE && bWaitting2 == FALSE ) pMatchResult->iResult = RESULT_ALLJOIN_ALLNOT_DRAW;
    //都参加,正在比赛,1PVS2P
    else if( bWaitting1 == TRUE  && bWaitting2 == TRUE )  pMatchResult->iResult = RESULT_ALLJOIN_1PVS2P;
  }
  //只有1号选手参加比赛的情况
  else if( pFighter1 )
  {
    pMatchResult->sVSInfo[0] = *pFighter1;
    //只有1P参加,1号在场,1P赢
    if( bWaitting1 == TRUE )  pMatchResult->iResult = MATCH_1PJOIN_1P_WON;
    //只有1P参加,1号不在场,1P输
    else                      pMatchResult->iResult = MATCH_1PJOIN_1PNOT_1P_DRAW;
  }
  //只有2号选手参加比赛的情况
  else if( pFighter2 )
  {
    pMatchResult->sVSInfo[1] = *pFighter2;
    //只有2P参加,2号在场,2P赢
    if( bWaitting2 == TRUE )  pMatchResult->iResult = MATCH_2PJOIN_2P_WON;
    //只有2P参加,2号不在场,2P输
    else                      pMatchResult->iResult = MATCH_2PJOIN_2PNOT_2P_DRAW;
  }
}

//使死去的选手复活
void CFightFieldMgr::ReviveFighter( CPlayer* pPlayer )
{
  if( pPlayer == NULL || pPlayer->GetHp() > 1 ) return;
  pPlayer->SetHP(1);
  // Get Message Buffer Data
  SMsgData  *pNewMsg = g_pGs->NewMsgBuffer( pPlayer->GetSelfCode() );
  
  if( pNewMsg == NULL )   return;
  //
  pNewMsg->Init();
  pNewMsg->dwAID				= A_REVIVE;
  pNewMsg->dwMsgLen			= 1;
  pNewMsg->Msgs[0].Size = sizeof( SNMReviveData );
  
  // Switch Revive Type
  SNMReviveData   *pRevive = ( SNMReviveData* )( pNewMsg->Msgs[0].Data );
  
  pRevive->dwCode_Sp       = MAKELONG( pPlayer->GetSp(), pPlayer->GetSelfCode() );
  pRevive->dwX_Y					 = MAKELONG( pPlayer->GetPosX(), pPlayer->GetPosY() );
  pRevive->dwHp_Mp				 = MAKELONG( pPlayer->GetMp(), pPlayer->GetHp() );
  pRevive->dwExp					 = pPlayer->GetExp();
  pRevive->dwMoney				 = pPlayer->GetMoney();
  pRevive->wSkillId        = 0;
  pRevive->qwSpecialStatus = pPlayer->GetSpecialStatus();
  
  pPlayer->AddSendMsg( pNewMsg );
  // Send The Clear Code Message To Near Client, Inlude Self
  pPlayer->SetWarpType( PLAYER_WARP_TYPE_DEAD );
  pPlayer->ClearMyCodeInMap();
  
  // Set the data of the Revived Player
  pPlayer->SetState( STATUS_PLAYER_STAND );
  pPlayer->SetPoseState( STATUS_PLAYER_POSE_STAND );
  pPlayer->SetCanAction();
}

//对参赛人员随机排序
NAME_LIST  CFightFieldMgr::GetRandomList( int iMatchType )
{
  NAME_LIST nameList, swapList;
  NAME_LIST& signupList = GetSignupList( iMatchType );
  nameList = signupList;
  NAME_ITER iter;
  int isize = nameList.size(), ipos = 0, icur = 0;
  while( !nameList.empty() )
  {
    ipos = gf_GetRandom( isize );
    for( icur = 0, iter = nameList.begin();
    icur<ipos; icur++ )
    {
      iter++;
    }
    //
    swapList.push_back( (*iter) );
    nameList.erase( iter );
    isize--;
  }
  //
  signupList = swapList;
  return swapList;
}

void CFightFieldMgr::HandleBackMoney( int iModeType )
{
  CPlayer* pPlayer = NULL;
  NAME_LIST& signupList = GetSignupList( iModeType );
  for( NAME_ITER iter = signupList.begin(); 
  iter != signupList.end();
  iter++ )
  {
    switch( GetMatchMode() )
    {
    case MATCH_MODE_OCCUPATION:    //职业赛
    case MATCH_MODE_WEIGHT:        //量级赛
    case MATCH_MODE_LIVEMODE:      //生存赛
      {
        pPlayer = g_pGs->GetPlayerFromMailId( (*iter)->sNameTip.iId );
        if( pPlayer )
        {
          pPlayer->SetMoney( pPlayer->GetMoney() + MATCH_MONEY_COST );
          pPlayer->Send_A_SETMONEY();
        }
      }
      break;
    case MATCH_MODE_TEAM:          //团体赛
      {
        CTeam* pTeam = ::GetTeam( (*iter)->sNameTip.iId );
        switch( iModeType )
        {
        case MATCH_TYPE_2VS2:
          if( pTeam )
          {
            pPlayer = g_pGs->GetPlayerFromMailId( pTeam->GetLeaderMailId() );
            if( pPlayer )
            {
              pPlayer->SetMoney( pPlayer->GetMoney() + MATCH_MONEY_COST*2 );
              pPlayer->Send_A_SETMONEY();
            }
          }
          break;
        case MATCH_TYPE_4VS4:
          if( pTeam )
          {
            pPlayer = g_pGs->GetPlayerFromMailId( pTeam->GetLeaderMailId() );
            if( pPlayer )
            {
              pPlayer->SetMoney( pPlayer->GetMoney() + MATCH_MONEY_COST*4 );
              pPlayer->Send_A_SETMONEY();
            }
          }
          break;
        case MATCH_TYPE_6VS6:
          if( pTeam )
          {
            pPlayer = g_pGs->GetPlayerFromMailId( pTeam->GetLeaderMailId() );
            if( pPlayer )
            {
              pPlayer->SetMoney( pPlayer->GetMoney() + MATCH_MONEY_COST*6 );
              pPlayer->Send_A_SETMONEY();
            }
          }
          break;
        }//switch( iModeType )
      }
      break;
    }//switch( GetMatchMode() )
  }
}
/////
void CFightFieldMgr::DrawMatch( int iPos )
{
  if( m_pResultTable[iPos] ) 
  { 
    LPSChallenger pSelfInfo = m_pResultTable[iPos]->GetSelfInfo();
#ifdef ELYSIUM_3_7_SCORE_VERSION
    if( pSelfInfo )
    {
      //Team Mode
      int      iMailId = 0;
      CPlayer* pPlayer = NULL;
      if( g_FightFieldMgr.GetMatchMode() == MATCH_MODE_TEAM )
      {
        for( int i = 0; i<TEAM_MEMBER_COUNT_MAX; i++ )
        {
          iMailId = pSelfInfo->iTeamMailId[i];
          if( iMailId == 0 ) continue;
          pPlayer = g_pGs->GetPlayerFromMailId( iMailId );
          if( pPlayer && pPlayer->GetCalScore() )
          {
            pPlayer->SetScore( pPlayer->GetScore() + 1 );
            pPlayer->SetCalScore( FALSE );
          }
        }
      }
      //Other Mode
      else
      {
        iMailId = pSelfInfo->sNameTip.iId;
        pPlayer = g_pGs->GetPlayerFromMailId( iMailId );
        if( pPlayer && pPlayer->GetCalScore() )
        {
          pPlayer->SetScore( pPlayer->GetScore() + 1 );
          pPlayer->SetCalScore( FALSE );
        }
      }
    }
#endif
    //对输掉的选手加1分
    m_pResultTable[iPos]->AddScore( 1 );
    m_pResultTable[iPos]->SetJoin( FALSE );
  }
  m_pResultTable[iPos] = NULL;
};
//显示计时器
void CFightFieldMgr::Send_A_SHOWTIMER( CPlayer* pPlayer )
{
  if( pPlayer == NULL ) return;
  SMsgData			*pNewMsg  = g_pGs->NewMsgBuffer( pPlayer->GetSelfCode() );
  if( pNewMsg == NULL )	return;
  
  WORD* pwTime = (WORD*)(pNewMsg->Msgs[0].Data);
  
  pNewMsg->Init();
  pNewMsg->dwAID							= A_SHOWTIMER;
  pNewMsg->dwMsgLen						= 1;
  pNewMsg->Msgs[0].Size				= sizeof( WORD );
  *pwTime = m_iBaseRefTime[GetMatchMode()][GetMatchType()]/60;
  pPlayer->AddSendMsg( pNewMsg );  
}

//放怪
void CFightFieldMgr::CreateMonster( BOOL bCreate )
{
  //如果不是无限生存赛
  if( GetMatchMode() != MATCH_MODE_LIVEMODE ) return;
  //
  if( GetFightResult( 0 ) == NULL ) return;
  //     
  int iNum   = GetFightResult( 0 )->GetNum();
  int iMapId = GetFightResult( 0 )->GetMapId();
  //收回所有的怪物...
  CGameMap* pMap = g_pGs->GetGameMap( iMapId );
  if( pMap == NULL ) return;
  pMap->ClearMonster();
  //不创建就退出
  if( bCreate == FALSE ) return;
  //  
  SMsgData* pTheMsg = g_pGs->NewMsgBuffer();
  if( NULL == pTheMsg ) return;
  pTheMsg->Init();
  SNMNpcInfo  *pTheNpcInfo = (SNMNpcInfo*)(pTheMsg->Msgs[1].Data);
  CMonster		*pNewMonster = NULL;
  WORD				wCreateCount = 0;
  //
  LPSMapWarp  pFieldInfo = GetFieldInfo( 0 );
  SWarpPoint* pWarp1  = NULL; //pFieldInfo->GetPoint();
  int iMonsterId = 0;
  //根据人数来放怪
  for( int i = 0; i< GetStepCount(); i++ )
  {
    int iMonsterCount = m_pLiveMatchData[i][MONSTER_COUNT];
    if( iNum >= m_pLiveMatchData[i][FIGHTER_NUM_LL] &&
      iNum <= m_pLiveMatchData[i][FIGHTER_NUM_UL] )
    {
      for( int m = 0; m<iMonsterCount; m++ )
      {
        pWarp1     = pFieldInfo->GetPoint();
        iMonsterId = GetMonsterId( m_pLiveMatchData[i][MONSTER_ID] );
        //////////////////////////////////////////////////////////////////
        //Create Monster...
        pNewMonster = NULL;
        if( NULL != ( pNewMonster = pMap->CreateMonster( iMonsterId, ( pWarp1->wTargetMapX - 1 + gf_GetRandom( 2 ) ), ( pWarp1->wTargetMapY - 1 + gf_GetRandom( 2 ) ), 0, 0 ) ) )
        {
          pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
          pTheNpcInfo++;
          wCreateCount++;
        }
      }//for( int m = 0; m<m_pLiveMatchData[i][MONSTER_COUNT]; m++ )
      if( wCreateCount )
      {
        pTheMsg->dwAID        = A_PLAYERINFO;
        pTheMsg->dwMsgLen     = 2;
        pTheMsg->Msgs[0].Size = 1;
        pTheMsg->Msgs[1].Size = sizeof( SNMNpcInfo ) * wCreateCount;
        
        //pMap->SendTheMsgToAll( pTheMsg );
        pMap->SendMsgNearPosition( *pTheMsg, pWarp1->wTargetMapX, pWarp1->wTargetMapY );
      }
      g_pGs->ReleaseMsg( pTheMsg );
      pTheMsg = NULL;
      ////////////////////////////////////////////////////////////////////
      //
      break;
    }//if( iNum >= m_pLiveMatchData[i][FIGHTER_NUM_LL] &&
    //    iNum <= m_pLiveMatchData[i][FIGHTER_NUM_UL] )
  }
};
////////////////////////////////////////////////////////////////////////////
void CFighter::Win()
{
  //当前层数的分数
  int iAdd = m_iWinTime + 1;
  AddScore( iAdd );
  //
#ifdef ELYSIUM_3_7_SCORE_VERSION
  if( m_pSelfInfo )
  {
    //Team Mode
    int      iMailId = 0;
    CPlayer* pPlayer = NULL;
    if( g_FightFieldMgr.GetMatchMode() == MATCH_MODE_TEAM )
    {
      for( int i = 0; i<TEAM_MEMBER_COUNT_MAX; i++ )
      {
        iMailId = m_pSelfInfo->iTeamMailId[i];
        if( iMailId == 0 ) continue;
        pPlayer = g_pGs->GetPlayerFromMailId( iMailId );
        if( pPlayer && pPlayer->GetCalScore() )
        {
          pPlayer->SetScore( pPlayer->GetScore() + iAdd );
          pPlayer->SetCalScore( FALSE );
        }
      }
    }
    //Other Mode
    else
    {
      iMailId = m_pSelfInfo->sNameTip.iId;
      pPlayer = g_pGs->GetPlayerFromMailId( iMailId );
      if( pPlayer && pPlayer->GetCalScore() )
      {
        pPlayer->SetScore( pPlayer->GetScore() + iAdd );
        pPlayer->SetCalScore( FALSE );
      }
    }
  }
#endif
  //
  m_iWinTime++;
  if( m_pSelfInfo ) m_pSelfInfo->iWinTime = m_iWinTime;
  SetJoin( FALSE );
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////
//
//记录武斗大赛结果记录
#ifdef _FIGHT_RESULT_LOG_
void CFightFieldMgr::SendFightResultlog( LPCFighter  pFighter1, LPCFighter  pFighter2, WORD m_iMatchMode, int iMatchResult )
{
#ifdef _DEBUG_CLOSE_ALL_LOG_ABOUT_ITEM_
  return;
#endif
  if( g_pGs->GetMccMode() != 2 )      return;
  //
  if( m_wFightResultLog == 0 )  m_pFightResultLog = (SFightResultData*)(m_FightResultMsg.Msgs[0].Data);
  //logtime
  ::CopyTime( &m_pFightResultLog->time, &g_SysTime );  
  
  switch( m_iMatchMode ) 
  {
  case MATCH_MODE_OCCUPATION:    //职业赛
    m_pFightResultLog->iMatchMode = 1;
    break;
  case MATCH_MODE_WEIGHT:        //量级赛
    m_pFightResultLog->iMatchMode = 2;
    break;
  case MATCH_MODE_LIVEMODE:      //生存赛
    m_pFightResultLog->iMatchMode = 3;
    break;
  case MATCH_MODE_TEAM:          //团体赛
    m_pFightResultLog->iMatchMode = 4;
    break;
  }	
  m_wFightResultLog++;
  m_pFightResultLog++;
  //
  if( m_wFightResultLog >= 1 )
  {
    SMccMsgData     *pNewMccMsg = g_pGs->NewMccMsgBuffer( );
    if( pNewMccMsg == NULL )
    {
      if( NULL == ( pNewMccMsg = g_pGs->NewMccMsgBuffer() ) )
      {
        m_wFightResultLog--;
        m_pFightResultLog--;
        return;
      }
    }
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AC_LOG_FIGHT_RESULT;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(SFightResultHead) + sizeof(SFightResultData) * m_wFightResultLog;
    // Fill Head Data
    SFightResultHead     *pHeadData = (SFightResultHead*)pNewMccMsg->Msgs[0].Data;
    CPlayer *pPlayer1 = NULL, *pPlayer2 = NULL;
    if(NULL != pFighter1)
    {
      pHeadData->iValue1 = pFighter1->GetValue();
      pPlayer1 = g_pGs->GetPlayerFromMailId(pFighter1->GetSelfInfo()->sNameTip.iId);
    }
    else
    {
      pHeadData->iValue1 = -1;
    }
    if(NULL != pFighter2)
    {
      pHeadData->iValue2 = pFighter2->GetValue();
      pPlayer2 = g_pGs->GetPlayerFromMailId(pFighter2->GetSelfInfo()->sNameTip.iId);
    }
    else
    {
      pHeadData->iValue2 = -1;
    }
    if(NULL!= pFighter1)
    {
      memcpy(pHeadData->szAccount1,"NO RECORD",MAX_ACCOUNT_LEN);
      pHeadData->szAccount1[MAX_ACCOUNT_LEN-1]        = '\0';
      memcpy(pHeadData->szPlayerName1, pFighter1->GetSelfInfo()->sNameTip.szName,MAX_PLAYER_NAME_LEN);
      pHeadData->szPlayerName1[MAX_PLAYER_NAME_LEN-1] = '\0';
    }
    else
    {
      strcpy(pHeadData->szAccount1,"NO RECORD");
      pHeadData->szAccount1[MAX_ACCOUNT_LEN-1] = '\0';
      strcpy(pHeadData->szPlayerName1,"Died");
      pHeadData->szPlayerName1[MAX_PLAYER_NAME_LEN-1] = '\0'; 
    }
    if(NULL!= pFighter2)
    {
      memcpy(pHeadData->szAccount2,"NO RECORD",MAX_ACCOUNT_LEN);
      pHeadData->szAccount2[MAX_ACCOUNT_LEN-1]        = '\0';
      memcpy(pHeadData->szPlayerName2, pFighter2->GetSelfInfo()->sNameTip.szName,MAX_PLAYER_NAME_LEN);
      pHeadData->szPlayerName2[MAX_PLAYER_NAME_LEN-1] = '\0';
      
    }
    else
    {
      strcpy(pHeadData->szAccount2,"NO RECORD");
      pHeadData->szAccount2[MAX_ACCOUNT_LEN-1] = '\0';
      strcpy(pHeadData->szPlayerName2,"Died");
      pHeadData->szPlayerName2[MAX_PLAYER_NAME_LEN-1] = '\0';
    }
    pHeadData->iMatchResult = iMatchResult;
    // Fill Resy Data
    memcpy( ( pNewMccMsg->Msgs[0].Data + sizeof(SFightResultHead) ), &m_FightResultMsg.Msgs[0].Data,
      sizeof(SFightResultData) * m_wFightResultLog );
    // Send
    g_pMccChat->AddSendMsg( pNewMccMsg );
    // Reinit Message
    m_wFightResultLog = 0;
    m_pFightResultLog = (SFightResultData*)(m_FightResultMsg.Msgs[0].Data); 
  }
}
#endif // #ifdef LOG
//
#ifdef _MODIFY_GUID_TEAMMACTH_
void CFightFieldMgr::HandleMatchResult(void)
{
	if (GetMatchMode() != MATCH_MODE_TEAM)	 return;
	if (GetSignupNum(MATCH_TYPE_6VS6) < 2)   return;
	//
	int iCount = SortMatchResult();
	SendMcc_MatchResult(iCount);
}
//
int CFightFieldMgr::SortMatchResult(void)
{
	int i,j,p,iCnt;
	CFighter temFigher;
	
	iCnt = TOTAL_MATCH_TIMES;
	for (i = 0; i < iCnt; ++ i)
	{
		p = i;
		if (NULL == m_Records[p].GetSelfInfo())
			break;
		for (j = p+1; j < iCnt; ++j)
		{
			if (NULL == m_Records[j].GetSelfInfo())
				break;
			if (m_Records[j].GetSelfInfo()->iWinTime >
				  m_Records[p].GetSelfInfo()->iWinTime)
			{
				p = j;
			}
		} // end for j
		if (p != i)
		{
			temFigher    = m_Records[i];
			m_Records[i] = m_Records[p];
			m_Records[p] = temFigher;
		}
	} //end for i
	//
	return i;
}
//
void CFightFieldMgr::SendMcc_MatchResult(int iRecord)
{
	int iCnt,iItemID[MAX_GUID_NUM];
	int i;
	char szInitLog[MAX_MEMO_MSG_LEN];

	//-------------读取奖品档----------------
	/*比赛结束后，系统自动回补奖品：
    第1名给予(2522)帮会大礼包
    第2名给予(2523)帮会大礼包
    第3、4名给予奖励品(2524)帮会大礼包。*/
#ifdef _REPAIR_SERVER_CRASH_NICK_
	SafeStrcpy( szInitLog, g_pBase->GetObjectFilePath(), MAX_MEMO_MSG_LEN );
#else
  strcpy( szInitLog, g_pBase->GetObjectFilePath() );
#endif
  strcat( szInitLog, BF_AWARD_ITEM_FILE );
	CInStream  AwardItem(szInitLog);
  assert(AwardItem);
	if (!AwardItem.fail() && AwardItem.GetFileSize() != 0 ) 
  {
		AwardItem >> iCnt;
		if (iCnt != MAX_GUID_NUM)
		{
			MessageBox( GetActiveWindow(), "guidaward.txt is error", "Warning..." ,MB_OK );
			return;
		}
		//读档时,奖品按帮会团体比赛所得的的名次依次读取(1--->4)
		for (i=0; i < iCnt; ++i)	AwardItem >> iItemID[i];
	}
	else
  {
    MessageBox( GetActiveWindow(), "Cannot Open guidaward.txt", "Warning..." ,MB_OK );
    return;
  }
	//整理帮会的结果
	int iMailId;
	CPlayer  *pPlayer;
	CTeam    *pTeam;
	if (iRecord > MAX_GUID_NUM)   iRecord = MAX_GUID_NUM;
	memset(&m_sGuidResult, 0, sizeof(SGuildMatchResult));
	m_sGuidResult.m_wMatchMode = GetMatchMode();
	m_sGuidResult.m_wMatchType = GetSignupNum(MATCH_TYPE_6VS6);
	for (i=0,	iCnt=0; iCnt < iRecord; ++i)
	{
		if (i >= TOTAL_MATCH_TIMES)              return;
		if (NULL == m_Records[i].GetSelfInfo())  continue;
		iMailId = m_Records[i].GetSelfInfo()->iTeamMailId[0];
		pPlayer = g_pGs->GetPlayerFromMailId( iMailId );
		if (NULL == pPlayer)                     continue;
		if (NULL == (pTeam =pPlayer->GetTeam())) continue;
		m_sGuidResult.m_sResultData[iCnt].m_wTeamId        = pTeam->GetTeamID();
		m_sGuidResult.m_sResultData[iCnt].m_dwLeaderMailId = pTeam->GetLeaderMailId();
		m_sGuidResult.m_sResultData[iCnt].m_wAwardOrder    = iCnt+1;
		m_sGuidResult.m_sResultData[iCnt].m_iItemId        = iItemID[iCnt];
		m_sGuidResult.m_sResultData[iCnt].m_iItemCnt       = 1;
		++iCnt;
	#ifdef ELYSIUM_3_7_VERSION
		pPlayer->SendTeamMatchSignUp(m_Records[i].GetSelfInfo(), MATCH_TYPE_6VS6, iCnt );
  #endif
	}
	//发公告：本次帮会团体赛已经结束。
  // if (pPlayer)  pPlayer->SendTalkToAll("セΩ腊穦刮砰辽竒挡");
	//向DBMcc发送帮会团体比赛的总结果
	SGuildResultData *pResultData;
	SMccMsgData*  pMccMsg = g_pGs->NewMccMsgBuffer();
	if(NULL == pMccMsg)	      	return;
	pMccMsg->Init(NULL);
	pMccMsg->dwAID = AP_GUILD_MATCH_RESULT;
	pMccMsg->dwMsgLen = 1;
	pMccMsg->Msgs[0].Size = iRecord * sizeof(SGuildResultData);
	pResultData = (SGuildResultData *)pMccMsg->Msgs[0].Data;
  for (i=0; i < iRecord; ++i, ++pResultData)
		*pResultData = m_sGuidResult.m_sResultData[i];
	g_pMccDB->AddSendMsg(pMccMsg);
}
//用来判断帮会团体比赛: 首先比对双方人数，人数多者为胜，其次比对比分，比分高者为胜
//通过返回值来判断当前的帮会团体比赛是否已经有了结果
bool CFightFieldMgr::JudeGuidMatchResult(LPCFighter pFighter1,
		                                     LPCFighter pFighter2,
													               LPSMatchResult pMatchResult,
													               int iRes,int *pDraw1,int *pDraw2)
{
	bool bIsResult=false;
	//
 	if (GetMatchMode() != MATCH_MODE_TEAM)	 return bIsResult;
 	if (GetSignupNum(MATCH_TYPE_6VS6) < 2)   return bIsResult;
	//
	if (pFighter1->GetNum() > pFighter2->GetNum())
	{
  #ifdef _FIGHT_RESULT_LOG_
		SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
  #endif
		pFighter1->Win();
		DrawMatch( iRes+1 ); *pDraw2 = iRes+1;
		//1队比2队人多,1P赢
		pMatchResult->iResult = RESULT_ALLJOIN_1P_WON;
		bIsResult = true;
	}
	else if (pFighter1->GetNum() < pFighter2->GetNum())
	{
  #ifdef _FIGHT_RESULT_LOG_
		SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
  #endif
		DrawMatch( iRes+0 ); *pDraw1 = iRes+0;
		pFighter2->Win();
		//2队比1队人多,2P赢
		pMatchResult->iResult = RESULT_ALLJOIN_2P_WON;
		bIsResult = true;
	}
	else if (pFighter1->GetNum() == pFighter2->GetNum())
	{
		//双方人数相同时，比较分数
		if( pFighter1->GetValue() > pFighter2->GetValue() )
		{
     #ifdef _FIGHT_RESULT_LOG_
			SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 1 );
     #endif
			pFighter1->Win();
			DrawMatch( iRes+1 ); *pDraw2 = iRes+1;
			//1队分多,1P赢
			pMatchResult->iResult = RESULT_ALLJOIN_1P_WON;
		}
		//Draw
		else if( pFighter1->GetValue() == pFighter2->GetValue() )
		{
     #ifdef _FIGHT_RESULT_LOG_
			SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 0 );
     #endif
			DrawMatch( iRes+0 ); *pDraw1 = iRes+0;
			DrawMatch( iRes+1 ); *pDraw2 = iRes+1;
			//2队分相同,打平
			pMatchResult->iResult = RESULT_ALLJOIN_BOTH_DRAW;
		}
		//Fail
		else//( pFighter1->GetValue() < pFighter2->GetValue() )
		{
     #ifdef _FIGHT_RESULT_LOG_
			SendFightResultlog( pFighter1, pFighter2, m_iMatchMode, 2 );
     #endif
			DrawMatch( iRes+0 ); *pDraw1 = iRes+0;
			pFighter2->Win();
			//2队分多,2P赢
			pMatchResult->iResult = RESULT_ALLJOIN_2P_WON;
		}
		//
		bIsResult = true;
	}
	//
	return bIsResult;
}

#endif//_MODIFY_GUID_TEAMMACTH_
//Add By HWH
#ifdef _VERSION40_CARRYDOOR
void CCarryDoor::SetCode()
{
  CGameMap * pMap =NULL;
  pMap = g_pGs->GetGameMap(m_CarryPoint.MapId);
  if(NULL == pMap)
    return;
  m_wCode = pMap->GetCodeWarp().GetNew();
}
void CCarryDoor::RandomMp(SMapPoint* Mp)
{
  WORD wVar = rand()%m_wAimNum;
  Mp->MapId = m_vecAimMp[wVar].MapId;
  Mp->x = m_vecAimMp[wVar].x;
  Mp->y = m_vecAimMp[wVar].y;
}

void CDoorManager::SetDoorOn(DWORD dwNowTime)
{
  CGameMap* pMap = NULL;
  CCarryDoor * pCarryDoor = NULL;
  SNMNpcInfo    *pWarpInfo   = (SNMNpcInfo*)(MsgEvilWarp.Msgs[1].Data);
  int iSrvX = 0;
  int iSrvY = 0;
  for(int i = m_wNowOpen*m_wDoorNum; i< (m_wNowOpen+1)*m_wDoorNum; i++)
  {
    for(vector<CCarryDoor*>::iterator iter_Cd = m_vecCarryDoor.begin(); iter_Cd != m_vecCarryDoor.end(); iter_Cd++)
    {
      pCarryDoor = *iter_Cd;
      pMap = g_pGs->GetGameMap(pCarryDoor->GetCarryPoint().MapId);
      if(NULL == pMap) continue;
      if(MULTPOLL != pCarryDoor->GetTimeCtlType() ) continue;
      iSrvX = pCarryDoor->GetCarryPoint().x;
      iSrvY = pCarryDoor->GetCarryPoint().y;
      if(pCarryDoor->GetMailId() == m_pwMailId[i])
      {
        pCarryDoor->SetStatus(CARRYDOOR_OPEN);
        pCarryDoor->GetNMDoorInfo(pWarpInfo);
        if(dwNowTime - pCarryDoor->m_dwLoopTime > 1500)
        {
          pMap->SendMsgNearPosition_Far(MsgEvilWarp,iSrvX,iSrvY);
          pCarryDoor->m_dwLoopTime = dwNowTime;
        }
      }
      else
      {
        pCarryDoor->SetStatus(CARRYDOOR_CLOSE);
        *(DWORD*)MsgClearCode.Msgs[0].Data = pCarryDoor->GetCode();
        if(dwNowTime - pCarryDoor->m_dwLoopTime > 1000)
        {
          pMap->SendMsgNearPosition_Far(MsgClearCode,iSrvX,iSrvY);
          pCarryDoor->m_dwLoopTime = dwNowTime;
        }
      }
    }
  }
}
void CCarryDoor::GetNMDoorInfo(SNMNpcInfo * pInfo)
{
  int iSrvX = m_CarryPoint.x;
  int iSrvY = m_CarryPoint.y;
  CGameMap* pMap = g_pGs->GetGameMap(m_CarryPoint.MapId);
  if(NULL == pMap) return;
  pInfo->dwCode_PicId     = ( m_wCode << 16 ) | 1;
  pInfo->dwX_Y            = ( iSrvX << 16 ) | iSrvY;
  pInfo->dwState_Dir_Id   = ( m_wStatus << 16 ) | 0;
  pInfo->dwHp             = 0;
  pInfo->dwMaxHp          = 0;
  pInfo->qwSpecialStatus  = 0;    
}
void CDoorManager::TimeLoopSelf(DWORD dwNowTime)//自己轮询
{
  CGameMap* pMap = NULL;
  CCarryDoor * pCarryDoor = NULL;
  SNMNpcInfo    *pWarpInfo   = (SNMNpcInfo*)(MsgEvilWarp.Msgs[1].Data);
  int iSrvX = 0;
  int iSrvY = 0;
  for(vector<CCarryDoor*>::iterator iter_Cd = m_vecCarryDoor.begin(); iter_Cd != m_vecCarryDoor.end(); iter_Cd++)
  {
    pCarryDoor = *iter_Cd;
    if(!pCarryDoor->m_bFirstOn)
    {
      pCarryDoor->m_dwEndTime = dwNowTime + m_dwTimeOutSelf;
      pCarryDoor->m_bFirstOn = TRUE;
    }
    pMap = g_pGs->GetGameMap(pCarryDoor->GetCarryPoint().MapId);
    if(NULL == pMap) continue;      
    iSrvX = pCarryDoor->GetCarryPoint().x;
    iSrvY = pCarryDoor->GetCarryPoint().y;
    if(SELFPOLL == pCarryDoor->GetTimeCtlType())
    {
/*#ifdef _DEBUG
      int iiiii = (dwNowTime - m_dwStartTime)/m_dwTimeOutSelf+1;
#endif
      if(((dwNowTime - m_dwStartTime)/m_dwTimeOutSelf+1)&1) //在开放时间内*/
      if(dwNowTime > pCarryDoor->m_dwStartTime && dwNowTime < pCarryDoor->m_dwEndTime)
      {
        pCarryDoor->m_bOn = TRUE;
        pCarryDoor->SetStatus(CARRYDOOR_OPEN);
        pCarryDoor->GetNMDoorInfo(pWarpInfo);
        if(dwNowTime - pCarryDoor->m_dwLoopTime > 1500)
        {
          pMap->SendMsgNearPosition_Far(MsgEvilWarp,iSrvX,iSrvY);
          pCarryDoor->m_dwLoopTime = dwNowTime;
        }
      }
      else
      {
        if(pCarryDoor->m_bOn)
        {
          pCarryDoor->m_dwStartTime = dwNowTime + m_dwSelfInter;
          pCarryDoor->m_dwEndTime = dwNowTime + m_dwSelfInter + m_dwTimeOutSelf;
          pCarryDoor->m_bOn = FALSE;
        }
        pCarryDoor->SetStatus(CARRYDOOR_CLOSE);
        *(DWORD*)MsgClearCode.Msgs[0].Data = pCarryDoor->GetCode();
        if(dwNowTime - pCarryDoor->m_dwLoopTime > 1500)
        {
          pMap->SendMsgNearPosition_Far(MsgClearCode,iSrvX,iSrvY);
          pCarryDoor->m_dwLoopTime = dwNowTime;
        }
      }
    }
    if( NOPOLL == pCarryDoor->GetTimeCtlType() )
    {
      pCarryDoor->GetNMDoorInfo(pWarpInfo);
      if(dwNowTime - pCarryDoor->m_dwLoopTime > 1500)
      {
        pMap->SendMsgNearPosition_Far(MsgEvilWarp,iSrvX,iSrvY);
        pCarryDoor->m_dwLoopTime = dwNowTime;
      }
    }
  }
}

void CDoorManager::TimeLoopMult(DWORD dwNowTime)//多组之间轮循
{
  DWORD dwTimeOutCount = dwNowTime - m_dwStartTime;
  m_wNowOpen = (dwTimeOutCount/m_dwTimeOut)%m_wGroupNum;
  SetDoorOn(dwNowTime);  
}


BOOL CDoorManager::DoAction(CGameMap* pMap, WORD wX, WORD wY, SWarpPoint * Wp)
{
  if( !pMap ) return FALSE;
  static int iSrvX = 0;
  static int iSrvY = 0;
  static int iClientX = 0;
  static int iClientY = 0;
  static SMapPoint MapPoint;          
  memset(&MapPoint,0,sizeof(SMapPoint));
  iSrvX = wX;
  iSrvY = wY;
  for(vector<CCarryDoor*>::iterator iter_Cd = m_vecCarryDoor.begin(); iter_Cd != m_vecCarryDoor.end(); iter_Cd++)
  {
    CCarryDoor* pCarryDoor = *iter_Cd;
    if( (pCarryDoor->GetCarryPoint().MapId == pMap->GetMapId()) && (CARRYDOOR_OPEN == pCarryDoor->GetStatus()))
    {
      if( (((iSrvX-1) <= pCarryDoor->GetCarryPoint().x) && (pCarryDoor->GetCarryPoint().x <= (iSrvX+1))) && 
          (((iSrvY-1) <= pCarryDoor->GetCarryPoint().y) && (pCarryDoor->GetCarryPoint().y <= (iSrvY+1)))) 
      {
        switch(pCarryDoor->GetType())
        {
        case CARRYDOOR_RANDOM:
          {
            pCarryDoor->RandomMp(&MapPoint);
            Wp->wType = WARPPOINT_TYPE_EVIL;
            Wp->wCode = pCarryDoor->GetCode();
            Wp->wMapId = pCarryDoor->GetCarryPoint().MapId;
            Wp->wPosX = pCarryDoor->GetCarryPoint().x;
            Wp->wPosY = pCarryDoor->GetCarryPoint().y;
            Wp->wTargetMapId = MapPoint.MapId;
            iClientX = MapPoint.x;
            iClientY = MapPoint.y;
            //pMap->ConvertCli2Srv(&iClientX,&iClientY);
            Wp->wTargetMapX = iClientX;
            Wp->wTargetMapY = iClientY;
            
            return TRUE;
          }
          break;
        case CARRYDOOR_FIX:
          {
            Wp->wType = WARPPOINT_TYPE_EVIL;
            Wp->wCode = pCarryDoor->GetCode();
            Wp->wMapId = pCarryDoor->GetCarryPoint().MapId;
            Wp->wPosX = pCarryDoor->GetCarryPoint().x;
            Wp->wPosY = pCarryDoor->GetCarryPoint().y;
            Wp->wTargetMapId = pCarryDoor->m_vecAimMp[0].MapId;
            iClientX = pCarryDoor->m_vecAimMp[0].x;
            iClientY = pCarryDoor->m_vecAimMp[0].y;
            //pMap->ConvertCli2Srv(&iClientX,&iClientY);
            Wp->wTargetMapX = iClientX;
            Wp->wTargetMapY = iClientY;
            
            return TRUE;
          }
          break;
        default:
          break;
        }
      }
    }   
  }
  return FALSE;
}

CCarryDoor* CDoorManager::GetDoorFromCode(WORD wCode)
{
  CCarryDoor * pCarryDoor = NULL;
  vector<CCarryDoor*>::iterator iter_CDoor = m_vecCarryDoor.begin();
  for(; iter_CDoor != m_vecCarryDoor.end(); iter_CDoor++)
  {
    pCarryDoor = *iter_CDoor;
    if(wCode == pCarryDoor->GetCode())
      return pCarryDoor;
  }
  return NULL;
}
//-------------------------------------------------------------------------------
void CDoorManager::ConvertCli2Srv()
{
  DWORD dwMapId = 0;
  SMapPoint MapPoint;
  memset(&MapPoint,0,sizeof(SMapPoint));
  CGameMap * pMap = NULL;
  CCarryDoor * pCarryDoor = NULL;
  int iCliX = 0;
  int iCliY = 0;
  vector<CCarryDoor*>::iterator iter_CDoor = m_vecCarryDoor.begin();
  for(; iter_CDoor != m_vecCarryDoor.end(); iter_CDoor++)
  {
    pCarryDoor = *iter_CDoor;
    MapPoint = pCarryDoor->GetCarryPoint();
    iCliX = MapPoint.x;
    iCliY = MapPoint.y;
    dwMapId = MapPoint.MapId;
    pMap = g_pGs->GetGameMap(dwMapId);
    if( pMap )
    {
      pMap->ConvertCli2Srv(&iCliX, &iCliY);
      MapPoint.x = iCliX;
      MapPoint.y = iCliY;
      pCarryDoor->SetCarryPoint(&MapPoint);
    }
  }
}
//---------------------------------------------------------------------------------
#endif // #ifdef CARRYDOOR
//---------------------------------------------------------------------------------
#ifdef VERSION_40_HOUSE_FUNCTION
void CGameMap::AddAllPlayerHPSPMP(int iHP,int iSP,int iMP)
{
  CPlayer		*pPlayer = NULL;
  if( m_mapCodePlayer.empty() )
  {
    return;
  }
  map<DWORD, CPlayer*>::iterator        Iter_Pl;
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pPlayer = Iter_Pl->second;
    if( pPlayer )
    {
      pPlayer->AddHp(iHP);
      pPlayer->AddSp(iSP);
      pPlayer->AddMp(iMP);
      pPlayer->Send_A_UPDATEHPMPSP(FALSE);
    }
  }
}
void CGameMap::AddAllPlayerSpecialTime(QWORD qwSpe, DWORD dwTime, int iNum)
{
  CPlayer		*pPlayer = NULL;
  if( m_mapCodePlayer.empty() )
  {
    return;
  }
  map<DWORD, CPlayer*>::iterator        Iter_Pl;
  for( Iter_Pl = m_mapCodePlayer.begin(); Iter_Pl != m_mapCodePlayer.end(); Iter_Pl++ )
  {
    pPlayer = Iter_Pl->second;
    if( pPlayer && pPlayer->GetSpecialStatus()&qwSpe )
    {
      if(qwSpe == SPE_STATUS_EXP)
      {
        pPlayer->ChangeStatusTime(dwTime,iNum);
        pPlayer->Send_A_SETDYSFUNC(NULL,2575);
      }
      else
      {
        //...other status
      }
    }
    else if( pPlayer )
    {
      if(qwSpe == SPE_STATUS_EXP)
      {

        pPlayer->ChangeStatusTime(dwTime,iNum,TRUE);
        CSrvBaseSkill *pSkill = g_pBase->GetBaseSkill(2575);
        if(pSkill)
        {
          pSkill->SetSpecialState((CLife*)pPlayer);
          pPlayer->Send_A_SETDYSFUNC(NULL,2575);
        }
      }
      else
      {
        //...other status
      }
    }
    else
    {
      //error
    }
  }
}
void CGameMap::GetViewMonsters(WORD* wId,WORD* X, WORD* Y)
{
  map<DWORD,CMonster*>::iterator		Iter;
  for(int i = 0; i < 5; i++)
  {
    wId[i] = 0;
    X[i] = 0;
    Y[i] = 0;
  }
  if(m_mapCodeMonster.size())
  {
    Iter = m_mapCodeMonster.begin();
    for(int i = 0; (i<5&&i<m_mapCodeMonster.size()&&Iter!=m_mapCodeMonster.end()); i++,Iter++)
    {
      CMonster * pMonter = Iter->second;
      if(pMonter->GetBaseId() < 7)
        continue;
      wId[i] = pMonter->GetBaseId();
      X[i] = pMonter->GetPosX();
      Y[i] = pMonter->GetPosY();
    }
  }
}
void CGameMap::DelAllMonster()
{
  map<DWORD,CMonster*>::iterator Iter;
  if(m_mapCodeMonster.size())
  {
    Iter = m_mapCodeMonster.begin();
    for(;Iter!=m_mapCodeMonster.end();)
    {
      CMonster * pMonter = Iter->second;
      DelMonster(pMonter->GetSelfCode(), &Iter);
    }
  }
}
BOOL CHouse::BeTaken(void *pHouseOwner, int iType)
{
  if(m_iType!=0 || m_pHouseOwner!=NULL) return FALSE;
  if(iType>3||iType<1) return FALSE;
  m_pHouseOwner = pHouseOwner;
  m_iType = iType;
  if(m_iType == T_GUILD)
  {
    ((CGuild*)(m_pHouseOwner))->SetHaveHouse(TRUE);
    ((CGuild*)(m_pHouseOwner))->SendBuyHouse();
  }
  m_iStatus = TAKEN;
  m_dwStatusCheckTime = TAKEN_STATUS_CHANGE_TIME;
  CGameMap * pTheMap = g_pGs->GetGameMap(m_wBelongMapId);
  if(pTheMap)
  {
    CNpc * pNpc = pTheMap->GetNpc(m_dwHouseNpcCode);
    {
      if(pNpc)
      {
        pNpc->SetHouseStatus(m_iStatus);
        SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
        if(pNewMsg)
        {
          pNewMsg->Init();
          pNewMsg->dwAID=A_HOUSENPC;
          pNewMsg->dwMsgLen = 2;
          pNewMsg->Msgs[0].Size = sizeof(DWORD);
          pNewMsg->Msgs[1].Size = sizeof(DWORD);
          *(DWORD*)pNewMsg->Msgs[0].Data = pNpc->GetSelfCode();
          *(DWORD*)pNewMsg->Msgs[1].Data = m_iStatus;
          pTheMap->SendTheMsgToAll(*pNewMsg);
          g_pGs->ReleaseMsg(pNewMsg);
        }
      }
    }
  }
  return TRUE;
}
BOOL CHouse::BeDebility()
{
  if(m_iType==0 || m_pHouseOwner==NULL) return FALSE;
  if(m_iStatus != TAKEN) return FALSE;
  m_iStatus = DEBILITY;
  m_dwStatusCheckTime = DEBILITY_STATUS_CHANGE_TIME;
  CGameMap * pTheMap = g_pGs->GetGameMap(m_wBelongMapId);
  if(pTheMap)
  {
    CNpc * pNpc = pTheMap->GetNpc(m_dwHouseNpcCode);
    {
      if(pNpc)
      {
        pNpc->SetHouseStatus(m_iStatus);
        SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
        if(pNewMsg)
        {
          pNewMsg->Init();
          pNewMsg->dwAID=A_HOUSENPC;
          pNewMsg->dwMsgLen = 2;
          pNewMsg->Msgs[0].Size = sizeof(DWORD);
          pNewMsg->Msgs[1].Size = sizeof(DWORD);
          *(DWORD*)pNewMsg->Msgs[0].Data = pNpc->GetSelfCode();
          *(DWORD*)pNewMsg->Msgs[1].Data = m_iStatus;
          pTheMap->SendTheMsgToAll(*pNewMsg);
          g_pGs->ReleaseMsg(pNewMsg);
        }
      }
    }
  }
  return TRUE;
}
BOOL CHouse::BeNoTaken()
{  
  if(m_iType==0 || m_pHouseOwner==NULL) return FALSE;
  if(m_iStatus != DEBILITY) return FALSE;
  if(m_iType == T_GUILD)
  {
    ((CGuild*)(m_pHouseOwner))->SetHaveHouse(FALSE);
    ((CGuild*)(m_pHouseOwner))->SendNoHouse();
  }
  SendLogHouseAction(ACTION_NOTAKE);
  m_iType = 0;
  m_pHouseOwner = NULL;
  m_iStatus = NOTAKEN;
  m_dwStatusCheckTime = 0;
  m_iBackMusic = 0;
  ZeroMemory(&m_SMessage,sizeof(m_SMessage));
  CGameMap * pHouseMap = g_pGs->GetGameMap(m_wHouseMapId);
  if(pHouseMap)
  {
    pHouseMap->DelAllMonster();
  }
  CGameMap * pTheMap = g_pGs->GetGameMap(m_wBelongMapId);
  if(pTheMap)
  {
    CNpc * pNpc = pTheMap->GetNpc(m_dwHouseNpcCode);
    {
      if(pNpc)
      {
        pNpc->SetHouseStatus(m_iStatus);
        SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
        if(pNewMsg)
        {
          pNewMsg->Init();
          pNewMsg->dwAID=A_HOUSENPC;
          pNewMsg->dwMsgLen = 2;
          pNewMsg->Msgs[0].Size = sizeof(DWORD);
          pNewMsg->Msgs[1].Size = sizeof(DWORD);
          *(DWORD*)pNewMsg->Msgs[0].Data = pNpc->GetSelfCode();
          *(DWORD*)pNewMsg->Msgs[1].Data = m_iStatus;
          pTheMap->SendTheMsgToAll(*pNewMsg);
          g_pGs->ReleaseMsg(pNewMsg);
        }
      }
    }
  }
  return TRUE;
}
BOOL CHouse::ReTaken()
{
  if(m_iType==0 || m_pHouseOwner==NULL) return FALSE;
  if(m_iStatus == NOTAKEN) return FALSE;
  m_iStatus = TAKEN;
  m_dwStatusCheckTime = m_dwStatusCheckTime + TAKEN_STATUS_CHANGE_TIME;
  if(m_dwStatusCheckTime > TAKEN_STATUS_MAX_TIME)
    m_dwStatusCheckTime = TAKEN_STATUS_MAX_TIME;
  CGameMap * pTheMap = g_pGs->GetGameMap(m_wBelongMapId);
  if(pTheMap)
  {
    CNpc * pNpc = pTheMap->GetNpc(m_dwHouseNpcCode);
    {
      if(pNpc)
      {
        pNpc->SetHouseStatus(m_iStatus);
        SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
        if(pNewMsg)
        {
          pNewMsg->Init();
          pNewMsg->dwAID=A_HOUSENPC;
          pNewMsg->dwMsgLen = 2;
          pNewMsg->Msgs[0].Size = sizeof(DWORD);
          pNewMsg->Msgs[1].Size = sizeof(DWORD);
          *(DWORD*)pNewMsg->Msgs[0].Data = pNpc->GetSelfCode();
          *(DWORD*)pNewMsg->Msgs[1].Data = m_iStatus;
          pTheMap->SendTheMsgToAll(*pNewMsg);
          g_pGs->ReleaseMsg(pNewMsg);
        }        
      }
    }
  }
  return TRUE;
}
//

void CHouse::CheckSatusAction(DWORD &dwNowTime)
{
  if(m_iStatus == NOTAKEN) return;
  if(dwNowTime>m_dwNextActionTime)
  {
    m_dwNextActionTime = dwNowTime + 300000;
    m_dwStatusCheckTime-=300000;
    /*
    #ifdef _DEBUG
    char szTemp[100];
    FuncName("CheckSatusAction()");
    _snprintf(szTemp,100,"%d,%d",m_wHouseMapId,m_dwStatusCheckTime);
    Fprintf("3.txt","%s\n",szTemp);
    AddMemoMsg(szTemp);
    #endif
    */
    switch(m_iStatus)
    {
    case TAKEN:
      {
        if(m_dwStatusCheckTime==0||m_dwStatusCheckTime>TAKEN_STATUS_MAX_TIME)
        {
          if(!BeDebility())
          {
            //Log error
#ifdef _DEBUG
            //g_ShowMessage.Show("House Cannot Be Debility!");
#endif
            m_dwStatusCheckTime = 0;
            return;
          }
          SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
          if(pNewMsg)
          {
            pNewMsg->Init();
            pNewMsg->dwAID = A_HOUSEEXPIRE;
            pNewMsg->dwMsgLen = 2;
            pNewMsg->Msgs[0].Size = sizeof(char)*MAX_GUILD_NAME_LEN;
            pNewMsg->Msgs[1].Size = sizeof(DWORD);
            strcpy((char*)pNewMsg->Msgs[0].Data,((CGuild*)GetHouseOwner())->GetName());
            *(DWORD*)pNewMsg->Msgs[1].Data = 2;
            g_pGs->SendTheMsgToAll(*pNewMsg);
            g_pGs->ReleaseMsg(pNewMsg);
          }
        }
      }
      break;
    case DEBILITY:
      {
        if(m_dwStatusCheckTime==0||m_dwStatusCheckTime>DEBILITY_STATUS_CHANGE_TIME)
        {
          SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
          if(pNewMsg)
          {
            pNewMsg->Init();
            pNewMsg->dwAID = A_HOUSEEXPIRE;
            pNewMsg->dwMsgLen = 2;
            pNewMsg->Msgs[0].Size = sizeof(char)*MAX_GUILD_NAME_LEN;
            pNewMsg->Msgs[1].Size = sizeof(DWORD);
            strcpy((char*)pNewMsg->Msgs[0].Data,((CGuild*)GetHouseOwner())->GetName());
            *(DWORD*)pNewMsg->Msgs[1].Data = 1;
            g_pGs->SendTheMsgToAll(*pNewMsg);
            g_pGs->ReleaseMsg(pNewMsg);
          }
          if(!BeNoTaken())
          {
            //Log error
#ifdef _DEBUG
            //g_ShowMessage.Show("House Cannot Be NoTaken!");
#endif
            m_dwStatusCheckTime = 0;
            return;
          }
          CGameMap* pHouseMap = g_pGs->GetGameMap(m_wHouseMapId);
          if(pHouseMap)
          {
            pHouseMap->WarpAll(m_wBelongMapId);
          }
          for(int i = 0; i < HOUSE_MAPREFER_FUNCTION_NUM; i++)
          {
            pHouseMap = g_pGs->GetGameMap(m_dwFuncMapId[i]);
            if(pHouseMap)
            {
              pHouseMap->WarpAll(m_wBelongMapId);
            }
          }
        }
      }
      break;
    default:
      break;
    }
  }
}
void CHouse::DoPlayerFunctionAction(DWORD &dwNowTime)
{
#ifdef _DEBUG_HOUSE
#else
  if(m_iStatus == NOTAKEN) return;
#endif
  for(int i = 0; i < HOUSE_PLAYER_FUNCTION_NUM; i++)
  {
    if(i == 0) // +HPSPMP
    {
      if(dwNowTime > m_dwFuncCheckTime[i]+3000)
      {
        m_dwFuncCheckTime[i]=dwNowTime; 
        CGameMap* pHouseMap = g_pGs->GetGameMap(m_wHouseMapId);
        if(pHouseMap)
        {
          switch(m_iSize)
          {
          case S_SMALL:
              pHouseMap->AddAllPlayerHPSPMP(20,10,10);
            break;
          case S_MIDDLE:
              pHouseMap->AddAllPlayerHPSPMP(40,30,20);
            break;
          case S_HUGEONE:
              pHouseMap->AddAllPlayerHPSPMP(60,30,30);
            break;
          default:
            break;
          }
        }
      }
    }
    else if(i == 1) // Db Exp time
    {
      if(dwNowTime > m_dwFuncCheckTime[i]+600000)
      {
        m_dwFuncCheckTime[i]=dwNowTime;
        CGameMap* pHouseMap = g_pGs->GetGameMap(m_wHouseMapId);
        if(pHouseMap)
        {
          switch(m_iSize)
          {
          case S_SMALL:
#ifdef _DEBUG_HOUSE
            pHouseMap->AddAllPlayerSpecialTime(SPE_STATUS_EXP,600000+600000,33);
#else
            pHouseMap->AddAllPlayerSpecialTime(SPE_STATUS_EXP,600000+200000,33);
#endif
            break;
          case S_MIDDLE:
#ifdef _DEBUG_HOUSE
            pHouseMap->AddAllPlayerSpecialTime(SPE_STATUS_EXP,600000+600000,33);
#else
            pHouseMap->AddAllPlayerSpecialTime(SPE_STATUS_EXP,600000+300000,33);
#endif
            break;
          case S_HUGEONE:
#ifdef _DEBUG_HOUSE
            pHouseMap->AddAllPlayerSpecialTime(SPE_STATUS_EXP,600000+600000,33);
#else
            pHouseMap->AddAllPlayerSpecialTime(SPE_STATUS_EXP,600000+600000,33);
#endif
            break;
          default:
            break;
          }
        }
      }
    }
  }
}
void CHouse::DoHouseAction(DWORD &dwNowTime)
{
  if(m_iStatus == NOTAKEN) 
  {
/*
#ifdef _DEBUG
    char szTemp[100];
    FuncName("DoHouseAction()");
    _snprintf(szTemp,100,"%dNOTAKEN,SORETURN\n",m_wHouseMapId);
    szTemp[20] = '\0';
    Fprintf("2.txt","%s\n",szTemp);
    AddMemoMsg(szTemp);
#endif
*/    
    return;
  }
  DoPlayerFunctionAction(dwNowTime);
  CheckSatusAction(dwNowTime);
}
BOOL CHouse::RecvLoadHouse(SHouse *pSHouse)
{
  if(MAKELONG(GetHouseId(),GetBelongMapIdId()) != pSHouse->dwId) return FALSE;
  m_iStatus = pSHouse->wStatus;
  m_iBackMusic = pSHouse->wBakMusic;
  m_dwStatusCheckTime = pSHouse->dwCheckTime;
  m_iType = pSHouse->wType;
  switch(m_iType)
  {
  case T_GUILD:
    {
      m_pHouseOwner = (void*)g_pGuildMng->FindOneGuild(pSHouse->dwOwnerId);
    }
    break;
  case T_PERSONAL:
  case T_UNION:
    {
      m_pHouseOwner = 0;
    }
  default:
    break;
  }
  m_bLoadedFromMcc = TRUE;
  CGameMap * pHouseMap = g_pGs->GetGameMap(m_wHouseMapId);
  if(pHouseMap)
  {
    for(int i = 0; i < 5; i++)
    {
      if(pSHouse->wViewMon[i])
      {
        CSrvBaseMonster *pMonster = g_pBase->GetBaseMonster(pSHouse->wViewMon[i]);
        if(pMonster)
        {
          pHouseMap->AddPutMonster(pMonster,pSHouse->wX[i],pSHouse->wY[i],TRUE);
        }
      }
    }
  }
  CGameMap * pTheMap = g_pGs->GetGameMap(m_wBelongMapId);
  if(pTheMap)
  {
    CNpc * pNpc = pTheMap->GetNpc(m_dwHouseNpcCode);
    {
      if(pNpc)
      {
        pNpc->SetHouseStatus(m_iStatus);
        SMsgData *pNewMsg = g_pGs->NewMsgBuffer();
        if(pNewMsg)
        {
          pNewMsg->Init();
          pNewMsg->dwAID=A_HOUSENPC;
          pNewMsg->dwMsgLen = 2;
          pNewMsg->Msgs[0].Size = sizeof(DWORD);
          pNewMsg->Msgs[1].Size = sizeof(DWORD);
          *(DWORD*)pNewMsg->Msgs[0].Data = pNpc->GetSelfCode();
          *(DWORD*)pNewMsg->Msgs[1].Data = m_iStatus;
          pTheMap->SendTheMsgToAll(*pNewMsg);
          g_pGs->ReleaseMsg(pNewMsg);
        }
      }
    }
  }
  return TRUE;
}
//
void CHouse::SendLoadMessage()
{
  if(!m_bSendedMessage)
  {
    SMccMsgData* pMccMsg = g_pGs->NewMccMsgBuffer();
    if( pMccMsg )
    {
      pMccMsg->Init( NULL );
      pMccMsg->dwAID = AP_LOADHOUSEMES;
      pMccMsg->dwMsgID = 0;
      pMccMsg->dwMsgLen = 1;
      pMccMsg->Msgs[0].Size = sizeof( DWORD );
      *(DWORD*)(pMccMsg->Msgs[0].Data) = MAKELONG(this->GetHouseId(),this->GetBelongMapIdId());
      g_pMccDB->AddSendMsg( pMccMsg );
      m_bSendedMessage = TRUE;
    }
  }
}
//
void CHouse::RecvLoadMessage(SMsgData * pMccMsg)
{	
  if( pMccMsg->dwAID != AP_LOADHOUSEMES || (pMccMsg->Msgs[0].Size % sizeof(SHouseMes)) != 0)
  {
    if(pMccMsg->Msgs[0].Size == sizeof(DWORD))
    {
      m_bLoadMessage = TRUE;
    }
		return;
  }
	int iNum =  pMccMsg->Msgs[0].Size / sizeof(SHouseMes);

  SHouseMes* pMes = (SHouseMes*)pMccMsg->Msgs[0].Data;
  m_SMessage.iMessageNum = iNum;
  for(int i = 0; i < iNum; i ++, pMes++)
	{
    m_SMessage.sHouseMessage[i].bUse = TRUE;
    SafeStrcpy(m_SMessage.sHouseMessage[i].szPlayerName, pMes->szPlayerName, MAX_PLAYER_NAME_LEN);
    SafeStrcpy(m_SMessage.sHouseMessage[i].szMessage, pMes->szContent, MAX_HOUSE_MESSAGE_LEN);
    m_SMessage.sHouseMessage[i].dwLeaveTime = pMes->dwTime;
	}
  m_bLoadMessage = TRUE;
}
//
void CHouse::SendSaveMessage()
{
  if(m_SMessage.iMessageNum <= 0)
    return;
  SMccMsgData* pMccMsg = g_pGs->NewMccMsgBuffer();
  SHouseMes* pHouseMes = NULL;
  if( pMccMsg )
  {
    pMccMsg->Init( NULL );
    pMccMsg->dwAID = AP_SAVEHOUSEMES;
    pMccMsg->dwMsgLen = 1;
    pMccMsg->Msgs[0].Size = sizeof(SHouseMes) * m_SMessage.iMessageNum;
    pHouseMes = (SHouseMes*)pMccMsg->Msgs[0].Data;
    for(int i = 0; i < m_SMessage.iMessageNum; i++)
    {
      pHouseMes->dwMailId = MAKELONG(m_wHouseId, m_wBelongMapId);
      pHouseMes->dwTime = m_SMessage.sHouseMessage[i].dwLeaveTime;
      SafeStrcpy(pHouseMes->szContent, m_SMessage.sHouseMessage[i].szMessage, MAX_HOUSE_MESSAGE_LEN);
      SafeStrcpy(pHouseMes->szPlayerName, m_SMessage.sHouseMessage[i].szPlayerName, MAX_PLAYER_NAME_LEN);
      pHouseMes ++;
    }
    g_pMccDB->AddSendMsg( pMccMsg );
  }
}
//
void CHouse::SendLogHouseAction( WORD wAction, CPlayer* pPlayer )
{
  if(!m_pHouseOwner)
    return;
  SMccMsgData* pMccMsg = g_pGs->NewMccMsgBuffer();
  SHouseLog* pLog = NULL;
  if( pMccMsg )
  {
    pMccMsg->Init( NULL );
    pMccMsg->dwAID = AC_LOG_HOUSEACTION;
    pMccMsg->dwMsgLen = 1;
    pMccMsg->Msgs[0].Size = sizeof(SHouseLog);
    pLog = (SHouseLog*)pMccMsg->Msgs[0].Data;
    pLog->dwGuildId = ((CGuild*)m_pHouseOwner)->GetGuildId();
    SafeStrcpy(pLog->szGuildName,((CGuild*)m_pHouseOwner)->GetName(),MAX_GUILD_NAME_LEN);
    if( pPlayer ) 
      SafeStrcpy(pLog->szAccount, pPlayer->GetAccount(), MAX_ACCOUNT_LEN);
    else
      SafeStrcpy(pLog->szAccount, "XIAOWUDAOQI", MAX_ACCOUNT_LEN);
    ::CopyTime(&pLog->time, &g_SysTime);
    pLog->wAction = wAction;
    pLog->wHouseId = m_wHouseId;
    pLog->wHouseMapId = m_wHouseMapId;
    pLog->dwRemainTime = m_dwStatusCheckTime;
    g_pMccChat->AddSendMsg( pMccMsg );    
  }
}
//
BOOL CHouseMgr::LoadHouseData(char *szFile)
{
  char szFindFile[MAX_PATH];
  sprintf(szFindFile,"%s\\%s",g_pBase->GetMapFilePath(),szFile);
#ifdef _DEBUG_JAPAN_DECRYPT_
  CInStream infile(szFindFile);
  if (infile.fail() || infile.GetFileSize() == 0)
    return FALSE;
#else
  ifstream infile(szFindFile);
  if (infile.fail())
    return FALSE;
#endif//_DEBUG_JAPAN_DECRYPT_
  //
  int iMapCount = 0;
  WORD wBelongMapId = 0;
  WORD wHouseMapId = 0;
  int iHouseCount = 0;
  WORD wHouseId = 0;
  DWORD dwHouseId = 0;
  //
  int iSize;
  DWORD dwFuncMapId[HOUSE_MAPREFER_FUNCTION_NUM];
  //
  CHouse* pHouse = NULL;
  //
  infile>>iMapCount;
  for(int i = 0; i < iMapCount; i++)
  {
    infile>>wBelongMapId;
    infile>>iHouseCount;
    for(int j = 0; j < iHouseCount; j++)
    {
      infile>>wHouseId>>wHouseMapId>>iSize;
      for(int k = 0; k < HOUSE_MAPREFER_FUNCTION_NUM; k++)
      {
        infile>>dwFuncMapId[k];
      }
      if(g_pGs->GetGameMap(wBelongMapId)!=NULL)
      {
        pHouse = new CHouse(wHouseId,wBelongMapId,wHouseMapId,iSize,dwFuncMapId);
        if(pHouse)
          AddHouse(pHouse);
      }
    }
  }
  return TRUE;
}
void CHouseMgr::DoAllHouseAction(DWORD dwNowTime)
{
  m_dwNowTime = dwNowTime;
  static DWORD dwSavedTime = dwNowTime+1800000;
  for(m_iter = m_mapHouse.begin();m_iter != m_mapHouse.end(); m_iter++)
  {
#ifdef _DEBUG
    CHouse* pHouse = m_iter->second;
#endif
    if(m_iter->second->m_bLoadedFromMcc&&
       m_iter->second->m_bLoadMessage)
    {
      m_iter->second->DoHouseAction(m_dwNowTime);
    }
    else
    {
      if(!m_iter->second->m_bSendedMcc)
      {
        SendLoadHouse(MAKELONG(m_iter->second->GetHouseId(),
          m_iter->second->GetBelongMapIdId()));
        m_iter->second->m_bSendedMcc = TRUE;
      }
      m_iter->second->SendLoadMessage();
    }
    if( dwNowTime > dwSavedTime &&
      m_iter->second->m_bLoadedFromMcc &&
      m_iter->second->m_bLoadMessage )
    {
      if( TRUE == SendSaveHouse() )
        dwSavedTime = dwNowTime + 1800000; //30 * 60 * 1000
    }
  }
}
//
void CHouseMgr::DoExitAction()
{
  static bSend = FALSE;
  if(bSend == TRUE)
    return;
  CHouse* pHouse = NULL;
  SendSaveHouse();
  for( m_iter = m_mapHouse.begin(); m_iter != m_mapHouse.end(); m_iter++)
  {
    pHouse = m_iter->second;
    pHouse->SendSaveMessage();
  }
  bSend = TRUE;
}
//
BOOL CHouseMgr::SendLoadHouse(DWORD dwHouseId)
{
  SMccMsgData  *pNewMccMsg = g_pGs->NewMccMsgBuffer();
  if( pNewMccMsg )
  {
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID        = AP_LOADHOUSE;
    pNewMccMsg->dwMsgLen     = 1;
    pNewMccMsg->Msgs[0].Size = sizeof(DWORD);
    //
    *(DWORD*)(pNewMccMsg->Msgs[0].Data) = dwHouseId;
    //
    g_pMccDB->AddSendMsg( pNewMccMsg );
    return TRUE;
  }
  return FALSE;
}
//
BOOL CHouseMgr::SendSaveHouse( )
{
  SMccMsgData *pNewMccMsg = g_pGs->NewMccMsgBuffer();
  int iNum = m_mapHouse.size();
  if( iNum <= 0 ) return FALSE;
  SHouse* pHouse = NULL;
  CHouse* pChouse = NULL;
  map<DWORD,CHouse*>::iterator iter_House = m_mapHouse.begin(); 
  if( pNewMccMsg )
  {
    pNewMccMsg->Init( NULL );
    pNewMccMsg->dwAID = AP_SAVEHOUSE;
    pNewMccMsg->dwMsgLen = 1;
    pNewMccMsg->Msgs[0].Size = iNum * sizeof(SHouse);
    pHouse = (SHouse*)pNewMccMsg->Msgs[0].Data;
    for( ; iter_House != m_mapHouse.end(); iter_House++ )
    {
      pChouse = iter_House->second;
      if(!pChouse) continue;
      pHouse->dwCheckTime = pChouse->GetCheckTime();
      pHouse->dwId = MAKELONG(pChouse->GetHouseId(),pChouse->GetBelongMapIdId());
      if(pChouse->GetHouseOwner())
      {
        switch(pChouse->GetHouseType())
        {
        case T_GUILD:
          {
            pHouse->dwOwnerId = ((CGuild*)(pChouse->GetHouseOwner()))->GetGuildId();
          }
          break;
        case T_PERSONAL:
        case T_UNION:
        default:
          {
            pHouse->dwOwnerId = 0;
          }
          break;
        }
      }
      else
        pHouse->dwOwnerId = 0;
			CGameMap* pHouseMap = g_pGs->GetGameMap(pChouse->GetHouseMapId());
			if(pHouseMap)
			{
        pHouseMap->GetViewMonsters(pHouse->wViewMon,pHouse->wX,pHouse->wY);
			}
      else
      {
        for(int i = 0; i < 5; i++)
        {
          pHouse->wViewMon[i] = 0;
          pHouse->wX[i] = 0;
          pHouse->wY[i] = 0;
        }
      }
      pHouse->wBakMusic = pChouse->GetHouseMusicId();
      pHouse->wStatus = pChouse->GetHouseSatus();
      pHouse->wType = pChouse->GetHouseType();
      pHouse ++;
    }
    g_pMccDB->AddSendMsg( pNewMccMsg );
    return TRUE;
  }
  return FALSE;
}
//
BOOL CHouseMgr::RecvLoadHouse(SMsgData * pMccMsg)
{
  if(pMccMsg->dwAID != AP_LOADHOUSE || (pMccMsg->Msgs[0].Size % sizeof(SHouse)) != 0)
  {
    return FALSE;
  }
  for(int i = 0; i < pMccMsg->Msgs[0].Size/sizeof(SHouse); i++)
  {
    SHouse *pSHouse = &((SHouse*)pMccMsg->Msgs[0].Data)[i];
    for(m_iter = m_mapHouse.begin();m_iter != m_mapHouse.end(); m_iter++)
    {
      if(!m_iter->second->m_bLoadedFromMcc)
      {
        if(pSHouse->dwId==MAKELONG(m_iter->second->GetHouseId(),
          m_iter->second->GetBelongMapIdId()))
        {
          if(!m_iter->second->RecvLoadHouse(pSHouse))
            return FALSE;
        }
      }
    }
  }
  return TRUE;
}
int CHouseMgr::GetCount()
{
  return m_mapHouse.size();
}
CHouse* CHouseMgr::GetHouseById(DWORD dwId)
{
  if(m_mapHouse.find(dwId)!=m_mapHouse.end())
    return m_mapHouse[dwId];
  return NULL;
}
CHouse* CHouseMgr::GetHouseByNpc(DWORD dwHouseBelongMapId,DWORD dwCode)
{
  for(m_iter = m_mapHouse.begin(); m_iter!=m_mapHouse.end(); m_iter++)
  {
    if(m_iter->second->m_wBelongMapId == dwHouseBelongMapId&&m_iter->second->m_dwHouseNpcCode == dwCode)
      return m_iter->second;
  }
  return NULL;
}
CHouse* CHouseMgr::GetHouseByHouseMapId(DWORD dwHouseMapId)
{
  for(m_iter = m_mapHouse.begin(); m_iter!=m_mapHouse.end(); m_iter++)
  {
    if(m_iter->second->m_wHouseMapId == dwHouseMapId)
      return m_iter->second;
  }
  return NULL;
}
BOOL CHouseMgr::AddHouse(CHouse* pHouse)
{
  DWORD dwTemp = MAKELONG(pHouse->GetHouseId(),pHouse->GetBelongMapIdId());
  m_iter = m_mapHouse.find(dwTemp);
  if(m_iter == m_mapHouse.end())
  {
    m_mapHouse[dwTemp]=pHouse;
    return TRUE;
  }
  return FALSE;
}
CHouseMgr::CHouseMgr()
{
#ifdef _MODIFY_GETHOUSE_MODE_
  InitCityToHouseMapping();
#endif
  m_mapHouse.clear();
}
//---------------------------------------------------------------
#ifdef _MODIFY_GETHOUSE_MODE_
void CHouseMgr::InitCityToHouseMapping()
{
  //1100 帝释天(中型小屋)：3005－4018
  m_i64CityToHouseMapping[0] = 1124;//1100
  m_i64CityToHouseMapping[0] = m_i64CityToHouseMapping[0]<<32 | MAKELONG(3,3005);
  //2100 多闻天(小型小屋)：3005－4001
  m_i64CityToHouseMapping[1] = 2107;//2100
  m_i64CityToHouseMapping[1] = m_i64CityToHouseMapping[1]<<32 | MAKELONG(2,3005);
  //3100 伊舍那天(小型小屋)：3005－4012
  m_i64CityToHouseMapping[2] = 3107;//3100
  m_i64CityToHouseMapping[2] = m_i64CityToHouseMapping[2]<<32 | MAKELONG(1,3005);
}
//---------------------------------------------------------------
DWORD CHouseMgr::GetHouseIdFromCityMapId(const DWORD dwCityMapId) const
{
  DWORD dwReValue(0);
  for(int i = 0; i < MAX_CITY_WAR_HAPPEN; ++i)
  {
    if( dwCityMapId == HIDWORD(m_i64CityToHouseMapping[i]) )
    {
      dwReValue = LODWORD(m_i64CityToHouseMapping[i]);
      break;
    }
  }
  return dwReValue;
}
#endif
//---------------------------------------------------------------
BOOL CHouseMgr::bHaveHouse(DWORD dwId,int iType)
{
  CHouse* pHouse;
  for(m_iter = m_mapHouse.begin(); m_iter!=m_mapHouse.end(); m_iter++)
  {
    pHouse = m_iter->second;
    if(pHouse->m_iStatus!=NOTAKEN)
    {
      if(pHouse->m_iType==iType)
      {
        switch(iType)
        {
        case T_GUILD:
          {
            if(pHouse->m_pHouseOwner)
            {
              if(((CGuild*)pHouse->m_pHouseOwner)->GetGuildId()==dwId)
              {
                return TRUE;
              }
            }
          }
          break;
        case T_PERSONAL:
        case T_UNION:
        default:
          {
          }
          break;
        }
      }
    }
  }
  return FALSE;
}
BOOL CHouseMgr::bIsInHouseMapAndIsOwner(DWORD dwMId,DWORD dwGId,BOOL & bFindMap,CHouse* &pHouseR)
{
  if(m_mapHouse.empty())
  {
    bFindMap = FALSE;
    pHouseR = NULL;
    return FALSE;
  }
  pHouseR = NULL;
  CHouse* pHouse;
  for(m_iter = m_mapHouse.begin(); m_iter!=m_mapHouse.end(); m_iter++)
  {
    bFindMap = FALSE;
    pHouse = m_iter->second;
    if(dwMId==pHouse->m_wHouseMapId)
    {
      bFindMap = TRUE;
      pHouseR = pHouse;
    }
    else
    {
      for(int i = 0; i < HOUSE_MAPREFER_FUNCTION_NUM; i++)
      {
        if(dwMId==pHouse->m_dwFuncMapId[i])
        { 
          bFindMap = TRUE;
          pHouseR = pHouse;
          break;
        }
      }
    }
    if(bFindMap&&dwGId)
    {
      if(pHouse->m_iStatus!=NOTAKEN)
      {
        if(pHouse->m_iType==T_GUILD&&pHouse->m_pHouseOwner)
        {
          if(dwGId==((CGuild*)(pHouse->m_pHouseOwner))->GetGuildId())
          {
            return TRUE;
          }
        }
      }
      return FALSE;
    }
    else if(bFindMap)
    {
      return FALSE;
    }
  }
  bFindMap = FALSE;
  pHouseR = NULL;
  return FALSE;
}
#endif


#ifdef VERSION40_FIELD
BOOL CField::Load()
{
  string strFilePath = g_pBase->GetObjectFilePath();
  string strPath;
  char szPath[23];
  int iNum = 0;
  int iTotal = 0;
  SMonsterId* pId = NULL; 
  CSrvBaseMonster* pBaseMonster = NULL;
  for(int iLoop=0; iLoop<5; iLoop++)
  {
    ifstream in;
    sprintf(szPath,"Monster/FieldList%d.txt",iLoop+1);
    strPath = strFilePath + szPath;  
    in.open(strPath.c_str(),ios::in);
    if(in.fail()) return FALSE;
    in >> m_wMapId; //地图ID
    in >> iNum;     //多少种怪物
    in >> iTotal;   //怪物总数
    m_IdList[iLoop].iTotal = iTotal;
    for(int i = 0; i < iNum; i++)
    {
      pId = new SMonsterId;
      in >> pId->wMonsterId;
      if(NULL == (pBaseMonster = g_pBase->GetBaseMonster(pId->wMonsterId)))
      {
        in.close();
        return FALSE;
      }
      in >> pId->wX;
      in >> pId->wY;
      in >> pId->wCount;
      m_IdList[iLoop].listId.push_back(pId);
    }
    m_IdList[iLoop].MonsterList = new LPCMonster[iTotal];
    for(i = 0; i < iTotal; i++)
      m_IdList[iLoop].MonsterList[i] =  NULL;
    in.close();
  }
  m_pMap = g_pGs->GetGameMap(m_wMapId);
  return TRUE;
}
//参数i表示第几波怪物
BOOL CField::PutMonster(int iSeq)
{
  list<SMonsterId*>::iterator iter_listId;
  SMonsterId* pId = NULL;
  CMonster * pNewMonster = NULL;
  int j = 0;
  SMsgData* pTheMsg = NULL;
  pTheMsg = g_pGs->NewMsgBuffer();
  if(!pTheMsg) return FALSE;
  int iCreateCount = 0;
  iter_listId = m_IdList[iSeq].listId.begin();
  for(; iter_listId != m_IdList[iSeq].listId.end(); iter_listId++)
  {
    pId = *iter_listId;
    if(pId == NULL) return FALSE;
    for(int i=0; i < pId->wCount; i++,j++)
    {
      pNewMonster = m_pMap->CreateMonster(pId->wMonsterId,pId->wX,pId->wY,0,0);
      if(pNewMonster)
      {
        m_IdList[iSeq].MonsterList[j] = pNewMonster;
        SNMNpcInfo			*pTheNpcInfo = (SNMNpcInfo*)(pTheMsg->Msgs[0].Data);
        pNewMonster->Get_SNMNpcInfo( pTheNpcInfo );
        pTheNpcInfo++;
        iCreateCount++;
      }   
    }
  }
  if( iCreateCount )
  {
    pTheMsg->dwAID        = A_PLAYERINFO;
    pTheMsg->dwMsgLen     = 1;
    pTheMsg->Msgs[0].Size = sizeof( SNMNpcInfo ) * iCreateCount;
    m_pMap->SendTheMsgToAll(*pTheMsg);
  }
  g_pGs->ReleaseMsg(pTheMsg);
  return TRUE;
}
//参数i表示第几波怪物
void CField::RemoveMonster(int iSeq)
{
/*
int iTotal = m_IdList[iSeq].iTotal;
for(int i=0; i<iTotal; i++)
{
delete m_IdList[iSeq].MonsterList[i];
}
*/  
  m_pMap->ClearMonster();
}
void CField::ResetMap()
{
  m_pMap->SetMapCanPK();
  m_pMap->SetMapIsAllDay();
}
void CField::DoAction(DWORD dwTime)
{
  if(!m_pMap) return;
  if( g_pStone )
  {
    g_pStone->SendMsg(dwTime);
    if(dwTime > g_pStone->m_dwEndTime)
    {
      *(DWORD*)MsgClearCode.Msgs[0].Data = g_pStone->m_wCode;
      m_pMap->SendMsgNearPosition_Far(MsgClearCode,g_pStone->m_wX,g_pStone->m_wY);
      delete g_pStone;
      g_pStone = NULL;
      SMccMsgData * pMccMsg = g_pGs->NewMccMsgBuffer();
      if(pMccMsg)
      {
        pMccMsg->Init(NULL);
        pMccMsg->dwMsgLen = 0;
        pMccMsg->dwAID = AP_STONEOFF;
        g_pMccDB->AddSendMsg(pMccMsg);
      }
    } 
  }
  switch(m_wStatus)
  {
  case FS_CLOSE:
    return;
  case FS_ON_TEN:
    {
      if(dwTime >= m_dwMonsterStartTime)
        m_wStatus = FS_ON_PUTMONSTER;
    }
    break;
  case FS_ON_PUTMONSTER:
    {
      if( m_wGroup == 5 && m_pMap->GetViveMonster() == 0 )
      {
        if(!m_bSetEndTime)
        {
          m_dwEndTime = dwTime + 60000; //一分钟后结束
          m_bSetEndTime = TRUE;
          ResetMap();
          SMsgData *pTheMapMsg = g_pGs->NewMsgBuffer();
          if(pTheMapMsg)
          {
            SNMMapAddInfo *pAddInfo = (SNMMapAddInfo*)pTheMapMsg->Msgs[0].Data;
            pTheMapMsg->Init();
            pTheMapMsg->dwAID = A_MAPADDINFO;
            pTheMapMsg->dwMsgLen = 1;
            pTheMapMsg->Msgs[0].Size = sizeof(SNMMapAddInfo);
            m_pMap->GetMapAddInfo(*pAddInfo);
            m_pMap->SendTheMsgToAll(*pTheMapMsg);
            g_pGs->ReleaseMsg(pTheMapMsg);
          }
        }
      }
      if((dwTime > m_dwEndTime) || m_pMap->GetPlayerCount() == 0 )
      {
        SetOff();
        m_iSendTime = 0;
        SendMccWhenVanish();
        return;
      }
      if(m_bFirst)
      {
        SetNoPK();
#ifndef _DEBUG
        SetNight();
#endif
        SMsgData *pTheMapMsg = g_pGs->NewMsgBuffer();
        if(pTheMapMsg)
        {
          SNMMapAddInfo *pAddInfo = (SNMMapAddInfo*)pTheMapMsg->Msgs[0].Data;
          pTheMapMsg->Init();
          pTheMapMsg->dwAID = A_MAPADDINFO;
          pTheMapMsg->dwMsgLen = 1;
          pTheMapMsg->Msgs[0].Size = sizeof(SNMMapAddInfo);
          m_pMap->GetMapAddInfo(*pAddInfo);
          m_pMap->SendTheMsgToAll(*pTheMapMsg);
          g_pGs->ReleaseMsg(pTheMapMsg);
        }
        m_bFirst = FALSE;
        PutMonster(m_wGroup);
        m_dwMonsterStartTime += 7200000;
        m_wGroup++;
        SMccMsgData * pTheMsg = g_pGs->NewMccMsgBuffer();
        if( pTheMsg )
        {
          pTheMsg->Init( NULL );
          pTheMsg->dwMsgLen = 1;
          pTheMsg->dwAID = AP_NEWS;
          pTheMsg->Msgs[0].Size = sizeof(SFieldNews);
          SFieldNews* pNews = (SFieldNews*)pTheMsg->Msgs[0].Data;
          pNews->wWave = m_wGroup;
          pNews->wHeroNum = m_pMap->GetPlayerCount();
          pNews->wMonsterNum = m_pMap->GetViveMonster();
          g_pMccDB->AddSendMsg( pTheMsg );
        }
      }
      else
      {
        if(0 == m_pMap->GetViveMonster())
        {
          if( !m_bSetTime )
          {
            m_dwMonsterStartTime = dwTime + 120000; //2*60*1000
            m_bSetTime = TRUE;
            if(m_iSendTime < 4) // for a bug
            {
              SMsgData * pTheMsg = g_pGs->NewMsgBuffer();
              if( pTheMsg )
              {
                pTheMsg->Init();
                pTheMsg->dwMsgLen = 0;
                pTheMsg->dwAID = A_COUNTDOWN;
                m_pMap->SendTheMsgToAll(*pTheMsg);
                g_pGs->ReleaseMsg(pTheMsg);
                m_iSendTime ++;
              }
            }
          }
        }
        if(dwTime >= m_dwMonsterStartTime)
        {
          RemoveMonster(m_wGroup-1);
          PutMonster(m_wGroup);
          m_dwMonsterStartTime += 7200000;
          m_wGroup++;
          SMccMsgData * pTheMsg = g_pGs->NewMccMsgBuffer();
          if( pTheMsg )
          {
            pTheMsg->Init( NULL );
            pTheMsg->dwMsgLen = 1;
            pTheMsg->dwAID = AP_NEWS;
            pTheMsg->Msgs[0].Size = sizeof(SFieldNews);
            SFieldNews* pNews = (SFieldNews*)pTheMsg->Msgs[0].Data;
            pNews->wWave = m_wGroup;
            pNews->wHeroNum = m_pMap->GetPlayerCount();
            pNews->wMonsterNum = m_pMap->GetViveMonster();
            g_pMccDB->AddSendMsg( pTheMsg );
          }
          m_bSetTime = FALSE;
        }
      }
    }  
    break;
  default:
    break;
  }
}
void CField::SendMccWhenVanish()
{
  SMccMsgData * pTheMsg = g_pGs->NewMccMsgBuffer();
  if( pTheMsg )
  {
    pTheMsg->Init(NULL);
    pTheMsg->dwAID = AP_FIELDEND;
    pTheMsg->dwMsgLen = 0;
    g_pMccDB->AddSendMsg(pTheMsg);
  }
}
CRecurStone::CRecurStone(DWORD dwTime,DWORD dwMapId,WORD wX,WORD wY, DWORD dwCode)
{
  m_pMap = g_pGs->GetGameMap(dwMapId);
  m_dwMapId = dwMapId;
  m_dwEndTime = dwTime + 600000;
  m_dwSendTime = dwTime;
  m_wCode = dwCode;
  m_wX = wX;
  m_wY = wY;
}
#endif

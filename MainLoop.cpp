#include "stdafx.h"
#include "MainLoop.h"

#include "../resource.h"
#include "SrvGsData.h"
#include "../Network/NetworkThread.h"
#include "../Network/SrvClientData.h"
#include "../Mcc/MccNetThread.h"
#include "../Mcc/SrvMccInfo.h"
#include "../SrvClass/Talk/SrvTalk.h"
#include "../SrvClass/Life/SrvPlayer.h"
#include "../DB Thread/DBThread.h"
#include "BfInit.h"
#include "..\DB Thread\DBSqlFunc.h"
#include "..\SrvClass\Guild\SrvGuild.h"
//===============================================================================================
//
// global function in BFInit.cpp
bool IntiAllData();
bool ReleaseAllData();
//===============================================================================================
//
// global variables
#define           ID_TOOLS          56791
DWORD				g_dwGsAutoRunTime = TimeGetTime() + 15000;
//
#ifdef _DEBUG_USE_SPECIAL_THREAD_MAINLOOP_
extern MainLoop_Thread_Handle    *g_pMainLoopThread;
#endif
extern DWORD      g_dwRefreshServerTick;

#ifdef _MONSTER_ATTACK_CITY_
CMonsterAttackCity     g_AttackCity;
#endif
//===============================================================================================
//
// global variables
extern LRESULT CALLBACK CheckData(HWND, UINT, WPARAM, LPARAM);
extern HINSTANCE                        g_hins;
extern HWND			                        g_hwnd;
extern CMain_network_thread             *g_pAcceptThread;
extern CMain_Client_ReadWrite_Thread		*g_pNetThread;
extern CMcc_Network_Thread	            *g_pMccThread;
extern Query                            *g_pQuery;

extern CMccInfo							*g_pMccDB;		// 负责存取玩家数据 -- DB
extern CMccInfo							*g_pMccChat;	// 负责聊天，组队，好友，Relogin
extern CChatroomList				*g_pChatroomList;
extern CReloginServer				*g_pReloginServer;
extern CDB_Handle_Thread    *g_pSqlDB;
extern CGuildManager        *g_pGuildMng;
extern list<CMonster*>      g_ListUnreviveMonster;
extern void DeleteAllUnreviveMonster();

static char szMainLoopLog[MAX_MEMO_MSG_LEN];

extern HWND			g_hwnd;
#ifdef _DEBUG_WILDCAT_
extern DWORD			g_dwNetFunc[5], g_dwMonster[MAX_MAP_NUM][2], g_dwNpc[MAX_MAP_NUM][2], g_dwPlayer[MAX_MAP_NUM][2], g_dwClientNet, g_dwMccNet, g_dwMap, g_dwLoop, g_dwLoopStart, g_dwNetLoop, g_dwAcceptLoop;
#endif
extern WORD							g_wCounterTotal;
BOOL g_bConvert = FALSE;
//============================================================================================================
//
// Mesage handler for about box.
LRESULT CALLBACK LoginDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  FuncName("LoginDlg");
  char      szAccount[128];
  char      szPassword[128];

	switch( message )
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if( LOWORD( wParam ) == ID_DB_LOGIN_OK )
		{
      GetDlgItemText( hDlg, IDC_EDIT_DB_ACCOUNT, szAccount, 127 );
      GetDlgItemText( hDlg, IDC_EDIT_DB_PASSWORD, szPassword, 127 );
      //
      g_pGs->SetDBAccount( szAccount );
      g_pGs->SetDBPassword( szPassword );
      //
      AddMemoMsg( "Accept DB Account And Password..." );
			EndDialog( hDlg, LOWORD( wParam ) );
      g_pGs->SetState( GSSTATE_INIT_BASEDATA );
			return TRUE;
		}
    else if( LOWORD( wParam ) == ID_DB_LOGIN_CANCEL )
    {
      AddMemoMsg( "Cancel Login DB, So Exit Game Server..." );
      EndDialog( hDlg, LOWORD( wParam ) );
      g_pGs->SetState( GSSTATE_INIT_FAIL );
      return true;
    }
		break;
	}
  return FALSE;
}
//===============================================================================================
//
//
//GS的状态切换 GSTATE_*
// -----------------------------------
// |GSSTATE_INIT_BASEDATA            |  初始化整个系统
// -----------------------------------
// |GSSTATE_INIT_MCC_THREAD          |  启动MCC的处理线程,只处理与网路相关的数据发送接收(MCC和ReloginServer)
// -----------------------------------
// |GSSTATE_AP_MAPINIT               |  等待MCC送回地图初始化信息
// -----------------------------------
// |GSSTATE_INIT_TAXMAP_LIST         |  初始化收税地图信息
// -----------------------------------
// |GSSTATE_AP_GETTIME               |  获得游戏世界里的时间
// -----------------------------------
// |GSSTATE_AP_CLEARLOCK             |  清除MCC上的玩家Onlien信息
// -----------------------------------
// |GSSTATE_INIT_MAP_OBJ             |  初始化Server上的地图相关数据
// -----------------------------------
// |GSSTATE_LOAD_GUILD_INFO_FROM_MCC |  从MCC上读取帮会资料
// -----------------------------------
// |GSSTATE_LOAD_CITYWAR_INFO        |  获取帮会战信息
// -----------------------------------
// |GSSTATE_LOAD_CITYWAR_TIME        |  获取帮会战时间信息
// -----------------------------------
// |GSSTATE_QUERY_TOPTEN_*           |  获取排行榜信息
// -----------------------------------
// |GSSTATE_WAIT_CHATMCC_CONNECT     |  等待CHAT MCC连接
// -----------------------------------
// |GSSTATE_INIT_RELOGINSERVER       |  连接ReloginServer
// -----------------------------------
// |GSSTATE_INIT_NET_THREAD          |  初始化Client端的网络监听线程
// -----------------------------------
// |GSSTATE_INIT_COMPLETE            |  GameServer初始化结束,进入游戏状态
// -----------------------------------
// |GSSTATE_GAME_START               |  游戏运行中
// -----------------------------------
void GsMainLoop()
{
  FuncName("GsMainLoop");

	int			iSize = 0;
  
  static DWORD	          dwLastStartTime = TimeGetTime(), g_dwLoopLoc = 0;
	static DWORD	          g_dwWaitForAllSave = 0;
  static DWORD            g_dwReviveMonster = 0;

  SMsgData			*pTheMsg = NULL;
  
	ThisTickCount = TimeGetTime();
	g_dwLoop++;
  g_wCounterTotal++;
  //
  switch(g_pGs->GetState())
  {
    // Game Server start
    case GSSTATE_GAME_START:
      {
        dwLastStartTime = ThisTickCount;
        // Handle All Mcc Msg
#ifdef SRVD_NO_MCC_MSG_TIMEOUT
        // Do nothing...
#else

#ifdef _DEBUG_WILDCAT_
				ENTERTIME(1);
#endif
				g_pMccDB->ClearTimeoutMsg();
				g_pMccChat->ClearTimeoutMsg();
#endif
#ifdef _DEBUG_RECORD_EVERY_RECV_AID_
        g_AIDLog.Write( "Clear Timeout" );
#endif

				g_pMccDB->DispatchAll();
				g_pMccChat->DispatchAll();
        g_pReloginServer->DispatchAll();
#ifdef _DEBUG_RECORD_EVERY_RECV_AID_
        g_AIDLog.Write( "Do Mcc And Relogin Action" );
#endif
        
#ifdef _DEBUG_WILDCAT_
        LEAVETIME(1);
        g_dwMccNet = GETRECORD(1);
        ENTERTIME(2);
#endif
        
        // Handle All Client Msg and Action
        g_pClientList->DispatchAll();
#ifdef _DEBUG_WILDCAT_
        LEAVETIME(2);
        g_dwClientNet = GETRECORD(2);
        ENTERTIME(3);
#endif

        // Handle NPC, Monster, Magic, GroundItem Action
        g_pGs->DoMapAction();
        //
#ifdef _DEBUG_RECORD_EVERY_RECV_AID_
//        g_AIDLog.Write( "Do MapAction" );
#endif
        
#ifdef  ELYSIUM_3_7_VERSION
        g_FightFieldMgr.DoAction();
#endif
#ifdef _VERSION40_CARRYDOOR
        if( !g_bConvert )
        {
          g_pDoorManager->ConvertCli2Srv();
          g_bConvert = TRUE;
        }
        g_pDoorManager->TimeLoopSelf(dwLastStartTime);
        g_pDoorManager->TimeLoopMult(dwLastStartTime);
#endif
#ifdef VERSION_40_HOUSE_FUNCTION
        g_pHouseMgr->DoAllHouseAction(ThisTickCount);
#endif
#ifdef VERSION40_FIELD
        if(g_pField->IsOn())
          g_pField->DoAction(ThisTickCount);
#endif
        //
        // Handle Monster That Revive Type == REVIVE_TYPE_DELETE
        if( ThisTickCount > g_dwReviveMonster )
        {
          g_dwReviveMonster = ThisTickCount + 5000;
          DeleteAllUnreviveMonster();
        }

#ifdef _DEBUG_WILDCAT_
				LEAVETIME(3);
				g_dwMap = GETRECORD(3);
#endif
#ifdef _DEBUG_RELEASE_MESS_ASHCAN_
        g_pGs->ReleaseMsgAshcan();
#endif

#ifdef _DEBUG_MICHAEL_LOG_MIANlOOP
        extern DWORD                  g_dwMaxMainLoop;
        static DWORD iTemp = 0;
        if( ThisTickCount - iTemp > g_dwMaxMainLoop && 0 != iTemp )
        {
          g_dwMaxMainLoop = ThisTickCount - iTemp;
        }
        iTemp = ThisTickCount;
#endif
#ifdef _MONSTER_ATTACK_CITY_
        g_AttackCity.Loop();
#endif
      }
      break;
    case GSSTATE_RETRY_CONNECT_MCC:
      break;
    case GSSTATE_CREATE_DB_THREAD:
      {
        //if( g_pSqlDB = new CDB_Handle_Thread( false ) )
        //{
        //  g_pGs->SetState( GSSTATE_WAIT_SAVE_ALL_CLIENT );
        //}
        //else
        //{
        //  g_pGs->SetState( GSSTATE_WAIT_SAVE_ALL_CLIENT );
        //}
        //AddMemoMsg( "Create DB Thread OK, Ready Save All Client..." );
      }
      break;
		case GSSTATE_SAVE_ALL_CLIENT:
//			if((ThisTickCount - dwLastStartTime) > (MAIN_GAME_LOOP_MS))
      {
        dwLastStartTime = ThisTickCount;
// ENTERTIME(a)
// LEAVETIME(a)
// GETRECORD(a)
      // Handle All Mcc Msg
#ifdef SRVD_NO_MCC_MSG_TIMEOUT
        // Do nothing...
#else
#ifdef _DEBUG_WILDCAT_
				ENTERTIME(1);	
#endif

				g_pMccDB->ClearTimeoutMsg();
				g_pMccChat->ClearTimeoutMsg();
#endif

				g_pMccDB->DispatchAll();
				g_pMccChat->DispatchAll();
        g_pReloginServer->DispatchAll();

#ifdef _DEBUG_WILDCAT_
				LEAVETIME(1);
				g_dwMccNet = GETRECORD(1);
				ENTERTIME(2);
#endif

        // Handle All Client Msg and Action
        g_pClientList->DispatchAll();

#ifdef _DEBUG_WILDCAT_
				LEAVETIME(2);
				g_dwClientNet = GETRECORD(2);
				ENTERTIME(3);
#endif

        // Handle NPC, Monster, Magic, GroundItem Action
        g_pGs->DoMapAction();
#ifdef VERSION_40_HOUSE_FUNCTION
        g_pHouseMgr->DoExitAction();
#endif

        // Handle Monster That Revive Type == REVIVE_TYPE_DELETE
        if( ThisTickCount > g_dwReviveMonster )
        {
          g_dwReviveMonster = ThisTickCount + 5000;
          DeleteAllUnreviveMonster();
        }

#ifdef _DEBUG_WILDCAT_
				LEAVETIME(3);
				g_dwMap = GETRECORD(3);
#endif
#ifdef _DEBUG_RELEASE_MESS_ASHCAN_
        g_pGs->ReleaseMsgAshcan();
#endif
      }
      //
			if( g_pClientList->AllClientCountdown() )
			{
        if( g_dwRefreshServerTick == 0 )      g_dwRefreshServerTick = ClientTickCount + 2000;
				g_pGs->SetState( GSSTATE_KICK_ALL_SAVE_CLIENT );
        AddMemoMsg( "Count Down OK, Kick All Client And Save Data..." );
			}
      break;
		case GSSTATE_KICK_ALL_SAVE_CLIENT:
      g_pMccDB->ClearTimeoutMsg();
			g_pMccChat->ClearTimeoutMsg();
      //
			g_pMccDB->DispatchAll();
			g_pMccChat->DispatchAll();
      //g_pReloginServer->DispatchAll();
      //
      g_pClientList->DispatchAll();
      //
      g_pGs->DoMapAction();
      //
			if( g_pClientList->AllClientSaveData() )
			{
        //
        g_pGs->SendMcc_AC_LOG_MONSTERDROP();
        g_pGs->SendMcc_AC_LOG_ITEMMIX();
        g_pGs->SendMcc_AC_LOG_ITEMTESSERA();
        g_pGs->SendMcc_AC_LOG_ITEMBLESS();
        g_pGs->SendMcc_AC_LOG_PLAYERALTER();
				//////////////////////
				// Michael added Log
        g_pGs->SendMcc_AC_LOG_GUILDINFO();
				g_pGs->SendMcc_AC_LOG_PICKDROPMONEY();
				// Michael added end
				//////////////////////
        g_pGuildMng->UpdateGuildTaxIncome();
        //
				g_pGs->SetState( GSSTATE_WAIT_SAVE_ALL_CLIENT );
        AddMemoMsg( "Set All Client State OK, Wait Save All Client Data" );
			}
			break;
		case GSSTATE_WAIT_SAVE_ALL_CLIENT:
      g_pMccDB->ClearTimeoutMsg();
			g_pMccChat->ClearTimeoutMsg();
      //
			g_pMccDB->DispatchAll();
			g_pMccChat->DispatchAll();
      //g_pReloginServer->DispatchAll();
      //
      g_pClientList->DispatchAll();
      //
      g_pGs->DoMapAction();
      //
      if( g_pClientList->IsAllExit() )
			{
				g_pGs->SetState( GSSTTATE_SAVE_ALL_GUILD_TAX_INCOME );
        AddMemoMsg( " Save All Client Data OK, Save All Guild Tax Income" );
//        g_pGuildMng->SendMcc_AP_SAVE_GUILDTAXINCOME();
			}
			break;
    case GSSTTATE_SAVE_ALL_GUILD_TAX_INCOME:
      //
      if( IDOK == ( MessageBox( g_hwnd, " Save All Player Data OK, Server Will Close ?", "Warning", 1 ) ) )
      {
        g_pGs->SetState( GSSTATE_CLOSE );
      }
      break;
    case GSSTTATE_CLEAR_ALL_TEAM:
      g_pGs->DeleteAllTeam();
      // Resume The Client Network Accept
      //g_pAcceptThread->SetAcceptMode( true );
      // Send A Message To Mcc
      g_pMccDB->ClearSendMsg();
      g_pMccDB->Send_AP_MAPINIT();
      g_pGs->SetState( GSSTATE_GAME_START );
      break;
    case GSSTATE_RUN:                     // Start Game Server when Button "Run" is Clicked
      AddMemoMsg("--- Try to run Game Server ... ---");
      DialogBox( g_hins, (LPCTSTR)IDD_DIALOG_DB_LOGIN, g_hwnd, (DLGPROC)LoginDlg );
      break;
    // start to initialize Game Data
    case GSSTATE_INIT_BASEDATA:          // Load Base Data and initialize Client Data List
      AddMemoMsg("--- Start To Initialize ---");
      if( IntiAllData() )
      {
        if( g_pGuildMng->InitGuildManager() )
        {
#ifdef _CHECK_SERVER_DATA_FILE_VERSION_
          g_pGs->SetState(GSSTATE_CHECK_ALL_DATA_RELATION);
#else
///////////
#ifdef _DEBUG_GAMESERVER_USE_SQL_QUERY_
          g_pGs->SetState(GSSTATE_INIT_DB_QUERY);
#else
          g_pGs->SetState(GSSTATE_INIT_MCC_THREAD);
#endif
//////////
#endif
          break;
        }
      }
#ifdef _CHECK_SERVER_DATA_FILE_VERSION_
      g_pGs->SetState(GSSTATE_STOP);
      AddMemoMsg("***** Initialize Server Base Data Failed ! *****");
#else
      g_pGs->SetState(GSSTATE_INIT_FAIL);
#endif
      break;
#ifdef _CHECK_SERVER_DATA_FILE_VERSION_
    case GSSTATE_CHECK_ALL_DATA_RELATION:
      Sleep( 50 );
      break;
#endif
    // Init DB Query
    case GSSTATE_INIT_DB_QUERY:
      /*if( g_pQuery = new Query( g_pGs->GetDBAccount(), g_pGs->GetDBPassword(), g_pGs->GetDBIp() ) )
      {
        if( g_pQuery->InitQuery() && g_pQuery->ConnectDB() )
        {
          AddMemoMsg("***** Initialize And Connect Query Object OK ! *****");
          g_pGs->SetState(GSSTATE_INIT_MCC_THREAD);
          break;
        }
      }
      g_pGs->SetState(GSSTATE_INIT_FAIL);*/
      g_pGs->SetState(GSSTATE_INIT_MCC_THREAD);
      break;
    ////////////////////////////////////////////
    //启动MCC的处理线程,只处理与网路相关的数据发送接收(MCC和ReloginServer)
    case GSSTATE_INIT_MCC_THREAD:            // Init Mcc and Net Thread
      // Init Mcc Thread
      g_pMccThread = new CMcc_Network_Thread(false);
      if( !g_pMccThread )
      {
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
        AddMemoErrMsg("Init Mcc Thread Error #");
#endif
        g_pGs->SetState(GSSTATE_INIT_FAIL);
        return;
      }
      //g_pMccThread->Resume();
      _snprintf(szMainLoopLog, MAX_MEMO_MSG_LEN-1, "Mcc Thread ID = %08x", g_pMccThread->GetThreadId());
      szMainLoopLog[MAX_MEMO_MSG_LEN-1] = '\0';
      AddMemoMsg(szMainLoopLog);
      g_pGs->SetState(GSSTATE_WAIT_DBMCC_CONNECT);
      break;
    // Connect to Mcc Server and Query Game information for initialization
    case GSSTATE_WAIT_DBMCC_CONNECT:
      if(g_pGs->IsNoMccMode())
      {
        g_pGs->SetState( GSSTATE_AP_MAPINIT );
        dwLastStartTime = 0;
        break;
      }
      if( g_pMccDB->IsConnected() )
      {
        if( g_pGs->GetMccMode() == 2 && g_pMccChat->IsConnected() )
        {
          g_pGs->SetState( GSSTATE_AP_MAPINIT );
          
        }
        else
        {
          g_pGs->SetState( GSSTATE_AP_MAPINIT );
        }
        dwLastStartTime = 0;
        break;
      }
      //AddMemoMsg("Wait for connecting to MCC");
      break;
    case GSSTATE_AP_MAPINIT:              // Query Map Information from Mcc
      pTheMsg = g_pMccDB->GetRecvMsg();
      if( pTheMsg )
      {
        if( AP_MAPINIT == pTheMsg->dwAID )
        {
          int       iRetMap = g_pGs->InitGameMap( pTheMsg );
          if( iRetMap > 0 )
          {
#ifdef _VERSION40_CARRYDOOR
            CCarryDoor * pCarryDoor = NULL;
            vector<CCarryDoor*>::iterator iter_CDoor = g_pDoorManager->m_vecCarryDoor.begin();
            for(; iter_CDoor != g_pDoorManager->m_vecCarryDoor.end(); iter_CDoor++)
            {
              pCarryDoor = *iter_CDoor;
              pCarryDoor->SetCode();
            }
#endif
#ifdef VERSION_40_HOUSE_FUNCTION
            g_pHouseMgr = new CHouseMgr;
            g_pHouseMgr->LoadHouseData("Housedata.txt");
#endif
#ifdef VERSION40_FIELD
            if(!g_pField->Load())
            {
              MessageBox(GetActiveWindow(),"Load Field Data Error","WARNING",MB_OK);
            }
#endif
            dwLastStartTime = 0;
            AddMemoMsg("AP_MAPINIT OK");
            // Release Unuse Map In This Server
            // ...
            
#ifndef _CHECK_SERVER_DATA_FILE_VERSION_
            //g_pGs->SetState( GSSTATE_AP_GMLIST );
            //g_pGs->SetState( GSSTATE_AP_GETTIME );
            g_pGs->SetState( GSSTATE_INIT_TAXMAP_LIST );
#else
            g_pGs->SetState( GSSTATE_AP_GETTIME );
#endif
            g_pMccChat->Send_AP_MAPINIT( TRUE );
            // If No Mcc, Set State To AP_GETTIME
            if( g_pGs->IsNoMccMode() )
            {
              //g_pGs->SetState( GSSTATE_AP_GETTIME );
              g_pGs->SetState( GSSTATE_INIT_TAXMAP_LIST );
            }
            break;
          }
          else if( iRetMap == -1 )
          {
            AddMemoMsg("AP_MAPINIT Continue");
            dwLastStartTime += 10000;
            break;
          }
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          AddMemoErrMsg("AP_MAPINIT Fail#");
#endif
          g_pGs->SetState(GSSTATE_INIT_FAIL);
        }
        else
        {
          g_pGs->ReleaseMsg( pTheMsg );
					pTheMsg = NULL;
        }
      }
      if( ( TimeGetTime() - dwLastStartTime ) > 10000 )
      {
        dwLastStartTime = TimeGetTime();
        g_pMccDB->Send_AP_MAPINIT();
      }
      break;
    case GSSTATE_INIT_TAXMAP_LIST:
      if( g_pGuildMng->ResetAllTaxMap() )
      {
        AddMemoMsg( "Reset All Tax Map OK..." );
        g_pGs->SetState( GSSTATE_AP_GETTIME );
        break;
      }
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
      AddMemoErrMsg( "Reset Tax Map List Fail" );
#endif
      g_pGs->SetState( GSSTATE_INIT_FAIL );
      break;
    case GSSTATE_AP_CHATROOMLIST:         // Query Chatroom Information from Mcc
      pTheMsg = g_pMccDB->GetRecvMsg();
      if(pTheMsg)
      {
        if(AC_CHATROOMCHECK == pTheMsg->dwAID)
        {
					// Check the Total of chatroom
          iSize = strlen(pTheMsg->Msgs[0].Data);
					dwLastStartTime = 0;
          if(g_pChatroomList->GetChatroomTotal() == *(int*)(pTheMsg->Msgs[0].Data) )
          { 
            AddMemoMsg("AC_CHATROOMCHECK OK");
            g_pGs->SetState(GSSTATE_AP_GMLIST);
            g_pGs->ReleaseMsg(pTheMsg);
            pTheMsg = NULL;
            break;
          }
#ifdef __DEBUG_DON_SHOW_ANY_RUNTIME_MSG_
#else
          AddMemoErrMsg("AC_CHATROOMCHECK Fail !, Reset AC_APPLYALLCHATROOM to Mcc");
#endif
          g_pGs->ReleaseMsg(pTheMsg);
					pTheMsg = NULL;
          //g_pChatroomList->ReleaseAllChatroom();
          g_pMccDB->Send_AC_APPLYALLCHATROOM();
        }
        else if( AC_APPLYALLCHATROOM == pTheMsg->dwAID )
        { 
					// Add new Chatroom
					g_pChatroomList->MccChatRoomDetailedInfo( pTheMsg );
          dwLastStartTime = TimeGetTime();
        }
        else
        {
          g_pGs->ReleaseMsg(pTheMsg);
          pTheMsg = NULL;
        }
      }
      if( TimeGetTime() - dwLastStartTime > 10000 )
      {
				// Time Out, Request The Chatroom List Again
        dwLastStartTime = TimeGetTime();
        g_pMccDB->Send_AC_APPLYALLCHATROOM();
      }
      break;
    case GSSTATE_AP_GMLIST:
			// Query GM Information from Mcc
      pTheMsg = g_pMccDB->GetRecvMsg();
      if(pTheMsg)
      {
        if(pTheMsg->dwAID == AP_GMLIST)
        {
          if(g_pGs->SetGmList(pTheMsg))   // including "delete pTheMsg;"
          {
            AddMemoMsg("AP_GMLIST OK");
            dwLastStartTime = 0;
            g_pGs->SetState(GSSTATE_AP_GETTIME);
            break;
          }
        }
        g_pGs->ReleaseMsg(pTheMsg);
        pTheMsg = NULL;
      }
      if( TimeGetTime() - dwLastStartTime > 10000 )
      {
				// Time Out, Request GM List Again
        dwLastStartTime = TimeGetTime();
        g_pMccDB->Send_AP_GMLIST();
      }
      break;
    case GSSTATE_AP_GETTIME:              
			// Query Time Information From Mcc
      pTheMsg = g_pMccDB->GetRecvMsg();
      if( pTheMsg )
      {
        if( pTheMsg->dwAID == AP_GETTIME )
        {
          g_pGs->SetGameTime( pTheMsg );
          AddMemoMsg( "AP_GETTIME  OK" );
          dwLastStartTime = 0;
          g_pGs->SetState( GSSTATE_AP_CLEARLOCK );
          break;
        }
        g_pGs->ReleaseMsg(pTheMsg);
        pTheMsg = NULL;
      }
      if( TimeGetTime()-dwLastStartTime > 10000 )
      {
				// Time Out, Query Time Info Again
        dwLastStartTime = TimeGetTime();
        g_pMccDB->Send_AP_GETTIME();
      }
      break;
    case GSSTATE_AP_CLEARLOCK:            // Send Clear Lock Msg to Mcc
      pTheMsg = g_pMccDB->GetRecvMsg();
      if( pTheMsg )
      {
        if( pTheMsg->dwAID == AP_CLEARLOCK )
        {
          AddMemoMsg( "AP_CLEARLOCK  OK" );
          dwLastStartTime = 0;
          g_pGs->SetState( GSSTATE_INIT_MAP_OBJ );
          //g_pGs->SetState( GSSTATE_AP_CHETERLIST );
          //g_pGs->SendMcc_AP_CHEATERLIST();
          dwLastStartTime = TimeGetTime() + 10000;
          //
          g_pGs->ReleaseMsg( pTheMsg );
          pTheMsg = NULL;
          break;
        }
        g_pGs->ReleaseMsg(pTheMsg);
        pTheMsg = NULL;
      }
      if( ( TimeGetTime() - dwLastStartTime ) > 10000 )
      {
        dwLastStartTime = TimeGetTime();
        g_pMccDB->Send_AP_CLEARLOCK();
      }
      break;
    case GSSTATE_AP_CHETERLIST:
      pTheMsg = g_pMccDB->GetRecvMsg();
      if( pTheMsg )
      {
        if( pTheMsg->dwAID == AP_CHEATERLIST )
        {
          if( g_pGs->RecvMcc_AP_CHEATERLIST( pTheMsg ) )
          {
            AddMemoMsg( "AP_CHEATERLIST  OK..." );
            g_pGs->SetState( GSSTATE_INIT_MAP_OBJ );
          }
        }
      }
      if( TimeGetTime() > dwLastStartTime )
      {
        dwLastStartTime = TimeGetTime() + 10000;
        AddMemoErrMsg( "Wati Mcc AP_CHEATERLIST Timeout One Time..." );
      }
      break;
    case GSSTATE_INIT_MAP_OBJ:              // Init All Game Map, NPC, and Monsters data
      if( !g_pGs->ResetMapObj() )
      {
        AddMemoErrMsg( "GSSTATE_INIT_MAP_OBJ Fialed..." );
				g_pGs->SetState(GSSTATE_INIT_FAIL);
        break;
      }
      AddMemoMsg( "Init Game Map, NPC, Monster Data OK..." );
///////////
#ifdef _DEBUG_GAMESERVER_USE_SQL_QUERY_
      g_pGs->SetState(GSSTATE_INIT_LOAD_GUILD);
      AddMemoMsg( "Ready Load Guild Info From DB..." );
      dwLastStartTime = 0;
#else
      g_pGs->SetState(GSSTATE_LOAD_GUILD_INFO_FROM_MCC);
      AddMemoMsg( "Ready Load Guild Info From Mcc..." );
      //
      g_pMccDB->SendMcc_AP_QUERYGUILD_Init( 0 );
      dwLastStartTime = TimeGetTime() + 10000;
#endif
//////////
      break;
    case GSSTATE_INIT_LOAD_GUILD:
      /*if( TimeGetTime() > dwLastStartTime )
      {
        if( g_pQuery->Sql_AP_QUERYGUILD() )
        {
          if( g_pGs->GetMccMode() < 2 )
          {
            g_pGs->SetState( GSSTATE_INIT_RELOGINSERVER );
            AddMemoMsg( "Ready Init Relogin Server..." );
          }
          else
          {
            g_pGs->SetState( GSSTATE_WAIT_CHATMCC_CONNECT );
            AddMemoMsg( "Ready Connect Chat Mcc Server..." );
          }
        }
        else
        {
          dwLastStartTime = TimeGetTime() + 1000;
          AddMemoMsg( "===>>> Load Guild Data From DB Failed One Time..." );
        }
      }*/
      if( g_pGs->GetMccMode() < 2 )
      {
        g_pGs->SetState( GSSTATE_INIT_RELOGINSERVER );
        AddMemoMsg( "Ready Init Relogin Server..." );
      }
      else
      {
        g_pGs->SetState( GSSTATE_WAIT_CHATMCC_CONNECT );
        AddMemoMsg( "Ready Connect Chat Mcc Server..." );
      }
      break;
    case GSSTATE_LOAD_GUILD_INFO_FROM_MCC:
      {
        pTheMsg = g_pMccDB->GetRecvMsg();
        if( pTheMsg )
        {
          if( pTheMsg->dwAID == AP_QUERYGUILD )
          {
            if( g_pMccDB->RecvMcc_AP_QUERYGUILD( pTheMsg ) )
            {
              _snprintf( szMainLoopLog, MAX_MEMO_MSG_LEN-1, "===>>> Load Guild Data From Mcc OK..., Guild Total=%d", g_pGuildMng->GetGuildTotal() );
              szMainLoopLog[MAX_MEMO_MSG_LEN-1] = '\0';
              AddMemoMsg( szMainLoopLog );
              //
              g_pGuildMng->AllGuildFind_League_Enemy();
              AddMemoMsg( "***** Init All Guild's League And Enemy Finish..." );
              //
              g_pGs->SetState( GSSTATE_LOAD_CITYWAR_INFO );
              g_pGuildMng->SendMcc_AC_CITYWARINFO();
              //
            }
            else
            {
              g_pMccDB->SendMcc_AP_QUERYGUILD_Init( g_pGuildMng->GetGuildTotal() );
              dwLastStartTime = TimeGetTime() + 10000;
            }
          }
          else
          {
            //sprintf( szMainLoopLog, "==>>> When Load Guild Info, Recv Invalid Mcc Msg AID=%d", pTheMsg->dwAID );
            //AddMemoErrMsg( szMainLoopLog );
            g_pGs->ReleaseMsg( pTheMsg );
						pTheMsg = NULL;
          }
        }
        else
        {
          if( dwLastStartTime < TimeGetTime() )
          {
            AddMemoMsg( "Wait Mcc Send Guilds Info Timeout One Time..." );
            dwLastStartTime = TimeGetTime() + 10000;
          }
        }
      }
      break;
    case GSSTATE_LOAD_CITYWAR_INFO:
      {
        pTheMsg = g_pMccDB->GetRecvMsg();
        if( pTheMsg )
        {
          if( pTheMsg->dwAID == AC_CITYWARINFO )
          {
            g_pGuildMng->RecvMcc_AC_CITYWARINFO( pTheMsg );
            g_pMccDB->SendMcc_AP_QUERYCITYWARTIME();
            //
            g_pGs->SetState( GSSTATE_LOAD_CITYWAR_TIME );
          }
          else
          {
            g_pGs->ReleaseMsg( pTheMsg );
						pTheMsg = NULL;
          }
        }
      }
      break;
    case GSSTATE_LOAD_CITYWAR_TIME:
      {
        pTheMsg = g_pMccDB->GetRecvMsg();
        if( pTheMsg )
        {
          if( pTheMsg->dwAID == AP_QUERYCITYWARTIME )
          {
            g_pMccDB->RecvMcc_AP_QUERYCITYWARTIME( pTheMsg );
#ifdef _DEBUG_CLOSE_TOPTEN_
            if( g_pGs->GetMccMode() < 2 )
            {
              g_pGs->SetState( GSSTATE_INIT_RELOGINSERVER );
              AddMemoMsg( "Ready Init Relogin Server..." );
            }
            else
            {
              g_pGs->SetState( GSSTATE_WAIT_CHATMCC_CONNECT );
              AddMemoMsg( "Ready Connect Chat Mcc Server..." );
            }
#else
            g_pGs->SetState( GSSTATE_QUERY_TOPTEN_1 );
            AddMemoMsg( "==>> Load City War Info From Mcc OK..." );
#endif
          }
          else
          {
            g_pGs->ReleaseMsg( pTheMsg );
						pTheMsg = NULL;
          }
        }
      }
      break;
    case GSSTATE_QUERY_TOPTEN_1:
      {
        if( g_pMccDB->IsConnected() )
        {
          if( g_pMccDB->SendMcc_AP_TOPTEN( 1 ) )
          {
            AddMemoMsg( "Ready Load Top Ten Info 1 From Mcc..." );
            g_pGs->SetState( GSSTATE_LOAD_TOPTEN_1 );
          }
        }
      }
      break;
    case GSSTATE_LOAD_TOPTEN_1:
      {
        pTheMsg = g_pMccDB->GetRecvMsg();
        if( pTheMsg )
        {
          if( pTheMsg->dwAID == AP_TOPTEN )
          {
            g_pMccDB->RecvMcc_AP_TOPTEN( pTheMsg );
            AddMemoMsg( "==>>> Load Top Ten Info 1 From Mcc OK..." );
            g_pGs->SetState( GSSTATE_QUERY_TOPTEN_2 );
          }
          else
					{
						g_pGs->ReleaseMsg( pTheMsg );
						pTheMsg = NULL;
					}
        }
      }
      break;
    case GSSTATE_QUERY_TOPTEN_2:
      {
        if( g_pMccDB->IsConnected() )
        {
          if( g_pMccDB->SendMcc_AP_TOPTEN( 2 ) )
          {
            AddMemoMsg( "Ready Load Top Ten Info 2 From Mcc..." );
            g_pGs->SetState( GSSTATE_LOAD_TOPTEN_2 );
          }
        }
      }
      break;
    case GSSTATE_LOAD_TOPTEN_2:
      {
        pTheMsg = g_pMccDB->GetRecvMsg();
        if( pTheMsg )
        {
          if( pTheMsg->dwAID == AP_TOPTEN )
          {
            g_pMccDB->RecvMcc_AP_TOPTEN( pTheMsg );
            AddMemoMsg( "==>>> Load Top Ten Info 2 From Mcc OK..." );
            if( g_pGs->GetMccMode() < 2 )
            {
              g_pGs->SetState( GSSTATE_INIT_RELOGINSERVER );
              AddMemoMsg( "Ready Init Relogin Server..." );
            }
            else
            {
              g_pGs->SetState( GSSTATE_WAIT_CHATMCC_CONNECT );
              AddMemoMsg( "Ready Connect Chat Mcc Server..." );
            }
          }
          else
					{
						g_pGs->ReleaseMsg( pTheMsg );
						pTheMsg = NULL;
					}
        }
      }
      break;
    case GSSTATE_WAIT_CHATMCC_CONNECT:
			if( g_pGs->GetMccMode() < 2 )
      {
        g_pGs->SetState( GSSTATE_INIT_RELOGINSERVER );
        dwLastStartTime = 0;
        break;
      }
      if( g_pMccChat->IsConnected() )
      {
        g_pGs->SetState( GSSTATE_INIT_RELOGINSERVER );
        dwLastStartTime = 0;
        AddMemoMsg( "Connect Chat Mcc Server OK" );
        break;
      }
			break;
		case GSSTATE_INIT_RELOGINSERVER:
			if( !g_pReloginServer->Init() )
			{
				g_pGs->SetState( GSSTATE_WAIT_RELOGINSERVER_CONNECT );
				AddMemoMsg( "Relogin Server Init Failed #" );
				g_pGs->SetState( GSSTATE_CLOSE );
				break;
			}
			if( g_pReloginServer->GetMode() == 0 )
			{
				g_pGs->SetState( GSSTATE_INIT_NET_THREAD );
				break;
			}
      g_pReloginServer->SetRSState( RSERVER_STATE_TRY_CONNECT );
			g_pReloginServer->SetRSState( RSERVER_STATE_TRY_CONNECT );
			AddMemoMsg( "Relogin Server Init OK #" );
			g_pGs->SetState( GSSTATE_WAIT_RELOGINSERVER_CONNECT );
			break;
		case GSSTATE_WAIT_RELOGINSERVER_CONNECT:
			if( g_pReloginServer->IsConnected() )
			{
				g_pGs->SetState( GSSTATE_INIT_NET_THREAD );
				break;
			}
			break;
    case GSSTATE_SEND_2_RSERVER_MYINFO:
      {
        if( g_pReloginServer->Send_AR_SERVERINFO() )
        {
          AddMemoMsg( "Send Server Info To Relogin Server OK #" );
          g_pGs->SetState( GSSTATE_INIT_NET_THREAD );
        }
        else
        {
          AddMemoErrMsg( "Send Server Info To Relogin Server Failed, Close Game Server #" );
          g_pGs->SetState( GSSTATE_CLOSE );
        }
      }
      break;
    case GSSTATE_INIT_NET_THREAD:
      {
        // Init Network Thread
        g_pAcceptThread = new CMain_network_thread( false, g_pGs->GetGsPort() );
        if( g_pAcceptThread->server_init_network() == false )
        {
          AddMemoErrMsg("Init Network Thread Error");
          g_pGs->SetState(GSSTATE_INIT_FAIL);
          return;
        }
        g_pAcceptThread->SetAcceptMode(true);
        _snprintf(szMainLoopLog, MAX_MEMO_MSG_LEN-1,"Accept Network Thread Id = %08x", g_pAcceptThread->GetThreadId());
        szMainLoopLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoMsg(szMainLoopLog);
#ifdef _USE_ACCEPT_CLIENT_THREAD_
        // Init Client Read And Write Thread
        g_pNetThread    = new CMain_Client_ReadWrite_Thread( false );
        _snprintf(szMainLoopLog, MAX_MEMO_MSG_LEN-1,"Client Read Write Network Thread Id = %08x", g_pNetThread->GetThreadId());
        szMainLoopLog[MAX_MEMO_MSG_LEN-1] = '\0';
        AddMemoMsg(szMainLoopLog);
#endif
        
        //---------------------------------------------------------------
        ///initialize monster attack city ini
#ifdef _MONSTER_ATTACK_CITY_
        char* szObjPath = g_pBase->GetObjectFilePath();
        char szPath[MAX_FILE_PATH];
        strcpy(szPath,szObjPath);
        strcat(szPath,"monsterattack.ini");
        if(!g_AttackCity.Init(szPath))
        {
#ifdef _DEBUG
          AddMemoErrMsg("this gameserver have no monster attack city");
#endif
        }
#endif // end _MONSTER_ATTACK_CITY_
        //
        g_pGs->SetState(GSSTATE_INIT_COMPLETE);
        break;
      }
    case GSSTATE_INIT_COMPLETE:           // Active all Game Data and then allow players connect to server
      AddMemoMsg("--- Initialization OK ---");
      AddMemoMsg("Resume Network Thread, Wait Connect ...");
      dwLastStartTime = 0;
      // Accept the connection from filters
      g_pGs->SetState(GSSTATE_GAME_START);
      break;
    case GSSTATE_INIT_FAIL:
      AddMemoErrMsg("Init Fail #");
      g_pGs->SetState(GSSTATE_ERROR_CLOSE);
      break;
    case GSSTATE_CLOSE:
      DestroyWindow( g_hwnd );
      break;
    case GSSTATE_ERROR_CLOSE:
      DestroyWindow( g_hwnd );
      break;
		case GSSTATE_STOP:
#ifdef _CHECK_SERVER_DATA_FILE_VERSION_
      if( IntiAllData() )
      {
        CreateDialog(g_hins, (LPCTSTR)IDD_DLG_CHECKDATA, g_hwnd, (DLGPROC)CheckData);
        g_pGs->SetState(GSSTATE_CHECK_ALL_DATA_RELATION);
      }
      else
      {
        ::MessageBox( NULL, "Init Game Data Failed, Please Check Log File !\n Exit...", "Error", 0 );
        g_pGs->SetState( GSSTATE_CLOSE );
      }
#else
      if( g_dwGsAutoRunTime < ThisTickCount )
			{
				// Run Gs If The Auto Run Time Out
#ifdef _DEBUG_USE_CHILDWINDOW_MENU_
				SendMessage( g_hwnd, WM_COMMAND, MAKELONG( ID_TOOLS, 0 ), 0 );
#else
				SendMessage( g_hwnd, WM_COMMAND, MAKELONG( IDM_RUN, 0 ), 0 );
#endif
			}
#endif
			break;
    default:
      break;
  }
	Sleep(1);
}

void GsDeleteThread()
{
  SAFE_DELETE( g_pAcceptThread );
  SAFE_DELETE( g_pNetThread );
  SAFE_DELETE( g_pMccThread );
#ifdef _DEBUG_USE_SPECIAL_THREAD_MAINLOOP_
  SAFE_DELETE( g_pMccThread );
#endif
  ReleaseAllData();
}
#ifdef _DEBUG_USE_SPECIAL_THREAD_MAINLOOP_
//=====================================================================================
//
//
DWORD WINAPI MainLoopExecute(LPVOID lpParameter)
{
  MainLoop_Thread_Handle    *ThisNetwork = (MainLoop_Thread_Handle*)lpParameter;
  //
  while( !ThisNetwork->IsExit() )
  {
    GsMainLoop();
  }
  ExitThread( 0 );
  return 1;
}
#endif

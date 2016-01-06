#ifndef __SRVSKILL_H__
#define __SRVSKILL_H__

#include "../../Main/Bf_Global.h"
#include "../../SrvClass/Map/SrvMap.h"

class CLife;
class CPlayer;
class CGameMap;
class CItem;
//
//#define _DEBUG_OPEN_MONSTER_DROP_ITEM_LOG_
//
//////////////////////////////////////////////////
class CSrvBaseSkill  //relate with the load skill file
{
protected:
  WORD	m_wId;																	// Skill's Id
  char  m_szName[MAX_SKILL_NAME_LEN];						// Skill's Name
  WORD	m_wIconId;															// Icon Used On Interface
	WORD	m_wPracticeMax;			// 这个SKILL升级到AdvanceSkill需要的熟练度
  WORD	m_wAdvanceSkillId;	// The Advanced Skill ID Of The This Skill
  WORD	m_wType;						// SKILL的类型，分为3大类别: 幻道专用型，其他3职业公用型，道具功能型
	//WORD  m_wAddiType;        // 当 Skill 的 m_wType 为攻击招式或攻击咒时，此栏位表示此招式的异常
  int		m_iEleEffect;				// 幻道的咒术对五行加成的伤害力的影响%,当element和敌人属性相克时有效
  int   m_iCostHp;					// Cost Hp
	int		m_iCostHpPer;				// Cost Hp %
  int   m_iCostMp;					// Cost Mp
	int		m_iCostMpPer;				// Cost Mp %
  int   m_iCostSp;					// Cost Sp
	int		m_iCostSpPer;				// Cost Sp %
	int		m_iCostSoul;				// Cost Soul
	WORD	m_bBlock;						// The Long Space Of A Skill
	int		m_iAlarmTime;				// 法术的持续时间,(Second)
	WORD	m_wAlarmSkillId;		// 当m_iAlarmTime的时间到达的时候，产生的效果对应的SKILL ID
	int		m_iTriggerTime;			// 周期效果需要的时间周期
	WORD	m_wTriggerSkillId;	// 周期效果对应的SKILL ID
	int		m_iHpChange;				// 法术对HP的改变量
	int		m_iMpChange;				// 法术对MP的改变量
	int		m_iSpChange;				// 法术对SP的改变量
	int		m_iSoulChange;			// 法术对soul的改变量
	int		m_iApChange;				// 法术对Ap的改变量
	int		m_iHitChange;				// 法术对Hit的改变量
	int		m_iDpChange;				// 法术对Dp的改变量
	int		m_iDgChange;				// 法术对Dg的改变量
	int		m_iIntChange;				// 法术对Intelligence的改变量	
  int   m_iBearPsbChange;   // 法术对霸体的改变量
	int   m_iRandom;					// Random Attack Point
	int   m_iCriticalHit;			// Critical Attack Hit Probability
	int		m_iHard;						// 咒术的硬度

	QWORD	m_qwStatus;					// 这个法术可以设置life状态的种类
	WORD	m_wSkillLearnId;		// 对应于学习的SKILL Id
	WORD	m_bPosition;
	WORD	m_bColor;           // 当 Skill 的 m_wType 为攻击招式或攻击咒时，此栏位表示此招式的异常
                            // 当 Skill 的 m_wType 为辅助咒时，此栏位表示此招式的攻击性
                            // 0: 非攻击性; 1: 攻击性;
	WORD	m_bCase;
	WORD	m_wWarpMapId;
	WORD	m_wWarpX;
	WORD	m_wWarpY;
	WORD	m_wFuncChangeId;
	int		m_iItemHm;
	int		m_iItemHp;
	int		m_iItemHard;
	int		m_iItemHu;
	int		m_iTimes;          // 对使用者速度的改变值
	int 	m_iProbability;
	WORD	m_wRace;
	WORD	m_wMonsterId;
	WORD	m_wChangeItemId;
	WORD	m_wUseEffect;
	WORD	m_wObjEffect;
	int   m_wBeforeDelay;
	int   m_wAfterDelay;			// 
	WORD	m_bTarget;					// 目标类型
	WORD	m_bShape;						// SKILL作用范围形状
	WORD	m_wRangeForShape;		// Shape对应的攻击距离(与企划设定值用途不一样)
	WORD	m_bRange;						// 目标到使用者的距离最大值
	WORD  m_wTargetCount;			// 最大目标数量
	WORD	m_wLoopCount;				// 此招式的Target Check循环次数

  // 克制怪物的相关属性
  WORD      m_wElement;               // 克制怪物的附加属性－－五行, 体形, 相性( 恶, 中立, 善 )
                                      //
                                      //
                                      //
                                      //
                                      //

  DWORD     m_dwRaceAttri;            // 克制怪物的种族属性－－怪物的类型
                                      //
                                      //
                                      //
                                      //
                                      //
                                      //
                                      //

  WORD      m_wRaceBonuRate;          // 克制怪物的种族属性的加成倍率

  DWORD     m_dwBossCode;             // 克制魔王的属性－－对应一( 两 )个魔王的Id
                                      // HI 16Bit: 第二个魔王Id;
                                      // LO 16Bit: 第一个魔王Id;

  WORD      m_wBossBonuRate;          // 克制魔王的属性的加成倍率

  WORD      m_wOwnAttrEffect;         // 招式与人物属性之间的加成关系对应

  WORD      m_wOwnAttrBonuRate;       // 招式与人物属性之间的加成倍率

  BOOL      m_bDrain;                 // 是否吸血
  int       m_iChain;                 // 连发的数量
  int       m_iIntonateTime;          // 念咒时间
  int       m_iBossSpecialTimeRate;   // BOSS异常状态的时间与其他小怪相对的百分比
  // About Special Status And Race Attribute Forbear
  WORD      m_wSpecialCount;
  DWORD     m_dwSimSpecial[32];
  WORD      m_wRaceCount;
  DWORD     m_dwSimRaceAttr[32];
  //////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-05
  WORD      m_wCostMana;
  //////////////////////////////////////////////////////////////////////
public:

	CSrvBaseSkill();
	~CSrvBaseSkill();
  
  inline  int GetIntonateTime()             { return m_iIntonateTime; }
  inline  BOOL IsDrain()                    { return m_bDrain; }
  inline  int GetChain()                    { return m_iChain; }
	inline	WORD	GetId()											{ return m_wId; }
	inline	char*	GetName()										{ return m_szName; }
	inline	WORD	GetIconId()									{ return m_wIconId; }
	inline	WORD	GetPracticeMax()						{ return m_wPracticeMax; }
	inline	WORD  GetAdvanceSkillId()					{ return m_wAdvanceSkillId; }
  inline	WORD	GetType()										{ return m_wType; }
  // New For 2003-2-13
  inline	WORD  GetElement()								{ return m_wElement;      }
  inline  DWORD GetRaceAttr()               { return m_dwRaceAttri;   }
  inline  WORD  GetRaceBonu()               { return m_wRaceBonuRate; }
  inline  DWORD GetBossCode()               { return m_dwBossCode;    }
  inline  WORD  GetBossBonu()               { return m_wBossBonuRate; }
  inline  WORD  GetAttrEffect()             { return m_wOwnAttrEffect;}
  inline  WORD  GetAttrBonu()               { return m_wOwnAttrBonuRate;}
  // End
	inline	int		GetEleEffect()							{ return m_iEleEffect; }
  inline	int   GetCostHp()									{ return m_iCostHp; }
  inline	int   GetCostHpPer()							{ return m_iCostHpPer; }
  inline	int   GetCostMp()									{ return m_iCostMp; }
  inline	int   GetCostMpPer()							{ return m_iCostMpPer; }
  inline	int   GetCostSp()									{ return m_iCostSp; }
  inline	int   GetCostSpPer()							{ return m_iCostSpPer; }
  inline	int   GetCostSoul()								{ return m_iCostSoul; }
  /////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-08
          int   GetCostMana()               { return m_wCostMana; }
          int   GetBearPosChange()          { return m_iBearPsbChange; }
  //////////////////////////////////////////////////////////////////
  inline	WORD  GetBlock()									{ return m_bBlock; }
	inline	int		GetAlarmTime()							{ return m_iAlarmTime; }
	inline	WORD	GetAlarmSkillId()						{ return m_wAlarmSkillId; }
	inline	int		GetTriggerTime()						{ return m_iTriggerTime;	}
	inline	WORD	GetTriggerSkillId()					{ return m_wTriggerSkillId; }
	inline	int		GetHpChange()								{ return m_iHpChange;	}
	inline	int		GetMpChange()								{ return m_iMpChange;	}
	inline	int		GetSpChange()								{ return m_iSpChange;	}
	inline	int		GetSoulChange()							{ return m_iSoulChange; }
	inline	int		GetApChange()								{ return m_iApChange;	}
	inline	int		GetHitChange()							{ return m_iHitChange; }
	inline	int		GetDpChange()								{ return m_iDpChange;	}
	inline	int		GetDgChange()								{ return m_iDgChange; }
	inline	int		GetIntChange()							{ return m_iIntChange; }
  inline	float GetRandom()									{ return (float)gf_GetRandom(m_iRandom)/100; }
  inline	int   GetCriticalHit()						{ return m_iCriticalHit; }
	inline	int		GetHard()										{ return m_iHard;	}
	inline	QWORD	GetStatus()									{ return m_qwStatus; }
	inline	WORD	GetSkillLearnId()						{ return m_wSkillLearnId; }
	inline	WORD	GetPosition()								{ return m_bPosition; }
	inline	WORD	GetColor()									{ return m_bColor; }
	inline	WORD	GetCaseType()								{ return m_bCase; }
	inline	WORD	GetWarpMapId()							{ return m_wWarpMapId; }
	inline	WORD	GetWrapX()									{ return m_wWarpX; }
	inline	WORD	GetWarpY()									{ return m_wWarpY; }
	inline	WORD	GetFuncChangeId()						{ return m_wFuncChangeId; }
	inline	int		GetItemHm()									{ return m_iItemHm; }
	inline	int		GetItemHp()									{ return m_iItemHp; }
	inline	int		GetItemHard()								{ return m_iItemHard; }
	inline	int		GetItemHu()									{ return m_iItemHu; }
	inline	int		GetTimes()									{ return m_iTimes; }
	inline	int 	GetProbability()						{ return m_iProbability; }
	inline	WORD	GetRace()										{ return m_wRace; }
	inline	WORD	GetMonsterId()							{ return m_wMonsterId; }
	inline	WORD	GetChangeItem()							{ return m_wChangeItemId; }
	inline	WORD	GetUseEffect()							{ return m_wUseEffect; }
	inline	WORD	GetObjEffect()							{ return m_wObjEffect;	}
  inline	int   GetBeforeDelay()						{ return m_wBeforeDelay; }
  inline	int   GetAfterDelay()							{ return m_wAfterDelay; }
	inline	WORD	GetTargetType()							{ return m_bTarget; }
	inline	WORD	GetShape()									{ return m_bShape; }
	inline	WORD	GetRange()									{ return m_bRange; }
	inline  WORD	GetShapeRange()							{ return m_wRangeForShape; }
	inline  WORD	GetLoopCount()							{ return m_wLoopCount; }

	inline	bool	IsElement(WORD elem)				{ return ((m_wElement&elem)!=0); }
	inline	bool	IsCommonSkill()							{ return (m_wType >= SKILL_TYPE_OCCU_A1 && m_wType <= SKILL_TYPE_WIZARD_SKILL); }
  inline  bool  IsAttackCurse()             { return (m_wType==SKILL_TYPE_WIZARD_ATTACK); }
  inline  bool  IsEnhanceCurse()            { return (m_wType==SKILL_TYPE_WIZARD_ENHANCE); }
  inline  bool  IsMagic()                   { return (m_wType==SKILL_TYPE_WIZARD_MAGIC); }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-06
          bool  IsEvilSummon()              { return ( (m_wType>=SKILL_TYPE_EVILWEAPON_OPENDOOR) && (m_wType<=SKILL_TYPE_EVILWEAPON_DELINT) ); }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  inline WORD GetBitCountForRace( const DWORD & dwData );
         BYTE GetHitRange(const POINT &ptTarget, CGameMap *pTheMap, const int &iDir, POINT* pAllTargets = NULL);
	//*******************************************************************************
	//			功能函数
	//*******************************************************************************


	// About Functions
	bool CheckCost(CPlayer *pTheUser);			              // Check Player Current Skill Cost
  bool DoCost(CPlayer *pTheUser);				                // Do Skill Cost 
	void ChangeChar(CLife * pLife);				                // 时效性改变玩家属性
	void DoChangeChar(CLife * pUser,CSkill* pSkill);			// 时效性改变玩家属性
	void SetSpecialState(CLife * pLife);		              // 时效性改变玩家状态
	int  DoSetSpecialState(CLife * pUser,CSkill* pSkill);	// 时效性改变玩家状态
	void ClearStatus(CLife *pUser,CSkill* pSkill);				// 清空状态
	bool ChangeElement(CLife *pUser, CSkill *pSkill);			// 时效性改变目标五行属性
	void Heal(CLife *pLife,CSkill* pSkill);								// 总的Heal
  void ClearTrap(CPlayer *pUser);                       // 使用清除陷阱的魔法

  //
	inline void	  Heal1(CLife *pLife);							// 直接增减Hp, Mp, Sp
	inline void	  Heal2(CLife *pLife);							// 以Max%增减Hp, Mp, Sp
	inline void	  Heal3(CLife *pLife);							// 改变MaxHp, MaxMp, MaxSp
	inline void	  Heal4(CLife *pLife);							// 时效性改变Hp, Mp, Sp

	//============= 不需要再进一步确定目标的功能函数
	//
	//
	void	SetAlarm(CItem * pItem);					// 在一个Item上设置Alarm
	void	EvilSword(CItem * pItem);					// 妖刀
	void	Insert();													// 镶嵌
	void	ChangeItem(CItem* pItemForChange);// 置换Item
	void	SetSkillEle(CSkill * pSkill);			// 更换Skill的五行属性

	bool	LearnSkill(CPlayer *pPlayer);							    // 技能书学习
	bool	Color(CPlayer *pPlayer);									    // 改变人物Pic Id
	bool	Warp(CPlayer *pUser,CItem* pItem);				    // 传送
	void	SetItem(CItem *pItem);										    // 改变Item的数值
  void  Revive(CLife *pUser, CSkill *pSkill);         // 复活
#ifdef _REVIVE_JUJU_
  BOOL  ReviveItem2Skill(CPlayer *pPlayer);
#endif
#ifdef _SLOW_JUJU_
  void  SlowJuJuItem2Skill(CPlayer *pPlayer, CSrvBaseSkill *pSkill);   //迟缓符
#endif  
  void  ChangeBearPos(CLife *pUser, CSkill *pSkill);  // 改变霸体几率
#ifdef _MUSIC_BOX_
  BOOL  UseMusicBox(CLife *pUser, CItem *pDbcItem);    //音乐盒
#endif  
#ifdef DEL_INSERT
  BOOL  DelInsert(CPlayer * pUser, int iWhere);
#endif
#ifdef FUNCTION_DENATURALIZATION
  BOOL  ChangeSex(CPlayer *pUser, CItem *pDbcItem);
#endif
#ifdef _DEBUG_MICHAEL_OPEN_GUILDMASTER_SKILL_
	BOOL  DoGuildSkill( CSkill* pSkill );
	BOOL  SetGuildSkillState( CLife* pPlayer );
#endif

	int  GetOwnAttributeForHuan(CLife* pUser);
	void GetMonsterRelateJinx( CMonster * pTMonster, float & F_M_Jinx,float & RaceJinx,float & BossJinx );

#ifdef _DEBUG_MICHAEL_TESSERA_EX_
  // About Suit Skill
  void SetPlayerSuitSkill( CPlayer * pPlayer );
	void SetPlayerTesseraSkill(   CPlayer * pPlayer, BOOL bAdjust = TRUE );
	void UnsetPlayerTesseraSkill( CPlayer * pPlayer );
#endif
  friend class CMonster;
  friend class CSrvBaseData;
  friend class CSkill;
  friend class CMagic;
  friend class CGsData;
  ////////////////////////////
  //Add by CECE 2004-04-08
#ifdef  EVILWEAPON_3_6_VERSION
  friend class CEvilWeapon;
#endif
  //
};
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Wizard skill data  
//
//////////////////////////////////////////////////////////////////////////////////////////////
class CWizardSkillData
{
public:
  float   m_iFinalInt;
  float   m_iFinalCostHp;
  float   m_iFinalCostMp;
  float   m_iFinalCostSp;
  int     m_iFinalCostSoul;
  int     m_iFinalBeforeTime;   // 捻咒的时间
  int     m_iFinalAfterTime;    // 捻咒结束的时间
  BOOL    m_bDrain;             // 是否吸血
  int     m_iChain;             // 连发的数量
  //
  //DWORD   m_dwSpeStatus;
  //DWORD		m_dwAlarmTime[MAX_TIMER_COUNT];
  //WORD	  m_wFuncAlarm[MAX_TIMER_COUNT];
	//DWORD		m_dwTrigger[MAX_TIMER_COUNT][2];
	//WORD		m_wFuncTrigger[MAX_TIMER_COUNT];
  //
  CWizardSkillData()
  {
    m_iFinalInt         = 0;
    m_iFinalCostHp      = 0;
    m_iFinalCostMp      = 0;
    m_iFinalCostSp      = 0;
    m_iFinalCostSoul    = 0;
    m_iFinalBeforeTime  = 0;  // 捻咒的时间
    m_iFinalAfterTime   = 0;  // 捻咒结束的时间
    m_bDrain            = 0;  // 是否吸血
    m_iChain            = 0;  // 连发的数量
  }
  ~CWizardSkillData()
  {
  }
  //
  inline void   Clear()
  {
    m_iFinalInt         = 0;
    m_iFinalCostHp      = 0;
    m_iFinalCostMp      = 0;
    m_iFinalCostSp      = 0;
    m_iFinalCostSoul    = 0;
    m_iFinalBeforeTime  = 0;  // 捻咒的时间
    m_iFinalAfterTime   = 0;  // 捻咒结束的时间
    m_bDrain            = 0;  // 是否吸血
    m_iChain            = 0;  // 连发的数量
  }
};
//////////////////////////////////////////////////////////////////////////////////////////////
//
//    class CSkill
//
//////////////////////////////////////////////////////////////////////////////////////////////
class CSkill
{
  CSrvBaseSkill     *m_pBase;			      // BASE SKILL

  // 克制怪物的相关属性
  WORD              m_wElement;         // 克制怪物的附加属性－－五行, 体形, 相性( 恶, 中立, 善 )

  WORD						  m_iLevel;			      // level of this skill
  WORD						  m_iExp;				      // experience of this skill
	WORD						  m_wPos;

	CPlayer*				  m_pPlayer;
	CMonster*				  m_pMonster;

  list<CLife*>		  m_listTargets;	
	list<CLife*>		  m_listMiss;

  DWORD             m_dwSrvCode;
  WORD              m_wMemState;
  WORD              m_wElementCount;
  WORD              m_wSimElement[16];
  // New For 2003-2-13
  inline	WORD  GetElement()								{ return m_wElement;               }
  inline  DWORD GetRaceAttr()               { return m_pBase->m_dwRaceAttri;   }
  inline  WORD  GetRaceBonu()               { return m_pBase->m_wRaceBonuRate; }
  inline  DWORD GetBossCode()               { return m_pBase->m_dwBossCode;    }
  inline  WORD  GetBossBonu()               { return m_pBase->m_wBossBonuRate; }
  inline  WORD  GetAttrEffect()             { return m_pBase->m_wOwnAttrEffect;}
  inline  float GetAttrBonu()               { return ((float)m_pBase->m_wOwnAttrBonuRate/100);}
  inline  int   GetTimes()                  { return m_pBase->GetTimes(); }
  inline  WORD  GetColor()                  { return m_pBase->GetColor(); }
  // End
  
  inline  void  InitElement(WORD Element)
  {
    m_wElement = Element;
    m_wElementCount = 0;
    memset( m_wSimElement, 0, sizeof( WORD ) * 16 );
    for( int i = 0; i < 16; i++ )
    {
      if( ( m_wElement & ( 1 << i ) ) )
      {
        m_wSimElement[m_wElementCount] = ( 1 << i );
        m_wElementCount++;
      }
    }
  }
#ifdef ZEOTOR_TEMP_DEBUG_EVILWEAPON
  inline  void  SetOwner( CPlayer* pThePlayer ){ m_pPlayer = pThePlayer;}
#endif
  inline  float GetChainJinx();
  inline  int   GetOwnAttribute();
  inline  int   GetOwnAttributeForHuan();
  inline  int   GetWpRandom(CLife *pTarget);
  inline  float GetElementJinx(CLife* pAttacker,CLife* pTarget);
  inline  float GetShieldTesseraJinx(CPlayer* pTarget,CLife *pAttacker);
  inline  float GetWeaponTesseraJinx(CLife *pAttacker,CLife* pTarget);
  inline  void  GetMonsterRelateJinx(CMonster *pTMonster, float& F_M_Jinx, float& RaceJinx, float& BossJinx);
	// About User
	inline BOOL IsPlayer()                { return (m_pPlayer  != NULL); }
	inline BOOL IsMonster()               { return (m_pMonster != NULL); }
  inline CPlayer  *GetPlayerUser()      { return m_pPlayer;  }
  inline CMonster *GetMonsterUser()     { return m_pMonster; }

	//*************************************************************************************
	//         处理攻击伤害的相关过程
	//*************************************************************************************
	inline bool	MonsterAttackPlayer(CPlayer *pTarget,SNMDamage *pDamageInfo);
	inline bool	PlayerCommonAttack(CLife *pTarget,SNMDamage *pDamageInfo,int iAddiDamage = 0);
	inline bool	PlayerCurseAttack(CLife *pTarget,SNMDamage *pDamageInfo,int iAddiDamage = 0);

  inline short int  CalDamageForWizard(CLife *pTarget, int iAddiDamage = 0);
	inline short int	CalDamageForPlayer(CLife *pTarget, BOOL bCritical, int iAddiDamage = 0);
	inline short int  CalDamageForMonster(CPlayer *pTarget, BOOL bCritical);
#ifdef RESTRAIN_EVIL   
  inline float      RestrainForevil( CLife *pTarget );     //克恶血珀
#endif
#ifdef RESTRAIN_TRAP
  inline float      RestrainFortrap(void);                 //以一定的几率免疫风水术
#endif
#ifdef RESTRAIN_BODY
  inline float      RestrainForbody(CLife *pTarget);       //克体型血珀
#endif
#ifdef DEPRESS_TESS_DEFENCE
  inline int        DepressDefence();                      //降低反弹
#endif
	inline int  IsMonsterCritical(CPlayer * pTarget);
	inline int  IsPlayerCritical(CLife * pTarget);
	inline bool	MonsterCheckHit(CPlayer *pTarget);
	inline bool	PlayerCheckHit(CLife *pTarget);
	inline WORD	GetChainHitRate();

	inline bool FindTarget();
	inline BYTE GetMonsterHitRange();
	inline void CheckLineD();
	inline void CheckLineW();
	inline void CheckRound();
  inline bool	CalItemWaste( CLife *pTarget );
  //
  inline void    DoDamageForTrap( CMagic *pMagic );
  inline void    CalAllSpecialStatus( CLife *pLife ); // 做普通攻击和攻击咒的异常状态

public:
  CSkill();
	~CSkill();

	inline CSrvBaseSkill*	GetBaseSkill(){ return m_pBase;									}
	inline char*		GetBaseName()				{ return m_pBase->m_szName;				}
	inline int			GetBaseRange()			{ return m_pBase->GetRange(); 		}
	inline int			GetBaseShapeRange()	{ return m_pBase->GetShapeRange();}
	inline int			GetBaseBlock()			{ return m_pBase->GetBlock();			}		
  inline WORD		  GetId()							{ return m_pBase->GetId();				}
  inline int			GetExp()						{ return m_iExp;									}
	inline int			GetMaxExp()					{ return m_pBase->GetPracticeMax();}
  inline WORD  		GetType()						{ return m_pBase->GetType();				}
  inline WORD     GetShape()          { return m_pBase->GetShape();     }
	inline int			GetTargetType()			{ return m_pBase->GetTargetType();	}
	inline int			GetDuration()				{ return m_pBase->GetAlarmTime();		}
  inline int			GetBeforeDelay()		{ return m_pBase->GetBeforeDelay(); }
  inline int			GetAfterDelay()			{ return m_pBase->GetAfterDelay();	}
	inline WORD			GetPos()						{ return m_wPos; }
	inline WORD			GetTargetCount()		{ return m_pBase->m_wTargetCount;   }
  inline int      GetItemHu()         { return m_pBase->GetItemHu();      }
  inline	int		  GetHpChange()			  { return m_pBase->m_iHpChange;	}
	inline	int		  GetMpChange()			  { return m_pBase->m_iMpChange;	}
	inline	int		  GetSpChange()				{ return m_pBase->m_iSpChange;	}
	inline	WORD	  GetChangeItem()			{ return m_pBase->m_wChangeItemId; }
  //////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-07
          int     GetSpeedChange()    { return m_pBase->m_iTimes; }
          int     GetSoulChange()     { return m_pBase->m_iSoulChange; }
  //////////////////////////////////////////////////////////////////////////


	inline void		SetLevel(int iTemp)					{ m_iLevel = iTemp; }
	inline void   SetPos(WORD Pos)						{ m_wPos = Pos; }
  inline bool		DoCost(CPlayer *pTheUser)		{ return m_pBase->DoCost(pTheUser); }
	inline bool		CheckCost(CPlayer *pTheUser){ return m_pBase->CheckCost(pTheUser); }
         bool   DoWizardCost(CPlayer *pTheUser, CWizardSkillData * pNowWizardData);

	// About Attack Ver 2.0
  inline WORD		SetExp( WORD theNewExp )	
	{
		m_iExp = theNewExp;
		if(m_iExp < 0)
		{
			m_iExp = 0;
		}
		else if(m_iExp > m_pBase->GetPracticeMax())
		{
			m_iExp = m_pBase->GetPracticeMax();
		}
		return m_iExp;
	}
  inline WORD		AddExp( const WORD & theAddExp )
	{
		if( m_pBase->GetPracticeMax() == 0 ) return 0;

		m_iExp += theAddExp;
		if( m_iExp < 0 )
		{
			m_iExp = 0;
		}
		else if( m_iExp >= m_pBase->GetPracticeMax() )
		{
			CSrvBaseSkill   *pBaseSkill = g_pBase->GetBaseSkill( m_pBase->GetAdvanceSkillId() );
			if( UpdateLevel( pBaseSkill ) )
			{
				return m_iExp;
			}
			m_iExp = m_pBase->GetPracticeMax();
		}
		return m_iExp;
	}
	BOOL		UpdateLevel(CSrvBaseSkill * pBaseSkill, SMsgData * pTheMsg = NULL);

	BOOL		MonsterUseSkill();
	BOOL		PlayerUseSkill();

  // About Trap And Durative Magic
  inline void    AddTarget( CLife * pTarget )
  {
    m_listTargets.push_back(pTarget);
  }
  inline void    AddMiss( CLife * pTarget )
  {
    m_listMiss.push_back(pTarget);
  }
  inline void    ClearAllTarget()
  {
    m_listTargets.clear();
    m_listMiss.clear();
  }
  //
  void    PlayerDoMagicDamage();
  inline void SetSkillElement( const WORD & wElement )
  {
    if( GetType() >= SKILL_TYPE_WIZARD_ATTACK )    return;
    //
    m_wElement &= 0xFFE0;
	  m_wElement |= wElement;
    //
    m_wElementCount = 0;
    memset( m_wSimElement, 0, sizeof( WORD ) * 16 );
    for( int i = 0; i < 16; i++ )
    {
      if( ( wElement & ( 1 << i ) ) )
      {
        m_wSimElement[m_wElementCount] = ( 1 << i );
        m_wElementCount++;
      }
    }
  }
#ifdef _DEBUG_MICHAEL_TESSERA_EX_
	void OnEquipAttack(  CItem* pItem, CLife * pTarget  );
#endif
	friend class CSrvBaseSkill;
	friend class CMonster;
	friend class CPlayer;
  friend class CMagic;
  friend class CGsData;
  //////////////////////////////////////////
  //Add by CECE 2004-04-08
#ifdef EVILWEAPON_3_6_VERSION
  friend class CEvilWeapon;
  //////////////////////////////////////////////////////////////////////////
  //Add by CECE 2004-04-06
  BOOL DoEvilDamage(CLife *pLife,SNMDamage *pDamageInfo);
  //////////////////////////////////////////////////////////////////////////
#endif
  //////////////////////////////////////////
};
//////////////////////////////////////////////////////////////////////////////////////////////
//
// Class  CMagic
//
//////////////////////////////////////////////////////////////////////////////////////////////
//
//
enum
{
  MAGICSTATE_NONE,
  MAGICSTATE_WAIT,
  MAGICSTATE_MOVE,
  MAGICSTATE_ATTACK,
  MAGICSTATE_DURATIVE_ATTACK,
  MAGICSTATE_DEAD,
};
//////////////////////////////////////////////////////////////////////////////////////////////
//
//
class CMagic
{
	CSkill		      *m_pBase;
	CPlayer					*m_pPlayer;
  CMonster        *m_pMonster;
  //CLife           *m_pMyUser;
  CLife           *m_pFirstTarget;
	CGameMap				*m_pInMap;

  BOOL            m_bCanPK;
	DWORD						m_dwLifeTime;
  DWORD           m_dwActionTime;
	WORD						m_wUpdateTurn;
	WORD						m_wCode;
	int							m_iDir;
	int							m_iStatus;
	WORD						m_iX;
	WORD						m_iY;
  WORD            m_iEndX;
  WORD            m_iEndY;
  //
  DWORD           m_dwSrvCode;
  WORD            m_wMemState;
          void    Move();
          void    ReleaseMyMapSign();
  inline  void    SetMyMapSign();
  inline  void    CheckRange();
  inline  void    IsTrapTarget( CLife * pTarget );
  inline  void    Attack();

public:

	CMagic();
	~CMagic();
#ifdef VERSION_38_FUNCTION
  inline  DWORD     GetLifeTime()               { return m_dwLifeTime;          }           
#endif
  inline  CGameMap  *GetInMap()                 { return m_pInMap;              }
  inline  DWORD     GetMapId()                  { return m_pInMap->GetMapId();  }
	inline	int			  GetCode()							      { return m_wCode;               }
	inline	int			  GetPosX()							      { return m_iX;                  }
	inline	int			  GetPosY()							      { return m_iY;                  }
	inline	int			  GetDir()							      { return m_iDir;                }
	inline	DWORD		  GetId()								      { return m_pBase->GetId();      }
	inline	CSkill    *GetBaseSkill()	            { return m_pBase;               }
  inline	void		  SetXY( int X, int Y )	      {	m_iX = X; m_iY = Y;           }
  inline  void      SetState(const int & State) { m_iStatus = State;            }
	inline	int			  BeUpdated()						      { return (m_wUpdateTurn <= 0);  }
  inline  int       GetStatus()                 { return m_iStatus;             }
          void      UserCancelMe()              { m_iStatus = MAGICSTATE_DEAD;  }
  inline  WORD      GetUserCode();

	        void	    DoAction();
	        void	    BeActivation( CLife *pTarget );

	inline	BOOL	    UpdateTurnCheck();
	inline	int		    GetDistance( int X, int Y );

  friend class CGsData;
  friend class CSkill;
};

extern list<CLife*> g_BaseTargetList;
#endif

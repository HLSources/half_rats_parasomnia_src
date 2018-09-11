/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== lights.cpp ========================================================

  spawn and think functions for editor-placed lights

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

//LRC
int GetStdLightStyle( int iStyle )
{
	switch ( iStyle )
	{
		// 0 normal
	case 0: return MAKE_STRING( "m" );

		// 1 FLICKER (first variety)
	case 1: return MAKE_STRING( "mmnmmommommnonmmonqnmmo" );

		// 2 SLOW STRONG PULSE
	case 2: return MAKE_STRING( "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba" );

		// 3 CANDLE (first variety)
	case 3: return MAKE_STRING( "mmmmmaaaaammmmmaaaaaabcdefgabcdefg" );

		// 4 FAST STROBE
	case 4: return MAKE_STRING( "mamamamamama" );

		// 5 GENTLE PULSE 1
	case 5: return MAKE_STRING( "jklmnopqrstuvwxyzyxwvutsrqponmlkj" );

		// 6 FLICKER (second variety)
	case 6: return MAKE_STRING( "nmonqnmomnmomomno" );

		// 7 CANDLE (second variety)
	case 7: return MAKE_STRING( "mmmaaaabcdefgmmmmaaaammmaamm" );

		// 8 CANDLE (third variety)
	case 8: return MAKE_STRING( "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa" );

		// 9 SLOW STROBE (fourth variety)
	case 9: return MAKE_STRING( "aaaaaaaazzzzzzzz" );

		// 10 FLUORESCENT FLICKER
	case 10: return MAKE_STRING( "mmamammmmammamamaaamammma" );

		// 11 SLOW PULSE NOT FADE TO BLACK
	case 11: return MAKE_STRING( "abcdefghijklmnopqrrqponmlkjihgfedcba" );

		// 12 UNDERWATER LIGHT MUTATION
		// this light only distorts the lightmap - no contribution
		// is made to the brightness of affected surfaces
	case 12: return MAKE_STRING( "mmnnmmnnnmmnn" );

		// 13 OFF (LRC)
	case 13: return MAKE_STRING( "a" );

		// 14 SLOW FADE IN (LRC)
	case 14: return MAKE_STRING( "aabbccddeeffgghhiijjkkllmmmmmmmmmmmmmm" );

		// 15 MED FADE IN (LRC)
	case 15: return MAKE_STRING( "abcdefghijklmmmmmmmmmmmmmmmmmmmmmmmmmm" );

		// 16 FAST FADE IN (LRC)
	case 16: return MAKE_STRING( "acegikmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm" );

		// 17 SLOW FADE OUT (LRC)
	case 17: return MAKE_STRING( "llkkjjiihhggffeeddccbbaaaaaaaaaaaaaaaa" );

		// 18 MED FADE OUT (LRC)
	case 18: return MAKE_STRING( "lkjihgfedcbaaaaaaaaaaaaaaaaaaaaaaaaaaa" );

		// 19 FAST FADE OUT (LRC)
	case 19: return MAKE_STRING( "kigecaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" );

	default: return MAKE_STRING( "m" );
	}
}

class CLight : public CPointEntity
{
public:
	virtual void	KeyValue( KeyValueData* pkvd ); 
	virtual void	Spawn( void );
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual STATE GetState( void ) { return m_iState; } // LRC
	void SetCorrectStyle( void ); // LRC
	void SetStyle( int iszPattern ); // LRC
	void Think( void ); // LRC
	BOOL ShouldToggle( USE_TYPE useType ); // LRC
private:
	STATE	m_iState; // LRC
	int		m_iszCurrentStyle; // LRC
	int		m_iOnStyle; // LRC
	int		m_iOffStyle; // LRC
	int		m_iTurnOnStyle; // LRC
	int		m_iTurnOffStyle; // LRC
	float	m_flTurnOnTime; // LRC
	float	m_flTurnOffTime; // LRC

	int		m_iszPattern;
	int		m_iStyle;
};
LINK_ENTITY_TO_CLASS( light, CLight );

TYPEDESCRIPTION	CLight::m_SaveData[] =
{
	DEFINE_FIELD( CLight, m_iszPattern, FIELD_STRING ),
	DEFINE_FIELD( CLight, m_iStyle, FIELD_INTEGER ),

	DEFINE_FIELD( CLight, m_iState, FIELD_INTEGER ), // LRC
	DEFINE_FIELD( CLight, m_iOnStyle, FIELD_INTEGER ), // LRC
	DEFINE_FIELD( CLight, m_iOffStyle, FIELD_INTEGER ), // LRC
	DEFINE_FIELD( CLight, m_iTurnOnStyle, FIELD_INTEGER ), // LRC
	DEFINE_FIELD( CLight, m_iTurnOffStyle, FIELD_INTEGER ), // LRC
	DEFINE_FIELD( CLight, m_flTurnOnTime, FIELD_TIME ), // LRC
	DEFINE_FIELD( CLight, m_flTurnOffTime, FIELD_TIME ), // LRC
	DEFINE_FIELD( CLight, m_iszCurrentStyle, FIELD_STRING ) // LRC
};

IMPLEMENT_SAVERESTORE( CLight, CPointEntity );


//
// Cache user-entity-field values until spawn is called.
//
// LRC
void CLight :: KeyValue( KeyValueData* pkvd )
{
	if ( FStrEq( pkvd->szKeyName, "m_iOnStyle" ) )
	{
		m_iOnStyle = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iOffStyle" ) )
	{
		m_iOffStyle = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iTurnOnStyle" ) )
	{
		m_iTurnOnStyle = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_iTurnOffStyle" ) )
	{
		m_iTurnOffStyle = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flTurnOnTime" ) )
	{
		m_flTurnOnTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "m_flTurnOffTime" ) )
	{
		m_flTurnOffTime = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "pitch" ) )
	{
		pev->angles.x = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "pattern" ) )
	{
		m_iszPattern = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "firetarget" ) )
	{
		pev->target = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq( pkvd->szKeyName, "style" ) )
	{
		m_iStyle = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue( pkvd );
}

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) LIGHT_START_OFF
Non-displayed light.
Default light value is 300
Default style is 0
If targeted, it will toggle between on or off.
*/

void CLight :: Spawn( void )
{
	if (FStringNull(pev->targetname))
	{       // inert light
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	// LRC
	if ( FBitSet( pev->spawnflags, SF_LIGHT_START_OFF ) )
		m_iState = STATE_OFF;
	else
		m_iState = STATE_ON;

	SetCorrectStyle();
}

BOOL CLight::ShouldToggle( USE_TYPE useType )
{
	STATE currentState = GetState();
	if ( useType != USE_TOGGLE && useType != USE_SET )
	{
		switch ( currentState )
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if ( useType == USE_ON )
				return FALSE;
			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if ( useType == USE_OFF )
				return FALSE;
			break;
		}
	}
	return TRUE;
}

void CLight :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( m_iStyle >= 32 )
	{
		if ( !ShouldToggle( useType ) )
			return;

		switch ( GetState() )
		{
		case STATE_ON:
		case STATE_TURN_ON:
			if ( m_flTurnOffTime )
			{
				m_iState = STATE_TURN_OFF;
				pev->nextthink = gpGlobals->time + m_flTurnOffTime;
			}
			else
				m_iState = STATE_OFF;
			break;
		case STATE_OFF:
		case STATE_TURN_OFF:
			if ( m_flTurnOnTime )
			{
				m_iState = STATE_TURN_ON;
				pev->nextthink = gpGlobals->time + m_flTurnOnTime;
			}
			else
				m_iState = STATE_ON;
			break;
		}
	}
	SetCorrectStyle();
}

// LRC
void CLight::SetCorrectStyle( void )
{
	if ( m_iStyle >= 32 )
	{
		switch ( m_iState )
		{
		case STATE_ON:
			if ( m_iszPattern )
				SetStyle( m_iszPattern );
			else if ( m_iOnStyle )
				SetStyle( GetStdLightStyle( m_iOnStyle ) );
			else
				SetStyle( MAKE_STRING( "m" ) );
			break;
		case STATE_OFF:
			if ( m_iOffStyle )
				SetStyle( GetStdLightStyle( m_iOffStyle ) );
			else
				SetStyle( MAKE_STRING( "a" ) );
			break;
		case STATE_TURN_ON:
			if ( m_iTurnOnStyle )
				SetStyle( GetStdLightStyle( m_iTurnOnStyle ) );
			else
				SetStyle( MAKE_STRING( "a" ) );
			break;
		case STATE_TURN_OFF:
			if ( m_iTurnOffStyle )
				SetStyle( GetStdLightStyle( m_iTurnOffStyle ) );
			else
				SetStyle( MAKE_STRING( "m" ) );
			break;
		}
	}
	else
		m_iszCurrentStyle = GetStdLightStyle( m_iStyle );
}

// LRC
void CLight::Think( void )
{
	switch ( GetState() )
	{
	case STATE_TURN_ON:
		m_iState = STATE_ON;
		FireTargets( STRING( pev->target ), this, this, USE_ON, 0.0f );
		break;
	case STATE_TURN_OFF:
		m_iState = STATE_OFF;
		FireTargets( STRING( pev->target ), this, this, USE_OFF, 0.0f );
		break;
	}
	SetCorrectStyle();
}

// LRC
void CLight::SetStyle( int iszPattern )
{
	if ( m_iStyle < 32 )
		return;

	m_iszCurrentStyle = iszPattern;
	LIGHT_STYLE( m_iStyle, (char *)STRING( iszPattern ) );
}

//
// shut up spawn functions for new spotlights
//
LINK_ENTITY_TO_CLASS( light_spot, CLight );


class CEnvLight : public CLight
{
public:
	void	KeyValue( KeyValueData* pkvd ); 
	void	Spawn( void );
};

LINK_ENTITY_TO_CLASS( light_environment, CEnvLight );

void CEnvLight::KeyValue( KeyValueData* pkvd )
{
	if (FStrEq(pkvd->szKeyName, "_light"))
	{
		int r, g, b, v, j;
		char szColor[64];
		j = sscanf( pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v );
		if (j == 1)
		{
			g = b = r;
		}
		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow( r / 114.0, 0.6 ) * 264;
		g = pow( g / 114.0, 0.6 ) * 264;
		b = pow( b / 114.0, 0.6 ) * 264;

		pkvd->fHandled = TRUE;
		sprintf( szColor, "%d", r );
		CVAR_SET_STRING( "sv_skycolor_r", szColor );
		sprintf( szColor, "%d", g );
		CVAR_SET_STRING( "sv_skycolor_g", szColor );
		sprintf( szColor, "%d", b );
		CVAR_SET_STRING( "sv_skycolor_b", szColor );
	}
	else
	{
		CLight::KeyValue( pkvd );
	}
}


void CEnvLight :: Spawn( void )
{
	char szVector[64];
	UTIL_MakeAimVectors( pev->angles );

	sprintf( szVector, "%f", gpGlobals->v_forward.x );
	CVAR_SET_STRING( "sv_skyvec_x", szVector );
	sprintf( szVector, "%f", gpGlobals->v_forward.y );
	CVAR_SET_STRING( "sv_skyvec_y", szVector );
	sprintf( szVector, "%f", gpGlobals->v_forward.z );
	CVAR_SET_STRING( "sv_skyvec_z", szVector );

	CLight::Spawn( );
}

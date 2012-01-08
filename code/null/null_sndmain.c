#include "../client/client.h"
#include "../client/snd_local.h"
#include "../client/snd_public.h"

cvar_t *s_volume;
cvar_t *s_muted;
cvar_t *s_musicVolume;
cvar_t *s_doppler;
cvar_t *s_backend;
cvar_t *s_muteWhenMinimized;
cvar_t *s_muteWhenUnfocused;

static soundInterface_t si;

void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx ) { }
void S_StartLocalSound( sfxHandle_t sfx, int channelNum ) { }
void S_StartBackgroundTrack( const char *intro, const char *loop ) { }
void S_StopBackgroundTrack( void ) { }
void S_RawSamples (int stream, int samples, int rate, int width, int channels,
		   const byte *data, float volume, int entityNum) { }
void S_StopAllSounds( void ) { }
void S_ClearLoopingSounds( qboolean killall ) { }
void S_AddLoopingSound( int entityNum, const vec3_t origin,
			const vec3_t velocity, sfxHandle_t sfx ) { }
void S_AddRealLoopingSound( int entityNum, const vec3_t origin,
			    const vec3_t velocity, sfxHandle_t sfx ) { }
void S_StopLoopingSound( int entityNum ) { }
void S_Respatialize( int entityNum, const vec3_t origin,
		     vec3_t axis[3], int inwater ) { }
void S_UpdateEntityPosition( int entityNum, const vec3_t origin ) { }

void S_Update( void )
{
	if(s_muted->integer)
	{
		if(!(s_muteWhenMinimized->integer && com_minimized->integer) &&
		   !(s_muteWhenUnfocused->integer && com_unfocused->integer))
		{
			s_muted->integer = qfalse;
			s_muted->modified = qtrue;
		}
	}
	else
	{
		if((s_muteWhenMinimized->integer && com_minimized->integer) ||
		   (s_muteWhenUnfocused->integer && com_unfocused->integer))
		{
			s_muted->integer = qtrue;
			s_muted->modified = qtrue;
		}
	}
}

void S_DisableSounds( void ) { }
void S_BeginRegistration( void ) { }
sfxHandle_t	S_RegisterSound( const char *sample, qboolean compressed ) { return 0; }
void S_ClearSoundBuffer( void ) { }
void S_SoundInfo( void ) { }
void S_SoundList( void ) { }

#ifdef USE_VOIP
void S_StartCapture( void ) { }
int S_AvailableCaptureSamples( void ) { return 0; }
void S_Capture( int samples, byte *data ) { }
void S_StopCapture( void ) { }
void S_MasterGain( float gain ) { }
#endif

void S_Play_f( void ) { }
void S_Music_f( void ) { }
void S_StopMusic_f( void ) { }

void S_Init( void )
{
	cvar_t		*cv;

	Com_Printf( "------ Initializing Fake Sound ------\n" );

	s_volume = Cvar_Get( "s_volume", "0.8", CVAR_ARCHIVE );
	s_musicVolume = Cvar_Get( "s_musicvolume", "0.25", CVAR_ARCHIVE );
	s_muted = Cvar_Get("s_muted", "0", CVAR_ROM);
	s_doppler = Cvar_Get( "s_doppler", "1", CVAR_ARCHIVE );
	s_backend = Cvar_Get( "s_backend", "", CVAR_ROM );
	s_muteWhenMinimized = Cvar_Get( "s_muteWhenMinimized", "0", CVAR_ARCHIVE );
	s_muteWhenUnfocused = Cvar_Get( "s_muteWhenUnfocused", "0", CVAR_ARCHIVE );
	cv = Cvar_Get( "s_initsound", "1", 0 );

	if( !cv->integer ) {
		Com_Printf( "Sound disabled.\n" );
	} else {
		Com_Printf( "Sound initialization successful.\n" );
	}
	Com_Printf( "--------------------------------\n");
}

void S_Shutdown( void )
{
	Com_Memset( &si, 0, sizeof( soundInterface_t ) );

	Cmd_RemoveCommand( "play" );
	Cmd_RemoveCommand( "music");
	Cmd_RemoveCommand( "stopmusic");
	Cmd_RemoveCommand( "s_list" );
	Cmd_RemoveCommand( "s_stop" );
	Cmd_RemoveCommand( "s_info" );
}

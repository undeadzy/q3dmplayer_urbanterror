/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "client.h"
#include "snd_codec.h"

/*
=================
FGetLittleLong
=================
*/
static int FGetLittleLong( fileHandle_t f ) {
	int		v;

	FS_Read( &v, sizeof(v), f );

	return LittleLong( v);
}

/*
=================
FGetLittleShort
=================
*/
static int FGetLittleShort( fileHandle_t f ) {
	short	v;

	FS_Read( &v, sizeof(v), f );

	return LittleShort( v);
}

/*
=================
S_ReadChunkInfo
=================
*/
static int S_ReadChunkInfo(fileHandle_t f, char *name)
{
	int len, r;

	name[4] = 0;

	r = FS_Read(name, 4, f);
	if(r != 4)
		return 0;

	len = FGetLittleLong(f);
	if(len < 0 || len > 0xffffffff)
		return 0;

	//FIXME: 11/11/05 <tim@ngus.net>
	// I'm not sure I understand why this needs to be padded.
	// Surely this results in reading past the end of the data?
	//len = (len + 1 ) & ~1;		// pad to word boundary

	return len;
}

/*
=================
S_SkipChunk
=================
*/
static void S_SkipChunk(fileHandle_t f, int length)
{
	byte buffer[32*1024];

	while(length > 0)
	{
		int toread = length;
		if(toread > sizeof(buffer))
			toread = sizeof(buffer);
		FS_Read(buffer, toread, f);
		length -= toread;
	}
}

/*
=================
S_FindWavChunk

Returns the length of the data in the chunk, or 0 if not found
=================
*/
static int S_FindWavChunk( fileHandle_t f, char *chunk ) {
	char	name[5];
	int		len;

	// This is a bit dangerous...
	while(1)
	{
		len = S_ReadChunkInfo(f, name);

		// Read failure?
		if(len == 0)
			return 0;

		// If this is the right chunk, return
		if(!strcmp(name, chunk))
			return len;

		// Not the right chunk - skip it
		S_SkipChunk(f, len);
	}
}

/*
=================
S_ByteSwapRawSamples
=================
*/
static void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) {
	int		i;

	if ( width != 2 ) {
		return;
	}
	if ( LittleShort( 256 ) == 256 ) {
		return;
	}

	if ( s_channels == 2 ) {
		samples <<= 1;
	}
	for ( i = 0 ; i < samples ; i++ ) {
		((short *)data)[i] = LittleShort( ((short *)data)[i] );
	}
}

/*
=================
S_ReadWavHeader
=================
*/
static qboolean S_ReadWavHeader(fileHandle_t file, snd_info_t *info)
{
	char dump[16];
	int wav_format;
	int fmtlen = 0;

	// skip the riff wav header
	FS_Read(dump, 12, file);

	// Scan for the format chunk
	if((fmtlen = S_FindWavChunk(file, "fmt ")) == 0)
	{
		Com_Printf("No fmt chunk\n");
		return qfalse;
	}

	// Save the parameters
	wav_format = FGetLittleShort(file);
	info->channels = FGetLittleShort(file);
	info->rate = FGetLittleLong(file);
	FGetLittleLong(file);
	FGetLittleShort(file);
	info->width = FGetLittleShort(file) / 8;
	info->dataofs = 0;

	// Skip the rest of the format chunk if required
	if(fmtlen > 16)
	{
		fmtlen -= 16;
		S_SkipChunk(file, fmtlen);
	}

	// Scan for the data chunk
	if( (info->size = S_FindWavChunk(file, "data")) == 0)
	{
		Com_Printf("No data chunk\n");
		return qfalse;
	}
	info->samples = (info->size / info->width) / info->channels;

	return qtrue;
}

// WAV codec
snd_codec_t wav_codec =
{
	".wav",
	S_WAV_CodecLoad,
	S_WAV_CodecOpenStream,
	S_WAV_CodecReadStream,
	S_WAV_CodecCloseStream,
	NULL
};

/*
=================
S_WAV_CodecLoad
=================
*/
void *S_WAV_CodecLoad(const char *filename, snd_info_t *info)
{
	fileHandle_t file;
	void *buffer;

	// Try to open the file
	FS_FOpenFileRead(filename, &file, qtrue);
	if(!file)
	{
		Com_Printf("Can't read sound file %s\n", filename);
		return NULL;
	}

	// Read the RIFF header
	if(!S_ReadWavHeader(file, info))
	{
		FS_FCloseFile(file);
		Com_Printf("Can't understand wav file %s\n", filename);
		return NULL;
	}

	// Allocate some memory
	buffer = Z_Malloc(info->size);
	if(!buffer)
	{
		FS_FCloseFile(file);
		Com_Printf("Out of memory reading %s\n", filename);
		return NULL;
	}

	// Read, byteswap
	FS_Read(buffer, info->size, file);
	S_ByteSwapRawSamples(info->samples, info->width, info->channels, (byte *)buffer);

	// Close and return
	FS_FCloseFile(file);
	return buffer;
}

/*
=================
S_WAV_CodecOpenStream
=================
*/
snd_stream_t *S_WAV_CodecOpenStream(const char *filename)
{
	snd_stream_t *rv;

	// Open
	rv = S_CodecUtilOpen(filename, &wav_codec);
	if(!rv)
		return NULL;

	// Read the RIFF header
	if(!S_ReadWavHeader(rv->file, &rv->info))
	{
		S_CodecUtilClose(rv);
		return NULL;
	}

	return rv;
}

/*
=================
S_WAV_CodecCloseStream
=================
*/
void S_WAV_CodecCloseStream(snd_stream_t *stream)
{
	S_CodecUtilClose(stream);
}

/*
=================
S_WAV_CodecReadStream
=================
*/
int S_WAV_CodecReadStream(snd_stream_t *stream, int bytes, void *buffer)
{
	int remaining = stream->info.size - stream->pos;
	int samples;

	if(remaining <= 0)
		return 0;
	if(bytes > remaining)
		bytes = remaining;
	stream->pos += bytes;
	samples = (bytes / stream->info.width) / stream->info.channels;
	FS_Read(buffer, bytes, stream->file);
	S_ByteSwapRawSamples(samples, stream->info.width, stream->info.channels, buffer);
	return bytes;
}
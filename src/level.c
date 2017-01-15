// -*- Mode: C; indent-tabs-mode: t; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// Multi-Phasic Applications: Trouble In Furry Land
//     Copyright (C) Steven Gawroriski <steven@multiphasicapps.net>
//     Copyright (C) Multi-Phasic Applications <multiphasicapps.net>
// ---------------------------------------------------------------------------
// Trouble In Furry Land is under the GNU General Public License v3+, or later.
// See license.mkd for licensing and copyright information.
// ---------------------------------------------------------------------------

#include <SDL_stdinc.h>

#include "level.h"
#include "entity.h"

/** The maximum number of holes in the level. */
#define MAX_LEVEL_HOLES (LEVEL_WIDTH - 8)

LevelTile leveldata[LEVEL_WIDTH][LEVEL_HEIGHT];

int currentlevelnum;

const TileInfo tileinfo[NUM_TILETYPES] =
{
	// Air
	{
		0x8888DD
	},
	
	// Grass
	{
		0x22DD66
	}
};

void InternalRespawnPlayer(Entity* oldplayer)
{
	// Always use the last entity for the player
	playerentity = &entities[MAX_ENTITIES - 1];
	
	// Keep the old player Y height, assuming it is safe to do so
	if (oldplayer == NULL || oldplayer->y <= FIXED_TILE_SIZE)
		playerentity->y = FIXED_TILE_SIZE;
	else
		playerentity->y = oldplayer->y;
	
	// Positive level, starts on left side
	if (currentlevelnum >= 0)
	{
		playerentity->x = FIXEDONE;
		playerentity->angle = FACETYPE_RIGHT;
	}
	
	// Negative level, starts on right side
	else
	{
		playerentity->x = RIGHT_SIDE_TRANSITION - FIXED_TILE_SIZE;
		playerentity->angle = FACETYPE_LEFT;
	}
	
	// Set player type
	playerentity->type = ENTITYTYPE_PLAYER;
}

void InitializeLevel(int levelnum)
{
	int x, i, n, absln, q, v;
	Entity oldplayer;
	
	// Remember the old player information
	if (playerentity != NULL)
		SDL_memmove(&oldplayer, playerentity, sizeof(oldplayer));
	else
		SDL_memset(&oldplayer, 0, sizeof(oldplayer));
	playerentity = NULL;
	
	// Clear entities and the level
	SDL_memset(entities, 0, sizeof(entities));
	SDL_memset(leveldata, 0, sizeof(leveldata));
	
	// Set new level number
	currentlevelnum = levelnum;
	if (levelnum < 0)
		absln = -levelnum;
	else
		absln = levelnum;
	
	// Add base floor to the level
	for (x = 0; x < LEVEL_WIDTH; x++)
		leveldata[x][0].type = TILETYPE_GRASS;
	
	// Cut holes in the floor
	n = ((absln / 2) & 0xF);
	if (n > MAX_LEVEL_HOLES)
		n = MAX_LEVEL_HOLES;
	else if (n < 8)
		n = 8;
	q = (n * levelnum * 37) + absln * 31;
	if (q < 0)
		q = -q;
	while (n > 0)
	{
		// Remove count
		n--;
		
		// Determine where to place the hole
		x = 4 + (q % (LEVEL_WIDTH - 4));
		q *= ((x & 1) == 0) ? 37 : 31;
		if (q < 0)
			q = -q;
		
		// Clear it out
		leveldata[x][0].type = TILETYPE_AIR;
	}
	
	// Spawn furries
	
	// Drop the player off at the spawn area
	InternalRespawnPlayer(&oldplayer);
}

void RespawnPlayer()
{
	InternalRespawnPlayer(NULL);
}


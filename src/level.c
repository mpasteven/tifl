// -*- Mode: C; indent-tabs-mode: t; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// Multi-Phasic Applications: Trouble In Furry Land
//     Copyright (C) Stephanie Gawroriski <xer@multiphasicapps.net>
//     Copyright (C) Multi-Phasic Applications <multiphasicapps.net>
// ---------------------------------------------------------------------------
// Trouble In Furry Land is under the GNU General Public License v3+, or later.
// See license.mkd for licensing and copyright information.
// ---------------------------------------------------------------------------

#include <SDL_stdinc.h>

#include "level.h"
#include "entity.h"
#include "random.h"

/** The maximum number of holes in the level. */
#define MAX_LEVEL_HOLES ((LEVEL_WIDTH - 8) / 4)

/** Maximum ground height. */
#define MAX_GROUND_HEIGHT 7

/** The height where clouds are. */
#define CLOUD_HEIGHT (LEVEL_HEIGHT - 4)

LevelTile leveldata[LEVEL_WIDTH * LEVEL_HEIGHT];

int currentlevelnum = 0;

static int lastlevelnum;

const TileInfo tileinfo[NUM_TILETYPES] =
{
	// Air
	{
		0x8888DD,
		false
	},
	
	// Grass
	{
		0x22DD66,
		true
	},
	
	// Wood
	{
		0x66AA111,
		true
	},
	
	// Cloud
	{
		0xFEDEFA,
		false
	}
};

void InternalRespawnPlayer(Entity* oldplayer)
{
	int groundy;
	
	// Always use the last entity for the player
	playerentity = &entities[MAX_ENTITIES - 1];
	
	// Start in the center of level zero
	if (currentlevelnum == 0 && currentlevelnum == lastlevelnum)
	{
		playerentity->x = LEVEL_WIDTH_PIXELS >> 1;
		playerentity->angle = FACETYPE_RIGHT;
	}
	
	// Came from a lower level
	else if (currentlevelnum > lastlevelnum)
	{
		playerentity->x = 1;
		playerentity->angle = FACETYPE_RIGHT;
	}
	
	// Came from a higher level
	else
	{
		playerentity->x = RIGHT_SIDE_TRANSITION - TILE_SIZE;
		playerentity->angle = FACETYPE_LEFT;
	}
	
	// Keep the player height, but never place underground
	groundy = GroundHeight(playerentity->x);
	if (oldplayer == NULL || oldplayer->y <= groundy)
		playerentity->y = groundy;
	else
		playerentity->y = oldplayer->y;
	
	// Set player type
	playerentity->type = ENTITYTYPE_PLAYER;
}

/**
 * Places the tile at the given position.
 *
 * @param x The x position of the tile.
 * @param y The y position of the tile.
 * @param type The type of tile to place.
 * @return The tile that was touched.
 * @since 2017/01/15
 */
static LevelTile* PlaceTile(int x, int y, TileType type)
{
	LevelTile* mod;
	
	// Out of bounds?
	if (x < 0 || y < 0 || x >= LEVEL_WIDTH || y >= LEVEL_HEIGHT)
		return NULL;
	
	// In bounds
	mod = &leveldata[(y * LEVEL_WIDTH) + x];
	mod->type = type;
	
	return mod;
}

EntityType RandomType()
{
	int32_t rv = NextRandom();
	if (rv < 0)
		rv = -rv;
	
	// Use it
	return START_FURRIES + (rv % (NUM_ENTITYTYPES - START_FURRIES));
}

void InitializeLevel(int levelnum)
{
	int32_t x, y, i, n, absln, q, v, gz, ngls, groundy, tries, spawny;
	Entity oldplayer, *entity;
	EntityInfo* info;
	boolean docloud, doground, doplatform, startblock;
	
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
	lastlevelnum = currentlevelnum;
	currentlevelnum = levelnum;
	if (levelnum < 0)
		absln = -levelnum;
	else
		absln = levelnum;
	
	// Seed the RNG
	SeedRandom(levelnum);
	
	// Go through the level and add things to it based on the seed
	ngls = 4;
	gz = 1;
	for (x = 0; x < LEVEL_WIDTH; x++)
	{
		// Is there clouds and/or ground here?
		startblock = (x == 0 || x >= (LEVEL_WIDTH - 2)) ||
			(levelnum == 0 && x == (LEVEL_WIDTH >> 1));
		docloud = ((NextRandom() & 1) != 0);
		doground = startblock || ((NextRandom() & 0xFF) < 192);
		doplatform = !startblock && ((NextRandom() & 0xFF) < 64);
		
		// Draw clouds at the higher heights
		if (docloud)
			PlaceTile(x, CLOUD_HEIGHT + y + (NextRandom() & 1),
				TILETYPE_CLOUD);
		
		// Drawing the ground?
		if (doground)
		{
			// Place ground
			for (y = 0; y < (startblock ? 1 : gz); y++)
				PlaceTile(x, y, TILETYPE_GRASS);
			
			// Change the ground Z height?
			if ((--ngls) == 0)
			{
				// Change breadth
				ngls = 1 + (NextRandom() & 3);
				
				// Increase or decrease height
				if (NextRandom() & 1)
					gz++;
				else
					gz--;
				
				// Cap
				if (gz < 1)
					gz = 1;
				else if (gz >= MAX_GROUND_HEIGHT)
					gz = MAX_GROUND_HEIGHT;
			}
		}
		
		// Otherwise draw some platforms maybe
		else if (doplatform)
		{
			// Place above the current ground
			PlaceTile(x, gz + 1 + (NextRandom() & 1), TILETYPE_WOOD);
		}
	}
	
	// Drop the player off at the spawn area
	InternalRespawnPlayer(&oldplayer);
	
	// Spawn furries
	n = 4 + (NextRandom() % 4) + (absln / 4);
	if (n > MAX_ENTITIES / 2)
		n = MAX_ENTITIES / 2;
	for (int i = 0; i < n; i++)
	{
		// Get next entity
		entity = BlankEntity();
		if (entity == NULL)
			break;
		
		// Clear it
		SDL_memset(entity, 0, sizeof(*entity));
		
		// Determine random type
		entity->type = RandomType();
		info = &entityinfo[entity->type];
		
		// Try placing it somewhere
		for (tries = 0; tries < 4; tries++)
		{
			// Use random X
			x = (NextRandom() % LEVEL_WIDTH - 2) * TILE_SIZE;
			if (x < 0)
				x = -x;
			x += 1;
			
			// If it cannot be placed on the ground, do not spawn it there
			groundy = GroundHeight(x);
			if (info->feelsgravity && groundy <= 0)
				continue;
			
			// Spawn on the ground
			if (info->feelsgravity)
				spawny = groundy;
			
			// Otherwise in the air somewhere
			else
				spawny = groundy + (((NextRandom() % 2) + 4) * TILE_SIZE);
			
			// Set position
			entity->x = x;
			entity->y = spawny;
			entity->angle = (NextRandom() & 1);
		}
	}
}

void RespawnPlayer()
{
	InternalRespawnPlayer(NULL);
}

int32_t GroundHeight(int32_t x)
{
	int y;
	
	// No ground possible.
	x /= TILE_SIZE;
	if (x < 0 || x >= LEVEL_WIDTH)
		return -1;
	
	// Find ground
	for (y = LEVEL_HEIGHT - 1; y >= 0; y--)
		if (tileinfo[leveldata[(y * LEVEL_WIDTH) + x].type].issolid)
			return (y + 1) * TILE_SIZE;
	
	// No ground
	return -1;
}


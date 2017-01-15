// -*- Mode: C; indent-tabs-mode: t; tab-width: 4 -*-
// ---------------------------------------------------------------------------
// Multi-Phasic Applications: Trouble In Furry Land
//     Copyright (C) Steven Gawroriski <steven@multiphasicapps.net>
//     Copyright (C) Multi-Phasic Applications <multiphasicapps.net>
// ---------------------------------------------------------------------------
// Trouble In Furry Land is under the GNU General Public License v3+, or later.
// See license.mkd for licensing and copyright information.
// ---------------------------------------------------------------------------

#ifndef TIFL_ENTITY_H
#define TIFL_ENTITY_H

#include "global.h"
#include "level.h"

/**
 * The type of entity this is.
 *
 * @since 2017/01/12
 */
typedef enum EntityType
{
	/** Nothing. */
	ENTITYTYPE_NOTHING,
	
	/** The player. */
	ENTITYTYPE_PLAYER,
	
	/** The number of entity types. */
	NUM_ENTITYTYPES
} EntityType;

/**
 * The direction the entity is facing.
 *
 * @since 2017/01/14
 */
typedef enum facetype
{
	/** Facing left. */
	FACETYPE_LEFT,
	
	/** Facing right. */
	FACETYPE_RIGHT,
	
	/** The number of face types. */
	NUM_FACETYPES
} facetype;

/**
 * Entity information.
 *
 * @since 2017/01/15
 */
typedef struct EntityInfo
{
	/** XPM images. */
	char** xpm[2];

	/** Raw pixel data. */
	uint32_t pixels[2][TILE_SIZE * TILE_SIZE];
} EntityInfo;

/**
 * Entity data.
 *
 * @since 2017/01/12
 */
typedef struct Entity
{
	/** The type of entity this is. */
	EntityType type;
	
	/** Postition of the entity. */
	int32_t x, y;
	
	/** The angle the entity is facing. */
	facetype angle;
} Entity;

/** Maximum number of entities. */
#define MAX_ENTITIES 64

/** Global entity data. */
extern Entity entities[MAX_ENTITIES];

/** Entity information. */
extern EntityInfo entityinfo[NUM_ENTITYTYPES];

/** The player entity. */
extern Entity* playerentity;

/**
 * Loads sprites.
 *
 * @since 2017/02/15
 */
void LoadSprites();

/**
 * Walks the given entity in the given direction.
 *
 * The movement is based on the angle the entity is facing.
 *
 * @param entity The entity to walk.
 * @param relx The x movement.
 * @param rely The y movement.
 * @param impulse Is this movement generated by an impulse?
 */
void WalkEntity(Entity* entity, int32_t relx, int32_t rely,
	boolean impulse);

#endif


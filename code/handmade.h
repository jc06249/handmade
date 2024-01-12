#if !defined(HANDMADE_H)

/*
    TODO:

    ARCHITECTURE EXPLORATION
    - Z!
        - Need to make a solid concept of ground levels so the camera can
          be freely placed in Z and have multiple ground levels in one
          sim region
        - 3D collision detection working properly!
        - How is this rendered?
        "Frinstances"!
    - Collision detection?
        - Clean up predicate proliferation! Can we make a nice clean
          set of flags/rules so that it's easy to understand how
          things work in terms of special handling?  This may involve
          making the iteration handle everything instead of handling
          overlap outside and so on.
        - Transient collision rules!  Clear based on flag.
            - Allow non-transient rules to override transient ones.
        - Entry/exit?
        - What's the plan for robustness / shape definition?
        - (Implement reprojection to handle interpenetration)
        - "Things pushing other things"
    - Implement multiple sim regions per frame
        - Per-entity clocking
        - Sim region merging? For multiple players?
        - Simple zoomed-out view for testing?

    - Debug code
        - Logging
        - Diagramming
        - (A LITTLE GUI, but only a little!) Switches / sliders / etc.

    - Audio
        - Sound effect triggers
        - Ambient sounds
        - Music
    - Asset streaming

    - Metagame / save game?
        - How do you enter "save slot"?
        - Persistent unlocks/etc.
        - Do we allow saved games?  Probably yes, just only for "pausing",
        * Continuous save for crash recovery?
    - Rudimentary would gen (no quality, just "what sorts of things" we do)
        - Placement of background things
        - Connectivity?
        - Non-overlapping?
        - Map display
            - Magnets - how they work???

    - AI
        - Rudimentary monstar behvior example
        * Pathfinding
        - AI "storage"

    * Animation, should probably lead into rendering
        - Skeletel animation
        - Particle systems

    PRODUCTION
    - Rendering
    -> GAME
        - Entity systems
        - World generation
*/

#include "handmade_platform.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

//
//
//

struct memory_arena
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;
};

inline void InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8 *)Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count) * sizeof(type))
inline void * PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return(Result);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void ZeroSize(memory_index Size, void *Ptr)
{
    // TODO: Check this guy for performance
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    uint32 *Pixels;
};

struct hero_bitmaps
{
    v2 Align;
    loaded_bitmap Head;
    loaded_bitmap Cape;
    loaded_bitmap Torso;
};

struct low_entity
{
    // TODO: It's kind of busted that P's can be invalid here,
    // AND we store whether they would be invalid in the flags field...
    // Can we do something better here?
    world_position P;
    sim_entity Sim;
};

struct entity_visible_piece
{
    loaded_bitmap *Bitmap;
    v2 Offset;
    real32 OffsetZ;
    real32 EntityZC;

    real32 R, G, B, A;
    v2 Dim;
};

struct controlled_hero
{
    uint32 EntityIndex;

    // NOTE: These are the controller requests for simulation
    v2 ddP;
    v2 dSword;
    real32 dZ;
};

struct pairwise_collision_rule
{
    bool32 CanCollide;
    uint32 StorageIndexA;
    uint32 StorageIndexB;
    pairwise_collision_rule *NextInHash;
};
struct game_state;
internal void AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 CanCollide);
internal void ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex);

struct game_state
{
    memory_arena WorldArena;
    world *World;

    // TODO: Should we allow split-screen?
    uint32 CameraFollowingEntityIndex;
    world_position CameraP;

    controlled_hero ControlledHeros[ArrayCount(((game_input *)0)->Controllers)];

    uint32 LowEntityCount;
    low_entity LowEntities[100000];

    loaded_bitmap Backdrop;
    loaded_bitmap Shadow;
    hero_bitmaps HeroBitmaps[4];

    loaded_bitmap Tree;
    loaded_bitmap Sword;
    loaded_bitmap Stairwell;

    real32 MetersToPixels;

    // TODO: Must be power of two
    pairwise_collision_rule *CollisionRuleHash[256];
    pairwise_collision_rule *FirstFreeCollisionRule;
};

// TODO: This is dumb, this should just be part of
// the renderer pushbuffer - add correctrion of coordinates
// in there and be done with it.
struct entity_visible_piece_group
{
    game_state *GameState;
    uint32 PieceCount;
    entity_visible_piece Pieces[32];
};

inline low_entity * GetLowEntity(game_state *GameState, uint32 Index)
{
    low_entity *Result = 0;

    if((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }

    return(Result);
}

#define HANDMADE_H
#endif
#if !defined(HANDMADE_AUDIO_H)

struct playing_sound
{
    v2 CurrentVolume;
    v2 dCurrentVolume;
    v2 TargetVolume;

    sound_id ID;
    int32 SamplesPlayed;
    playing_sound *Next;
};

struct audio_state
{
    memory_arena *PermArena;
    playing_sound *FirstPlayingSound;
    playing_sound *FirstFreePlayingSound;

    v2 MasterVolume;
};

#define HANDMADE_AUDIO_H
#endif
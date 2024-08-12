
internal void OutputTestSineWave(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;

    for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
#if 1
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 1
        GameState->tSine += Tau32 * 1.0f / (real32)WavePeriod;
        if(GameState->tSine > Tau32)
        {
            GameState->tSine -= Tau32;
        }
#endif
    }
}

internal playing_sound *PlaySound(audio_state *AudioState, sound_id SoundID)
{
    if(!AudioState->FirstFreePlayingSound)
    {
        AudioState->FirstFreePlayingSound = PushStruct(AudioState->PermArena, playing_sound);
        AudioState->FirstFreePlayingSound->Next = 0;
    }

    playing_sound *PlayingSound = AudioState->FirstFreePlayingSound;
    AudioState->FirstFreePlayingSound = PlayingSound->Next;

    PlayingSound->SamplesPlayed = 0;
    // TODO: Should these default to 0.5f/0.5f for centered?
    PlayingSound->CurrentVolume = PlayingSound->TargetVolume = V2(1.0f, 1.0f);
    PlayingSound->dCurrentVolume = V2(0.0f, 0.0f);
    PlayingSound->ID = SoundID;
    PlayingSound->dSample = 1.0f;

    PlayingSound->Next = AudioState->FirstPlayingSound;
    AudioState->FirstPlayingSound = PlayingSound;

    return(PlayingSound);
}

internal void ChangeVolume(audio_state *AudioState, playing_sound *Sound, real32 FadeDurationInSeconds, v2 Volume)
{
    if(FadeDurationInSeconds <= 0.0f)
    {
        Sound->CurrentVolume = Sound->TargetVolume = Volume;
    }
    else
    {
        real32 OneOverFade = 1.0f / FadeDurationInSeconds;
        Sound->TargetVolume = Volume;
        Sound->dCurrentVolume = OneOverFade * (Sound->TargetVolume - Sound->CurrentVolume);
    }
}

internal void ChangePitch(audio_state *AudioState, playing_sound *Sound, real32 dSample)
{
    Sound->dSample = dSample;
}
internal void OutputPlayingSounds(audio_state *AudioState, game_sound_output_buffer *SoundBuffer, game_assets *Assets, memory_arena *TempArena)
{
    temporary_memory MixerMemory = BeginTemporaryMemory(TempArena);

    real32 *RealChannel0 = PushArray(TempArena, SoundBuffer->SampleCount, real32);
    real32 *RealChannel1 = PushArray(TempArena, SoundBuffer->SampleCount, real32);

    real32 SecondsPerSample = 1.0f / (real32)SoundBuffer->SamplesPerSecond;
#define AudioStateOutputChannelCount 2

    // NOTE: Clear out the mixer channels
    {
        real32 *Dest0 = RealChannel0;
        real32 *Dest1 = RealChannel1;
        for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
        {
            *Dest0++ = 0.0f;
            *Dest1++ = 0.0f;
        }
    }

    // NOTE: Sum all sounds
    for(playing_sound **PlayingSoundPtr = &AudioState->FirstPlayingSound; *PlayingSoundPtr;)
    {
        playing_sound *PlayingSound = *PlayingSoundPtr;
        bool32 SoundFinished = false;

        uint32 TotalSamplesToMix = SoundBuffer->SampleCount;
        real32 *Dest0 = RealChannel0;
        real32 *Dest1 = RealChannel1;

        while(TotalSamplesToMix && !SoundFinished)
        {
            loaded_sound *LoadedSound = GetSound(Assets, PlayingSound->ID);
            if(LoadedSound)
            {
                asset_sound_info *Info = GetSoundInfo(Assets, PlayingSound->ID);
                PrefetchSound(Assets, Info->NextIDToPlay);

                v2 Volume = PlayingSound->CurrentVolume;
                v2 dVolume = SecondsPerSample * PlayingSound->dCurrentVolume;
                real32 dSample = PlayingSound->dSample;

                Assert(PlayingSound->SamplesPlayed >= 0);

                u32 SamplesToMix = TotalSamplesToMix;
                r32 RealSampleRemainingInSound = (LoadedSound->SampleCount - RoundReal32ToInt32(PlayingSound->SamplesPlayed)) / dSample;
                uint32 SamplesRemainingInSound = RoundReal32ToInt32(RealSampleRemainingInSound);
                if(SamplesToMix > SamplesRemainingInSound)
                {
                    SamplesToMix = SamplesRemainingInSound;
                }

                b32 VolumeEnded[AudioStateOutputChannelCount] = {};
                for(u32 ChannelIndex = 0; ChannelIndex < ArrayCount(VolumeEnded); ++ChannelIndex)
                {
                    if(dVolume.E[ChannelIndex] != 0.0f)
                    {
                        real32 DeltaVolume = (PlayingSound->TargetVolume.E[ChannelIndex] - Volume.E[ChannelIndex]);
                        u32 VolumeSampleCount = (u32)((DeltaVolume / dVolume.E[ChannelIndex]) + 0.5f);
                        if(SamplesToMix > VolumeSampleCount)
                        {
                            SamplesToMix = VolumeSampleCount;
                            VolumeEnded[ChannelIndex] = true;
                        }
                    }
                }

                // TODO: Handle stereo!
                real32 SamplePosition = PlayingSound->SamplesPlayed;
                for(u32 LoopIndex = 0; LoopIndex < SamplesToMix; ++LoopIndex)
                {
#if 0
                    u32 SampleIndex = FloorReal32ToInt32(SamplePosition);
                    r32 Frac = SamplePosition - (r32)SampleIndex;
                    r32 Sample0 = (r32)LoadedSound->Samples[0][SampleIndex];
                    r32 Sample1 = (r32)LoadedSound->Samples[0][SampleIndex + 1];
                    r32 SampleValue = Lerp(Sample0, Frac, Sample1);
#else

                    u32 SampleIndex = RoundReal32ToInt32(SamplePosition);
                    real32 SampleValue = LoadedSound->Samples[0][SampleIndex];
#endif
                    *Dest0++ += AudioState->MasterVolume.E[0] * Volume.E[0] * SampleValue;
                    *Dest1++ += AudioState->MasterVolume.E[1] * Volume.E[1] * SampleValue;

                    Volume += dVolume;
                    SamplePosition += dSample;
                }

                PlayingSound->CurrentVolume = Volume;

                // TODO: This is not correct yet!  Need to truncate the loop!
                for(u32 ChannelIndex = 0; ChannelIndex < ArrayCount(VolumeEnded); ++ChannelIndex)
                {
                    if(VolumeEnded[ChannelIndex])
                    {
                        PlayingSound->CurrentVolume.E[ChannelIndex] = PlayingSound->TargetVolume.E[ChannelIndex];
                        PlayingSound->dCurrentVolume.E[ChannelIndex] = 0.0f;
                    }
                }

                Assert(TotalSamplesToMix >= SamplesToMix);
                PlayingSound->SamplesPlayed = SamplePosition;
                TotalSamplesToMix -= SamplesToMix;

                if((uint32)PlayingSound->SamplesPlayed == LoadedSound->SampleCount)
                {
                    if(IsValid(Info->NextIDToPlay))
                    {
                        PlayingSound->ID = Info->NextIDToPlay;
                        PlayingSound->SamplesPlayed = 0;
                    }
                    else
                    {
                        SoundFinished = true;
                    }
                }
            }
            else
            {
                LoadSound(Assets, PlayingSound->ID);
                break;
            }
        }

        if(SoundFinished)
        {
            *PlayingSoundPtr = PlayingSound->Next;
            PlayingSound->Next = AudioState->FirstFreePlayingSound;
            AudioState->FirstFreePlayingSound = PlayingSound;
        }
        else
        {
            PlayingSoundPtr = &PlayingSound->Next;
        }
    }

    // NOTE: Convert to 16-bit
    {
        real32 *Source0 = RealChannel0;
        real32 *Source1 = RealChannel1;

        int16 *SampleOut = SoundBuffer->Samples;
        for(int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
        {
            //TODO: Once this in in SIMD, clamp!

            *SampleOut++ = (int16)(*Source0++ + 0.5f);
            *SampleOut++ = (int16)(*Source1++ + 0.5f);
        }
    }

    EndTemporaryMemory(MixerMemory);
}

internal void InitializeAudioState(audio_state *AudioState, memory_arena *PermArena)
{
    AudioState->PermArena = PermArena;
    AudioState->FirstPlayingSound = 0;
    AudioState->FirstFreePlayingSound = 0;

    AudioState->MasterVolume = V2(1.0f, 1.0f);
}
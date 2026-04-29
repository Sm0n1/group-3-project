#include <SDL3_mixer/SDL_mixer.h>
#include <cassert>
#include "audio.hpp"
#include "SDL3/SDL_log.h"
#include "physics.hpp"
#include "camera.hpp"

namespace clayborne {
    // Load audio
    audio_loader::result_type audio_loader::operator()(
        const std::filesystem::path &path,
        MIX_Mixer *mixer
    ) noexcept {
        auto audio{ MIX_LoadAudio(mixer, path.c_str(), false) };
        if (!audio) {
            SDL_Log("Could not load audio %s: %s", path.c_str(), SDL_GetError());
            return nullptr;
        }

        return std::shared_ptr<MIX_Audio>{
            audio,
            [](MIX_Audio *a) { MIX_DestroyAudio(a); }
        };
    }

    bool load_debug_sounds(
        audio_cache &sounds,
        MIX_Mixer *mixer
    ) {
        if (!sounds.load(entt::hashed_string{ "data/jump.wav" }, "data/jump.wav", mixer).first->second) {
            return false;
        }

        if (!sounds.load(entt::hashed_string{ "data/explosion.wav" }, "data/explosion.wav", mixer).first->second) {
            return false;
        }

        return true;
    }

    entt::entity play_sound(
        entt::registry &registry,
        audio_cache &sounds,
        MIX_Mixer *mixer,
        const entt::id_type sound,
        const float gain,
        const bool is_looping
    ) noexcept {
        assert(sounds.contains(sound));
        assert(mixer);

        auto track{ MIX_CreateTrack(mixer) };
        if (!track) {
            SDL_Log("MIX create track failed: %s", SDL_GetError());
            return entt::null;
        }

        if (!MIX_SetTrackAudio(track, sounds[sound].handle().get())) {
            SDL_Log("MIX set track audio failed: %s", SDL_GetError());
            MIX_DestroyTrack(track);
            return entt::null;
        }

        if (!MIX_SetTrackGain(track, gain)) {
            SDL_Log("MIX set track gain failed: %s", SDL_GetError());
            MIX_DestroyTrack(track);
            return entt::null;
        }

        if (!MIX_SetTrackLoops(track, is_looping ? -1 : 1)) {
            SDL_Log("MIX set track loops failed: %s", SDL_GetError());
            MIX_DestroyTrack(track);
            return entt::null;
        }

        MIX_PlayTrack(track, 0);

        if (!MIX_PlayTrack(track, 0)) {
            SDL_Log("MIX play track failed: %s", SDL_GetError());
            MIX_DestroyTrack(track);
            return entt::null;
        }

        auto entity{ registry.create() };

        registry.emplace<struct sound>(entity, track);

        return entity;
    }

    entt::entity play_sound_at(
        entt::registry &registry,
        audio_cache &sounds,
        MIX_Mixer *mixer,
        const entt::id_type sound,
        const float gain,
        const bool is_looping,
        const float x,
        const float y
    ) noexcept {
        const auto entity{ play_sound(registry, sounds, mixer, sound, gain, is_looping) };
        
        if (entity != entt::null) {
            registry.emplace<struct position>(entity, x, y);
        }

        return entity;
    }

    void update_audio(
        entt::registry &registry,
        const entt::entity camera
    ) noexcept {
        assert(registry.all_of<struct position>(camera));

        const auto &camera_position{ registry.get<struct position>(camera) };

        auto sound_view{ registry.view<struct sound>() };
        for (auto entity : sound_view) {
            auto &sound{ sound_view.get<struct sound>(entity) };
            if (MIX_GetTrackLoops(sound.track) == 0) {
                if (MIX_GetTrackRemaining(sound.track) == 0) {
                    MIX_DestroyTrack(sound.track);
                    // TODO: implement a `temporary` component for entities that can be destroyed.
                    registry.destroy(entity);
                    continue;
                }
            }
        }

        auto moving_sound_view{ registry.view<struct position, struct sound>() };
        for (auto entity : moving_sound_view) {
            auto &position{ moving_sound_view.get<struct position>(entity) };
            auto &sound{ moving_sound_view.get<struct sound>(entity) };

            const MIX_Point3D microphone_relative_position{
                .x = mic_x(camera_position.x) + position.x,
                .y = mic_y(camera_position.y) + position.y,
                .z = mic_z(),
            };

            MIX_SetTrack3DPosition(sound.track, &microphone_relative_position);
        }
    }
}
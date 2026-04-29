#ifndef CLAYBORNE_AUDIO_HPP
#define CLAYBORNE_AUDIO_HPP

#include <filesystem>
#include <SDL3_mixer/SDL_mixer.h>
#include <entt/entt.hpp>

namespace clayborne {
    using audio_resource = MIX_Audio;

    struct audio_loader {
        using result_type = std::shared_ptr<audio_resource>;
        result_type operator()(const std::filesystem::path &path, MIX_Mixer *mixer) noexcept;
    };

    using audio_cache = entt::resource_cache<audio_resource, audio_loader>;

    // TODO: cache unused tracks instead of creating a new track for every sound.

    struct sound {
        // TODO: use handle instead
        MIX_Track *track{ nullptr };
    };

    bool load_debug_sounds(
        audio_cache &sounds,
        MIX_Mixer *mixer
    );

    entt::entity play_sound(
        entt::registry &registry,
        audio_cache &sounds,
        MIX_Mixer *mixer,
        const entt::id_type audio_resource,
        const float gain,
        const bool is_looping
    ) noexcept;

    entt::entity play_sound_at(
        entt::registry &registry,
        audio_cache &sounds,
        MIX_Mixer *mixer,
        const entt::id_type audio_resource,
        const float gain,
        const bool is_looping,
        const float x,
        const float y
    ) noexcept;

    void update_audio(
        entt::registry &registry,
        const entt::entity camera
    ) noexcept;
}

#endif // CLAYBORNE_AUDIO_HPP
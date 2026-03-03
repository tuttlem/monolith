# Execution Personality Hooks

This layer adds neutral runtime-personality registration hooks above hardware substrate.

## Purpose

- Allow downstream OS developers to plug in different execution models.
- Keep substrate policy-neutral (no forced POSIX/DOS/Plan9 semantics).
- Provide stable activation and execution entry points.

## Public API

Header: `kernel/include/personality.h`

- `status_t personality_register(const personality_ops_t *ops)`
- `status_t personality_activate(personality_id_t id)`
- `status_t personality_exec(const exec_image_t *img, exec_result_t *out)`
- `personality_id_t personality_active_id(void)`
- `const char *personality_active_name(void)`

## Notes

- Personalities are optional and can be compiled out by downstream OS designs.
- Substrate owns mechanism only; semantics remain downstream-defined.

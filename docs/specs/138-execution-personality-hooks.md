# 138 Execution Personality Hooks

## Goal

Add neutral execution hooks so downstream OS developers can implement different application models (DOS-like, POSIX-like, Plan9-like, custom) without changing substrate hardware layers.

## In Scope

- Personality registration API (`personality_ops_t`).
- Program image handoff contract (`exec_image_t`) without file format lock-in.
- Trap/exception translation hooks per personality.
- Syscall namespace handoff hooks (transport-level only).

## Out of Scope

- Implementing DOS/POSIX/Plan9 semantics in substrate.
- Defining mandatory process model.

## Public Interfaces

- Header: `kernel/include/personality.h`
- APIs:
  - `status_t personality_register(const personality_ops_t *ops)`
  - `status_t personality_activate(personality_id_t id)`
  - `status_t personality_exec(const exec_image_t *img, exec_result_t *out)`

## Design Rules

- Substrate owns mechanism only.
- Personalities own semantics and ABI policy.
- No reserved claim over downstream syscall ranges beyond substrate base range.

## Tests

- dummy personality plugin integration test.
- trap-to-policy callback path test.

## Acceptance Criteria

1. Multiple personalities can be compiled without architecture changes.
2. Disabling all personalities leaves substrate boot unaffected.

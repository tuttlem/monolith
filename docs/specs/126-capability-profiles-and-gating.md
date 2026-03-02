# 126 Capability Profiles and Feature Gating

## Goal

Define how the base exposes standard hardware capability domains while allowing each OS to opt in/out cleanly.

## Problem

A reusable base should include common device infrastructure, but not force every OS to ship every subsystem (for example audio).

## Scope

- Define capability profile concept:
  - `minimal` (boot + cpu/mmu/irq/timer + discovery + reporting)
  - `headless-server` (`minimal` + storage + network)
  - `desktop-base` (`headless-server` + input + display + audio)
- Define compile-time feature gates for each domain.
- Define runtime reporting of enabled/disabled capability domains.
- Require all optional domains to degrade to `STATUS_DEFERRED`/"disabled" cleanly when not enabled.

## Non-Goals

- User-space package management.
- Distribution policy.

## Acceptance Criteria

- Kernel builds and boots with each profile.
- Disabled capability domains do not break initialization order.
- Documentation clearly states what each profile includes.

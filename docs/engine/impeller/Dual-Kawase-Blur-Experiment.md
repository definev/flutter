# Dual Kawase Backdrop Blur — Engine Experiment

**Branch:** `experiment/dual-kawase-backdrop-blur`  
**Repo:** `/Users/vsf/source/github.com/definev/flutter` (fork of flutter/flutter monorepo)  
**Goal:** Run Dual Kawase as a real Impeller `ImageFilter` / `BackdropFilter` path (engine-owned backdrop snapshot), gated behind an experiment flag.

## Why this lives in the engine

`BackdropFilter` snapshots content already painted behind the layer inside Impeller, then applies a `DlImageFilter`. Package-level Flutter GPU (gpublur) never sees that snapshot. Multi-pass Dual Kawase therefore has to be an Impeller `FilterContents`, same family as `GaussianBlurFilterContents`.

## Current Impeller blur (baseline)

| Piece | Path |
|-------|------|
| Dart → DL | `lib/ui/painting/image_filter.cc` → `DlBlurImageFilter` |
| DL → Impeller | `impeller/display_list/image_filter.cc` → `FilterContents::MakeGaussianBlur` |
| Implementation | `impeller/entity/contents/filters/gaussian_blur_filter_contents.*` |
| Strategy | 1× downsample + separable X/Y Gaussian (lerp-hack kernels) |

## Proposed Dual Kawase path

```
ImageFilter.blur(sigma)
  → DlBlurImageFilter
  → [flag] FilterContents::MakeDualKawaseBlur   ← experiment hook
       → DualKawaseBlurFilterContents::RenderFilter
            for i in 0..levels-1:  downsample 5-tap (½ res)
            for i in levels-1..0:  upsample 8-tap tent
```

Sigma → `(levels, offset)` uses the same WebRender / gpublur fit:

- `levels ≈ clamp(round((4/3)·ln(σ)), 1, maxLevels)`
- `offset ≈ 0.4538^levels · σ`

Reference package: `github.com/definev/gpublur`.

## Files in this experiment

| File | Role |
|------|------|
| `…/filters/dual_kawase_blur_filter_contents.h/.cc` | FilterContents implementation |
| `…/shaders/filters/dual_kawase_downsample.frag` | 5-tap Dual Kawase down |
| `…/shaders/filters/dual_kawase_upsample.frag` | 8-tap tent Dual Kawase up |
| `docs/engine/impeller/Dual-Kawase-Blur-Experiment.md` | This doc |
| `engine/scripts/setup_dual_kawase_experiment.sh` | depot_tools / gclient bootstrap helper |

## Implementation checklist

1. **Host setup** (blocked until disk + depot_tools)
   - Free **≥80 GB** (you had ~22 GB free — not enough for full `gclient sync` + build)
   - Install [depot_tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html), put it first on `PATH`
   - From flutter root: `cp engine/scripts/standard.gclient .gclient` then edit `url` to your fork SSH URL
   - `gclient sync --no-history`

2. **Shaders + pipelines**
   - Register frags in `impeller/entity/BUILD.gn` (`entity_shaders`)
   - Add `DualKawaseDownsamplePipeline` / `DualKawaseUpsamplePipeline` in `pipelines.h`
   - Create variants + getters in `content_context.h/.cc`

3. **FilterContents**
   - Finish `RenderFilter` multi-pass with `MakeSubpass` (mirror Gaussian downsample/blur subpasses)
   - Coverage / source coverage: treat effective radius ≈ `offset * (2^levels - 1)` for halo

4. **Wire selection**
   - In `impeller/display_list/image_filter.cc` `kBlur` case, if env `IMPELLER_DUAL_KAWASE_BLUR=1` (or GN flag), call `MakeDualKawaseBlur` instead of `MakeGaussianBlur`
   - No Dart API change required for the A/B experiment — stock `BackdropFilter(filter: ImageFilter.blur(...))` exercises it

5. **Validate**
   - Playground / `aiks_dl_blur_unittests` golden comparisons vs Gaussian
   - Perf: `backdrop_filter_perf_*` timeline vs baseline Gaussian
   - Visual: animate σ across level boundaries (known Dual Kawase discontinuity)

6. **Upstream**
   - Open design discussion with Impeller owners before landing (perf/quality tradeoffs; WebRender previously abandoned Dual Kawase for their needs)
   - Prefer experiment flag → opt-in → maybe large-σ only fallback

## Non-goals (v0)

- Progressive / spatially varying blur (gpublur progressive upsample)
- Anisotropic σ (use `max(σx, σy)`)
- Replacing Gaussian for small σ (Gaussian stays better for tiny kernels)

## Local layout note

- **Use this monorepo** (`definev/flutter` → `engine/src/flutter/impeller/…`)
- `definev/engine` standalone clone is a **2023 shallow tip** — do not develop there
- Framework-only fork is not enough; Impeller C++ + shaders are required

## Quick status

| Item | Status |
|------|--------|
| Flutter fork cloned | ✅ `/Users/vsf/source/github.com/definev/flutter` |
| Experiment branch | ✅ `experiment/dual-kawase-backdrop-blur` |
| Filter + shader stubs | ✅ under Impeller tree |
| `gclient sync` / local engine build | ❌ needs depot_tools + disk |
| Plugged into `BackdropFilter` | ❌ after pipelines + flag hook |

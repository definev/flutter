// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/filters/dual_kawase_blur_filter_contents.h"

#include <algorithm>
#include <cmath>

#include "flutter/fml/logging.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/geometry/constants.h"

namespace impeller {

namespace {

Scalar MaxSigma(Scalar sigma_x, Scalar sigma_y) {
  return std::max(sigma_x, sigma_y);
}

}  // namespace

DualKawaseBlurFilterContents::DualKawaseBlurFilterContents(
    Scalar sigma_x,
    Scalar sigma_y,
    Entity::TileMode tile_mode,
    std::optional<Rect> bounds,
    BlurStyle mask_blur_style,
    const Geometry* mask_geometry)
    : sigma_(Vector2(sigma_x, sigma_y)),
      tile_mode_(tile_mode),
      bounds_(bounds),
      mask_blur_style_(mask_blur_style),
      mask_geometry_(mask_geometry) {
  ApproximateGaussian(MaxSigma(sigma_x, sigma_y), kMaxLevels, levels_, offset_);
}

void DualKawaseBlurFilterContents::ApproximateGaussian(Scalar sigma,
                                                       int max_levels,
                                                       int& out_levels,
                                                       Scalar& out_offset) {
  max_levels = std::clamp(max_levels, 1, kMaxLevels);
  if (sigma <= 0.0f) {
    out_levels = 1;
    out_offset = 0.5f;
    return;
  }
  if (sigma < 1.0f) {
    out_levels = 1;
    out_offset = std::clamp(0.4538f * sigma, 0.15f, 1.0f);
    return;
  }
  out_levels = std::clamp(
      static_cast<int>(std::lround((4.0 / 3.0) * std::log(sigma))), 1,
      max_levels);
  out_offset = std::clamp(
      static_cast<Scalar>(std::pow(0.4538, out_levels) * sigma), 0.25f, 8.0f);
}

Scalar DualKawaseBlurFilterContents::EffectiveRadius() const {
  // Rough halo: each pyramid level roughly doubles reach.
  return offset_ * (std::pow(2.0f, static_cast<Scalar>(levels_)) - 1.0f);
}

std::optional<Rect> DualKawaseBlurFilterContents::GetFilterSourceCoverage(
    const Matrix& effect_transform,
    const Rect& output_limit) const {
  Scalar radius = EffectiveRadius();
  Vector3 blur_radii =
      (effect_transform.Basis() * Vector3{radius, radius, 0.0}).Abs();
  return output_limit.Expand(Point(blur_radii.x, blur_radii.y));
}

std::optional<Rect> DualKawaseBlurFilterContents::GetFilterCoverage(
    const FilterInput::Vector& inputs,
    const Entity& entity,
    const Matrix& effect_transform) const {
  if (inputs.empty()) {
    return std::nullopt;
  }
  std::optional<Rect> input_coverage = inputs[0]->GetCoverage(entity);
  if (!input_coverage.has_value()) {
    return std::nullopt;
  }
  Scalar radius = EffectiveRadius();
  Vector2 scale = effect_transform.GetScale().Abs();
  return input_coverage->Expand(
      Point(radius * scale.x, radius * scale.y));
}

std::optional<Entity> DualKawaseBlurFilterContents::RenderFilter(
    const FilterInput::Vector& input_textures,
    const ContentContext& renderer,
    const Entity& entity,
    const Matrix& effect_transform,
    const Rect& coverage,
    const std::optional<Rect>& coverage_hint) const {
  // EXPERIMENT STUB — full multi-pass MakeSubpass chain lands once Dual Kawase
  // pipelines are registered on ContentContext (see experiment doc checklist).
  //
  // Intended pass graph (mirrors gpublur DualKawaseBlur::blur):
  //   input → down[0] (½) → down[1] (¼) → … → down[levels-1]
  //        → up[levels-2] → … → up[0] → full-res result
  // Each pass: linear clamp sampler, FragInfo { half_pixel, offset }.
  //
  // Until pipelines exist, fall back is intentionally empty so misuse fails
  // loudly in debug rather than silently looking like Gaussian.
  FML_LOG(ERROR) << "DualKawaseBlurFilterContents::RenderFilter is not wired "
                    "yet. Register DualKawase downsample/upsample pipelines "
                    "and implement MakeSubpass chain. See "
                    "docs/engine/impeller/Dual-Kawase-Blur-Experiment.md";
  (void)input_textures;
  (void)renderer;
  (void)entity;
  (void)effect_transform;
  (void)coverage;
  (void)coverage_hint;
  (void)tile_mode_;
  (void)bounds_;
  (void)mask_blur_style_;
  (void)mask_geometry_;
  return std::nullopt;
}

}  // namespace impeller

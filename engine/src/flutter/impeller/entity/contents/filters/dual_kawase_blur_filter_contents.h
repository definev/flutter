// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_DUAL_KAWASE_BLUR_FILTER_CONTENTS_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_DUAL_KAWASE_BLUR_FILTER_CONTENTS_H_

#include <optional>

#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/geometry/geometry.h"

namespace impeller {

/// Experimental Dual Kawase blur approximating a Gaussian of the given sigma.
///
/// Pyramid of 5-tap downsamples + 8-tap tent upsamples. Cost scales with
/// levels (O(log radius)) instead of Gaussian kernel width.
///
/// Gated behind the Dual Kawase engine experiment — see
/// docs/engine/impeller/Dual-Kawase-Blur-Experiment.md.
class DualKawaseBlurFilterContents final : public FilterContents {
 public:
  static constexpr int kMaxLevels = 5;

  explicit DualKawaseBlurFilterContents(
      Scalar sigma_x,
      Scalar sigma_y,
      Entity::TileMode tile_mode,
      std::optional<Rect> bounds = std::nullopt,
      BlurStyle mask_blur_style = BlurStyle::kNormal,
      const Geometry* mask_geometry = nullptr);

  Scalar GetSigmaX() const { return sigma_.x; }
  Scalar GetSigmaY() const { return sigma_.y; }
  int GetLevels() const { return levels_; }
  Scalar GetOffset() const { return offset_; }

  /// Maps Gaussian sigma → Dual Kawase (levels, offset).
  /// WebRender / gpublur fit: levels ≈ (4/3)·ln(σ), offset ≈ 0.4538^levels · σ.
  static void ApproximateGaussian(Scalar sigma,
                                  int max_levels,
                                  int& out_levels,
                                  Scalar& out_offset);

  // |FilterContents|
  std::optional<Rect> GetFilterSourceCoverage(
      const Matrix& effect_transform,
      const Rect& output_limit) const override;

  // |FilterContents|
  std::optional<Rect> GetFilterCoverage(
      const FilterInput::Vector& inputs,
      const Entity& entity,
      const Matrix& effect_transform) const override;

 private:
  // |FilterContents|
  std::optional<Entity> RenderFilter(
      const FilterInput::Vector& input_textures,
      const ContentContext& renderer,
      const Entity& entity,
      const Matrix& effect_transform,
      const Rect& coverage,
      const std::optional<Rect>& coverage_hint) const override;

  Scalar EffectiveRadius() const;

  const Vector2 sigma_;
  const Entity::TileMode tile_mode_;
  const std::optional<Rect> bounds_;
  const BlurStyle mask_blur_style_;
  const Geometry* mask_geometry_ = nullptr;
  int levels_ = 1;
  Scalar offset_ = 1.0f;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_DUAL_KAWASE_BLUR_FILTER_CONTENTS_H_

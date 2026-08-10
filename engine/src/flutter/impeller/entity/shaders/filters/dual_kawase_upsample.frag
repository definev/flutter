// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Dual Kawase upsample: 8-tap tent filter.
// 4 edge midpoints (weight 1) + 4 diagonals (weight 2), normalized by 12.
//
// EXPERIMENT: not registered in entity_shaders until pipeline wiring lands.
// See docs/engine/impeller/Dual-Kawase-Blur-Experiment.md.

#include <impeller/types.glsl>

uniform f16sampler2D texture_sampler;

uniform FragInfo {
  vec2 half_pixel;
  float offset;
  float _pad;
}
frag_info;

in vec2 v_texture_coords;

out f16vec4 frag_color;

void main() {
  vec2 uv = v_texture_coords;
  vec2 hp = frag_info.half_pixel * frag_info.offset;

  f16vec4 s = texture(texture_sampler, uv + vec2(-hp.x * 2.0, 0.0));
  s += texture(texture_sampler, uv + vec2(-hp.x, hp.y)) * float16_t(2.0);
  s += texture(texture_sampler, uv + vec2(0.0, hp.y * 2.0));
  s += texture(texture_sampler, uv + vec2(hp.x, hp.y)) * float16_t(2.0);
  s += texture(texture_sampler, uv + vec2(hp.x * 2.0, 0.0));
  s += texture(texture_sampler, uv + vec2(hp.x, -hp.y)) * float16_t(2.0);
  s += texture(texture_sampler, uv + vec2(0.0, -hp.y * 2.0));
  s += texture(texture_sampler, uv + vec2(-hp.x, -hp.y)) * float16_t(2.0);
  frag_color = s / float16_t(12.0);
}

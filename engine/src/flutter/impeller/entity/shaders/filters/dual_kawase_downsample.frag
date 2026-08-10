// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Dual Kawase downsample: center-weighted 5-tap kernel.
// Offsets are destination half-texel * uOffset. Bilinear filtering makes each
// fetch average 4 source texels.
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

  f16vec4 sum = texture(texture_sampler, uv) * float16_t(4.0);
  sum += texture(texture_sampler, uv - hp);
  sum += texture(texture_sampler, uv + hp);
  sum += texture(texture_sampler, uv + vec2(hp.x, -hp.y));
  sum += texture(texture_sampler, uv - vec2(hp.x, -hp.y));
  frag_color = sum / float16_t(8.0);
}

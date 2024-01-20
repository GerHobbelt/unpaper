// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2023 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "imageprocess/primitives.h"
#include "imageprocess/math_util.h"

Rectangle rectangle_from_size(Point origin, RectangleSize size) {
  return (Rectangle){
      .vertex =
          {
              origin,
              {
                  .x = origin.x + size.width - 1,
                  .y = origin.y + size.height - 1,
              },
          },
  };
}

RectangleSize size_of_rectangle(Rectangle rect) {
  return (RectangleSize){
      .width = abs(rect.vertex[0].x - rect.vertex[1].x) + 1,
      .height = abs(rect.vertex[0].y - rect.vertex[1].y) + 1,
  };
}

Rectangle normalize_rectangle(Rectangle input) {
  return (Rectangle){
      .vertex =
          {
              {
                  .x = min(input.vertex[0].x, input.vertex[1].x),
                  .y = min(input.vertex[0].y, input.vertex[1].y),
              },
              {
                  .x = max(input.vertex[0].x, input.vertex[1].x),
                  .y = max(input.vertex[0].y, input.vertex[1].y),
              },
          },
  };
}

Rectangle clip_rectangle(AVFrame *image, Rectangle area) {
  Rectangle normal_area = normalize_rectangle(area);

  return (Rectangle){
      .vertex =
          {
              {
                  .x = max(normal_area.vertex[0].x, 0),
                  .y = max(normal_area.vertex[0].y, 0),
              },
              {
                  .x = min(normal_area.vertex[1].x, (image->width - 1)),
                  .y = min(normal_area.vertex[1].y, (image->height - 1)),
              },
          },
  };
}

uint64_t count_pixels(Rectangle area) {
  RectangleSize size = size_of_rectangle(area);

  return size.width * size.height;
}

bool point_in_rectangle(Point p, Rectangle input_area) {
  Rectangle area = normalize_rectangle(input_area);

  return (p.x >= area.vertex[0].x && p.x <= area.vertex[1].x &&
          p.y >= area.vertex[0].y && p.y <= area.vertex[1].y);
}

bool rectangles_overlap(Rectangle first_input, Rectangle second_input) {
  Rectangle first = normalize_rectangle(first_input);
  Rectangle second = normalize_rectangle(second_input);

  return point_in_rectangle(first.vertex[0], second) ||
         point_in_rectangle(first.vertex[1], second);
}

bool rectangle_overlap_any(Rectangle first_input, size_t count,
                           Rectangle *rectangles) {
  for (size_t n = 0; n < count; n++) {
    if (rectangles_overlap(first_input, rectangles[n])) {
      return true;
    }
  }

  return false;
}

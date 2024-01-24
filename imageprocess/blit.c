// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "imageprocess/blit.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

/**
 * Wipe a rectangular area of pixels with the defined color.
 * @return The number of pixels actually changed.
 */
uint64_t wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  Rectangle area = clip_rectangle(image, input_area);

  scan_rectangle(area) {
    if (set_pixel(image, (Point){x, y}, color, abs_black_threshold)) {
      count++;
    }
  }

  return count;
}

void copy_rectangle(AVFrame *source, AVFrame *target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold) {
  Rectangle area = clip_rectangle(source, source_area);

  // naive but generic implementation
  for (int32_t sY = area.vertex[0].y, tY = target_coords.y;
       sY <= area.vertex[1].y; sY++, tY++) {
    for (int32_t sX = area.vertex[0].x, tX = target_coords.x;
         sX <= area.vertex[1].x; sX++, tX++) {
      set_pixel(target, (Point){tX, tY}, get_pixel(source, (Point){sX, sY}),
                abs_black_threshold);
    }
  }
}

/**
 * Returns the average brightness of a rectangular area.
 */
uint8_t inverse_brightness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t grayscale = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  if (count == 0) {
    return 0;
  }

  scan_rectangle(area) {
    grayscale += get_pixel_grayscale(image, (Point){x, y});
  }

  return 0xFF - (grayscale / count);
}

/**
 * Returns the inverse average lightness of a rectangular area.
 */
uint8_t inverse_lightness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t lightness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  if (count == 0) {
    return 0;
  }

  scan_rectangle(area) {
    lightness += get_pixel_lightness(image, (Point){x, y});
  }

  return 0xFF - (lightness / count);
}

/**
 * Returns the average darkness of a rectangular area.
 */
uint8_t darkness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t darkness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  if (count == 0) {
    return 0;
  }

  scan_rectangle(area) {
    darkness += get_pixel_darkness_inverse(image, (Point){x, y});
  }

  return 0xFF - (darkness / count);
}

uint64_t count_pixels_within_brightness(AVFrame *image, Rectangle area,
                                        uint8_t min_brightness,
                                        uint8_t max_brightness, bool clear,
                                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  scan_rectangle(area) {
    Point p = {x, y};
    uint8_t brightness = get_pixel_grayscale(image, p);
    if (brightness < min_brightness || brightness > max_brightness) {
      continue;
    }

    if (clear) {
      set_pixel(image, p, PIXEL_WHITE, abs_black_threshold);
    }
    count++;
  }

  return count;
}

/**
 * Allocates a memory block for storing image data and fills the AVFrame-struct
 * with the specified values.
 */
AVFrame *create_image(RectangleSize size, int pixel_format, bool fill,
                      Pixel sheet_background, uint8_t abs_black_threshold) {
  AVFrame *image;

  image = av_frame_alloc();
  image->width = size.width;
  image->height = size.height;
  image->format = pixel_format;

  int ret = av_frame_get_buffer(image, 8);
  if (ret < 0) {
    char errbuff[1024];
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to allocate buffer: %s", errbuff);
  }

  if (fill) {
    wipe_rectangle(image, RECT_FULL_IMAGE, sheet_background,
                   abs_black_threshold);
  }

  return image;
}

void replace_image(AVFrame **image, AVFrame **new_image) {
  free_image(image);
  *image = *new_image;
  *new_image = NULL;
}

void free_image(AVFrame **pImage) { av_frame_free(pImage); }

/**
 * Centers one area of an image inside an area of another image.
 * If the source area is smaller than the target area, is is equally
 * surrounded by a white border, if it is bigger, it gets equally cropped
 * at the edges.
 */
void center_image(AVFrame *source, AVFrame *target, Point target_origin,
                  RectangleSize target_size, Pixel sheet_background,
                  uint8_t abs_black_threshold) {
  Point source_origin = POINT_ORIGIN;
  RectangleSize source_size =
      size_of_rectangle(clip_rectangle(source, RECT_FULL_IMAGE));

  if (source_size.width < target_size.width ||
      source_size.height < target_size.height) {
    wipe_rectangle(target, rectangle_from_size(target_origin, target_size),
                   sheet_background, abs_black_threshold);
  }

  if (source_size.width <= target_size.width) {
    target_origin.x += (target_size.width - source_size.width) / 2;
  } else {
    source_origin.x += (source_size.width - target_size.width) / 2;
    source_size.width = target_size.width;
  }
  if (source_size.height <= target_size.height) {
    target_origin.y += (target_size.height - source_size.height) / 2;
  } else {
    source_origin.y += (source_size.height - target_size.height) / 2;
    source_size.height = target_size.height;
  }

  copy_rectangle(source, target,
                 rectangle_from_size(source_origin, source_size), target_origin,
                 abs_black_threshold);
}

static void stretch_frame(AVFrame *source, AVFrame *target,
                          Interpolation interpolate_type,
                          uint8_t abs_black_threshold) {
  const float horizontal_ratio = (float)source->width / (float)target->width;
  const float vertical_ratio = (float)source->height / (float)target->height;

  verboseLog(VERBOSE_MORE, "stretching %dx%d -> %dx%d\n", source->width,
             source->height, target->width, target->height);

  Rectangle target_area = clip_rectangle(target, RECT_FULL_IMAGE);
  scan_rectangle(target_area) {
    const Point target_coords = {x, y};
    const FloatPoint source_coords = {x * horizontal_ratio, y * vertical_ratio};
    set_pixel(target, target_coords,
              interpolate(source, source_coords, interpolate_type),
              abs_black_threshold);
  }
}

void stretch_and_replace(AVFrame **pImage, RectangleSize size,
                         Interpolation interpolate_type,
                         uint8_t abs_black_threshold) {
  if ((*pImage)->width == size.width && (*pImage)->height == size.height)
    return;

  AVFrame *target = create_image(size, (*pImage)->format, false, PIXEL_WHITE,
                                 abs_black_threshold);

  stretch_frame(*pImage, target, interpolate_type, abs_black_threshold);
  replace_image(pImage, &target);
}

void resize_and_replace(AVFrame **pImage, RectangleSize size,
                        Interpolation interpolate_type, Pixel sheet_background,
                        uint8_t abs_black_threshold) {
  if ((*pImage)->width == size.width && (*pImage)->height == size.height)
    return;

  verboseLog(VERBOSE_NORMAL, "resizing %dx%d -> %dx%d\n", (*pImage)->width,
             (*pImage)->height, size.width, size.height);

  const float horizontal_ratio = (float)size.width / (float)(*pImage)->width;
  const float vertical_ratio = (float)size.height / (float)(*pImage)->height;

  RectangleSize stretch_size;
  if (horizontal_ratio < vertical_ratio) {
    // horizontally more shrinking/less enlarging is needed:
    // fill width fully, adjust height
    stretch_size =
        (RectangleSize){size.width, (*pImage)->height * horizontal_ratio};
  } else if (vertical_ratio < horizontal_ratio) {
    stretch_size =
        (RectangleSize){(*pImage)->width * vertical_ratio, size.height};
  } else { // wRat == hRat
    stretch_size = size;
  }
  stretch_and_replace(pImage, stretch_size, interpolate_type,
                      abs_black_threshold);

  // Check if any centering needs to be done, otherwise make a new
  // copy, center and return that.  Check for the stretched
  // width/height to be the same rather than comparing the ratio, as
  // it is possible that the ratios are just off enough that they
  // generate the same width/height.
  if (size.width == stretch_size.width && size.height == stretch_size.height) {
    return;
  }

  AVFrame *resized = create_image(size, (*pImage)->format, true,
                                  sheet_background, abs_black_threshold);
  center_image(*pImage, resized, POINT_ORIGIN, size, sheet_background,
               abs_black_threshold);
  replace_image(pImage, &resized);
}

void flip_rotate_90(AVFrame **pImage, RotationDirection direction,
                    uint8_t abs_black_threshold) {

  // exchanged width and height
  AVFrame *newimage =
      create_image((RectangleSize){(*pImage)->height, (*pImage)->width},
                   (*pImage)->format, false, PIXEL_WHITE, abs_black_threshold);

  for (int y = 0; y < (*pImage)->height; y++) {
    const int xx =
        ((direction > 0) ? (*pImage)->height - 1 : 0) - y * direction;
    for (int x = 0; x < (*pImage)->width; x++) {
      const int yy =
          ((direction < 0) ? (*pImage)->width - 1 : 0) + x * direction;

      Point point1 = {x, y};
      Point point2 = {xx, yy};

      set_pixel(newimage, point2, get_pixel(*pImage, point1),
                abs_black_threshold);
    }
  }
  replace_image(pImage, &newimage);
}

void mirror(AVFrame *image, bool horizontal, bool vertical,
            uint8_t abs_black_threshold) {
  Rectangle source = {{POINT_ORIGIN, POINT_INFINITY}};

  if (horizontal && !vertical) {
    source.vertex[1].x = (image->width - 1) / 2;
  }

  if (vertical) {
    source.vertex[1].y = (image->height - 1) / 2;
  }

  source = clip_rectangle(image, source);

  // Cannot use scan_rectangle() because of the midpoint turn.
  for (int32_t y = source.vertex[0].y; y <= source.vertex[1].y; y++) {
    int32_t yy = vertical ? image->height - y - 1 : y;
    // Special case: the last middle line in odd-lined images that are
    // to be mirrored both horizontally and vertically.
    if (vertical && horizontal && y == yy) {
      source.vertex[1].x = (image->width - 1) / 2;
    }

    for (int32_t x = 0; x <= source.vertex[1].x; x++) {
      int32_t xx = horizontal ? image->width - x - 1 : x;

      Point point1 = {x, y};
      Point point2 = {xx, yy};
      Pixel pixel1 = get_pixel(image, point1);
      Pixel pixel2 = get_pixel(image, point2);
      set_pixel(image, point1, pixel2, abs_black_threshold);
      set_pixel(image, point2, pixel1, abs_black_threshold);
    }
  }
}

void shift_image(AVFrame **pImage, Delta d, Pixel sheet_background,
                 uint8_t abs_black_threshold) {
  // allocate new buffer's memory
  AVFrame *newimage = create_image(
      (RectangleSize){(*pImage)->width, (*pImage)->height}, (*pImage)->format,
      true, sheet_background, abs_black_threshold);

  copy_rectangle(*pImage, newimage, RECT_FULL_IMAGE,
                 shift_point(POINT_ORIGIN, d), abs_black_threshold);
  replace_image(pImage, &newimage);
}

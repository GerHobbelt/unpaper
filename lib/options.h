// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>

#include <libavutil/pixfmt.h>

#include "constants.h"
#include "imageprocess/deskew.h"
#include "imageprocess/filters.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/masks.h"
#include "imageprocess/primitives.h"
#include "parse.h"

typedef struct {
  bool write_output;
  bool overwrite_output;
  bool multiple_sheets;
  enum AVPixelFormat output_pixel_format;

  Layout layout;
  int start_sheet;
  int end_sheet;
  int start_input;
  int start_output;
  int input_count;
  int output_count;

  struct MultiIndex sheet_multi_index;
  struct MultiIndex exclude_multi_index;
  struct MultiIndex ignore_multi_index;
  struct MultiIndex insert_blank;
  struct MultiIndex replace_blank;

  // 0: allow all, -1: disable all, n: individual entries
  struct MultiIndex no_blackfilter_multi_index;
  struct MultiIndex no_noisefilter_multi_index;
  struct MultiIndex no_blurfilter_multi_index;
  struct MultiIndex no_grayfilter_multi_index;
  struct MultiIndex no_mask_scan_multi_index;
  struct MultiIndex no_mask_center_multi_index;
  struct MultiIndex no_deskew_multi_index;
  struct MultiIndex no_wipe_multi_index;
  struct MultiIndex no_border_multi_index;
  struct MultiIndex no_border_scan_multi_index;
  struct MultiIndex no_border_align_multi_index;

  Wipes pre_wipes;
  Wipes wipes;
  Wipes post_wipes;

  Delta pre_shift;
  Delta post_shift;

  int16_t pre_rotate;
  int16_t post_rotate;

  Direction pre_mirror;
  Direction post_mirror;

  RectangleSize sheet_size;
  RectangleSize page_size;
  RectangleSize post_page_size;
  RectangleSize stretch_size;
  RectangleSize post_stretch_size;

  float pre_zoom_factor;
  float post_zoom_factor;

  Pixel sheet_background;
  Pixel mask_color;
  uint8_t abs_black_threshold;
  uint8_t abs_white_threshold;

  Border pre_border;
  Border border;
  Border post_border;

  DeskewParameters deskew_parameters;
  MaskDetectionParameters mask_detection_parameters;
  MaskAlignmentParameters mask_alignment_parameters;
  BorderScanParameters border_scan_parameters;

  Interpolation interpolate_type;
  GrayfilterParameters grayfilter_parameters;
  BlackfilterParameters blackfilter_parameters;
  BlurfilterParameters blurfilter_parameters;
  uint64_t noisefilter_intensity;
} Options;

void options_init(Options *o);

bool parse_symmetric_integers(const char *str, int32_t *value_1,
                              int32_t *value_2);
bool parse_symmetric_floats(const char *str, float *value_1, float *value_2);

bool parse_rectangle(const char *str, Rectangle *rect);
int print_rectangle(Rectangle rect);

bool parse_rectangle_size(const char *str, RectangleSize *size);
int print_rectangle_size(RectangleSize size);

bool parse_delta(const char *str, Delta *delta);
bool parse_scan_step(const char *str, Delta *delta);
int print_delta(Delta delta);

bool parse_wipe(const char *optname, const char *str, Wipes *wipes);

bool parse_border(const char *str, Border *rect);
int print_border(Border rect);

bool parse_color(const char *str, Pixel *color);
int print_color(Pixel color);

bool parse_direction(const char *str, Direction *direction);
const char *direction_to_string(Direction direction);

bool parse_edges(const char *str, Edges *edges);
int print_edges(Edges edges);

bool parse_layout(const char *str, Layout *layout);

bool parse_interpolate(const char *str, Interpolation *interpolation);

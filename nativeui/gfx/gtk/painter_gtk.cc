// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_GFX_GTK_PAINTER_GTK_CC_
#define NATIVEUI_GFX_GTK_PAINTER_GTK_CC_

#include "nativeui/gfx/gtk/painter_gtk.h"

#include <math.h>

#include <gtk/gtk.h>

#include "nativeui/gfx/canvas.h"
#include "nativeui/gfx/font.h"
#include "nativeui/gfx/image.h"

namespace nu {

PainterGtk::PainterGtk(cairo_t* context)
    : context_(context),
      is_context_managed_(false) {
  Initialize();
}

PainterGtk::PainterGtk(cairo_surface_t* surface, float scale_factor)
    : context_(cairo_create(surface)),
      is_context_managed_(true) {
  Initialize();
}

PainterGtk::~PainterGtk() {
  if (is_context_managed_)
    cairo_destroy(context_);
}

void PainterGtk::Save() {
  states_.push(states_.top());
  cairo_save(context_);
}

void PainterGtk::Restore() {
  if (states_.empty())
    return;
  states_.pop();
  cairo_restore(context_);
}

void PainterGtk::BeginPath() {
  cairo_new_path(context_);
}

void PainterGtk::ClosePath() {
  cairo_close_path(context_);
}

void PainterGtk::MoveTo(const PointF& point) {
  cairo_move_to(context_, point.x(), point.y());
}

void PainterGtk::LineTo(const PointF& point) {
  cairo_line_to(context_, point.x(), point.y());
}

void PainterGtk::BezierCurveTo(const PointF& cp1,
                               const PointF& cp2,
                               const PointF& ep) {
  cairo_curve_to(
      context_, cp1.x(), cp1.y(), cp2.x(), cp2.y(), ep.x(), ep.y());
}

void PainterGtk::Arc(const PointF& point, float radius, float sa, float ea) {
  cairo_arc(context_, point.x(), point.y(), radius, sa, ea);
}

void PainterGtk::Rect(const RectF& rect) {
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
}

void PainterGtk::Clip() {
  cairo_clip(context_);
}

void PainterGtk::ClipRect(const RectF& rect) {
  cairo_new_path(context_);
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  cairo_clip(context_);
}

void PainterGtk::Translate(const Vector2dF& offset) {
  cairo_translate(context_, offset.x(), offset.y());
}

void PainterGtk::Rotate(float angle) {
  cairo_rotate(context_, angle);
}

void PainterGtk::Scale(const Vector2dF& scale) {
  cairo_scale(context_, scale.x(), scale.y());
}

void PainterGtk::SetColor(Color color) {
  states_.top().stroke_color = color;
  states_.top().fill_color = color;
}

void PainterGtk::SetStrokeColor(Color color) {
  states_.top().stroke_color = color;
}

void PainterGtk::SetFillColor(Color color) {
  states_.top().fill_color = color;
}

void PainterGtk::SetLineWidth(float width) {
  cairo_set_line_width(context_, width);
}

void PainterGtk::Stroke() {
  SetSourceColor(true);
  cairo_stroke(context_);
}

void PainterGtk::Fill() {
  SetSourceColor(false);
  cairo_fill(context_);
}

void PainterGtk::StrokeRect(const RectF& rect) {
  cairo_new_path(context_);
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  SetSourceColor(true);
  cairo_stroke(context_);
}

void PainterGtk::FillRect(const RectF& rect) {
  cairo_new_path(context_);
  cairo_rectangle(context_, rect.x(), rect.y(), rect.width(), rect.height());
  SetSourceColor(false);
  cairo_fill(context_);
}

void PainterGtk::DrawImage(Image* image, const RectF& rect) {
  DrawImageFromRect(image, RectF(image->GetSize()), rect);
}

void PainterGtk::DrawImageFromRect(Image* image, const RectF& src,
                                   const RectF& dest) {
  RectF ps = ScaleRect(src, image->GetScaleFactor());
  cairo_save(context_);
  // Clip the image to |dest|.
  cairo_translate(context_, dest.x(), dest.y());
  cairo_new_path(context_);
  cairo_rectangle(context_, 0, 0, dest.width(), dest.height());
  cairo_clip(context_);
  // Scale if needed.
  float x_scale = dest.width() / ps.width();
  float y_scale = dest.height() / ps.height();
  if (x_scale != 1.0f || y_scale != 1.0f)
    cairo_scale(context_, x_scale, y_scale);
  // Draw.
  GdkPixbuf* pixbuf = gdk_pixbuf_animation_get_static_image(image->GetNative());
  gdk_cairo_set_source_pixbuf(context_, pixbuf, -ps.x(), -ps.y());
  cairo_paint(context_);
  cairo_restore(context_);
}

void PainterGtk::DrawCanvas(Canvas* canvas, const RectF& rect) {
  DrawCanvasFromRect(canvas, RectF(canvas->GetSize()), rect);
}

void PainterGtk::DrawCanvasFromRect(Canvas* canvas, const RectF& src,
                                    const RectF& dest) {
  cairo_save(context_);
  // Clip the image to |dest|.
  cairo_translate(context_, dest.x(), dest.y());
  cairo_new_path(context_);
  cairo_rectangle(context_, 0, 0, dest.width(), dest.height());
  cairo_clip(context_);
  // Scale if needed.
  float x_scale = dest.width() / src.width();
  float y_scale = dest.height() / src.height();
  if (x_scale != 1.0f || y_scale != 1.0f)
    cairo_scale(context_, x_scale, y_scale);
  // Draw.
  cairo_set_source_surface(context_, canvas->GetBitmap(), -src.x(), -src.y());
  cairo_paint(context_);
  cairo_restore(context_);
}

TextMetrics PainterGtk::MeasureText(const std::string& text, float width,
                                    const TextAttributes& attributes) {
  PangoLayout* layout = pango_cairo_create_layout(context_);
  pango_layout_set_font_description(layout, attributes.font->GetNative());
  pango_layout_set_text(layout, text.data(), text.length());
  if (width >= 0)
    pango_layout_set_width(layout, width * PANGO_SCALE);
  int bwidth, bheight;
  pango_layout_get_pixel_size(layout, &bwidth, &bheight);
  g_object_unref(layout);
  return { SizeF(bwidth, bheight) };
}

void PainterGtk::DrawText(const std::string& text, const RectF& rect,
                          const TextAttributes& attributes) {
  PangoLayout* layout = pango_cairo_create_layout(context_);
  pango_layout_set_font_description(layout, attributes.font->GetNative());
  cairo_save(context_);

  // Text size.
  int width, height;
  if (attributes.wrap) {
    pango_layout_set_width(layout, std::floor(rect.width() * PANGO_SCALE));
    pango_layout_set_height(layout, std::floor(rect.height() *PANGO_SCALE));
  } else {
    pango_layout_set_width(layout, -1);
  }
  pango_layout_set_text(layout, text.data(), text.length());
  pango_layout_get_pixel_size(layout, &width, &height);

  // Horizontal alignment.
  RectF bounds(rect);
  if (attributes.align == TextAlign::Center)
    bounds.Inset((rect.width() - width) / 2.f, 0.f);
  else if (attributes.align == TextAlign::End)
    bounds.Inset(rect.width() - width, 0.f, 0.f, 0.f);

  // Vertical alignment
  if (attributes.valign == TextAlign::Center)
    bounds.Inset(0.f, (rect.height() - height) / 2);
  else if (attributes.valign == TextAlign::End)
    bounds.Inset(0.f, rect.height() - height, 0.f, 0.f);

  // Apply the color.
  Color color = attributes.color;
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);

  // Don't draw outside boundry.
  ClipRect(rect);

  // Draw text.
  cairo_move_to(context_, bounds.x(), bounds.y());
  if (attributes.ellipsis)
    pango_layout_set_ellipsize(layout, PANGO_ELLIPSIZE_END);
  if (attributes.wrap) {
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_width(layout, bounds.width() * PANGO_SCALE);
    pango_layout_set_height(layout, bounds.height() * PANGO_SCALE);
  } else {
    pango_layout_set_width(layout, -1);
  }
  pango_cairo_show_layout(context_, layout);

  cairo_restore(context_);
  g_object_unref(layout);
}

void PainterGtk::Initialize() {
  // Initial state.
  states_.push({Color(), Color()});
}

void PainterGtk::SetSourceColor(bool stroke) {
  Color color = stroke ? states_.top().stroke_color
                       : states_.top().fill_color;
  cairo_set_source_rgba(context_, color.r() / 255., color.g() / 255.,
                                  color.b() / 255., color.a() / 255.);
}

}  // namespace nu

#endif  // NATIVEUI_GFX_GTK_PAINTER_GTK_CC_

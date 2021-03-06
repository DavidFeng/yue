// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_MAC_NU_PRIVATE_H_
#define NATIVEUI_MAC_NU_PRIVATE_H_

#include <functional>
#include <memory>

#include "base/mac/scoped_nsobject.h"

@class DataProvider;
@class NSCursor;
@class NSTrackingArea;

namespace nu {

class MouseCapture;
class View;

// A private class that holds nativeui specific private data.
// Object-C does not support multi-inheiritance, so it is impossible to add
// common data members for UI elements. Our workaround is to manually add
// this class as member for each view.
struct NUPrivate {
  NUPrivate();
  ~NUPrivate();

  View* shell = nullptr;
  bool focusable = true;
  bool draggable = false;
  bool hovered = false;
  bool is_content_view = false;
  bool wants_layer = false;  // default value for wantsLayer
  bool wants_layer_infected = false;  // infects the wantsLayer property
  base::scoped_nsobject<NSCursor> cursor;
  base::scoped_nsobject<NSTrackingArea> tracking_area;
  std::unique_ptr<MouseCapture> mouse_capture;

  // Drag target properties.
  int last_drop_operation = -1;  // cached drop result

  // Drag source properties.
  int supported_drag_operation = 0;  // supported drag operation
  int drag_result = 0;  // cached drag result
  std::function<void()> quit_dragging;  // quit current drag session
  base::scoped_nsobject<DataProvider> data_source;  // data of drag source
};

}  // namespace nu

#endif  // NATIVEUI_MAC_NU_PRIVATE_H_

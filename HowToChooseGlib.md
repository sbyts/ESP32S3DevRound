# UI Framework vs Graphics Driver: How to Choose for ESP32-S3 and Circular Displays

When building graphic applications for round displays (like GC9B72 or GC9A01) on the **ESP32-S3**, choosing the right library architecture determines your project's performance, frame rate (FPS), and visual quality. Below is a structural comparison between **LVGL** (a heavy UI framework) and **LovyanGFX** (a low-level hardware-accelerated graphics driver) to help you make an informed architectural decision.

---

## Direct Comparison

| Selection Criteria | LVGL (Light and Versatile Graphics Library) | LovyanGFX (LGFX Hardware Driver) |
| :--- | :--- | :--- |
| **Architecture Type** | Full-scale heavyweight UI framework (resembles an Android/iOS view ecosystem for microcontrollers). | Lightweight, low-level hardware abstraction layer (HAL) + direct graphics rendering engine. |
| **Arc & Circle Rendering** | Demands complex software floating-point pipelines. Frequently produces aliasing artifacts or jagged edges unless slow CPU-bound Anti-Aliasing is toggled. | **Native hardware anti-aliasing.** Leverages sub-pixel edge smoothing directly integrated with ESP32-S3 math extensions for flawless arcs. |
| **Memory Footprint (RAM/ROM)** | **Heavyweight.** Allocates significant internal RAM allocations for deep object hierarchies, widgets, styles, and dirty-region structures. | **Minimalist.** Allocates memory strictly for user-defined canvases or DMA sprites, leaving the rest of the SRAM untouched. |
| **Performance & Refresh Rates** | Difficult to sustain high frame rates (FPS) on large $360\times360$ round panels without extensive PSRAM tuning and hardware canvas patching. | **Maximum FPS throughput.** Ships with tight, direct integration into the ESP32-S3 DMA channels for asynchronous background flushing. |
| **Code Structure Complexity** | High complexity. Altering a simple arc property requires creating objects, initializing style descriptors, and binding them via callbacks. | Very simple. Executed via straightforward, immediate-mode drawing commands like `spr.fillArc(...)`. |

---

## Architectural Analysis: Why LVGL Causes Artifacts on Round Displays

Many developers experience **thin pixel artifacts, moiré patterns, or inverted color borders** around complex widgets (such as `lv_meter`) when porting LVGL to specialized round displays. This behavior stems from two core architectural issues:

1. **Buffer Alignment and Stride Overlaps:** Round display controllers read data in rigid pixel blocks. Because LVGL renders UI changes in partial rectangular "dirty zones", any misalignment between the widget bounding box and the round display's memory stride causes adjacent sub-pixels to shift, rendering as thin phantom lines on the edges of arcs.
2. **Sub-pixel Anti-Aliasing Overhead:** LVGL handles anti-aliasing by blending background pixels. If your display pipeline experiences internal color-space conversion bugs (e.g., RGB vs. BRG byte swapping), the calculated edge blending pixels will pick up inverted color hues, destroying the clean vector look of dial needles and circular scales.

Conversely, **LovyanGFX** bypasses these abstract layout layers entirely. By rendering directly into a localized 16-bit DMA sprite buffer matching the native color mapping (BRG/RGB) of the panel, every trigonometric calculation lines up exactly with the physical pixel matrix, completely preventing artifacts.

---

## Summary Recommendation

* Choose **LVGL** if you are building an extensive multi-page system with complex touch interactions, scrolling list views, drop-down menus, and multi-layered window components where UI layout management outweighs raw rendering speed.
* Choose **LovyanGFX** if your primary goal is a high-performance, smooth, and artifact-free hardware interface (like a modern digital watch face, an automotive gauge, or a real-time climate telemetry monitor) requiring flawless anti-aliased geometry and an immediate-mode codebase.